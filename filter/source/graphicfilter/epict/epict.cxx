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

#include <vcl/metaact.hxx>
#include <vcl/graphicfilter.hxx>
#include <svl/solar.hrc>
#include <vcl/fltcall.hxx>

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
#include <tools/bigint.hxx>
#include <tools/fract.hxx>
#include <o3tl/numeric.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <memory>

// PictWriter
struct PictWriterAttrStackMember {
    struct PictWriterAttrStackMember *  pSucc;
    Color                               aLineColor;
    Color                               aFillColor;
    RasterOp                            eRasterOp;
    vcl::Font                           aFont;
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

    bool bStatus;
    sal_uLong nLastPercent; // with which number pCallback has been called the last time
    css::uno::Reference< css::task::XStatusIndicator > xStatusIndicator;

    SvStream * pPict;

    // current attributes in the source-metafile:
    Color       aLineColor;
    Color       aFillColor;
    RasterOp    eSrcRasterOp;
    vcl::Font   aSrcFont;
    MapMode     aSrcMapMode;
    MapMode     aTargetMapMode;
    Rectangle   aClipRect;
    PictWriterAttrStackMember * pAttrStack;

    // current attributes in the target-metafile and whether they are valid
    bool bDstBkPatValid;
    sal_uInt8        nDstTxFace;            bool bDstTxFaceValid;
    RasterOp    eDstTxMode;         bool bDstTxModeValid;
    sal_uInt16      nDstPnSize;         bool bDstPnSizeValid;
    RasterOp    eDstPnMode;         bool bDstPnModeValid;
    PictPattern aDstPnPat;          bool bDstPnPatValid;
    bool bDstFillPatValid;
    sal_uInt16      nDstTxSize;         bool bDstTxSizeValid;
    Color       aDstFgCol;          bool bDstFgColValid;
    Color       aDstBkCol;          bool bDstBkColValid;
    Point       aDstPenPosition;    bool bDstPenPositionValid;
    Point       aDstTextPosition;   bool bDstTextPositionValid;
    OUString    aDstFontName; sal_uInt16 nDstFontNameId; bool bDstFontNameValid;

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

    static tools::Polygon PolyPolygonToPolygon(const tools::PolyPolygon & rPoly);
        // generates a relatively sane polygon on the basis of a PolyPolygon

    Rectangle MapRectangle( const Rectangle& rRect );
    void WritePoint(const Point & rPoint);
    void WriteSize(const Size & rSize);
    void WriteRGBColor(const Color & rColor);
    void WriteString( const OUString & rString );
    void WriteRectangle(const Rectangle & rRect);
    void WritePolygon(const tools::Polygon & rPoly);
    void WriteArcAngles(const Rectangle & rRect, const Point & rStartPt, const Point & rEndPt);

    static void ConvertLinePattern(PictPattern & rPat, bool bVisible);
    static void ConvertFillPattern(PictPattern & rPat, bool bVisible);

    void WriteOpcode_TxFace(const vcl::Font & rFont);
    void WriteOpcode_TxMode(RasterOp eMode);
    void WriteOpcode_PnSize(sal_uInt16 nSize);
    void WriteOpcode_PnMode(RasterOp eMode);
    void WriteOpcode_PnLinePat(bool bVisible);
    void WriteOpcode_PnFillPat(bool bVisible);
    void WriteOpcode_OvSize(const Size & rSize);
    void WriteOpcode_TxSize(sal_uInt16 nSize);
    void WriteOpcode_RGBFgCol(const Color & rColor);
    void WriteOpcode_RGBBkCol(const Color & rColor);
    void WriteOpcode_Line(const Point & rLocPt, const Point & rNewPt);
    void WriteOpcode_LineFrom(const Point & rNewPt);
    void WriteOpcode_Text(const Point & rPoint, const OUString& rString, bool bDelta);
    void WriteOpcode_FontName(const vcl::Font & rFont);
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
    void WriteOpcode_Poly(PictDrawingMethod eMethod, const tools::Polygon & rPoly);
    void WriteOpcode_BitsRect(const Point & rPoint, const Size & rSize, const Bitmap & rBitmap);
    void WriteOpcode_EndOfFile();

    void SetAttrForPaint();
    void SetAttrForFrame();
    void SetAttrForText();

    void WriteTextArray(Point & rPoint, const OUString& rString, const long* pDXAry);

    void HandleLineInfoPolyPolygons(const LineInfo& rInfo, const basegfx::B2DPolygon& rLinePolygon);
    void WriteOpcodes(const GDIMetaFile & rMTF);

    void WriteHeader(const GDIMetaFile & rMTF);
    void UpdateHeader();

public:

    bool WritePict( const GDIMetaFile & rMTF, SvStream & rTargetStream, FilterConfigItem* pFilterConfigItem );
};


// Methods of PictWriter
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

    nActionCount = rMTF.GetActionSize();

    for (nAction=0; nAction < nActionCount; nAction++)
    {
        const MetaAction* pMA = rMTF.GetAction( nAction );

        switch( pMA->GetType() )
        {
            case MetaActionType::BMP:
            case MetaActionType::BMPSCALE:
            case MetaActionType::BMPSCALEPART:
            case MetaActionType::BMPEX:
            case MetaActionType::BMPEXSCALE:
            case MetaActionType::BMPEXSCALEPART:
                nNumberOfBitmaps++;
            break;
            default: break;
        }

        nNumberOfActions++;
    }
}


tools::Polygon PictWriter::PolyPolygonToPolygon(const tools::PolyPolygon & rPolyPoly)
{
    sal_uInt16 nCount,nSize1,nSize2,np,i1,i2,i3,nBestIdx1,nBestIdx2;
    long nDistSqr;
    Point aP1,aPRel;
    tools::Polygon aPoly1, aPoly2, aPoly3;

    nCount=rPolyPoly.Count();
    if (nCount==0) return tools::Polygon(0);

    aPoly1=rPolyPoly.GetObject(0);
    for (np=1; np<nCount; np++) {
        aPoly2=rPolyPoly.GetObject(np);

        // The following code merges aPoly1 and aPoly2 to aPoly1
        nSize1=aPoly1.GetSize();
        nSize2=aPoly2.GetSize();

        // At first we look for a point in aPoly1 (referenced by nBestIdx1) and a
        // point in aPoly2 (referenced by nBestid2), which are as close together as
        // possible. Because this is following square complexity and therefore some
        // pictures would need infinite time to export, we limit the number of test
        // by the number of 1000 and cancel the search if necessary preliminarily.
        // The result of this will not be wrong but rather not so beautiful.
        long nCountdownTests = 1000;
        long nBestDistSqr = 0x7fffffff;
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
    }
    return aPoly1;
}


