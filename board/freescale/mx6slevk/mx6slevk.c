/*
 * Copyright (C) 2013-2014 Freescale Semiconductor, Inc.
 *
 * Author: Fabio Estevam <fabio.estevam@freescale.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#include <asm/arch/clock.h>
#include <asm/arch/iomux.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/imx-common/iomux-v3.h>
#include <asm/imx-common/boot_mode.h>
#include <asm/io.h>
#include <asm/sizes.h>
#include <common.h>
#include <fsl_esdhc.h>
#include <mmc.h>
#include <netdev.h>
#if CONFIG_I2C_MXC
#include <i2c.h>
#include <asm/imx-common/mxc_i2c.h>
#endif
#if defined(CONFIG_MXC_EPDC)
#include <lcd.h>
#include <mxc_epdc_fb.h>
#endif
#ifdef CONFIG_FASTBOOT
#include <fastboot.h>
#ifdef CONFIG_ANDROID_RECOVERY
#include <recovery.h>
#endif
#endif /*CONFIG_FASTBOOT*/

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL  (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_22K_UP  | PAD_CTL_SPEED_LOW |		\
	PAD_CTL_DSE_80ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define ENET_PAD_CTRL  (PAD_CTL_PKE | PAD_CTL_PUE |             \
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED   |             \
	PAD_CTL_DSE_40ohm   | PAD_CTL_HYS)

#define SPI_PAD_CTRL (PAD_CTL_HYS |				\
	PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_40ohm | PAD_CTL_SRE_FAST)

#define I2C_PAD_CTRL    (PAD_CTL_PKE | PAD_CTL_PUE |            \
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |               \
	PAD_CTL_DSE_40ohm | PAD_CTL_HYS |			\
	PAD_CTL_ODE | PAD_CTL_SRE_FAST)

#define EPDC_PAD_CTRL    (PAD_CTL_PKE | PAD_CTL_SPEED_MED |	\
	PAD_CTL_DSE_40ohm | PAD_CTL_HYS)

#if CONFIG_I2C_MXC
#define PC MUX_PAD_CTRL(I2C_PAD_CTRL)
/* I2C1 for PMIC */
struct i2c_pads_info i2c_pad_info0 = {
	.scl = {
		.i2c_mode = MX6_PAD_I2C1_SDA__I2C1_SDA | PC,
		.gpio_mode = MX6_PAD_I2C1_SDA__GPIO_3_13 | PC,
		.gp = IMX_GPIO_NR(3, 13),
	},
	.sda = {
		.i2c_mode = MX6_PAD_I2C1_SCL__I2C1_SCL | PC,
		.gpio_mode = MX6_PAD_I2C1_SCL__GPIO_3_12 | PC,
		.gp = IMX_GPIO_NR(3, 12),
	},
};
#endif

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)PHYS_SDRAM, PHYS_SDRAM_SIZE);

	return 0;
}

