/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _GSTCOMMON_HXX
#define _GSTCOMMON_HXX

#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>
#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <tools/string.hxx>
#include <tools/urlobj.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/factory.hxx>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <com/sun/star/awt/MouseButton.hpp>
#include <com/sun/star/media/XManager.hpp>

#include <memory>

#define AVMEDIA_GSTREAMER_MANAGER_IMPLEMENTATIONNAME "com.sun.star.comp.avmedia.Manager_GStreamer"
#define AVMEDIA_GSTREAMER_MANAGER_SERVICENAME "com.sun.star.media.Manager_GStreamer"

#define AVMEDIA_GSTREAMER_PLAYER_IMPLEMENTATIONNAME "com.sun.star.comp.avmedia.Player_GStreamer"
#define AVMEDIA_GSTREAMER_PLAYER_SERVICENAME "com.sun.star.media.Player_GStreamer"

#define AVMEDIA_GSTREAMER_WINDOW_IMPLEMENTATIONNAME "com.sun.star.comp.avmedia.Window_GStreamer"
#define AVMEDIA_GSTREAMER_WINDOW_SERVICENAME "com.sun.star.media.Window_GStreamer"

#define AVMEDIA_GSTREAMER_FRAMEGRABBER_IMPLEMENTATIONNAME "com.sun.star.comp.avmedia.FrameGrabber_GStreamer"
#define AVMEDIA_GSTREAMER_FRAMEGRABBER_SERVICENAME "com.sun.star.media.FrameGrabber_GStreamer"

#endif // _GSTCOMMOM_HXX
