/*************************************************************************
 *
 *  $RCSfile: sgfbram.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:59 $
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

#include <string.h>
#include <tools/stream.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/color.hxx>
#include <vcl/virdev.hxx>
#include "sgffilt.hxx"
#include "sgfbram.hxx"

#if defined( WIN ) && defined( MSC )
#pragma code_seg( "SVTOOLS_FILTER4", "SVTOOLS_CODE" )
#endif

/*************************************************************************
|*
|*    operator>>( SvStream&, SgfHeader& )
|*
|*    Beschreibung
|*    Ersterstellung    JOE 23.06.93
|*    Letzte Aenderung  JOE 23.06.93
|*
*************************************************************************/
SvStream& operator>>(SvStream& rIStream, SgfHeader& rHead)
{
    rIStream.Read((char*)&rHead.Magic,SgfHeaderSize);
#if defined __BIGENDIAN
    rHead.Magic  =SWAPSHORT(rHead.Magic  );
    rHead.Version=SWAPSHORT(rHead.Version);
    rHead.Typ    =SWAPSHORT(rHead.Typ    );
    rHead.Xsize  =SWAPSHORT(rHead.Xsize  );
    rHead.Ysize  =SWAPSHORT(rHead.Ysize  );
    rHead.Xoffs  =SWAPSHORT(rHead.Xoffs  );
    rHead.Yoffs  =SWAPSHORT(rHead.Yoffs  );
    rHead.Planes =SWAPSHORT(rHead.Planes );
    rHead.SwGrCol=SWAPSHORT(rHead.SwGrCol);
    rHead.OfsLo  =SWAPSHORT(rHead.OfsLo  );
    rHead.OfsHi  =SWAPSHORT(rHead.OfsHi  );
#endif
    return rIStream;
}


/*************************************************************************
|*
|*    SgfHeader::ChkMagic()
|*
|*    Beschreibung
|*    Ersterstellung    JOE 23.06.93
|*    Letzte Aenderung  JOE 23.06.93
|*
*************************************************************************/
BOOL SgfHeader::ChkMagic()
{ return Magic=='J'*256+'J'; }

UINT32 SgfHeader::GetOffset()
{ return UINT32(OfsLo)+0x00010000*UINT32(OfsHi); }


/*************************************************************************
|*
|*    operator>>( SvStream&, SgfEntry& )
|*
|*    Beschreibung
|*    Ersterstellung    JOE 23.06.93
|*    Letzte Aenderung  JOE 23.06.93
|*
*************************************************************************/
SvStream& operator>>(SvStream& rIStream, SgfEntry& rEntr)
{
    rIStream.Read((char*)&rEntr.Typ,SgfEntrySize);
#if defined __BIGENDIAN
    rEntr.Typ  =SWAPSHORT(rEntr.Typ  );
    rEntr.iFrei=SWAPSHORT(rEntr.iFrei);
    rEntr.lFreiLo=SWAPSHORT (rEntr.lFreiLo);
    rEntr.lFreiHi=SWAPSHORT (rEntr.lFreiHi);
    rEntr.OfsLo=SWAPSHORT(rEntr.OfsLo);
    rEntr.OfsHi=SWAPSHORT(rEntr.OfsHi);
#endif
    return rIStream;
}

UINT32 SgfEntry::GetOffset()
{ return UINT32(OfsLo)+0x00010000*UINT32(OfsHi); }


/*************************************************************************
|*
|*    operator>>( SvStream&, SgfVector& )
|*
|*    Beschreibung
|*    Ersterstellung    JOE 23.06.93
|*    Letzte Aenderung  JOE 23.06.93
|*
*************************************************************************/
SvStream& operator>>(SvStream& rIStream, SgfVector& rVect)
{
    rIStream.Read((char*)&rVect,sizeof(rVect));
#if defined __BIGENDIAN
    rVect.Flag =SWAPSHORT(rVect.Flag );
    rVect.x    =SWAPSHORT(rVect.x    );
    rVect.y    =SWAPSHORT(rVect.y    );
    rVect.OfsLo=SWAPLONG (rVect.OfsLo);
    rVect.OfsHi=SWAPLONG (rVect.OfsHi);
#endif
    return rIStream;
}


