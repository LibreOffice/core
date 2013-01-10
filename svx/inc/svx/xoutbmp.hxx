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
#define XOUTBMP_DONT_ADD_EXTENSION      0x00000008L

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

    static Graphic      MirrorGraphic( const Graphic& rGraphic, const sal_uIntPtr nMirrorFlags );
    static Animation    MirrorAnimation( const Animation& rAnimation, sal_Bool bHMirr, sal_Bool bVMirr );
    static sal_uInt16       WriteGraphic( const Graphic& rGraphic, String& rFileName,
                                      const String& rFilterName, const sal_uIntPtr nFlags = 0L,
                                      const Size* pMtfSize_100TH_MM = NULL );
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
