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


#include <vcl/metaact.hxx>
#include <svtools/filter.hxx>
#include <svl/solar.hrc>
#include <svtools/fltcall.hxx>

#include <math.h>
#include <vcl/bmpacc.hxx>
#include <vcl/graph.hxx>
#include <vcl/gradient.hxx>
#include <vcl/hatch.hxx>
#include <vcl/metric.hxx>
#include <vcl/font.hxx>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/rendergraphicrasterizer.hxx>

#include <tools/bigint.hxx>

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>

//============================== PictWriter ===================================

struct PictWriterAttrStackMember {
    struct PictWriterAttrStackMember *  pSucc;
    Color                               aLineColor;
    Color                               aFillColor;
    RasterOp                            eRasterOp;
    Font                                aFont;
    MapMode                             aMapMode;
    Rectangle                           aClipRect;
};


enum PictDrawingMethod {
    PDM_FRAME, PDM_PAINT, PDM_ERASE, PDM_INVERT, PDM_FILL
};


struct PictPattern {
    sal_uInt32 nLo, nHi;
};

class PictWriter {

private:

    sal_Bool bStatus;
    sal_uLong nLastPercent; // with wich number pCallback has been called the last time
    com::sun::star::uno::Reference< com::sun::star::task::XStatusIndicator > xStatusIndicator;

    SvStream * pPict;

    // current attributes in the source-metafile:
    Color       aLineColor;
    Color       aFillColor;
    RasterOp    eSrcRasterOp;
    Font        aSrcFont;
    MapMode     aSrcMapMode;
    MapMode     aTargetMapMode;
    Rectangle   aClipRect;
    PictWriterAttrStackMember * pAttrStack;

    // current attributes in the target-metafile and whether they are valid
    sal_Bool bDstBkPatValid;
    sal_uInt8        nDstTxFace;            sal_Bool bDstTxFaceValid;
    RasterOp    eDstTxMode;         sal_Bool bDstTxModeValid;
    sal_uInt16      nDstPnSize;         sal_Bool bDstPnSizeValid;
    RasterOp    eDstPnMode;         sal_Bool bDstPnModeValid;
    PictPattern aDstPnPat;          sal_Bool bDstPnPatValid;
    sal_Bool bDstFillPatValid;
    sal_uInt16      nDstTxSize;         sal_Bool bDstTxSizeValid;
    Color       aDstFgCol;          sal_Bool bDstFgColValid;
    Color       aDstBkCol;          sal_Bool bDstBkColValid;
    Point       aDstPenPosition;    sal_Bool bDstPenPositionValid;
    Point       aDstTextPosition;   sal_Bool bDstTextPositionValid;
    String      aDstFontName; sal_uInt16 nDstFontNameId; sal_Bool bDstFontNameValid;

    sal_uLong nNumberOfActions;  // number of actions in the GDIMetafile
    sal_uLong nNumberOfBitmaps;  // number of bitmaps
    sal_uLong nWrittenActions;   // number of already processed actions during writing the Opcodes
    sal_uLong nWrittenBitmaps;   // number of already written Bitmaps
    sal_uLong nActBitmapPercent; // what percentage of the next bitmap is already written

    void MayCallback();
        // calculates a percentage on the basis of the 5 parameters above
        // and then does a Callback should the situation arise. Sets bStatus to sal_False
        // if the user wants to cancel

    void CountActionsAndBitmaps(const GDIMetaFile & rMTF);
        // counts the bitmaps and actions (nNumberOfActions and nNumberOfBitmaps
        // have to be set to 0 at the beginning, since this method is recursive)

    Polygon PolyPolygonToPolygon(const PolyPolygon & rPoly);
        // generates a relatively sane polygon on the basis of a PolyPolygon

    Rectangle MapRectangle( const Rectangle& rRect );
    void WritePoint(const Point & rPoint);
    void WriteSize(const Size & rSize);
    void WriteRGBColor(const Color & rColor);
    void WriteString( const String & rString );
    void WriteRectangle(const Rectangle & rRect);
    void WritePolygon(const Polygon & rPoly);
    void WriteArcAngles(const Rectangle & rRect, const Point & rStartPt, const Point & rEndPt);

    void ConvertLinePattern(PictPattern & rPat, sal_Bool bVisible) const;
    void ConvertFillPattern(PictPattern & rPat, sal_Bool bVisible) const;

    void WriteOpcode_TxFace(const Font & rFont);
    void WriteOpcode_TxMode(RasterOp eMode);
    void WriteOpcode_PnSize(sal_uInt16 nSize);
    void WriteOpcode_PnMode(RasterOp eMode);
    void WriteOpcode_PnLinePat(sal_Bool bVisible);
    void WriteOpcode_PnFillPat(sal_Bool bVisible);
    void WriteOpcode_OvSize(const Size & rSize);
    void WriteOpcode_TxSize(sal_uInt16 nSize);
    void WriteOpcode_RGBFgCol(const Color & rColor);
    void WriteOpcode_RGBBkCol(const Color & rColor);
    void WriteOpcode_Line(const Point & rLocPt, const Point & rNewPt);
    void WriteOpcode_LineFrom(const Point & rNewPt);
    void WriteOpcode_Text(const Point & rPoint, const String& rString, sal_Bool bDelta);
    void WriteOpcode_FontName(const Font & rFont);
    void WriteOpcode_ClipRect( const Rectangle& rRect );
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

    void WriteTextArray(Point & rPoint, const String& rString, const sal_Int32 * pDXAry);

    void HandleLineInfoPolyPolygons(const LineInfo& rInfo, const basegfx::B2DPolygon& rLinePolygon);
    void WriteOpcodes(const GDIMetaFile & rMTF);

    void WriteHeader(const GDIMetaFile & rMTF);
    void UpdateHeader();

public:

    sal_Bool WritePict( const GDIMetaFile & rMTF, SvStream & rTargetStream, FilterConfigItem* pFilterConfigItem );
};


//========================== Methods of PictWriter ==========================


void PictWriter::MayCallback()
{
    if ( xStatusIndicator.is() )
    {
        sal_uLong nPercent;
        nPercent=((nWrittenBitmaps<<14)+(nActBitmapPercent<<14)/100+nWrittenActions)
                *100
                /((nNumberOfBitmaps<<14)+nNumberOfActions);

        if (nPercent>=nLastPercent+3)
        {
            nLastPercent=nPercent;
            if( nPercent<=100 )
                xStatusIndicator->setValue( nPercent );
        }
    }
}

void PictWriter::CountActionsAndBitmaps(const GDIMetaFile & rMTF)
{
    size_t              nAction, nActionCount;
    const MetaAction*   pMA;

    nActionCount = rMTF.GetActionSize();

    for (nAction=0; nAction < nActionCount; nAction++)
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
            case META_RENDERGRAPHIC_ACTION:
                nNumberOfBitmaps++;
            break;
        }

        nNumberOfActions++;
    }
}


