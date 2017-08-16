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

#include <rtl/math.hxx>
#include <osl/endian.h>
#include <vcl/graph.hxx>
#include <tools/poly.hxx>
#include <tools/fract.hxx>
#include <vcl/graphicfilter.hxx>
#include "sgffilt.hxx"
#include "sgfbram.hxx"
#include "sgvmain.hxx"
#include "sgvspln.hxx"
#include <unotools/ucbstreamhelper.hxx>

//  Restrictions:

//  - area patterns are matched to the available ones in Starview.
//  - line ends are always rounded in StarView and continue past the end of line.
//  - line patterns are matched to the available ones in Starview.
//    transparency/opacity is not taken into account
//  - no rotated ellipses

// for font translation
SgfFontLst* pSgfFonts = nullptr;

// for circle kinds, text and rotated rectangles
void RotatePoint(PointType& P, sal_Int16 cx, sal_Int16 cy, double sn, double cs)
{
    sal_Int16  dx,dy;
    double x1,y1;
    dx=P.x-cx;
    dy=P.y-cy;
    x1=dx*cs-dy*sn;
    y1=dy*cs+dx*sn;
    P.x=cx+sal_Int16(x1);
    P.y=cy+sal_Int16(y1);
}

void RotatePoint(Point& P, sal_Int16 cx, sal_Int16 cy, double sn, double cs)
{
    sal_Int16  dx,dy;
    double x1,y1;
    dx=(sal_Int16)(P.X()-cx);
    dy=(sal_Int16)(P.Y()-cy);
    x1=dx*cs-dy*sn;
    y1=dy*cs+dx*sn;
    P=Point(cx+sal_Int16(x1),cy+sal_Int16(y1));
}

sal_Int16 iMulDiv(sal_Int16 a, sal_Int16 Mul, sal_Int16 Div)
{
    sal_Int32 Temp;
    Temp=sal_Int32(a)*sal_Int32(Mul)/sal_Int32(Div);
    return sal_Int16(Temp);
}

sal_uInt16 MulDiv(sal_uInt16 a, sal_uInt16 Mul, sal_uInt16 Div)
{
    sal_uInt32 Temp;
    Temp=sal_uInt32(a)*sal_uInt32(Mul)/sal_uInt32(Div);
    return sal_uInt16(Temp);
}

// SgfFilterSDrw

void DtHdOverSeek(SvStream& rInp)
{
    sal_uLong FPos=rInp.Tell();
    FPos+=(sal_uLong)DtHdSize;
    rInp.Seek(FPos);
}

PageType::PageType()
{
    memset( this, 0, sizeof( PageType ) );
}

SvStream& ReadPageType(SvStream& rIStream, PageType& rPage)
{
    sal_uInt64 const nOldPos(rIStream.Tell());
    rIStream.ReadUInt32(rPage.Next);
    rIStream.ReadUInt32(rPage.nList);
    rIStream.ReadUInt32(rPage.ListEnd);
    rIStream.ReadInt16(rPage.Paper.Size.x);
    rIStream.ReadInt16(rPage.Paper.Size.y);
    rIStream.ReadInt16(rPage.Paper.RandL);
    rIStream.ReadInt16(rPage.Paper.RandR);
    rIStream.ReadInt16(rPage.Paper.RandO);
    rIStream.ReadInt16(rPage.Paper.RandU);
    rIStream.ReadUChar(rPage.Paper.PColor);
    rIStream.ReadUChar(rPage.Paper.PIntens);
    rIStream.ReadCharAsBool(rPage.BorderClip);
    rIStream.ReadUChar(rPage.StdPg);
    rIStream.ReadInt16(rPage.U.x);
    rIStream.ReadInt16(rPage.U.y);
    for (int i = 0; i < 20; ++i)
    {
        rIStream.ReadInt16(rPage.HlpLnH[i]);
    }
    for (int i = 0; i < 20; ++i)
    {
        rIStream.ReadInt16(rPage.HlpLnV[i]);
    }
    rIStream.ReadUChar(rPage.LnAnzH);
    rIStream.ReadUChar(rPage.LnAnzV);
    for (int i = 0; i < 32; ++i)
    {
        rIStream.ReadUChar(rPage.PgName[i]);
    }
    assert(rIStream.GetError() || rIStream.Tell() == nOldPos + PageSize);
    (void) nOldPos;
    return rIStream;
}

void ReadObjLineType(SvStream & rInp, ObjLineType & rLine)
{
    // reads 8 bytes
    rInp.ReadUChar(rLine.LFarbe);
    rInp.ReadUChar(rLine.LBFarbe);
    rInp.ReadUChar(rLine.LIntens);
    rInp.ReadUChar(rLine.LMuster);
    rInp.ReadInt16(rLine.LMSize);
    rInp.ReadInt16(rLine.LDicke);
}

void ReadObjAreaType(SvStream & rInp, ObjAreaType & rArea)
{
    // reads 8 bytes
    rInp.ReadUChar(rArea.FFarbe);
    rInp.ReadUChar(rArea.FBFarbe);
    rInp.ReadUChar(rArea.FIntens);
    rInp.ReadUChar(rArea.FDummy1);
    rInp.ReadInt16(rArea.FDummy2);
    rInp.ReadUInt16(rArea.FMuster);
}