void PictWriter::WritePoint(const Point & rPoint)
{
    Point aPoint = OutputDevice::LogicToLogic( rPoint, aSrcMapMode, aTargetMapMode );
    pPict->WriteInt16( aPoint.Y() ).WriteInt16( aPoint.X() );
}


void PictWriter::WriteSize(const Size & rSize)
{
    OutputDevice::LogicToLogic( rSize, aSrcMapMode, aTargetMapMode ); // -Wall is this needed.
    pPict->WriteInt16( rSize.Height() ).WriteInt16( rSize.Width() );
}


void PictWriter::WriteRGBColor(const Color & rColor)
{
    const sal_uInt16 nR = ( (sal_uInt16) rColor.GetRed() << 8 ) | (sal_uInt16) rColor.GetRed();
    const sal_uInt16 nG = ( (sal_uInt16) rColor.GetGreen() << 8 ) | (sal_uInt16) rColor.GetGreen();
    const sal_uInt16 nB = ( (sal_uInt16) rColor.GetBlue() << 8 ) | (sal_uInt16) rColor.GetBlue();

    pPict->WriteUInt16( nR ).WriteUInt16( nG ).WriteUInt16( nB );
}

void PictWriter::WriteString( const OUString & rString )
{
    OString aString(OUStringToOString(rString, osl_getThreadTextEncoding()));
    sal_Int32 nLen = aString.getLength();
    if ( nLen > 255 )
        nLen = 255;
    pPict->WriteUChar( nLen  );
    for (sal_Int32 i = 0; i < nLen; ++i)
        pPict->WriteChar( aString[i] );
}

Rectangle PictWriter::MapRectangle( const Rectangle& rRect )
{
    Point   aPoint = OutputDevice::LogicToLogic( rRect.TopLeft(), aSrcMapMode, aTargetMapMode );
    Size    aSize = OutputDevice::LogicToLogic( rRect.GetSize(), aSrcMapMode, aTargetMapMode );
    Rectangle aRect( aPoint, aSize );
    aRect.Justify();
    ++aRect.Bottom();
    ++aRect.Right();
    return aRect;
}

void PictWriter::WriteRectangle(const Rectangle & rRect)
{
    Rectangle aRect( MapRectangle( rRect ) );
    pPict ->WriteInt16( aRect.Top() ).WriteInt16( aRect.Left() )
           .WriteInt16( aRect.Bottom() ).WriteInt16( aRect.Right() );
}

void PictWriter::WritePolygon(const tools::Polygon & rPoly)
{
    sal_uInt16 nDataSize,i,nSize;
    short nMinX = 0, nMinY = 0, nMaxX = 0, nMaxY = 0;
    tools::Polygon aPoly(rPoly);

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

        short nx = (short) aPoint.X();
        short ny = (short) aPoint.Y();

        if ( i==0 || nMinX>nx )
            nMinX=nx;
        if ( i==0 || nMinY>ny )
            nMinY=ny;
        if ( i==0 || nMaxX<nx )
            nMaxX=nx;
        if ( i==0 || nMaxY<ny )
            nMaxY=ny;
    }

    pPict->WriteUInt16( nDataSize ).WriteInt16( nMinY ).WriteInt16( nMinX ).WriteInt16( nMaxY ).WriteInt16( nMaxX );

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
    pPict->WriteInt16( nStartAngle ).WriteInt16( nArcAngle );
}


void PictWriter::ConvertLinePattern(PictPattern & rPat, bool bVisible)
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

void PictWriter::ConvertFillPattern(PictPattern & rPat, bool bVisible)
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


void PictWriter::WriteOpcode_TxFace(const vcl::Font & rFont)
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
    if (rFont.IsOutline())              nFace|=0x08;
    if (rFont.IsShadow())               nFace|=0x10;

    if (!bDstTxFaceValid || nDstTxFace!=nFace) {
        pPict->WriteUInt16( 0x0004 ).WriteUChar( nFace ).WriteUChar( 0 );
        nDstTxFace=nFace;
        bDstTxFaceValid=true;
    }
}


void PictWriter::WriteOpcode_TxMode(RasterOp eMode)
{
    sal_uInt16 nVal;

    if (!bDstTxModeValid || eDstTxMode!=eMode) {
        switch (eMode) {
            case ROP_INVERT: nVal=0x000c; break;
            case ROP_XOR:    nVal=0x000a; break;
            default:         nVal=0x0008;
        }
        pPict->WriteUInt16( 0x0005 ).WriteUInt16( nVal );
        eDstTxMode=eMode;
        bDstTxModeValid=true;
    }
}


void PictWriter::WriteOpcode_PnSize(sal_uInt16 nSize)
{
    if (nSize==0) nSize=1;
    if (!bDstPnSizeValid || nDstPnSize!=nSize) {
        pPict->WriteUInt16( 0x0007 ).WriteUInt16( nSize ).WriteUInt16( nSize );
        nDstPnSize=nSize;
        bDstPnSizeValid=true;
    }
}


void PictWriter::WriteOpcode_PnMode(RasterOp eMode)
{
    sal_uInt16 nVal;

    if (!bDstPnModeValid || eDstPnMode!=eMode) {
        switch (eMode)
        {
            case ROP_INVERT: nVal=0x000c; break;
            case ROP_XOR:    nVal=0x000a; break;
            default:         nVal=0x0008;
        }
        pPict->WriteUInt16( 0x0008 ).WriteUInt16( nVal );
        eDstPnMode=eMode;
        bDstPnModeValid=true;
    }
}


void PictWriter::WriteOpcode_PnLinePat(bool bVisible)
{
    PictPattern aPat;

    ConvertLinePattern(aPat,bVisible);
    if (!bDstPnPatValid || aDstPnPat.nHi!=aPat.nHi || aDstPnPat.nLo!=aPat.nLo) {
        pPict->WriteUInt16( 0x0009 ).WriteUInt32( aPat.nHi ).WriteUInt32( aPat.nLo );
        aDstPnPat=aPat;
        bDstPnPatValid=true;
    }
}


void PictWriter::WriteOpcode_PnFillPat(bool bVisible)
{
    PictPattern aPat;

    ConvertFillPattern(aPat,bVisible);
    if (!bDstPnPatValid || aDstPnPat.nHi!=aPat.nHi || aDstPnPat.nLo!=aPat.nLo) {
        pPict->WriteUInt16( 0x0009 ).WriteUInt32( aPat.nHi ).WriteUInt32( aPat.nLo );
        aDstPnPat=aPat;
        bDstPnPatValid=true;
    }
}


void PictWriter::WriteOpcode_OvSize(const Size & rSize)
{
    pPict->WriteUInt16( 0x000b );
    WriteSize(rSize);
}


