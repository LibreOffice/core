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

#ifndef _EMFWR_HXX
#define _EMFWR_HXX

#include <vcl/metaact.hxx>
#include <vcl/graph.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/virdev.hxx>
#include <vcl/fltcall.hxx>

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
    sal_uLong               mnRecordPlusPos;
    sal_Bool                mbRecordOpen;
    sal_Bool                mbRecordPlusOpen;
    sal_Bool                mbLineChanged;
    sal_uInt32          mnLineHandle;
    sal_Bool                mbFillChanged;
    sal_uInt32          mnFillHandle;
    sal_Bool                mbTextChanged;
    sal_uInt32          mnTextHandle;
    sal_uInt32          mnHorTextAlign;

    void                ImplBeginRecord( sal_uInt32 nType );
    void                ImplEndRecord();
    void                ImplBeginPlusRecord( sal_uInt16 nType, sal_uInt16 nFlags );
    void                ImplEndPlusRecord();
    void                ImplPlusRecord( sal_uInt16 nType, sal_uInt16 nFlags );
    void                ImplBeginCommentRecord( sal_Int32 nCommentType );
    void                ImplEndCommentRecord();

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
    void                ImplWriteTextRecord( const Point& rPos, const OUString rText, const sal_Int32* pDXArray, sal_uInt32 nWidth );

    void                Impl_handleLineInfoPolyPolygons(const LineInfo& rInfo, const basegfx::B2DPolygon& rLinePolygon);
    void                ImplWrite( const GDIMetaFile& rMtf );
    void                WriteEMFPlusHeader( const Size &rMtfSizePix, const Size &rMtfSizeLog );
    void                ImplWritePlusEOF();
    void                ImplWritePlusFillPolygonRecord( const Polygon& rPoly, const sal_uInt32& nTrans );
    void                ImplWritePlusColor( const Color& rColor, const sal_uInt32& nTrans );
    void                ImplWritePlusPoint( const Point& rPoint );

public:

    EMFWriter(SvStream &rStream) : m_rStm(rStream) {}

    sal_Bool WriteEMF( const GDIMetaFile& rMtf, FilterConfigItem* pConfigItem = NULL );
};

#endif // _EMFWR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