void ObjkOverSeek(SvStream& rInp, ObjkType const & rObjk)
{
    sal_uLong Siz;
    Siz=(sal_uLong)rObjk.MemSize+rObjk.Last;  // ObjSize+ObjAnhSize
    rInp.Seek(rInp.Tell()+Siz);
}

SvStream& ReadObjkType(SvStream& rInp, ObjkType& rObjk, bool const isRewind = true)
{
    // fileposition in stream is not changed!
    sal_uInt64 const nOldPos = rInp.Tell();
    rInp.ReadUInt32(rObjk.Last);
    rInp.ReadUInt32(rObjk.Next);
    rInp.ReadUInt16(rObjk.MemSize);
    rInp.ReadInt16(rObjk.ObjMin.x);
    rInp.ReadInt16(rObjk.ObjMin.y);
    rInp.ReadInt16(rObjk.ObjMax.x);
    rInp.ReadInt16(rObjk.ObjMax.y);
    rInp.ReadUChar(rObjk.Art);
    rInp.ReadUChar(rObjk.Layer);
    assert(rInp.GetError() || rInp.Tell() == nOldPos + ObjkSize);
    if (isRewind)
        rInp.Seek(nOldPos);
    return rInp;
}
SvStream& ReadStrkType(SvStream& rInp, StrkType& rStrk)
{
    sal_uInt64 const nOldPos(rInp.Tell());
    ReadObjkType(rInp, rStrk, false);
    rInp.ReadUChar(rStrk.Flags);
    rInp.ReadUChar(rStrk.LEnden);
    ReadObjLineType(rInp, rStrk.L);
    rInp.ReadInt16(rStrk.Pos1.x);
    rInp.ReadInt16(rStrk.Pos1.y);
    rInp.ReadInt16(rStrk.Pos2.x);
    rInp.ReadInt16(rStrk.Pos2.y);
    assert(rInp.GetError() || rInp.Tell() == nOldPos + StrkSize);
    (void) nOldPos;
    return rInp;
}
SvStream& ReadRectType(SvStream& rInp, RectType& rRect)
{
    sal_uInt64 const nOldPos(rInp.Tell());
    ReadObjkType(rInp, rRect, false);
    rInp.ReadUChar(rRect.Flags);
    rInp.ReadUChar(rRect.Reserve);
    ReadObjLineType(rInp, rRect.L);
    ReadObjAreaType(rInp, rRect.F);
    rInp.ReadInt16(rRect.Pos1.x);
    rInp.ReadInt16(rRect.Pos1.y);
    rInp.ReadInt16(rRect.Pos2.x);
    rInp.ReadInt16(rRect.Pos2.y);
    rInp.ReadInt16(rRect.Radius);
    rInp.ReadUInt16(rRect.RotationAngle);
    rInp.ReadUInt16(rRect.Slant);
    assert(rInp.GetError() || rInp.Tell() == nOldPos + RectSize);
    (void) nOldPos;
    return rInp;
}
SvStream& ReadPolyType(SvStream& rInp, PolyType& rPoly)
{
    sal_uInt64 const nOldPos(rInp.Tell());
    ReadObjkType(rInp, rPoly, false);
    rInp.ReadUChar(rPoly.Flags);
    rInp.ReadUChar(rPoly.LEnden);
    ReadObjLineType(rInp, rPoly.L);
    ReadObjAreaType(rInp, rPoly.F);
    rInp.ReadUChar(rPoly.nPoints);
    rInp.ReadUChar(rPoly.Reserve);
    rInp.ReadUInt32(rPoly.SD_EckP);
    assert(rInp.GetError() || rInp.Tell() == nOldPos + PolySize);
    (void) nOldPos;
    return rInp;
}

