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

#ifndef INCLUDED_VCL_SOURCE_FILTER_WMF_EMFWR_HXX
#define INCLUDED_VCL_SOURCE_FILTER_WMF_EMFWR_HXX

#include <vcl/gdimtf.hxx>
#include <vcl/virdev.hxx>

class LineInfo;
namespace basegfx { class B2DPolygon; }
enum class EmfPlusRecordType;

class EMFWriter
{
private:

    ScopedVclPtr<VirtualDevice> maVDev;
    MapMode             maDestMapMode;
    SvStream& m_rStm;
    std::vector<bool>       mHandlesUsed;
    sal_uLong               mnHandleCount;
    sal_uLong               mnRecordCount;
    sal_uLong               mnRecordPos;
    sal_uLong               mnRecordPlusPos;
    bool                mbRecordOpen;
    bool                mbRecordPlusOpen;
    bool                mbLineChanged;
    sal_uInt32          mnLineHandle;
    bool                mbFillModeChanged;
    sal_uInt32          mnFillModeHandle;
    bool                mbFillChanged;
    sal_uInt32          mnFillHandle;
    bool                mbTextChanged;
    sal_uInt32          mnTextHandle;
    sal_uInt32          mnHorTextAlign;

    void                ImplBeginRecord( sal_uInt32 nType );
    void                ImplEndRecord();
    void                ImplBeginPlusRecord( EmfPlusRecordType nType, sal_uInt16 nFlags );
    void                ImplEndPlusRecord();
    void                ImplPlusRecord( EmfPlusRecordType nType, sal_uInt16 nFlags );
    void                ImplBeginCommentRecord( sal_Int32 nCommentType );
    void                ImplEndCommentRecord();

    sal_uLong               ImplAcquireHandle();
    void                ImplReleaseHandle( sal_uLong nHandle );

    bool                ImplPrepareHandleSelect( sal_uInt32& rHandle, sal_uLong nSelectType );
    void                ImplCheckLineAttr();
    void                ImplCheckFillAttr();
    void                ImplCheckTextAttr();

    void                ImplWriteColor( const Color& rColor );
    void                ImplWriteRasterOp( RasterOp eRop );
    void                ImplWriteExtent( tools::Long nExtent );
    void                ImplWritePoint( const Point& rPoint );
    void                ImplWriteSize( const Size& rSize);
    void                ImplWriteRect( const tools::Rectangle& rRect );
    void                ImplWritePath( const tools::PolyPolygon& rPolyPoly, bool bClose );
    void                ImplWritePolygonRecord( const tools::Polygon& rPoly, bool bClose );
    void                ImplWritePolyPolygonRecord( const tools::PolyPolygon& rPolyPoly );
    void                ImplWriteBmpRecord( const Bitmap& rBmp, const Point& rPt, const Size& rSz, sal_uInt32 nROP );
    void                ImplWriteTextRecord( const Point& rPos, const OUString& rText, o3tl::span<const sal_Int32> pDXArray, sal_uInt32 nWidth );

    void                Impl_handleLineInfoPolyPolygons(const LineInfo& rInfo, const basegfx::B2DPolygon& rLinePolygon);
    void                ImplWrite( const GDIMetaFile& rMtf );
    void                WriteEMFPlusHeader( const Size &rMtfSizePix, const Size &rMtfSizeLog );
    void                ImplWritePlusEOF();
    void                ImplWritePlusFillPolygonRecord( const tools::Polygon& rPoly, sal_uInt32 nTrans );
    void                ImplWritePlusColor( const Color& rColor, sal_uInt32 nTrans );
    void                ImplWritePlusPoint( const Point& rPoint );

public:

    explicit EMFWriter(SvStream &rStream)
        : maVDev( VclPtr<VirtualDevice>::Create() )
        , m_rStm(rStream)
        , mnHandleCount(0)
        , mnRecordCount(0)
        , mnRecordPos(0)
        , mnRecordPlusPos(0)
        , mbRecordOpen(false)
        , mbRecordPlusOpen(false)
        , mbLineChanged(false)
        , mnLineHandle(0)
        , mbFillChanged(false)
        , mnFillHandle(0)
        , mbTextChanged(false)
        , mnTextHandle(0)
        , mnHorTextAlign(0)
    {
    }

    bool WriteEMF(const GDIMetaFile& rMtf);
};

#endif // INCLUDED_VCL_SOURCE_FILTER_WMF_EMFWR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
