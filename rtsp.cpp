#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>

#include <iostream>

int main(int argc, char **argv)
{

    GMainLoop *loop;
    GstRTSPServer *server;
    GstRTSPMountPoints *mounts;
    GstRTSPMediaFactory *factory;

    /** Init GStreamer */
    gst_init(&argc, &argv);

    /** create gst main loop */
    loop = g_main_loop_new(NULL, FALSE);

    /** create RTSP server instance */
    server = gst_rtsp_server_new();

    /** create authentication manager */
    GstRTSPAuth *auth = gst_rtsp_auth_new();

    /** Create and configurate authentication token */
    GstRTSPToken *token = gst_rtsp_token_new(GST_RTSP_TOKEN_MEDIA_FACTORY_ROLE, G_TYPE_STRING, "user", NULL);
    gchar *basic = gst_rtsp_auth_make_basic("test", "test1234");
    gst_rtsp_auth_add_basic(auth, basic, token);
    g_free(basic);
    gst_rtsp_token_unref(token);

    /** attach authentication manager to RTSP server */
    gst_rtsp_server_set_auth(server, auth);

 
    /** get the mount of RTSP server */
    mounts = gst_rtsp_server_get_mount_points(server);

    /** create a media factory in order to create video stream */
    factory = gst_rtsp_media_factory_new();
    gst_rtsp_media_factory_set_launch(factory,
                                      "( v4l2src device=/dev/video0 ! video/x-raw,format=NV12,width=640,height=480,framerate=30/1 ! videoconvert !  mpph264enc ! h264parse  ! rtph264pay name=pay0 pt=96 )");
    /** allow user and admin to access this resource */
    gst_rtsp_media_factory_add_role(factory, "user",
                                    GST_RTSP_PERM_MEDIA_FACTORY_ACCESS, G_TYPE_BOOLEAN, TRUE,
                                    GST_RTSP_PERM_MEDIA_FACTORY_CONSTRUCT, G_TYPE_BOOLEAN, TRUE, NULL);

    gst_rtsp_media_factory_set_shared(factory, TRUE);
    /** mount the factory to RTSP path */
    gst_rtsp_mount_points_add_factory(mounts, "/video", factory);

    /** create the second factory */
    GstRTSPMediaFactory *factory2 = gst_rtsp_media_factory_new();
    gst_rtsp_media_factory_set_launch(factory2,
                                      "( v4l2src device=/dev/video1 ! video/x-raw,format=NV12,width=640,height=480,framerate=30/1 ! videoconvert !  mpph264enc ! h264parse  ! rtph264pay name=pay0 pt=96 )");
    
    /** allow user and admin to access this resource */
    gst_rtsp_media_factory_add_role(factory2, "user",
                                    GST_RTSP_PERM_MEDIA_FACTORY_ACCESS, G_TYPE_BOOLEAN, TRUE,
                                    GST_RTSP_PERM_MEDIA_FACTORY_CONSTRUCT, G_TYPE_BOOLEAN, TRUE, NULL);

    gst_rtsp_media_factory_set_shared(factory2, TRUE);
    gst_rtsp_mount_points_add_factory(mounts, "/video1", factory2);

    /** RTSP server is responsibility for cleaning object resources */
    g_object_unref(mounts);

    /* bind the address and port for RTSP server */
    gst_rtsp_server_attach(server, NULL);

    /** start the main loop */
    //g_print("RTSP server started at rtsp://127.0.0.1:8554/test\n");
    g_main_loop_run(loop);

    /** free resources */
    g_main_loop_unref(loop);
    g_object_unref(server);
    g_object_unref(auth);
    g_object_unref(factory);
    g_object_unref(factory2);

    return 0;
}