Polygon PictWriter::PolyPolygonToPolygon(const PolyPolygon & rPolyPoly)
{
    sal_uInt16 nCount,nSize1,nSize2,np,i1,i2,i3,nBestIdx1,nBestIdx2;
    long nDistSqr,nBestDistSqr, nCountdownTests;
    Point aP1,aPRel;
    Polygon aPoly1, aPoly2, aPoly3;

    nCount=rPolyPoly.Count();
    if (nCount==0) return Polygon(0);

    aPoly1=rPolyPoly.GetObject(0);
    for (np=1; np<nCount; np++) {
        aPoly2=rPolyPoly.GetObject(np);

        //-----------------The following code merges aPoly1 and aPoly2 to aPoly1-----------------

        nSize1=aPoly1.GetSize();
        nSize2=aPoly2.GetSize();

        // At first we look for a point in aPoly1 (referenced by nBestIdx1) and a
        // point in aPoly2 (referenced by nBestid2), which 
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

        // Now aPoly1 and aPoly2 are being merged to a polygon aPoly3 (later aPoly1)
        // Both polygons are being connected by two additional edges between the points found above
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
    OutputDevice::LogicToLogic( rSize, aSrcMapMode, aTargetMapMode ); // -Wall is this needed.
    *pPict << ((short)rSize.Height()) << ((short)rSize.Width());
}


void PictWriter::WriteRGBColor(const Color & rColor)
{
    const sal_uInt16 nR = ( (sal_uInt16) rColor.GetRed() << 8 ) | (sal_uInt16) rColor.GetRed();
    const sal_uInt16 nG = ( (sal_uInt16) rColor.GetGreen() << 8 ) | (sal_uInt16) rColor.GetGreen();
    const sal_uInt16 nB = ( (sal_uInt16) rColor.GetBlue() << 8 ) | (sal_uInt16) rColor.GetBlue();

    *pPict << nR << nG << nB;
}

void PictWriter::WriteString( const String & rString )
{
    rtl::OString aString(rtl::OUStringToOString(rString, osl_getThreadTextEncoding()));
    sal_Int32 nLen = aString.getLength();
    if ( nLen > 255 )
        nLen = 255;
    *pPict << ( (sal_uInt8)nLen );
    for (sal_Int32 i = 0; i < nLen; ++i)
        *pPict << aString[i];
}

Rectangle PictWriter::MapRectangle( const Rectangle& rRect )
{
    Point   aPoint = OutputDevice::LogicToLogic( rRect.TopLeft(), aSrcMapMode, aTargetMapMode );
    Size    aSize = OutputDevice::LogicToLogic( rRect.GetSize(), aSrcMapMode, aTargetMapMode );
    Rectangle aRect( aPoint, aSize );
    aRect.Justify();
    aRect.nBottom++;
    aRect.nRight++;
    return aRect;
}

void PictWriter::WriteRectangle(const Rectangle & rRect)
{
    Rectangle aRect( MapRectangle( rRect ) );
    *pPict  << (sal_Int16)aRect.Top() << (sal_Int16)aRect.Left()
            << (sal_Int16)aRect.Bottom() << (sal_Int16)aRect.Right();
}

void PictWriter::WritePolygon(const Polygon & rPoly)
{
    sal_uInt16 nDataSize,i,nSize;
    short nMinX = 0, nMinY = 0, nMaxX = 0, nMaxY = 0;
    short nx,ny;
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


void PictWriter::ConvertLinePattern(PictPattern & rPat, sal_Bool bVisible) const
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

void PictWriter::ConvertFillPattern(PictPattern & rPat, sal_Bool bVisible) const
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


void PictWriter::WriteOpcode_TxFace(const Font & rFont)
{
    sal_uInt8 nFace;
    FontWeight eWeight;

    nFace=0;
    eWeight=rFont.GetWeight();
    if (eWeight==WEIGHT_BOLD ||
        eWeight==WEIGHT_SEMIBOLD ||
        eWeight==WEIGHT_ULTRABOLD ||
        eWeight==WEIGHT_BLACK)                nFace|=0x01;
    if (rFont.GetItalic()!=ITALIC_NONE)       nFace|=0x02;
    if (rFont.GetUnderline()!=UNDERLINE_NONE) nFace|=0x04;
    if (rFont.IsOutline()==sal_True)              nFace|=0x08;
    if (rFont.IsShadow()==sal_True)               nFace|=0x10;

    if (bDstTxFaceValid==sal_False || nDstTxFace!=nFace) {
        *pPict << (sal_uInt16)0x0004 << nFace << (sal_uInt8)0;
        nDstTxFace=nFace;
        bDstTxFaceValid=sal_True;
    }
}


void PictWriter::WriteOpcode_TxMode(RasterOp eMode)
{
    sal_uInt16 nVal;

    if (bDstTxModeValid==sal_False || eDstTxMode!=eMode) {
        switch (eMode) {
            case ROP_INVERT: nVal=0x000c; break;
            case ROP_XOR:    nVal=0x000a; break;
            default:         nVal=0x0008;
        }
        *pPict << (sal_uInt16)0x0005 << nVal;
        eDstTxMode=eMode;
        bDstTxModeValid=sal_True;
    }
}


void PictWriter::WriteOpcode_PnSize(sal_uInt16 nSize)
{
    if (nSize==0) nSize=1;
    if (bDstPnSizeValid==sal_False || nDstPnSize!=nSize) {
        *pPict << (sal_uInt16)0x0007 << nSize << nSize;
        nDstPnSize=nSize;
        bDstPnSizeValid=sal_True;
    }
}


void PictWriter::WriteOpcode_PnMode(RasterOp eMode)
{
    sal_uInt16 nVal;

    if (bDstPnModeValid==sal_False || eDstPnMode!=eMode) {
        switch (eMode)
        {
            case ROP_INVERT: nVal=0x000c; break;
            case ROP_XOR:    nVal=0x000a; break;
            default:         nVal=0x0008;
        }
        *pPict << (sal_uInt16)0x0008 << nVal;
        eDstPnMode=eMode;
        bDstPnModeValid=sal_True;
    }
}


void PictWriter::WriteOpcode_PnLinePat(sal_Bool bVisible)
{
    PictPattern aPat;

    ConvertLinePattern(aPat,bVisible);
    if (bDstPnPatValid==sal_False || aDstPnPat.nHi!=aPat.nHi || aDstPnPat.nLo!=aPat.nLo) {
        *pPict << (sal_uInt16)0x0009 << aPat.nHi << aPat.nLo;
        aDstPnPat=aPat;
        bDstPnPatValid=sal_True;
    }
}


void PictWriter::WriteOpcode_PnFillPat(sal_Bool bVisible)
{
    PictPattern aPat;

    ConvertFillPattern(aPat,bVisible);
    if (bDstPnPatValid==sal_False || aDstPnPat.nHi!=aPat.nHi || aDstPnPat.nLo!=aPat.nLo) {
        *pPict << (sal_uInt16)0x0009 << aPat.nHi << aPat.nLo;
        aDstPnPat=aPat;
        bDstPnPatValid=sal_True;
    }
}


void PictWriter::WriteOpcode_OvSize(const Size & rSize)
{
    *pPict << (sal_uInt16)0x000b;
    WriteSize(rSize);
}


void PictWriter::WriteOpcode_TxSize(sal_uInt16 nSize)
{
    if (bDstTxSizeValid==sal_False || nDstTxSize!=nSize) {

        nDstTxSize = (sal_uInt16) OutputDevice::LogicToLogic( Size( 0, nSize ),
                                                          aSrcMapMode, aTargetMapMode ).Height();

        *pPict << (sal_uInt16)0x000d << nDstTxSize;
        bDstTxSizeValid=sal_True;
    }
}


void PictWriter::WriteOpcode_RGBFgCol(const Color & rColor)
{
    if (bDstFgColValid==sal_False || aDstFgCol!=rColor) {
        *pPict << (sal_uInt16)0x001a;
        WriteRGBColor(rColor);
        aDstFgCol=rColor;
        bDstFgColValid=sal_True;
    }
}


void PictWriter::WriteOpcode_RGBBkCol(const Color & rColor)
{
    if (bDstBkColValid==sal_False || aDstBkCol!=rColor) {
        *pPict << (sal_uInt16)0x001b;
        WriteRGBColor(rColor);
        aDstBkCol=rColor;
        bDstBkColValid=sal_True;
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
        *pPict << (sal_uInt16)0x0022;
        WritePoint(rLocPt);
        *pPict << (char)dh << (char)dv;
    }
    else
    {
        *pPict << (sal_uInt16)0x0020;
        WritePoint(rLocPt);
        WritePoint(rNewPt);
    }
    aDstPenPosition=rNewPt;
    bDstPenPositionValid=sal_True;
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
        *pPict << (sal_uInt16)0x0023;
        *pPict << (char)dh << (char)dv;
    }
    else
    {
        *pPict << (sal_uInt16)0x0021;
        WritePoint(rNewPt);
    }
    aDstPenPosition=rNewPt;
    bDstPenPositionValid=sal_True;
}


void PictWriter::WriteOpcode_Text(const Point & rPoint, const String& rString, sal_Bool bDelta)
{
    Point aPoint = OutputDevice::LogicToLogic( rPoint,
                                               aSrcMapMode,
                                               aTargetMapMode );
    long  dh,dv;
    sal_uLong nPos;

    nPos = pPict->Tell();
    dh = aPoint.X()-aDstTextPosition.X();
    dv = aPoint.Y()-aDstTextPosition.Y();

    if (bDstTextPositionValid==sal_False || dh<0 || dh>255 || dv<0 || dv>0 || bDelta==sal_False)
    {
        *pPict << (sal_uInt16)0x0028;
        WritePoint(rPoint);
    }
    else if (dv==0)
    {
        *pPict << (sal_uInt16)0x0029 << (sal_uInt8)dh;
    }
    else if (dh==0)
    {
        *pPict << (sal_uInt16)0x002a << (sal_uInt8)dv;
    }
    else
    {
        *pPict << (sal_uInt16)0x002b << (sal_uInt8)dh << (sal_uInt8)dv;
    }

    WriteString( rString );
    if (((pPict->Tell()-nPos)&1)!=0)
        *pPict << (sal_uInt8)0;

    aDstTextPosition = aPoint;
    bDstTextPositionValid=sal_True;
}


void PictWriter::WriteOpcode_FontName(const Font & rFont)
{
    sal_uInt16 nDataLen,nFontId;

    switch (rFont.GetFamily()) {
        case FAMILY_MODERN:     nFontId=22; break;
        case FAMILY_ROMAN:      nFontId=20; break;
        case FAMILY_SWISS:      nFontId=21; break;
        default:                nFontId=1;
    }

    if (bDstFontNameValid==sal_False || nDstFontNameId!=nFontId || aDstFontName!=rFont.GetName())
    {
        rtl::OString aString(rtl::OUStringToOString(rFont.GetName(), osl_getThreadTextEncoding()));
        sal_uInt16 nFontNameLen = aString.getLength();
        if ( nFontNameLen )
        {
            nDataLen = 3 + nFontNameLen;
            *pPict << (sal_uInt16)0x002c << nDataLen << nFontId;
            WriteString( rFont.GetName() );
            if ( ( nFontNameLen & 1 ) == 0 )
                *pPict << (sal_uInt8)0;
        }
        *pPict << (sal_uInt16)0x0003 << nFontId;
        aDstFontName=rFont.GetName();
        nDstFontNameId=nFontId;
        bDstFontNameValid=sal_True;
    }
}

void PictWriter::WriteOpcode_ClipRect( const Rectangle& rRect )
{
    Rectangle aRect( MapRectangle( rRect ) );
    aRect.nBottom++;
    aRect.nRight++;
    *pPict  << (sal_uInt16)1    // opcode 1
            << (sal_uInt16)10   // data size
            << (sal_Int16)aRect.Top() << (sal_Int16)aRect.Left()
            << (sal_Int16)aRect.Bottom() << (sal_Int16)aRect.Right();
    aClipRect = aRect;
}

void PictWriter::WriteOpcode_Rect(PictDrawingMethod eMethod, const Rectangle & rRect)
{
    sal_uInt16 oc;
    switch (eMethod) {
        case PDM_FRAME:  oc=0x0030; break;
        case PDM_PAINT:  oc=0x0031; break;
        case PDM_ERASE:  oc=0x0032; break;
        case PDM_INVERT: oc=0x0033; break;
        case PDM_FILL:   oc=0x0034; break;
        default:         oc=0;      break;   // -Wall a default for oc...
    }
    *pPict << oc;
    WriteRectangle(rRect);
}


void PictWriter::WriteOpcode_SameRect(PictDrawingMethod eMethod)
{
    sal_uInt16 oc;
    switch (eMethod) {
        case PDM_FRAME:  oc=0x0038; break;
        case PDM_PAINT:  oc=0x0039; break;
        case PDM_ERASE:  oc=0x003a; break;
        case PDM_INVERT: oc=0x003b; break;
        case PDM_FILL:   oc=0x003c; break;
        default:         oc=0;      break;   // -Wall a default for oc...
    }
    *pPict << oc;
}


void PictWriter::WriteOpcode_RRect(PictDrawingMethod eMethod, const Rectangle & rRect)
{
    sal_uInt16 oc;
    switch (eMethod) {
        case PDM_FRAME:  oc=0x0040; break;
        case PDM_PAINT:  oc=0x0041; break;
        case PDM_ERASE:  oc=0x0042; break;
        case PDM_INVERT: oc=0x0043; break;
        case PDM_FILL:   oc=0x0044; break;
        default:         oc=0;      break;   // -Wall a default for oc...
    }
    *pPict << oc;
    WriteRectangle(rRect);
}


void PictWriter::WriteOpcode_SameRRect(PictDrawingMethod eMethod)
{
    sal_uInt16 oc;
    switch (eMethod) {
        case PDM_FRAME:  oc=0x0048; break;
        case PDM_PAINT:  oc=0x0049; break;
        case PDM_ERASE:  oc=0x004a; break;
        case PDM_INVERT: oc=0x004b; break;
        case PDM_FILL:   oc=0x004c; break;
        default:         oc=0;      break;   // -Wall a default for oc...
    }
    *pPict << oc;
}


void PictWriter::WriteOpcode_Oval(PictDrawingMethod eMethod, const Rectangle & rRect)
{
    sal_uInt16 oc;
    switch (eMethod) {
        case PDM_FRAME:  oc=0x0050; break;
        case PDM_PAINT:  oc=0x0051; break;
        case PDM_ERASE:  oc=0x0052; break;
        case PDM_INVERT: oc=0x0053; break;
        case PDM_FILL:   oc=0x0054; break;
        default:         oc=0;      break;   // -Wall a default for oc...
    }
    *pPict << oc;
    WriteRectangle(rRect);
}


void PictWriter::WriteOpcode_SameOval(PictDrawingMethod eMethod)
{
    sal_uInt16 oc;
    switch (eMethod) {
        case PDM_FRAME:  oc=0x0058; break;
        case PDM_PAINT:  oc=0x0059; break;
        case PDM_ERASE:  oc=0x005a; break;
        case PDM_INVERT: oc=0x005b; break;
        case PDM_FILL:   oc=0x005c; break;
        default:         oc=0;      break;   // -Wall a default for oc...
    }
    *pPict << oc;
}


void PictWriter::WriteOpcode_Arc(PictDrawingMethod eMethod, const Rectangle & rRect,
                                 const Point & rStartPt, const Point & rEndPt)
{
    sal_uInt16 oc;
    switch (eMethod) {
        case PDM_FRAME:  oc=0x0060; break;
        case PDM_PAINT:  oc=0x0061; break;
        case PDM_ERASE:  oc=0x0062; break;
        case PDM_INVERT: oc=0x0063; break;
        case PDM_FILL:   oc=0x0064; break;
        default:         oc=0;      break;   // -Wall a default for oc...
    }
    *pPict << oc;
    WriteRectangle(rRect);
    WriteArcAngles(rRect,rStartPt,rEndPt);
}


void PictWriter::WriteOpcode_SameArc(PictDrawingMethod eMethod, const Rectangle & rRect,
                                     const Point & rStartPt, const Point & rEndPt)
{
    sal_uInt16 oc;
    switch (eMethod) {
        case PDM_FRAME:  oc=0x0068; break;
        case PDM_PAINT:  oc=0x0069; break;
        case PDM_ERASE:  oc=0x006a; break;
        case PDM_INVERT: oc=0x006b; break;
        case PDM_FILL:   oc=0x006c; break;
        default:         oc=0;      break;   // -Wall a default for oc...
    }
    *pPict << oc;
    WriteArcAngles(rRect,rStartPt,rEndPt);
}


void PictWriter::WriteOpcode_Poly(PictDrawingMethod eMethod, const Polygon & rPoly)
{
    sal_uInt16 oc;

    if (rPoly.GetSize()<3) return;
    switch (eMethod) {
        case PDM_FRAME:  oc=0x0070; break;
        case PDM_PAINT:  oc=0x0071; break;
        case PDM_ERASE:  oc=0x0072; break;
        case PDM_INVERT: oc=0x0073; break;
        case PDM_FILL:   oc=0x0074; break;
        default:         oc=0;      break;   // -Wall a default for oc...
    }
    *pPict << oc;
    WritePolygon(rPoly);
}


void PictWriter::WriteOpcode_BitsRect(const Point & rPoint, const Size & rSize, const Bitmap & rBitmap)
{
    BitmapReadAccess*   pAcc = NULL;
    Bitmap              aBitmap( rBitmap );

    sal_uLong   nWidth, nHeight, nDstRowBytes, nx, nc, ny, nCount, nColTabSize, i;
    sal_uLong   nDstRowPos, nSrcRowBytes, nEqu3, nPos, nDstMapPos;
    sal_uInt16  nBitsPerPixel, nPackType;
    sal_uInt8   *pComp[4], *pPix, *pTemp;
    sal_uInt8    nEquData = 0;
    sal_uInt8    nFlagCounterByte, nRed, nGreen, nBlue;

    SetAttrForPaint();

    // generating a temporary Windows-BMP-File:
    nActBitmapPercent=30;
    MayCallback();

    if ( bStatus == sal_False )
        return;
    if ( ( pAcc = aBitmap.AcquireReadAccess() ) == NULL )
        return;

    nBitsPerPixel = aBitmap.GetBitCount();

    // export code below only handles four discrete cases
    nBitsPerPixel =
        nBitsPerPixel <= 1 ? 1 : nBitsPerPixel <= 4 ? 4 : nBitsPerPixel <= 8 ? 8 : 24;

    nWidth = pAcc->Width();
    nHeight = pAcc->Height();

    // If 24-Bit, then create the Opcode 'DirectBitsRect':
    if ( nBitsPerPixel == 24 )
    {
        // Anzahl Bytes einer (ungepackten) Zeile in Quelle und Ziel berechnen:
        nSrcRowBytes =( ( 3 * nWidth ) + 0x0003 ) & 0xfffc;
        nDstRowBytes = nWidth * 4;

        // writing Opcode and BaseAddr (?):
        *pPict << (sal_uInt16)0x009a << (sal_uInt32)0x000000ff;

        // Normalerweise wollen wir den Packing-Type 4 (Run length encoding
        // for 32-Bit Pixels) erzeugen. Wenn aber RowBytes<8 gilt, sind die Daten
        // grundsaetzlich ungepackt, auch wenn der Packing-Type 4 angegeben ist,
        // was etwas komisch erscheint. Daher wollen wir in so einem Fall lieber
        // gleich den Packing-Type 1 (ungepackt) angeben:

        if ( nDstRowBytes < 8 )
            nPackType = 1;
        else
            nPackType = 4;

        // writing PixMap-Structure:
        *pPict << (sal_uInt16)(nDstRowBytes|0x8000) // Bytes per row and the fact that it's a 'PixMap'
               << (sal_uInt16)0x0000                // Y1-position of the bitmap in the source
               << (sal_uInt16)0x0000                // X1-position of the bitmap in the source
               << (sal_uInt16)nHeight               // Y2-position of the bitmap in the source
               << (sal_uInt16)nWidth                // X2-position of the bitmap in the source
               << (sal_uInt16)0x0000                // Version
               << (sal_uInt16)nPackType             // Packing type
               << (sal_uInt32) 0x00000000            // Packing size (?)
               << (sal_uInt32) 0x00480000            // H-Res
               << (sal_uInt32) 0x00480000            // V-Res
               << (sal_uInt16)0x0010                // Pixel type (?)
               << (sal_uInt16)0x0020                // Pixel size: 32 bit
               << (sal_uInt16)0x0004                // CmpCount: 4 components
               << (sal_uInt16)0x0008                // CmpSize: 8 bits
               << (sal_uInt32) 0x00000000            // PlaneBytes (?)
               << (sal_uInt32) 0x00000000            // (?)
               << (sal_uInt32) 0x00000000;           // (?)

        // Source-Rectangle schreiben:
        *pPict << (sal_uInt16)0x0000                // Y1-position on the bitmap
               << (sal_uInt16)0x0000                // X1-position on the bitmap
               << (sal_uInt16)nHeight               // Y2-position on the bitmap
               << (sal_uInt16)nWidth;               // X2-position on the bitmap

        // writing the Destination-Rectangle:
        WritePoint( rPoint );
        WritePoint( Point( rPoint.X() + rSize.Width(), rPoint.Y() + rSize.Height() ) );

        // writing the Transfer mode:
        *pPict << (sal_uInt16)0x0000; // (?)

        // remember position of the Map-data in the target:
        nDstMapPos=pPict->Tell();

        if ( nPackType == 1 )               //  when 24 bits nWidth == 1 !!
        {                                   // don't pack
            for ( ny = 0; ny < nHeight; ny++ )
            {
                *pPict << (sal_uInt8)0;
                *pPict << (sal_uInt8)pAcc->GetPixel( ny, 0 ).GetRed();
                *pPict << (sal_uInt8)pAcc->GetPixel( ny, 0 ).GetGreen();
                *pPict << (sal_uInt8)pAcc->GetPixel( ny, 0 ).GetBlue();
                // count percentages, Callback, check errors:
                nActBitmapPercent = ( ny * 70 / nHeight ) + 30; // (30% already added up to the writing of the Win-BMP file)
                MayCallback();
            }
        }
        else    // packing ( PackType == 4 )
        {
            // Speicher fuer Zeilen-Zwischen-Datenstruktur allozieren:
            for ( nc = 0; nc < 4; nc++ )
                pComp[ nc ] = new sal_uInt8[ nWidth ];

            // loop trough rows:
            for ( ny = 0; ny < nHeight; ny++ )
            {
                // Zeil ny der Quelle in die Zwischen-Datenstrktur einlesen:

                for ( nx = 0; nx < nWidth; nx++ )
                {
                    pComp[ 1 ][ nx ] = (sal_uInt8)pAcc->GetPixel( ny, nx ) .GetRed();
                    pComp[ 2 ][ nx ] = (sal_uInt8)pAcc->GetPixel( ny, nx ) .GetGreen();
                    pComp[ 3 ][ nx ] = (sal_uInt8)pAcc->GetPixel( ny, nx ) .GetBlue();
                    pComp[ 0 ][ nx ] = 0;
                }

                // remember start of the row in the target:
                nDstRowPos = pPict->Tell();

                // ByteCount (that's the size of the packed row) is at first 0 (will be corrected later):
                if ( nDstRowBytes > 250 )
                    *pPict << (sal_uInt16)0;
                else
                    *pPict << (sal_uInt8)0;

                // loop trough components:
                for ( nc = 0; nc < 4; nc++ )
                {
                    // loop through x:
                    nx = 0;
                    while ( nx < nWidth )
                    {
                        // look up the position of 3 equal bytes and seve it in nEqu3
                        // if it's not found, set nEqu3=nWidth
                        // if it's found save the value of the bytes in nEquData
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
                            nFlagCounterByte = (sal_uInt8)(nCount-1);
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
                        {               // Hint: Then one has nx==nEqu3 (hopefully)
                            nCount=3;   // Three bytes are equal, as we found out above
                                        // Check, whether there are further equal bytes (and pay attention to Max.-Record-Size):
                            while ( nx + nCount < nWidth && nCount < 128 )
                            {
                                if ( nEquData != pComp[ nc ][ nx + nCount ] )
                                    break;
                                nCount++;
                            }
                            // nCount write equal Bytes compressed:
                            nFlagCounterByte = (sal_uInt8)( 1 - (long)nCount );
                            *pPict << nFlagCounterByte << nEquData;
                            nx += nCount;
                        }
                    }
                }
                // correct ByteCount:
                nPos = pPict->Tell();
                pPict->Seek( nDstRowPos );
                if ( nDstRowBytes > 250 )
                    *pPict << ( (sal_uInt16)( nPos - nDstRowPos - 2 ) );
                else
                    *pPict << ( (sal_uInt8)( nPos - nDstRowPos - 1 ) );
                pPict->Seek( nPos );

                // count percentages, Callback, check errors:
                nActBitmapPercent = ( ny * 70 / nHeight ) + 30; // (30% machten schon das Schreiben der Win-BMP-Datei aus)
                MayCallback();
            }
            // clean up:
            for ( nc = 0; nc < 4; nc++ )
                delete pComp[ nc ];
        }
    }
    else
    {   // don't generate 24-bit i.e. Opcode 'PackBitsRect':

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

        // writing Opcode:
        *pPict << (sal_uInt16)0x0098;

        // Normalerweise wollen wir den Packing-Type 0 (default Packing) erzeugen.
        // Wenn aber RowBytes<8 gilt, sind die Daten grundsaetzlich ungepackt,
        // auch wenn der Packing-Type 0 angegeben ist, was etwas komisch erscheint.
        // Daher wollen wir in so einem Fall lieber gleich den Packing-Type 1 (ungepackt)
        // angeben:
        if ( nDstRowBytes < 8 )
            nPackType = 1;
        else
            nPackType = 0;

        // write PixMap-Structure:
        *pPict << (sal_uInt16)(nDstRowBytes|0x8000) // Bytes per row and the fact that it's a 'PixMap'
               << (sal_uInt16)0x0000                // Y1-position of the bitmap in the source
               << (sal_uInt16)0x0000                // X1-position of the bitmap in the source
               << (sal_uInt16)nHeight               // Y2-position of the bitmap in the source
               << (sal_uInt16)nWidth                // X2-position of the bitmap in the source
               << (sal_uInt16)0x0000                // Version
               << (sal_uInt16)nPackType             // Packing type
               << (sal_uInt32) 0x00000000            // Packing size (?)
               << (sal_uInt32) 0x00480000            // H-Res
               << (sal_uInt32) 0x00480000            // V-Res
               << (sal_uInt16)0x0000                // Pixel type (?)
               << (sal_uInt16)nBitsPerPixel         // Pixel size
               << (sal_uInt16)0x0001                // CmpCount: 1 component
               << (sal_uInt16)nBitsPerPixel         // CmpSize
               << (sal_uInt32) 0x00000000            // PlaneBytes (?)
               << (sal_uInt32) 0x00000000            // (?)
               << (sal_uInt32) 0x00000000;           // (?)

        // writing and reading the palette:
        nColTabSize = pAcc->GetPaletteEntryCount();
        *pPict << (sal_uInt32)0 << (sal_uInt16)0x8000 << (sal_uInt16)( nColTabSize - 1 );

        for ( i = 0; i < nColTabSize; i++ )
        {
            nRed = (sal_uInt8)pAcc->GetPaletteColor( (sal_uInt16)i ).GetRed();
            nGreen = (sal_uInt8)pAcc->GetPaletteColor( (sal_uInt16)i ).GetGreen();
            nBlue = (sal_uInt8)pAcc->GetPaletteColor( (sal_uInt16)i ).GetBlue();
            *pPict << (sal_uInt16)0 << nRed << nRed << nGreen << nGreen << nBlue << nBlue;
        }

        // writing Source-Rectangle:
        *pPict << (sal_uInt16)0 << (sal_uInt16)0 << (sal_uInt16)nHeight << (sal_uInt16)nWidth;

        // writing Destination-Rectangle:
        WritePoint( rPoint );
        WritePoint( Point( rPoint.X() + rSize.Width(), rPoint.Y() + rSize.Height() ) );

        // writing Transfer mode:
        *pPict << (sal_uInt16)0;            // (?)

        // allocate memory for a row:
        pPix = new sal_uInt8[ nSrcRowBytes ];

        // Position der Map-Daten in Ziel merken:
        nDstMapPos=pPict->Tell();

        // loop trough rows:
        for ( ny = 0; ny < nHeight; ny++ )
        {

            // Zeile ny der Quelle in den Zwischenspeicher einlesen:

            switch ( nBitsPerPixel )
            {
                case 1 :
                    for ( pTemp = pPix, i = 0; i < nSrcRowBytes; i++ )
                        *pTemp++ = (sal_uInt8)0;
                    for ( i = 0; i < nWidth; i++ )
                        pPix[ ( i >> 3 ) ] |= (sal_uInt8)( pAcc->GetPixel( ny, i ) & 1 ) << ( ( i & 7 ) ^ 7 );
                    break;
                case 4 :
                    for ( pTemp = pPix, i = 0; i < nSrcRowBytes; i++ )
                        *pTemp++ = (sal_uInt8)0;
                    for ( i = 0; i < nWidth; i++ )
                        pPix[ ( i >> 1 ) ] |= (sal_uInt8)( pAcc->GetPixel( ny, i ) & 15 ) << ( ( i & 1 ) << 2 ) ;
                    break;
                case 8 :
                    for ( i = 0; i < nWidth; i++ )
                        pPix[ i ] = (sal_uInt8)pAcc->GetPixel( ny, i );
                    break;
            }

            if ( nPackType == 1 )
            {   // don't pack
                pPict->Write( pPix, nDstRowBytes );
            }
            else
            {   // Ppacking (nPackType==0)

                // remeber start of the row in the target:
                nDstRowPos = pPict->Tell();

                // ByteCount (das ist die Groesse der gepackten Zeile) zunaechst 0 (wird spaeter berichtigt):
                if ( nDstRowBytes > 250 )
                    *pPict << (sal_uInt16)0;
                else
                    *pPict << (sal_uInt8)0;

                // loop trough bytes of the row:
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
                        nFlagCounterByte = (sal_uInt8)( nCount - 1 );
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
                        nFlagCounterByte = (sal_uInt8)( 1 - (long)nCount );
                        *pPict << nFlagCounterByte << nEquData;
                        nx += nCount;
                    }
                }

                // correct ByteCount:
                nPos = pPict->Tell();
                pPict->Seek( nDstRowPos );
                if ( nDstRowBytes > 250 )
                    *pPict << ( (sal_uInt16)( nPos - nDstRowPos - 2 ) );
                else
                    *pPict << ( (sal_uInt8)( nPos - nDstRowPos - 1 ) );
                pPict->Seek( nPos );
            }

            // count percentages, Callback, check errors:
            nActBitmapPercent =( ny * 70 / nHeight ) + 30; // (30% machten schon das Schreiben der Win-BMP-Datei aus)
            MayCallback();
            if ( pPict->GetError() )
                bStatus = sal_False;
        }
        // cleaning up:
        delete[] pPix;
    }

    // Map-Data has to be an even number of bytes:
    if ( ( ( pPict->Tell() - nDstMapPos ) & 1 ) != 0 )
        *pPict << (sal_uInt8)0;

    // counting Bitmaps:
    nWrittenBitmaps++;
    nActBitmapPercent = 0;
    if ( pAcc )
        aBitmap.ReleaseAccess( pAcc );
}