SvStream& ReadSplnType(SvStream& rInp, SplnType& rSpln)
{
    sal_uInt64 const nOldPos(rInp.Tell());
    ReadObjkType(rInp, rSpln, false);
    rInp.ReadUChar(rSpln.Flags);
    rInp.ReadUChar(rSpln.LEnden);
    ReadObjLineType(rInp, rSpln.L);
    ReadObjAreaType(rInp, rSpln.F);
    rInp.ReadUChar(rSpln.nPoints);
    rInp.ReadUChar(rSpln.Reserve);
    rInp.ReadUInt32(rSpln.SD_EckP);
    assert(rInp.GetError() || rInp.Tell() == nOldPos + SplnSize);
    (void) nOldPos;
    return rInp;
}
SvStream& ReadCircType(SvStream& rInp, CircType& rCirc)
{
    sal_uInt64 const nOldPos(rInp.Tell());
    ReadObjkType(rInp, rCirc, false);
    rInp.ReadUChar(rCirc.Flags);
    rInp.ReadUChar(rCirc.LEnden);
    ReadObjLineType(rInp, rCirc.L);
    ReadObjAreaType(rInp, rCirc.F);
    rInp.ReadInt16(rCirc.Center.x);
    rInp.ReadInt16(rCirc.Center.y);
    rInp.ReadInt16(rCirc.Radius.x);
    rInp.ReadInt16(rCirc.Radius.y);
    rInp.ReadUInt16(rCirc.RotationAngle);
    rInp.ReadUInt16(rCirc.StartAngle);
    rInp.ReadUInt16(rCirc.RelAngle);
    assert(rInp.GetError() || rInp.Tell() == nOldPos + CircSize);
    (void) nOldPos;
    return rInp;
}
SvStream& ReadTextType(SvStream& rInp, TextType& rText)
{
    sal_uInt64 const nOldPos(rInp.Tell());
    ReadObjkType(rInp, rText, false);
    rInp.ReadUChar(rText.Flags);
    rInp.ReadUChar(rText.Reserve);
    ReadObjLineType(rInp, rText.T.L);
    ReadObjAreaType(rInp, rText.T.F);
    rInp.ReadUInt16(rText.T.FontLo);
    rInp.ReadUInt16(rText.T.FontHi);
    rInp.ReadUInt16(rText.T.Grad);
    rInp.ReadUInt16(rText.T.Breite);
    rInp.ReadUChar(rText.T.Justify);
    rInp.ReadUChar(rText.T.Kapit);
    rInp.ReadUInt16(rText.T.Schnitt);
    rInp.ReadUInt16(rText.T.LnFeed);
    rInp.ReadUInt16(rText.T.Slant);
    rInp.ReadUChar(rText.T.ZAbst);
    rInp.ReadSChar(rText.T.ChrVPos);
    ReadObjLineType(rInp, rText.T.ShdL);
    ReadObjAreaType(rInp, rText.T.ShdF);
    rInp.ReadInt16(rText.T.ShdVers.x);
    rInp.ReadInt16(rText.T.ShdVers.y);
    rInp.ReadCharAsBool(rText.T.ShdAbs);
    rInp.ReadCharAsBool(rText.T.NoSpc);
    ReadObjAreaType(rInp, rText.T.BackF);
    rInp.ReadInt16(rText.Pos1.x);
    rInp.ReadInt16(rText.Pos1.y);
    rInp.ReadInt16(rText.Pos2.x);
    rInp.ReadInt16(rText.Pos2.y);
    rInp.ReadInt16(rText.TopOfs);
    rInp.ReadUInt16(rText.RotationAngle);
    rInp.ReadUInt16(rText.BoxSlant);
    rInp.ReadUInt16(rText.BufSize);
    rInp.ReadUInt16(rText.BufLo);
    rInp.ReadUInt16(rText.BufHi);
    rInp.ReadUInt16(rText.ExtLo);
    rInp.ReadUInt16(rText.ExtHi);
    rInp.ReadInt16(rText.FitSize.x);
    rInp.ReadInt16(rText.FitSize.y);
    rInp.ReadInt16(rText.FitBreit);
    assert(rInp.GetError() || rInp.Tell() == nOldPos + TextSize);
    (void) nOldPos;
    return rInp;
}
SvStream& ReadBmapType(SvStream& rInp, BmapType& rBmap)
{
    sal_uInt64 const nOldPos(rInp.Tell());
    ReadObjkType(rInp, rBmap, false);
    rInp.ReadUChar(rBmap.Flags);
    rInp.ReadUChar(rBmap.Reserve);
    ReadObjAreaType(rInp, rBmap.F);
    rInp.ReadInt16(rBmap.Pos1.x);
    rInp.ReadInt16(rBmap.Pos1.y);
    rInp.ReadInt16(rBmap.Pos2.x);
    rInp.ReadInt16(rBmap.Pos2.y);
    rInp.ReadUInt16(rBmap.RotationAngle);
    rInp.ReadUInt16(rBmap.Slant);
    for (int i = 0; i < 80; ++i)
    {
        rInp.ReadUChar(rBmap.Filename[i]);
    }
    rInp.ReadInt16(rBmap.PixSize.x);
    rInp.ReadInt16(rBmap.PixSize.y);
    static_assert(sizeof(enum GrafStat) == 4, "enum has unexpected size");
    sal_uInt32 nTemp(0);
    rInp.ReadUInt32(nTemp);
    rBmap.Format = static_cast<GrafStat>(nTemp);
    rInp.ReadUChar(rBmap.nPlanes);
    rInp.ReadCharAsBool(rBmap.RawOut);
    rInp.ReadCharAsBool(rBmap.InvOut);
    rInp.ReadCharAsBool(rBmap.LightOut);
    rInp.ReadUChar(rBmap.GrfFlg);
    assert(rInp.GetError() || rInp.Tell() == nOldPos + BmapSize);
    (void) nOldPos;
    return rInp;
}
SvStream& ReadGrupType(SvStream& rInp, GrupType& rGrup)
{
    sal_uInt64 const nOldPos(rInp.Tell());
    ReadObjkType(rInp, rGrup, false);
    rInp.ReadUChar(rGrup.Flags);
    for (int i = 0; i < 13; ++i)
    {
        rInp.ReadUChar(rGrup.Name[i]);
    }
    rInp.ReadUInt16(rGrup.SbLo);
    rInp.ReadUInt16(rGrup.SbHi);
    rInp.ReadUInt16(rGrup.UpLo);
    rInp.ReadUInt16(rGrup.UpHi);
    rInp.ReadUInt16(rGrup.ChartSize);
    rInp.ReadUInt32(rGrup.ChartPtr);
    assert(rInp.GetError() || rInp.Tell() == nOldPos + GrupSize);
    (void) nOldPos;
    return rInp;
}

