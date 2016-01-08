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

#if defined OSL_BIGENDIAN

#define SWAPPOINT(p) {  \
    p.x=OSL_SWAPWORD(p.x); \
    p.y=OSL_SWAPWORD(p.y); }

#define SWAPPAGE(p) {                         \
    p.Next   =OSL_SWAPDWORD (p.Next   );           \
    p.nList  =OSL_SWAPDWORD (p.nList  );           \
    p.ListEnd=OSL_SWAPDWORD (p.ListEnd);           \
    p.Paper.Size.x=OSL_SWAPWORD(p.Paper.Size.x); \
    p.Paper.Size.y=OSL_SWAPWORD(p.Paper.Size.y); \
    p.Paper.RandL =OSL_SWAPWORD(p.Paper.RandL ); \
    p.Paper.RandR =OSL_SWAPWORD(p.Paper.RandR ); \
    p.Paper.RandO =OSL_SWAPWORD(p.Paper.RandO ); \
    p.Paper.RandU =OSL_SWAPWORD(p.Paper.RandU ); \
    SWAPPOINT(p.U);                           \
    sal_uInt16 iTemp;                             \
    for (iTemp=0;iTemp<20;iTemp++) {          \
        rPage.HlpLnH[iTemp]=OSL_SWAPWORD(rPage.HlpLnH[iTemp]);       \
        rPage.HlpLnV[iTemp]=OSL_SWAPWORD(rPage.HlpLnV[iTemp]);      }}

#define SWAPOBJK(o) {                 \
    o.Last    =OSL_SWAPDWORD (o.Last    ); \
    o.Next    =OSL_SWAPDWORD (o.Next    ); \
    o.MemSize =OSL_SWAPWORD(o.MemSize ); \
    SWAPPOINT(o.ObjMin);              \
    SWAPPOINT(o.ObjMax);              }

#define SWAPLINE(l) {             \
    l.LMSize=OSL_SWAPWORD(l.LMSize); \
    l.LDicke=OSL_SWAPWORD(l.LDicke); }

#define SWAPAREA(a) {               \
    a.FDummy2=OSL_SWAPWORD(a.FDummy2); \
    a.FMuster=OSL_SWAPWORD(a.FMuster); }

#define SWAPTEXT(t) {               \
    SWAPLINE(t.L);                  \
    SWAPAREA(t.F);                  \
    t.FontLo =OSL_SWAPWORD(t.FontLo ); \
    t.FontHi =OSL_SWAPWORD(t.FontHi ); \
    t.Grad   =OSL_SWAPWORD(t.Grad   ); \
    t.Breite =OSL_SWAPWORD(t.Breite ); \
    t.Schnitt=OSL_SWAPWORD(t.Schnitt); \
    t.LnFeed =OSL_SWAPWORD(t.LnFeed ); \
    t.Slant  =OSL_SWAPWORD(t.Slant  ); \
    SWAPLINE(t.ShdL);               \
    SWAPAREA(t.ShdF);               \
    SWAPPOINT(t.ShdVers);           \
    SWAPAREA(t.BackF);              }

#endif

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
    rIStream.Read(&rPage.Next, PageSize);
#if defined OSL_BIGENDIAN
    SWAPPAGE(rPage);
#endif
    return rIStream;
}

void ObjkOverSeek(SvStream& rInp, ObjkType& rObjk)
{
    sal_uLong Siz;
    Siz=(sal_uLong)rObjk.MemSize+rObjk.Last;  // ObjSize+ObjAnhSize
    rInp.Seek(rInp.Tell()+Siz);
}