void PictWriter::WriteOpcode_EndOfFile()
{
    *pPict << (sal_uInt16)0x00ff;
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
    WriteOpcode_PnLinePat(sal_True);
    WriteOpcode_FontName(aSrcFont);
    WriteOpcode_TxSize((sal_uInt16)(aSrcFont.GetSize().Height()));
    WriteOpcode_TxMode(eSrcRasterOp);
    WriteOpcode_TxFace(aSrcFont);
}


void PictWriter::WriteTextArray(Point & rPoint, const String& rString, const sal_Int32 * pDXAry)
{
    sal_uInt16 i,nLen;
    sal_Unicode c;
    sal_Bool bDelta;
    Point aPt;

    if ( pDXAry == NULL )
        WriteOpcode_Text( rPoint, rString, sal_False );
    else
    {
        bDelta = sal_False;
        nLen = rString.Len();
        for ( i = 0; i < nLen; i++ )
        {
            c = rString.GetChar( i );
            if ( c && ( c != 0x20 ) )
            {
                aPt = rPoint;
                if ( i > 0 )
                    aPt.X() += pDXAry[ i - 1 ];

                WriteOpcode_Text( aPt, rtl::OUString( c ), bDelta );
                bDelta = sal_True;
            }
        }
    }
}

void PictWriter::HandleLineInfoPolyPolygons(const LineInfo& rInfo, const basegfx::B2DPolygon& rLinePolygon)
{
    if(rLinePolygon.count())
    {
        basegfx::B2DPolyPolygon aLinePolyPolygon(rLinePolygon);
        basegfx::B2DPolyPolygon aFillPolyPolygon;

        rInfo.applyToB2DPolyPolygon(aLinePolyPolygon, aFillPolyPolygon);

        if(aLinePolyPolygon.count())
        {
            aLinePolyPolygon = aLinePolyPolygon.getDefaultAdaptiveSubdivision();
            const sal_uInt32 nPolyCount(aLinePolyPolygon.count());
            SetAttrForFrame();

            for(sal_uInt32 a(0); a < nPolyCount; a++)
            {
                const basegfx::B2DPolygon aCandidate(aLinePolyPolygon.getB2DPolygon(a));
                const sal_uInt32 nPointCount(aCandidate.count());

                if(nPointCount)
                {
                    const sal_uInt32 nEdgeCount(aCandidate.isClosed() ? nPointCount + 1 : nPointCount);
                    const basegfx::B2DPoint aCurr(aCandidate.getB2DPoint(0));
                    Point nCurr(basegfx::fround(aCurr.getX()), basegfx::fround(aCurr.getY()));

                    for(sal_uInt32 b(0); b < nEdgeCount; b++)
                    {
                        const sal_uInt32 nNextIndex((b + 1) % nPointCount);
                        const basegfx::B2DPoint aNext(aCandidate.getB2DPoint(nNextIndex));
                        const Point nNext(basegfx::fround(aNext.getX()), basegfx::fround(aNext.getY()));

                        WriteOpcode_Line(nCurr, nNext);
                        nCurr = nNext;
                    }
                }
            }
        }

        if(aFillPolyPolygon.count())
        {
            const Color aOldLineColor(aLineColor);
            const Color aOldFillColor(aFillColor);

            aLineColor = Color( COL_TRANSPARENT );
            aFillColor = aOldLineColor;
            SetAttrForPaint();

            for(sal_uInt32 a(0); a < aFillPolyPolygon.count(); a++)
            {
                const Polygon aPolygon(aFillPolyPolygon.getB2DPolygon(a).getDefaultAdaptiveSubdivision());
                WriteOpcode_Poly(PDM_PAINT, aPolygon);
            }

            aLineColor = aOldLineColor;
            aFillColor = aOldFillColor;
        }
    }
}

