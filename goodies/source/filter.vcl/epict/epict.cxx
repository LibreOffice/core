/*************************************************************************
 *
 *  $RCSfile: epict.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:12 $
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

#if SUPD <= 364
#include <svtfilt.hxx>
#include <solar.hrc>
#include <fltcall.hxx>
#else
#include <vcl/metaact.hxx>
#include <svtools/filter.hxx>
#include <svtools/solar.hrc>
#include <svtools/fltcall.hxx>
#endif

#include <math.h>
#include <vcl/bmpacc.hxx>
#include <vcl/metaact.hxx>
#include <vcl/graph.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/gradient.hxx>
#include <vcl/hatch.hxx>
#include <vcl/metric.hxx>
#include <vcl/font.hxx>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include <tools/bigint.hxx>
#include "strings.hrc"
#include "dlgepct.hrc"
#include "dlgepct.hxx"

//============================== PictWriter ===================================

struct PictWriterAttrStackMember {
    struct PictWriterAttrStackMember *  pSucc;
    Color                               aLineColor;
    Color                               aFillColor;
    RasterOp                            eRasterOp;
    Font                                aFont;
    MapMode                             aMapMode;
};


enum PictDrawingMethod {
    PDM_FRAME, PDM_PAINT, PDM_ERASE, PDM_INVERT, PDM_FILL
};


struct PictPattern {
    ULONG nLo, nHi;
};


class PictWriter {

private:

    BOOL bStatus;

    PFilterCallback pCallback;
    void * pCallerData;
    ULONG nLastPercent; // Mit welcher Zahl pCallback zuletzt aufgerufen wurde.

    SvStream * pPict;

    // Aktuelle Attribute im Quell-Metafile:
    Color    aLineColor;
    Color    aFillColor;
    RasterOp eSrcRasterOp;
    Font     aSrcFont;
    MapMode  aSrcMapMode;
    MapMode  aTargetMapMode;
    PictWriterAttrStackMember * pAttrStack;

    // Aktuelle Attribute im Ziel-Metafile, und ob sie gueltig sind
    BOOL        bDstBkPatVisible;   BOOL bDstBkPatValid;
    BYTE        nDstTxFace;         BOOL bDstTxFaceValid;
    RasterOp    eDstTxMode;         BOOL bDstTxModeValid;
    USHORT      nDstPnSize;         BOOL bDstPnSizeValid;
    RasterOp    eDstPnMode;         BOOL bDstPnModeValid;
    PictPattern aDstPnPat;          BOOL bDstPnPatValid;
    BOOL        bDstFillPatVisible; BOOL bDstFillPatValid;
    USHORT      nDstTxSize;         BOOL bDstTxSizeValid;
    Color       aDstFgCol;          BOOL bDstFgColValid;
    Color       aDstBkCol;          BOOL bDstBkColValid;
    Point       aDstPenPosition;    BOOL bDstPenPositionValid;
    Point       aDstTextPosition;   BOOL bDstTextPositionValid;
    String      aDstFontName; USHORT nDstFontNameId; BOOL bDstFontNameValid;

    ULONG nNumberOfActions;  // Anzahl der Actions im GDIMetafile
    ULONG nNumberOfBitmaps;  // Anzahl der Bitmaps
    ULONG nWrittenActions;   // Anzahl der bereits verarbeiteten Actions beim Schreiben der Opcodes
    ULONG nWrittenBitmaps;   // Anzahl der bereits geschriebenen Bitmaps
    ULONG nActBitmapPercent; // Wieviel Prozent die naechste Bitmap schon geschrieben ist.

    void MayCallback();
        // Berechnet anhand der obigen 5 Parameter eine Prozentzahl
        // und macht dann ggf. einen Callback. Setzt bStatus auf FALSE wenn User abbrechen
        // moechte.

    void CountActionsAndBitmaps(const GDIMetaFile & rMTF);
        // Zaehlt die Bitmaps und Actions (nNumberOfActions und nNumberOfBitmaps muessen
        // zu Anfang auf 0 gesetzt werden, weil diese Methode rekursiv ist)

    Polygon PolyPolygonToPolygon(const PolyPolygon & rPoly);
        // Macht aus einem PolyPolygon ein halbwegs vernuenftiges Polygon

    void WritePoint(const Point & rPoint);
    void WriteSize(const Size & rSize);
    void WriteRGBColor(const Color & rColor);
    void WriteString( const ByteString & rString );
    void WriteRectangle(const Rectangle & rRect);
    void WritePolygon(const Polygon & rPoly);
    void WriteArcAngles(const Rectangle & rRect, const Point & rStartPt, const Point & rEndPt);

    void ConvertLinePattern(PictPattern & rPat, BOOL bVisible);
    void ConvertFillPattern(PictPattern & rPat, BOOL bVisible);

    void WriteOpcode_BkPat(BOOL bVisible);
    void WriteOpcode_TxFace(const Font & rFont);
    void WriteOpcode_TxMode(RasterOp eMode);
    void WriteOpcode_PnSize(USHORT nSize);
    void WriteOpcode_PnMode(RasterOp eMode);
    void WriteOpcode_PnLinePat(BOOL bVisible);
    void WriteOpcode_PnFillPat(BOOL bVisible);
    void WriteOpcode_FillPat(BOOL bVisible);
    void WriteOpcode_OvSize(const Size & rSize);
    void WriteOpcode_TxSize(USHORT nSize);
    void WriteOpcode_RGBFgCol(const Color & rColor);
    void WriteOpcode_RGBBkCol(const Color & rColor);
    void WriteOpcode_Line(const Point & rLocPt, const Point & rNewPt);
    void WriteOpcode_LineFrom(const Point & rNewPt);
    void WriteOpcode_Text(const Point & rPoint, const ByteString& rString, BOOL bDelta);
    void WriteOpcode_FontName(const Font & rFont);
    void WriteOpcode_Rect(PictDrawingMethod eMethod, const Rectangle & rRect);
    void WriteOpcode_SameRect(PictDrawingMethod eMethod);
    void WriteOpcode_RRect(PictDrawingMethod eMethod, const Rectangle & rRect);
    void WriteOpcode_SameRRect(PictDrawingMethod eMethod);
    void WriteOpcode_Oval(PictDrawingMethod eMethod, const Rectangle & rRect);
    void WriteOpcode_SameOval(PictDrawingMethod eMethod);
    void WriteOpcode_Arc(PictDrawingMethod eMethod, const Rectangle & rRect,
                         const Point & rStartPt, const Point & rEndPt);
    void WriteOpcode_SameArc(PictDrawingMethod eMethod, const Rectangle & rRect,
                             const Point & rStartPt, const Point & rEndPt);
    void WriteOpcode_Poly(PictDrawingMethod eMethod, const Polygon & rPoly);
    void WriteOpcode_BitsRect(const Point & rPoint, const Size & rSize, const Bitmap & rBitmap);
    void WriteOpcode_EndOfFile();

    void SetAttrForPaint();
    void SetAttrForFrame();
    void SetAttrForText();

    void WriteTextArray(Point & rPoint, const ByteString& rString, const long * pDXAry);

    void WriteOpcodes(const GDIMetaFile & rMTF);

    void WriteHeader(const GDIMetaFile & rMTF);
    void UpdateHeader();

public:

    BOOL WritePict(const GDIMetaFile & rMTF, SvStream & rTargetStream,
                   PFilterCallback pcallback, void * pcallerdata);
};


//========================== Methoden von PictWriter ==========================


void PictWriter::MayCallback()
{
    ULONG nPercent;

    // Wir gehen mal einfach so davon aus, dass 16386 Actions einer Bitmap entsprechen
    // (in der Regel wird ein Metafile entweder nur Actions oder einige Bitmaps und fast
    // keine Actions enthalten. Dann ist das Verhaeltnis ziemlich unwichtig)

    nPercent=((nWrittenBitmaps<<14)+(nActBitmapPercent<<14)/100+nWrittenActions)
             *100
             /((nNumberOfBitmaps<<14)+nNumberOfActions);

    if (nPercent>=nLastPercent+3) {
        nLastPercent=nPercent;
        if(pCallback!=NULL && nPercent<=100) {
            if (((*pCallback)(pCallerData,(USHORT)nPercent))==TRUE) bStatus=FALSE;
        }
    }
}


void PictWriter::CountActionsAndBitmaps(const GDIMetaFile & rMTF)
{
    ULONG               nAction, nActionCount;
    const MetaAction*   pMA;

    nActionCount = rMTF.GetActionCount();

    for (nAction=0; nAction<nActionCount; nAction++)
    {
        pMA = rMTF.GetAction( nAction );

        switch( pMA->GetType() )
        {
            case META_BMP_ACTION:
            case META_BMPSCALE_ACTION:
            case META_BMPSCALEPART_ACTION:
            case META_BMPEX_ACTION:
            case META_BMPEXSCALE_ACTION:
            case META_BMPEXSCALEPART_ACTION:
                nNumberOfBitmaps++;
            break;
        }

        nNumberOfActions++;
    }
}


Polygon PictWriter::PolyPolygonToPolygon(const PolyPolygon & rPolyPoly)
{
    USHORT nCount,nSize1,nSize2,np,i1,i2,i3,nBestIdx1,nBestIdx2;
    long nDistSqr,nBestDistSqr, nCountdownTests;
    Point aP1,aPRel;
    Polygon aPoly1, aPoly2, aPoly3;

    nCount=rPolyPoly.Count();
    if (nCount==0) return Polygon(0);

    aPoly1=rPolyPoly.GetObject(0);
    for (np=1; np<nCount; np++) {
        aPoly2=rPolyPoly.GetObject(np);

        //-----------------Folgendes verschmilzt aPoly1 und aPoly2 zu aPoly1-----------------

        nSize1=aPoly1.GetSize();
        nSize2=aPoly2.GetSize();

        // Zunaechst werden ein Punkt in aPoly1 (referenziert durch nBestIdx1) und ein
        // Punkt in aPoly2 (referenziert durch nBestIdx2) gesucht, die moeglichst dicht
        // beieinander liegen. Da dies mit quadratischem Aufwand einher geht, und somit
        // manche Bilder Ewigkeiten benoetigen, um exportiert zu werden, begrenzen wir
        // die Anzahl der Tests auf 1000, und brechen die Suche ggf. schon vorher ab.
        // Dadruch wird das Ergebnis nicht falsch, sondern eventuell nicht so schoen.
        nCountdownTests=1000;
        nBestDistSqr=0x7fffffff;
        nBestIdx1=0;
        nBestIdx2=0;
        for (i1=0; i1<nSize1; i1++) {
            aP1=aPoly1.GetPoint(i1);
            for (i2=0; i2<nSize2; i2++) {
                aPRel=aPoly2.GetPoint(i2); aPRel-=aP1;
                nDistSqr=aPRel.X()*aPRel.X()+aPRel.Y()*aPRel.Y();
                if (nDistSqr<nBestDistSqr) {
                    nBestIdx1=i1;
                    nBestIdx2=i2;
                    nBestDistSqr=nDistSqr;
                }
                if (nCountdownTests<=0) break;
                nCountdownTests--;
            }
            if (nCountdownTests<=0) break;
        }

        // Nun werden aPoly1 und aPoly2 zu einem Polygon aPoly3 (spaeter aPoly1) zusammengefuegt.
        // Die beiden Polygone werden verbunden durch zwei zusaetzliche Kanten zwischen den oben
        // gefundenen Punkten.
        aPoly3.Clear();
        aPoly3.SetSize(nSize1+nSize2+2);
        i3=0;
        for (i1=nBestIdx1; i1<nSize1;     i1++) aPoly3.SetPoint(aPoly1.GetPoint(i1),i3++);
        for (i1=0;         i1<=nBestIdx1; i1++) aPoly3.SetPoint(aPoly1.GetPoint(i1),i3++);
        for (i2=nBestIdx2; i2<nSize2;     i2++) aPoly3.SetPoint(aPoly2.GetPoint(i2),i3++);
        for (i2=0;         i2<=nBestIdx2; i2++) aPoly3.SetPoint(aPoly2.GetPoint(i2),i3++);

        aPoly1=aPoly3;

        //-----------------------------------------------------------------------------------

    }
    return aPoly1;
}


void PictWriter::WritePoint(const Point & rPoint)
{
    Point aPoint = OutputDevice::LogicToLogic( rPoint, aSrcMapMode, aTargetMapMode );
    *pPict << ((short)aPoint.Y()) << ((short)aPoint.X());
}


void PictWriter::WriteSize(const Size & rSize)
{
    Size aSize = OutputDevice::LogicToLogic( rSize, aSrcMapMode, aTargetMapMode );
    *pPict << ((short)rSize.Height()) << ((short)rSize.Width());
}


void PictWriter::WriteRGBColor(const Color & rColor)
{
    const UINT16 nR = ( (UINT16) rColor.GetRed() << 8 ) | (UINT16) rColor.GetRed();
    const UINT16 nG = ( (UINT16) rColor.GetGreen() << 8 ) | (UINT16) rColor.GetGreen();
    const UINT16 nB = ( (UINT16) rColor.GetBlue() << 8 ) | (UINT16) rColor.GetBlue();

    *pPict << nR << nG << nB;
}


void PictWriter::WriteString( const ByteString & rString )
{
    USHORT i,nLen;

    nLen = rString.Len();
    if ( nLen > 255 )
        nLen = 255;
    *pPict << ( (BYTE)nLen );
    for ( i = 0; i < nLen; i++ )
        *pPict << rString.GetChar( i );
}


void PictWriter::WriteRectangle(const Rectangle & rRect)
{
    Point   aPoint = OutputDevice::LogicToLogic( rRect.TopLeft(),
                                                 aSrcMapMode,
                                                 aTargetMapMode );
    Size    aSize = OutputDevice::LogicToLogic( rRect.GetSize(),
                                                aSrcMapMode,
                                                aTargetMapMode );
    Rectangle aRect( aPoint, aSize );


    short x1,y1,x2,y2,t;

    x1=(short) aRect.Left();
    y1=(short) aRect.Top();
    x2=(short) aRect.Right();
    y2=(short) aRect.Bottom();
    if (x1>x2) { t=x1; x1=x2; x2=t; }
    if (y1>y2) { t=y1; y1=y2; y2=t; }
    x2++;
    y2++;
    *pPict << y1 << x1 << y2 << x2;
}


void PictWriter::WritePolygon(const Polygon & rPoly)
{
    USHORT nDataSize,i,nSize;
    short nMinX,nMinY,nMaxX,nMaxY,nx,ny;
    Polygon aPoly(rPoly);

    nSize=aPoly.GetSize();

    if (aPoly.GetPoint(0) != aPoly.GetPoint(nSize-1))
    {
        nSize++;
        aPoly.SetSize(nSize);
        aPoly.SetPoint(aPoly.GetPoint(0),nSize-1);
    }

    nDataSize=nSize*4+10;
    for (i=0; i<nSize; i++)
    {
        Point aPoint = OutputDevice::LogicToLogic( aPoly.GetPoint( i ),
                                                   aSrcMapMode,
                                                   aTargetMapMode );

        nx = (short) aPoint.X();
        ny = (short) aPoint.Y();

        if ( i==0 || nMinX>nx )
            nMinX=nx;
        if ( i==0 || nMinY>ny )
            nMinY=ny;
        if ( i==0 || nMaxX<nx )
            nMaxX=nx;
        if ( i==0 || nMaxY<ny )
            nMaxY=ny;
    }

    *pPict << nDataSize << nMinY << nMinX << nMaxY << nMaxX;

    for (i=0; i<nSize; i++)
        WritePoint( aPoly.GetPoint(i) );
}


void PictWriter::WriteArcAngles(const Rectangle & rRect, const Point & rStartPt, const Point & rEndPt)
{
    Point       aStartPt = OutputDevice::LogicToLogic( rStartPt,
                                                       aSrcMapMode,
                                                       aTargetMapMode );
    Point       aEndPt = OutputDevice::LogicToLogic( rEndPt,
                                                     aSrcMapMode,
                                                     aTargetMapMode );
    Rectangle   aRect( OutputDevice::LogicToLogic( rRect.TopLeft(),
                                                   aSrcMapMode,
                                                   aTargetMapMode ),
                       OutputDevice::LogicToLogic( rRect.GetSize(),
                                                   aSrcMapMode,
                                                   aTargetMapMode ) );
    Point aCenter;
    double fAngS, fAngE, fdx, fdy;
    short nStartAngle, nArcAngle;


    aCenter=Point( ( aRect.Left() + aRect.Right() ) >> 1,
                   ( aRect.Top() + aRect.Bottom() ) >> 1 );

    fdx=(double)(aStartPt.X()-aCenter.X());
    fdy=(double)(aStartPt.Y()-aCenter.Y());
    if ( fdx==0.0 && fdy==0.0 )
        fdx=1.0;
    fAngE=atan2(fdx,-fdy);

    fdx=(double)(aEndPt.X()-aCenter.X());
    fdy=(double)(aEndPt.Y()-aCenter.Y());
    if ( fdx==0.0 && fdy==0.0 )
        fdx=1.0;
    fAngS=atan2(fdx,-fdy);

    nStartAngle=(short)(fAngS*180.0/3.14159265359);
    nArcAngle=((short)(fAngE*180.0/3.14159265359))-nStartAngle;
    if (nArcAngle<0)
        nArcAngle += 360;
    *pPict << nStartAngle << nArcAngle;
}


void PictWriter::ConvertLinePattern(PictPattern & rPat, BOOL bVisible)
{
    if( bVisible )
    {
        rPat.nHi=0xffffffff;
        rPat.nLo=0xffffffff;
    }
    else
    {
        rPat.nHi=0x00000000;
        rPat.nLo=0x00000000;
    }
}

void PictWriter::ConvertFillPattern(PictPattern & rPat, BOOL bVisible)
{
    if( bVisible )
    {
        rPat.nHi=0xffffffff;
        rPat.nLo=0xffffffff;
    }
    else
    {
        rPat.nHi=0x00000000;
        rPat.nLo=0x00000000;
    }
}


void PictWriter::WriteOpcode_BkPat(BOOL bVisible)
{
    if (bDstBkPatValid==FALSE || bDstBkPatVisible!=bVisible) {
        PictPattern aPat;
        ConvertFillPattern(aPat,bVisible);
        *pPict << (USHORT)0x0002 << aPat.nHi << aPat.nLo;
        bDstBkPatVisible=bVisible;
        bDstBkPatValid=TRUE;
    }
}


void PictWriter::WriteOpcode_TxFace(const Font & rFont)
{
    BYTE nFace;
    FontWeight eWeight;

    nFace=0;
    eWeight=rFont.GetWeight();
    if (eWeight==WEIGHT_BOLD ||
        eWeight==WEIGHT_SEMIBOLD ||
        eWeight==WEIGHT_ULTRABOLD ||
        eWeight==WEIGHT_BLACK)                nFace|=0x01;
    if (rFont.GetItalic()!=ITALIC_NONE)       nFace|=0x02;
    if (rFont.GetUnderline()!=UNDERLINE_NONE) nFace|=0x04;
    if (rFont.IsOutline()==TRUE)              nFace|=0x08;
    if (rFont.IsShadow()==TRUE)               nFace|=0x10;

    if (bDstTxFaceValid==FALSE || nDstTxFace!=nFace) {
        *pPict << (USHORT)0x0004 << nFace << (BYTE)0;
        nDstTxFace=nFace;
        bDstTxFaceValid=TRUE;
    }
}


void PictWriter::WriteOpcode_TxMode(RasterOp eMode)
{
    USHORT nVal;

    if (bDstTxModeValid==FALSE || eDstTxMode!=eMode) {
        switch (eMode) {
            case ROP_INVERT: nVal=0x000c; break;
            case ROP_XOR:    nVal=0x000a; break;
            default:         nVal=0x0008;
        }
        *pPict << (USHORT)0x0005 << nVal;
        eDstTxMode=eMode;
        bDstTxModeValid=TRUE;
    }
}


void PictWriter::WriteOpcode_PnSize(USHORT nSize)
{
    if (nSize==0) nSize=1;
    if (bDstPnSizeValid==FALSE || nDstPnSize!=nSize) {
        *pPict << (USHORT)0x0007 << nSize << nSize;
        nDstPnSize=nSize;
        bDstPnSizeValid=TRUE;
    }
}


void PictWriter::WriteOpcode_PnMode(RasterOp eMode)
{
    USHORT nVal;

    if (bDstPnModeValid==FALSE || eDstPnMode!=eMode) {
        switch (eMode)
        {
            case ROP_INVERT: nVal=0x000c; break;
            case ROP_XOR:    nVal=0x000a; break;
            default:         nVal=0x0008;
        }
        *pPict << (USHORT)0x0008 << nVal;
        eDstPnMode=eMode;
        bDstPnModeValid=TRUE;
    }
}


void PictWriter::WriteOpcode_PnLinePat(BOOL bVisible)
{
    PictPattern aPat;

    ConvertLinePattern(aPat,bVisible);
    if (bDstPnPatValid==FALSE || aDstPnPat.nHi!=aPat.nHi || aDstPnPat.nLo!=aPat.nLo) {
        *pPict << (USHORT)0x0009 << aPat.nHi << aPat.nLo;
        aDstPnPat=aPat;
        bDstPnPatValid=TRUE;
    }
}


void PictWriter::WriteOpcode_PnFillPat(BOOL bVisible)
{
    PictPattern aPat;

    ConvertFillPattern(aPat,bVisible);
    if (bDstPnPatValid==FALSE || aDstPnPat.nHi!=aPat.nHi || aDstPnPat.nLo!=aPat.nLo) {
        *pPict << (USHORT)0x0009 << aPat.nHi << aPat.nLo;
        aDstPnPat=aPat;
        bDstPnPatValid=TRUE;
    }
}


void PictWriter::WriteOpcode_FillPat(BOOL bVisible)
{
    if (bDstFillPatValid==FALSE || bDstFillPatVisible!=bVisible) {
        PictPattern aPat;
        ConvertFillPattern(aPat,bVisible);
        *pPict << (USHORT)0x000a << aPat.nHi << aPat.nLo;
        bDstFillPatVisible=bVisible;
        bDstFillPatValid=TRUE;
    }
}


void PictWriter::WriteOpcode_OvSize(const Size & rSize)
{
    *pPict << (USHORT)0x000b;
    WriteSize(rSize);
}


void PictWriter::WriteOpcode_TxSize(USHORT nSize)
{
    if (bDstTxSizeValid==FALSE || nDstTxSize!=nSize) {

        nDstTxSize = (USHORT) OutputDevice::LogicToLogic( Size( 0, nSize ),
                                                          aSrcMapMode, aTargetMapMode ).Height();

        *pPict << (USHORT)0x000d << nDstTxSize;
        bDstTxSizeValid=TRUE;
    }
}


void PictWriter::WriteOpcode_RGBFgCol(const Color & rColor)
{
    if (bDstFgColValid==FALSE || aDstFgCol!=rColor) {
        *pPict << (USHORT)0x001a;
        WriteRGBColor(rColor);
        aDstFgCol=rColor;
        bDstFgColValid=TRUE;
    }
}


void PictWriter::WriteOpcode_RGBBkCol(const Color & rColor)
{
    if (bDstBkColValid==FALSE || aDstBkCol!=rColor) {
        *pPict << (USHORT)0x001b;
        WriteRGBColor(rColor);
        aDstBkCol=rColor;
        bDstBkColValid=TRUE;
    }
}


void PictWriter::WriteOpcode_Line(const Point & rLocPt, const Point & rNewPt)
{
    Point aLocPt = OutputDevice::LogicToLogic( rLocPt,
                                               aSrcMapMode,
                                               aTargetMapMode );
    Point aNewPt = OutputDevice::LogicToLogic( rNewPt,
                                               aSrcMapMode,
                                               aTargetMapMode );
    long  dh,dv;

    dh=aNewPt.X()-aLocPt.X();
    dv=aNewPt.Y()-aLocPt.Y();
    if (dh<=127 && dh>=-128 && dv<=127 && dv>=-128)
    { // ShortLine
        *pPict << (USHORT)0x0022;
        WritePoint(rLocPt);
        *pPict << (char)dh << (char)dv;
    }
    else
    {
        *pPict << (USHORT)0x0020;
        WritePoint(rLocPt);
        WritePoint(rNewPt);
    }
    aDstPenPosition=rNewPt;
    bDstPenPositionValid=TRUE;
}


void PictWriter::WriteOpcode_LineFrom(const Point & rNewPt)
{
    Point aNewPt = OutputDevice::LogicToLogic( rNewPt,
                                               aSrcMapMode,
                                               aTargetMapMode );
    long  dh,dv;

    dh = aNewPt.X()-aDstPenPosition.X();
    dv = aNewPt.Y()-aDstPenPosition.Y();

    if (dh<=127 && dh>=-128 && dv<=127 && dv>=-128)
    { // ShortLine
        *pPict << (USHORT)0x0023;
        *pPict << (char)dh << (char)dv;
    }
    else
    {
        *pPict << (USHORT)0x0021;
        WritePoint(rNewPt);
    }
    aDstPenPosition=rNewPt;
    bDstPenPositionValid=TRUE;
}


void PictWriter::WriteOpcode_Text(const Point & rPoint, const ByteString& rString, BOOL bDelta)
{
    Point aPoint = OutputDevice::LogicToLogic( rPoint,
                                               aSrcMapMode,
                                               aTargetMapMode );
    long  dh,dv;
    ULONG nPos;

    nPos = pPict->Tell();
    dh = aPoint.X()-aDstTextPosition.X();
    dv = aPoint.Y()-aDstTextPosition.Y();

    if (bDstTextPositionValid==FALSE || dh<0 || dh>255 || dv<0 || dv>0 || bDelta==FALSE)
    {
        *pPict << (USHORT)0x0028;
        WritePoint(rPoint);
    }
    else if (dv==0)
    {
        *pPict << (USHORT)0x0029 << (BYTE)dh;
    }
    else if (dh==0)
    {
        *pPict << (USHORT)0x002a << (BYTE)dv;
    }
    else
    {
        *pPict << (USHORT)0x002b << (BYTE)dh << (BYTE)dv;
    }

    WriteString( rString );
    if (((pPict->Tell()-nPos)&1)!=0)
        *pPict << (BYTE)0;

    aDstTextPosition = aPoint;
    bDstTextPositionValid=TRUE;
}


void PictWriter::WriteOpcode_FontName(const Font & rFont)
{
    USHORT nDataLen,nFontId;

    switch (rFont.GetFamily()) {
        case FAMILY_MODERN:     nFontId=22; break;
        case FAMILY_ROMAN:      nFontId=20; break;
        case FAMILY_SWISS:      nFontId=21; break;
        default:                nFontId=1;
    }

    if (bDstFontNameValid==FALSE || nDstFontNameId!=nFontId || aDstFontName!=rFont.GetName()) {
        if (rFont.GetName().Len()>0) {
            nDataLen=3+rFont.GetName().Len();
            *pPict << (USHORT)0x002c << nDataLen << nFontId;
            WriteString( ByteString( rFont.GetName(), RTL_TEXTENCODING_UTF8 ) );
            if ( ( ( rFont.GetName().Len() ) & 1 ) == 0 )
                *pPict << (BYTE)0;
        }
        *pPict << (USHORT)0x0003 << nFontId;
        aDstFontName=rFont.GetName();
        nDstFontNameId=nFontId;
        bDstFontNameValid=TRUE;
    }
}


void PictWriter::WriteOpcode_Rect(PictDrawingMethod eMethod, const Rectangle & rRect)
{
    USHORT oc;
    switch (eMethod) {
        case PDM_FRAME:  oc=0x0030; break;
        case PDM_PAINT:  oc=0x0031; break;
        case PDM_ERASE:  oc=0x0032; break;
        case PDM_INVERT: oc=0x0033; break;
        case PDM_FILL:   oc=0x0034; break;
    }
    *pPict << oc;
    WriteRectangle(rRect);
}


void PictWriter::WriteOpcode_SameRect(PictDrawingMethod eMethod)
{
    USHORT oc;
    switch (eMethod) {
        case PDM_FRAME:  oc=0x0038; break;
        case PDM_PAINT:  oc=0x0039; break;
        case PDM_ERASE:  oc=0x003a; break;
        case PDM_INVERT: oc=0x003b; break;
        case PDM_FILL:   oc=0x003c; break;
    }
    *pPict << oc;
}


void PictWriter::WriteOpcode_RRect(PictDrawingMethod eMethod, const Rectangle & rRect)
{
    USHORT oc;
    switch (eMethod) {
        case PDM_FRAME:  oc=0x0040; break;
        case PDM_PAINT:  oc=0x0041; break;
        case PDM_ERASE:  oc=0x0042; break;
        case PDM_INVERT: oc=0x0043; break;
        case PDM_FILL:   oc=0x0044; break;
    }
    *pPict << oc;
    WriteRectangle(rRect);
}


void PictWriter::WriteOpcode_SameRRect(PictDrawingMethod eMethod)
{
    USHORT oc;
    switch (eMethod) {
        case PDM_FRAME:  oc=0x0048; break;
        case PDM_PAINT:  oc=0x0049; break;
        case PDM_ERASE:  oc=0x004a; break;
        case PDM_INVERT: oc=0x004b; break;
        case PDM_FILL:   oc=0x004c; break;
    }
    *pPict << oc;
}


void PictWriter::WriteOpcode_Oval(PictDrawingMethod eMethod, const Rectangle & rRect)
{
    USHORT oc;
    switch (eMethod) {
        case PDM_FRAME:  oc=0x0050; break;
        case PDM_PAINT:  oc=0x0051; break;
        case PDM_ERASE:  oc=0x0052; break;
        case PDM_INVERT: oc=0x0053; break;
        case PDM_FILL:   oc=0x0054; break;
    }
    *pPict << oc;
    WriteRectangle(rRect);
}


void PictWriter::WriteOpcode_SameOval(PictDrawingMethod eMethod)
{
    USHORT oc;
    switch (eMethod) {
        case PDM_FRAME:  oc=0x0058; break;
        case PDM_PAINT:  oc=0x0059; break;
        case PDM_ERASE:  oc=0x005a; break;
        case PDM_INVERT: oc=0x005b; break;
        case PDM_FILL:   oc=0x005c; break;
    }
    *pPict << oc;
}


void PictWriter::WriteOpcode_Arc(PictDrawingMethod eMethod, const Rectangle & rRect,
                                 const Point & rStartPt, const Point & rEndPt)
{
    USHORT oc;
    switch (eMethod) {
        case PDM_FRAME:  oc=0x0060; break;
        case PDM_PAINT:  oc=0x0061; break;
        case PDM_ERASE:  oc=0x0062; break;
        case PDM_INVERT: oc=0x0063; break;
        case PDM_FILL:   oc=0x0064; break;
    }
    *pPict << oc;
    WriteRectangle(rRect);
    WriteArcAngles(rRect,rStartPt,rEndPt);
}


void PictWriter::WriteOpcode_SameArc(PictDrawingMethod eMethod, const Rectangle & rRect,
                                     const Point & rStartPt, const Point & rEndPt)
{
    USHORT oc;
    switch (eMethod) {
        case PDM_FRAME:  oc=0x0068; break;
        case PDM_PAINT:  oc=0x0069; break;
        case PDM_ERASE:  oc=0x006a; break;
        case PDM_INVERT: oc=0x006b; break;
        case PDM_FILL:   oc=0x006c; break;
    }
    *pPict << oc;
    WriteArcAngles(rRect,rStartPt,rEndPt);
}


void PictWriter::WriteOpcode_Poly(PictDrawingMethod eMethod, const Polygon & rPoly)
{
    USHORT oc;

    if (rPoly.GetSize()<3) return;
    switch (eMethod) {
        case PDM_FRAME:  oc=0x0070; break;
        case PDM_PAINT:  oc=0x0071; break;
        case PDM_ERASE:  oc=0x0072; break;
        case PDM_INVERT: oc=0x0073; break;
        case PDM_FILL:   oc=0x0074; break;
    }
    *pPict << oc;
    WritePolygon(rPoly);
}


void PictWriter::WriteOpcode_BitsRect(const Point & rPoint, const Size & rSize, const Bitmap & rBitmap)
{
    BitmapReadAccess*   pAcc = NULL;
    Bitmap              aBitmap( rBitmap );

    ULONG   nWidth, nHeight, nDstRowBytes, nx, nc, ny, nCount, nColTabSize, i;
    ULONG   nDstRowPos, nSrcRowBytes, nEqu3, nPos, nDstMapPos;
    USHORT  nBitsPerPixel, nPackType;
    BYTE    *pComp[4], *pPix, *pTemp, nEquData, nFlagCounterByte, nRed, nGreen, nBlue;

    SetAttrForPaint();

    // temopraere Windows-BMP-Datei erzeugen:
    nActBitmapPercent=30;
    MayCallback();

    if ( bStatus == FALSE )
        return;
    if ( ( pAcc = aBitmap.AcquireReadAccess() ) == NULL )
        return;

    nBitsPerPixel = aBitmap.GetBitCount();
    nWidth = pAcc->Width();
    nHeight = pAcc->Height();

    // Wenn 24-Bit, dann den Opcode 'DirectBitsRect' erzeugen:
    if ( nBitsPerPixel == 24 )
    {

        // Anzahl Bytes einer (ungepackten) Zeile in Quelle und Ziel berechnen:
        nSrcRowBytes =( ( 3 * nWidth ) + 0x0003 ) & 0xfffc;
        nDstRowBytes = nWidth * 4;

        // Opcode und BaseAddr (?) schreiben:
        *pPict << (USHORT)0x009a << (ULONG)0x000000ff;

        // Normalerweise wollen wir den Packing-Type 4 (Run length encoding
        // for 32-Bit Pixels) erzeugen. Wenn aber RowBytes<8 gilt, sind die Daten
        // grundsaetzlich ungepackt, auch wenn der Packing-Type 4 angegeben ist,
        // was etwas komisch erscheint. Daher wollen wir in so einem Fall lieber
        // gleich den Packing-Type 1 (ungepackt) angeben:

        if ( nDstRowBytes < 8 )
            nPackType = 1;
        else
            nPackType = 4;

        // PixMap-Struktur schreiben:
        *pPict << (USHORT)(nDstRowBytes|0x8000) // Bytes pro Zeile und dass es eine 'PixMap' ist
               << (USHORT)0x0000                // Y1-Position der Bitmap in der Quelle
               << (USHORT)0x0000                // X1-Position der Bitmap in der Quelle
               << (USHORT)nHeight               // Y2-Position der Bitmap in der Quelle
               << (USHORT)nWidth                // X2-Position der Bitmap in der Quelle
               << (USHORT)0x0000                // Version
               << (USHORT)nPackType             // Packing type
               << (ULONG) 0x00000000            // Packing size (?)
               << (ULONG) 0x00480000            // H-Res
               << (ULONG) 0x00480000            // V-Res
               << (USHORT)0x0010                // Pixel type (?)
               << (USHORT)0x0020                // Pixel size: 32 bit
               << (USHORT)0x0004                // CmpCount: 4 Komponenten
               << (USHORT)0x0008                // CmpSize: 8 Bits
               << (ULONG) 0x00000000            // PlaneBytes (?)
               << (ULONG) 0x00000000            // (?)
               << (ULONG) 0x00000000;           // (?)

        // Source-Rectangle schreiben:
        *pPict << (USHORT)0x0000                // Y1-Position auf der Bitmap
               << (USHORT)0x0000                // X1-Position auf der Bitmap
               << (USHORT)nHeight               // Y2-Position auf der Bitmap
               << (USHORT)nWidth;               // X2-Position auf der Bitmap

        // Destination-Rectangle schreiben:
        WritePoint( rPoint );
        WritePoint( Point( rPoint.X() + rSize.Width(), rPoint.Y() + rSize.Height() ) );

        // Transfer mode schreiben:
        *pPict << (USHORT)0x0000; // (?)

        // Position der Map-Daten in Ziel merken:
        nDstMapPos=pPict->Tell();

        if ( nPackType == 1 )               // bei 24 bits nWidth == 1 !!
        {                                   // nicht packen
            for ( ny = 0; ny < nHeight; ny++ )
            {
                *pPict << (BYTE)0;
                *pPict << (BYTE)pAcc->GetPixel( ny, 0 ).GetRed();
                *pPict << (BYTE)pAcc->GetPixel( ny, 0 ).GetGreen();
                *pPict << (BYTE)pAcc->GetPixel( ny, 0 ).GetBlue();
                // Prozente zaehlen, Callback, Fehler pruefen:
                nActBitmapPercent = ( ny * 70 / nHeight ) + 30; // (30% machten schon das Schreiben der Win-BMP-Datei aus)
                MayCallback();
            }
        }
        else    // packen ( PackType == 4 )
        {
            // Speicher fuer Zeilen-Zwischen-Datenstruktur allozieren:
            for ( nc = 0; nc < 4; nc++ )
                pComp[ nc ] = new BYTE[ nWidth ];

            // Schleife ueber Zeilen:
            for ( ny = 0; ny < nHeight; ny++ )
            {
                // Zeil ny der Quelle in die Zwischen-Datenstrktur einlesen:

                for ( nx = 0; nx < nWidth; nx++ )
                {
                    pComp[ 1 ][ nx ] = (BYTE)pAcc->GetPixel( ny, nx ) .GetRed();
                    pComp[ 2 ][ nx ] = (BYTE)pAcc->GetPixel( ny, nx ) .GetGreen();
                    pComp[ 3 ][ nx ] = (BYTE)pAcc->GetPixel( ny, nx ) .GetBlue();
                    pComp[ 0 ][ nx ] = 0;
                }

                // Anfang der Zeile im Ziel merken:
                nDstRowPos = pPict->Tell();

                // ByteCount (das ist die Groesse der gepackten Zeile) zunaechst 0 (wird spaeter berichtigt):
                if ( nDstRowBytes > 250 )
                    *pPict << (USHORT)0;
                else
                    *pPict << (BYTE)0;

                // Schleife ueber Componenten:
                for ( nc = 0; nc < 4; nc++ )
                {
                    // Schleife ueber x:
                    nx = 0;
                    while ( nx < nWidth )
                    {
                        // Die Position von 3 gleichen Bytes suchen und in nEqu3 merken.
                        // wenn nicht gefunden, dann nEqu3=nWidth setzten.
                        // Wenn doch gefunden, dann in nEquData den Wert der Bytes merken.
                        nEqu3 = nx;
                        for (;;)
                        {
                            if ( nEqu3 + 2 >= nWidth )
                            {
                                nEqu3 = nWidth;
                                break;
                            }
                            nEquData = pComp[nc][nEqu3];
                            if ( nEquData == pComp[nc][nEqu3+1] && nEquData==pComp[nc][nEqu3+2] )
                                break;
                            nEqu3++;
                        }

                        // Die Daten von nx bis nEqu3 unkomprimiert schreiben (ggf. in mehreren Records):
                        while ( nEqu3 > nx )
                        {
                            nCount = nEqu3 - nx;
                            if ( nCount > 128 )
                                nCount=128;
                            nFlagCounterByte = (BYTE)(nCount-1);
                            *pPict << nFlagCounterByte;
                            do
                            {
                                *pPict << pComp[nc][nx++];
                                nCount--;
                            }
                            while ( nCount > 0 );
                        }

                        // Jetzt einen Komprimierungs-Record erzeugen (falls oben mindestens 3
                        // gleiche Bytes gefunden):
                        if ( nx < nWidth )
                        {               // Hinweis: es gilt nx==nEqu3 (hoffentlich)
                            nCount=3;   // Drei Bytes sind gleich, wie weiter oben herausgefunden.
                                        // Pruefen, ob es weitere gleiche Bytes gibts (dabei Max.-Record-Groesse beachten):
                            while ( nx + nCount < nWidth && nCount < 128 )
                            {
                                if ( nEquData != pComp[ nc ][ nx + nCount ] )
                                    break;
                                nCount++;
                            }
                            // nCount gleiche Bytes komprimiert schreiben:
                            nFlagCounterByte = (BYTE)( 1 - (long)nCount );
                            *pPict << nFlagCounterByte << nEquData;
                            nx += nCount;
                        }
                    }
                }
                // ByteCount berichtigen:
                nPos = pPict->Tell();
                pPict->Seek( nDstRowPos );
                if ( nDstRowBytes > 250 )
                    *pPict << ( (USHORT)( nPos - nDstRowPos - 2 ) );
                else
                    *pPict << ( (BYTE)( nPos - nDstRowPos - 1 ) );
                pPict->Seek( nPos );

                // Prozente zaehlen, Callback, Fehler pruefen:
                nActBitmapPercent = ( ny * 70 / nHeight ) + 30; // (30% machten schon das Schreiben der Win-BMP-Datei aus)
                MayCallback();
            }
            // Aufraeumen:
            for ( nc = 0; nc < 4; nc++ )
                delete pComp[ nc ];
        }
    }
    else
    {   // nicht 24-Bit also Opcode 'PackBitsRect' erzeugen:

        // Bei 1-Bit-Bildern ignorieren manche Import-Filter die Palette und nehmen statt
        // dessen die Vorder- und Hintergrundfarbe:
        if ( nBitsPerPixel == 1 )
        {
            WriteOpcode_RGBBkCol( pAcc->GetPaletteColor( 0 ) );
            WriteOpcode_RGBFgCol( pAcc->GetPaletteColor( 1 ) );
        }
        else
        {
            WriteOpcode_RGBBkCol( Color( COL_BLACK ) );
            WriteOpcode_RGBFgCol( Color( COL_WHITE ) );
        }

        // Anzahl Bytes einer (ungepackten) Zeile in Ziel und Quelle berechnen:
        nDstRowBytes = ( nWidth * nBitsPerPixel + 7 ) >> 3;
        nSrcRowBytes = ( nDstRowBytes + 3 ) & 0xfffffffc;

        // Opcode schreiben:
        *pPict << (USHORT)0x0098;

        // Normalerweise wollen wir den Packing-Type 0 (default Packing) erzeugen.
        // Wenn aber RowBytes<8 gilt, sind die Daten grundsaetzlich ungepackt,
        // auch wenn der Packing-Type 0 angegeben ist, was etwas komisch erscheint.
        // Daher wollen wir in so einem Fall lieber gleich den Packing-Type 1 (ungepackt)
        // angeben:
        if ( nDstRowBytes < 8 )
            nPackType = 1;
        else
            nPackType = 0;

        // PixMap-Struktur schreiben:
        *pPict << (USHORT)(nDstRowBytes|0x8000) // Bytes pro Zeile und dass es eine 'PixMap' ist
               << (USHORT)0x0000                // Y1-Position der Bitmap in der Quelle
               << (USHORT)0x0000                // X1-Position der Bitmap in der Quelle
               << (USHORT)nHeight               // Y2-Position der Bitmap in der Quelle
               << (USHORT)nWidth                // X2-Position der Bitmap in der Quelle
               << (USHORT)0x0000                // Version
               << (USHORT)nPackType             // Packing type
               << (ULONG) 0x00000000            // Packing size (?)
               << (ULONG) 0x00480000            // H-Res
               << (ULONG) 0x00480000            // V-Res
               << (USHORT)0x0000                // Pixel type (?)
               << (USHORT)nBitsPerPixel         // Pixel size
               << (USHORT)0x0001                // CmpCount: 1 Komponente
               << (USHORT)nBitsPerPixel         // CmpSize
               << (ULONG) 0x00000000            // PlaneBytes (?)
               << (ULONG) 0x00000000            // (?)
               << (ULONG) 0x00000000;           // (?)

        // Palette lesen und schreiben:
        nColTabSize = pAcc->GetPaletteEntryCount();
        *pPict << (ULONG)0 << (USHORT)0x8000 << (USHORT)( nColTabSize - 1 );

        for ( i = 0; i < nColTabSize; i++ )
        {
            nRed = (BYTE)pAcc->GetPaletteColor( (USHORT)i ).GetRed();
            nGreen = (BYTE)pAcc->GetPaletteColor( (USHORT)i ).GetGreen();
            nBlue = (BYTE)pAcc->GetPaletteColor( (USHORT)i ).GetBlue();
            *pPict << (UINT16)0 << nRed << nRed << nGreen << nGreen << nBlue << nBlue;
        }

        // Source-Rectangle schreiben:
        *pPict << (USHORT)0 << (USHORT)0 << (USHORT)nHeight << (USHORT)nWidth;

        // Destination-Rectangle schreiben:
        WritePoint( rPoint );
        WritePoint( Point( rPoint.X() + rSize.Width(), rPoint.Y() + rSize.Height() ) );

        // Transfer mode schreiben:
        *pPict << (USHORT)0;            // (?)

        // Speicher fuer eine Zeile allozieren:
        pPix = new BYTE[ nSrcRowBytes ];

        // Position der Map-Daten in Ziel merken:
        nDstMapPos=pPict->Tell();

        // Schleife ueber Zeilen:
        for ( ny = 0; ny < nHeight; ny++ )
        {

            // Zeile ny der Quelle in den Zwischenspeicher einlesen:

            switch ( nBitsPerPixel )
            {
                case 1 :
                    for ( pTemp = pPix, i = 0; i < nSrcRowBytes; i++ )
                        *pTemp++ = (BYTE)0;
                    for ( i = 0; i < nWidth; i++ )
                        pPix[ ( i >> 3 ) ] |= (BYTE)( pAcc->GetPixel( ny, i ) & 1 ) << ( ( i & 7 ) ^ 7 );
                    break;
                case 4 :
                    for ( pTemp = pPix, i = 0; i < nSrcRowBytes; i++ )
                        *pTemp++ = (BYTE)0;
                    for ( i = 0; i < nWidth; i++ )
                        pPix[ ( i >> 1 ) ] |= (BYTE)( pAcc->GetPixel( ny, i ) & 15 ) << ( ( i & 1 ) << 2 ) ;
                    break;
                case 8 :
                    for ( i = 0; i < nWidth; i++ )
                        pPix[ i ] = (BYTE)pAcc->GetPixel( ny, i );
                    break;
            }

            if ( nPackType == 1 )
            {   // nicht packen
                pPict->Write( pPix, nDstRowBytes );
            }
            else
            {   // Packen (nPackType==0)

                // Anfang der Zeile im Ziel merken:
                nDstRowPos = pPict->Tell();

                // ByteCount (das ist die Groesse der gepackten Zeile) zunaechst 0 (wird spaeter berichtigt):
                if ( nDstRowBytes > 250 )
                    *pPict << (USHORT)0;
                else
                    *pPict << (BYTE)0;

                // Schleife ueber Bytes der Zeile:
                nx=0;
                while ( nx < nDstRowBytes && bStatus )
                {
                    // Die Position von 3 gleichen Bytes suchen und in nEqu3 merken.
                    // wenn nicht gefunden, dann nEqu3=nDstRowBytes setzten.
                    // Wenn doch gefunden, dann in nEquData den Wert der Bytes merken.
                    nEqu3 = nx;
                    for (;;)
                    {
                        if ( nEqu3 + 2 >= nDstRowBytes )
                        {
                            nEqu3 = nDstRowBytes;
                            break;
                        }
                        nEquData = pPix[ nEqu3 ];
                        if ( nEquData == pPix[ nEqu3 + 1 ] && nEquData == pPix[ nEqu3 + 2 ] )
                            break;
                        nEqu3++;
                    }

                    // Die Daten von nx bis nEqu3 unkomprimiert schreiben (ggf. in mehreren Records):
                    while ( nEqu3 > nx )
                    {
                        nCount = nEqu3 - nx;
                        if ( nCount > 128 )
                            nCount = 128;
                        nFlagCounterByte = (BYTE)( nCount - 1 );
                        *pPict << nFlagCounterByte;
                        do
                        {
                            *pPict << pPix[ nx++ ];
                            nCount--;
                        } while ( nCount > 0 );
                    }

                    // Jetzt einen Komprimierungs-Record erzeugen (falls oben mindestens 3
                    // gleiche Bytes gefunden):
                    if ( nx < nDstRowBytes )
                    {   // Hinweis: es gilt nx==nEqu3 (hoffentlich)
                        nCount = 3; // Drei Bytes sind gleich, wie weiter oben herausgefunden.
                        // Pruefen, ob es weitere gleiche Bytes gibts (dabei Max.-Record-Groesse beachten):
                        while ( nx + nCount < nDstRowBytes && nCount < 128 )
                        {
                            if ( nEquData != pPix[ nx + nCount ] )
                                break;
                            nCount++;
                        }
                        // nCount gleiche Bytes komprimiert schreiben:
                        nFlagCounterByte = (BYTE)( 1 - (long)nCount );
                        *pPict << nFlagCounterByte << nEquData;
                        nx += nCount;
                    }
                }

                // ByteCount berichtigen:
                nPos = pPict->Tell();
                pPict->Seek( nDstRowPos );
                if ( nDstRowBytes > 250 )
                    *pPict << ( (USHORT)( nPos - nDstRowPos - 2 ) );
                else
                    *pPict << ( (BYTE)( nPos - nDstRowPos - 1 ) );
                pPict->Seek( nPos );
            }

            // Prozente zaehlen, Callback, Fehler pruefen:
            nActBitmapPercent =( ny * 70 / nHeight ) + 30; // (30% machten schon das Schreiben der Win-BMP-Datei aus)
            MayCallback();
            if ( pPict->GetError() )
                bStatus = FALSE;
        }
        // Aufraeumen:
        delete pPix;
    }

    // Map-Daten muessen gerade Anzahl von Bytes sein:
    if ( ( ( pPict->Tell() - nDstMapPos ) & 1 ) != 0 )
        *pPict << (BYTE)0;

    // Bitmaps zaehlen:
    nWrittenBitmaps++;
    nActBitmapPercent = 0;
    if ( pAcc )
        aBitmap.ReleaseAccess( pAcc );
}

void PictWriter::WriteOpcode_EndOfFile()
{
    *pPict << (USHORT)0x00ff;
}


void PictWriter::SetAttrForPaint()
{
    WriteOpcode_PnMode(eSrcRasterOp);
    WriteOpcode_RGBFgCol(aFillColor);
    WriteOpcode_RGBBkCol(aFillColor);
    WriteOpcode_PnFillPat(aFillColor!=Color( COL_TRANSPARENT ));
}


void PictWriter::SetAttrForFrame()
{
    WriteOpcode_PnMode(eSrcRasterOp);
    WriteOpcode_PnSize(0);
    WriteOpcode_RGBFgCol(aLineColor);
    WriteOpcode_PnLinePat(aLineColor!=Color( COL_TRANSPARENT ));
}


void PictWriter::SetAttrForText()
{
    WriteOpcode_RGBFgCol(aSrcFont.GetColor());
    WriteOpcode_RGBBkCol(aSrcFont.GetFillColor());
    WriteOpcode_PnLinePat(TRUE);
    WriteOpcode_FontName(aSrcFont);
    WriteOpcode_TxSize((USHORT)(aSrcFont.GetSize().Height()));
    WriteOpcode_TxMode(eSrcRasterOp);
    WriteOpcode_TxFace(aSrcFont);
}


void PictWriter::WriteTextArray(Point & rPoint, const ByteString& rString, const long * pDXAry)
{
    USHORT i,nLen;
    sal_Char c;
    BOOL bDelta;
    Point aPt;

    if ( pDXAry == NULL )
        WriteOpcode_Text( rPoint, rString, FALSE );
    else
    {
        bDelta = FALSE;
        nLen = rString.Len();
        for ( i = 0; i < nLen; i++ )
        {
            c = rString.GetChar( i );
            if ( (unsigned char) c > 32 )
            {
                aPt = rPoint;
                if ( i > 0 )
                    aPt.X() += pDXAry[ i - 1 ];

                WriteOpcode_Text( aPt, ByteString( c ), bDelta );
                bDelta = TRUE;
            }
        }
    }
}


void PictWriter::WriteOpcodes( const GDIMetaFile & rMTF )
{
    ULONG nA, nACount;
    const MetaAction* pMA;

    if( !bStatus)
        return;

    nACount=rMTF.GetActionCount();

    for (nA=0; nA<nACount; nA++)
    {
        pMA = rMTF.GetAction(nA);

        switch (pMA->GetType())
        {
            case META_PIXEL_ACTION:
            {
                const MetaPixelAction* pA = (const MetaPixelAction*) pMA;
                WriteOpcode_PnMode(eSrcRasterOp);
                WriteOpcode_PnSize(1);
                WriteOpcode_RGBFgCol(pA->GetColor());
                WriteOpcode_PnLinePat(TRUE);
                WriteOpcode_Line(pA->GetPoint(),pA->GetPoint());
            }
            break;

            case META_POINT_ACTION:
            {
                const MetaPointAction* pA = (const MetaPointAction*) pMA;

                if( aLineColor != Color( COL_TRANSPARENT ) )
                {
                    SetAttrForFrame();
                    WriteOpcode_Line( pA->GetPoint(),pA->GetPoint() );
                }
            }
            break;

            case META_LINE_ACTION:
            {
                const MetaLineAction* pA = (const MetaLineAction*) pMA;

                if( aLineColor != Color( COL_TRANSPARENT ) )
                {
                    SetAttrForFrame();
                    WriteOpcode_Line( pA->GetStartPoint(),pA->GetEndPoint() );
                }
                break;
            }

            case META_RECT_ACTION:
            {
                const MetaRectAction* pA = (const MetaRectAction*) pMA;

                if (aFillColor!=Color( COL_TRANSPARENT ))
                {
                    SetAttrForPaint();
                    WriteOpcode_Rect(PDM_PAINT,pA->GetRect());
                    if (aLineColor!=Color( COL_TRANSPARENT ))
                    {
                        SetAttrForFrame();
                        WriteOpcode_SameRect(PDM_FRAME);
                    }
                }
                else if (aLineColor!=Color( COL_TRANSPARENT ))
                {
                    SetAttrForFrame();
                    WriteOpcode_Rect(PDM_FRAME,pA->GetRect());
                }
            }
            break;

            case META_ROUNDRECT_ACTION:
            {
                const MetaRoundRectAction* pA = (const MetaRoundRectAction*) pMA;

                WriteOpcode_OvSize( Size( pA->GetHorzRound(), pA->GetVertRound() ) );

                if (aFillColor!=Color( COL_TRANSPARENT ))
                {
                    SetAttrForPaint();
                    WriteOpcode_RRect(PDM_PAINT,pA->GetRect());
                    if (aLineColor!=Color( COL_TRANSPARENT ))
                    {
                        SetAttrForFrame();
                        WriteOpcode_SameRRect(PDM_FRAME);
                    }
                }
                else if (aLineColor!=Color( COL_TRANSPARENT ))
                {
                    SetAttrForFrame();
                    WriteOpcode_RRect(PDM_FRAME,pA->GetRect());
                }
            }
            break;

            case META_ELLIPSE_ACTION:
            {
                const MetaEllipseAction* pA = (const MetaEllipseAction*) pMA;

                if (aFillColor!=Color( COL_TRANSPARENT ))
                {
                    SetAttrForPaint();
                    WriteOpcode_Oval(PDM_PAINT,pA->GetRect());
                    if (aLineColor!=Color( COL_TRANSPARENT ))
                    {
                        SetAttrForFrame();
                        WriteOpcode_SameOval(PDM_FRAME);
                    }
                }
                else if (aLineColor!=Color( COL_TRANSPARENT ))
                {
                    SetAttrForFrame();
                    WriteOpcode_Oval(PDM_FRAME,pA->GetRect());
                }
            }
            break;

            case META_ARC_ACTION:
            {
                const MetaArcAction* pA = (const MetaArcAction*) pMA;

                if (aLineColor!=Color( COL_TRANSPARENT ))
                {
                    SetAttrForFrame();
                    WriteOpcode_Arc(PDM_FRAME,pA->GetRect(),pA->GetStartPoint(),pA->GetEndPoint());
                }
            }
            break;

            case META_PIE_ACTION:
            {
                const MetaPieAction* pA = (const MetaPieAction *) pMA;

                if (aFillColor!=Color( COL_TRANSPARENT ))
                {
                    SetAttrForPaint();
                    WriteOpcode_Arc(PDM_PAINT,pA->GetRect(),pA->GetStartPoint(),pA->GetEndPoint());

                    if (aLineColor!=Color( COL_TRANSPARENT ))
                    {
                        SetAttrForFrame();
                        WriteOpcode_SameArc(PDM_FRAME,pA->GetRect(),pA->GetStartPoint(),pA->GetEndPoint());
                    }
                }
                else if (aLineColor!=Color( COL_TRANSPARENT ))
                {
                    SetAttrForFrame();
                    WriteOpcode_Arc(PDM_FRAME,pA->GetRect(),pA->GetStartPoint(),pA->GetEndPoint());
                }

                if (aLineColor!=Color( COL_TRANSPARENT ))
                {
                    double fxc,fyc,fxr,fyr,fx1,fy1,fx2,fy2,l1,l2;

                    fxc=((double)(pA->GetRect().Left()+pA->GetRect().Right()))/2.0;
                    fyc=((double)(pA->GetRect().Top()+pA->GetRect().Bottom()))/2.0;
                    fxr=((double)pA->GetRect().GetWidth())/2.0;
                    fyr=((double)pA->GetRect().GetHeight())/2.0;
                    fx1=((double)pA->GetStartPoint().X())-fxc;
                    fy1=((double)pA->GetStartPoint().Y())-fyc;
                    fx2=((double)pA->GetEndPoint().X())-fxc;
                    fy2=((double)pA->GetEndPoint().Y())-fyc;
                    l1=sqrt(fx1*fx1+fy1*fy1);
                    l2=sqrt(fx2*fx2+fy2*fy2);

                    if (l1>0)
                    {
                        fx1=fx1/l1*fxr;
                        fy1=fy1/l1*fyr;
                    }

                    if (l2>0)
                    {
                        fx2=fx2/l2*fxr;
                        fy2=fy2/l2*fyr;
                    }
                    fx1+=fxc; fy1+=fyc; fx2+=fxc; fy2+=fyc;
                    WriteOpcode_Line(Point((long)(fx1+0.5),(long)(fy1+0.5)), Point((long)(fxc+0.5),(long)(fyc+0.5)));
                    WriteOpcode_LineFrom(Point((long)(fx2+0.5),(long)(fy2+0.5)));
                }
            }
            break;

            case META_CHORD_ACTION:
            {
//                DBG_ERROR( "Unsupported PICT-Action: META_CHORD_ACTION!" );
            }
            break;

            case META_POLYLINE_ACTION:
            {
                const MetaPolyLineAction* pA = (const MetaPolyLineAction*) pMA;

                if( aLineColor!=Color( COL_TRANSPARENT ) )
                {
                    const Polygon&  rPoly = pA->GetPolygon();
                    const USHORT    nSize = rPoly.GetSize();
                    Point           aLast;

                    if ( nSize )
                    {
                        SetAttrForFrame();
                        aLast = rPoly[0];

                        for ( USHORT i = 1; i < nSize; i++ )
                        {
                            WriteOpcode_Line( aLast, rPoly[i] );
                            aLast = rPoly[i];
                        }
                    }
                }
            }
            break;

            case META_POLYGON_ACTION:
            {
                const MetaPolygonAction* pA = (const MetaPolygonAction*) pMA;

                if (aFillColor!=Color( COL_TRANSPARENT ))
                {
                    SetAttrForPaint();
                    WriteOpcode_Poly(PDM_PAINT,pA->GetPolygon());
                }

                if (aLineColor!=Color( COL_TRANSPARENT ))
                {
                    SetAttrForFrame();
                    WriteOpcode_Poly(PDM_FRAME,pA->GetPolygon());
                }
            }
            break;

            case META_POLYPOLYGON_ACTION:
            {
                const MetaPolyPolygonAction* pA = (const MetaPolyPolygonAction*) pMA;

                if (aFillColor!=Color( COL_TRANSPARENT ))
                {
                    SetAttrForPaint();
                    WriteOpcode_Poly(PDM_PAINT,PolyPolygonToPolygon(pA->GetPolyPolygon()));
                }

                if (aLineColor!=Color( COL_TRANSPARENT ))
                {
                    USHORT nCount,i;
                    SetAttrForFrame();
                    nCount=pA->GetPolyPolygon().Count();
                    for (i=0; i<nCount; i++)
                        WriteOpcode_Poly(PDM_FRAME,pA->GetPolyPolygon().GetObject(i));
                }
            }
            break;

            case META_TEXT_ACTION:
            {
                const MetaTextAction*   pA = (const MetaTextAction*) pMA;
                Point                   aPt( pA->GetPoint() );

                if ( aSrcFont.GetAlign() != ALIGN_BASELINE )
                {
                    VirtualDevice aVirDev;

                    if (aSrcFont.GetAlign()==ALIGN_TOP)
                        aPt.Y()+=(long)aVirDev.GetFontMetric(aSrcFont).GetAscent();
                    else
                        aPt.Y()-=(long)aVirDev.GetFontMetric(aSrcFont).GetDescent();
                }

                SetAttrForText();
                String aStr( pA->GetText(),pA->GetIndex(),pA->GetLen() );
                WriteOpcode_Text( aPt, ByteString( aStr, RTL_TEXTENCODING_UTF8 ) ,FALSE );
            }
            break;

            case META_TEXTARRAY_ACTION:
            {
                const MetaTextArrayAction*  pA = (const MetaTextArrayAction*) pMA;
                Point                       aPt( pA->GetPoint() );

                if (aSrcFont.GetAlign()!=ALIGN_BASELINE)
                {
                    VirtualDevice aVirDev;

                    if (aSrcFont.GetAlign()==ALIGN_TOP)
                        aPt.Y()+=(long)aVirDev.GetFontMetric(aSrcFont).GetAscent();
                    else
                        aPt.Y()-=(long)aVirDev.GetFontMetric(aSrcFont).GetDescent();
                }
                SetAttrForText();
                String aStr( pA->GetText(),pA->GetIndex(),pA->GetLen() );
                WriteTextArray( aPt, ByteString( aStr, RTL_TEXTENCODING_UTF8 ), pA->GetDXArray() );
                break;
            }

            case META_STRETCHTEXT_ACTION:
            {
                const MetaStretchTextAction*    pA = (const MetaStretchTextAction*) pMA;
                Point                           aPt( pA->GetPoint() );
                String                          aStr( pA->GetText(),pA->GetIndex(),pA->GetLen() );
                VirtualDevice                   aVirDev;
                long*                           pDXAry = new long[ aStr.Len() ];
                sal_Int32                       nNormSize( aVirDev.GetTextArray( aStr,pDXAry ) );
                USHORT                          i;

                if (aSrcFont.GetAlign()!=ALIGN_BASELINE)
                {
                    if (aSrcFont.GetAlign()==ALIGN_TOP)
                        aPt.Y()+=(long)aVirDev.GetFontMetric(aSrcFont).GetAscent();
                    else
                        aPt.Y()-=(long)aVirDev.GetFontMetric(aSrcFont).GetDescent();
                }

                for ( i = 0; i < aStr.Len() - 1; i++ )
                    pDXAry[ i ] = pDXAry[ i ] * ( (long)pA->GetWidth() ) / nNormSize;

                SetAttrForText();
                WriteTextArray( aPt, ByteString( aStr, RTL_TEXTENCODING_UTF8 ), pDXAry );
                delete[] pDXAry;
            }
            break;

            case META_TEXTRECT_ACTION:
            {
//                DBG_ERROR( "Unsupported PICT-Action: META_TEXTRECT_ACTION!" );
            }
            break;

            case META_BMP_ACTION:
            {
                const MetaBmpAction* pA = (const MetaBmpAction*) pMA;
                WriteOpcode_BitsRect( pA->GetPoint(),pA->GetBitmap().GetSizePixel(), pA->GetBitmap() );
            }
            break;

            case META_BMPSCALE_ACTION:
            {
                const MetaBmpScaleAction* pA = (const MetaBmpScaleAction*) pMA;
                WriteOpcode_BitsRect( pA->GetPoint(), pA->GetSize(), pA->GetBitmap() );
            }
            break;

            case META_BMPSCALEPART_ACTION:
            {
                const MetaBmpScalePartAction*   pA = (const MetaBmpScalePartAction*) pMA;
                Bitmap                          aBmp( pA->GetBitmap() );

                aBmp.Crop( Rectangle( pA->GetSrcPoint(), pA->GetSrcSize() ) );
                WriteOpcode_BitsRect( pA->GetDestPoint(), pA->GetDestSize(), aBmp );
            }
            break;

            case META_BMPEX_ACTION:
            {
                const MetaBmpExAction*  pA = (const MetaBmpExAction*) pMA;
                const Bitmap            aBmp( Graphic( pA->GetBitmapEx() ).GetBitmap() );

                WriteOpcode_BitsRect( pA->GetPoint(), aBmp.GetSizePixel(), aBmp );
            }
            break;

            case META_BMPEXSCALE_ACTION:
            {
                const MetaBmpExScaleAction* pA = (const MetaBmpExScaleAction*) pMA;
                const Bitmap                aBmp( Graphic( pA->GetBitmapEx() ).GetBitmap() );

                WriteOpcode_BitsRect( pA->GetPoint(), pA->GetSize(), aBmp );
            }
            break;

            case META_BMPEXSCALEPART_ACTION:
            {
                const MetaBmpExScalePartAction* pA = (const MetaBmpExScalePartAction*) pMA;
                Bitmap                          aBmp( Graphic( pA->GetBitmapEx() ).GetBitmap() );

                aBmp.Crop( Rectangle( pA->GetSrcPoint(), pA->GetSrcSize() ) );
                WriteOpcode_BitsRect( pA->GetDestPoint(), pA->GetDestSize(), aBmp );
            }
            break;

            case META_EPS_ACTION :
            {
                const MetaEPSAction* pA = (const MetaEPSAction*)pMA;
                const GDIMetaFile aGDIMetaFile( pA->GetSubstitute() );

                INT32 nCount = aGDIMetaFile.GetActionCount();
                for ( INT32 i = 0; i < nCount; i++ )
                {
                    const MetaAction* pMetaAct = aGDIMetaFile.GetAction( i );
                    if ( pMetaAct->GetType() == META_BMPSCALE_ACTION )
                    {
                        const MetaBmpScaleAction* pBmpScaleAction = (const MetaBmpScaleAction*)pMetaAct;
                        WriteOpcode_BitsRect( pA->GetPoint(), pA->GetSize(), pBmpScaleAction->GetBitmap() );
                        break;
                    }
                }
            }
            break;

            case META_MASK_ACTION:
            case META_MASKSCALE_ACTION:
            case META_MASKSCALEPART_ACTION:
            {
//                DBG_ERROR( "Unsupported PICT-Action: META_MASK..._ACTION!" );
            }
            break;

            case META_GRADIENT_ACTION:
            {
                VirtualDevice               aVDev;
                GDIMetaFile                 aTmpMtf;
                const MetaGradientAction*   pA = (const MetaGradientAction*) pMA;

                aVDev.SetMapMode( aTargetMapMode );
                aVDev.AddGradientActions( pA->GetRect(), pA->GetGradient(), aTmpMtf );
                WriteOpcodes( aTmpMtf );
            }
            break;

            case META_HATCH_ACTION:
            {
                VirtualDevice           aVDev;
                GDIMetaFile             aTmpMtf;
                const MetaHatchAction*  pA = (const MetaHatchAction*) pMA;

                aVDev.SetMapMode( aTargetMapMode );
                aVDev.AddHatchActions( pA->GetPolyPolygon(), pA->GetHatch(), aTmpMtf );
                WriteOpcodes( aTmpMtf );
            }
            break;

            case META_WALLPAPER_ACTION:
            {
//                DBG_ERROR( "Unsupported PICT-Action: META_WALLPAPER_ACTION!" );
            }
            break;

            case META_CLIPREGION_ACTION:
            {
//                DBG_ERROR( "Unsupported PICT-Action: META_CLIPREGION_ACTION!" );
            }
            break;

            case META_ISECTRECTCLIPREGION_ACTION:
            {
//                DBG_ERROR( "Unsupported PICT-Action: META_ISECTRECTCLIPREGION_ACTION!" );
            }
            break;

            case META_ISECTREGIONCLIPREGION_ACTION:
            {
//                DBG_ERROR( "Unsupported PICT-Action: META_ISECTREGIONCLIPREGION_ACTION!" );
            }
            break;

            case META_MOVECLIPREGION_ACTION:
            {
//                DBG_ERROR( "Unsupported PICT-Action: META_MOVECLIPREGION_ACTION!" );
            }
            break;

            case META_LINECOLOR_ACTION:
            {
                const MetaLineColorAction* pA = (const MetaLineColorAction*) pMA;

                if( pA->IsSetting() )
                    aLineColor = pA->GetColor();
                else
                    aLineColor = Color( COL_TRANSPARENT );
            }
            break;

            case META_FILLCOLOR_ACTION:
            {
                const MetaFillColorAction* pA = (const MetaFillColorAction*) pMA;

                if( pA->IsSetting() )
                    aFillColor = pA->GetColor();
                else
                    aFillColor = Color( COL_TRANSPARENT );
            }
            break;

            case META_TEXTCOLOR_ACTION:
            {
                const MetaTextColorAction* pA = (const MetaTextColorAction*) pMA;
                aSrcFont.SetColor( pA->GetColor() );
            }
            break;

            case META_TEXTFILLCOLOR_ACTION:
            {
                const MetaTextFillColorAction* pA = (const MetaTextFillColorAction*) pMA;

                if( pA->IsSetting() )
                    aSrcFont.SetFillColor( pA->GetColor() );
                else
                    aSrcFont.SetFillColor( Color( COL_TRANSPARENT ) );
            }
            break;

            case META_TEXTALIGN_ACTION:
            {
//                DBG_ERROR( "Unsupported PICT-Action: META_TEXTALIGN_ACTION!" );
            }
            break;

            case META_MAPMODE_ACTION:
            {
                const MetaMapModeAction* pA = (const MetaMapModeAction*) pMA;

                if (aSrcMapMode!=pA->GetMapMode())
                {
                    if( pA->GetMapMode().GetMapUnit() == MAP_RELATIVE )
                    {
                        MapMode aMM = pA->GetMapMode();
                        Fraction aScaleX = aMM.GetScaleX();
                        Fraction aScaleY = aMM.GetScaleY();

                        Point aOrigin = aSrcMapMode.GetOrigin();
                        BigInt aX( aOrigin.X() );
                        aX *= BigInt( aScaleX.GetDenominator() );
                        if( aOrigin.X() >= 0 )
                        {
                            if( aScaleX.GetNumerator() >= 0 )
                                aX += BigInt( aScaleX.GetNumerator()/2 );
                            else
                                aX -= BigInt( (aScaleX.GetNumerator()+1)/2 );
                        }
                        else
                        {
                            if( aScaleX.GetNumerator() >= 0 )
                                aX -= BigInt( (aScaleX.GetNumerator()-1)/2 );
                            else
                                aX += BigInt( aScaleX.GetNumerator()/2 );
                        }

                        aX /= BigInt( aScaleX.GetNumerator() );
                        aOrigin.X() = (long)aX + aMM.GetOrigin().X();
                        BigInt aY( aOrigin.Y() );
                        aY *= BigInt( aScaleY.GetDenominator() );

                        if( aOrigin.Y() >= 0 )
                        {
                            if( aScaleY.GetNumerator() >= 0 )
                                aY += BigInt( aScaleY.GetNumerator()/2 );
                            else
                                aY -= BigInt( (aScaleY.GetNumerator()+1)/2 );
                        }
                        else
                        {
                            if( aScaleY.GetNumerator() >= 0 )
                                aY -= BigInt( (aScaleY.GetNumerator()-1)/2 );
                            else
                                aY += BigInt( aScaleY.GetNumerator()/2 );
                        }

                        aY /= BigInt( aScaleY.GetNumerator() );
                        aOrigin.Y() = (long)aY + aMM.GetOrigin().Y();
                        aSrcMapMode.SetOrigin( aOrigin );

                        aScaleX *= aSrcMapMode.GetScaleX();
                        aScaleY *= aSrcMapMode.GetScaleY();
                        aSrcMapMode.SetScaleX( aScaleX );
                        aSrcMapMode.SetScaleY( aScaleY );
                    }
                    else
                        aSrcMapMode = pA->GetMapMode();
                }
            }
            break;

            case META_FONT_ACTION:
            {
                const MetaFontAction* pA = (const MetaFontAction*) pMA;
                aSrcFont=pA->GetFont();
            }
            break;

            case META_PUSH_ACTION:
            {
                PictWriterAttrStackMember * pAt = new PictWriterAttrStackMember;
                pAt->aLineColor=aLineColor;
                pAt->aFillColor=aFillColor;
                pAt->eRasterOp=eSrcRasterOp;
                pAt->aFont=aSrcFont;
                pAt->aMapMode=aSrcMapMode;
                pAt->pSucc=pAttrStack;
                pAttrStack=pAt;
            }
            break;

            case META_POP_ACTION:
            {
                PictWriterAttrStackMember* pAt=pAttrStack;

                if( pAt )
                {
                    aLineColor=pAt->aLineColor;
                    aFillColor=pAt->aFillColor;
                    eSrcRasterOp=pAt->eRasterOp;
                    aSrcFont=pAt->aFont;
                    aSrcMapMode=pAt->aMapMode;
                    pAttrStack=pAt->pSucc;
                    delete pAt;
                }
            }
            break;

            case META_RASTEROP_ACTION:
            {
                const MetaRasterOpAction* pA = (const MetaRasterOpAction*) pMA;
                eSrcRasterOp=pA->GetRasterOp();
            }
            break;

            case META_TRANSPARENT_ACTION:
            {
                const PolyPolygon& rPolyPoly = ( (const MetaTransparentAction*) pMA )->GetPolyPolygon();

                if (aFillColor!=Color( COL_TRANSPARENT ))
                {
                    SetAttrForPaint();
                    WriteOpcode_Poly( PDM_PAINT, PolyPolygonToPolygon( rPolyPoly ) );
                }

                if (aLineColor!=Color( COL_TRANSPARENT ))
                {
                    SetAttrForFrame();
                    for( USHORT i = 0, nCount = rPolyPoly.Count(); i < nCount; i++ )
                        WriteOpcode_Poly( PDM_FRAME, rPolyPoly.GetObject( i ) );
                }
            }
            break;

            case META_FLOATTRANSPARENT_ACTION:
            {
                const MetaFloatTransparentAction* pA = (const MetaFloatTransparentAction*) pMA;

                GDIMetaFile     aTmpMtf( pA->GetGDIMetaFile() );
                Point           aSrcPt( aTmpMtf.GetPrefMapMode().GetOrigin() );
                const Size      aSrcSize( aTmpMtf.GetPrefSize() );
                const Point     aDestPt( pA->GetPoint() );
                const Size      aDestSize( pA->GetSize() );
                const double    fScaleX = aSrcSize.Width() ? (double) aDestSize.Width() / aSrcSize.Width() : 1.0;
                const double    fScaleY = aSrcSize.Height() ? (double) aDestSize.Height() / aSrcSize.Height() : 1.0;
                long            nMoveX, nMoveY;

                if( fScaleX != 1.0 || fScaleY != 1.0 )
                {
                    aTmpMtf.Scale( fScaleX, fScaleY );
                    aSrcPt.X() = FRound( aSrcPt.X() * fScaleX ), aSrcPt.Y() = FRound( aSrcPt.Y() * fScaleY );
                }

                nMoveX = aDestPt.X() - aSrcPt.X(), nMoveY = aDestPt.Y() - aSrcPt.Y();

                if( nMoveX || nMoveY )
                    aTmpMtf.Move( nMoveX, nMoveY );

                WriteOpcodes( aTmpMtf );
            }
            break;
        }

        nWrittenActions++;
        MayCallback();

        if (pPict->GetError())
        bStatus=FALSE;

        if (bStatus==FALSE)
        break;
    }
}


void PictWriter::WriteHeader(const GDIMetaFile & rMTF)
{
    USHORT  i;
    Size    aSize( OutputDevice::LogicToLogic( rMTF.GetPrefSize(),
                                               rMTF.GetPrefMapMode(),
                                               aTargetMapMode ) );
    short   nWidth = (short) ( aSize.Width() + 1L );
    short   nHeight = (short) ( aSize.Height() + 1L );

    // 512 Bytes "Muell" am Anfang:
    for (i=0;i<128;i++) *pPict << (ULONG)0;

    // Lo-16-Bits der Groesse der Datei ohne die 512 Bytes Muell:
    *pPict << (USHORT)0; // wird spaeter durch UpdateHeader() berichtigt

    // Das Bounding-Rectangle (y1,x1,y2,x2 !):
    *pPict << (short)0 << (short)0
           << nHeight << nWidth;

    // Version 2:
    *pPict << (ULONG)0x001102ff;

    // Extended-Version-2-Header:
    *pPict << (USHORT)0x0c00                            // Opcode
           << (USHORT)0xfffe                            // Version (?)
           << (USHORT)0x0000                            // Reserved
           << (ULONG) 0x00480000                        // hRes
           << (ULONG) 0x00480000                        // vRes
           << (USHORT)0x0000                            // SrcRect-Y1
           << (USHORT)0x0000                            // SrcRect-X1
           << nHeight                                   // SrcRect-Y2
           << nWidth                                    // SrcRect-X2
           << (ULONG)0x00000000;                        // Reserved

    // viele Import-Filter verlangen die Angabe eines
    // Clipping-Bereichs am Anfang
    *pPict << (USHORT)0x0001
           << (USHORT)0x000a
           << (USHORT)0x0000
           << (USHORT)0x0000
           << nHeight
           << nWidth;
}


void PictWriter::UpdateHeader()
{
    ULONG nPos;

    // Lo-16-Bits der Groesse der Datei ohne die 512 Bytes Muell berichtigen:
    nPos=pPict->Tell();
    pPict->Seek(512);
    *pPict << (USHORT)((nPos-512)&0x0000ffff);
    pPict->Seek(nPos);
}


BOOL PictWriter::WritePict(const GDIMetaFile & rMTF, SvStream & rTargetStream, PFilterCallback pcallback, void * pcallerdata)
{
    PictWriterAttrStackMember*  pAt;
    MapMode                     aMap72( MAP_INCH );
    Fraction                    aDPIFrac( 1, 72 );

    bStatus=TRUE;

    pCallback=pcallback;
    pCallerData=pcallerdata;
    nLastPercent=0;

    pPict=&rTargetStream;
    pPict->SetNumberFormatInt(NUMBERFORMAT_INT_BIGENDIAN);

    aLineColor=Color( COL_BLACK );
    aFillColor=Color( COL_WHITE );
    eSrcRasterOp=ROP_OVERPAINT;
    aSrcFont=Font();
    aSrcMapMode = rMTF.GetPrefMapMode();

    aMap72.SetScaleX( aDPIFrac );
    aMap72.SetScaleY( aDPIFrac );
    aTargetMapMode = aMap72;

    pAttrStack=NULL;

    bDstBkPatValid=FALSE;
    bDstTxFaceValid=FALSE;
    bDstTxModeValid=FALSE;
    bDstPnSizeValid=FALSE;
    bDstPnModeValid=FALSE;
    bDstPnPatValid=FALSE;
    bDstFillPatValid=FALSE;
    bDstTxSizeValid=FALSE;
    bDstFgColValid=FALSE;
    bDstBkColValid=FALSE;
    bDstPenPositionValid=FALSE;
    bDstTextPositionValid=FALSE;
    bDstFontNameValid=FALSE;

    nNumberOfActions=0;
    nNumberOfBitmaps=0;
    nWrittenActions=0;
    nWrittenBitmaps=0;
    nActBitmapPercent=0;

    CountActionsAndBitmaps(rMTF);

    WriteHeader(rMTF);
    WriteOpcodes(rMTF);
    WriteOpcode_EndOfFile();
    UpdateHeader();

    while (pAttrStack!=NULL) {
        pAt=pAttrStack;
        pAttrStack=pAt->pSucc;
        delete pAt;
    }

    return bStatus;
}

//================== GraphicExport - die exportierte Funktion ================

#ifdef WNT
extern "C" BOOL _cdecl GraphicExport(SvStream & rStream, Graphic & rGraphic,
                              PFilterCallback pCallback, void * pCallerData,
                              Config *, BOOL)
#else
extern "C" BOOL GraphicExport(SvStream & rStream, Graphic & rGraphic,
                              PFilterCallback pCallback, void * pCallerData,
                              Config *, BOOL)
#endif
{
    PictWriter      aPictWriter;

    if (rGraphic.GetType()==GRAPHIC_GDIMETAFILE)
    {
        GDIMetaFile aScaledMtf( rGraphic.GetGDIMetaFile() );

/*
        MapMode     aMap72( MAP_INCH );
        Fraction    aDPIFrac( 1, 72 );
        Size        aOldSize = aScaledMtf.GetPrefSize();

        aMap72.SetScaleX( aDPIFrac );
        aMap72.SetScaleY( aDPIFrac );

        Size aNewSize = OutputDevice::LogicToLogic( aOldSize,
                                                    aScaledMtf.GetPrefMapMode(),
                                                    aMap72 );

        aScaledMtf.Scale( Fraction( aNewSize.Width(), aOldSize.Width() ),
                          Fraction( aNewSize.Height(), aOldSize.Height() ) );
        aScaledMtf.SetPrefMapMode( aMap72 );
        aScaledMtf.SetPrefSize( aNewSize );
*/

        return aPictWriter.WritePict( aScaledMtf, rStream, pCallback, pCallerData );
    }
    else
    {
        Bitmap aBmp=rGraphic.GetBitmap();
        GDIMetaFile aMTF;
        VirtualDevice aVirDev;

        aMTF.Record(&aVirDev);
        aVirDev.DrawBitmap(Point(),aBmp);
        aMTF.Stop();
        aMTF.SetPrefSize(aBmp.GetSizePixel());
        return aPictWriter.WritePict(aMTF,rStream,pCallback,pCallerData);
    }
}

