/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _QUICKTIMECOMMON_HXX
#define _QUICKTIMECOMMON_HXX

#ifdef MACOSX
#include <premac.h>
#import <Cocoa/Cocoa.h>
#import <QTKit/QTKit.h>
#import <QuickTime/QuickTime.h>
#include <postmac.h>
#endif
#include <osl/mutex.hxx>
#ifndef __RTL_USTRING_
#include <rtl/ustring.hxx>
#endif
#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <tools/string.hxx>
#include <tools/urlobj.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
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

#define AVMEDIA_QUICKTIME_MANAGER_IMPLEMENTATIONNAME "com.sun.star.comp.avmedia.Manager_QuickTime"
#define AVMEDIA_QUICKTIME_MANAGER_SERVICENAME "com.sun.star.media.Manager_QuickTime"

#define AVMEDIA_QUICKTIME_PLAYER_IMPLEMENTATIONNAME "com.sun.star.comp.avmedia.Player_QuickTime"
#define AVMEDIA_QUICKTIME_PLAYER_SERVICENAME "com.sun.star.media.Player_QuickTime"

#define AVMEDIA_QUICKTIME_WINDOW_IMPLEMENTATIONNAME "com.sun.star.comp.avmedia.Window_QuickTime"
#define AVMEDIA_QUICKTIME_WINDOW_SERVICENAME "com.sun.star.media.Window_QuickTime"

#define WM_GRAPHNOTIFY (WM_USER + 567)

// Quicktime 7+ in Mac OS X 10.4
#define QT701 0x07010000

// Quicktime 6.4+ in Mac OS X 10.3
#define QT64 0x06400000


#endif // _QUICKTIMECOMMOM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
