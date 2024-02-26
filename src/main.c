#include <stdint.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/display.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/display/cfb.h>

#include "logo_image.h"

#define DISPLAY_BUFFER_PITCH 128

#define LED0_NODE DT_ALIAS(led0)

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

LOG_MODULE_REGISTER(display);

static const struct device *display = DEVICE_DT_GET(DT_NODELABEL(ssd1306));

const struct device *uart = DEVICE_DT_GET(DT_NODELABEL(uart0));

void main(void)
{
  if (!device_is_ready(uart))
  {
    return;
  }

  printf("Hello World! %s\n", CONFIG_BOARD);
  gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
  gpio_pin_set_dt(&led0, 0);

  if (display == NULL)
  {
    LOG_ERR("device pointer is NULL");
    return;
  }

  if (!device_is_ready(display))
  {
    LOG_ERR("display device is not ready,%d, %d", display, display->name);
    while (true)
    {
      gpio_pin_toggle_dt(&led0);
      k_sleep(K_SECONDS(1));
    }

    return;
  }

  struct display_capabilities capabilities;
  display_get_capabilities(display, &capabilities);

  const uint16_t x_res = capabilities.x_resolution;
  const uint16_t y_res = capabilities.y_resolution;

  LOG_INF("x_resolution: %d", x_res);
  LOG_INF("y_resolution: %d", y_res);
  LOG_INF("supported pixel formats: %d", capabilities.supported_pixel_formats);
  LOG_INF("screen_info: %d", capabilities.screen_info);
  LOG_INF("current_pixel_format: %d", capabilities.current_pixel_format);
  LOG_INF("current_orientation: %d", capabilities.current_orientation);

  const struct display_buffer_descriptor buf_desc = {
      .width = x_res,
      .height = y_res,
      .buf_size = x_res * y_res,
      .pitch = DISPLAY_BUFFER_PITCH};

  cfb_framebuffer_clear(display, true);
  if (cfb_framebuffer_init(display) != 0)
  {
    LOG_ERR("Framebuffer initialization failed!");
    return;
  }

  if (display_set_contrast(display, 0) != 0)
  {
    LOG_ERR("could not set display contrast");
  }
  size_t ms_sleep = 5;

  gpio_pin_toggle_dt(&led0);

  while (true)
  {

    gpio_pin_toggle_dt(&led0);

    k_sleep(K_SECONDS(2));

    if (display_write(display, 0, 0, &buf_desc, buf) != 0)
    {
      LOG_ERR("could not write to display");
    }
    manipulate_display_contrast(display, ms_sleep);

    k_sleep(K_SECONDS(2));

    if (cfb_print(display, "Hello World!", 0, 0) != 0)
    {
      LOG_ERR("Failed to print a string on the screen!");
      return;
    }

    if (cfb_framebuffer_finalize(display) != 0)
    {
      LOG_ERR("Framebuffer finalization failed!");
      return;
    }

    manipulate_display_contrast(display, ms_sleep);
  }
}

void manipulate_display_contrast(const struct device *display, size_t ms_sleep)
{
  for (size_t i = 0; i < 255; i++)
  {
    display_set_contrast(display, i);
    k_sleep(K_MSEC(ms_sleep));
  }

  for (size_t i = 255; i > 0; i--)
  {
    display_set_contrast(display, i);
    k_sleep(K_MSEC(ms_sleep));
  }
}