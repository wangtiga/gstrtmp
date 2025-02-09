#include "gst.h"
#include <gst/app/gstappsrc.h>


GMainLoop *gst_rtmp_main_loop = NULL;

void gst_rtmp_start_mainloop(void) {
  gst_rtmp_main_loop = g_main_loop_new(NULL, FALSE);

  g_main_loop_run(gst_rtmp_main_loop);
}

static gboolean gst_rtmp_bus_call(GstBus *bus, GstMessage *msg, gpointer data) {
  switch (GST_MESSAGE_TYPE(msg)) {

  case GST_MESSAGE_EOS:
    g_print("End of stream\n");
    exit(1);
    break;

  case GST_MESSAGE_ERROR: {
    gchar *debug;
    GError *error;

    gst_message_parse_error(msg, &error, &debug);
    g_free(debug);

    g_printerr("Error: %s\n", error->message);
    g_error_free(error);
    exit(1);
    break;
  }
  default:
    break;
  }

  return TRUE;
}

GstElement *gst_rtmp_create_pipeline(char *pipeline) {
  g_printerr("create_pipeline from ws\n");
  gst_init(NULL, NULL);
  GError *error = NULL;
  return gst_parse_launch(pipeline, &error);
}

void gst_rtmp_start_pipeline(GstElement *pipeline) {
  GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));

  gst_bus_add_watch(bus, gst_rtmp_bus_call, NULL);
  
  gst_object_unref(bus);

  gst_element_set_state(pipeline, GST_STATE_PLAYING);
}

void gst_rtmp_stop_pipeline(GstElement *pipeline) { gst_element_set_state(pipeline, GST_STATE_NULL); }

void gst_rtmp_push_buffer(GstElement *pipeline, void *buffer, int mtype ,int len) {
  GstElement *src = gst_bin_get_by_name(GST_BIN(pipeline), "src");
  if (src != NULL) {
    gpointer p = g_memdup(buffer, len);
    GstBuffer *buffer = gst_buffer_new_wrapped(p, len);
    GstFlowReturn ret = gst_app_src_push_buffer(GST_APP_SRC(src), buffer);
    if (GST_FLOW_OK != ret ) {
        g_printerr("gst_app_src_push_buffer Error: %d\n", ret);
    } else {
        g_printerr("gst_rtmp_push_buffer by ws\n");
    }
  }
}