SvStream& ReadObjkType(SvStream& rInp, ObjkType& rObjk)
{
    // fileposition in stream is not changed!
    sal_uLong nPos;
    nPos=rInp.Tell();
    rInp.Read(&rObjk.Last, ObjkSize);
#if defined OSL_BIGENDIAN
    SWAPOBJK(rObjk);
#endif
    rInp.Seek(nPos);
    return rInp;
}
SvStream& ReadStrkType(SvStream& rInp, StrkType& rStrk)
{
    rInp.Read(&rStrk.Last, StrkSize);
#if defined OSL_BIGENDIAN
    SWAPOBJK (rStrk);
    SWAPLINE (rStrk.L);
    SWAPPOINT(rStrk.Pos1);
    SWAPPOINT(rStrk.Pos2);
#endif
    return rInp;
}
SvStream& ReadRectType(SvStream& rInp, RectType& rRect)
{
    rInp.Read(&rRect.Last, RectSize);
#if defined OSL_BIGENDIAN
    SWAPOBJK (rRect);
    SWAPLINE (rRect.L);
    SWAPAREA (rRect.F);
    SWAPPOINT(rRect.Pos1);
    SWAPPOINT(rRect.Pos2);
    rRect.Radius        = OSL_SWAPWORD(rRect.Radius  );
    rRect.RotationAngle = OSL_SWAPWORD(rRect.RotationAngle);
    rRect.Slant         = OSL_SWAPWORD(rRect.Slant   );
#endif
    return rInp;
}
SvStream& ReadPolyType(SvStream& rInp, PolyType& rPoly)
{
    rInp.Read(&rPoly.Last, PolySize);
#if defined OSL_BIGENDIAN
    SWAPOBJK (rPoly);
    SWAPLINE (rPoly.L);
    SWAPAREA (rPoly.F);
#endif
    return rInp;
}
SvStream& ReadSplnType(SvStream& rInp, SplnType& rSpln)
{
    rInp.Read(&rSpln.Last, SplnSize);
#if defined OSL_BIGENDIAN
    SWAPOBJK (rSpln);
    SWAPLINE (rSpln.L);
    SWAPAREA (rSpln.F);
#endif
    return rInp;
}
SvStream& ReadCircType(SvStream& rInp, CircType& rCirc)
{
    rInp.Read(&rCirc.Last, CircSize);
#if defined OSL_BIGENDIAN
    SWAPOBJK (rCirc);
    SWAPLINE (rCirc.L);
    SWAPAREA (rCirc.F);
    SWAPPOINT(rCirc.Radius);
    SWAPPOINT(rCirc.Center);
    rCirc.RotationAngle = OSL_SWAPWORD(rCirc.RotationAngle );
    rCirc.StartAngle     = OSL_SWAPWORD(rCirc.StartAngle);
    rCirc.RelAngle       = OSL_SWAPWORD(rCirc.RelAngle  );
#endif
    return rInp;
}
SvStream& ReadTextType(SvStream& rInp, TextType& rText)
{
    rInp.Read(&rText.Last, TextSize);
#if defined OSL_BIGENDIAN
    SWAPOBJK (rText);
    SWAPTEXT (rText.T);
    SWAPPOINT(rText.Pos1);
    SWAPPOINT(rText.Pos2);
    rText.TopOfs        = OSL_SWAPWORD(rText.TopOfs  );
    rText.RotationAngle = OSL_SWAPWORD(rText.RotationAngle);
    rText.BoxSlant      = OSL_SWAPWORD(rText.BoxSlant);
    rText.BufSize       = OSL_SWAPWORD(rText.BufSize );
    SWAPPOINT(rText.FitSize);
    rText.FitBreit      = OSL_SWAPWORD(rText.FitBreit);
#endif
    rText.Buffer=nullptr;
    return rInp;
}
SvStream& ReadBmapType(SvStream& rInp, BmapType& rBmap)
{
    rInp.Read(&rBmap.Last, BmapSize);
#if defined OSL_BIGENDIAN
    SWAPOBJK (rBmap);
    SWAPAREA (rBmap.F);
    SWAPPOINT(rBmap.Pos1);
    SWAPPOINT(rBmap.Pos2);
    rBmap.RotationAngle = OSL_SWAPWORD(rBmap.RotationAngle);
    rBmap.Slant         = OSL_SWAPWORD(rBmap.Slant   );
    SWAPPOINT(rBmap.PixSize);
#endif
    return rInp;
}
SvStream& ReadGrupType(SvStream& rInp, GrupType& rGrup)
{
    rInp.Read(&rGrup.Last, GrupSize);
#if defined OSL_BIGENDIAN
    SWAPOBJK (rGrup);
    rGrup.SbLo     =OSL_SWAPWORD(rGrup.SbLo     );
    rGrup.SbHi     =OSL_SWAPWORD(rGrup.SbHi     );
    rGrup.UpLo     =OSL_SWAPWORD(rGrup.UpLo     );
    rGrup.UpHi     =OSL_SWAPWORD(rGrup.UpHi     );
    rGrup.ChartSize=OSL_SWAPWORD(rGrup.ChartSize);
    rGrup.ChartPtr =OSL_SWAPDWORD (rGrup.ChartPtr );
#endif
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

void ObjkType::Draw(OutputDevice&)
{
}

void StrkType::Draw(OutputDevice& rOut)
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
        rOut.DrawRect(Rectangle(x1,y1,x2,y2));
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
                        rOut.DrawRect(Rectangle(x1,i0,x2,i-1));
                        i0=i; b0=b;
                    }
                    i++;
                }
                SgfAreaColorIntens(F.FMuster,(sal_uInt8)Col1,(sal_uInt8)Col2,(sal_uInt8)Int2,rOut);
                rOut.DrawRect(Rectangle(x1,i0,x2,y2));
            } break;
            case 0x28: { // horizontal
                i0=x1;
                i=x1;
                while (i<=x2) {
                    b=Int1+sal_Int16((sal_Int32)(Int2-Int1)*(sal_Int32)(i-x1) /(sal_Int32)(x2-x1+1));
                    if (b!=b0) {
                        SgfAreaColorIntens(F.FMuster,(sal_uInt8)Col1,(sal_uInt8)Col2,(sal_uInt8)b0,rOut);
                        rOut.DrawRect(Rectangle(i0,y1,i-1,y2));
                        i0=i; b0=b;
                    }
                    i++;
                }
                SgfAreaColorIntens(F.FMuster,(sal_uInt8)Col1,(sal_uInt8)Col2,(sal_uInt8)Int2,rOut);
                rOut.DrawRect(Rectangle(i0,y1,x2,y2));
            } break;

            case 0x18: case 0x38: { // circle
                vcl::Region ClipMerk=rOut.GetClipRegion();
                double a;

                rOut.SetClipRegion(vcl::Region(Rectangle(x1,y1,x2,y2)));
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
                        rOut.DrawEllipse(Rectangle(cx-i0,cy-i0,cx+i0,cy+i0));
                        i0=i; b0=b;
                    }
                    i--;
                }
                SgfAreaColorIntens(F.FMuster,(sal_uInt8)Col1,(sal_uInt8)Col2,(sal_uInt8)Int1,rOut);
                rOut.DrawEllipse(Rectangle(cx-i0,cy-i0,cx+i0,cy+i0));
                rOut.SetClipRegion(ClipMerk);
            } break; // circle
        }
    }
}