void PictWriter::WriteOpcode_TxSize(sal_uInt16 nSize)
{
    if (!bDstTxSizeValid || nDstTxSize!=nSize) {

        nDstTxSize = (sal_uInt16) OutputDevice::LogicToLogic( Size( 0, nSize ),
                                                          aSrcMapMode, aTargetMapMode ).Height();

        pPict->WriteUInt16( 0x000d ).WriteUInt16( nDstTxSize );
        bDstTxSizeValid=true;
    }
}


void PictWriter::WriteOpcode_RGBFgCol(const Color & rColor)
{
    if (!bDstFgColValid || aDstFgCol!=rColor) {
        pPict->WriteUInt16( 0x001a );
        WriteRGBColor(rColor);
        aDstFgCol=rColor;
        bDstFgColValid=true;
    }
}


void PictWriter::WriteOpcode_RGBBkCol(const Color & rColor)
{
    if (!bDstBkColValid || aDstBkCol!=rColor) {
        pPict->WriteUInt16( 0x001b );
        WriteRGBColor(rColor);
        aDstBkCol=rColor;
        bDstBkColValid=true;
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
        pPict->WriteUInt16( 0x0022 );
        WritePoint(rLocPt);
        pPict->WriteChar( (char)dh ).WriteChar( (char)dv );
    }
    else
    {
        pPict->WriteUInt16( 0x0020 );
        WritePoint(rLocPt);
        WritePoint(rNewPt);
    }
    aDstPenPosition=rNewPt;
    bDstPenPositionValid=true;
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
        pPict->WriteUInt16( 0x0023 );
        pPict->WriteChar( (char)dh ).WriteChar( (char)dv );
    }
    else
    {
        pPict->WriteUInt16( 0x0021 );
        WritePoint(rNewPt);
    }
    aDstPenPosition=rNewPt;
    bDstPenPositionValid=true;
}


void PictWriter::WriteOpcode_Text(const Point & rPoint, const OUString& rString, bool bDelta)
{
    Point aPoint = OutputDevice::LogicToLogic( rPoint,
                                               aSrcMapMode,
                                               aTargetMapMode );
    long  dh,dv;
    sal_uLong nPos;

    nPos = pPict->Tell();
    dh = aPoint.X()-aDstTextPosition.X();
    dv = aPoint.Y()-aDstTextPosition.Y();

    if (!bDstTextPositionValid || dh<0 || dh>255 || dv<0 || dv>255 || !bDelta)
    {
        pPict->WriteUInt16( 0x0028 );
        WritePoint(rPoint);
    }
    else if (dv==0)
    {
        pPict->WriteUInt16( 0x0029 ).WriteUChar( dh );
    }
    else if (dh==0)
    {
        pPict->WriteUInt16( 0x002a ).WriteUChar( dv );
    }
    else
    {
        pPict->WriteUInt16( 0x002b ).WriteUChar( dh ).WriteUChar( dv );
    }

    WriteString( rString );
    if (((pPict->Tell()-nPos)&1)!=0)
        pPict->WriteUChar( 0 );

    aDstTextPosition = aPoint;
    bDstTextPositionValid=true;
}


void PictWriter::WriteOpcode_FontName(const vcl::Font & rFont)
{
    sal_uInt16 nDataLen,nFontId;

    switch (rFont.GetFamily()) {
        case FAMILY_MODERN:     nFontId=22; break;
        case FAMILY_ROMAN:      nFontId=20; break;
        case FAMILY_SWISS:      nFontId=21; break;
        default:                nFontId=1;
    }

    if (!bDstFontNameValid || nDstFontNameId!=nFontId || aDstFontName!=rFont.GetName())
    {
        OString aString(OUStringToOString(rFont.GetName(), osl_getThreadTextEncoding()));
        sal_uInt16 nFontNameLen = aString.getLength();
        if ( nFontNameLen )
        {
            nDataLen = 3 + nFontNameLen;
            pPict->WriteUInt16( 0x002c ).WriteUInt16( nDataLen ).WriteUInt16( nFontId );
            WriteString( rFont.GetName() );
            if ( ( nFontNameLen & 1 ) == 0 )
                pPict->WriteUChar( 0 );
        }
        pPict->WriteUInt16( 0x0003 ).WriteUInt16( nFontId );
        aDstFontName=rFont.GetName();
        nDstFontNameId=nFontId;
        bDstFontNameValid=true;
    }
}

void PictWriter::WriteOpcode_ClipRect( const Rectangle& rRect )
{
    Rectangle aRect( MapRectangle( rRect ) );
    ++aRect.Bottom();
    ++aRect.Right();
    pPict ->WriteUInt16( 1 )    // opcode 1
           .WriteUInt16( 10 )   // data size
           .WriteInt16( aRect.Top() ).WriteInt16( aRect.Left() )
           .WriteInt16( aRect.Bottom() ).WriteInt16( aRect.Right() );
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
    pPict->WriteUInt16( oc );
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
    pPict->WriteUInt16( oc );
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
    pPict->WriteUInt16( oc );
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
    pPict->WriteUInt16( oc );
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
    pPict->WriteUInt16( oc );
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
    pPict->WriteUInt16( oc );
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
    pPict->WriteUInt16( oc );
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
    pPict->WriteUInt16( oc );
    WriteArcAngles(rRect,rStartPt,rEndPt);
}


void PictWriter::WriteOpcode_Poly(PictDrawingMethod eMethod, const tools::Polygon & rPoly)
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
    pPict->WriteUInt16( oc );
    WritePolygon(rPoly);
}


