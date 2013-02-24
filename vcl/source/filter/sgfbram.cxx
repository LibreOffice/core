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


#include <string.h>
#include <osl/endian.h>
#include <tools/stream.hxx>
#include <vcl/gdimtf.hxx>
#include <tools/color.hxx>
#include <vcl/virdev.hxx>
#include "sgffilt.hxx"
#include "sgfbram.hxx"

SvStream& operator>>(SvStream& rIStream, SgfHeader& rHead)
{
    rIStream.Read((char*)&rHead.Magic,SgfHeaderSize);
#if defined OSL_BIGENDIAN
    rHead.Magic  =OSL_SWAPWORD(rHead.Magic  );
    rHead.Version=OSL_SWAPWORD(rHead.Version);
    rHead.Typ    =OSL_SWAPWORD(rHead.Typ    );
    rHead.Xsize  =OSL_SWAPWORD(rHead.Xsize  );
    rHead.Ysize  =OSL_SWAPWORD(rHead.Ysize  );
    rHead.Xoffs  =OSL_SWAPWORD(rHead.Xoffs  );
    rHead.Yoffs  =OSL_SWAPWORD(rHead.Yoffs  );
    rHead.Planes =OSL_SWAPWORD(rHead.Planes );
    rHead.SwGrCol=OSL_SWAPWORD(rHead.SwGrCol);
    rHead.OfsLo  =OSL_SWAPWORD(rHead.OfsLo  );
    rHead.OfsHi  =OSL_SWAPWORD(rHead.OfsHi  );
#endif
    return rIStream;
}

sal_Bool SgfHeader::ChkMagic()
{ return Magic=='J'*256+'J'; }

sal_uInt32 SgfHeader::GetOffset()
{ return sal_uInt32(OfsLo)+0x00010000*sal_uInt32(OfsHi); }

SvStream& operator>>(SvStream& rIStream, SgfEntry& rEntr)
{
    rIStream.Read((char*)&rEntr.Typ,SgfEntrySize);
#if defined OSL_BIGENDIAN
    rEntr.Typ  =OSL_SWAPWORD(rEntr.Typ  );
    rEntr.iFrei=OSL_SWAPWORD(rEntr.iFrei);
    rEntr.lFreiLo=OSL_SWAPWORD (rEntr.lFreiLo);
    rEntr.lFreiHi=OSL_SWAPWORD (rEntr.lFreiHi);
    rEntr.OfsLo=OSL_SWAPWORD(rEntr.OfsLo);
    rEntr.OfsHi=OSL_SWAPWORD(rEntr.OfsHi);
#endif
    return rIStream;
}

sal_uInt32 SgfEntry::GetOffset()
{ return sal_uInt32(OfsLo)+0x00010000*sal_uInt32(OfsHi); }

SvStream& operator>>(SvStream& rIStream, SgfVector& rVect)
{
    rIStream.Read((char*)&rVect,sizeof(rVect));
#if defined OSL_BIGENDIAN
    rVect.Flag =OSL_SWAPWORD(rVect.Flag );
    rVect.x    =OSL_SWAPWORD(rVect.x    );
    rVect.y    =OSL_SWAPWORD(rVect.y    );
    rVect.OfsLo=OSL_SWAPDWORD (rVect.OfsLo);
    rVect.OfsHi=OSL_SWAPDWORD (rVect.OfsHi);
#endif
    return rIStream;
}

SvStream& operator<<(SvStream& rOStream, BmpFileHeader& rHead)
{
#if defined OSL_BIGENDIAN
    rHead.Typ     =OSL_SWAPWORD(rHead.Typ     );
    rHead.SizeLo  =OSL_SWAPWORD(rHead.SizeLo  );
    rHead.SizeHi  =OSL_SWAPWORD(rHead.SizeHi  );
    rHead.Reserve1=OSL_SWAPWORD(rHead.Reserve1);
    rHead.Reserve2=OSL_SWAPWORD(rHead.Reserve2);
    rHead.OfsLo   =OSL_SWAPWORD(rHead.OfsLo   );
    rHead.OfsHi   =OSL_SWAPWORD(rHead.OfsHi   );
#endif
    rOStream.Write((char*)&rHead,sizeof(rHead));
#if defined OSL_BIGENDIAN
    rHead.Typ     =OSL_SWAPWORD(rHead.Typ     );
    rHead.SizeLo  =OSL_SWAPWORD(rHead.SizeLo  );
    rHead.SizeHi  =OSL_SWAPWORD(rHead.SizeHi  );
    rHead.Reserve1=OSL_SWAPWORD(rHead.Reserve1);
    rHead.Reserve2=OSL_SWAPWORD(rHead.Reserve2);
    rHead.OfsLo   =OSL_SWAPWORD(rHead.OfsLo   );
    rHead.OfsHi   =OSL_SWAPWORD(rHead.OfsHi   );
#endif
    return rOStream;
}