void PictWriter::WriteOpcodes( const GDIMetaFile & rMTF )
{
    size_t nA, nACount;
    const MetaAction* pMA;

    if( !bStatus)
        return;

    nACount = rMTF.GetActionSize();

    for (nA=0; nA < nACount; nA++)
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
                WriteOpcode_PnLinePat(sal_True);
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
                    if(pA->GetLineInfo().IsDefault())
                    {
                        SetAttrForFrame();
                        WriteOpcode_Line( pA->GetStartPoint(),pA->GetEndPoint() );
                    }
                    else
                    {
                        // LineInfo used; handle Dash/Dot and fat lines
                        basegfx::B2DPolygon aPolygon;
                        aPolygon.append(basegfx::B2DPoint(pA->GetStartPoint().X(), pA->GetStartPoint().Y()));
                        aPolygon.append(basegfx::B2DPoint(pA->GetEndPoint().X(), pA->GetEndPoint().Y()));
                        HandleLineInfoPolyPolygons(pA->GetLineInfo(), aPolygon);
                    }
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
//                OSL_FAIL( "Unsupported PICT-Action: META_CHORD_ACTION!" );
            }
            break;

            case META_POLYLINE_ACTION:
            {
                const MetaPolyLineAction* pA = (const MetaPolyLineAction*) pMA;

                if( aLineColor!=Color( COL_TRANSPARENT ) )
                {
                    const Polygon&  rPoly = pA->GetPolygon();

                    if( rPoly.GetSize() )
                    {
                        if(pA->GetLineInfo().IsDefault())
                        {
                            Polygon aSimplePoly;
                            if ( rPoly.HasFlags() )
                                rPoly.AdaptiveSubdivide( aSimplePoly );
                            else
                                aSimplePoly = rPoly;

                            const sal_uInt16    nSize = aSimplePoly.GetSize();
                            Point           aLast;

                            if ( nSize )
                            {
                                SetAttrForFrame();
                                aLast = aSimplePoly[0];

                                for ( sal_uInt16 i = 1; i < nSize; i++ )
                                {
                                    WriteOpcode_Line( aLast, aSimplePoly[i] );
                                    aLast = aSimplePoly[i];
                                }
                            }
                        }
                        else
                        {
                            // LineInfo used; handle Dash/Dot and fat lines
                            HandleLineInfoPolyPolygons(pA->GetLineInfo(), rPoly.getB2DPolygon());
                        }
                    }
                }
            }
            break;

            case META_POLYGON_ACTION:
            {
                const MetaPolygonAction* pA = (const MetaPolygonAction*) pMA;

                const Polygon& rPoly = pA->GetPolygon();

                Polygon aSimplePoly;
                if ( rPoly.HasFlags() )
                    rPoly.AdaptiveSubdivide( aSimplePoly );
                else
                    aSimplePoly = rPoly;

                if (aFillColor!=Color( COL_TRANSPARENT ))
                {
                    SetAttrForPaint();
                    WriteOpcode_Poly( PDM_PAINT, aSimplePoly );
                }
                if (aLineColor!=Color( COL_TRANSPARENT ))
                {
                    SetAttrForFrame();
                    WriteOpcode_Poly( PDM_FRAME, aSimplePoly );
                }
            }
            break;

            case META_POLYPOLYGON_ACTION:
            {
                const MetaPolyPolygonAction* pA = (const MetaPolyPolygonAction*) pMA;

                const PolyPolygon& rPolyPoly = pA->GetPolyPolygon();
                sal_uInt16 nPolyCount = rPolyPoly.Count();
                PolyPolygon aSimplePolyPoly( rPolyPoly );
                for ( sal_uInt16 i = 0; i < nPolyCount; i++ )
                {
                    if ( aSimplePolyPoly[ i ].HasFlags() )
                    {
                        Polygon aSimplePoly;
                        aSimplePolyPoly[ i ].AdaptiveSubdivide( aSimplePoly );
                        aSimplePolyPoly[ i ] = aSimplePoly;
                    }
                }
                if (aFillColor!=Color( COL_TRANSPARENT ))
                {
                    SetAttrForPaint();
                    WriteOpcode_Poly( PDM_PAINT, PolyPolygonToPolygon( aSimplePolyPoly ));
                }

                if (aLineColor!=Color( COL_TRANSPARENT ))
                {
                    sal_uInt16 nCount,i;
                    SetAttrForFrame();
                    nCount = aSimplePolyPoly.Count();
                    for ( i = 0; i < nCount; i++ )
                        WriteOpcode_Poly( PDM_FRAME, aSimplePolyPoly.GetObject( i ) );
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
                WriteOpcode_Text( aPt, aStr, sal_False );
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
                WriteTextArray( aPt, aStr, pA->GetDXArray() );
                break;
            }

            case META_STRETCHTEXT_ACTION:
            {
                const MetaStretchTextAction*    pA = (const MetaStretchTextAction*) pMA;
                Point                           aPt( pA->GetPoint() );
                String                          aStr( pA->GetText(),pA->GetIndex(),pA->GetLen() );
                VirtualDevice                   aVirDev;
                sal_Int32*                      pDXAry = new sal_Int32[ aStr.Len() ];
                sal_Int32                       nNormSize( aVirDev.GetTextArray( aStr,pDXAry ) );
                sal_uInt16                          i;

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
                WriteTextArray( aPt, aStr, pDXAry );
                delete[] pDXAry;
            }
            break;

            case META_TEXTRECT_ACTION:
            {
//                OSL_FAIL( "Unsupported PICT-Action: META_TEXTRECT_ACTION!" );
            }
            break;

            case META_BMP_ACTION:
            {
                const MetaBmpAction*    pA = (const MetaBmpAction*) pMA;
                const Bitmap            aBmp( pA->GetBitmap() );
                VirtualDevice           aVirDev;

                WriteOpcode_BitsRect( pA->GetPoint(), aVirDev.PixelToLogic( aBmp.GetSizePixel(), aSrcMapMode ), aBmp );
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
                VirtualDevice           aVirDev;

                WriteOpcode_BitsRect( pA->GetPoint(), aVirDev.PixelToLogic( aBmp.GetSizePixel(), aSrcMapMode ), aBmp );
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

                size_t nCount = aGDIMetaFile.GetActionSize();
                for ( size_t i = 0; i < nCount; i++ )
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
//                OSL_FAIL( "Unsupported PICT-Action: META_MASK..._ACTION!" );
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
//                OSL_FAIL( "Unsupported PICT-Action: META_WALLPAPER_ACTION!" );
            }
            break;

            case META_CLIPREGION_ACTION:
            {
//                OSL_FAIL( "Unsupported PICT-Action: META_CLIPREGION_ACTION!" );
            }
            break;

            case META_ISECTRECTCLIPREGION_ACTION:
            {
                const MetaISectRectClipRegionAction* pA = (const MetaISectRectClipRegionAction*) pMA;
                WriteOpcode_ClipRect( pA->GetRect() );
            }
            break;

            case META_ISECTREGIONCLIPREGION_ACTION:
            {
//                OSL_FAIL( "Unsupported PICT-Action: META_ISECTREGIONCLIPREGION_ACTION!" );
            }
            break;

            case META_MOVECLIPREGION_ACTION:
            {
//                OSL_FAIL( "Unsupported PICT-Action: META_MOVECLIPREGION_ACTION!" );
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
//                OSL_FAIL( "Unsupported PICT-Action: META_TEXTALIGN_ACTION!" );
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
                pAt->aClipRect=aClipRect;
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
                    if ( pAt->aClipRect != aClipRect )
                    {
                        Rectangle aRect( pAt->aClipRect );
                        *pPict  << (sal_uInt16)1    // opcode 1
                                << (sal_uInt16)10   // data size
                                << (sal_Int16)aRect.Top() << (sal_Int16)aRect.Left()
                                << (sal_Int16)aRect.Bottom() << (sal_Int16)aRect.Right();
                    }
                    aClipRect=pAt->aClipRect;
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
                    for( sal_uInt16 i = 0, nCount = rPolyPoly.Count(); i < nCount; i++ )
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

            case( META_RENDERGRAPHIC_ACTION ):
            {
                const MetaRenderGraphicAction*          pA = (const MetaRenderGraphicAction*) pMA;
                const ::vcl::RenderGraphicRasterizer    aRasterizer( pA->GetRenderGraphic() );
                VirtualDevice                           aVirDev;
                const Bitmap                            aBmp( Graphic( aRasterizer.Rasterize(
                                                            aVirDev.LogicToPixel( pA->GetSize() ) ) ).GetBitmap() );

                WriteOpcode_BitsRect( pA->GetPoint(), pA->GetSize(), aBmp );
            }
            break;
        }

        nWrittenActions++;
        MayCallback();

        if (pPict->GetError())
        bStatus=sal_False;

        if (bStatus==sal_False)
        break;
    }
}