Color Sgv2SvFarbe(sal_uInt8 nFrb1, sal_uInt8 nFrb2, sal_uInt8 nInts)
{
    sal_uInt16 r1=0,g1=0,b1=0,r2=0,g2=0,b2=0;
    sal_uInt8   nInt2=100-nInts;
    switch(nFrb1 & 0x07) {
        case 0:  r1=0xFF; g1=0xFF; b1=0xFF; break;
        case 1:  r1=0xFF; g1=0xFF;          break;
        case 2:           g1=0xFF; b1=0xFF; break;
        case 3:           g1=0xFF;          break;
        case 4:  r1=0xFF;          b1=0xFF; break;
        case 5:  r1=0xFF;                   break;
        case 6:                    b1=0xFF; break;
        case 7:                             break;
    }
    switch(nFrb2 & 0x07) {
        case 0:  r2=0xFF; g2=0xFF; b2=0xFF; break;
        case 1:  r2=0xFF; g2=0xFF;          break;
        case 2:           g2=0xFF; b2=0xFF; break;
        case 3:           g2=0xFF;          break;
        case 4:  r2=0xFF;          b2=0xFF; break;
        case 5:  r2=0xFF;                   break;
        case 6:                    b2=0xFF; break;
        case 7:                             break;
    }
    r1=(sal_uInt16)((sal_uInt32)r1*nInts/100+(sal_uInt32)r2*nInt2/100);
    g1=(sal_uInt16)((sal_uInt32)g1*nInts/100+(sal_uInt32)g2*nInt2/100);
    b1=(sal_uInt16)((sal_uInt32)b1*nInts/100+(sal_uInt32)b2*nInt2/100);
    Color aColor( (sal_uInt8)r1, (sal_uInt8)g1, (sal_uInt8)b1 );
    return aColor;
}

void SetLine(ObjLineType& rLine, OutputDevice& rOut)
{
    if( 0 == ( rLine.LMuster & 0x07 ) )
        rOut.SetLineColor();
    else
        rOut.SetLineColor( Sgv2SvFarbe(rLine.LFarbe,rLine.LBFarbe,rLine.LIntens) );
}

void SetArea(ObjAreaType& rArea, OutputDevice& rOut)
{
    if( 0 == ( rArea.FMuster & 0x00FF ) )
        rOut.SetFillColor();
    else
        rOut.SetFillColor( Sgv2SvFarbe( rArea.FFarbe,rArea.FBFarbe,rArea.FIntens ) );
}

void ObjkType::Draw(OutputDevice&, UCHAR *)
{
}

void StrkType::Draw(OutputDevice& rOut, UCHAR *)
{
    SetLine(L,rOut);
    rOut.DrawLine(Point(Pos1.x,Pos1.y),Point(Pos2.x,Pos2.y)); // !!!
}

void SgfAreaColorIntens(sal_uInt16 Muster, sal_uInt8 Col1, sal_uInt8 Col2, sal_uInt8 Int, OutputDevice& rOut)
{
    ObjAreaType F;
    F.FMuster=Muster;
    F.FFarbe=Col2;
    F.FBFarbe=Col1;
    F.FIntens=Int;
    SetArea(F,rOut);
}

void DrawSlideRect(sal_Int16 x1, sal_Int16 y1, sal_Int16 x2, sal_Int16 y2, ObjAreaType& F, OutputDevice& rOut)
{
    sal_Int16 i,i0,b,b0;
    sal_Int16 Int1,Int2;
    sal_Int16 Col1,Col2;
    //     ClipMerk: HgdClipRec;
    sal_Int16 cx,cy;
    sal_Int16 MaxR;
    sal_Int32 dx,dy;

    rOut.SetLineColor();
    if (x1>x2) { i=x1; x1=x2; x2=i; }
    if (y1>y2) { i=y1; y1=y2; y2=i; }
    Col1=F.FBFarbe & 0x87; Col2=F.FFarbe & 0x87;
    Int1=100-F.FIntens; Int2=F.FIntens;
    if (Int1==Int2) {
        SgfAreaColorIntens(F.FMuster,(sal_uInt8)Col1,(sal_uInt8)Col2,(sal_uInt8)Int2,rOut);
        rOut.DrawRect(tools::Rectangle(x1,y1,x2,y2));
    } else {
        b0=Int1;
        switch (F.FBFarbe & 0x38) {
            case 0x08: { // vertikal
                i0=y1;
                i=y1;
                while (i<=y2) {
                    b=Int1+sal_Int16((sal_Int32)(Int2-Int1)*(sal_Int32)(i-y1) /(sal_Int32)(y2-y1+1));
                    if (b!=b0) {
                        SgfAreaColorIntens(F.FMuster,(sal_uInt8)Col1,(sal_uInt8)Col2,(sal_uInt8)b0,rOut);
                        rOut.DrawRect(tools::Rectangle(x1,i0,x2,i-1));
                        i0=i; b0=b;
                    }
                    i++;
                }
                SgfAreaColorIntens(F.FMuster,(sal_uInt8)Col1,(sal_uInt8)Col2,(sal_uInt8)Int2,rOut);
                rOut.DrawRect(tools::Rectangle(x1,i0,x2,y2));
            } break;
            case 0x28: { // horizontal
                i0=x1;
                i=x1;
                while (i<=x2) {
                    b=Int1+sal_Int16((sal_Int32)(Int2-Int1)*(sal_Int32)(i-x1) /(sal_Int32)(x2-x1+1));
                    if (b!=b0) {
                        SgfAreaColorIntens(F.FMuster,(sal_uInt8)Col1,(sal_uInt8)Col2,(sal_uInt8)b0,rOut);
                        rOut.DrawRect(tools::Rectangle(i0,y1,i-1,y2));
                        i0=i; b0=b;
                    }
                    i++;
                }
                SgfAreaColorIntens(F.FMuster,(sal_uInt8)Col1,(sal_uInt8)Col2,(sal_uInt8)Int2,rOut);
                rOut.DrawRect(tools::Rectangle(i0,y1,x2,y2));
            } break;

            case 0x18: case 0x38: { // circle
                vcl::Region ClipMerk=rOut.GetClipRegion();
                double a;

                rOut.SetClipRegion(vcl::Region(tools::Rectangle(x1,y1,x2,y2)));
                cx=(x1+x2) /2;
                cy=(y1+y2) /2;
                dx=x2-x1+1;
                dy=y2-y1+1;
                a=sqrt((double)(dx*dx+dy*dy));
                MaxR=sal_Int16(a) /2 +1;
                b0=Int2;
                i0=MaxR; if (MaxR<1) MaxR=1;
                i=MaxR;
                while (i>=0) {
                    b=Int1+sal_Int16((sal_Int32(Int2-Int1)*sal_Int32(i)) /sal_Int32(MaxR));
                    if (b!=b0) {
                        SgfAreaColorIntens(F.FMuster,(sal_uInt8)Col1,(sal_uInt8)Col2,(sal_uInt8)b0,rOut);
                        rOut.DrawEllipse(tools::Rectangle(cx-i0,cy-i0,cx+i0,cy+i0));
                        i0=i; b0=b;
                    }
                    i--;
                }
                SgfAreaColorIntens(F.FMuster,(sal_uInt8)Col1,(sal_uInt8)Col2,(sal_uInt8)Int1,rOut);
                rOut.DrawEllipse(tools::Rectangle(cx-i0,cy-i0,cx+i0,cy+i0));
                rOut.SetClipRegion(ClipMerk);
            } break; // circle
        }
    }
}