void BmpFileHeader::SetSize(sal_uInt32 Size)
{
    SizeLo=sal_uInt16(Size & 0x0000FFFF);
    SizeHi=sal_uInt16((Size & 0xFFFF0000)>>16);
}

void BmpFileHeader::SetOfs(sal_uInt32 Ofs)
{
    OfsLo=sal_uInt16(Ofs & 0x0000FFFF);
    OfsHi=sal_uInt16((Ofs & 0xFFFF0000)>>16);
}

sal_uInt32 BmpFileHeader::GetOfs()
{
    return sal_uInt32(OfsLo)+0x00010000*sal_uInt32(OfsHi);
}

SvStream& operator<<(SvStream& rOStream, BmpInfoHeader& rInfo)
{
#if defined OSL_BIGENDIAN
    rInfo.Size    =OSL_SWAPDWORD (rInfo.Size    );
    rInfo.Width   =OSL_SWAPDWORD (rInfo.Width   );
    rInfo.Hight   =OSL_SWAPDWORD (rInfo.Hight   );
    rInfo.Planes  =OSL_SWAPWORD(rInfo.Planes  );
    rInfo.PixBits =OSL_SWAPWORD(rInfo.PixBits );
    rInfo.Compress=OSL_SWAPDWORD (rInfo.Compress);
    rInfo.ImgSize =OSL_SWAPDWORD (rInfo.ImgSize );
    rInfo.xDpmm   =OSL_SWAPDWORD (rInfo.xDpmm   );
    rInfo.yDpmm   =OSL_SWAPDWORD (rInfo.yDpmm   );
    rInfo.ColUsed =OSL_SWAPDWORD (rInfo.ColUsed );
    rInfo.ColMust =OSL_SWAPDWORD (rInfo.ColMust );
#endif
    rOStream.Write((char*)&rInfo,sizeof(rInfo));
#if defined OSL_BIGENDIAN
    rInfo.Size    =OSL_SWAPDWORD (rInfo.Size    );
    rInfo.Width   =OSL_SWAPDWORD (rInfo.Width   );
    rInfo.Hight   =OSL_SWAPDWORD (rInfo.Hight   );
    rInfo.Planes  =OSL_SWAPWORD(rInfo.Planes  );
    rInfo.PixBits =OSL_SWAPWORD(rInfo.PixBits );
    rInfo.Compress=OSL_SWAPDWORD (rInfo.Compress);
    rInfo.ImgSize =OSL_SWAPDWORD (rInfo.ImgSize );
    rInfo.xDpmm   =OSL_SWAPDWORD (rInfo.xDpmm   );
    rInfo.yDpmm   =OSL_SWAPDWORD (rInfo.yDpmm   );
    rInfo.ColUsed =OSL_SWAPDWORD (rInfo.ColUsed );
    rInfo.ColMust =OSL_SWAPDWORD (rInfo.ColMust );
#endif
    return rOStream;
}

