/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xoutbmp.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 13:02:43 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _XOUTBMP_HXX
#define _XOUTBMP_HXX

#ifndef _SV_GRAPH_HXX
#include <vcl/graph.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

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
    static Graphic      MirrorGraphic( const Graphic& rGraphic, const ULONG nMirrorFlags );
    static Animation    MirrorAnimation( const Animation& rAnimation, BOOL bHMirr, BOOL bVMirr );
    static USHORT       WriteGraphic( const Graphic& rGraphic, String& rFileName,
                                      const String& rFilterName, const ULONG nFlags = 0L,
                                      const Size* pMtfSize_100TH_MM = NULL );
    static void         DrawQuickDrawBitmapEx( OutputDevice* pOutDev, const Point& rPt,
                                               const Size& rSize, const BitmapEx& rBmpEx );
    static void         DrawTiledBitmapEx( OutputDevice* pOutDev, const Point& rStartPt, const Size& rGrfSize,
                                           const Rectangle& rTileRect, const BitmapEx& rBmpEx );

    static USHORT       ExportGraphic( const Graphic& rGraphic, const INetURLObject& rURL,
                                       GraphicFilter& rFilter, const USHORT nFormat,
                                       const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >* pFilterData = NULL );

    static Bitmap       DetectEdges( const Bitmap& rBmp, const BYTE cThreshold );

    static Polygon      GetCountour( const Bitmap& rBmp, const ULONG nContourFlags,
                                     const BYTE cEdgeDetectThreshold = 50,
                                     const Rectangle* pWorkRect = NULL );
};

// ----------------
// - DitherBitmap -
// ----------------

BOOL DitherBitmap( Bitmap& rBitmap );

#endif // _XOUTBMP_HXX