void RectType::Draw(OutputDevice& rOut, UCHAR *)
{
    if (L.LMuster!=0) L.LMuster=1; // no line separator here, only on or off
    SetArea(F,rOut);
    if (RotationAngle==0) {
    if ((F.FBFarbe & 0x38)==0 || Radius!=0) {
            SetLine(L,rOut);
            rOut.DrawRect(tools::Rectangle(Pos1.x,Pos1.y,Pos2.x,Pos2.y),Radius,Radius);
        } else {
            DrawSlideRect(Pos1.x,Pos1.y,Pos2.x,Pos2.y,F,rOut);
            if (L.LMuster!=0) {
                SetLine(L,rOut);
                rOut.SetFillColor();
                rOut.DrawRect(tools::Rectangle(Pos1.x,Pos1.y,Pos2.x,Pos2.y));
            }
        }
    } else {
        Point  aPts[4];
        sal_uInt16 i;
        double sn,cs;
        sn=sin(double(RotationAngle)*3.14159265359/18000);
        cs=cos(double(RotationAngle)*3.14159265359/18000);
        aPts[0]=Point(Pos1.x,Pos1.y);
        aPts[1]=Point(Pos2.x,Pos1.y);
        aPts[2]=Point(Pos2.x,Pos2.y);
        aPts[3]=Point(Pos1.x,Pos2.y);
        for (i=0;i<4;i++) {
            RotatePoint(aPts[i],Pos1.x,Pos1.y,sn,cs);
        }
        SetLine(L,rOut);
        tools::Polygon aPoly(4,aPts);
        rOut.DrawPolygon(aPoly);
    }
}

void PolyType::Draw(OutputDevice& rOut, UCHAR *)
{
    if ((Flags & PolyClosBit) !=0) SetArea(F,rOut);
    SetLine(L,rOut);
    tools::Polygon aPoly(nPoints);
    sal_uInt16 i;
    for(i=0;i<nPoints;i++) aPoly.SetPoint(Point(EckP[i].x,EckP[i].y),i);
    if ((Flags & PolyClosBit) !=0) {
        rOut.DrawPolygon(aPoly);
    } else {
        rOut.DrawPolyLine(aPoly);
    }
}

void SplnType::Draw(OutputDevice& rOut, UCHAR *)
{
    if ((Flags & PolyClosBit) !=0) SetArea(F,rOut);
    SetLine(L,rOut);
    tools::Polygon aPoly(0);
    tools::Polygon aSpln(nPoints);
    sal_uInt16 i;
    for(i=0;i<nPoints;i++) aSpln.SetPoint(Point(EckP[i].x,EckP[i].y),i);
    if ((Flags & PolyClosBit) !=0) {
        Spline2Poly(aSpln,true,aPoly);
        if (aPoly.GetSize()>0) rOut.DrawPolygon(aPoly);
    } else {
        Spline2Poly(aSpln,false,aPoly);
        if (aPoly.GetSize()>0) rOut.DrawPolyLine(aPoly);
    }
}

