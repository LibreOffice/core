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
#include <tools/fract.hxx>
#include <vcl/gdimtf.hxx>
#include <tools/color.hxx>
#include <vcl/virdev.hxx>
#include "sgffilt.hxx"
#include "sgfbram.hxx"
#include <memory>

SgfHeader::SgfHeader()
{
    memset( this, 0, sizeof( SgfHeader ) );
}

SvStream& ReadSgfHeader(SvStream& rIStream, SgfHeader& rHead)
{
    rIStream.Read(&rHead.Magic, SgfHeaderSize);
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

bool SgfHeader::ChkMagic()
{ return Magic=='J'*256+'J'; }

sal_uInt32 SgfHeader::GetOffset()
{ return sal_uInt32(OfsLo)+0x00010000*sal_uInt32(OfsHi); }

SgfEntry::SgfEntry()
{
    memset( this, 0, sizeof( SgfEntry ) );
}

SvStream& ReadSgfEntry(SvStream& rIStream, SgfEntry& rEntr)
{
    rIStream.Read(&rEntr.Typ, SgfEntrySize);
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

SvStream& ReadSgfVector(SvStream& rIStream, SgfVector& rVect)
{
    rIStream.Read(&rVect, sizeof(rVect));
#if defined OSL_BIGENDIAN
    rVect.Flag =OSL_SWAPWORD(rVect.Flag );
    rVect.x    =OSL_SWAPWORD(rVect.x    );
    rVect.y    =OSL_SWAPWORD(rVect.y    );
    rVect.OfsLo=OSL_SWAPDWORD (rVect.OfsLo);
    rVect.OfsHi=OSL_SWAPDWORD (rVect.OfsHi);
#endif
    return rIStream;
}

SvStream& WriteBmpFileHeader(SvStream& rOStream, BmpFileHeader& rHead)
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
    rOStream.Write(&rHead, sizeof(rHead));
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

SvStream& WriteBmpInfoHeader(SvStream& rOStream, BmpInfoHeader& rInfo)
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
    rOStream.Write(&rInfo, sizeof(rInfo));
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

SvStream& WriteRGBQuad(SvStream& rOStream, const RGBQuad& rQuad)
{
    rOStream.Write(&rQuad, sizeof(rQuad));
    return rOStream;
}

class PcxExpand
{
private:
    sal_uInt16 Count;
    sal_uInt8   Data;
public:
    PcxExpand()
        : Count(0)
        , Data(0)
    {}
    sal_uInt8 GetByte(SvStream& rInp);
};

sal_uInt8 PcxExpand::GetByte(SvStream& rInp)
{
    if (Count>0) {
        Count--;
    } else {
        rInp.Read(&Data, 1);
        if ((Data & 0xC0) == 0xC0) {
            Count=(Data & 0x3F) -1;
            rInp.Read(&Data, 1);
        }
    }
    return Data;
}

bool SgfFilterBMap(SvStream& rInp, SvStream& rOut, SgfHeader& rHead, SgfEntry&)
{
    BmpFileHeader  aBmpHead;
    BmpInfoHeader  aBmpInfo;
    sal_uInt16 nWdtInp=(rHead.Xsize+7)/8;  // width of input bitmap in bytes
    sal_uInt16         nWdtOut;            // width of output bitmap in bytes
    sal_uInt16         nColors;            // color count (1, 16, 256)
    sal_uInt16         nColBits;           // number of bits per pixel (2, 4, 8)
    sal_uInt16         i,j,k;              // column/row/plane counter
    sal_uInt16         a,b;                // helper variables
    sal_uInt8           pl1 = 0;            // masks for the planes
    std::unique_ptr<sal_uInt8[]> pBuf;   // buffer for a pixel row
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
    pBuf.reset(new sal_uInt8[nWdtOut]);
    if (!pBuf) return false;       // error: no more memory available
    WriteBmpFileHeader( rOut, aBmpHead );
    WriteBmpInfoHeader( rOut, aBmpInfo );
    memset(pBuf.get(),0,nWdtOut);        // fill buffer with zeroes

    if (nColors==2)
    {

        WriteRGBQuad( rOut, RGBQuad(0x00,0x00,0x00) ); // black
        WriteRGBQuad( rOut, RGBQuad(0xFF,0xFF,0xFF) ); // white
        nOfs=rOut.Tell();
        for (j=0;j<rHead.Ysize;j++)
            rOut.Write(pBuf.get(), nWdtOut); // fill file with zeroes
        for (j=0;j<rHead.Ysize;j++) {
            for(i=0;i<nWdtInp;i++) {
                pBuf[i]=aPcx.GetByte(rInp);
            }
            for(i=nWdtInp;i<nWdtOut;i++) pBuf[i]=0;     // up to 3 bytes
            rOut.Seek(nOfs+((sal_uLong)rHead.Ysize-j-1L)*(sal_uLong)nWdtOut); // write backwards
            rOut.Write(pBuf.get(), nWdtOut);
        }
    } else if (nColors==16) {
        sal_uInt8 pl2= 0;     // planes' masks

        WriteRGBQuad( rOut, RGBQuad(0x00,0x00,0x00) ); // black
        WriteRGBQuad( rOut, RGBQuad(0x24,0x24,0x24) ); // gray 80%
        WriteRGBQuad( rOut, RGBQuad(0x49,0x49,0x49) ); // gray 60%
        WriteRGBQuad( rOut, RGBQuad(0x92,0x92,0x92) ); // gray 40%
        WriteRGBQuad( rOut, RGBQuad(0x6D,0x6D,0x6D) ); // gray 30%
        WriteRGBQuad( rOut, RGBQuad(0xB6,0xB6,0xB6) ); // gray 20%
        WriteRGBQuad( rOut, RGBQuad(0xDA,0xDA,0xDA) ); // gray 10%
        WriteRGBQuad( rOut, RGBQuad(0xFF,0xFF,0xFF) ); // white
        WriteRGBQuad( rOut, RGBQuad(0x00,0x00,0x00) ); // black
        WriteRGBQuad( rOut, RGBQuad(0xFF,0x00,0x00) ); // red
        WriteRGBQuad( rOut, RGBQuad(0x00,0x00,0xFF) ); // blue
        WriteRGBQuad( rOut, RGBQuad(0xFF,0x00,0xFF) ); // magenta
        WriteRGBQuad( rOut, RGBQuad(0x00,0xFF,0x00) ); // green
        WriteRGBQuad( rOut, RGBQuad(0xFF,0xFF,0x00) ); // yellow
        WriteRGBQuad( rOut, RGBQuad(0x00,0xFF,0xFF) ); // cyan
        WriteRGBQuad( rOut, RGBQuad(0xFF,0xFF,0xFF) ); // white

        nOfs=rOut.Tell();
        for (j=0;j<rHead.Ysize;j++)
            rOut.Write(pBuf.get(), nWdtOut); // fill file with zeroes
        for (j=0;j<rHead.Ysize;j++) {
            memset(pBuf.get(),0,nWdtOut);
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
            for(i=nWdtInp*4;i<nWdtOut;i++) pBuf[i]=0;            // up to 3 bytes
            rOut.Seek(nOfs+((sal_uLong)rHead.Ysize-j-1L)*(sal_uLong)nWdtOut); // write backwards
            rOut.Write(pBuf.get(), nWdtOut);
        }
    } else if (nColors==256) {
        cRGB[3]=0;                      // fourth palette entry for BMP
        for (i=0;i<256;i++) {           // copy palette
            rInp.Read(cRGB, 3);
            pl1=cRGB[0];                // switch red and blue
            cRGB[0]=cRGB[2];
            cRGB[2]=pl1;
            rOut.Write(cRGB, 4);
        }

        nOfs=rOut.Tell();
        for (j=0;j<rHead.Ysize;j++)
            rOut.Write(pBuf.get(), nWdtOut); // fill file with zeroes
        for (j=0;j<rHead.Ysize;j++) {
            for(i=0;i<rHead.Xsize;i++)
                pBuf[i]=aPcx.GetByte(rInp);
            for(i=rHead.Xsize;i<nWdtOut;i++) pBuf[i]=0;          // up to 3 bytes
            rOut.Seek(nOfs+((sal_uLong)rHead.Ysize-j-1L)*(sal_uLong)nWdtOut); // write backwards
            rOut.Write(pBuf.get(), nWdtOut);
        }
    }
    return true;
}

bool SgfBMapFilter(SvStream& rInp, SvStream& rOut)
{
    sal_uLong     nFileStart;        // offset of SgfHeaders. Usually 0.
    SgfHeader aHead;
    SgfEntry  aEntr;
    sal_uLong     nNext;
    bool      bRet=false;            // return value

    nFileStart=rInp.Tell();
    ReadSgfHeader( rInp, aHead );
    if (aHead.ChkMagic() && (aHead.Typ==SgfBitImag0 || aHead.Typ==SgfBitImag1 ||
                             aHead.Typ==SgfBitImag2 || aHead.Typ==SgfBitImgMo))
    {
        bool bRdFlag = false;         // read graphics entry?
        nNext = aHead.GetOffset();
        while (nNext && !bRdFlag && !rInp.GetError() && !rOut.GetError()) {
            rInp.Seek(nFileStart+nNext);
            ReadSgfEntry( rInp, aEntr );
            nNext=aEntr.GetOffset();
            if (aEntr.Typ==aHead.Typ) {
                bRdFlag=true;
                switch(aEntr.Typ) {
                    case SgfBitImag0:
                    case SgfBitImag1:
                    case SgfBitImag2:
                    case SgfBitImgMo: bRet=SgfFilterBMap(rInp,rOut,aHead,aEntr); break;
                }
            }
        } // while(nNext)
    }
    if (rInp.GetError()) bRet=false;
    return bRet;
}

// for StarDraw embedded SGF vector
long SgfVectXofs=0;
long SgfVectYofs=0;
long SgfVectXmul=0;
long SgfVectYmul=0;
long SgfVectXdiv=0;
long SgfVectYdiv=0;
bool SgfVectScal=false;

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

bool SgfFilterVect(SvStream& rInp, SgfHeader& rHead, SgfEntry&, GDIMetaFile& rMtf)
{
    ScopedVclPtrInstance< VirtualDevice > aOutDev;
    SgfVector aVect;
    sal_uInt8      nFarb;
    sal_uInt8      nFrb0=7;
    sal_uInt8      nLTyp;
    sal_uInt8      nOTyp;
    bool      bEoDt=false;
    Point     aP0(0,0);
    Point     aP1(0,0);
    sal_uInt16    RecNr=0;

    rMtf.Record(aOutDev.get());
    aOutDev->SetLineColor(Color(COL_BLACK));
    aOutDev->SetFillColor(Color(COL_BLACK));

    while (!bEoDt && !rInp.GetError()) {
        ReadSgfVector( rInp, aVect ); RecNr++;
        nFarb=(sal_uInt8) (aVect.Flag & 0x000F);
        nLTyp=(sal_uInt8)((aVect.Flag & 0x00F0) >>4);
        nOTyp=(sal_uInt8)((aVect.Flag & 0x0F00) >>8);
        bEoDt=(aVect.Flag & 0x4000) !=0;
        bool bPDwn=(aVect.Flag & 0x8000) !=0;

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
                                    case SgfVectFarb: aOutDev->SetLineColor(Hpgl2SvFarbe(nFarb)); break;
                                    case SgfVectGray:                          break;
                                    case SgfVectWdth:                          break;
                                }
                            }
                            aOutDev->DrawLine(aP0,aP1);            break; // line
                    case 2:                                       break; // circle
                    case 3:                                       break; // text
                    case 5: aOutDev->DrawRect(Rectangle(aP0,aP1)); break; // rectangle (solid)
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
    return true;
}

bool SgfVectFilter(SvStream& rInp, GDIMetaFile& rMtf)
{
    sal_uLong     nFileStart;        // offset of SgfHeaders. Usually 0.
    SgfHeader aHead;
    SgfEntry  aEntr;
    sal_uLong     nNext;
    bool      bRet=false;            // return value

    nFileStart=rInp.Tell();
    ReadSgfHeader( rInp, aHead );
    if (aHead.ChkMagic() && aHead.Typ==SGF_SIMPVECT) {
        nNext=aHead.GetOffset();
        while (nNext && !rInp.GetError()) {
            rInp.Seek(nFileStart+nNext);
            ReadSgfEntry( rInp, aEntr );
            nNext=aEntr.GetOffset();
            if (aEntr.Typ==aHead.Typ) {
                bRet=SgfFilterVect(rInp,aHead,aEntr,rMtf);
            }
        } // while(nNext)
    }
    return bRet;
}

/*************************************************************************
|*
|*    CheckSgfTyp()
|*
|*    Description      determine which kind of SGF/SGV it is
|*
*************************************************************************/
sal_uInt8 CheckSgfTyp(SvStream& rInp, sal_uInt16& nVersion)
{
#if OSL_DEBUG_LEVEL > 1 // check record size, new Compiler had different alignment!
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
    ReadSgfHeader( rInp, aHead );
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