/*************************************************************************
|*
|*    operator<<( SvStream&, BmpFileHeader& )
|*
|*    Beschreibung
|*    Ersterstellung    JOE 23.06.93
|*    Letzte Aenderung  JOE 23.06.93
|*
*************************************************************************/
SvStream& operator<<(SvStream& rOStream, BmpFileHeader& rHead)
{
#if defined __BIGENDIAN
    rHead.Typ     =SWAPSHORT(rHead.Typ     );
    rHead.SizeLo  =SWAPSHORT(rHead.SizeLo  );
    rHead.SizeHi  =SWAPSHORT(rHead.SizeHi  );
    rHead.Reserve1=SWAPSHORT(rHead.Reserve1);
    rHead.Reserve2=SWAPSHORT(rHead.Reserve2);
    rHead.OfsLo   =SWAPSHORT(rHead.OfsLo   );
    rHead.OfsHi   =SWAPSHORT(rHead.OfsHi   );
#endif
    rOStream.Write((char*)&rHead,sizeof(rHead));
#if defined __BIGENDIAN
    rHead.Typ     =SWAPSHORT(rHead.Typ     );
    rHead.SizeLo  =SWAPSHORT(rHead.SizeLo  );
    rHead.SizeHi  =SWAPSHORT(rHead.SizeHi  );
    rHead.Reserve1=SWAPSHORT(rHead.Reserve1);
    rHead.Reserve2=SWAPSHORT(rHead.Reserve2);
    rHead.OfsLo   =SWAPSHORT(rHead.OfsLo   );
    rHead.OfsHi   =SWAPSHORT(rHead.OfsHi   );
#endif
    return rOStream;
}

void BmpFileHeader::SetSize(UINT32 Size)
{
    SizeLo=UINT16(Size & 0x0000FFFF);
    SizeHi=UINT16((Size & 0xFFFF0000)>>16);
}

void BmpFileHeader::SetOfs(UINT32 Ofs)
{
    OfsLo=UINT16(Ofs & 0x0000FFFF);
    OfsHi=UINT16((Ofs & 0xFFFF0000)>>16);
}

UINT32 BmpFileHeader::GetOfs()
{
    return UINT32(OfsLo)+0x00010000*UINT32(OfsHi);
}

/*************************************************************************
|*
|*    operator<<( SvStream&, BmpInfoHeader& )
|*
|*    Beschreibung
|*    Ersterstellung    JOE 23.06.93
|*    Letzte Aenderung  JOE 23.06.93
|*
*************************************************************************/
SvStream& operator<<(SvStream& rOStream, BmpInfoHeader& rInfo)
{
#if defined __BIGENDIAN
    rInfo.Size    =SWAPLONG (rInfo.Size    );
    rInfo.Width   =SWAPLONG (rInfo.Width   );
    rInfo.Hight   =SWAPLONG (rInfo.Hight   );
    rInfo.Planes  =SWAPSHORT(rInfo.Planes  );
    rInfo.PixBits =SWAPSHORT(rInfo.PixBits );
    rInfo.Compress=SWAPLONG (rInfo.Compress);
    rInfo.ImgSize =SWAPLONG (rInfo.ImgSize );
    rInfo.xDpmm   =SWAPLONG (rInfo.xDpmm   );
    rInfo.yDpmm   =SWAPLONG (rInfo.yDpmm   );
    rInfo.ColUsed =SWAPLONG (rInfo.ColUsed );
    rInfo.ColMust =SWAPLONG (rInfo.ColMust );
#endif
    rOStream.Write((char*)&rInfo,sizeof(rInfo));
#if defined __BIGENDIAN
    rInfo.Size    =SWAPLONG (rInfo.Size    );
    rInfo.Width   =SWAPLONG (rInfo.Width   );
    rInfo.Hight   =SWAPLONG (rInfo.Hight   );
    rInfo.Planes  =SWAPSHORT(rInfo.Planes  );
    rInfo.PixBits =SWAPSHORT(rInfo.PixBits );
    rInfo.Compress=SWAPLONG (rInfo.Compress);
    rInfo.ImgSize =SWAPLONG (rInfo.ImgSize );
    rInfo.xDpmm   =SWAPLONG (rInfo.xDpmm   );
    rInfo.yDpmm   =SWAPLONG (rInfo.yDpmm   );
    rInfo.ColUsed =SWAPLONG (rInfo.ColUsed );
    rInfo.ColMust =SWAPLONG (rInfo.ColMust );
#endif
    return rOStream;
}


/*************************************************************************
|*
|*    operator<<( SvStream&, RGBQuad& )
|*
|*    Beschreibung
|*    Ersterstellung    JOE 23.06.93
|*    Letzte Aenderung  JOE 23.06.93
|*
*************************************************************************/
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
    USHORT Count;
    BYTE   Data;
public:
                  PcxExpand() { Count=0; }
    BYTE GetByte(SvStream& rInp);
};

BYTE PcxExpand::GetByte(SvStream& rInp)
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