void RectType::Draw(OutputDevice& rOut)
{
    if (L.LMuster!=0) L.LMuster=1; // no line separator here, only on or off
    SetArea(F,rOut);
    if (RotationAngle==0) {
    if ((F.FBFarbe & 0x38)==0 || Radius!=0) {
            SetLine(L,rOut);
            rOut.DrawRect(Rectangle(Pos1.x,Pos1.y,Pos2.x,Pos2.y),Radius,Radius);
        } else {
            DrawSlideRect(Pos1.x,Pos1.y,Pos2.x,Pos2.y,F,rOut);
            if (L.LMuster!=0) {
                SetLine(L,rOut);
                rOut.SetFillColor();
                rOut.DrawRect(Rectangle(Pos1.x,Pos1.y,Pos2.x,Pos2.y));
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

void PolyType::Draw(OutputDevice& rOut)
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

void SplnType::Draw(OutputDevice& rOut)
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
        rOut.DrawEllipse(Rectangle(x1,y1,x2,y2));
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
                        rOut.SetClipRegion(vcl::Region(Rectangle(x1,i0,x2,i-1)));
                        rOut.DrawEllipse(Rectangle(x1,y1,x2,y2));
                        i0=i; b0=b;
                    }
                    i++;
                }
                SgfAreaColorIntens(F.FMuster,(sal_uInt8)Col1,(sal_uInt8)Col2,(sal_uInt8)Int2,rOut);
                rOut.SetClipRegion(vcl::Region(Rectangle(x1,i0,x2,y2)));
                rOut.DrawEllipse(Rectangle(x1,y1,x2,y2));
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
                        rOut.SetClipRegion(vcl::Region(Rectangle(i0,y1,i-1,y2)));
                        rOut.DrawEllipse(Rectangle(x1,y1,x2,y2));
                        i0=i; b0=b;
                    }
                    i++;
                }
                SgfAreaColorIntens(F.FMuster,(sal_uInt8)Col1,(sal_uInt8)Col2,(sal_uInt8)Int2,rOut);
                rOut.SetClipRegion(vcl::Region(Rectangle(i0,y1,x2,y2)));
                rOut.DrawEllipse(Rectangle(x1,y1,x2,y2));
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
                        rOut.DrawEllipse(Rectangle(cx-i0,cy-j0,cx+i0,cy+j0));
                        i0=i; b0=b;
                    }
                    i--;
                }
                SgfAreaColorIntens(F.FMuster,(sal_uInt8)Col1,(sal_uInt8)Col2,(sal_uInt8)Int1,rOut);
                rOut.DrawEllipse(Rectangle(cx-i0,cy-i0,cx+i0,cy+i0));
            } break; // circle
        }
    }
}

