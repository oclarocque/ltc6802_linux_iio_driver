#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spi/spi.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/device.h>

#include <linux/iio/iio.h>
#include <linux/iio/buffer.h>
#include <linux/iio/trigger.h>
#include <linux/iio/trigger_consumer.h>
#include <linux/iio/triggered_buffer.h>

/* LTC6802 Commands */
#define LTC6802_CMD_WRCFG		0x01
#define LTC6802_CMD_RDCFG		0x02
#define LTC6802_CMD_RDCV		0x04
#define LTC6802_CMD_RDFLG		0x06
#define LTC6802_CMD_RDTMP		0x08
#define LTC6802_CMD_STCVAD		0x10
  #define LTC6802_CMD_STCVAD_CELL1	0x11
  #define LTC6802_CMD_STCVAD_CELL2	0x12
  #define LTC6802_CMD_STCVAD_CELL3	0x13
  #define LTC6802_CMD_STCVAD_CELL4	0x14
  #define LTC6802_CMD_STCVAD_CELL5	0x15
  #define LTC6802_CMD_STCVAD_CELL6	0x16
  #define LTC6802_CMD_STCVAD_CELL7	0x17
  #define LTC6802_CMD_STCVAD_CELL8	0x18
  #define LTC6802_CMD_STCVAD_CELL9	0x19
  #define LTC6802_CMD_STCVAD_CELL10	0x1A
  #define LTC6802_CMD_STCVAD_CELL11	0x1B /* if CELL10 bit = 0 */
  #define LTC6802_CMD_STCVAD_CELL12	0x1C /* if CELL10 bit = 0 */
  #define LTC6802_CMD_STCVAD_TEST1	0x1E
  #define LTC6802_CMD_STCVAD_TEST2	0x1F
#define LTC6802_CMD_STOWAD		0x10
  #define LTC6802_CMD_STOWAD_CELL1	0x21
  #define LTC6802_CMD_STOWAD_CELL2	0x22
  #define LTC6802_CMD_STOWAD_CELL3	0x23
  #define LTC6802_CMD_STOWAD_CELL4	0x24
  #define LTC6802_CMD_STOWAD_CELL5	0x25
  #define LTC6802_CMD_STOWAD_CELL6	0x26
  #define LTC6802_CMD_STOWAD_CELL7	0x27
  #define LTC6802_CMD_STOWAD_CELL8	0x28
  #define LTC6802_CMD_STOWAD_CELL9	0x29
  #define LTC6802_CMD_STOWAD_CELL10	0x2A
  #define LTC6802_CMD_STOWAD_CELL11	0x2B /* if CELL10 bit = 0 */
  #define LTC6802_CMD_STOWAD_CELL12	0x2C /* if CELL10 bit = 0 */
  #define LTC6802_CMD_STOWAD_TEST1	0x2E
  #define LTC6802_CMD_STOWAD_TEST2	0x2F
#define LTC6802_CMD_STTMPAD		0x30
  #define LTC6802_CMD_STTMPAD_ETEMP1	0x31
  #define LTC6802_CMD_STTMPAD_ETEMP2	0x32
  #define LTC6802_CMD_STTMPAD_ITEMP	0x33
  #define LTC6802_CMD_STTMPAD_TEST1	0x3E
  #define LTC6802_CMD_STTMPAD_TEST2	0x3F
#define LTC6802_CMD_PLADC		0x40
#define LTC6802_CMD_PLINT		0x50
#define LTC6802_CMD_STCVDC		0x60
  #define LTC6802_CMD_STCVDC_CELL1	0x61
  #define LTC6802_CMD_STCVDC_CELL2	0x62
  #define LTC6802_CMD_STCVDC_CELL3	0x63
  #define LTC6802_CMD_STCVDC_CELL4	0x64
  #define LTC6802_CMD_STCVDC_CELL5	0x65
  #define LTC6802_CMD_STCVDC_CELL6	0x66
  #define LTC6802_CMD_STCVDC_CELL7	0x67
  #define LTC6802_CMD_STCVDC_CELL8	0x68
  #define LTC6802_CMD_STCVDC_CELL9	0x69
  #define LTC6802_CMD_STCVDC_CELL10	0x6A
  #define LTC6802_CMD_STCVDC_CELL11	0x6B /* if CELL10 bit = 0 */
  #define LTC6802_CMD_STCVDC_CELL12	0x6C /* if CELL10 bit = 0 */
  #define LTC6802_CMD_STCVDC_TEST1	0x6E
  #define LTC6802_CMD_STCVDC_TEST2	0x6F
