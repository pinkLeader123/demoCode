#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/timer.h>

static unsigned int blink_period = 1000; 

struct p8_08_led_data {
    struct gpio_desc *gpiod;
    struct timer_list timer;
    int state;
};

static void led_blink(struct timer_list *t)
{
    struct p8_08_led_data *data = from_timer(data, t, timer);
    data->state = !data->state;
    gpiod_set_value(data->gpiod, data->state);
    mod_timer(&data->timer, jiffies + blink_period); 
}

static int p8_08_led_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct p8_08_led_data *data;
    data = devm_kzalloc(dev, sizeof(*data), GFP_KERNEL);
    if (!data){
        return -ENOMEM;
    }
    data->gpiod = devm_gpiod_get(dev, NULL, GPIOD_OUT_LOW);
    if (IS_ERR(data->gpiod)){
        return PTR_ERR(data->gpiod);
    }
    gpiod_set_consumer_name(data->gpiod, "P8_08_BLINK");
    timer_setup(&data->timer, led_blink, 0);
    data->state = 0;
    mod_timer(&data->timer, jiffies + blink_period);
    platform_set_drvdata(pdev, data);
    return 0;
}

static int p8_08_led_remove(struct platform_device *pdev)
{
    struct p8_08_led_data *data = platform_get_drvdata(pdev);
    del_timer_sync(&data->timer);
    gpiod_set_value(data->gpiod, 0);
    return 0;
}

static const struct of_device_id p8_08_led_dt_ids[] = {
    { .compatible = "customled,p8-08-blink" },
    { }
};
MODULE_DEVICE_TABLE(of, p8_08_led_dt_ids);

static struct platform_driver p8_08_led_driver = {
    .probe  = p8_08_led_probe,
    .remove = p8_08_led_remove,
    .driver = {
        .name = "p8-08-blink-driver",
        .of_match_table = p8_08_led_dt_ids,
    },
};

module_platform_driver(p8_08_led_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dang Van Phuc");
MODULE_DESCRIPTION("blink led su dung device tree");