void PictWriter::WriteHeader(const GDIMetaFile & rMTF)
{
    sal_uInt16  i;
    Size aSize( rMTF.GetPrefSize() );
    Point aPoint;
    Rectangle   aRect( aPoint, aSize );

    // 512 Bytes "trash" at the beginning:
    for (i=0;i<128;i++) *pPict << (sal_uInt32)0;

    // Lo-16-Bits of the file size without the 512 bytes trash:
    *pPict << (sal_uInt16)0; // gets corrected later on by UpdateHeader()

    // The Bounding-Rectangle (y1,x1,y2,x2 !):
    WriteRectangle( aRect );

    // Version 2:
    *pPict << (sal_uInt32)0x001102ff;

    // Extended-Version-2-Header:
    *pPict << (sal_uInt16)0x0c00                            // Opcode
           << (sal_uInt16)0xfffe                            // Version (?)
           << (sal_uInt16)0x0000                            // Reserved
           << (sal_uInt32) 0x00480000                        // hRes
           << (sal_uInt32) 0x00480000;
    WriteRectangle( aRect );
    *pPict << (sal_uInt32)0x00000000;                        // Reserved

    // many import filters demand the declaration
    // of a clipping area at the beginning

    WriteOpcode_ClipRect( aRect );
}


void PictWriter::UpdateHeader()
{
    sal_uLong nPos;

    // correct the Lo-16-Bits of the file size without the 512 bytes trash:
    nPos=pPict->Tell();
    pPict->Seek(512);
    *pPict << (sal_uInt16)((nPos-512)&0x0000ffff);
    pPict->Seek(nPos);
}