#define LTC6802_CMD_STOWDC		0x70
  #define LTC6802_CMD_STOWDC_CELL1	0x71
  #define LTC6802_CMD_STOWDC_CELL2	0x72
  #define LTC6802_CMD_STOWDC_CELL3	0x73
  #define LTC6802_CMD_STOWDC_CELL4	0x74
  #define LTC6802_CMD_STOWDC_CELL5	0x75
  #define LTC6802_CMD_STOWDC_CELL6	0x76
  #define LTC6802_CMD_STOWDC_CELL7	0x77
  #define LTC6802_CMD_STOWDC_CELL8	0x78
  #define LTC6802_CMD_STOWDC_CELL9	0x79
  #define LTC6802_CMD_STOWDC_CELL10	0x7A
  #define LTC6802_CMD_STOWDC_CELL11	0x7B /* if CELL10 bit = 0 */
  #define LTC6802_CMD_STOWDC_CELL12	0x7C /* if CELL10 bit = 0 */
  #define LTC6802_CMD_STOWDC_TEST1	0x7E
  #define LTC6802_CMD_STOWDC_TEST2	0x7F

/* LTC6802 Configuration Register Group 0 */
#define LTC6802_CFGR0_WDT		BIT(7)
#define LTC6802_CFGR0_GPIO2		BIT(6)
#define LTC6802_CFGR0_GPIO1		BIT(5)
#define LTC6802_CFGR0_LVPL		BIT(4)
#define LTC6802_CFGR0_CELL10		BIT(3)
#define LTC6802_CFGR0_CDC_MODE0		0x000
#define LTC6802_CFGR0_CDC_MODE1		0x001
#define LTC6802_CFGR0_CDC_MODE2		0x010
#define LTC6802_CFGR0_CDC_MODE3		0x011
#define LTC6802_CFGR0_CDC_MODE4		0x100
#define LTC6802_CFGR0_CDC_MODE5		0x101
#define LTC6802_CFGR0_CDC_MODE6		0x110
#define LTC6802_CFGR0_CDC_MODE7		0x111

/* LTC6802 Configuration Register Group 1 */
#define LTC6802_CFGR1_DCC7		BIT(7)
#define LTC6802_CFGR1_DCC6		BIT(6)
#define LTC6802_CFGR1_DCC5		BIT(5)
#define LTC6802_CFGR1_DCC4		BIT(4)
#define LTC6802_CFGR1_DCC3		BIT(3)
#define LTC6802_CFGR1_DCC2		BIT(2)
#define LTC6802_CFGR1_DCC1		BIT(1)
#define LTC6802_CFGR1_DCC0		BIT(0)

/* LTC6802 Configuration Register Group 2 */
#define LTC6802_CFGR2_MC4I		BIT(7)
#define LTC6802_CFGR2_MC3I		BIT(6)
#define LTC6802_CFGR2_MC2I		BIT(5)
#define LTC6802_CFGR2_MC1I		BIT(4)
#define LTC6802_CFGR2_DCC12		BIT(3)
#define LTC6802_CFGR2_DCC11		BIT(2)
#define LTC6802_CFGR2_DCC10		BIT(1)
#define LTC6802_CFGR2_DCC9		BIT(0)

/* LTC6802 Configuration Register Group 3 */
#define LTC6802_CFGR3_MC12I		BIT(7)
#define LTC6802_CFGR3_MC11I		BIT(6)
#define LTC6802_CFGR3_MC10I		BIT(5)
#define LTC6802_CFGR3_MC9I		BIT(4)
#define LTC6802_CFGR3_MC8I		BIT(3)
#define LTC6802_CFGR3_MC7I		BIT(2)
#define LTC6802_CFGR3_MC6I		BIT(1)
#define LTC6802_CFGR3_MC5I		BIT(0)

#define LTC6802_INPUT_DELTA_MV		6144
#define LTC6802_ADC_RESOLUTION_BIT	12

enum ltc6802_register_group {
	LTC6802_CFG,
	LTC6802_CV,
	LTC6802_FLG,
	LTC6802_TMP
};

static ssize_t test = 0;

enum ltc6802_id {
	ltc6802
};

static const struct spi_device_id ltc6802_id[] = {
	{"ltc6802", ltc6802},
	{}
};
MODULE_DEVICE_TABLE(spi, ltc6802_id);