static iomux_v3_cfg_t const uart1_pads[] = {
	MX6_PAD_UART1_TXD__UART1_TXD | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_UART1_RXD__UART1_RXD | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static iomux_v3_cfg_t const usdhc1_pads[] = {
	/* 8 bit SD */
	MX6_PAD_SD1_CLK__USDHC1_CLK | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_CMD__USDHC1_CMD | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DAT0__USDHC1_DAT0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DAT1__USDHC1_DAT1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DAT2__USDHC1_DAT2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DAT3__USDHC1_DAT3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DAT4__USDHC1_DAT4 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DAT5__USDHC1_DAT5 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DAT6__USDHC1_DAT6 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DAT7__USDHC1_DAT7 | MUX_PAD_CTRL(USDHC_PAD_CTRL),

	/*CD pin*/
	MX6_PAD_KEY_ROW7__GPIO_4_7 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static iomux_v3_cfg_t const usdhc2_pads[] = {
	MX6_PAD_SD2_CLK__USDHC2_CLK | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD2_CMD__USDHC2_CMD | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD2_DAT0__USDHC2_DAT0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD2_DAT1__USDHC2_DAT1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD2_DAT2__USDHC2_DAT2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD2_DAT3__USDHC2_DAT3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),

	/*CD pin*/
	MX6_PAD_SD2_DAT7__GPIO_5_0 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static iomux_v3_cfg_t const usdhc3_pads[] = {
	MX6_PAD_SD3_CLK__USDHC3_CLK | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_CMD__USDHC3_CMD | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT0__USDHC3_DAT0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT1__USDHC3_DAT1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT2__USDHC3_DAT2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT3__USDHC3_DAT3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),

	/*CD pin*/
	MX6_PAD_REF_CLK_32K__GPIO_3_22 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static iomux_v3_cfg_t const fec_pads[] = {
	MX6_PAD_FEC_MDC__FEC_MDC | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_FEC_MDIO__FEC_MDIO | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_FEC_CRS_DV__FEC_RX_DV | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_FEC_RXD0__FEC_RX_DATA0 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_FEC_RXD1__FEC_RX_DATA1 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_FEC_TX_EN__FEC_TX_EN | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_FEC_TXD0__FEC_TX_DATA0 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_FEC_TXD1__FEC_TX_DATA1 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_FEC_REF_CLK__FEC_REF_OUT | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_FEC_RX_ER__GPIO_4_19 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_FEC_TX_CLK__GPIO_4_21 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static iomux_v3_cfg_t const epdc_enable_pads[] = {
	MX6_PAD_EPDC_D0__EPDC_SDDO_0	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EPDC_D1__EPDC_SDDO_1	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EPDC_D2__EPDC_SDDO_2	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EPDC_D3__EPDC_SDDO_3	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EPDC_D4__EPDC_SDDO_4	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EPDC_D5__EPDC_SDDO_5	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EPDC_D6__EPDC_SDDO_6	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EPDC_D7__EPDC_SDDO_7	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EPDC_GDCLK__EPDC_GDCLK	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EPDC_GDSP__EPDC_GDSP	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EPDC_GDOE__EPDC_GDOE	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EPDC_GDRL__EPDC_GDRL	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EPDC_SDCLK__EPDC_SDCLK	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EPDC_SDOE__EPDC_SDOE	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EPDC_SDLE__EPDC_SDLE	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EPDC_SDSHR__EPDC_SDSHR	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EPDC_BDR0__EPDC_BDR_0	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EPDC_SDCE0__EPDC_SDCE_0	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EPDC_SDCE1__EPDC_SDCE_1	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EPDC_SDCE2__EPDC_SDCE_2	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
};

static iomux_v3_cfg_t const epdc_disable_pads[] = {
	MX6_PAD_EPDC_D0__GPIO_1_7,
	MX6_PAD_EPDC_D1__GPIO_1_8,
	MX6_PAD_EPDC_D2__GPIO_1_9,
	MX6_PAD_EPDC_D3__GPIO_1_10,
	MX6_PAD_EPDC_D4__GPIO_1_11,
	MX6_PAD_EPDC_D5__GPIO_1_12,
	MX6_PAD_EPDC_D6__GPIO_1_13,
	MX6_PAD_EPDC_D7__GPIO_1_14,
	MX6_PAD_EPDC_GDCLK__GPIO_1_31,
	MX6_PAD_EPDC_GDSP__GPIO_2_2,
	MX6_PAD_EPDC_GDOE__GPIO_2_0,
	MX6_PAD_EPDC_GDRL__GPIO_2_1,
	MX6_PAD_EPDC_SDCLK__GPIO_1_23,
	MX6_PAD_EPDC_SDOE__GPIO_1_25,
	MX6_PAD_EPDC_SDLE__GPIO_1_24,
	MX6_PAD_EPDC_SDSHR__GPIO_1_26,
	MX6_PAD_EPDC_BDR0__GPIO_2_5,
	MX6_PAD_EPDC_SDCE0__GPIO_1_27,
	MX6_PAD_EPDC_SDCE1__GPIO_1_28,
	MX6_PAD_EPDC_SDCE2__GPIO_1_29,
};

static void setup_iomux_uart(void)
{
	imx_iomux_v3_setup_multiple_pads(uart1_pads, ARRAY_SIZE(uart1_pads));
}

static void setup_iomux_fec(void)
{
	imx_iomux_v3_setup_multiple_pads(fec_pads, ARRAY_SIZE(fec_pads));

	/* Reset LAN8720 PHY */
	gpio_direction_output(IMX_GPIO_NR(4, 21) , 0);
	udelay(1000);
	gpio_set_value(IMX_GPIO_NR(4, 21), 1);
}

#ifdef CONFIG_SYS_USE_SPINOR
iomux_v3_cfg_t const ecspi1_pads[] = {
	MX6_PAD_ECSPI1_SCLK__ECSPI1_SCLK | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_ECSPI1_MISO__ECSPI1_MISO | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_ECSPI1_MOSI__ECSPI1_MOSI | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_ECSPI1_SS0__GPIO_4_11    | MUX_PAD_CTRL(NO_PAD_CTRL),
};

void setup_spinor(void)
{
	imx_iomux_v3_setup_multiple_pads(ecspi1_pads,
					 ARRAY_SIZE(ecspi1_pads));
	gpio_direction_output(IMX_GPIO_NR(4, 11), 0);
}
#endif

#ifdef CONFIG_FSL_ESDHC

#define USDHC1_CD_GPIO	IMX_GPIO_NR(4, 7)
#define USDHC2_CD_GPIO	IMX_GPIO_NR(5, 0)
#define USDHC3_CD_GPIO	IMX_GPIO_NR(3, 22)

static struct fsl_esdhc_cfg usdhc_cfg[3] = {
	{USDHC1_BASE_ADDR},
	{USDHC2_BASE_ADDR, 0, 4},
	{USDHC3_BASE_ADDR, 0, 4},
};

int mmc_get_env_devno(void)
{
	u32 soc_sbmr = readl(SRC_BASE_ADDR + 0x4);
	u32 dev_no;

	/* BOOT_CFG2[3] and BOOT_CFG2[4] */
	dev_no = (soc_sbmr & 0x00001800) >> 11;

	return dev_no;
}

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	switch (cfg->esdhc_base) {
	case USDHC1_BASE_ADDR:
		ret = !gpio_get_value(USDHC1_CD_GPIO);
		break;
	case USDHC2_BASE_ADDR:
		ret = !gpio_get_value(USDHC2_CD_GPIO);
		break;
	case USDHC3_BASE_ADDR:
		ret = !gpio_get_value(USDHC3_CD_GPIO);
		break;
	}

	return ret;
}

int board_mmc_init(bd_t *bis)
{
	int i;

	/*
	 * According to the board_mmc_init() the following map is done:
	 * (U-boot device node)    (Physical Port)
	 * mmc0                    USDHC1
	 * mmc1                    USDHC2
	 * mmc2                    USDHC3
	 */
	for (i = 0; i < CONFIG_SYS_FSL_USDHC_NUM; i++) {
		switch (i) {
		case 0:
			imx_iomux_v3_setup_multiple_pads(
				usdhc1_pads, ARRAY_SIZE(usdhc1_pads));
			gpio_direction_input(USDHC1_CD_GPIO);
			usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
			break;
		case 1:
			imx_iomux_v3_setup_multiple_pads(
				usdhc2_pads, ARRAY_SIZE(usdhc2_pads));
			gpio_direction_input(USDHC2_CD_GPIO);
			usdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
			break;
		case 2:
			imx_iomux_v3_setup_multiple_pads(
				usdhc3_pads, ARRAY_SIZE(usdhc3_pads));
			gpio_direction_input(USDHC3_CD_GPIO);
			usdhc_cfg[2].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
			break;
		default:
			printf("Warning: you configured more USDHC controllers"
				"(%d) than supported by the board\n", i + 1);
			return 0;
			}

			if (fsl_esdhc_initialize(bis, &usdhc_cfg[i]))
				printf("Warning: failed to initialize mmc dev %d\n", i);
	}

	return 0;
}

void board_late_mmc_env_init(void)
{
	char cmd[32];
	u32 dev_no = mmc_get_env_devno();

	setenv_ulong("mmcdev", dev_no);

	sprintf(cmd, "mmc dev %d", dev_no);
	run_command(cmd, 0);
}
#endif

#ifdef CONFIG_MXC_EPDC
#ifdef CONFIG_SPLASH_SCREEN
extern int mmc_get_env_devno(void);
int setup_splash_img(void)
{
#ifdef CONFIG_SPLASH_IS_IN_MMC
	int mmc_dev = mmc_get_env_devno();
	ulong offset = CONFIG_SPLASH_IMG_OFFSET;
	ulong size = CONFIG_SPLASH_IMG_SIZE;
	ulong addr = 0;
	char *s = NULL;
	struct mmc *mmc = find_mmc_device(mmc_dev);
	uint blk_start, blk_cnt, n;

	s = getenv("splashimage");

	if (NULL == s) {
		puts("env splashimage not found!\n");
		return -1;
	}
	addr = simple_strtoul(s, NULL, 16);

	if (!mmc) {
		printf("MMC Device %d not found\n", mmc_dev);
		return -1;
	}

	if (mmc_init(mmc)) {
		puts("MMC init failed\n");
		return -1;
	}

	blk_start = ALIGN(offset, mmc->read_bl_len) / mmc->read_bl_len;
	blk_cnt = ALIGN(size, mmc->read_bl_len) / mmc->read_bl_len;
	n = mmc->block_dev.block_read(mmc_dev, blk_start,
				      blk_cnt, (u_char *)addr);
	flush_cache((ulong)addr, blk_cnt * mmc->read_bl_len);

	return (n == blk_cnt) ? 0 : -1;
#endif

	return 0;
}
#endif

vidinfo_t panel_info = {
	.vl_refresh = 85,
	.vl_col = 800,
	.vl_row = 600,
	.vl_pixclock = 26666667,
	.vl_left_margin = 8,
	.vl_right_margin = 100,
	.vl_upper_margin = 4,
	.vl_lower_margin = 8,
	.vl_hsync = 4,
	.vl_vsync = 1,
	.vl_sync = 0,
	.vl_mode = 0,
	.vl_flag = 0,
	.vl_bpix = 3,
	.cmap = 0,
};

struct epdc_timing_params panel_timings = {
	.vscan_holdoff = 4,
	.sdoed_width = 10,
	.sdoed_delay = 20,
	.sdoez_width = 10,
	.sdoez_delay = 20,
	.gdclk_hp_offs = 419,
	.gdsp_offs = 20,
	.gdoe_offs = 0,
	.gdclk_offs = 5,
	.num_ce = 1,
};

static void setup_epdc_power(void)
{
	/* Setup epdc voltage */

	/* EPDC_PWRSTAT - GPIO2[13] for PWR_GOOD status */
	imx_iomux_v3_setup_pad(MX6_PAD_EPDC_PWRSTAT__GPIO_2_13 |
				MUX_PAD_CTRL(EPDC_PAD_CTRL));
	gpio_direction_input(IMX_GPIO_NR(2, 13));

	/* EPDC_VCOM0 - GPIO2[3] for VCOM control */
	imx_iomux_v3_setup_pad(MX6_PAD_EPDC_VCOM0__GPIO_2_3 |
				MUX_PAD_CTRL(EPDC_PAD_CTRL));

	/* Set as output */
	gpio_direction_output(IMX_GPIO_NR(2, 3), 1);

	/* EPDC_PWRWAKEUP - GPIO2[14] for EPD PMIC WAKEUP */
	imx_iomux_v3_setup_pad(MX6_PAD_EPDC_PWRWAKEUP__GPIO_2_14 |
				MUX_PAD_CTRL(EPDC_PAD_CTRL));
	/* Set as output */
	gpio_direction_output(IMX_GPIO_NR(2, 14), 1);

	/* EPDC_PWRCTRL0 - GPIO2[7] for EPD PWR CTL0 */
	imx_iomux_v3_setup_pad(MX6_PAD_EPDC_PWRCTRL0__GPIO_2_7 |
				MUX_PAD_CTRL(EPDC_PAD_CTRL));
	/* Set as output */
	gpio_direction_output(IMX_GPIO_NR(2, 7), 1);
}

int setup_waveform_file(void)
{
#ifdef CONFIG_WAVEFORM_FILE_IN_MMC
	int mmc_dev = mmc_get_env_devno();
	ulong offset = CONFIG_WAVEFORM_FILE_OFFSET;
	ulong size = CONFIG_WAVEFORM_FILE_SIZE;
	ulong addr = CONFIG_WAVEFORM_BUF_ADDR;
	struct mmc *mmc = find_mmc_device(mmc_dev);
	uint blk_start, blk_cnt, n;

	if (!mmc) {
		printf("MMC Device %d not found\n", mmc_dev);
		return -1;
	}

	if (mmc_init(mmc)) {
		puts("MMC init failed\n");
		return -1;
	}

	blk_start = ALIGN(offset, mmc->read_bl_len) / mmc->read_bl_len;
	blk_cnt = ALIGN(size, mmc->read_bl_len) / mmc->read_bl_len;
	n = mmc->block_dev.block_read(mmc_dev, blk_start,
				      blk_cnt, (u_char *)addr);
	flush_cache((ulong)addr, blk_cnt * mmc->read_bl_len);

	return (n == blk_cnt) ? 0 : -1;
#else
	return -1;
#endif
}

static void epdc_enable_pins(void)
{
	/* epdc iomux settings */
	imx_iomux_v3_setup_multiple_pads(epdc_enable_pads,
				ARRAY_SIZE(epdc_enable_pads));
}

static void epdc_disable_pins(void)
{
	/* Configure MUX settings for EPDC pins to GPIO  and drive to 0 */
	imx_iomux_v3_setup_multiple_pads(epdc_disable_pads,
				ARRAY_SIZE(epdc_disable_pads));
}

static void setup_epdc(void)
{
	unsigned int reg;

	/*** epdc Maxim PMIC settings ***/

	/* EPDC PWRSTAT - GPIO2[13] for PWR_GOOD status */
	imx_iomux_v3_setup_pad(MX6_PAD_EPDC_PWRSTAT__GPIO_2_13 |
				MUX_PAD_CTRL(EPDC_PAD_CTRL));

	/* EPDC VCOM0 - GPIO2[3] for VCOM control */
	imx_iomux_v3_setup_pad(MX6_PAD_EPDC_VCOM0__GPIO_2_3 |
				MUX_PAD_CTRL(EPDC_PAD_CTRL));

	/* UART4 TXD - GPIO2[14] for EPD PMIC WAKEUP */
	imx_iomux_v3_setup_pad(MX6_PAD_EPDC_PWRWAKEUP__GPIO_2_14 |
				MUX_PAD_CTRL(EPDC_PAD_CTRL));

	/* EIM_A18 - GPIO2[7] for EPD PWR CTL0 */
	imx_iomux_v3_setup_pad(MX6_PAD_EPDC_PWRCTRL0__GPIO_2_7 |
				MUX_PAD_CTRL(EPDC_PAD_CTRL));

	/*** Set pixel clock rates for EPDC ***/

	/* EPDC AXI clk from PFD_400M, set to 396/2 = 198MHz */
	reg = readl(CCM_BASE_ADDR + CLKCTL_CHSCCDR);
	reg &= ~0x3F000;
	reg |= (0x4 << 15) | (1 << 12);
	writel(reg, CCM_BASE_ADDR + CLKCTL_CHSCCDR);

	/* EPDC AXI clk enable */
	reg = readl(CCM_BASE_ADDR + CLKCTL_CCGR3);
	reg |= 0x0030;
	writel(reg, CCM_BASE_ADDR + CLKCTL_CCGR3);

	/* EPDC PIX clk from PFD_540M, set to 540/4/5 = 27MHz */
	reg = readl(CCM_BASE_ADDR + CLKCTL_CSCDR2);
	reg &= ~0x03F000;
	reg |= (0x5 << 15) | (4 << 12);
	writel(reg, CCM_BASE_ADDR + CLKCTL_CSCDR2);

	reg = readl(CCM_BASE_ADDR + CLKCTL_CBCMR);
	reg &= ~0x03800000;
	reg |= (0x3 << 23);
	writel(reg, CCM_BASE_ADDR + CLKCTL_CBCMR);

	/* EPDC PIX clk enable */
	reg = readl(CCM_BASE_ADDR + CLKCTL_CCGR3);
	reg |= 0x0C00;
	writel(reg, CCM_BASE_ADDR + CLKCTL_CCGR3);

	panel_info.epdc_data.working_buf_addr = CONFIG_WORKING_BUF_ADDR;
	panel_info.epdc_data.waveform_buf_addr = CONFIG_WAVEFORM_BUF_ADDR;

	panel_info.epdc_data.wv_modes.mode_init = 0;
	panel_info.epdc_data.wv_modes.mode_du = 1;
	panel_info.epdc_data.wv_modes.mode_gc4 = 3;
	panel_info.epdc_data.wv_modes.mode_gc8 = 2;
	panel_info.epdc_data.wv_modes.mode_gc16 = 2;
	panel_info.epdc_data.wv_modes.mode_gc32 = 2;

	panel_info.epdc_data.epdc_timings = panel_timings;

	setup_epdc_power();

	/* Assign fb_base */
	gd->fb_base = CONFIG_FB_BASE;
}

void epdc_power_on(void)
{
	unsigned int reg;

	/* Set EPD_PWR_CTL0 to high - enable EINK_VDD (3.15) */
	gpio_set_value(IMX_GPIO_NR(2, 7), 1);
	udelay(1000);

	/* Enable epdc signal pin */
	epdc_enable_pins();

	/* Set PMIC Wakeup to high - enable Display power */
	gpio_set_value(IMX_GPIO_NR(2, 14), 1);

	/* Wait for PWRGOOD == 1 */
	while (1) {
		reg = readl(GPIO2_BASE_ADDR + GPIO_PSR);
		if (!(reg & (1 << 13)))
			break;

		udelay(100);
	}

	/* Enable VCOM */
	gpio_set_value(IMX_GPIO_NR(2, 3), 1);

	udelay(500);
}

void epdc_power_off(void)
{
	/* Set PMIC Wakeup to low - disable Display power */
	gpio_set_value(IMX_GPIO_NR(2, 14), 0);

	/* Disable VCOM */
	gpio_set_value(IMX_GPIO_NR(2, 3), 0);

	epdc_disable_pins();

	/* Set EPD_PWR_CTL0 to low - disable EINK_VDD (3.15) */
	gpio_set_value(IMX_GPIO_NR(2, 7), 0);
}
#endif

#ifdef CONFIG_FEC_MXC
int board_eth_init(bd_t *bis)
{
	int ret;

	setup_iomux_fec();

	ret = cpu_eth_init(bis);
	if (ret)
		printf("FEC MXC: %s:failed\n", __func__);

	return 0;
}

static int setup_fec(void)
{
	struct iomuxc_base_regs *const iomuxc_regs
		= (struct iomuxc_base_regs *) IOMUXC_BASE_ADDR;
	int ret;

	/* clear gpr1[14], gpr1[18:17] to select anatop clock */
	clrsetbits_le32(&iomuxc_regs->gpr[1], IOMUX_GPR1_FEC_MASK, 0);

	ret = enable_fec_clock();
	if (ret)
		return ret;

	return 0;
}
#endif

#ifdef CONFIG_I2C_MXC
static int setup_pmic_voltages(void)
{
	unsigned char value, rev_id = 0;

	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
	if (!i2c_probe(0x8)) {
		if (i2c_read(0x8, 0, 1, &value, 1)) {
			printf("Read device ID error!\n");
			return -1;
		}
		if (i2c_read(0x8, 3, 1, &rev_id, 1)) {
			printf("Read Rev ID error!\n");
			return -1;
		}
		printf("Found PFUZE100! deviceid=%x,revid=%x\n", value, rev_id);

		/* set SW1AB staby volatage 0.975V */
		if (i2c_read(0x8, 0x21, 1, &value, 1)) {
			printf("Read SW1ABSTBY error!\n");
			return -1;
		}
		value &= ~0x3f;
		value |= 0x1b;
		if (i2c_write(0x8, 0x21, 1, &value, 1)) {
			printf("Set SW1ABSTBY error!\n");
			return -1;
		}

		/* set SW1AB/VDDARM step ramp up time from 16us to 4us/25mV */
		if (i2c_read(0x8, 0x24, 1, &value, 1)) {
			printf("Read SW1ABCONFIG error!\n");
			return -1;
		}
		value &= ~0xc0;
		value |= 0x40;
		if (i2c_write(0x8, 0x24, 1, &value, 1)) {
			printf("Set SW1ABCONFIG error!\n");
			return -1;
		}

		/* set SW1C staby volatage 0.975V */
		if (i2c_read(0x8, 0x2f, 1, &value, 1)) {
			printf("Read SW1CSTBY error!\n");
			return -1;
		}
		value &= ~0x3f;
		value |= 0x1b;
		if (i2c_write(0x8, 0x2f, 1, &value, 1)) {
			printf("Set SW1CSTBY error!\n");
			return -1;
		}

		/* set SW1C/VDDSOC step ramp up time to from 16us to 4us/25mV */
		if (i2c_read(0x8, 0x32, 1, &value, 1)) {
			printf("Read SW1CCONFIG error!\n");
			return -1;
		}
		value &= ~0xc0;
		value |= 0x40;
		if (i2c_write(0x8, 0x32, 1, &value, 1)) {
			printf("Set SW1CCONFIG error!\n");
			return -1;
		}
	}

	return 0;
}

#ifdef CONFIG_LDO_BYPASS_CHECK
void ldo_mode_set(int ldo_bypass)
{
	unsigned char value;
	/* swith to ldo_bypass mode */
	if (ldo_bypass) {
		/* decrease VDDARM to 1.15V */
		if (i2c_read(0x8, 0x20, 1, &value, 1)) {
			printf("Read SW1AB error!\n");
			return;
		}
		value &= ~0x3f;
		value |= 0x22;
		if (i2c_write(0x8, 0x20, 1, &value, 1)) {
			printf("Set SW1AB error!\n");
			return;
		}
		/* increase VDDSOC to 1.15V */
		if (i2c_read(0x8, 0x2e, 1, &value, 1)) {
			printf("Read SW1C error!\n");
			return;
		}
		value &= ~0x3f;
		value |= 0x22;
		if (i2c_write(0x8, 0x2e, 1, &value, 1)) {
			printf("Set SW1C error!\n");
			return;
		}

		set_anatop_bypass();
		printf("switch to ldo_bypass mode!\n");
	}

}
#endif
#endif

int board_early_init_f(void)
{
	setup_iomux_uart();

#ifdef CONFIG_SYS_USE_SPINOR
	setup_spinor();
#endif

	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

#ifdef	CONFIG_FEC_MXC
	setup_fec();
#endif

#ifdef	CONFIG_MXC_EPDC
	setup_epdc();
#endif
	return 0;
}

int board_late_init(void)
{
	int ret = 0;

#ifdef CONFIG_I2C_MXC
	setup_i2c(0, CONFIG_SYS_I2C_SPEED,
			CONFIG_SYS_I2C_SLAVE, &i2c_pad_info0);
	ret = setup_pmic_voltages();
	if (ret)
		return -1;
#endif

#ifdef CONFIG_ENV_IS_IN_MMC
	board_late_mmc_env_init();
#endif

	return 0;
}

u32 get_board_rev(void)
{
	return get_cpu_rev();
}

#ifdef CONFIG_FASTBOOT

void board_fastboot_setup(void)
{
	switch (get_boot_device()) {
#if defined(CONFIG_FASTBOOT_STORAGE_MMC)
	case SD1_BOOT:
	case MMC1_BOOT:
		if (!getenv("fastboot_dev"))
			setenv("fastboot_dev", "mmc0");
		if (!getenv("bootcmd"))
			setenv("bootcmd", "booti mmc0");
		break;
	case SD2_BOOT:
	case MMC2_BOOT:
		if (!getenv("fastboot_dev"))
			setenv("fastboot_dev", "mmc1");
		if (!getenv("bootcmd"))
			setenv("bootcmd", "booti mmc1");
		break;
	case SD3_BOOT:
	case MMC3_BOOT:
		if (!getenv("fastboot_dev"))
			setenv("fastboot_dev", "mmc2");
		if (!getenv("bootcmd"))
			setenv("bootcmd", "booti mmc2");
		break;
#endif /*CONFIG_FASTBOOT_STORAGE_MMC*/
	default:
		printf("unsupported boot devices\n");
		break;
	}

}

#ifdef CONFIG_ANDROID_RECOVERY
int check_recovery_cmd_file(void)
{
    return recovery_check_and_clean_flag();
}

void board_recovery_setup(void)
{
	int bootdev = get_boot_device();

	/*current uboot BSP only supports USDHC2*/
	switch (bootdev) {
#if defined(CONFIG_FASTBOOT_STORAGE_MMC)
	case SD1_BOOT:
	case MMC1_BOOT:
		if (!getenv("bootcmd_android_recovery"))
			setenv("bootcmd_android_recovery",
					"booti mmc0 recovery");
		break;
	case SD2_BOOT:
	case MMC2_BOOT:
		if (!getenv("bootcmd_android_recovery"))
			setenv("bootcmd_android_recovery",
					"booti mmc1 recovery");
		break;
	case SD3_BOOT:
	case MMC3_BOOT:
		if (!getenv("bootcmd_android_recovery"))
			setenv("bootcmd_android_recovery",
					"booti mmc2 recovery");
		break;
#endif /*CONFIG_FASTBOOT_STORAGE_MMC*/
	default:
		printf("Unsupported bootup device for recovery: dev: %d\n",
			bootdev);
		return;
	}

	printf("setup env for recovery..\n");
	setenv("bootcmd", "run bootcmd_android_recovery");
}

#endif /*CONFIG_ANDROID_RECOVERY*/

#endif /*CONFIG_FASTBOOT*/

int checkboard(void)
{
	puts("Board: MX6SLEVK\n");

	return 0;
}

#ifdef CONFIG_MXC_KPD
#define MX6SL_KEYPAD_CTRL	(PAD_CTL_HYS | PAD_CTL_PKE | PAD_CTL_PUE | \
				PAD_CTL_PUS_100K_UP | PAD_CTL_DSE_120ohm)

iomux_v3_cfg_t const mxc_kpd_pads[] = {
	(MX6_PAD_KEY_COL0__KPP_COL_0 | MUX_PAD_CTRL(NO_PAD_CTRL)),
	(MX6_PAD_KEY_COL1__KPP_COL_1 | MUX_PAD_CTRL(NO_PAD_CTRL)),
	(MX6_PAD_KEY_COL2__KPP_COL_2 | MUX_PAD_CTRL(NO_PAD_CTRL)),
	(MX6_PAD_KEY_COL3__KPP_COL_3 | MUX_PAD_CTRL(NO_PAD_CTRL)),

	(MX6_PAD_KEY_ROW0__KPP_ROW_0 | MUX_PAD_CTRL(MX6SL_KEYPAD_CTRL)),
	(MX6_PAD_KEY_ROW1__KPP_ROW_1 | MUX_PAD_CTRL(MX6SL_KEYPAD_CTRL)),
	(MX6_PAD_KEY_ROW2__KPP_ROW_2 | MUX_PAD_CTRL(MX6SL_KEYPAD_CTRL)),
	(MX6_PAD_KEY_ROW3__KPP_ROW_3 | MUX_PAD_CTRL(MX6SL_KEYPAD_CTRL)),
};
int setup_mxc_kpd(void)
{
	imx_iomux_v3_setup_multiple_pads(mxc_kpd_pads,
			ARRAY_SIZE(mxc_kpd_pads));

	return 0;
}
#endif /*CONFIG_MXC_KPD*/

#ifdef CONFIG_IMX_UDC
iomux_v3_cfg_t const otg_udc_pads[] = {
	(MX6_PAD_EPDC_PWRCOM__ANATOP_USBOTG1_ID | MUX_PAD_CTRL(NO_PAD_CTRL)),
};
void udc_pins_setting(void)
{
	imx_iomux_v3_setup_multiple_pads(otg_udc_pads,
			ARRAY_SIZE(otg_udc_pads));
}
#endif /*CONFIG_IMX_UDC*/

#ifdef CONFIG_USB_EHCI_MX6
iomux_v3_cfg_t const usb_otg1_pads[] = {
	MX6_PAD_KEY_COL4__USB_USBOTG1_PWR | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_EPDC_PWRCOM__ANATOP_USBOTG1_ID | MUX_PAD_CTRL(NO_PAD_CTRL)
};

iomux_v3_cfg_t const usb_otg2_pads[] = {
	MX6_PAD_KEY_COL5__USB_USBOTG2_PWR | MUX_PAD_CTRL(NO_PAD_CTRL),
};

int board_ehci_hcd_init(int port)
{
	switch (port) {
	case 0:
		imx_iomux_v3_setup_multiple_pads(usb_otg1_pads,
			ARRAY_SIZE(usb_otg1_pads));
		break;
	case 1:
		imx_iomux_v3_setup_multiple_pads(usb_otg2_pads,
			ARRAY_SIZE(usb_otg2_pads));
		break;
	default:
		printf("MXC USB port %d not yet supported\n", port);
		return 1;
	}
	return 0;
}
#endif