/*************************************************************************
|*
|*    SgfFilterBmp()
|*
|*    Beschreibung
|*    Ersterstellung    JOE 23.06.93
|*    Letzte Aenderung  JOE 23.06.93
|*
*************************************************************************/
BOOL SgfFilterBMap(SvStream& rInp, SvStream& rOut, SgfHeader& rHead, SgfEntry&)
{
    BmpFileHeader  aBmpHead;
    BmpInfoHeader  aBmpInfo;
    USHORT nWdtInp=(rHead.Xsize+7)/8;  // Breite der Input-Bitmap in Bytes
    USHORT         nWdtOut;            // Breite der Output-Bitmap in Bytes
    USHORT         nColors;            // Anzahl der Farben     (1,16,256)
    USHORT         nColBits;           // Anzahl der Bits/Pixel (2, 4,  8)
    USHORT         i,j,k;              // SpaltenzÑhler, ZeilenzÑhler, PlanezÑhler
    USHORT         a,b;                // Hilfsvariable
    BYTE           pl1,pl2;            // Masken fÅr die Planes
    BYTE*          pBuf=NULL;          // Buffer fÅr eine Pixelzeile
    PcxExpand      aPcx;
    ULONG          nOfs;
    BYTE           cRGB[4];

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
    pBuf=new BYTE[nWdtOut];
    if (!pBuf) return FALSE;       // Fehler: kein Speichel da
    rOut<<aBmpHead<<aBmpInfo;
    memset(pBuf,0,nWdtOut);        // Buffer mit Nullen fÅllen

    if (nColors==2)
    {

        rOut<<RGBQuad(0x00,0x00,0x00); // Schwarz
        rOut<<RGBQuad(0xFF,0xFF,0xFF); // Weiss
        nOfs=rOut.Tell();
        for (j=0;j<rHead.Ysize;j++)
            rOut.Write((char*)pBuf,nWdtOut);  // Datei erstmal komplett mit Nullen fÅllen
        for (j=0;j<rHead.Ysize;j++) {
            for(i=0;i<nWdtInp;i++) {
                pBuf[i]=aPcx.GetByte(rInp);
            }
            for(i=nWdtInp;i<nWdtOut;i++) pBuf[i]=0;     // noch bis zu 3 Bytes
            rOut.Seek(nOfs+((ULONG)rHead.Ysize-j-1L)*(ULONG)nWdtOut); // rÅckwÑrts schreiben!
            rOut.Write((char*)pBuf,nWdtOut);
        }
    } else if (nColors==16) {
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
            rOut.Write((char*)pBuf,nWdtOut);  // Datei erstmal komplett mit Nullen fÅllen
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
            rOut.Seek(nOfs+((ULONG)rHead.Ysize-j-1L)*(ULONG)nWdtOut); // rÅckwÑrts schreiben!
            rOut.Write((char*)pBuf,nWdtOut);
        }
    } else if (nColors==256) {
        cRGB[3]=0;                      // der 4. Paletteneintrag fÅr BMP
        for (i=0;i<256;i++) {           // Palette kopieren
            rInp.Read((char*)cRGB,3);
            pl1=cRGB[0];                // Rot mit Blau tauschen
            cRGB[0]=cRGB[2];
            cRGB[2]=pl1;
            rOut.Write((char*)cRGB,4);
        }

        nOfs=rOut.Tell();
        for (j=0;j<rHead.Ysize;j++)
            rOut.Write((char*)pBuf,nWdtOut);  // Datei erstmal komplett mit Nullen fÅllen
        for (j=0;j<rHead.Ysize;j++) {
            for(i=0;i<rHead.Xsize;i++)
                pBuf[i]=aPcx.GetByte(rInp);
            for(i=rHead.Xsize;i<nWdtOut;i++) pBuf[i]=0;          // noch bis zu 3 Bytes
            rOut.Seek(nOfs+((ULONG)rHead.Ysize-j-1L)*(ULONG)nWdtOut); // rÅckwÑrts schreiben!
            rOut.Write((char*)pBuf,nWdtOut);
        }
    }
    delete pBuf;
    return TRUE;
}


/*************************************************************************
|*
|*    SgfBMapFilter()
|*
|*    Beschreibung
|*    Ersterstellung    JOE 23.06.93
|*    Letzte Aenderung  JOE 23.06.93
|*
*************************************************************************/
BOOL SgfBMapFilter(SvStream& rInp, SvStream& rOut)
{
    ULONG     nFileStart;            // Offset des SgfHeaders. Im allgemeinen 0.
    SgfHeader aHead;
    SgfEntry  aEntr;
    ULONG     nNext;
    BOOL      bRdFlag=FALSE;         // Grafikentry gelesen ?
    BOOL      bRet=FALSE;            // Returncode

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
                bRdFlag=TRUE;
                switch(aEntr.Typ) {
                    case SgfBitImag0:
                    case SgfBitImag1:
                    case SgfBitImag2:
                    case SgfBitImgMo: bRet=SgfFilterBMap(rInp,rOut,aHead,aEntr); break;
                }
            }
        } // while(nNext)
    }
    if (rInp.GetError()) bRet=FALSE;
    return(bRet);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// SgfVectFilter ///////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// FÅr StarDraw Embedded SGF-Vector