#ifdef CONFIG_OF
static const struct of_device_id ltc6802_adc_dt_ids[] = {
	{ .compatible = "ltc,ltc6802" },
	{},
};
MODULE_DEVICE_TABLE(of, ltc6802_adc_dt_ids);
#endif

#define LTC6802_V_CHAN(index)						\
	{								\
		.type = IIO_VOLTAGE,					\
		.indexed = 1,						\
		.channel = index,					\
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),		\
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_SCALE),	\
	}

#define LTC6802_DV_CHAN(index)						\
	{								\
		.type = IIO_VOLTAGE,					\
		.indexed = 1,						\
		.channel = index,					\
		.channel2 = index - 1,					\
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),		\
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_SCALE),	\
		.differential = 1,					\
	}

#define LTC6802_T_CHAN(index)						\
	{								\
		.type = IIO_TEMP,					\
		.indexed = 1,						\
		.channel = index,					\
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),		\
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_SCALE),	\
	}

static const struct iio_chan_spec ltc6802_channels[] = {
	LTC6802_T_CHAN(0),  /* Int. temp   */
	LTC6802_T_CHAN(1),  /* Ext. temp 1 */
	LTC6802_T_CHAN(2),  /* Ext. temp 2 */
	LTC6802_V_CHAN(0),
	LTC6802_DV_CHAN(1),
	LTC6802_DV_CHAN(2),
	LTC6802_DV_CHAN(3),
	LTC6802_DV_CHAN(4),
	LTC6802_DV_CHAN(5),
	LTC6802_DV_CHAN(6),
	LTC6802_DV_CHAN(7),
	LTC6802_DV_CHAN(8),
	LTC6802_DV_CHAN(9),
	LTC6802_DV_CHAN(10),
	LTC6802_DV_CHAN(11),
};

struct ltc6802_chip_info {
	const struct iio_chan_spec *channels;
	unsigned int num_channels;
};

static const struct ltc6802_chip_info ltc6802_chip_info_tbl[] = {
	[ltc6802] = {
		.channels = ltc6802_channels,
		.num_channels = ARRAY_SIZE(ltc6802_channels),
	},
};

struct ltc6802_state {
	const struct ltc6802_chip_info	*info;
	struct spi_device		*spi;
	__be16				*buffer;
	struct mutex			lock;
	unsigned int			address;
	u8				cfg_reg[6] ____cacheline_aligned;
};

static int ltc6802_read_reg_group(struct iio_dev *indio_dev,
				  int reg, u8 *rx_buf, int rx_buf_size)
{
	int ret;
	u8 tx_buf[2];
	struct ltc6802_state *st = iio_priv(indio_dev);
	struct spi_transfer t[2] = {
		{
		       .tx_buf = tx_buf,
		       .len = ARRAY_SIZE(tx_buf),
		}, {
		       .rx_buf = rx_buf,
		       .len = rx_buf_size,
		},
	};

	tx_buf[0] = 0x80 | st->address;;
	switch(reg) {
	case LTC6802_CFG:
		tx_buf[1] = LTC6802_CMD_RDCFG;
		break;
	case LTC6802_CV:
		tx_buf[1] = LTC6802_CMD_RDCV;
		break;
	case LTC6802_FLG:
		tx_buf[1] = LTC6802_CMD_RDFLG;
		break;
	case LTC6802_TMP:
		tx_buf[1] = LTC6802_CMD_RDTMP;
		break;
	default:
		return -EINVAL;
	}

	ret = spi_sync_transfer(st->spi, t, ARRAY_SIZE(t));
	if (ret < 0) {
		dev_err(&indio_dev->dev,
			"Failed to read register group\n");
	}

	return ret;
}

static int ltc6802_get_chan_value_from_reg_group(int chan, u8 *buf)
{
	int value;
	int index;

	chan++;
	if (chan % 2) {
		index = (chan - 1) + ((chan - 1) / 2);
		value = ((buf[index + 1] & 0x0F) << 8) | buf[index];

	} else {
		index = chan + (chan / 2) - 1;
		value = (buf[index] << 4) | ((buf[index - 1] & 0xF0) >> 4);
	}

	return value;
}