//================== GraphicDialog - die exportierte Funktion ================

extern "C" BOOL SAL_CALL DoExportDialog( FltCallDialogParameter& rPara )
{
    BOOL    bRet = FALSE;

    if ( rPara.pWindow && rPara.pCfg )
    {
        ByteString  aResMgrName( "ept" );
        ResMgr* pResMgr;

        aResMgrName.Append( ByteString::CreateFromInt32( SOLARUPD ) );
        pResMgr = ResMgr::CreateResMgr( aResMgrName.GetBuffer(), Application::GetAppInternational().GetLanguage() );

        if( pResMgr )
        {
            rPara.pResMgr = pResMgr;
            bRet = ( DlgExportEPCT( rPara ).Execute() == RET_OK );
            delete pResMgr;
        }
        else
            bRet = TRUE;
    }

    return bRet;
}


//=============================== fuer Windows ==============================

#pragma hdrstop

#ifdef WIN

static HINSTANCE hDLLInst = 0;      // HANDLE der DLL

extern "C" int CALLBACK LibMain( HINSTANCE hDLL, WORD, WORD nHeap, LPSTR )
{
#ifndef WNT
    if ( nHeap )
        UnlockData( 0 );
#endif

    hDLLInst = hDLL;

    return TRUE;
}

extern "C" int CALLBACK WEP( int )
{
    return 1;
}

#endif