sal_Bool PictWriter::WritePict(const GDIMetaFile & rMTF, SvStream & rTargetStream, FilterConfigItem* pFilterConfigItem )
{
    PictWriterAttrStackMember*  pAt;
    MapMode                     aMap72( MAP_INCH );
    Fraction                    aDPIFrac( 1, 72 );

    bStatus=sal_True;
    nLastPercent=0;

    if ( pFilterConfigItem )
    {
        xStatusIndicator = pFilterConfigItem->GetStatusIndicator();
        if ( xStatusIndicator.is() )
        {
            rtl::OUString aMsg;
            xStatusIndicator->start( aMsg, 100 );
        }
    }

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

    bDstBkPatValid=sal_False;
    bDstTxFaceValid=sal_False;
    bDstTxModeValid=sal_False;
    bDstPnSizeValid=sal_False;
    bDstPnModeValid=sal_False;
    bDstPnPatValid=sal_False;
    bDstFillPatValid=sal_False;
    bDstTxSizeValid=sal_False;
    bDstFgColValid=sal_False;
    bDstBkColValid=sal_False;
    bDstPenPositionValid=sal_False;
    bDstTextPositionValid=sal_False;
    bDstFontNameValid=sal_False;

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

    if ( xStatusIndicator.is() )
        xStatusIndicator->end();

    return bStatus;
}

//================== GraphicExport - the exported Function ================

extern "C" SAL_DLLPUBLIC_EXPORT sal_Bool __LOADONCALLAPI
GraphicExport(SvStream & rStream, Graphic & rGraphic, FilterConfigItem* pFilterConfigItem, sal_Bool)
{
    PictWriter      aPictWriter;

    if (rGraphic.GetType()==GRAPHIC_GDIMETAFILE)
    {
        GDIMetaFile aScaledMtf( rGraphic.GetGDIMetaFile() );
        return aPictWriter.WritePict( aScaledMtf, rStream, pFilterConfigItem );
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
        return aPictWriter.WritePict( aMTF, rStream, pFilterConfigItem );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
