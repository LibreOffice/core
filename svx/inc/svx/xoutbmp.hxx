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

#ifndef _XOUTBMP_HXX
#define _XOUTBMP_HXX

#include <vcl/graph.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include "svx/svxdllapi.h"

// -----------
// - Defines -
// -----------

#define XOUTBMP_MIRROR_HORZ             0x00000001L
#define XOUTBMP_MIRROR_VERT             0x00000010L

#define XOUTBMP_CONTOUR_HORZ            0x00000001L
#define XOUTBMP_CONTOUR_VERT            0x00000002L
#define XOUTBMP_CONTOUR_EDGEDETECT      0x00000004L

#define XOUTBMP_DONT_EXPAND_FILENAME    0x10000000L
#define XOUTBMP_USE_GIF_IF_POSSIBLE     0x20000000L
#define XOUTBMP_USE_GIF_IF_SENSIBLE     0x40000000L
#define XOUTBMP_USE_NATIVE_IF_POSSIBLE  0x80000000L

// --------------
// - XOutBitmap -
// --------------

class GraphicFilter;
class VirtualDevice;
class INetURLObject;
class Polygon;

class SVX_DLLPUBLIC XOutBitmap
{
public:

    static GraphicFilter* pGrfFilter;

    static BitmapEx     CreateQuickDrawBitmapEx( const Graphic& rGraphic, const OutputDevice& rCompDev,
                                                 const MapMode& rMapMode, const Size& rLogSize,
                                                 const Point& rPoint, const Size& rSize );
    static Graphic      MirrorGraphic( const Graphic& rGraphic, const sal_uIntPtr nMirrorFlags );
    static Animation    MirrorAnimation( const Animation& rAnimation, sal_Bool bHMirr, sal_Bool bVMirr );
    static sal_uInt16       WriteGraphic( const Graphic& rGraphic, String& rFileName,
                                      const String& rFilterName, const sal_uIntPtr nFlags = 0L,
                                      const Size* pMtfSize_100TH_MM = NULL );
    static void         DrawQuickDrawBitmapEx( OutputDevice* pOutDev, const Point& rPt,
                                               const Size& rSize, const BitmapEx& rBmpEx );
    static void         DrawTiledBitmapEx( OutputDevice* pOutDev, const Point& rStartPt, const Size& rGrfSize,
                                           const Rectangle& rTileRect, const BitmapEx& rBmpEx );

    static sal_uInt16       ExportGraphic( const Graphic& rGraphic, const INetURLObject& rURL,
                                       GraphicFilter& rFilter, const sal_uInt16 nFormat,
                                       const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >* pFilterData = NULL );

    static Bitmap       DetectEdges( const Bitmap& rBmp, const sal_uInt8 cThreshold );

    static Polygon      GetCountour( const Bitmap& rBmp, const sal_uIntPtr nContourFlags,
                                     const sal_uInt8 cEdgeDetectThreshold = 50,
                                     const Rectangle* pWorkRect = NULL );
};

// ----------------
// - DitherBitmap -
// ----------------

SVX_DLLPUBLIC sal_Bool DitherBitmap( Bitmap& rBitmap );

#endif // _XOUTBMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
