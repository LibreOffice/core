/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#define AVMEDIA_GST_0_10

// forward compatibility goodness
#include <gst/interfaces/xoverlay.h>
#define gst_video_overlay_expose(a) gst_x_overlay_expose(GST_X_OVERLAY (a))
#define gst_video_overlay_set_window_handle(a,b) gst_x_overlay_set_xwindow_id( \
                                                    reinterpret_cast<GstXOverlay *>(a), (b) )
#define GST_VIDEO_OVERLAY(a) reinterpret_cast<GstVideoOverlay *>(a)

// #define GstVideoOverlay GstXOverlay

#include "gstmanager.cxx"
#include "gstplayer.cxx"
#include "gstuno.cxx"
#include "gstwindow.cxx"
#include "gstframegrabber.cxx"
