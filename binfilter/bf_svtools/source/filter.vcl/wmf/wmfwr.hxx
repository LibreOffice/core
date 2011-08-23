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

#ifndef _WMFWR_HXX
#define _WMFWR_HXX

#include <tools/bigint.hxx>
#include <tools/debug.hxx>
#include <vcl/metaact.hxx>
#include <vcl/graph.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/virdev.hxx>
#include <bf_svtools/fltcall.hxx>

class StarSymbolToMSMultiFont;

namespace binfilter
{

// -----------------------------------------------------------------------------

#define MAXOBJECTHANDLES 16

// -----------------------------------------------------------------------------

struct WMFWriterAttrStackMember
{
    struct WMFWriterAttrStackMember * pSucc;
    Color aLineColor;
    Color aFillColor;
    Color aTextColor;
    LineInfo aLineInfo;
    TextAlign eTextAlign;
    RasterOp eRasterOp;
    Font aFont;
    MapMode aMapMode;
    Region aClipRegion;
    sal_uInt16 nFlags;
};

// -------------
// - WMFWriter -
// -------------

class WMFWriter
{
private:

    BOOL			bStatus;

    ULONG					nLastPercent; // Mit welcher Zahl pCallback zuletzt aufgerufen wurde.
    FilterConfigItem*		pFilterConfigItem;

    com::sun::star::uno::Reference< com::sun::star::task::XStatusIndicator > xStatusIndicator;

    SvStream*				pWMF;
    VirtualDevice*			pVirDev;
    StarSymbolToMSMultiFont *pConvert;
    MapMode					aTargetMapMode;
    Size					aTargetSize;
    USHORT					nTargetDivisor;

    ULONG nMetafileHeaderPos;
    sal_uInt32 nMaxRecordSize; // in Worten
    ULONG nActRecordPos;

    // Aktuelle Attribute im Quell-Metafile:
    Color	  aSrcLineColor;
    Color	  aSrcFillColor;
    Color     aSrcTextColor;
    LineInfo  aSrcLineInfo;
    RasterOp  eSrcRasterOp;
    FontAlign eSrcTextAlign;
    Font      aSrcFont;
    MapMode   aSrcMapMode;
    BOOL      bSrcIsClipping;
    Region    aSrcClipRegion;
    WMFWriterAttrStackMember * pAttrStack;

    // Aktuelle Attribute im Ziel-Metafile:
    Color	  aDstLineColor;
    Color	  aDstFillColor;
    Color     aDstTextColor;
    LineInfo  aDstLineInfo;
    RasterOp  eDstROP2;
    FontAlign eDstTextAlign;
    Font      aDstFont;

    BOOL      bDstIsClipping; // ???: derzeit unberuecksichtigt
    Region    aDstClipRegion; // ???: derzeit unberuecksichtigt
    BOOL bHandleAllocated[MAXOBJECTHANDLES];             // Welche Handles vergeben sind
    USHORT nDstPenHandle,nDstFontHandle,nDstBrushHandle; // Welche Handles die jeweiligen
                                                         // Selected-Objects besitzen
                                                         // 0xffff = keines:

    // Damit nicht bei jeder Operation alle Attribute verglichen werden muessen:

    ULONG nNumberOfActions; // Anzahl der Actions im GDIMetafile
    ULONG nNumberOfBitmaps; // Anzahl der Bitmaps
    ULONG nWrittenActions;  // Anzahl der bereits verarbeiteten Actions beim Schreiben der Orders
    ULONG nWrittenBitmaps;  // Anzahl der bereits geschriebenen Bitmaps
    ULONG nActBitmapPercent; // Wieviel Prozent die naechste Bitmap schon geschrieben ist.

    void MayCallback();
        // Berechnet anhand der obigen 5 Parameter eine Prozentzahl
        // und macht dann ggf. einen Callback. Setzt bStatus auf FALSE wenn User abbrechen
        // moechte.

    void CountActionsAndBitmaps(const GDIMetaFile & rMTF);
        // Zaehlt die Bitmaps und Actions (nNumberOfActions und nNumberOfBitmaps muessen
        // zu Anfang auf 0 gesetzt werden, weil diese Methode rekursiv ist)

    void WritePointXY(const Point & rPoint);
    void WritePointYX(const Point & rPoint);
    sal_Int32 ScaleWidth( sal_Int32 nDX );
    void WriteSize(const Size & rSize);
    void WriteHeightWidth(const Size & rSize);
    void WriteRectangle(const Rectangle & rRect);
    void WriteColor(const Color & rColor);