void DrawSlideCirc(sal_Int16 cx, sal_Int16 cy, sal_Int16 rx, sal_Int16 ry, ObjAreaType& F, OutputDevice& rOut)
{
    sal_Int16 x1=cx-rx;
    sal_Int16 y1=cy-ry;
    sal_Int16 x2=cx+rx;
    sal_Int16 y2=cy+ry;

    sal_Int16 i,i0,b,b0;
    sal_Int16 Int1,Int2;
    sal_Int16 Col1,Col2;

    rOut.SetLineColor();
    Col1=F.FBFarbe & 0x87; Col2=F.FFarbe & 0x87;
    Int1=100-F.FIntens; Int2=F.FIntens;
    if (Int1==Int2) {
        SgfAreaColorIntens(F.FMuster,(sal_uInt8)Col1,(sal_uInt8)Col2,(sal_uInt8)Int2,rOut);
        rOut.DrawEllipse(tools::Rectangle(x1,y1,x2,y2));
    } else {
        b0=Int1;
        switch (F.FBFarbe & 0x38) {
            case 0x08: { // vertical
                vcl::Region ClipMerk=rOut.GetClipRegion();
                i0=y1;
                i=y1;
                while (i<=y2) {
                    b=Int1+sal_Int16((sal_Int32)(Int2-Int1)*(sal_Int32)(i-y1) /(sal_Int32)(y2-y1+1));
                    if (b!=b0) {
                        SgfAreaColorIntens(F.FMuster,(sal_uInt8)Col1,(sal_uInt8)Col2,(sal_uInt8)b0,rOut);
                        rOut.SetClipRegion(vcl::Region(tools::Rectangle(x1,i0,x2,i-1)));
                        rOut.DrawEllipse(tools::Rectangle(x1,y1,x2,y2));
                        i0=i; b0=b;
                    }
                    i++;
                }
                SgfAreaColorIntens(F.FMuster,(sal_uInt8)Col1,(sal_uInt8)Col2,(sal_uInt8)Int2,rOut);
                rOut.SetClipRegion(vcl::Region(tools::Rectangle(x1,i0,x2,y2)));
                rOut.DrawEllipse(tools::Rectangle(x1,y1,x2,y2));
                rOut.SetClipRegion(ClipMerk);
            } break;
            case 0x28: { // horizontal
                vcl::Region ClipMerk=rOut.GetClipRegion();
                i0=x1;
                i=x1;
                while (i<=x2) {
                    b=Int1+sal_Int16((sal_Int32)(Int2-Int1)*(sal_Int32)(i-x1) /(sal_Int32)(x2-x1+1));
                    if (b!=b0) {
                        SgfAreaColorIntens(F.FMuster,(sal_uInt8)Col1,(sal_uInt8)Col2,(sal_uInt8)b0,rOut);
                        rOut.SetClipRegion(vcl::Region(tools::Rectangle(i0,y1,i-1,y2)));
                        rOut.DrawEllipse(tools::Rectangle(x1,y1,x2,y2));
                        i0=i; b0=b;
                    }
                    i++;
                }
                SgfAreaColorIntens(F.FMuster,(sal_uInt8)Col1,(sal_uInt8)Col2,(sal_uInt8)Int2,rOut);
                rOut.SetClipRegion(vcl::Region(tools::Rectangle(i0,y1,x2,y2)));
                rOut.DrawEllipse(tools::Rectangle(x1,y1,x2,y2));
                rOut.SetClipRegion(ClipMerk);
            } break;

            case 0x18: case 0x38: { // circle
                sal_Int16 MaxR;

                if (rx<1) rx=1;
                if (ry<1) ry=1;
                MaxR=rx;
                b0=Int2;
                i0=MaxR;
                i=MaxR;
                while (i>=0) {
                    b=Int1+sal_Int16((sal_Int32(Int2-Int1)*sal_Int32(i)) /sal_Int32(MaxR));
                    if (b!=b0) {
                        sal_Int32 temp=sal_Int32(i0)*sal_Int32(ry)/sal_Int32(rx);
                        sal_Int16 j0=sal_Int16(temp);
                        SgfAreaColorIntens(F.FMuster,(sal_uInt8)Col1,(sal_uInt8)Col2,(sal_uInt8)b0,rOut);
                        rOut.DrawEllipse(tools::Rectangle(cx-i0,cy-j0,cx+i0,cy+j0));
                        i0=i; b0=b;
                    }
                    i--;
                }
                SgfAreaColorIntens(F.FMuster,(sal_uInt8)Col1,(sal_uInt8)Col2,(sal_uInt8)Int1,rOut);
                rOut.DrawEllipse(tools::Rectangle(cx-i0,cy-i0,cx+i0,cy+i0));
            } break; // circle
        }
    }
}

