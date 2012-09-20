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

#define Button MacOSButton
#define Byte MacOSByte
#define Control MacOSControl
#define Cursor MacOSCursor
#define DirInfo MacOSDirInfo
#define FontInfo MacOSFontInfo
#define LSize MacOSLSize
#define Line MacOSLine
#define MemoryBlock MacOSMemoryBlock
#define ModalDialog MacOSModalDialog
#define Palette MacOSPalette
#define Pattern MacOSPattern
#define Point MacOSPoint
#define Polygon MacOSPolygon
#define Ptr MacOSPtr
#define Region MacOSRegion
#define SetCursor MacOSSetCursor
#define Size MacOSSize
#define StringPtr MacOSStringPtr
#define TimeValue MacOSTimeValue

#ifdef __OBJC__
#import <Foundation/NSObjCRuntime.h>
#if defined (NSFoundationVersionNumber10_5) &&  MAC_OS_X_VERSION_MAX_ALLOWED < 1050
@class CALayer;
@class NSViewController;
typedef int NSColorRenderingIntent;
#endif
#endif

#if MACOSX_SDK_VERSION < 1050
typedef float CGFloat;
typedef int NSInteger;
typedef unsigned NSUInteger;
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