void CircType::Draw(OutputDevice& rOut)
{
    Rectangle aRect(Center.x-Radius.x,Center.y-Radius.y,Center.x+Radius.x,Center.y+Radius.y);

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

void BmapType::Draw(OutputDevice& rOut)
{
    //ifstream aInp;
    sal_uInt16      nVersion;
    OUString        aStr(
        reinterpret_cast< char const * >(&Filename[ 1 ]),
        (sal_Int32)Filename[ 0 ], RTL_TEXTENCODING_UTF8 );
    INetURLObject   aFNam( aStr );

    SvStream* pInp = ::utl::UcbStreamHelper::CreateStream( aFNam.GetMainURL( INetURLObject::NO_DECODE ), StreamMode::READ );
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
                case ObjStrk: { StrkType aStrk; ReadStrkType( rInp, aStrk ); if (!rInp.GetError()) aStrk.Draw(rOut); } break;
                case ObjRect: { RectType aRect; ReadRectType( rInp, aRect ); if (!rInp.GetError()) aRect.Draw(rOut); } break;
                case ObjCirc: { CircType aCirc; ReadCircType( rInp, aCirc ); if (!rInp.GetError()) aCirc.Draw(rOut); } break;
                case ObjText: {
                    TextType aText;
                    ReadTextType( rInp, aText );
                    if (!rInp.GetError()) {
                        aText.Buffer=new UCHAR[aText.BufSize+1]; // add one for LookAhead at CK-separation
                        rInp.Read(aText.Buffer, aText.BufSize);
                        if (!rInp.GetError()) aText.Draw(rOut);
                        delete[] aText.Buffer;
                    }
                } break;
                case ObjBmap: {
                    BmapType aBmap;
                    ReadBmapType( rInp, aBmap );
                    if (!rInp.GetError()) {
                        aBmap.Draw(rOut);
                    }
                } break;
                case ObjPoly: {
                    PolyType aPoly;
                    ReadPolyType( rInp, aPoly );
                    if (!rInp.GetError()) {
                        aPoly.EckP=new PointType[aPoly.nPoints];
                        rInp.Read(aPoly.EckP, 4*aPoly.nPoints);
#if defined OSL_BIGENDIAN
                        for(short i=0;i<aPoly.nPoints;i++) SWAPPOINT(aPoly.EckP[i]);
#endif
                        if (!rInp.GetError()) aPoly.Draw(rOut);
                        delete[] aPoly.EckP;
                    }
                } break;
                case ObjSpln: {
                    SplnType aSpln;
                    ReadSplnType( rInp, aSpln );
                    if (!rInp.GetError()) {
                        aSpln.EckP=new PointType[aSpln.nPoints];
                        rInp.Read(aSpln.EckP, 4*aSpln.nPoints);
#if defined OSL_BIGENDIAN
                        for(short i=0;i<aSpln.nPoints;i++) SWAPPOINT(aSpln.EckP[i]);
#endif
                        if (!rInp.GetError()) aSpln.Draw(rOut);
                        delete[] aSpln.EckP;
                    }
                } break;
                case ObjGrup: {
                    GrupType aGrup;
                    ReadGrupType( rInp, aGrup );
                    if (!rInp.GetError()) {
                        rInp.Seek(rInp.Tell()+aGrup.Last);   // object appendix
                        if(aGrup.GetSubPtr()!=0L) nGrpCnt++; // DrawObjkList(rInp,rOut );
                    }
                } break;
                default: {
                    aObjk.Draw(rOut);          // object name on 2. Screen
                    ObjkOverSeek(rInp,aObjk);  // to next object
                }
            }
        } // if rInp
        if (!rInp.GetError()) {
            if (aObjk.Next==0L) {
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
            if(aGrup.GetSubPtr()!=0L) SkipObjkList(rInp);
        } else {
            ObjkOverSeek(rInp,aObjk);  // to next object
        }
    } while (aObjk.Next!=0L && !rInp.GetError());
}