static int ltc6802_read_single_value(struct iio_dev *indio_dev,
				     struct iio_chan_spec const *chan,
				     int *val)
{
	int ret;
	int reg;
	u8 rx_buf[19];
	struct ltc6802_state *st = iio_priv(indio_dev);

	/* Get LTC6802 out of default standby mode */
	st->cfg_reg[0] = LTC6802_CFGR0_CDC_MODE1;
	st->cfg_reg[1] = LTC6802_CMD_WRCFG;
	st->cfg_reg[2] = 0x00;
	st->cfg_reg[3] = 0x00;
	st->cfg_reg[4] = 0x00;
	st->cfg_reg[5] = 0x00;
	st->cfg_reg[6] = 0x00;
	ret = spi_write(st->spi, &st->cfg_reg, 7);
	if (ret < 0) {
		dev_err(&indio_dev->dev,
			"Failed to configure config registers\n");
		return ret;
	}

	switch (chan->type) {
	case IIO_TEMP:
		st->cfg_reg[0] = LTC6802_CMD_STTMPAD_TEST2;
		reg = LTC6802_TMP;
		break;
	case IIO_VOLTAGE:
		st->cfg_reg[0] = LTC6802_CMD_STCVAD_TEST2;
		reg = LTC6802_CV;
		break;
	default:
		return -EINVAL;
	}

	ret = spi_write(st->spi, &st->cfg_reg, 1);
	if (ret < 0) {
		dev_err(&indio_dev->dev,
			"Failed to request A/D conversion start\n");
	}

	mdelay(30); /* Wait for the conversion to be complete */
	
	ret = ltc6802_read_reg_group(indio_dev, reg, rx_buf, ARRAY_SIZE(rx_buf));
	*val = ltc6802_get_chan_value_from_reg_group(chan->channel, rx_buf);

	return IIO_VAL_INT;
}

static int ltc6802_read_raw(struct iio_dev *indio_dev,
			    struct iio_chan_spec const *chan,
			    int *val, int *val2, long mask)
{
	int ret = 0;
	struct ltc6802_state *st = iio_priv(indio_dev);

	mutex_lock(&st->lock);

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		ret = ltc6802_read_single_value(indio_dev, chan, val);
		break;
	case IIO_CHAN_INFO_SCALE:
		switch (chan->type) {
		case IIO_TEMP:
		case IIO_VOLTAGE:
			*val = LTC6802_INPUT_DELTA_MV;
			*val2 = LTC6802_ADC_RESOLUTION_BIT;
			ret = IIO_VAL_FRACTIONAL_LOG2;
			break;
		default:
			ret = -EINVAL;
			break;
		}
		break;
	default:
		ret = -EINVAL;
		break;
	}

	mutex_unlock(&st->lock);

	return ret;
}

static const struct iio_info ltc6802_info = {
	.driver_module = THIS_MODULE,
	.read_raw = &ltc6802_read_raw,
};

static ssize_t digital_io_show(struct device *dev,
                               struct device_attribute *attr, char *buf)
{
	pr_info("%s\n", attr->attr.name);
	return sprintf(buf, "%d\n", test);
}
static ssize_t digital_io_store(struct device *dev,
                                struct device_attribute *attr, const char *buf,
                                size_t count)
{
	pr_info("%s\n", attr->attr.name);
	sscanf(buf, "%d\n", &test);
	return count;
}

static DEVICE_ATTR(cell1_bypass,  S_IWUSR | S_IRUGO, digital_io_show, digital_io_store);
static DEVICE_ATTR(cell2_bypass,  S_IWUSR | S_IRUGO, digital_io_show, digital_io_store);
static DEVICE_ATTR(cell3_bypass,  S_IWUSR | S_IRUGO, digital_io_show, digital_io_store);
static DEVICE_ATTR(cell4_bypass,  S_IWUSR | S_IRUGO, digital_io_show, digital_io_store);
static DEVICE_ATTR(cell5_bypass,  S_IWUSR | S_IRUGO, digital_io_show, digital_io_store);
static DEVICE_ATTR(cell6_bypass,  S_IWUSR | S_IRUGO, digital_io_show, digital_io_store);
static DEVICE_ATTR(cell7_bypass,  S_IWUSR | S_IRUGO, digital_io_show, digital_io_store);
static DEVICE_ATTR(cell8_bypass,  S_IWUSR | S_IRUGO, digital_io_show, digital_io_store);
static DEVICE_ATTR(cell9_bypass,  S_IWUSR | S_IRUGO, digital_io_show, digital_io_store);
static DEVICE_ATTR(cell10_bypass, S_IWUSR | S_IRUGO, digital_io_show, digital_io_store);
static DEVICE_ATTR(cell11_bypass, S_IWUSR | S_IRUGO, digital_io_show, digital_io_store);
static DEVICE_ATTR(cell12_bypass, S_IWUSR | S_IRUGO, digital_io_show, digital_io_store);