SvStream& operator<<(SvStream& rOStream, const RGBQuad& rQuad)
{
    rOStream.Write((char*)&rQuad,sizeof(rQuad));
    return rOStream;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// PcxExpand ///////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class PcxExpand
{
private:
    sal_uInt16 Count;
    sal_uInt8   Data;
public:
                  PcxExpand() { Count=0; }
    sal_uInt8 GetByte(SvStream& rInp);
};

sal_uInt8 PcxExpand::GetByte(SvStream& rInp)
{
    if (Count>0) {
        Count--;
    } else {
        rInp.Read((char*)&Data,1);
        if ((Data & 0xC0) == 0xC0) {
            Count=(Data & 0x3F) -1;
            rInp.Read((char*)&Data,1);
        }
    }
    return Data;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// SgfBMapFilter ///////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

sal_Bool SgfFilterBMap(SvStream& rInp, SvStream& rOut, SgfHeader& rHead, SgfEntry&)
{
    BmpFileHeader  aBmpHead;
    BmpInfoHeader  aBmpInfo;
    sal_uInt16 nWdtInp=(rHead.Xsize+7)/8;  // Breite der Input-Bitmap in Bytes
    sal_uInt16         nWdtOut;            // Breite der Output-Bitmap in Bytes
    sal_uInt16         nColors;            // Anzahl der Farben     (1,16,256)
    sal_uInt16         nColBits;           // Anzahl der Bits/Pixel (2, 4,  8)
    sal_uInt16         i,j,k;              // Spaltenzaehler, Zeilenzaehler, Planezaehler
    sal_uInt16         a,b;                // Hilfsvariable
    sal_uInt8           pl1 = 0;            // Masken fuer die Planes
    sal_uInt8*          pBuf=NULL;          // Buffer fuer eine Pixelzeile
    PcxExpand      aPcx;
    sal_uLong          nOfs;
    sal_uInt8           cRGB[4];

    if (rHead.Planes<=1) nColBits=1; else nColBits=4; if (rHead.Typ==4) nColBits=8;
    nColors=1<<nColBits;
    nWdtOut=((rHead.Xsize*nColBits+31)/32)*4;
    aBmpHead.Typ='B'+'M'*256;
    aBmpHead.SetOfs(sizeof(aBmpHead)+sizeof(aBmpInfo)+nColors*4);
    aBmpHead.SetSize(aBmpHead.GetOfs()+nWdtOut*rHead.Ysize);
    aBmpHead.Reserve1=0;
    aBmpHead.Reserve2=0;
    aBmpInfo.Size=sizeof(aBmpInfo);
    aBmpInfo.Width=rHead.Xsize;
    aBmpInfo.Hight=rHead.Ysize;
    aBmpInfo.Planes=1;
    aBmpInfo.PixBits=nColBits;
    aBmpInfo.Compress=0;
    aBmpInfo.ImgSize=0;
    aBmpInfo.xDpmm=0;
    aBmpInfo.yDpmm=0;
    aBmpInfo.ColUsed=0;
    aBmpInfo.ColMust=0;
    pBuf=new sal_uInt8[nWdtOut];
    if (!pBuf) return sal_False;       // Fehler: kein Speichel da
    rOut<<aBmpHead<<aBmpInfo;
    memset(pBuf,0,nWdtOut);        // Buffer mit Nullen fuellen

    if (nColors==2)
    {

        rOut<<RGBQuad(0x00,0x00,0x00); // Schwarz
        rOut<<RGBQuad(0xFF,0xFF,0xFF); // Weiss
        nOfs=rOut.Tell();
        for (j=0;j<rHead.Ysize;j++)
            rOut.Write((char*)pBuf,nWdtOut);  // Datei erstmal komplett mit Nullen fuellen
        for (j=0;j<rHead.Ysize;j++) {
            for(i=0;i<nWdtInp;i++) {
                pBuf[i]=aPcx.GetByte(rInp);
            }
            for(i=nWdtInp;i<nWdtOut;i++) pBuf[i]=0;     // noch bis zu 3 Bytes
            rOut.Seek(nOfs+((sal_uLong)rHead.Ysize-j-1L)*(sal_uLong)nWdtOut); // rueckwaerts schreiben!
            rOut.Write((char*)pBuf,nWdtOut);
        }
    } else if (nColors==16) {
        sal_uInt8 pl2= 0;     // Masken fuer die Planes

        rOut<<RGBQuad(0x00,0x00,0x00); // Schwarz
        rOut<<RGBQuad(0x24,0x24,0x24); // Grau 80%
        rOut<<RGBQuad(0x49,0x49,0x49); // Grau 60%
        rOut<<RGBQuad(0x92,0x92,0x92); // Grau 40%
        rOut<<RGBQuad(0x6D,0x6D,0x6D); // Grau 30%
        rOut<<RGBQuad(0xB6,0xB6,0xB6); // Grau 20%
        rOut<<RGBQuad(0xDA,0xDA,0xDA); // Grau 10%
        rOut<<RGBQuad(0xFF,0xFF,0xFF); // Weiss
        rOut<<RGBQuad(0x00,0x00,0x00); // Schwarz
        rOut<<RGBQuad(0xFF,0x00,0x00); // Rot
        rOut<<RGBQuad(0x00,0x00,0xFF); // Blau
        rOut<<RGBQuad(0xFF,0x00,0xFF); // Magenta
        rOut<<RGBQuad(0x00,0xFF,0x00); // Gruen
        rOut<<RGBQuad(0xFF,0xFF,0x00); // Gelb
        rOut<<RGBQuad(0x00,0xFF,0xFF); // Cyan
        rOut<<RGBQuad(0xFF,0xFF,0xFF); // Weiss

        nOfs=rOut.Tell();
        for (j=0;j<rHead.Ysize;j++)
            rOut.Write((char*)pBuf,nWdtOut);  // Datei erstmal komplett mit Nullen fuellen
        for (j=0;j<rHead.Ysize;j++) {
            memset(pBuf,0,nWdtOut);
            for(k=0;k<4;k++) {
                if (k==0) {
                    pl1=0x10; pl2=0x01;
                } else {
                    pl1<<=1; pl2<<=1;
                }
                for(i=0;i<nWdtInp;i++) {
                    a=i*4;
                    b=aPcx.GetByte(rInp);
                    if (b & 0x80) pBuf[a  ]|=pl1;
                    if (b & 0x40) pBuf[a  ]|=pl2;
                    if (b & 0x20) pBuf[a+1]|=pl1;
                    if (b & 0x10) pBuf[a+1]|=pl2;
                    if (b & 0x08) pBuf[a+2]|=pl1;
                    if (b & 0x04) pBuf[a+2]|=pl2;
                    if (b & 0x02) pBuf[a+3]|=pl1;
                    if (b & 0x01) pBuf[a+3]|=pl2;
                }
            }
            for(i=nWdtInp*4;i<nWdtOut;i++) pBuf[i]=0;            // noch bis zu 3 Bytes
            rOut.Seek(nOfs+((sal_uLong)rHead.Ysize-j-1L)*(sal_uLong)nWdtOut); // rueckwaerts schreiben!
            rOut.Write((char*)pBuf,nWdtOut);
        }
    } else if (nColors==256) {
        cRGB[3]=0;                      // der 4. Paletteneintrag fuer BMP
        for (i=0;i<256;i++) {           // Palette kopieren
            rInp.Read((char*)cRGB,3);
            pl1=cRGB[0];                // Rot mit Blau tauschen
            cRGB[0]=cRGB[2];
            cRGB[2]=pl1;
            rOut.Write((char*)cRGB,4);
        }

        nOfs=rOut.Tell();
        for (j=0;j<rHead.Ysize;j++)
            rOut.Write((char*)pBuf,nWdtOut);  // Datei erstmal komplett mit Nullen fuellen
        for (j=0;j<rHead.Ysize;j++) {
            for(i=0;i<rHead.Xsize;i++)
                pBuf[i]=aPcx.GetByte(rInp);
            for(i=rHead.Xsize;i<nWdtOut;i++) pBuf[i]=0;          // noch bis zu 3 Bytes
            rOut.Seek(nOfs+((sal_uLong)rHead.Ysize-j-1L)*(sal_uLong)nWdtOut); // rueckwaerts schreiben!
            rOut.Write((char*)pBuf,nWdtOut);
        }
    }
    delete[] pBuf;
    return sal_True;
}

sal_Bool SgfBMapFilter(SvStream& rInp, SvStream& rOut)
{
    sal_uLong     nFileStart;            // Offset des SgfHeaders. Im allgemeinen 0.
    SgfHeader aHead;
    SgfEntry  aEntr;
    sal_uLong     nNext;
    sal_Bool      bRdFlag=sal_False;         // Grafikentry gelesen ?
    sal_Bool      bRet=sal_False;            // Returncode

    nFileStart=rInp.Tell();
    rInp>>aHead;
    if (aHead.ChkMagic() && (aHead.Typ==SgfBitImag0 || aHead.Typ==SgfBitImag1 ||
                             aHead.Typ==SgfBitImag2 || aHead.Typ==SgfBitImgMo)) {
        nNext=aHead.GetOffset();
        while (nNext && !bRdFlag && !rInp.GetError() && !rOut.GetError()) {
            rInp.Seek(nFileStart+nNext);
            rInp>>aEntr;
            nNext=aEntr.GetOffset();
            if (aEntr.Typ==aHead.Typ) {
                bRdFlag=sal_True;
                switch(aEntr.Typ) {
                    case SgfBitImag0:
                    case SgfBitImag1:
                    case SgfBitImag2:
                    case SgfBitImgMo: bRet=SgfFilterBMap(rInp,rOut,aHead,aEntr); break;
                }
            }
        } // while(nNext)
    }
    if (rInp.GetError()) bRet=sal_False;
    return(bRet);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// SgfVectFilter ///////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// Fuer StarDraw Embedded SGF-Vector
long SgfVectXofs=0;
long SgfVectYofs=0;
long SgfVectXmul=0;
long SgfVectYmul=0;
long SgfVectXdiv=0;
long SgfVectYdiv=0;
sal_Bool SgfVectScal=sal_False;

////////////////////////////////////////////////////////////
// Hpgl2SvFarbe ////////////////////////////////////////////
////////////////////////////////////////////////////////////

Color Hpgl2SvFarbe( sal_uInt8 nFarb )
{
    sal_uLong nColor = COL_BLACK;

    switch (nFarb & 0x07) {
        case 0:  nColor=COL_WHITE;        break;
        case 1:  nColor=COL_YELLOW;       break;
        case 2:  nColor=COL_LIGHTMAGENTA; break;
        case 3:  nColor=COL_LIGHTRED;     break;
        case 4:  nColor=COL_LIGHTCYAN;    break;
        case 5:  nColor=COL_LIGHTGREEN;   break;
        case 6:  nColor=COL_LIGHTBLUE;    break;
        case 7:  nColor=COL_BLACK;        break;
    }
    Color aColor( nColor );
    return aColor;
}

sal_Bool SgfFilterVect(SvStream& rInp, SgfHeader& rHead, SgfEntry&, GDIMetaFile& rMtf)
{
    VirtualDevice aOutDev;
    SgfVector aVect;
    sal_uInt8      nFarb;
    sal_uInt8      nFrb0=7;
    sal_uInt8      nLTyp;
    sal_uInt8      nOTyp;
    sal_Bool      bEoDt=sal_False;
    sal_Bool      bPDwn=sal_False;
    Point     aP0(0,0);
    Point     aP1(0,0);
    sal_uInt16    RecNr=0;

    rMtf.Record(&aOutDev);
    aOutDev.SetLineColor(Color(COL_BLACK));
    aOutDev.SetFillColor(Color(COL_BLACK));

    while (!bEoDt && !rInp.GetError()) {
        rInp>>aVect; RecNr++;
        nFarb=(sal_uInt8) (aVect.Flag & 0x000F);
        nLTyp=(sal_uInt8)((aVect.Flag & 0x00F0) >>4);
        nOTyp=(sal_uInt8)((aVect.Flag & 0x0F00) >>8);
        bEoDt=(aVect.Flag & 0x4000) !=0;
        bPDwn=(aVect.Flag & 0x8000) !=0;

        long x=aVect.x-rHead.Xoffs;
        long y=rHead.Ysize-(aVect.y-rHead.Yoffs);
        if (SgfVectScal) {
            if (SgfVectXdiv==0) SgfVectXdiv=rHead.Xsize;
            if (SgfVectYdiv==0) SgfVectYdiv=rHead.Ysize;
            if (SgfVectXdiv==0) SgfVectXdiv=1;
            if (SgfVectYdiv==0) SgfVectYdiv=1;
            x=SgfVectXofs+ x *SgfVectXmul /SgfVectXdiv;
            y=SgfVectYofs+ y *SgfVectXmul /SgfVectYdiv;
        }
        aP1=Point(x,y);
        if (!bEoDt && !rInp.GetError()) {
            if (bPDwn && nLTyp<=6) {
                switch(nOTyp) {
                    case 1: if (nFarb!=nFrb0) {
                                switch(rHead.SwGrCol) {
                                    case SgfVectFarb: aOutDev.SetLineColor(Hpgl2SvFarbe(nFarb)); break;
                                    case SgfVectGray:                          break;
                                    case SgfVectWdth:                          break;
                                }
                            }
                            aOutDev.DrawLine(aP0,aP1);            break; // Linie
                    case 2:                                       break; // Kreis
                    case 3:                                       break; // Text
                    case 5: aOutDev.DrawRect(Rectangle(aP0,aP1)); break; // Rechteck (solid)
                }
            }
            aP0=aP1;
            nFrb0=nFarb;
        }
    }
    rMtf.Stop();
    rMtf.WindStart();
    MapMode aMap( MAP_10TH_MM, Point(),
                  Fraction( 1, 4 ), Fraction( 1, 4 ) );
    rMtf.SetPrefMapMode( aMap );
    rMtf.SetPrefSize( Size( (short)rHead.Xsize, (short)rHead.Ysize ) );
    return sal_True;
}


sal_Bool SgfVectFilter(SvStream& rInp, GDIMetaFile& rMtf)
{
    sal_uLong     nFileStart;            // Offset des SgfHeaders. Im allgemeinen 0.
    SgfHeader aHead;
    SgfEntry  aEntr;
    sal_uLong     nNext;
    sal_Bool      bRdFlag=sal_False;         // Grafikentry gelesen ?
    sal_Bool      bRet=sal_False;            // Returncode

    nFileStart=rInp.Tell();
    rInp>>aHead;
    if (aHead.ChkMagic() && aHead.Typ==SGF_SIMPVECT) {
        nNext=aHead.GetOffset();
        while (nNext && !bRdFlag && !rInp.GetError()) {
            rInp.Seek(nFileStart+nNext);
            rInp>>aEntr;
            nNext=aEntr.GetOffset();
            if (aEntr.Typ==aHead.Typ) {
                bRet=SgfFilterVect(rInp,aHead,aEntr,rMtf);
            }
        } // while(nNext)
        if (bRdFlag) {
            if (!rInp.GetError()) bRet=sal_True;  // Scheinbar Ok
        }
    }
    return(bRet);
}

/*************************************************************************
|*
|*    CheckSgfTyp()
|*
|*    Beschreibung      Feststellen, um was fuer ein SGF/SGV es sich handelt.
|*
*************************************************************************/
sal_uInt8 CheckSgfTyp(SvStream& rInp, sal_uInt16& nVersion)
{
#if OSL_DEBUG_LEVEL > 1 // Recordgroessen checken. Neuer Compiler hat vielleichte anderes Allignment!
    if (sizeof(SgfHeader)!=SgfHeaderSize ||
        sizeof(SgfEntry) !=SgfEntrySize  ||
        sizeof(SgfVector)!=SgfVectorSize ||
        sizeof(BmpFileHeader)!=BmpFileHeaderSize ||
        sizeof(BmpInfoHeader)!=BmpInfoHeaderSize ||
        sizeof(RGBQuad  )!=RGBQuadSize   )  return SGF_DONTKNOW;
#endif

    sal_uLong     nPos;
    SgfHeader aHead;
    nVersion=0;
    nPos=rInp.Tell();
    rInp>>aHead;
    rInp.Seek(nPos);
    if (aHead.ChkMagic()) {
        nVersion=aHead.Version;
        switch(aHead.Typ) {
            case SgfBitImag0:
            case SgfBitImag1:
            case SgfBitImag2:
            case SgfBitImgMo: return SGF_BITIMAGE;
            case SgfSimpVect: return SGF_SIMPVECT;
            case SgfPostScrp: return SGF_POSTSCRP;
            case SgfStarDraw: return SGF_STARDRAW;
            default         : return SGF_DONTKNOW;
        }
    } else {
        return SGF_DONTKNOW;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
