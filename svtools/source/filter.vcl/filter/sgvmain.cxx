/*************************************************************************
 *
 *  $RCSfile: sgvmain.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: sj $ $Date: 2001-03-07 19:58:01 $
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

#ifdef MAC
#include <mac_start.h>
#include <math.h>
#include <mac_end.h>
#else
#include <math.h>
#endif
#include <vcl/graph.hxx>
#include <vcl/poly.hxx>
#include "filter.hxx"
#include "sgffilt.hxx"
#include "sgfbram.hxx"
#include "sgvmain.hxx"
#include "sgvspln.hxx"
#ifndef _UNTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif

#ifdef DEBUG
//#include "Debug.c"
#endif

#define SWAPPOINT(p) {  \
    p.x=SWAPSHORT(p.x); \
    p.y=SWAPSHORT(p.y); }

#define SWAPPAGE(p) {                         \
    p.Next   =SWAPLONG (p.Next   );           \
    p.nList  =SWAPLONG (p.nList  );           \
    p.ListEnd=SWAPLONG (p.ListEnd);           \
    p.Paper.Size.x=SWAPSHORT(p.Paper.Size.x); \
    p.Paper.Size.y=SWAPSHORT(p.Paper.Size.y); \
    p.Paper.RandL =SWAPSHORT(p.Paper.RandL ); \
    p.Paper.RandR =SWAPSHORT(p.Paper.RandR ); \
    p.Paper.RandO =SWAPSHORT(p.Paper.RandO ); \
    p.Paper.RandU =SWAPSHORT(p.Paper.RandU ); \
    SWAPPOINT(p.U);                           \
    UINT16 iTemp;                             \
    for (iTemp=0;iTemp<20;iTemp++) {          \
        rPage.HlpLnH[iTemp]=SWAPSHORT(rPage.HlpLnH[iTemp]);       \
        rPage.HlpLnV[iTemp]=SWAPSHORT(rPage.HlpLnV[iTemp]);      }}

#define SWAPOBJK(o) {                 \
    o.Last    =SWAPLONG (o.Last    ); \
    o.Next    =SWAPLONG (o.Next    ); \
    o.MemSize =SWAPSHORT(o.MemSize ); \
    SWAPPOINT(o.ObjMin);              \
    SWAPPOINT(o.ObjMax);              }

#define SWAPLINE(l) {             \
    l.LMSize=SWAPSHORT(l.LMSize); \
    l.LDicke=SWAPSHORT(l.LDicke); }

#define SWAPAREA(a) {               \
    a.FDummy2=SWAPSHORT(a.FDummy2); \
    a.FMuster=SWAPSHORT(a.FMuster); }

#define SWAPTEXT(t) {               \
    SWAPLINE(t.L);                  \
    SWAPAREA(t.F);                  \
    t.FontLo =SWAPSHORT(t.FontLo ); \
    t.FontHi =SWAPSHORT(t.FontHi ); \
    t.Grad   =SWAPSHORT(t.Grad   ); \
    t.Breite =SWAPSHORT(t.Breite ); \
    t.Schnitt=SWAPSHORT(t.Schnitt); \
    t.LnFeed =SWAPSHORT(t.LnFeed ); \
    t.Slant  =SWAPSHORT(t.Slant  ); \
    SWAPLINE(t.ShdL);               \
    SWAPAREA(t.ShdF);               \
    SWAPPOINT(t.ShdVers);           \
    SWAPAREA(t.BackF);              }


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Einschränkungen:
//
//  þ Flächenmuster werden den unter StarView verfügbaren Mustern angenähert.
//  þ Linienenden werden unter StarView immer rund dargestellt und gehen über
//    den Endpunkt der Linie hinaus.
//  þ Linienmuster werden den unter StarView verfügbaren Mustern angenähert.
//    Transparent/Opak wird zur Zeit noch nicht berücksichtigt.
//  þ Keine gedrehten Ellipsen
//
//
//
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined( WIN ) && defined( MSC )
#pragma code_seg( "svtools", "AUTO_CODE" )
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// Für Fontübersetzung /////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
SgfFontLst* pSgfFonts = 0;

#if defined( WIN ) && defined( MSC )
#pragma code_seg( "SVTOOLS_FILTER3", "SVTOOLS_CODE" )
static void AntiMscBug() {}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// Für Kreisunterarten, Text und gedrehte Rechtecke ////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
void RotatePoint(PointType& P, INT16 cx, INT16 cy, double sn, double cs)
{
    INT16  dx,dy;
    double x1,y1;
    dx=P.x-cx;
    dy=P.y-cy;
    x1=dx*cs-dy*sn;
    y1=dy*cs+dx*sn;
    P.x=cx+INT16(x1);
    P.y=cy+INT16(y1);
}

void RotatePoint(Point& P, INT16 cx, INT16 cy, double sn, double cs)
{
    INT16  dx,dy;
    double x1,y1;
    dx=(INT16)(P.X()-cx);
    dy=(INT16)(P.Y()-cy);
    x1=dx*cs-dy*sn;
    y1=dy*cs+dx*sn;
    P=Point(cx+INT16(x1),cy+INT16(y1));
}

INT16 iMulDiv(INT16 a, INT16 Mul, INT16 Div)
{
    INT32 Temp;
    Temp=INT32(a)*INT32(Mul)/INT32(Div);
    return INT16(Temp);
}

UINT16 MulDiv(UINT16 a, UINT16 Mul, UINT16 Div)
{
    UINT32 Temp;
    Temp=UINT32(a)*UINT32(Mul)/UINT32(Div);
    return UINT16(Temp);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// SgfFilterSDrw ///////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

SvStream& operator>>(SvStream& rIStream, DtHdType& rDtHd)
{
    rIStream.Read((char*)&rDtHd.Reserved[0],DtHdSize);
    return rIStream;
}

void DtHdOverSeek(SvStream& rInp)
{
    ULONG FPos=rInp.Tell();
    FPos+=(ULONG)DtHdSize;
    rInp.Seek(FPos);
//    rInp.seekg(rInp.tellg()+(ULONG)DtHdSize);
}


SvStream& operator>>(SvStream& rIStream, PageType& rPage)
{
    rIStream.Read((char*)&rPage.Next,PageSize);
#if defined __BIGENDIAN
    SWAPPAGE(rPage);
#endif
    return rIStream;
}

void ObjkOverSeek(SvStream& rInp, ObjkType& rObjk)
{
    ULONG Siz;
    Siz=(ULONG)rObjk.MemSize+rObjk.Last;  // ObjSize+ObjAnhSize
    rInp.Seek(rInp.Tell()+Siz);
}

SvStream& operator>>(SvStream& rInp, ObjkType& rObjk)
{   // Die Fileposition im Stream bleibt unverändert!
    ULONG nPos;
    nPos=rInp.Tell();
    rInp.Read((char*)&rObjk.Last,ObjkSize);
#if defined __BIGENDIAN
    SWAPOBJK(rObjk);
#endif
#ifdef InArbeit
    ULONG nPos1=rInp.Tell();
    if(nPos == nPos1) InfoBox( NULL, "tellg funkt nich" ).Execute();
#endif
    rInp.Seek(nPos);
#ifdef InArbeit
    if (rInp.Tell() != nPos) InfoBox( NULL, "seekg funkt nich" ).Execute();
#endif
    return rInp;
}
SvStream& operator>>(SvStream& rInp, StrkType& rStrk)
{
    rInp.Read((char*)&rStrk.Last,StrkSize);
#if defined __BIGENDIAN
    SWAPOBJK (rStrk);
    SWAPLINE (rStrk.L);
    SWAPPOINT(rStrk.Pos1);
    SWAPPOINT(rStrk.Pos2);
#endif
    return rInp;
}
SvStream& operator>>(SvStream& rInp, RectType& rRect)
{
    rInp.Read((char*)&rRect.Last,RectSize);
#if defined __BIGENDIAN
    SWAPOBJK (rRect);
    SWAPLINE (rRect.L);
    SWAPAREA (rRect.F);
    SWAPPOINT(rRect.Pos1);
    SWAPPOINT(rRect.Pos2);
    rRect.Radius  =SWAPSHORT(rRect.Radius  );
    rRect.DrehWink=SWAPSHORT(rRect.DrehWink);
    rRect.Slant   =SWAPSHORT(rRect.Slant   );
#endif
    return rInp;
}
SvStream& operator>>(SvStream& rInp, PolyType& rPoly)
{
    rInp.Read((char*)&rPoly.Last,PolySize);
#if defined __BIGENDIAN
    SWAPOBJK (rPoly);
    SWAPLINE (rPoly.L);
    SWAPAREA (rPoly.F);
    // rPoly.EckP=SWAPLONG(rPoly.EckP);
#endif
    return rInp;
}
SvStream& operator>>(SvStream& rInp, SplnType& rSpln)
{
    rInp.Read((char*)&rSpln.Last,SplnSize);
#if defined __BIGENDIAN
    SWAPOBJK (rSpln);
    SWAPLINE (rSpln.L);
    SWAPAREA (rSpln.F);
    // rSpln.EckP=SWAPLONG(rSpln.EckP);
#endif
    return rInp;
}
SvStream& operator>>(SvStream& rInp, CircType& rCirc)
{
    rInp.Read((char*)&rCirc.Last,CircSize);
#if defined __BIGENDIAN
    SWAPOBJK (rCirc);
    SWAPLINE (rCirc.L);
    SWAPAREA (rCirc.F);
    SWAPPOINT(rCirc.Radius);
    SWAPPOINT(rCirc.Center);
    rCirc.DrehWink =SWAPSHORT(rCirc.DrehWink );
    rCirc.StartWink=SWAPSHORT(rCirc.StartWink);
    rCirc.RelWink  =SWAPSHORT(rCirc.RelWink  );
#endif
    return rInp;
}
SvStream& operator>>(SvStream& rInp, TextType& rText)
{
    rInp.Read((char*)&rText.Last,TextSize);
#if defined __BIGENDIAN
    SWAPOBJK (rText);
    SWAPTEXT (rText.T);
    SWAPPOINT(rText.Pos1);
    SWAPPOINT(rText.Pos2);
    rText.TopOfs  =SWAPSHORT(rText.TopOfs  );
    rText.DrehWink=SWAPSHORT(rText.DrehWink);
    rText.BoxSlant=SWAPSHORT(rText.BoxSlant);
    rText.BufSize =SWAPSHORT(rText.BufSize );
    //rText.Buf     =SWAPLONG (rText.Buf     );
    //rText.Ext     =SWAPLONG (rText.Ext     );
    SWAPPOINT(rText.FitSize);
    rText.FitBreit=SWAPSHORT(rText.FitBreit);
#endif
    rText.Buffer=NULL;
    return rInp;
}
SvStream& operator>>(SvStream& rInp, BmapType& rBmap)
{
    rInp.Read((char*)&rBmap.Last,BmapSize);
#if defined __BIGENDIAN
    SWAPOBJK (rBmap);
    SWAPAREA (rBmap.F);
    SWAPPOINT(rBmap.Pos1);
    SWAPPOINT(rBmap.Pos2);
    rBmap.DrehWink=SWAPSHORT(rBmap.DrehWink);
    rBmap.Slant   =SWAPSHORT(rBmap.Slant   );
    SWAPPOINT(rBmap.PixSize);
#endif
    return rInp;
}
SvStream& operator>>(SvStream& rInp, GrupType& rGrup)
{
    rInp.Read((char*)&rGrup.Last,GrupSize);
#if defined __BIGENDIAN
    SWAPOBJK (rGrup);
    rGrup.SbLo     =SWAPSHORT(rGrup.SbLo     );
    rGrup.SbHi     =SWAPSHORT(rGrup.SbHi     );
    rGrup.UpLo     =SWAPSHORT(rGrup.UpLo     );
    rGrup.UpHi     =SWAPSHORT(rGrup.UpHi     );
    rGrup.ChartSize=SWAPSHORT(rGrup.ChartSize);
    rGrup.ChartPtr =SWAPLONG (rGrup.ChartPtr );
#endif
    return rInp;
}



/*************************************************************************
|*
|*    Sgv2SvFarbe()
|*
|*    Beschreibung
|*    Ersterstellung    JOE 23.06.93
|*    Letzte Aenderung  JOE 23.06.93
|*
*************************************************************************/
#ifndef VCL
Color Sgv2SvFarbe(BYTE nFrb1, BYTE nFrb2, BYTE nInts)
{
    UINT16 r1=0,g1=0,b1=0,r2=0,g2=0,b2=0;
    BYTE   nInt2=100-nInts;
    switch(nFrb1 & 0x07) {
        case 0:  r1=0xFFFF; g1=0xFFFF; b1=0xFFFF; break;
        case 1:  r1=0xFFFF; g1=0xFFFF;            break;
        case 2:             g1=0xFFFF; b1=0xFFFF; break;
        case 3:             g1=0xFFFF;            break;
        case 4:  r1=0xFFFF;            b1=0xFFFF; break;
        case 5:  r1=0xFFFF;                       break;
        case 6:                        b1=0xFFFF; break;
        case 7:                                   break;
    }
    switch(nFrb2 & 0x07) {
        case 0:  r2=0xFFFF; g2=0xFFFF; b2=0xFFFF; break;
        case 1:  r2=0xFFFF; g2=0xFFFF;            break;
        case 2:             g2=0xFFFF; b2=0xFFFF; break;
        case 3:             g2=0xFFFF;            break;
        case 4:  r2=0xFFFF;            b2=0xFFFF; break;
        case 5:  r2=0xFFFF;                       break;
        case 6:                        b2=0xFFFF; break;
        case 7:                                   break;
    }
#else
Color Sgv2SvFarbe(BYTE nFrb1, BYTE nFrb2, BYTE nInts)
{
    UINT16 r1=0,g1=0,b1=0,r2=0,g2=0,b2=0;
    BYTE   nInt2=100-nInts;
    switch(nFrb1 & 0x07) {
        case 0:  r1=0xFF; g1=0xFF; b1=0xFF; break;
        case 1:  r1=0xFF; g1=0xFF;            break;
        case 2:           g1=0xFF; b1=0xFF; break;
        case 3:           g1=0xFF;            break;
        case 4:  r1=0xFF;          b1=0xFF; break;
        case 5:  r1=0xFF;                       break;
        case 6:                    b1=0xFF; break;
        case 7:                                   break;
    }
    switch(nFrb2 & 0x07) {
        case 0:  r2=0xFF; g2=0xFF; b2=0xFF; break;
        case 1:  r2=0xFF; g2=0xFF;            break;
        case 2:           g2=0xFF; b2=0xFF; break;
        case 3:           g2=0xFF;            break;
        case 4:  r2=0xFF;          b2=0xFF; break;
        case 5:  r2=0xFF;                       break;
        case 6:                    b2=0xFF; break;
        case 7:                                   break;
    }
#endif
    r1=(UINT16)((UINT32)r1*nInts/100+(UINT32)r2*nInt2/100);
    g1=(UINT16)((UINT32)g1*nInts/100+(UINT32)g2*nInt2/100);
    b1=(UINT16)((UINT32)b1*nInts/100+(UINT32)b2*nInt2/100);
    Color aColor( (sal_uInt8)r1, (sal_uInt8)g1, (sal_uInt8)b1 );
    return aColor;
}

void SetLine(ObjLineType& rLine, OutputDevice& rOut)
{
/* !!!
    PenStyle aStyle=PEN_SOLID;
    switch(rLine.LMuster & 0x07) {
        case 0: aStyle=PEN_NULL;    break;
        case 1: aStyle=PEN_SOLID;   break;
        case 2: aStyle=PEN_DOT;     break;    // ù ù ù ù ù ù ù ù ù ù ù ù ù ù
        case 3: aStyle=PEN_DASH;    break;    // ÄÄ ÄÄ ÄÄ ÄÄ ÄÄ ÄÄ ÄÄ ÄÄ ÄÄ
        case 4: aStyle=PEN_DASH;    break;    // ÄÄÄ ÄÄÄ ÄÄÄ ÄÄÄ ÄÄÄ ÄÄÄ ÄÄÄ
        case 5: aStyle=PEN_DASHDOT; break;    // ÄÄ ù ÄÄ ù ÄÄ ù ÄÄ ù ÄÄ ù ÄÄ
        case 6: aStyle=PEN_DASHDOT; break;    // ÄÄ Ä ÄÄ Ä ÄÄ Ä ÄÄ Ä ÄÄ Ä ÄÄ
        case 7: aStyle=PEN_DASHDOT; break;    // ÄÄÄ Ä Ä ÄÄÄ Ä Ä ÄÄÄ Ä Ä ÄÄÄ
    }
    Pen aPen(Sgv2SvFarbe(rLine.LFarbe,rLine.LBFarbe,rLine.LIntens),rLine.LDicke,aStyle);
    SetPen(aPen,rOut);
*/
    if( 0 == ( rLine.LMuster & 0x07 ) )
        rOut.SetLineColor();
    else
        rOut.SetLineColor( Sgv2SvFarbe(rLine.LFarbe,rLine.LBFarbe,rLine.LIntens) );
}

void SetArea(ObjAreaType& rArea, OutputDevice& rOut)
{
/*
    BrushStyle aStyle=BRUSH_SOLID;
    switch(rArea.FMuster & 0x00FF) {
        case  0: aStyle=BRUSH_NULL;      break;
        case  1: aStyle=BRUSH_SOLID;     break;
        case  2: case  4: case  6: case  8:
        case 10: case 12: case 14: case 16:
        case 43: case 45:                   aStyle=BRUSH_VERT;      break;
        case  3: case  5: case  7: case  9:
        case 11: case 13: case 15: case 17:
        case 42: case 44:                   aStyle=BRUSH_HORZ;      break;
        case 18: case 20: case 22: case 24:
        case 26: case 28: case 30: case 32:
        case 46: case 48:                   aStyle=BRUSH_UPDIAG;    break;
        case 19: case 21: case 23: case 25:
        case 27: case 29: case 31: case 33:
        case 47: case 49:                   aStyle=BRUSH_DOWNDIAG;  break;
        case 34: case 35: case 36: case 37: aStyle=BRUSH_CROSS;     break;
        case 38: case 39: case 40: case 41: aStyle=BRUSH_DIAGCROSS; break;
        default: aStyle=BRUSH_DIAGCROSS; break;
    }
    Brush aBrush(Sgv2SvFarbe(rArea.FFarbe,rArea.FBFarbe,rArea.FIntens),aStyle);
    aBrush.SetTransparent((rArea.FMuster & 0x80) !=0L);
    SetBrush(aBrush,rOut);
*/
    if( 0 == ( rArea.FMuster & 0x00FF ) )
        rOut.SetFillColor();
    else
        rOut.SetFillColor( Sgv2SvFarbe( rArea.FFarbe,rArea.FBFarbe,rArea.FIntens ) );
}

/*************************************************************************
|*
|*    ObjkType::DrawObjekt()
|*
|*    Beschreibung
|*    Ersterstellung    JOE 23.06.93
|*    Letzte Aenderung  JOE 23.06.93
|*
*************************************************************************/
void ObjkType::Draw(OutputDevice&)
{
//    ShowSDObjk(*this);
}

void Obj0Type::Draw(OutputDevice&) {}

/*************************************************************************
|*
|*    StrkType::DrawObjekt()
|*
|*    Beschreibung
|*    Ersterstellung    JOE 23.06.93
|*    Letzte Aenderung  JOE 23.06.93
|*
*************************************************************************/
void StrkType::Draw(OutputDevice& rOut)
{
    SetLine(L,rOut);
    rOut.DrawLine(Point(Pos1.x,Pos1.y),Point(Pos2.x,Pos2.y)); // !!!
}

/*************************************************************************
|*
|*    RectType::DrawObjekt()
|*
|*    Beschreibung
|*    Ersterstellung    JOE 23.06.93
|*    Letzte Aenderung  JOE 23.06.93
|*
*************************************************************************/
void SgfAreaColorIntens(UINT16 Muster, BYTE Col1, BYTE Col2, BYTE Int, OutputDevice& rOut)
{
    ObjAreaType F;
    F.FMuster=Muster;
    F.FFarbe=Col2;
    F.FBFarbe=Col1;
    F.FIntens=Int;
    SetArea(F,rOut);
}

void DrawSlideRect(INT16 x1, INT16 y1, INT16 x2, INT16 y2, ObjAreaType& F, OutputDevice& rOut)
{
    INT16 i,i0,b,b0;
    INT16 Int1,Int2;
    INT16 Col1,Col2;
    //     ClipMerk: HgdClipRec;
    INT16 cx,cy;
    INT16 MaxR;
    INT32 dx,dy;

    rOut.SetLineColor();
    if (x1>x2) { i=x1; x1=x2; x2=i; }
    if (y1>y2) { i=y1; y1=y2; y2=i; }
    Col1=F.FBFarbe & 0x87; Col2=F.FFarbe & 0x87;
    Int1=100-F.FIntens; Int2=F.FIntens;
    if (Int1==Int2) {
        SgfAreaColorIntens(F.FMuster,(BYTE)Col1,(BYTE)Col2,(BYTE)Int2,rOut);
        rOut.DrawRect(Rectangle(x1,y1,x2,y2));
    } else {
        b0=Int1;
        switch (F.FBFarbe & 0x38) {
            case 0x08: { // vertikal
                i0=y1;
                i=y1;
                while (i<=y2) {
                    b=Int1+INT16((INT32)(Int2-Int1)*(INT32)(i-y1) /(INT32)(y2-y1+1));
                    if (b!=b0) {
                        SgfAreaColorIntens(F.FMuster,(BYTE)Col1,(BYTE)Col2,(BYTE)b0,rOut);
                        rOut.DrawRect(Rectangle(x1,i0,x2,i-1));
                        i0=i; b0=b;
                    }
                    i++;
                }
                SgfAreaColorIntens(F.FMuster,(BYTE)Col1,(BYTE)Col2,(BYTE)Int2,rOut);
                rOut.DrawRect(Rectangle(x1,i0,x2,y2));
            } break;
            case 0x28: { // horizontal
                i0=x1;
                i=x1;
                while (i<=x2) {
                    b=Int1+INT16((INT32)(Int2-Int1)*(INT32)(i-x1) /(INT32)(x2-x1+1));
                    if (b!=b0) {
                        SgfAreaColorIntens(F.FMuster,(BYTE)Col1,(BYTE)Col2,(BYTE)b0,rOut);
                        rOut.DrawRect(Rectangle(i0,y1,i-1,y2));
                        i0=i; b0=b;
                    }
                    i++;
                }
                SgfAreaColorIntens(F.FMuster,(BYTE)Col1,(BYTE)Col2,(BYTE)Int2,rOut);
                rOut.DrawRect(Rectangle(i0,y1,x2,y2));
            } break;

            case 0x18: case 0x38: { // Kreis
                Region ClipMerk=rOut.GetClipRegion();
                double a;

                rOut.SetClipRegion(Region(Rectangle(x1,y1,x2,y2)));
                cx=(x1+x2) /2;
                cy=(y1+y2) /2;
                dx=x2-x1+1;
                dy=y2-y1+1;
                a=sqrt((double)(dx*dx+dy*dy));
                MaxR=INT16(a) /2 +1;
                b0=Int2;
                i0=MaxR; if (MaxR<1) MaxR=1;
                i=MaxR;
                while (i>=0) {
                    b=Int1+INT16((INT32(Int2-Int1)*INT32(i)) /INT32(MaxR));
                    if (b!=b0) {
                        SgfAreaColorIntens(F.FMuster,(BYTE)Col1,(BYTE)Col2,(BYTE)b0,rOut);
                        //if (i0>200 || (Col1 & $80)!=0 || (Col2 & $80)!=0) {
                        //  then begin { Fallunterscheidung für etwas bessere Performance }
                        //    s2:=i0-i+2;
                        //    SetPenSize(s2);
                        //    s2:=s2 div 2;
                        //    Circle(cx,cy,i0-s2,i0-s2);{}
                        //  else
                        rOut.DrawEllipse(Rectangle(cx-i0,cy-i0,cx+i0,cy+i0));
                        i0=i; b0=b;
                    }
                    i--;
                }
                SgfAreaColorIntens(F.FMuster,(BYTE)Col1,(BYTE)Col2,(BYTE)Int1,rOut);
                rOut.DrawEllipse(Rectangle(cx-i0,cy-i0,cx+i0,cy+i0));
                rOut.SetClipRegion(ClipMerk);
            } break; // Kreis
        }
    }
}


void RectType::Draw(OutputDevice& rOut)
{
    if (L.LMuster!=0) L.LMuster=1; // keine Linienmuster hier, nur an oder aus
    SetArea(F,rOut);
    if (DrehWink==0) {
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
        USHORT i;
        double sn,cs;
        sn=sin(double(DrehWink)*3.14159265359/18000);
        cs=cos(double(DrehWink)*3.14159265359/18000);
        aPts[0]=Point(Pos1.x,Pos1.y);
        aPts[1]=Point(Pos2.x,Pos1.y);
        aPts[2]=Point(Pos2.x,Pos2.y);
        aPts[3]=Point(Pos1.x,Pos2.y);
        for (i=0;i<4;i++) {
            RotatePoint(aPts[i],Pos1.x,Pos1.y,sn,cs);
        }
        SetLine(L,rOut);
        Polygon aPoly(4,aPts);
        rOut.DrawPolygon(aPoly);
    }
}

/*************************************************************************
|*
|*    PolyType::Draw()
|*
|*    Beschreibung
|*    Ersterstellung    JOE 23.06.93
|*    Letzte Aenderung  JOE 23.06.93
|*
*************************************************************************/
void PolyType::Draw(OutputDevice& rOut)
{
    if ((Flags & PolyClosBit) !=0) SetArea(F,rOut);
    SetLine(L,rOut);
    Polygon aPoly(nPoints);
    USHORT i;
    for(i=0;i<nPoints;i++) aPoly.SetPoint(Point(EckP[i].x,EckP[i].y),i);
    if ((Flags & PolyClosBit) !=0) {
        rOut.DrawPolygon(aPoly);
    } else {
        rOut.DrawPolyLine(aPoly);
    }
}

/*************************************************************************
|*
|*    SplnType::Draw()
|*
|*    Beschreibung
|*    Ersterstellung    JOE 23.06.93
|*    Letzte Aenderung  JOE 23.06.93
|*
*************************************************************************/
void SplnType::Draw(OutputDevice& rOut)
{
    if ((Flags & PolyClosBit) !=0) SetArea(F,rOut);
    SetLine(L,rOut);
    Polygon aPoly(0);
    Polygon aSpln(nPoints);
    USHORT i;
    for(i=0;i<nPoints;i++) aSpln.SetPoint(Point(EckP[i].x,EckP[i].y),i);
    if ((Flags & PolyClosBit) !=0) {
        Spline2Poly(aSpln,TRUE,aPoly);
        if (aPoly.GetSize()>0) rOut.DrawPolygon(aPoly);
    } else {
        Spline2Poly(aSpln,FALSE,aPoly);
        if (aPoly.GetSize()>0) rOut.DrawPolyLine(aPoly);
    }
}

/*************************************************************************
|*
|*    CircType::Draw()
|*
|*    Beschreibung
|*    Ersterstellung    JOE 23.06.93
|*    Letzte Aenderung  JOE 23.06.93
|*
*************************************************************************/
void DrawSlideCirc(INT16 cx, INT16 cy, INT16 rx, INT16 ry, ObjAreaType& F, OutputDevice& rOut)
{
    INT16 x1=cx-rx;
    INT16 y1=cy-ry;
    INT16 x2=cx+rx;
    INT16 y2=cy+ry;

    INT16 i,i0,b,b0;
    INT16 Int1,Int2;
    INT16 Col1,Col2;

    rOut.SetLineColor();
    Col1=F.FBFarbe & 0x87; Col2=F.FFarbe & 0x87;
    Int1=100-F.FIntens; Int2=F.FIntens;
    if (Int1==Int2) {
        SgfAreaColorIntens(F.FMuster,(BYTE)Col1,(BYTE)Col2,(BYTE)Int2,rOut);
        rOut.DrawEllipse(Rectangle(x1,y1,x2,y2));
    } else {
        b0=Int1;
        switch (F.FBFarbe & 0x38) {
            case 0x08: { // vertikal
                Region ClipMerk=rOut.GetClipRegion();
                i0=y1;
                i=y1;
                while (i<=y2) {
                    b=Int1+INT16((INT32)(Int2-Int1)*(INT32)(i-y1) /(INT32)(y2-y1+1));
                    if (b!=b0) {
                        SgfAreaColorIntens(F.FMuster,(BYTE)Col1,(BYTE)Col2,(BYTE)b0,rOut);
                        rOut.SetClipRegion(Rectangle(x1,i0,x2,i-1));
                        rOut.DrawEllipse(Rectangle(x1,y1,x2,y2));
                        i0=i; b0=b;
                    }
                    i++;
                }
                SgfAreaColorIntens(F.FMuster,(BYTE)Col1,(BYTE)Col2,(BYTE)Int2,rOut);
                rOut.SetClipRegion(Rectangle(x1,i0,x2,y2));
                rOut.DrawEllipse(Rectangle(x1,y1,x2,y2));
                rOut.SetClipRegion(ClipMerk);
            } break;
            case 0x28: { // horizontal
                Region ClipMerk=rOut.GetClipRegion();
                i0=x1;
                i=x1;
                while (i<=x2) {
                    b=Int1+INT16((INT32)(Int2-Int1)*(INT32)(i-x1) /(INT32)(x2-x1+1));
                    if (b!=b0) {
                        SgfAreaColorIntens(F.FMuster,(BYTE)Col1,(BYTE)Col2,(BYTE)b0,rOut);
                        rOut.SetClipRegion(Rectangle(i0,y1,i-1,y2));
                        rOut.DrawEllipse(Rectangle(x1,y1,x2,y2));
                        i0=i; b0=b;
                    }
                    i++;
                }
                SgfAreaColorIntens(F.FMuster,(BYTE)Col1,(BYTE)Col2,(BYTE)Int2,rOut);
                rOut.SetClipRegion(Rectangle(i0,y1,x2,y2));
                rOut.DrawEllipse(Rectangle(x1,y1,x2,y2));
                rOut.SetClipRegion(ClipMerk);
            } break;

            case 0x18: case 0x38: { // Kreis
                INT16 MaxR;

                if (rx<1) rx=1;
                if (ry<1) ry=1;
                MaxR=rx;
                b0=Int2;
                i0=MaxR; if (MaxR<1) MaxR=1;
                i=MaxR;
                while (i>=0) {
                    b=Int1+INT16((INT32(Int2-Int1)*INT32(i)) /INT32(MaxR));
                    if (b!=b0) {
                        INT32 temp=INT32(i0)*INT32(ry)/INT32(rx);
                        INT16 j0=INT16(temp);
                        SgfAreaColorIntens(F.FMuster,(BYTE)Col1,(BYTE)Col2,(BYTE)b0,rOut);
                        rOut.DrawEllipse(Rectangle(cx-i0,cy-j0,cx+i0,cy+j0));
                        i0=i; b0=b;
                    }
                    i--;
                }
                SgfAreaColorIntens(F.FMuster,(BYTE)Col1,(BYTE)Col2,(BYTE)Int1,rOut);
                rOut.DrawEllipse(Rectangle(cx-i0,cy-i0,cx+i0,cy+i0));
            } break; // Kreis
        }
    }
}


void CircType::Draw(OutputDevice& rOut)
{
    Rectangle aRect(Center.x-Radius.x,Center.y-Radius.y,Center.x+Radius.x,Center.y+Radius.y);

    if (L.LMuster!=0) L.LMuster=1; // keine Linienmuster hier, nur an oder aus
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
        sn=sin(double(StartWink)*3.14159265359/18000);
        cs=cos(double(StartWink)*3.14159265359/18000);
        RotatePoint(a,Center.x,Center.y,sn,cs);
        sn=sin(double(StartWink+RelWink)*3.14159265359/18000);
        cs=cos(double(StartWink+RelWink)*3.14159265359/18000);
        RotatePoint(b,Center.x,Center.y,sn,cs);
        if (Radius.x!=Radius.y) {
          if (Radius.x<1) Radius.x=1;
          if (Radius.y<1) Radius.y=1;
          a.y-=Center.y;
          b.y-=Center.y;
          a.y=iMulDiv(a.y,Radius.y,Radius.x);
          b.y=iMulDiv(b.y,Radius.y,Radius.x);
          a.y+=Center.y;
          b.y+=Center.y;
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

/*************************************************************************
|*
|*    BmapType::Draw()
|*
|*    Beschreibung
|*    Ersterstellung    JOE 23.06.93
|*    Letzte Aenderung  JOE 23.06.93
|*
*************************************************************************/

void BmapType::Draw(OutputDevice& rOut)
{
    //ifstream aInp;
    unsigned char   nSgfTyp;
    USHORT          nVersion;
    String          aStr( Filename[ 1 ], (xub_StrLen)Filename[ 0 ], RTL_TEXTENCODING_UTF8 );
    INetURLObject   aFNam( aStr );

    SvStream* pInp = ::utl::UcbStreamHelper::CreateStream( aFNam.GetMainURL(), STREAM_READ );
    if ( pInp )
    {
        nSgfTyp=CheckSgfTyp( *pInp,nVersion);
        switch(nSgfTyp) {
            case SGF_BITIMAGE: {
                GraphicFilter aFlt;
                Graphic aGrf;
                USHORT nRet;
                nRet=aFlt.ImportGraphic(aGrf,aFNam);
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
                SgfVectScal=TRUE;
                SgfVectFilter(*pInp,aMtf);
                SgfVectXofs=0;
                SgfVectYofs=0;
                SgfVectXmul=0;
                SgfVectYmul=0;
                SgfVectXdiv=0;
                SgfVectYdiv=0;
                SgfVectScal=FALSE;
                aMtf.Play(&rOut);
            } break;
        }
        delete pInp;
    }
}


/*************************************************************************
|*
|*    GrupType::...
|*
|*    Beschreibung
|*    Ersterstellung    JOE 23.06.93
|*    Letzte Aenderung  JOE 23.06.93
|*
*************************************************************************/
UINT32 GrupType::GetSubPtr()
{
    return UINT32(SbLo)+0x00010000*UINT32(SbHi);
}

/*************************************************************************
|*
|*    DrawObjkList()
|*
|*    Beschreibung
|*    Ersterstellung    JOE 23.06.93
|*    Letzte Aenderung  JOE 23.06.93
|*
*************************************************************************/
void DrawObjkList( SvStream& rInp, OutputDevice& rOut )
{
    ObjkType aObjk;
    USHORT nGrpCnt=0;
    BOOL bEnde=FALSE;
    do {
        rInp>>aObjk;
        if (!rInp.GetError()) {
            switch(aObjk.Art) {
                case ObjStrk: { StrkType aStrk; rInp>>aStrk; if (!rInp.GetError()) aStrk.Draw(rOut); } break;
                case ObjRect: { RectType aRect; rInp>>aRect; if (!rInp.GetError()) aRect.Draw(rOut); } break;
                case ObjCirc: { CircType aCirc; rInp>>aCirc; if (!rInp.GetError()) aCirc.Draw(rOut); } break;
                case ObjText: {
                    TextType aText;
                    rInp>>aText;
                    if (!rInp.GetError()) {
                        aText.Buffer=new UCHAR[aText.BufSize+1]; // Ein mehr für LookAhead bei CK-Trennung
                        rInp.Read((char* )aText.Buffer,aText.BufSize);
                        if (!rInp.GetError()) aText.Draw(rOut);
                        delete aText.Buffer;
                    }
                } break;
                case ObjBmap: {
                    BmapType aBmap;
                    rInp>>aBmap;
                    if (!rInp.GetError()) {
                        aBmap.Draw(rOut);
                    }
                } break;
                case ObjPoly: {
                    PolyType aPoly;
                    rInp>>aPoly;
                    if (!rInp.GetError()) {
                        aPoly.EckP=new PointType[aPoly.nPoints];
                        rInp.Read((char*)aPoly.EckP,4*aPoly.nPoints);
#if defined __BIGENDIAN
                        for(short i=0;i<aPoly.nPoints;i++) SWAPPOINT(aPoly.EckP[i]);
#endif
                        if (!rInp.GetError()) aPoly.Draw(rOut);
                        delete aPoly.EckP;
                    }
                } break;
                case ObjSpln: {
                    SplnType aSpln;
                    rInp>>aSpln;
                    if (!rInp.GetError()) {
                        aSpln.EckP=new PointType[aSpln.nPoints];
                        rInp.Read((char*)aSpln.EckP,4*aSpln.nPoints);
#if defined __BIGENDIAN
                        for(short i=0;i<aSpln.nPoints;i++) SWAPPOINT(aSpln.EckP[i]);
#endif
                        if (!rInp.GetError()) aSpln.Draw(rOut);
                        delete aSpln.EckP;
                    }
                } break;
                case ObjGrup: {
                    GrupType aGrup;
                    rInp>>aGrup;
                    if (!rInp.GetError()) {
                        rInp.Seek(rInp.Tell()+aGrup.Last); // Obj-Anhängsel
                        if(aGrup.GetSubPtr()!=0L) nGrpCnt++;// DrawObjkList(rInp,rOut );
                    }
                } break;
                default: {
                    aObjk.Draw(rOut);          // Objektbezeichnung auf 2. Screen
                    ObjkOverSeek(rInp,aObjk);  // zum nächsten Objekt
                }
            }
        } // if rInp
        if (!rInp.GetError()) {
            if (aObjk.Next==0L) {
                if (nGrpCnt==0) bEnde=TRUE;
                else nGrpCnt--;
            }
        } else {
            bEnde=TRUE;  // Lesefehler
        }
    } while (!bEnde);
}

/*************************************************************************
|*
|*    SkipObjkList()
|*
|*    Beschreibung
|*    Ersterstellung    JOE 23.06.93
|*    Letzte Aenderung  JOE 23.06.93
|*
*************************************************************************/
void SkipObjkList(SvStream& rInp)
{
    ObjkType aObjk;
    do
    {
        rInp>>aObjk;
        if(aObjk.Art==ObjGrup) {
            GrupType aGrup;
            rInp>>aGrup;
            rInp.Seek(rInp.Tell()+aGrup.Last); // Obj-Anhängsel
            if(aGrup.GetSubPtr()!=0L) SkipObjkList(rInp);
        } else {
            ObjkOverSeek(rInp,aObjk);  // zum nächsten Objekt
        }
    } while (aObjk.Next!=0L && !rInp.GetError());
}

/*************************************************************************
|*
|*    SgfFilterSDrw()
|*
|*    Beschreibung
|*    Ersterstellung    JOE 23.06.93
|*    Letzte Aenderung  JOE 23.06.93
|*
*************************************************************************/
BOOL SgfFilterSDrw( SvStream& rInp, SgfHeader&, SgfEntry&, GDIMetaFile& rMtf )
{
    BOOL          bRet = FALSE;
    ObjkType      aObjk;
    PageType      aPage;
    VirtualDevice aOutDev;
    OutputDevice* pOutDev;
    ULONG         nStdPos;
    ULONG         nZchPos;
    USHORT        Num;

    pOutDev=&aOutDev;
    DtHdOverSeek(rInp); // DataHeader weglesen

    nStdPos=rInp.Tell();
    do {                // Standardseiten weglesen
        rInp>>aPage;
        if (aPage.nList!=0) SkipObjkList(rInp);
    } while (aPage.Next!=0L && !rInp.GetError());

//    ShowMsg("Zeichnungseite(n)\n");
    nZchPos=rInp.Tell();
    rInp>>aPage;

    rMtf.Record(pOutDev);
    Num=aPage.StdPg;
    if (Num!=0) {
      rInp.Seek(nStdPos);
      while(Num>1 && aPage.Next!=0L && !rInp.GetError()) { // Standardseite suchen
        rInp>>aPage;
        if (aPage.nList!=0) SkipObjkList(rInp);
        Num--;
      }
      rInp>>aPage;
      if(Num==1 && aPage.nList!=0L) DrawObjkList( rInp,*pOutDev );
      rInp.Seek(nZchPos);
      nZchPos=rInp.Tell();
      rInp>>aPage;
    }
    if (aPage.nList!=0L) DrawObjkList(rInp,*pOutDev );

    rMtf.Stop();
    rMtf.WindStart();
    MapMode aMap(MAP_10TH_MM,Point(),Fraction(1,4),Fraction(1,4));
    rMtf.SetPrefMapMode(aMap);
    rMtf.SetPrefSize(Size((INT16)aPage.Paper.Size.x,(INT16)aPage.Paper.Size.y));
    bRet=TRUE;
    return bRet;
}



/*************************************************************************
|*
|*    SgfSDrwFilter()
|*
|*    Beschreibung
|*    Ersterstellung    JOE 23.06.93
|*    Letzte Aenderung  JOE 23.06.93
|*
*************************************************************************/
BOOL SgfSDrwFilter(SvStream& rInp, GDIMetaFile& rMtf, INetURLObject aIniPath )
{
#ifdef DEBUG  // Recordgrößen checken. Neuer Compiler hat vielleichte anderes Allignment!
    if (sizeof(ObjTextType)!=ObjTextTypeSize)  return FALSE;
#endif

    ULONG     nFileStart;            // Offset des SgfHeaders. Im allgemeinen 0.
    SgfHeader aHead;
    SgfEntry  aEntr;
    ULONG     nNext;
    BOOL      bRdFlag=FALSE;         // Grafikentry gelesen ?
    BOOL      bRet=FALSE;            // Returncode

    aIniPath.Append( String::CreateFromAscii( "sgf.ini", 7 ) );
//  aIniPath.ToAbs();

    pSgfFonts = new SgfFontLst;

    pSgfFonts->AssignFN( aIniPath.GetMainURL() );
    nFileStart=rInp.Tell();
    rInp>>aHead;
    if (aHead.ChkMagic() && aHead.Typ==SgfStarDraw && aHead.Version==SGV_VERSION) {
        nNext=aHead.GetOffset();
        while (nNext && !bRdFlag && !rInp.GetError()) {
            rInp.Seek(nFileStart+nNext);
            rInp>>aEntr;
            nNext=aEntr.GetOffset();
            if (aEntr.Typ==aHead.Typ) {
                bRet=SgfFilterSDrw( rInp,aHead,aEntr,rMtf );
            }
        } // while(nNext)
        if (bRdFlag) {
            if (!rInp.GetError()) bRet=TRUE;  // Scheinbar Ok
        }
    }
    delete pSgfFonts;
    return(bRet);
}

/*
Bitmap Dither(BYTE Intens)
{
    Bitmap aBmp;
    BmpInfoHeader Info;


const dmatrix: array[0..7,0..7] of byte =
        ((  0, 48, 12, 60,  3, 51, 15, 63 ),
         ( 32, 16, 44, 28, 35, 19, 47, 31 ),
         (  8, 56,  4, 52, 11, 59,  7, 55 ),
         ( 40, 24, 36, 20, 43, 27, 39, 23 ),
         (  2, 50, 14, 62,  1, 49, 13, 61 ),
         ( 34, 18, 46, 30, 33, 17, 45, 29 ),
         ( 10, 58,  6, 54,  9, 57,  5, 53 ),
         ( 42, 26, 38, 22, 41, 25, 37, 21 ));


    cmatrix: array[0..7,0..7] of byte;
    dmatrixn,dmatrixi: array[0..7] of byte;


procedure SetColorIntens(col0,col1,bal: integer);
var cmatrix0: array[0..63] of byte absolute cmatrix;
    dmatrix0: array[0..63] of byte absolute dmatrix;
    n,i: integer;
    b,bit: byte;
begin
if col0=col1 then bal:=0;
if bal<=32 then
  begin
  plotcolor0:=col0 and $1F; plotcolor1:=col1 and $1F;
  plotbal:=bal;
  end
else
  begin
  plotcolor0:=col1 and $1F; plotcolor1:=col0 and $1F;
  plotbal:=64-bal;
  end;
for n:=0 to 63 do
 if plotbal<=dmatrix0[n] then cmatrix0[n]:=col0 else cmatrix0[n]:=col1;
end;
*/

#if defined( WIN ) && defined( MSC )
#pragma code_seg( "svtools", "AUTO_CODE" )
#endif

