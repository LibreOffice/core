/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _XOUTBMP_HXX
#define _XOUTBMP_HXX

#include <vcl/graph.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include "svx/svxdllapi.h"

#define XOUTBMP_MIRROR_HORZ             0x00000001L
#define XOUTBMP_MIRROR_VERT             0x00000010L

#define XOUTBMP_CONTOUR_HORZ            0x00000001L
#define XOUTBMP_CONTOUR_VERT            0x00000002L
#define XOUTBMP_CONTOUR_EDGEDETECT      0x00000004L
#define XOUTBMP_DONT_ADD_EXTENSION      0x00000008L

#define XOUTBMP_DONT_EXPAND_FILENAME    0x10000000L
#define XOUTBMP_USE_GIF_IF_POSSIBLE     0x20000000L
#define XOUTBMP_USE_GIF_IF_SENSIBLE     0x40000000L
#define XOUTBMP_USE_NATIVE_IF_POSSIBLE  0x80000000L

class GraphicFilter;
class VirtualDevice;
class INetURLObject;
class Polygon;

class SVX_DLLPUBLIC XOutBitmap
{
public:

    static GraphicFilter* pGrfFilter;

    static Graphic      MirrorGraphic( const Graphic& rGraphic, const sal_uIntPtr nMirrorFlags );
    static Animation    MirrorAnimation( const Animation& rAnimation, sal_Bool bHMirr, sal_Bool bVMirr );
    static sal_uInt16   WriteGraphic( const Graphic& rGraphic, OUString& rFileName,
                                      const OUString& rFilterName, const sal_uIntPtr nFlags = 0L,
                                      const Size* pMtfSize_100TH_MM = NULL );
    static sal_uLong    GraphicToBase64(const Graphic& rGraphic,OUString& rOUString);

    static sal_uInt16   ExportGraphic( const Graphic& rGraphic, const INetURLObject& rURL,
                                       GraphicFilter& rFilter, const sal_uInt16 nFormat,
                                       const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >* pFilterData = NULL );

    static Bitmap       DetectEdges( const Bitmap& rBmp, const sal_uInt8 cThreshold );

    static Polygon      GetCountour( const Bitmap& rBmp, const sal_uIntPtr nContourFlags,
                                     const sal_uInt8 cEdgeDetectThreshold = 50,
                                     const Rectangle* pWorkRect = NULL );
};

SVX_DLLPUBLIC sal_Bool DitherBitmap( Bitmap& rBitmap );

#endif // _XOUTBMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
