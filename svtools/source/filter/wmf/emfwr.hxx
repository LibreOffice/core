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

#ifndef _EMFWR_HXX
#define _EMFWR_HXX

#include <tools/debug.hxx>
#include <vcl/metaact.hxx>
#include <vcl/graph.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/virdev.hxx>
#include <svtools/fltcall.hxx>

// -------------
// - EMFWriter -
// -------------

class LineInfo;
namespace basegfx { class B2DPolygon; }

class EMFWriter
{
private:

    VirtualDevice       maVDev;
    MapMode             maDestMapMode;
    FilterConfigItem*   mpFilterConfigItem;
    SvStream& m_rStm;
    sal_Bool*               mpHandlesUsed;
    sal_uLong               mnHandleCount;
    sal_uLong               mnLastPercent;
    sal_uLong               mnRecordCount;
    sal_uLong               mnRecordPos;
    sal_Bool                mbRecordOpen;
    sal_Bool                mbLineChanged;
    sal_uInt32          mnLineHandle;
    sal_Bool                mbFillChanged;
    sal_uInt32          mnFillHandle;
    sal_Bool                mbTextChanged;
    sal_uInt32          mnTextHandle;
    sal_uInt32          mnHorTextAlign;

    void                ImplBeginRecord( sal_uInt32 nType );
    void                ImplEndRecord();

    sal_uLong               ImplAcquireHandle();
    void                ImplReleaseHandle( sal_uLong nHandle );

    sal_Bool                ImplPrepareHandleSelect( sal_uInt32& rHandle, sal_uLong nSelectType );
    void                ImplCheckLineAttr();
    void                ImplCheckFillAttr();
    void                ImplCheckTextAttr();

    void                ImplWriteColor( const Color& rColor );
    void                ImplWriteRasterOp( RasterOp eRop );
    void                ImplWriteExtent( long nExtent );
    void                ImplWritePoint( const Point& rPoint );
    void                ImplWriteSize( const Size& rSize);
    void                ImplWriteRect( const Rectangle& rRect );
    void                ImplWritePath( const PolyPolygon& rPolyPoly, sal_Bool bClose );
    void                ImplWritePolygonRecord( const Polygon& rPoly, sal_Bool bClose );
    void                ImplWritePolyPolygonRecord( const PolyPolygon& rPolyPoly );
    void                ImplWriteBmpRecord( const Bitmap& rBmp, const Point& rPt, const Size& rSz, sal_uInt32 nROP );
    void                ImplWriteTextRecord( const Point& rPos, const String rText, const sal_Int32* pDXArray, sal_uInt32 nWidth );

    void                Impl_handleLineInfoPolyPolygons(const LineInfo& rInfo, const basegfx::B2DPolygon& rLinePolygon);
    void                ImplWrite( const GDIMetaFile& rMtf );

public:

    EMFWriter(SvStream &rStream) : m_rStm(rStream) {}

    sal_Bool WriteEMF( const GDIMetaFile& rMtf, FilterConfigItem* pConfigItem = NULL );
};

#endif // _EMFWR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