void PictWriter::WriteOpcode_BitsRect(const Point & rPoint, const Size & rSize, const Bitmap & rBitmap)
{
    BitmapReadAccess*   pAcc = NULL;
    Bitmap              aBitmap( rBitmap );

    sal_uLong   nWidth, nHeight, nDstRowBytes, nx, nc, ny, nCount, nColTabSize, i;
    sal_uLong   nDstRowPos, nEqu3, nPos, nDstMapPos;
    sal_uInt16  nBitsPerPixel, nPackType;
    sal_uInt8   *pComp[4];
    sal_uInt8    nEquData = 0;
    sal_uInt8    nFlagCounterByte, nRed, nGreen, nBlue;

    SetAttrForPaint();

    // generating a temporary Windows-BMP-File:
    nActBitmapPercent=30;
    MayCallback();

    if ( !bStatus )
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
        // Calculate the number of bytes of an (uncompressed) line of destination.
        nDstRowBytes = nWidth * 4;

        // writing Opcode and BaseAddr (?):
        pPict->WriteUInt16( 0x009a ).WriteUInt32( 0x000000ff );

        // Normally we want to produce packing type 4 (run length encoding
        // for 32-bit pixels). But if RowBytes<8 is true, generally all data is
        // unpacked even if packing type 4 is specified, which seems a little bit
        // strange. Hence we want to specify packing type 1 (no packing) in these cases:

        if ( nDstRowBytes < 8 )
            nPackType = 1;
        else
            nPackType = 4;

        // writing PixMap-Structure:
        pPict->WriteUInt16( nDstRowBytes|0x8000 )   // Bytes per row and the fact that it's a 'PixMap'
              .WriteUInt16( 0x0000 )                // Y1-position of the bitmap in the source
              .WriteUInt16( 0x0000 )                // X1-position of the bitmap in the source
              .WriteUInt16( nHeight )               // Y2-position of the bitmap in the source
              .WriteUInt16( nWidth )                // X2-position of the bitmap in the source
              .WriteUInt16( 0x0000 )                // Version
              .WriteUInt16( nPackType )             // Packing type
              .WriteUInt32( 0x00000000 )            // Packing size (?)
              .WriteUInt32( 0x00480000 )            // H-Res
              .WriteUInt32( 0x00480000 )            // V-Res
              .WriteUInt16( 0x0010 )                // Pixel type (?)
              .WriteUInt16( 0x0020 )                // Pixel size: 32 bit
              .WriteUInt16( 0x0004 )                // CmpCount: 4 components
              .WriteUInt16( 0x0008 )                // CmpSize: 8 bits
              .WriteUInt32( 0x00000000 )            // PlaneBytes (?)
              .WriteUInt32( 0x00000000 )            // (?)
              .WriteUInt32( 0x00000000 );           // (?)

        // Source-Rectangle schreiben:
        pPict->WriteUInt16( 0x0000 )                // Y1-position on the bitmap
              .WriteUInt16( 0x0000 )                // X1-position on the bitmap
              .WriteUInt16( nHeight )               // Y2-position on the bitmap
              .WriteUInt16( nWidth );               // X2-position on the bitmap

        // writing the Destination-Rectangle:
        WritePoint( rPoint );
        WritePoint( Point( rPoint.X() + rSize.Width(), rPoint.Y() + rSize.Height() ) );

        // writing the Transfer mode:
        pPict->WriteUInt16( 0x0000 ); // (?)

        // remember position of the Map-data in the target:
        nDstMapPos=pPict->Tell();

        if ( nPackType == 1 )               //  when 24 bits nWidth == 1 !!
        {                                   // don't pack
            for ( ny = 0; ny < nHeight; ny++ )
            {
                pPict->WriteUChar( 0 );
                pPict->WriteUChar( pAcc->GetPixel( ny, 0 ).GetRed() );
                pPict->WriteUChar( pAcc->GetPixel( ny, 0 ).GetGreen() );
                pPict->WriteUChar( pAcc->GetPixel( ny, 0 ).GetBlue() );
                // count percentages, Callback, check errors:
                nActBitmapPercent = ( ny * 70 / nHeight ) + 30; // (30% already added up to the writing of the Win-BMP file)
                MayCallback();
            }
        }
        else    // packing ( PackType == 4 )
        {
            // allocate memory for lines-intermediate-data-structure
            for ( nc = 0; nc < 4; nc++ )
                pComp[ nc ] = new sal_uInt8[ nWidth ];

            // loop through rows:
            for ( ny = 0; ny < nHeight; ny++ )
            {
                // read line ny of source into the intermediate data structure

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
                    pPict->WriteUInt16( 0 );
                else
                    pPict->WriteUChar( 0 );

                // loop through components:
                for ( nc = 0; nc < 4; nc++ )
                {
                    // loop through x:
                    nx = 0;
                    while ( nx < nWidth )
                    {
                        // look up the position of 3 equal bytes and save it in nEqu3
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

                        // write the data from nx to nEqu3 uncompressed (into multiple records if necessarcy);
                        while ( nEqu3 > nx )
                        {
                            nCount = nEqu3 - nx;
                            if ( nCount > 128 )
                                nCount=128;
                            nFlagCounterByte = (sal_uInt8)(nCount-1);
                            pPict->WriteUChar( nFlagCounterByte );
                            do
                            {
                                pPict->WriteUChar( pComp[nc][nx++] );
                                nCount--;
                            }
                            while ( nCount > 0 );
                        }

                        // now create a compression record (if at least 3 identical bytes were found above)
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
                            pPict->WriteUChar( nFlagCounterByte ).WriteUChar( nEquData );
                            nx += nCount;
                        }
                    }
                }
                // correct ByteCount:
                nPos = pPict->Tell();
                pPict->Seek( nDstRowPos );
                if ( nDstRowBytes > 250 )
                    pPict->WriteUInt16( nPos - nDstRowPos - 2 );
                else
                    pPict->WriteUChar( nPos - nDstRowPos - 1 );
                pPict->Seek( nPos );

                // count percentages, Callback, check errors:
                nActBitmapPercent = ( ny * 70 / nHeight ) + 30; // (30% already added up to the writing of the Win-BMP file)
                MayCallback();
            }
            // clean up:
            for ( nc = 0; nc < 4; nc++ )
                delete pComp[ nc ];
        }
    }
    else
    {   // don't generate 24-bit i.e. Opcode 'PackBitsRect':

        // Some input filters are ignoring the palette of 1-bit images and are using
        // the foreground and the background color instead.
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

        // Calculate the number of bytes of an (unpacked) line of source an destination.
        nDstRowBytes = ( nWidth * nBitsPerPixel + 7 ) >> 3;
        sal_uLong nSrcRowBytes = ( nDstRowBytes + 3 ) & 0xfffffffc;

        // writing Opcode:
        pPict->WriteUInt16( 0x0098 );

        // Normally we want to produce packing type 0 (default packing).
        // But if RowBytes<8 is true, generally all data is unpacked even if packing
        // type 0 is specified, which seems a little bit strange. Hence we want to
        // specify packing type 1 (no packing) in these cases.
        if ( nDstRowBytes < 8 )
            nPackType = 1;
        else
            nPackType = 0;

        // write PixMap-Structure:
        pPict->WriteUInt16( nDstRowBytes|0x8000 )   // Bytes per row and the fact that it's a 'PixMap'
              .WriteUInt16( 0x0000 )                // Y1-position of the bitmap in the source
              .WriteUInt16( 0x0000 )                // X1-position of the bitmap in the source
              .WriteUInt16( nHeight )               // Y2-position of the bitmap in the source
              .WriteUInt16( nWidth )                // X2-position of the bitmap in the source
              .WriteUInt16( 0x0000 )                // Version
              .WriteUInt16( nPackType )             // Packing type
              .WriteUInt32( 0x00000000 )            // Packing size (?)
              .WriteUInt32( 0x00480000 )            // H-Res
              .WriteUInt32( 0x00480000 )            // V-Res
              .WriteUInt16( 0x0000 )                // Pixel type (?)
              .WriteUInt16( nBitsPerPixel )         // Pixel size
              .WriteUInt16( 0x0001 )                // CmpCount: 1 component
              .WriteUInt16( nBitsPerPixel )         // CmpSize
              .WriteUInt32( 0x00000000 )            // PlaneBytes (?)
              .WriteUInt32( 0x00000000 )            // (?)
              .WriteUInt32( 0x00000000 );           // (?)

        // writing and reading the palette:
        nColTabSize = pAcc->GetPaletteEntryCount();
        pPict->WriteUInt32( 0 ).WriteUInt16( 0x8000 ).WriteUInt16( nColTabSize - 1 );

        for ( i = 0; i < nColTabSize; i++ )
        {
            nRed = (sal_uInt8)pAcc->GetPaletteColor( (sal_uInt16)i ).GetRed();
            nGreen = (sal_uInt8)pAcc->GetPaletteColor( (sal_uInt16)i ).GetGreen();
            nBlue = (sal_uInt8)pAcc->GetPaletteColor( (sal_uInt16)i ).GetBlue();
            pPict->WriteUInt16( 0 ).WriteUChar( nRed ).WriteUChar( nRed ).WriteUChar( nGreen ).WriteUChar( nGreen ).WriteUChar( nBlue ).WriteUChar( nBlue );
        }

        // writing Source-Rectangle:
        pPict->WriteUInt16( 0 ).WriteUInt16( 0 ).WriteUInt16( nHeight ).WriteUInt16( nWidth );

        // writing Destination-Rectangle:
        WritePoint( rPoint );
        WritePoint( Point( rPoint.X() + rSize.Width(), rPoint.Y() + rSize.Height() ) );

        // writing Transfer mode:
        pPict->WriteUInt16( 0 );            // (?)

        // allocate memory for a row:
        std::unique_ptr<sal_uInt8[]> pPix(new sal_uInt8[ nSrcRowBytes ]);

        // remember position of the map-data in the target:
        nDstMapPos=pPict->Tell();

        // loop through rows:
        for ( ny = 0; ny < nHeight; ny++ )
        {
            sal_uInt8* pTemp;
            // read line ny of source into the buffer:
            switch ( nBitsPerPixel )
            {
                case 1 :
                    for ( pTemp = pPix.get(), i = 0; i < nSrcRowBytes; i++ )
                        *pTemp++ = (sal_uInt8)0;
                    for ( i = 0; i < nWidth; i++ )
                        pPix[ ( i >> 3 ) ] |= (pAcc->GetPixelIndex( ny, i ) & 1) << ((i & 7) ^ 7);
                    break;
                case 4 :
                    for ( pTemp = pPix.get(), i = 0; i < nSrcRowBytes; i++ )
                        *pTemp++ = (sal_uInt8)0;
                    for ( i = 0; i < nWidth; i++ )
                        pPix[ ( i >> 1 ) ] |= (pAcc->GetPixelIndex( ny, i ) & 15) << ((i & 1) << 2);
                    break;
                case 8 :
                    for ( i = 0; i < nWidth; i++ )
                        pPix[ i ] = pAcc->GetPixelIndex( ny, i );
                    break;
            }

            if ( nPackType == 1 )
            {   // don't pack
                pPict->Write( pPix.get(), nDstRowBytes );
            }
            else
            {   // Ppacking (nPackType==0)

                // remember start of the row in the target:
                nDstRowPos = pPict->Tell();

                // ByteCount (this is the size of the packed line) initialized with 0 (will be corrected later):
                if ( nDstRowBytes > 250 )
                    pPict->WriteUInt16( 0 );
                else
                    pPict->WriteUChar( 0 );

                // loop through bytes of the row:
                nx=0;
                while ( nx < nDstRowBytes && bStatus )
                {
                    // Look for the position of three identical bytes and remember it in nEqu3.
                    // Set nEqu3=nDstRowBytes if not found.
                    // Else remember the value of these bytes in nEquData.
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

                    // Write the data unpacked from nx to nEqu3 (in multiple records if necessary):
                    while ( nEqu3 > nx )
                    {
                        nCount = nEqu3 - nx;
                        if ( nCount > 128 )
                            nCount = 128;
                        nFlagCounterByte = (sal_uInt8)( nCount - 1 );
                        pPict->WriteUChar( nFlagCounterByte );
                        do
                        {
                            pPict->WriteUChar( pPix[ nx++ ] );
                            nCount--;
                        } while ( nCount > 0 );
                    }

                    // Now create a compression record (if at least 3 identical bytes were found above):
                    if ( nx < nDstRowBytes )
                    {   // Note: it is imperative nx==nEqu3 (hopefully)
                        nCount = 3; // three bytes are identically, as identified above
                        // Check if more identical bytes exist. (in doing so, consider max record size):
                        while ( nx + nCount < nDstRowBytes && nCount < 128 )
                        {
                            if ( nEquData != pPix[ nx + nCount ] )
                                break;
                            nCount++;
                        }
                        // write nCount identical bytes unpacked:
                        nFlagCounterByte = (sal_uInt8)( 1 - (long)nCount );
                        pPict->WriteUChar( nFlagCounterByte ).WriteUChar( nEquData );
                        nx += nCount;
                    }
                }

                // correct ByteCount:
                nPos = pPict->Tell();
                pPict->Seek( nDstRowPos );
                if ( nDstRowBytes > 250 )
                    pPict->WriteUInt16( nPos - nDstRowPos - 2 );
                else
                    pPict->WriteUChar( nPos - nDstRowPos - 1 );
                pPict->Seek( nPos );
            }

            // count percentages, Callback, check errors:
            nActBitmapPercent =( ny * 70 / nHeight ) + 30; // (30% already added up to the writing of the Win-BMP file)
            MayCallback();
            if ( pPict->GetError() )
                bStatus = false;
        }
    }

    // Map-Data has to be an even number of bytes:
    if ( ( ( pPict->Tell() - nDstMapPos ) & 1 ) != 0 )
        pPict->WriteUChar( 0 );

    // counting Bitmaps:
    nWrittenBitmaps++;
    nActBitmapPercent = 0;
    if ( pAcc )
        Bitmap::ReleaseAccess( pAcc );
}