static DEVICE_ATTR(gpio1_value, S_IWUSR | S_IRUGO, digital_io_show, digital_io_store);
static DEVICE_ATTR(gpio2_value, S_IWUSR | S_IRUGO, digital_io_show, digital_io_store);

static DEVICE_ATTR(gpio1_direction, S_IWUSR | S_IRUGO, digital_io_show, digital_io_store);
static DEVICE_ATTR(gpio2_direction, S_IWUSR | S_IRUGO, digital_io_show, digital_io_store);

static struct attribute *dev_attrs[] = {
	&dev_attr_cell1_bypass.attr,
	&dev_attr_cell2_bypass.attr,
	&dev_attr_cell3_bypass.attr,
	&dev_attr_cell4_bypass.attr,
	&dev_attr_cell5_bypass.attr,
	&dev_attr_cell6_bypass.attr,
	&dev_attr_cell7_bypass.attr,
	&dev_attr_cell8_bypass.attr,
	&dev_attr_cell9_bypass.attr,
	&dev_attr_cell10_bypass.attr,
	&dev_attr_cell11_bypass.attr,
	&dev_attr_cell12_bypass.attr,
	&dev_attr_gpio1_value.attr,
	&dev_attr_gpio2_value.attr,
	&dev_attr_gpio1_direction.attr,
	&dev_attr_gpio2_direction.attr,
	NULL,
};
ATTRIBUTE_GROUPS(dev);

static int ltc6802_probe(struct spi_device *spi)
{
	int ret;
	const void *ltc6802_addr;
	struct iio_dev *indio_dev;
	struct ltc6802_state *st;

	pr_info("%s: probe(spi = 0x%p)\n", __func__, spi);

	indio_dev = devm_iio_device_alloc(&spi->dev, sizeof(*st));
	if (indio_dev == NULL) {
		pr_err("Can't allocate iio device\n");
		return -ENOMEM;
	}

	spi_set_drvdata(spi, indio_dev);

	st = iio_priv(indio_dev);
	st->spi = spi;
	st->info = &ltc6802_chip_info_tbl[spi_get_device_id(spi)->driver_data];

	ltc6802_addr = of_get_property(st->spi->dev.of_node, "ltc6802,addr", NULL);
	if (!ltc6802_addr) {
		pr_err("Addr field not present in device tree\n");
		return -EINVAL;
	}
	st->address = be32_to_cpup(ltc6802_addr);
	pr_info("Addr: %d\n", st->address);

	mutex_init(&st->lock);

	indio_dev->name = spi_get_device_id(spi)->name;
	indio_dev->dev.parent = &spi->dev;
	indio_dev->info = &ltc6802_info;
	indio_dev->modes = INDIO_DIRECT_MODE;
	indio_dev->channels = st->info->channels;
	indio_dev->num_channels = st->info->num_channels;
	/* indio_dev->groups[0] is already used by &indio_dev->chan_attr_group */
	indio_dev->groups[1] = dev_groups[0];

	st->buffer = devm_kmalloc(&indio_dev->dev,
				  indio_dev->num_channels * 2,
				  GFP_KERNEL);
	if (st->buffer == NULL) {
		dev_err(&indio_dev->dev, "Can't allocate buffer\n");
		return -ENOMEM;
	}

	ret = iio_device_register(indio_dev);
	if (ret < 0) {
		dev_err(&indio_dev->dev, "Failed to register iio device\n");
		return ret;
	}

	return ret;
}

static int ltc6802_remove(struct spi_device *spi)
{
	struct iio_dev *indio_dev = spi_get_drvdata(spi);

	pr_info("%s: remove(spi = 0x%p)\n", __func__, spi);

	iio_device_unregister(indio_dev);

	return 0;
}

static struct spi_driver ltc6802_driver = {
	.driver = {
		.name	= "ltc6802",
		.of_match_table = of_match_ptr(ltc6802_adc_dt_ids),
	},
	.probe		= ltc6802_probe,
	.remove		= ltc6802_remove,
	.id_table	= ltc6802_id,
};
module_spi_driver(ltc6802_driver);

MODULE_AUTHOR("Olivier C. Larocque <olivier.c.larocque@gmail.com>");
MODULE_DESCRIPTION("LTC6802 Battery Stack Monitor");
MODULE_LICENSE("GPL v2");