long SgfVectXofs=0;
long SgfVectYofs=0;
long SgfVectXmul=0;
long SgfVectYmul=0;
long SgfVectXdiv=0;
long SgfVectYdiv=0;
BOOL SgfVectScal=FALSE;

////////////////////////////////////////////////////////////
// Hpgl2SvFarbe ////////////////////////////////////////////
////////////////////////////////////////////////////////////

Color Hpgl2SvFarbe( BYTE nFarb )
{
    ULONG nColor = COL_BLACK;

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

/*************************************************************************
|*
|*    SgfFilterVect()
|*
|*    Beschreibung
|*    Ersterstellung    JOE 23.06.93
|*    Letzte Aenderung  JOE 23.06.93
|*
*************************************************************************/
BOOL SgfFilterVect(SvStream& rInp, SgfHeader& rHead, SgfEntry&, GDIMetaFile& rMtf)
{
    VirtualDevice aOutDev;
    SgfVector aVect;
    BYTE      nFarb;
    BYTE      nFrb0=7;
    BYTE      nLTyp;
    BYTE      nOTyp;
    BOOL      bEoDt=FALSE;
    BOOL      bPDwn=FALSE;
    Point     aP0(0,0);
    Point     aP1(0,0);
    String    Msg;
    USHORT    RecNr=0;

    rMtf.Record(&aOutDev);
    aOutDev.SetLineColor(Color(COL_BLACK));
    aOutDev.SetFillColor(Color(COL_BLACK));

    while (!bEoDt && !rInp.GetError()) {
        rInp>>aVect; RecNr++;
        nFarb=(BYTE) (aVect.Flag & 0x000F);
        nLTyp=(BYTE)((aVect.Flag & 0x00F0) >>4);
        nOTyp=(BYTE)((aVect.Flag & 0x0F00) >>8);
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
    return TRUE;
}


/*************************************************************************
|*
|*    SgfVectFilter()
|*
|*    Beschreibung
|*    Ersterstellung    JOE 23.06.93
|*    Letzte Aenderung  JOE 23.06.93
|*
*************************************************************************/
BOOL SgfVectFilter(SvStream& rInp, GDIMetaFile& rMtf)
{
    ULONG     nFileStart;            // Offset des SgfHeaders. Im allgemeinen 0.
    SgfHeader aHead;
    SgfEntry  aEntr;
    ULONG     nNext;
    BOOL      bRdFlag=FALSE;         // Grafikentry gelesen ?
    BOOL      bRet=FALSE;            // Returncode

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
            if (!rInp.GetError()) bRet=TRUE;  // Scheinbar Ok
        }
    }
    return(bRet);
}


/*************************************************************************
|*
|*    SgfFilterPScr()
|*
|*    Beschreibung
|*    Ersterstellung    JOE 23.06.93
|*    Letzte Aenderung  JOE 23.06.93
|*
*************************************************************************/
BOOL SgfFilterPScr(SvStream&, SgfHeader&, SgfEntry&)
{
    return FALSE;  // PostSrcipt wird noch nicht unterstuetzt !
}


/*************************************************************************
|*
|*    CheckSgfTyp()
|*
|*    Beschreibung      Feststellen, uwas fÅr ein SGF/SGV es sich handelt.
|*    Ersterstellung    JOE 23.06.93
|*    Letzte Aenderung  JOE 23.06.93
|*
*************************************************************************/
BYTE CheckSgfTyp(SvStream& rInp, USHORT& nVersion)
{
#ifdef DEBUG  // Recordgrî·en checken. Neuer Compiler hat vielleichte anderes Allignment!
    if (sizeof(SgfHeader)!=SgfHeaderSize ||
        sizeof(SgfEntry) !=SgfEntrySize  ||
        sizeof(SgfVector)!=SgfVectorSize ||
        sizeof(BmpFileHeader)!=BmpFileHeaderSize ||
        sizeof(BmpInfoHeader)!=BmpInfoHeaderSize ||
        sizeof(RGBQuad  )!=RGBQuadSize   )  return SGF_DONTKNOW;
#endif

    ULONG     nPos;
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
