/*************************************************************************
 *
 *  $RCSfile: xoutbmp.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ka $ $Date: 2000-11-10 14:51:03 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _XOUTBMP_HXX
#define _XOUTBMP_HXX

#ifndef _SV_GRAPH_HXX
#include <vcl/graph.hxx>
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

class XOutBitmap
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
                                       BOOL bIgnoreOptions );

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