void CircType::Draw(OutputDevice& rOut, UCHAR *)
{
    tools::Rectangle aRect(Center.x-Radius.x,Center.y-Radius.y,Center.x+Radius.x,Center.y+Radius.y);

    if (L.LMuster!=0) L.LMuster=1; // no line pattern here, only on or off
    SetArea(F,rOut);
    if ((Flags & 0x03)==CircFull) {
        if ((F.FBFarbe & 0x38)==0) {
            SetLine(L,rOut);
            rOut.DrawEllipse(aRect);
        } else {
            DrawSlideCirc(Center.x,Center.y,Radius.x,Radius.y,F,rOut);
            if (L.LMuster!=0) {
                SetLine(L,rOut);
                rOut.SetFillColor();
                rOut.DrawEllipse(aRect);
            }
        }
    } else {
        PointType a,b;
        Point  aStrt,aEnde;
        double sn,cs;

        a.x=Center.x+Radius.x; a.y=Center.y; b=a;
        sn=sin(double(StartAngle)*3.14159265359/18000);
        cs=cos(double(StartAngle)*3.14159265359/18000);
        RotatePoint(a,Center.x,Center.y,sn,cs);
        sn=sin(double(StartAngle+RelAngle)*3.14159265359/18000);
        cs=cos(double(StartAngle+RelAngle)*3.14159265359/18000);
        RotatePoint(b,Center.x,Center.y,sn,cs);
        if (Radius.x!=Radius.y) {
          if (Radius.x<1) Radius.x=1;
          if (Radius.y<1) Radius.y=1;
          a.y = a.y - Center.y;
          b.y = b.y - Center.y;
          a.y=iMulDiv(a.y,Radius.y,Radius.x);
          b.y=iMulDiv(b.y,Radius.y,Radius.x);
          a.y = a.y + Center.y;
          b.y = b.y + Center.y;
        }
        aStrt=Point(a.x,a.y);
        aEnde=Point(b.x,b.y);
        SetLine(L,rOut);
        switch (Flags & 0x03) {
            case CircArc : rOut.DrawArc(aRect,aEnde,aStrt); break;
            case CircSect:
            case CircAbsn: rOut.DrawPie(aRect,aEnde,aStrt); break;
        }
    }
}

void BmapType::Draw(OutputDevice& rOut, UCHAR *)
{
    //ifstream aInp;
    sal_uInt16      nVersion;
    OUString        aStr(
        reinterpret_cast< char const * >(&Filename[ 1 ]),
        (sal_Int32)Filename[ 0 ], RTL_TEXTENCODING_UTF8 );
    INetURLObject   aFNam( aStr );

    SvStream* pInp = ::utl::UcbStreamHelper::CreateStream( aFNam.GetMainURL( INetURLObject::DecodeMechanism::NONE ), StreamMode::READ );
    if ( pInp )
    {
        unsigned char nSgfTyp = CheckSgfTyp( *pInp,nVersion);
        switch(nSgfTyp) {
            case SGF_BITIMAGE: {
                GraphicFilter aFlt;
                Graphic aGrf;
                aFlt.ImportGraphic(aGrf,aFNam);
                aGrf.Draw(&rOut,Point(Pos1.x,Pos1.y),Size(Pos2.x-Pos1.x,Pos2.y-Pos1.y));
            } break;
            case SGF_SIMPVECT: {
                GDIMetaFile aMtf;
                SgfVectXofs=Pos1.x;
                SgfVectYofs=Pos1.y;
                SgfVectXmul=Pos2.x-Pos1.x;
                SgfVectYmul=Pos2.y-Pos1.y;
                SgfVectXdiv=0;
                SgfVectYdiv=0;
                SgfVectScal=true;
                SgfVectFilter(*pInp,aMtf);
                SgfVectXofs=0;
                SgfVectYofs=0;
                SgfVectXmul=0;
                SgfVectYmul=0;
                SgfVectXdiv=0;
                SgfVectYdiv=0;
                SgfVectScal=false;
                aMtf.Play(&rOut);
            } break;
        }
        delete pInp;
    }
}

sal_uInt32 GrupType::GetSubPtr()
{
    return sal_uInt32(SbLo)+0x00010000*sal_uInt32(SbHi);
}

void DrawObjkList( SvStream& rInp, OutputDevice& rOut )
{
    ObjkType aObjk;
    sal_uInt16 nGrpCnt=0;
    bool bEnd=false;
    do {
        ReadObjkType( rInp, aObjk );
        if (!rInp.GetError()) {
            switch(aObjk.Art) {
                case ObjStrk: { StrkType aStrk; ReadStrkType( rInp, aStrk ); if (!rInp.GetError()) aStrk.Draw(rOut, nullptr); } break;
                case ObjRect: { RectType aRect; ReadRectType( rInp, aRect ); if (!rInp.GetError()) aRect.Draw(rOut, nullptr); } break;
                case ObjCirc: { CircType aCirc; ReadCircType( rInp, aCirc ); if (!rInp.GetError()) aCirc.Draw(rOut, nullptr); } break;
                case ObjText: {
                    TextType aText;
                    ReadTextType( rInp, aText );
                    if (!rInp.GetError()) {
                        const size_t nRemainingSize = rInp.remainingSize();
                        size_t nSize = aText.BufSize;
                        if (nSize > nRemainingSize)
                        {
                            SAL_WARN("vcl", "file is shorter than requested len");
                            nSize = nRemainingSize;
                        }
                        std::vector<UCHAR> aBuffer(nSize+1); // add one for LookAhead at CK-separation
                        size_t nReadSize = rInp.ReadBytes(aBuffer.data(), nSize);
                        aBuffer[nReadSize] = 0;
                        if (!rInp.GetError() && nReadSize == aText.BufSize)
                            aText.Draw(rOut, aBuffer.data());
                    }
                } break;
                case ObjBmap: {
                    BmapType aBmap;
                    ReadBmapType( rInp, aBmap );
                    if (!rInp.GetError()) {
                        aBmap.Draw(rOut, nullptr);
                    }
                } break;
                case ObjPoly: {
                    PolyType aPoly;
                    ReadPolyType( rInp, aPoly );
                    if (!rInp.GetError()) {
                        aPoly.EckP=new PointType[aPoly.nPoints];
                        for (int i = 0; i < aPoly.nPoints; ++i)
                        {
                            rInp.ReadInt16(aPoly.EckP[i].x);
                            rInp.ReadInt16(aPoly.EckP[i].y);
                        }
                        if (!rInp.GetError()) aPoly.Draw(rOut, nullptr);
                        delete[] aPoly.EckP;
                    }
                } break;
                case ObjSpln: {
                    SplnType aSpln;
                    ReadSplnType( rInp, aSpln );
                    if (!rInp.GetError()) {
                        aSpln.EckP=new PointType[aSpln.nPoints];
                        for (int i = 0; i < aSpln.nPoints; ++i)
                        {
                            rInp.ReadInt16(aSpln.EckP[i].x);
                            rInp.ReadInt16(aSpln.EckP[i].y);
                        }
                        if (!rInp.GetError()) aSpln.Draw(rOut, nullptr);
                        delete[] aSpln.EckP;
                    }
                } break;
                case ObjGrup: {
                    GrupType aGrup;
                    ReadGrupType( rInp, aGrup );
                    if (!rInp.GetError()) {
                        rInp.Seek(rInp.Tell()+aGrup.Last);   // object appendix
                        if(aGrup.GetSubPtr()!=0) nGrpCnt++; // DrawObjkList(rInp,rOut );
                    }
                } break;
                default: {
                    aObjk.Draw(rOut, nullptr); // object name on 2. Screen
                    ObjkOverSeek(rInp,aObjk);  // to next object
                }
            }
        } // if rInp
        if (!rInp.GetError()) {
            if (aObjk.Next==0) {
                if (nGrpCnt==0) bEnd=true;
                else nGrpCnt--;
            }
        } else {
            bEnd=true;  // read error
        }
    } while (!bEnd);
}