void PictWriter::WriteOpcode_EndOfFile()
{
    pPict->WriteUInt16( 0x00ff );
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
    WriteOpcode_PnLinePat(true);
    WriteOpcode_FontName(aSrcFont);
    WriteOpcode_TxSize((sal_uInt16)(aSrcFont.GetSize().Height()));
    WriteOpcode_TxMode(eSrcRasterOp);
    WriteOpcode_TxFace(aSrcFont);
}


void PictWriter::WriteTextArray(Point & rPoint, const OUString& rString, const long* pDXAry)
{
    if ( pDXAry == NULL )
        WriteOpcode_Text( rPoint, rString, false );
    else
    {
        bool bDelta = false;
        sal_Int32 nLen = rString.getLength();
        for ( sal_Int32 i = 0; i < nLen; i++ )
        {
            sal_Unicode c = rString[ i ];
            if ( c && ( c != 0x20 ) )
            {
                Point aPt = rPoint;
                if ( i > 0 )
                    aPt.X() += pDXAry[ i - 1 ];

                WriteOpcode_Text( aPt, OUString( c ), bDelta );
                bDelta = true;
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
                const tools::Polygon aPolygon(aFillPolyPolygon.getB2DPolygon(a).getDefaultAdaptiveSubdivision());
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

    if( !bStatus)
        return;

    nACount = rMTF.GetActionSize();

    for (nA=0; nA < nACount; nA++)
    {
        const MetaAction* pMA = rMTF.GetAction(nA);

        switch (pMA->GetType())
        {
            case MetaActionType::PIXEL:
            {
                const MetaPixelAction* pA = static_cast<const MetaPixelAction*>(pMA);
                WriteOpcode_PnMode(eSrcRasterOp);
                WriteOpcode_PnSize(1);
                WriteOpcode_RGBFgCol(pA->GetColor());
                WriteOpcode_PnLinePat(true);
                WriteOpcode_Line(pA->GetPoint(),pA->GetPoint());
            }
            break;

            case MetaActionType::POINT:
            {
                const MetaPointAction* pA = static_cast<const MetaPointAction*>(pMA);

                if( aLineColor != Color( COL_TRANSPARENT ) )
                {
                    SetAttrForFrame();
                    WriteOpcode_Line( pA->GetPoint(),pA->GetPoint() );
                }
            }
            break;

            case MetaActionType::LINE:
            {
                const MetaLineAction* pA = static_cast<const MetaLineAction*>(pMA);

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

            case MetaActionType::RECT:
            {
                const MetaRectAction* pA = static_cast<const MetaRectAction*>(pMA);

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

            case MetaActionType::ROUNDRECT:
            {
                const MetaRoundRectAction* pA = static_cast<const MetaRoundRectAction*>(pMA);

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

            case MetaActionType::ELLIPSE:
            {
                const MetaEllipseAction* pA = static_cast<const MetaEllipseAction*>(pMA);

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

            case MetaActionType::ARC:
            {
                const MetaArcAction* pA = static_cast<const MetaArcAction*>(pMA);

                if (aLineColor!=Color( COL_TRANSPARENT ))
                {
                    SetAttrForFrame();
                    WriteOpcode_Arc(PDM_FRAME,pA->GetRect(),pA->GetStartPoint(),pA->GetEndPoint());
                }
            }
            break;

            case MetaActionType::PIE:
            {
                const MetaPieAction* pA = static_cast<const MetaPieAction *>(pMA);

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

            case MetaActionType::CHORD:
            {
                // OSL_FAIL( "Unsupported PICT-Action: MetaActionType::CHORD!" );
            }
            break;

            case MetaActionType::POLYLINE:
            {
                const MetaPolyLineAction* pA = static_cast<const MetaPolyLineAction*>(pMA);

                if( aLineColor!=Color( COL_TRANSPARENT ) )
                {
                    const tools::Polygon&  rPoly = pA->GetPolygon();

                    if( rPoly.GetSize() )
                    {
                        if(pA->GetLineInfo().IsDefault())
                        {
                            tools::Polygon aSimplePoly;
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

            case MetaActionType::POLYGON:
            {
                const MetaPolygonAction* pA = static_cast<const MetaPolygonAction*>(pMA);

                const tools::Polygon& rPoly = pA->GetPolygon();

                tools::Polygon aSimplePoly;
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

            case MetaActionType::POLYPOLYGON:
            {
                const MetaPolyPolygonAction* pA = static_cast<const MetaPolyPolygonAction*>(pMA);

                const tools::PolyPolygon& rPolyPoly = pA->GetPolyPolygon();
                sal_uInt16 nPolyCount = rPolyPoly.Count();
                tools::PolyPolygon aSimplePolyPoly( rPolyPoly );
                for ( sal_uInt16 i = 0; i < nPolyCount; i++ )
                {
                    if ( aSimplePolyPoly[ i ].HasFlags() )
                    {
                        tools::Polygon aSimplePoly;
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

            case MetaActionType::TEXT:
            {
                const MetaTextAction*   pA = static_cast<const MetaTextAction*>(pMA);
                Point                   aPt( pA->GetPoint() );

                if ( aSrcFont.GetAlign() != ALIGN_BASELINE )
                {
                    ScopedVclPtrInstance< VirtualDevice > pVirDev;
                    if (aSrcFont.GetAlign()==ALIGN_TOP)
                        aPt.Y()+=(long)pVirDev->GetFontMetric(aSrcFont).GetAscent();
                    else
                        aPt.Y()-=(long)pVirDev->GetFontMetric(aSrcFont).GetDescent();
                }

                SetAttrForText();
                OUString aStr = pA->GetText().copy( pA->GetIndex(),pA->GetLen() );
                WriteOpcode_Text( aPt, aStr, false );
            }
            break;

            case MetaActionType::TEXTARRAY:
            {
                const MetaTextArrayAction*  pA = static_cast<const MetaTextArrayAction*>(pMA);
                Point                       aPt( pA->GetPoint() );

                if (aSrcFont.GetAlign()!=ALIGN_BASELINE)
                {
                    ScopedVclPtrInstance< VirtualDevice > pVirDev;

                    if (aSrcFont.GetAlign()==ALIGN_TOP)
                        aPt.Y()+=(long)pVirDev->GetFontMetric(aSrcFont).GetAscent();
                    else
                        aPt.Y()-=(long)pVirDev->GetFontMetric(aSrcFont).GetDescent();
                }
                SetAttrForText();
                OUString aStr = pA->GetText().copy( pA->GetIndex(),pA->GetLen() );
                WriteTextArray( aPt, aStr, pA->GetDXArray() );
                break;
            }

            case MetaActionType::STRETCHTEXT:
            {
                const MetaStretchTextAction*    pA = static_cast<const MetaStretchTextAction*>(pMA);
                Point                           aPt( pA->GetPoint() );
                OUString                        aStr = pA->GetText().copy( pA->GetIndex(),pA->GetLen() );
                ScopedVclPtrInstance< VirtualDevice > pVirDev;
                std::unique_ptr<long[]>       pDXAry(new long[ aStr.getLength() ]);
                sal_Int32                       nNormSize( pVirDev->GetTextArray( aStr,pDXAry.get() ) );

                if (aSrcFont.GetAlign()!=ALIGN_BASELINE)
                {
                    if (aSrcFont.GetAlign()==ALIGN_TOP)
                        aPt.Y()+=(long)pVirDev->GetFontMetric(aSrcFont).GetAscent();
                    else
                        aPt.Y()-=(long)pVirDev->GetFontMetric(aSrcFont).GetDescent();
                }

                sal_Int32 nLength = aStr.getLength() - 1;
                if (nLength > 0)
                {
                    if (nNormSize == 0)
                        throw o3tl::divide_by_zero();
                    for (sal_Int32 i = 0; i < nLength; ++i)
                        pDXAry[ i ] = pDXAry[ i ] * ( (long)pA->GetWidth() ) / nNormSize;
                }

                SetAttrForText();
                WriteTextArray( aPt, aStr, pDXAry.get() );
            }
            break;

            case MetaActionType::TEXTRECT:
            {
                // OSL_FAIL( "Unsupported PICT-Action: MetaActionType::TEXTRECT!" );
            }
            break;

            case MetaActionType::BMP:
            {
                const MetaBmpAction*    pA = static_cast<const MetaBmpAction*>(pMA);
                const Bitmap            aBmp( pA->GetBitmap() );
                ScopedVclPtrInstance< VirtualDevice > pVirDev;

                WriteOpcode_BitsRect( pA->GetPoint(), pVirDev->PixelToLogic( aBmp.GetSizePixel(), aSrcMapMode ), aBmp );
            }
            break;

            case MetaActionType::BMPSCALE:
            {
                const MetaBmpScaleAction* pA = static_cast<const MetaBmpScaleAction*>(pMA);
                WriteOpcode_BitsRect( pA->GetPoint(), pA->GetSize(), pA->GetBitmap() );
            }
            break;

            case MetaActionType::BMPSCALEPART:
            {
                const MetaBmpScalePartAction*   pA = static_cast<const MetaBmpScalePartAction*>(pMA);
                Bitmap                          aBmp( pA->GetBitmap() );

                aBmp.Crop( Rectangle( pA->GetSrcPoint(), pA->GetSrcSize() ) );
                WriteOpcode_BitsRect( pA->GetDestPoint(), pA->GetDestSize(), aBmp );
            }
            break;

            case MetaActionType::BMPEX:
            {
                const MetaBmpExAction*  pA = static_cast<const MetaBmpExAction*>(pMA);
                const Bitmap            aBmp( Graphic( pA->GetBitmapEx() ).GetBitmap() );
                ScopedVclPtrInstance< VirtualDevice > pVirDev;

                WriteOpcode_BitsRect( pA->GetPoint(), pVirDev->PixelToLogic( aBmp.GetSizePixel(), aSrcMapMode ), aBmp );
            }
            break;

            case MetaActionType::BMPEXSCALE:
            {
                const MetaBmpExScaleAction* pA = static_cast<const MetaBmpExScaleAction*>(pMA);
                const Bitmap                aBmp( Graphic( pA->GetBitmapEx() ).GetBitmap() );

                WriteOpcode_BitsRect( pA->GetPoint(), pA->GetSize(), aBmp );
            }
            break;

            case MetaActionType::BMPEXSCALEPART:
            {
                const MetaBmpExScalePartAction* pA = static_cast<const MetaBmpExScalePartAction*>(pMA);
                Bitmap                          aBmp( Graphic( pA->GetBitmapEx() ).GetBitmap() );

                aBmp.Crop( Rectangle( pA->GetSrcPoint(), pA->GetSrcSize() ) );
                WriteOpcode_BitsRect( pA->GetDestPoint(), pA->GetDestSize(), aBmp );
            }
            break;

            case MetaActionType::EPS :
            {
                const MetaEPSAction* pA = static_cast<const MetaEPSAction*>(pMA);
                const GDIMetaFile aGDIMetaFile( pA->GetSubstitute() );

                size_t nCount = aGDIMetaFile.GetActionSize();
                for ( size_t i = 0; i < nCount; i++ )
                {
                    const MetaAction* pMetaAct = aGDIMetaFile.GetAction( i );
                    if ( pMetaAct->GetType() == MetaActionType::BMPSCALE )
                    {
                        const MetaBmpScaleAction* pBmpScaleAction = static_cast<const MetaBmpScaleAction*>(pMetaAct);
                        WriteOpcode_BitsRect( pA->GetPoint(), pA->GetSize(), pBmpScaleAction->GetBitmap() );
                        break;
                    }
                }
            }
            break;

            case MetaActionType::MASK:
            case MetaActionType::MASKSCALE:
            case MetaActionType::MASKSCALEPART:
            {
                // OSL_FAIL( "Unsupported PICT-Action: META_MASK..._ACTION!" );
            }
            break;

            case MetaActionType::GRADIENT:
            {
                ScopedVclPtrInstance<VirtualDevice> aVDev;
                GDIMetaFile                 aTmpMtf;
                const MetaGradientAction*   pA = static_cast<const MetaGradientAction*>(pMA);

                aVDev->SetMapMode( aTargetMapMode );
                aVDev->AddGradientActions( pA->GetRect(), pA->GetGradient(), aTmpMtf );
                WriteOpcodes( aTmpMtf );
            }
            break;

            case MetaActionType::HATCH:
            {
                ScopedVclPtrInstance<VirtualDevice> aVDev;
                GDIMetaFile             aTmpMtf;
                const MetaHatchAction*  pA = static_cast<const MetaHatchAction*>(pMA);

                aVDev->SetMapMode( aTargetMapMode );
                aVDev->AddHatchActions( pA->GetPolyPolygon(), pA->GetHatch(), aTmpMtf );
                WriteOpcodes( aTmpMtf );
            }
            break;

            case MetaActionType::WALLPAPER:
            {
                // OSL_FAIL( "Unsupported PICT-Action: MetaActionType::WALLPAPER!" );
            }
            break;

            case MetaActionType::CLIPREGION:
            {
                // OSL_FAIL( "Unsupported PICT-Action: MetaActionType::CLIPREGION!" );
            }
            break;

            case MetaActionType::ISECTRECTCLIPREGION:
            {
                const MetaISectRectClipRegionAction* pA = static_cast<const MetaISectRectClipRegionAction*>(pMA);
                WriteOpcode_ClipRect( pA->GetRect() );
            }
            break;

            case MetaActionType::ISECTREGIONCLIPREGION:
            {
                // OSL_FAIL( "Unsupported PICT-Action: MetaActionType::ISECTREGIONCLIPREGION!" );
            }
            break;

            case MetaActionType::MOVECLIPREGION:
            {
                // OSL_FAIL( "Unsupported PICT-Action: MetaActionType::MOVECLIPREGION!" );
            }
            break;

            case MetaActionType::LINECOLOR:
            {
                const MetaLineColorAction* pA = static_cast<const MetaLineColorAction*>(pMA);

                if( pA->IsSetting() )
                    aLineColor = pA->GetColor();
                else
                    aLineColor = Color( COL_TRANSPARENT );
            }
            break;

            case MetaActionType::FILLCOLOR:
            {
                const MetaFillColorAction* pA = static_cast<const MetaFillColorAction*>(pMA);

                if( pA->IsSetting() )
                    aFillColor = pA->GetColor();
                else
                    aFillColor = Color( COL_TRANSPARENT );
            }
            break;

            case MetaActionType::TEXTCOLOR:
            {
                const MetaTextColorAction* pA = static_cast<const MetaTextColorAction*>(pMA);
                aSrcFont.SetColor( pA->GetColor() );
            }
            break;

            case MetaActionType::TEXTFILLCOLOR:
            {
                const MetaTextFillColorAction* pA = static_cast<const MetaTextFillColorAction*>(pMA);

                if( pA->IsSetting() )
                    aSrcFont.SetFillColor( pA->GetColor() );
                else
                    aSrcFont.SetFillColor( Color( COL_TRANSPARENT ) );
            }
            break;

            case MetaActionType::TEXTALIGN:
            {
                // OSL_FAIL( "Unsupported PICT-Action: MetaActionType::TEXTALIGN!" );
            }
            break;

            case MetaActionType::MAPMODE:
            {
                const MetaMapModeAction* pA = static_cast<const MetaMapModeAction*>(pMA);

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

            case MetaActionType::FONT:
            {
                const MetaFontAction* pA = static_cast<const MetaFontAction*>(pMA);
                aSrcFont=pA->GetFont();
            }
            break;

            case MetaActionType::PUSH:
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

            case MetaActionType::POP:
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
                        pPict ->WriteUInt16( 1 )    // opcode 1
                               .WriteUInt16( 10 )   // data size
                               .WriteInt16( aRect.Top() ).WriteInt16( aRect.Left() )
                               .WriteInt16( aRect.Bottom() ).WriteInt16( aRect.Right() );
                    }
                    aClipRect=pAt->aClipRect;
                    pAttrStack=pAt->pSucc;
                    delete pAt;
                }
            }
            break;

            case MetaActionType::RASTEROP:
            {
                const MetaRasterOpAction* pA = static_cast<const MetaRasterOpAction*>(pMA);
                eSrcRasterOp=pA->GetRasterOp();
            }
            break;

            case MetaActionType::Transparent:
            {
                const tools::PolyPolygon& rPolyPoly = static_cast<const MetaTransparentAction*>(pMA)->GetPolyPolygon();

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

            case MetaActionType::FLOATTRANSPARENT:
            {
                const MetaFloatTransparentAction* pA = static_cast<const MetaFloatTransparentAction*>(pMA);

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
            default: break;
        }

        nWrittenActions++;
        MayCallback();

        if (pPict->GetError())
            bStatus=false;

        if (!bStatus)
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
    for (i=0;i<128;i++) pPict->WriteUInt32( 0 );

    // Lo-16-Bits of the file size without the 512 bytes trash:
    pPict->WriteUInt16( 0 ); // gets corrected later on by UpdateHeader()

    // The Bounding-Rectangle (y1,x1,y2,x2 !):
    WriteRectangle( aRect );

    // Version 2:
    pPict->WriteUInt32( 0x001102ff );

    // Extended-Version-2-Header:
    pPict->WriteUInt16( 0x0c00 )                            // Opcode
          .WriteUInt16( 0xfffe )                            // Version (?)
          .WriteUInt16( 0x0000 )                            // Reserved
          .WriteUInt32( 0x00480000 )                        // hRes
          .WriteUInt32( 0x00480000 );
    WriteRectangle( aRect );
    pPict->WriteUInt32( 0x00000000 );                        // Reserved

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
    pPict->WriteUInt16( (nPos-512) & 0xffff );
    pPict->Seek(nPos);
}


bool PictWriter::WritePict(const GDIMetaFile & rMTF, SvStream & rTargetStream, FilterConfigItem* pFilterConfigItem )
{
    PictWriterAttrStackMember*  pAt;
    MapMode                     aMap72( MAP_INCH );
    Fraction                    aDPIFrac( 1, 72 );

    bStatus=true;
    nLastPercent=0;

    if ( pFilterConfigItem )
    {
        xStatusIndicator = pFilterConfigItem->GetStatusIndicator();
        if ( xStatusIndicator.is() )
        {
            OUString aMsg;
            xStatusIndicator->start( aMsg, 100 );
        }
    }

    pPict=&rTargetStream;
    pPict->SetEndian(SvStreamEndian::BIG);

    aLineColor=Color( COL_BLACK );
    aFillColor=Color( COL_WHITE );
    eSrcRasterOp=ROP_OVERPAINT;
    aSrcFont=vcl::Font();
    aSrcMapMode = rMTF.GetPrefMapMode();

    aMap72.SetScaleX( aDPIFrac );
    aMap72.SetScaleY( aDPIFrac );
    aTargetMapMode = aMap72;

    pAttrStack=NULL;

    bDstBkPatValid=false;
    bDstTxFaceValid=false;
    bDstTxModeValid=false;
    bDstPnSizeValid=false;
    bDstPnModeValid=false;
    bDstPnPatValid=false;
    bDstFillPatValid=false;
    bDstTxSizeValid=false;
    bDstFgColValid=false;
    bDstBkColValid=false;
    bDstPenPositionValid=false;
    bDstTextPositionValid=false;
    bDstFontNameValid=false;

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

// GraphicExport - the exported Function

// this needs to be kept in sync with
// ImpFilterLibCacheEntry::GetImportFunction() from
// vcl/source/filter/graphicfilter.cxx
#if defined(DISABLE_DYNLOADING)
#define GraphicExport eptGraphicExport
#endif

extern "C" SAL_DLLPUBLIC_EXPORT bool SAL_CALL
GraphicExport( SvStream & rStream, Graphic & rGraphic, FilterConfigItem* pFilterConfigItem )
{
    PictWriter      aPictWriter;

    // #i119735# just use GetGDIMetaFile, it will create a bufferd version of contained bitmap now automatically
    GDIMetaFile aScaledMtf( rGraphic.GetGDIMetaFile() );

    return aPictWriter.WritePict( aScaledMtf, rStream, pFilterConfigItem );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