bool SgfFilterSDrw( SvStream& rInp, SgfHeader&, SgfEntry&, GDIMetaFile& rMtf )
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
    } while (aPage.Next!=0L && !rInp.GetError());

    nCharPos=rInp.Tell();
    ReadPageType( rInp, aPage );

    rMtf.Record(pOutDev);
    Num=aPage.StdPg;
    if (Num!=0) {
      rInp.Seek(nStdPos);
      while(Num>1 && aPage.Next!=0L && !rInp.GetError()) { // search standard page
        ReadPageType( rInp, aPage );
        if (aPage.nList!=0) SkipObjkList(rInp);
        Num--;
      }
      ReadPageType( rInp, aPage );
      if(Num==1 && aPage.nList!=0L) DrawObjkList( rInp,*pOutDev );
      rInp.Seek(nCharPos);
      ReadPageType( rInp, aPage );
    }
    if (aPage.nList!=0L) DrawObjkList(rInp,*pOutDev );

    rMtf.Stop();
    rMtf.WindStart();
    MapMode aMap(MAP_10TH_MM,Point(),Fraction(1,4),Fraction(1,4));
    rMtf.SetPrefMapMode(aMap);
    rMtf.SetPrefSize(Size((sal_Int16)aPage.Paper.Size.x,(sal_Int16)aPage.Paper.Size.y));
    bRet=true;
    return bRet;
}

bool SgfSDrwFilter(SvStream& rInp, GDIMetaFile& rMtf, const INetURLObject& _aIniPath )
{
#if OSL_DEBUG_LEVEL > 1 // check record size. New compiler possibly aligns different!
    if (sizeof(ObjTextType)!=ObjTextTypeSize)  return false;
#endif

    sal_uLong   nFileStart;        // offset of SgfHeaders. In general 0.
    SgfHeader   aHead;
    SgfEntry    aEntr;
    sal_uLong   nNext;
    bool        bRet=false;        // return value

    INetURLObject aIniPath = _aIniPath;
    aIniPath.Append("sgf.ini");

    pSgfFonts = new SgfFontLst;

    pSgfFonts->AssignFN( aIniPath.GetMainURL( INetURLObject::NO_DECODE ) );
    nFileStart=rInp.Tell();
    ReadSgfHeader( rInp, aHead );
    if (aHead.ChkMagic() && aHead.Typ==SgfStarDraw && aHead.Version==SGV_VERSION) {
        nNext=aHead.GetOffset();
        while (nNext && !rInp.GetError()) {
            rInp.Seek(nFileStart+nNext);
            ReadSgfEntry( rInp, aEntr );
            nNext=aEntr.GetOffset();
            if (aEntr.Typ==aHead.Typ) {
                bRet=SgfFilterSDrw( rInp,aHead,aEntr,rMtf );
            }
        } // while(nNext)
    }
    delete pSgfFonts;
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