void SkipObjkList(SvStream& rInp)
{
    ObjkType aObjk;
    do
    {
        ReadObjkType( rInp, aObjk );
        if(aObjk.Art==ObjGrup) {
            GrupType aGrup;
            ReadGrupType( rInp, aGrup );
            rInp.Seek(rInp.Tell()+aGrup.Last); // object appendix
            if(aGrup.GetSubPtr()!=0) SkipObjkList(rInp);
        } else {
            ObjkOverSeek(rInp,aObjk);  // to next object
        }
    } while (aObjk.Next!=0 && !rInp.GetError());
}

bool SgfFilterSDrw( SvStream& rInp, GDIMetaFile& rMtf )
{
    bool          bRet = false;
    PageType      aPage;
    ScopedVclPtrInstance< VirtualDevice > aOutDev;
    OutputDevice* pOutDev;
    sal_uLong         nStdPos;
    sal_uLong         nCharPos;
    sal_uInt16        Num;

    pOutDev=aOutDev.get();
    DtHdOverSeek(rInp); // read dataheader

    nStdPos=rInp.Tell();
    do {                // read standard page
        ReadPageType( rInp, aPage );
        if (aPage.nList!=0) SkipObjkList(rInp);
    } while (aPage.Next!=0 && !rInp.GetError());

    nCharPos=rInp.Tell();
    ReadPageType( rInp, aPage );

    rMtf.Record(pOutDev);
    Num=aPage.StdPg;
    if (Num!=0) {
      rInp.Seek(nStdPos);
      while(Num>1 && aPage.Next!=0 && !rInp.GetError()) { // search standard page
        ReadPageType( rInp, aPage );
        if (aPage.nList!=0) SkipObjkList(rInp);
        Num--;
      }
      ReadPageType( rInp, aPage );
      if(Num==1 && aPage.nList!=0) DrawObjkList( rInp,*pOutDev );
      rInp.Seek(nCharPos);
      ReadPageType( rInp, aPage );
    }
    if (aPage.nList!=0) DrawObjkList(rInp,*pOutDev );

    rMtf.Stop();
    rMtf.WindStart();
    MapMode aMap(MapUnit::Map10thMM,Point(),Fraction(1,4),Fraction(1,4));
    rMtf.SetPrefMapMode(aMap);
    rMtf.SetPrefSize(Size(aPage.Paper.Size.x, aPage.Paper.Size.y));
    bRet=true;
    return bRet;
}

bool SgfSDrwFilter(SvStream& rInp, GDIMetaFile& rMtf, const INetURLObject& _aIniPath )
{
    sal_uLong   nFileStart;        // offset of SgfHeaders. In general 0.
    SgfHeader   aHead;
    SgfEntry    aEntr;
    sal_uLong   nNext;
    bool        bRet=false;        // return value

    INetURLObject aIniPath = _aIniPath;
    aIniPath.Append("sgf.ini");

    pSgfFonts = new SgfFontLst;

    pSgfFonts->AssignFN( aIniPath.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
    nFileStart=rInp.Tell();
    ReadSgfHeader( rInp, aHead );
    if (aHead.ChkMagic() && aHead.Typ==SgfStarDraw && aHead.Version==SGV_VERSION) {
        nNext=aHead.GetOffset();
        while (nNext && !rInp.GetError()) {
            rInp.Seek(nFileStart+nNext);
            ReadSgfEntry( rInp, aEntr );
            nNext=aEntr.GetOffset();
            if (aEntr.Typ==aHead.Typ) {
                bRet=SgfFilterSDrw( rInp,rMtf );
            }
        } // while(nNext)
    }
    delete pSgfFonts;
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