    void WriteRecordHeader(sal_uInt32 nSizeWords, sal_uInt16 nType);
        // nSizeWords ist die Groesse des gesammten Records in Anzahl Worte.
        // Wenn nSizeWords unbekannt ist, dann 0 einsetzen (siehe UpdateRecordHeader()).

    void UpdateRecordHeader();
        // berichtig die Groesse des Records nach dem Schreiben der Parameter, wenn
        // nSizeWords bei Aufruf von WriteRecordHeader(..) unbekannt war.
        // fuegt ggf. noch ein BYTE 0 ein damit Anzahl Bytes immer gerade.

    void WMFRecord_Arc(const Rectangle & rRect, const Point & rStartPt, const Point & rEndPt);
    void WMFRecord_Chord(const Rectangle & rRect, const Point & rStartPt, const Point & rEndPt);
    void WMFRecord_CreateBrushIndirect(const Color& rColor);
    void WMFRecord_CreateFontIndirect(const Font & rFont);
    void WMFRecord_CreatePenIndirect(const Color& rColor, const LineInfo& rLineInfo );
    void WMFRecord_DeleteObject(USHORT nObjectHandle);
    void WMFRecord_Ellipse(const Rectangle & rRect);
    void WMFRecord_Escape( sal_uInt32 nEsc, sal_uInt32 nLen, const sal_Int8* pData );
    sal_Bool WMFRecord_Escape_Unicode( const Point& rPoint, const String& rStr, const sal_Int32 * pDXAry );
    void WMFRecord_ExtTextOut(const Point & rPoint, const String & rString, const sal_Int32 * pDXAry);

    void TrueExtTextOut(const Point & rPoint, const String & rString, 
        const ByteString & rByteString, const sal_Int32 * pDXAry);
    void TrueTextOut(const Point & rPoint, const ByteString& rString);
    void WMFRecord_LineTo(const Point & rPoint);
    void WMFRecord_MoveTo(const Point & rPoint);
    void WMFRecord_Pie(const Rectangle & rRect, const Point & rStartPt, const Point & rEndPt);
    void WMFRecord_Polygon(const Polygon & rPoly);
    void WMFRecord_PolyLine(const Polygon & rPoly);
    void WMFRecord_PolyPolygon(const PolyPolygon & rPolyPoly);
    void WMFRecord_Rectangle(const Rectangle & rRect);
    void WMFRecord_RestoreDC();
    void WMFRecord_RoundRect(const Rectangle & rRect, long nHorzRound, long nVertRound);
    void WMFRecord_SaveDC();
    void WMFRecord_SelectObject(USHORT nObjectHandle);
    void WMFRecord_SetBkMode(BOOL bTransparent);
    void WMFRecord_SetStretchBltMode();
    void WMFRecord_SetPixel(const Point & rPoint, const Color & rColor);
    void WMFRecord_SetROP2(RasterOp eROP);
    void WMFRecord_SetTextAlign(FontAlign eFontAlign);
    void WMFRecord_SetTextColor(const Color & rColor);
    void WMFRecord_SetWindowExt(const Size & rSize);
    void WMFRecord_SetWindowOrg(const Point & rPoint);
    void WMFRecord_StretchDIB(const Point & rPoint, const Size & rSize, const Bitmap & rBitmap, sal_uInt32 nROP = 0UL );
    void WMFRecord_TextOut(const Point & rPoint, const String & rString);
    void WMFRecord_EndOfFile();
    void WMFRecord_IntersectClipRect( const Rectangle& rRect);

    USHORT AllocHandle();
    void FreeHandle(USHORT nObjectHandle);
    void CreateSelectDeletePen( const Color& rColor, const LineInfo& rLineInfo );
    void CreateSelectDeleteFont(const Font & rFont);
    void CreateSelectDeleteBrush(const Color& rColor);

    void SetLineAndFillAttr();
    void SetAllAttr();

    void WriteRecords(const GDIMetaFile & rMTF);

    void WriteHeader(const GDIMetaFile & rMTF, BOOL bPlaceable);
    void UpdateHeader();

    USHORT CalcSaveTargetMapMode(MapMode& rMapMode, const Size& rPrefSize);

public:

    WMFWriter() {}

    BOOL WriteWMF(const GDIMetaFile & rMTF, SvStream & rTargetStream, FilterConfigItem* pFilterConfigItem, BOOL bPlaceable=TRUE);
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
