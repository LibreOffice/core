/*************************************************************************
 *
 *  $RCSfile: wmfwr.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:59:00 $
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

#ifndef _WMFWR_HXX
#define _WMFWR_HXX

#include <tools/bigint.hxx>
#include <tools/debug.hxx>
#include <vcl/metaact.hxx>
#include <vcl/graph.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/virdev.hxx>
#include "fltcall.hxx"

// -----------------------------------------------------------------------------

#define MAXOBJECTHANDLES 16

// -----------------------------------------------------------------------------

struct WMFWriterAttrStackMember
{
    struct WMFWriterAttrStackMember * pSucc;
    Color aLineColor;
    Color aFillColor;
    RasterOp eRasterOp;
    Font aFont;
    MapMode aMapMode;
    Region aClipRegion;
};

// -------------
// - WMFWriter -
// -------------

class WMFWriter
{
private:

    BOOL            bStatus;

    PFilterCallback pCallback;
    void*           pCallerData;
    ULONG           nLastPercent; // Mit welcher Zahl pCallback zuletzt aufgerufen wurde.

    SvStream*       pWMF;
    VirtualDevice*  pVirDev;
    MapMode         aTargetMapMode;
    Size            aTargetSize;
    USHORT          nTargetDivisor;

    ULONG nMetafileHeaderPos;
    ULONG nMaxRecordSize; // in Worten
    ULONG nActRecordPos;

    // Aktuelle Attribute im Quell-Metafile:
    Color    aSrcLineColor;
    Color    aSrcFillColor;
    RasterOp eSrcRasterOp;
    Font     aSrcFont;
    MapMode  aSrcMapMode;
    BOOL     bSrcIsClipping;
    Region   aSrcClipRegion;
    WMFWriterAttrStackMember * pAttrStack;

    // Aktuelle Attribute im Ziel-Metafile:
    RasterOp  eDstROP2;
    Color     aDstTextColor;
    FontAlign eDstTextAlign;
    Color     aDstLineColor;
    Color     aDstFillColor;
    Font      aDstFont;
    BOOL      bDstIsClipping; // ???: derzeit unberuecksichtigt
    Region    aDstClipRegion; // ???: derzeit unberuecksichtigt
    BOOL bHandleAllocated[MAXOBJECTHANDLES];             // Welche Handles vergeben sind
    USHORT nDstPenHandle,nDstFontHandle,nDstBrushHandle; // Welche Handles die jeweiligen
                                                         // Selected-Objects besitzen
                                                         // 0xffff = keines:
    BOOL bDstTextInvalid; // ist TRUE, wenn die Variablen aDstTextColor, eDstTextAlign
                          // und aDstFont nicht mit den Attributen im Ziel-Metafile
                          // uebereinstimmen (Das ist am Anfang der Fall)
    BOOL bDstGraphicsInvalid;  // dito fuer alle anderen Attribute

    // Damit nicht bei jeder Operation alle Attribute verglichen werden muessen:
    BOOL bAttrReadyForLines;    // Ist TRUE, wenn die Attribute im Ziel-Metafile so
                                // gesetzt sind, dass Linien ausgegeben werden koennen.
    BOOL bAttrReadyForAreas;    // Ist TRUE, wenn die Attribute im Ziel-Metafile so
                                // gesetzt sind, dass gefuellte Polygone, Rechtecke etc.
                                // (einschliesslich Umrahmung) ausgegeben werden koennen.
    BOOL bAttrReadyForText;     // Ist TRUE, wenn die Attribute im Ziel-Metafile so
                                // gesetzt sind, dass Text ausgegeben werden kann.

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
    void WriteDX(long nDX);
    void WriteSize(const Size & rSize);
    void WriteHeightWidth(const Size & rSize);
    void WriteRectangle(const Rectangle & rRect);
    void WriteColor(const Color & rColor);

    void WriteRecordHeader(ULONG nSizeWords, USHORT nType);
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
    void WMFRecord_CreatePenIndirect(const Color& rColor);
    void WMFRecord_CreatePenIndirect(const Color& rColor, const LineInfo& rLineInfo );
    void WMFRecord_DeleteObject(USHORT nObjectHandle);
    void WMFRecord_Ellipse(const Rectangle & rRect);
    void WMFRecord_ExtTextOut(const Point & rPoint, const ByteString & rString, const long * pDXAry);
    void WMFRecord_ExtTextOut(const Point & rPoint, const ByteString & rString, ULONG nWidth);
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
    void WMFRecord_SetBkColor(const Color & rColor);
    void WMFRecord_SetBkMode(BOOL bTransparent);
    void WMFRecord_SetStretchBltMode();
    void WMFRecord_SetPixel(const Point & rPoint, const Color & rColor);
    void WMFRecord_SetROP2(RasterOp eROP);
    void WMFRecord_SetTextAlign(FontAlign eFontAlign);
    void WMFRecord_SetTextColor(const Color & rColor);
    void WMFRecord_SetWindowExt(const Size & rSize);
    void WMFRecord_SetWindowOrg(const Point & rPoint);
    void WMFRecord_StretchDIB(const Point & rPoint, const Size & rSize, const Bitmap & rBitmap, ULONG nROP = 0UL );
    void WMFRecord_TextOut(const Point & rPoint, const ByteString & rString);
    void WMFRecord_EndOfFile();
    void WMFRecord_IntersectClipRect( const Rectangle& rRect);

    USHORT AllocHandle();
    void FreeHandle(USHORT nObjectHandle);
    void CreateSelectDeletePen(const Color& rColor);
    void CreateSelectDeleteFont(const Font & rFont);
    void CreateSelectDeleteBrush(const Color& rColor);

    void SetAttrForLines();    // Setzt die Dst-Attribute fuer Linien
    void SetAttrForAreas();    // Setzt die Dst-Attribute fuer gefuellte Dinge.
                               // Macht automatisch SetAttrForLines, wenn BrushStyle NULL
    void SetAttrForText();     // Setzt die Dst-Attribute fuer Text-Ausgabe

    void WriteRecords(const GDIMetaFile & rMTF);

    void WriteHeader(const GDIMetaFile & rMTF, BOOL bPlaceable);
    void UpdateHeader();

    USHORT CalcSaveTargetMapMode(MapMode& rMapMode, const Size& rPrefSize);

public:

    WMFWriter() {}

    BOOL WriteWMF(const GDIMetaFile & rMTF, SvStream & rTargetStream,
                  PFilterCallback pcallback, void * pcallerdata,
                  BOOL bPlaceable=TRUE);
};

#endif
