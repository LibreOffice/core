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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <stdlib.h>
#ifdef UNX
  #include <unistd.h>
#endif

#include <unotools/tempfile.hxx>
#include <vcl/graph.hxx>
#include <tools/poly.hxx>
#include <bf_svtools/filter.hxx>
#include <vcl/virdev.hxx>
#include <tools/cachestr.hxx>
#include <bf_svx/impgrf.hxx>

#include <shellio.hxx>
#include <w4wstk.hxx>          // fuer den Attribut Stack
#include <w4wpar.hxx>          // ERR_CHAR
#include <w4wgraf.hxx>         // eigenes
#include <osl/endian.h>
namespace binfilter {

#define MAX_MEM_GRAF 300000         // ab 300K im File statt im Speicher

////////////////////////////////////////////////////////////////////////////////////////////////////
// Windows BMP /////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class BmpFileHd
{                     // Vorsicht: Struktur ist nicht aligned
public:
    USHORT Typ;       // = "BM"
    ULONG  Size;      // Filesize in Bytes
    USHORT Reserve1;  // Reserviert
    USHORT Reserve2;  // Reserviert
    ULONG  Offset;    // Offset?

    friend SvStream& operator << ( SvStream& rOStream, BmpFileHd& rHead ); //$ ostream
};

#define BMP_FILE_HD_SIZ 14  // Groesse der Struktur BmpFileHd


class BmpInfoHd
{                      // Dieser Header ist aligned
public:
    ULONG  Size;       // Gr”áe des BmpInfoHeaders
    long   Width;      // Breite in Pixel
    long   Height;     // H”he in Pixel
    USHORT Planes;     // Anzahl der Planes (immer 1)

    USHORT PixBits;    // Anzahl der Bit je Pixel (1,4,8,oder 24)
    ULONG  Compress;   // Datenkompression
    ULONG  ImgSize;    // Gr”áe der Images in Bytes. Ohne Kompression ist auch 0 erlaubt.
    long   xDpmm;      // Dot per Meter (0 ist erlaubt)
    long   yDpmm;      // Dot per Meter (0 ist erlaubt)
    ULONG  ColUsed;    // Anzahl der verwendeten Farben (0=alle)
    ULONG  ColMust;    // Anzahl der wichtigen Farben (0=alle)

    friend SvStream& operator << ( SvStream& rOStream, BmpInfoHd& rHead ); //$ ostream
};

class RGBQuad {
private:
    unsigned char Red;
    unsigned char Grn;
    unsigned char Blu;
    unsigned char Fil;
public:
    RGBQuad( unsigned char R, unsigned char G, unsigned char B )
            { Red=R; Grn=G; Blu=B; Fil=0; }
};


SvStream& operator << ( SvStream& rOStream, BmpFileHd& rHead ) //$ ostream
{
    BmpFileHd aHd = rHead;

#ifndef OSL_LITENDIAN
    aHd.Typ     =SWAPSHORT( aHd.Typ );
    aHd.Size    =SWAPLONG ( aHd.Size );
    aHd.Reserve1=SWAPSHORT( aHd.Reserve1 );
    aHd.Reserve2=SWAPSHORT( aHd.Reserve2 );
    aHd.Offset  =SWAPLONG ( aHd.Offset );
#endif // !OSL_LITENDIAN

                            // Da MS die BmpFileHd-Struktur nicht aligned hat,
                            // muessen die Member einzeln 'rausgeschrieben werden,
                            // da die Groesse der Sruktur von Compiler zu Compiler
                            // unterschiedlich ist.

    rOStream.Write( (char*)&aHd.Typ, sizeof( aHd.Typ ) );
    rOStream.Write( (char*)&aHd.Size, sizeof( aHd.Size ) );
    rOStream.Write( (char*)&aHd.Reserve1, sizeof( aHd.Reserve1 ) );
    rOStream.Write( (char*)&aHd.Reserve2, sizeof( aHd.Reserve2 ) );
    rOStream.Write( (char*)&aHd.Offset, sizeof( aHd.Offset ) );

    return rOStream;
}


SvStream& operator << ( SvStream& rOStream, BmpInfoHd& rInfo) //$ ostream
{
#ifndef OSL_LITENDIAN
    BmpInfoHd aInfo = rInfo;

    aInfo.Size    =SWAPLONG ( aInfo.Size );
    aInfo.Width   =SWAPLONG ( aInfo.Width );
    aInfo.Height  =SWAPLONG ( aInfo.Height );
    aInfo.Planes  =SWAPSHORT( aInfo.Planes );
    aInfo.PixBits =SWAPSHORT( aInfo.PixBits );
    aInfo.Compress=SWAPLONG ( aInfo.Compress );
    aInfo.ImgSize =SWAPLONG ( aInfo.ImgSize );
    aInfo.xDpmm   =SWAPLONG ( aInfo.xDpmm );
    aInfo.yDpmm   =SWAPLONG ( aInfo.yDpmm );
    aInfo.ColUsed =SWAPLONG ( aInfo.ColUsed );
    aInfo.ColMust =SWAPLONG ( aInfo.ColMust );

    rOStream.Write( (char*)&aInfo, sizeof(aInfo) );
#else // !OSL_LITENDIAN
    rOStream.Write( (char*)&rInfo, sizeof(rInfo) );
#endif // !OSL_LITENDIAN
    return rOStream;
}


SwW4WGraf::SwW4WGraf( SvStream& rIstream )
    : rInp( rIstream )
{
    pBmp = 0;
    pGraph = 0;
    nError = 0;
    nStat = 0;
    pPal = 0;
    nPalColors = 0;
    pBmpHead = 0;
    pBmpInfo = 0;
    nVectMinX = nVectMinY = USHRT_MAX;
    nVectMaxX = nVectMaxY = 0;
}


SwW4WGraf::~SwW4WGraf()
{
    if( pGraph )
        delete pGraph;
    if( pBmp )
        delete pBmp;
}


short SwW4WGraf::GetHexByte()       // Ret: -1 fuer W4W_TXT_TERM, W4W_RED
{
    register unsigned int c;
    register unsigned int nHexVal = 0;              // Default fuer Error

    for ( int i = 0; i < 2; ++i )
    {
        c = ReadChar();
        if( c == 0 )
        {						// eof oder Stuss
            nError = ERR_CHAR;
            return -1;
        }
        if( c >= '0' && c <= '9' )      (nHexVal <<= 4 )+= (c - '0');
        else if( c >= 'A' && c <= 'F' ) (nHexVal <<= 4 )+= (c - 'A' + 10);
        else if( c >= 'a' && c <= 'f' ) (nHexVal <<= 4 )+= (c - 'a' + 10);
        else
        {
            nError = ERR_CHAR;
            return 0;
        }
    }

    return (short)nHexVal;
}


USHORT SwW4WGraf::GetHexUShort()
{
    int c = (int)GetHexByte();
    return( (BYTE)c | ( (USHORT)GetHexByte() << 8 ) );
}


ULONG SwW4WGraf::GetHexULong()
{
    register ULONG c = 0;
    register int i;
    for ( i=0; i<32; i+=8 )
        c |= ((ULONG)GetHexByte()) << i;
    return c;
}


long SwW4WGraf::GetVarSize()    // Zum Lesen der Recordlaenge (in 1..5 Byte codiert)
{
    BYTE c0, c1, c2, c3, c4;

    if ( ( c0 = (BYTE)GetHexByte() ) != 0xff )  // 1-Byte-Darstellung : Wert
        return c0;

    c1 = (BYTE)GetHexByte();
    c2 = (BYTE)GetHexByte();
    if ( ( c2 & 0x80 ) == 0 )               // 3-Byte-Darstellung :
        return ( (USHORT)c2 << 8 ) | c1;    // 0xff LowByte HiByte

    c3 = (BYTE)GetHexByte();                        // 5-Byte-Darstellung:
    c4 = (BYTE)GetHexByte();                        // 0xff Byte2 (Byte3 | 0x80) Byte0 Byte1
    return(   ( ( (long)  c2 & 0x7f ) << 24 )
            | (   (long)  c1          << 16 )
            | (   (USHORT)c4          <<  8 )
            |             c3                  );
}


short SwW4WGraf::DefaultPalette( USHORT nColors )
{
    pPal = (RGBQuad*)new char[ nColors*sizeof(RGBQuad) ];
    if ( !pPal ) return -1;
    nPalColors = nColors;
    register RGBQuad* pP = pPal;

    if (nColors==2) {
        *pP++ = RGBQuad(0x00,0x00,0x00); // Schwarz
        *pP++ = RGBQuad(0xFF,0xFF,0xFF); // Weiss
    }
    else if (nColors==16)
    {
        *pP++ = RGBQuad(0x00,0x00,0x00); // Schwarz
        *pP++ = RGBQuad(0x24,0x24,0x24); // Grau 80%
        *pP++ = RGBQuad(0x49,0x49,0x49); // Grau 60%
        *pP++ = RGBQuad(0x92,0x92,0x92); // Grau 40%
        *pP++ = RGBQuad(0x6D,0x6D,0x6D); // Grau 30%
        *pP++ = RGBQuad(0xB6,0xB6,0xB6); // Grau 20%
        *pP++ = RGBQuad(0xDA,0xDA,0xDA); // Grau 10%
        *pP++ = RGBQuad(0xFF,0xFF,0xFF); // Weiss
        *pP++ = RGBQuad(0x00,0x00,0x00); // Schwarz
        *pP++ = RGBQuad(0xFF,0x00,0x00); // Rot
        *pP++ = RGBQuad(0x00,0x00,0xFF); // Blau
        *pP++ = RGBQuad(0xFF,0x00,0xFF); // Magenta
        *pP++ = RGBQuad(0x00,0xFF,0x00); // Gruen
        *pP++ = RGBQuad(0xFF,0xFF,0x00); // Gelb
        *pP++ = RGBQuad(0x00,0xFF,0xFF); // Cyan
        *pP++ = RGBQuad(0xFF,0xFF,0xFF); // Weiss
    } else
        return -1;
    return 0;
}



short SwW4WGraf::ReadPalette( long nVarSize )
{
    nPalColors = (USHORT)(nVarSize / 3);
    pPal = (RGBQuad*)new char[ nPalColors*sizeof(RGBQuad) ];
    if ( !pPal ){ nPalColors = 0; return -1; };
    register RGBQuad* pP = pPal;
    USHORT i;

    for (i=0; i<nPalColors; i++)
        *pP++ = RGBQuad( (BYTE)GetHexByte(), (BYTE)GetHexByte(), (BYTE)GetHexByte() );
    return 0;
}



short SwW4WGraf::ReadBitmap( long )	// Mastersoft internal Format Bitmap
{
    ULONG n4Width = GetHexULong();                  // Lies Header ein
    ULONG n4Height = GetHexULong();
    USHORT n4ResWidth = GetHexUShort();
    USHORT n4ResHeight = GetHexUShort();
    USHORT n4ColBits = GetHexUShort();

    USHORT nColors = (1 << n4ColBits);              // Anzahl der Farben     (1,16,256)
    USHORT nWdtIn = (USHORT)((n4Width*n4ColBits+7)/8);
    USHORT nWdtOut = (USHORT)(((n4Width*n4ColBits+31)/32)*4);

if( 24 > n4ColBits )
{


    if ( !pPal || ( nColors != nPalColors ) ){      // keine oder falsche Palette
        if ( pPal ) DELETEZ( pPal );
        if ( DefaultPalette( nColors ) < 0 )        // mach selbst
            return -1;
    }


}


    pBmpHead = new BmpFileHd;                           // erzeuge BMP-Header
    if ( !pBmpHead ) return -1;
    pBmpInfo = new BmpInfoHd;
    if ( !pBmpInfo ) return -1;

    pBmpHead->Typ='B'+'M'*256;
    pBmpHead->Offset=BMP_FILE_HD_SIZ+sizeof(*pBmpInfo);
    if( pPal ) pBmpHead->Offset += nColors*4;
    pBmpHead->Size=pBmpHead->Offset+nWdtOut*n4Height;   // Reihenfolge wichtig
    pBmpHead->Reserve1=0;
    pBmpHead->Reserve2=0;

    pBmpInfo->Size=sizeof(*pBmpInfo);
    pBmpInfo->Width=n4Width;
    pBmpInfo->Height=n4Height;
    pBmpInfo->Planes=1;
    pBmpInfo->PixBits=n4ColBits;
    pBmpInfo->Compress=0;
    pBmpInfo->ImgSize=0;
    pBmpInfo->xDpmm=n4ResWidth*1000L/254L;      // DPI in Pixel per Meter
    pBmpInfo->yDpmm=n4ResHeight*1000L/254;      // dito
    pBmpInfo->ColUsed=0;
    pBmpInfo->ColMust=0;

    if ( nWdtOut != (size_t)nWdtOut ) return -1;    // Zeile nicht allozierbar
    BYTE* pBuf = new BYTE[nWdtOut];
    if ( !pBuf ) return -1;       // Fehler: kein Speicher da

    String sExt(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM( ".bmp" )));
    ::utl::TempFile aTempFile(aEmptyStr, &sExt, 0);
    aTempFile.EnableKillingFile();

    SvStream* pOut = aTempFile.GetStream( STREAM_READ | STREAM_WRITE |
                                          STREAM_TRUNC );

    *pOut << *pBmpHead<<*pBmpInfo;
    if ( pPal )
        pOut->Write( (char*)pPal, nPalColors*sizeof(*pPal) );    // hier kein ByteSwap noetig
                                                                // da nur Bytes enthalten
    USHORT x, y;
    short nLeft;
    BYTE c, nRun;
    BYTE* pB;

    for( y=0; y<n4Height; y++)
    {
        nLeft = nWdtIn;
        pB = pBuf;
        while ( nLeft > 0 ){
            nRun = (BYTE)GetHexByte();
            if ( ( nRun & 0x80 ) != 0 ){    // komprimiert
                nRun &= 0x7f;
                c = (BYTE)GetHexByte();
                for ( x=0; x<nRun; x++){
                    *pB++ = c;
                }
            }else{                          // unkomprimiert
                for ( x=0; x<nRun; x++){
                    *pB++ = (BYTE)GetHexByte();
                }
            }
            nLeft -= nRun;
        }
        for ( x=nWdtIn; x<nWdtOut; x++ )
            *pB++ = 0x00;                   // 0..3 Bytes Luecke
        pOut->Write( (char*)pBuf, (size_t)nWdtOut );
    }
    pOut->Seek(0);
    GraphicFilter& rGF = *GetGrfFilter();      // lese ueber Filter ein
    pGraph = new Graphic;
    BOOL bOk = ( rGF.ImportGraphic( *pGraph, aEmptyStr, *pOut,
                                    GRFILTER_FORMAT_DONTKNOW )
            == 0 );

    if( !bOk )
        DELETEZ( pGraph );

    return ( bOk ) ? 0 : -1;			   // OK / Error
}


void SwW4WGraf::SkipBytes( register long nBytes )
{
    nBytes *= 2;                            // 1 Byte = 2 Chars im File
    rInp.SeekRel( nBytes );
}

#define W4WRG_Ignore    0x0000   /* Dummy                                 */
#define W4WRG_RecdID    0x0001   /* orginal file type, etc.               */
#define W4WRG_DefPal    0x0002   /* Define Palette                        */
#define W4WRG_DefPen    0x0003   /* Define Pen                            */
#define W4WRG_DefBrs    0x0004   /* Define Brush                          */
#define W4WRG_DefMrk    0x0005   /* Define Marker                         */
#define W4WRG_MoveTo    0x0008   /* Move To                               */
#define W4WRG_LineTo    0x0009   /* Line To                               */
#define W4WRG_FloodF    0x000A   /* Floodfill Area                        */
#define W4WRG_SetFil    0x000B   /* Set Fillmode                          */
#define W4WRG_DrMark    0x000C   /* Draw Marker                           */
#define W4WRG_DrMrkP    0x000D   /* Draw an Array of Markers              */
#define W4WRG_DrRect    0x000E   /* Draw a SRectangle                      */
#define W4WRG_DrPLin    0x000F   /* Draw a Polyline                       */
#define W4WRG_DrPoly    0x0010   /* Draw a Polygon                        */
#define W4WRG_DrwArc    0x0011   /* Draw an Arc                           */
#define W4WRG_DrwPie    0x0012   /* Draw a Pie                            */
#define W4WRG_DrCirc    0x0013   /* Draw a Circle or an Ellipse           */
#define W4WRG_DrBMap    0x0014   /* Draw a Bitmap                         */
#define W4WRG_Scalng    0x0015   /* Set Scaling Mode                      */
#define W4WRG_Rotate    0x0016   /* Set Rotation Value                    */
#define W4WRG_DefFnt    0x0017   /* Define Font                           */
#define W4WRG_DrText    0x0018   /* Draw Text                             */
#define W4WRG_BckCol    0x0019   /* Set Backgroundcolor                   */
#define W4WRG_StGrup    0x001A   /* Start grouped Objects                 */
#define W4WRG_EoGrup    0x001B   /* End grouped Objects                   */
#define W4WRG_DrChrd    0x001C   /* Draw a Chord                          */
#define W4WRG_DefP16    0x001D   /* Define 16 Bit Palette                 */
#define W4WRG_DefGCv    0x001E   /* Define Gray Curve                     */
#define W4WRG_DefFHd    0x001F   /* Define File Header                    */

#define W4WRG_TextBox   0x0020   /* Text box                              */
#define W4WRG_StClPath  0x0021   /* Start enclosed path                   */
#define W4WRG_EoClPath  0x0022   /* End enclosed path                     */
#define W4WRG_Bezier    0x0023   /* Draw a bezier curve                   */
#define W4WRG_Dr24bitBMap 0x0024 /* Draw a 24bit Bitmap                   */
#define W4WRG_Comment   0x0025   /* intermal MSG comment                  */
#define W4WRG_Spline    0x0026   /* to be defined                         */
#define W4WRG_Transform 0x0027   /* to be defined                         */
#define W4WRG_LineHead  0x0028   /* to be defined                         */
#define W4WRG_LineTail  0x0029   /* to be defined                         */
#define W4WRG_CanvasCol 0x002A   /* canvas color of image                 */
#define W4WRG_BmpPos    0x002B   /* x,y and scaling of bitmap             */

#define W4WRG_EoFile    0xFFFF   /* End of File                           */



int SwW4WGraf::GetNextRecord()
{
    USHORT nRecId;
    long   nVarSize;
    short  nRet=0;

    nRecId=GetHexUShort();
    nVarSize=GetVarSize();

    switch( nRecId )
    {
        case W4WRG_DefPal:      nRet = ReadPalette( nVarSize );  break;
        case W4WRG_DrBMap:      nRet = ReadBitmap(  nVarSize );  break;
        case W4WRG_Dr24bitBMap: nRet = ReadBitmap(  nVarSize );  break;
        default: SkipBytes( nVarSize );
    }
    if( nRet < 0 ) nStat = nRet;
    return (nRet) ? nRet : nRecId;
}


short SwW4WGraf::ReadW4WGrafBMap( long, long, long )    // Mastersoft internal Format Bitmap
{
    int nRet;
    do {
        nRet=GetNextRecord();
    } while (nError==0 && nRet!=(int)W4WRG_EoFile && nRet!=-1 && nRet!=-1); //$ EOF
    return nStat;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// W4W Vectorimport von Joe  (BEGIN) ///////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef JOEDEBUG


void ShowW4WGrafRec(USHORT nRecID)
{
    static USHORT nLnCnt=0;
    switch (nRecID) {
        case W4WRG_Ignore: ShowMsg("Dummy     "); break;
        case W4WRG_RecdID: ShowMsg("OrgFileTp "); break;
        case W4WRG_DefPal: ShowMsg("Def_Pal  Û"); break;
        case W4WRG_DefPen: ShowMsg("Def_Pen   "); break;
        case W4WRG_DefBrs: ShowMsg("Def_Brs   "); break;
        case W4WRG_DefMrk: ShowMsg("Def_Mark Û"); break;
        case W4WRG_MoveTo: ShowMsg("Move_To   "); break;
        case W4WRG_LineTo: ShowMsg("Line_To   "); break;
        case W4WRG_FloodF: ShowMsg("FloodfillÛ"); break;
        case W4WRG_SetFil: ShowMsg("Set_Fill Û"); break;
        case W4WRG_DrMark: ShowMsg("Drw_Mark Û"); break;
        case W4WRG_DrMrkP: ShowMsg("Drw_MarkAÛ"); break;
        case W4WRG_DrRect: ShowMsg("Drw_Rect  "); break;
        case W4WRG_DrPLin: ShowMsg("Drw_PLine "); break;
        case W4WRG_DrPoly: ShowMsg("Drw_Poly  "); break;
        case W4WRG_DrwArc: ShowMsg("Drw_Arc  Û"); break;
        case W4WRG_DrwPie: ShowMsg("Drw_Pie  Û"); break;
        case W4WRG_DrCirc: ShowMsg("Drw_Circ Û"); break;
        case W4WRG_DrBMap: ShowMsg("Drw_BMap Û"); break;
        case W4WRG_Scalng: ShowMsg("Set_ScaleÛ"); break;
        case W4WRG_Rotate: ShowMsg("Set_Rota Û"); break;
        case W4WRG_DefFnt: ShowMsg("Def_Font Û"); break;
        case W4WRG_DrText: ShowMsg("Drw_Text Û"); break;
        case W4WRG_BckCol: ShowMsg("Set_BCol Û"); break;
        case W4WRG_StGrup: ShowMsg("Start_Grp "); break;
        case W4WRG_EoGrup: ShowMsg("End_Grp   "); break;
        case W4WRG_DrChrd: ShowMsg("Drw_ChordÛ"); break;
        case W4WRG_DefP16: ShowMsg("Def_Pal16Û"); break;
        case W4WRG_DefGCv: ShowMsg("Def_PalGrÛ"); break;
        case W4WRG_DefFHd: ShowMsg("Def_FHeadÛ"); break;
        case W4WRG_EoFile: ShowMsg("EOFÞÜÝÞßÝ \n"); break;
    }
    nLnCnt++;
    if (nLnCnt>=23*8) {
        nLnCnt=0;
        WaitKey();
    }
}

#endif


BOOL SwW4WGraf::CheckW4WVector() // enth„lt die W4W-Grafik Vektordaten ? (Joe)
{
    long   nFPosMerk = rInp.Tell();
    long   nVarSize;
    USHORT nRecID;
    BOOL   bRet = FALSE;

    do {
        nRecID   = GetHexUShort();
        nVarSize = GetVarSize();

        // Bitmap ?
        if(    ( nRecID == W4WRG_DefPal )
            || ( nRecID == W4WRG_DrBMap )
            || ( nRecID == W4WRG_Dr24bitBMap ) ) break;

        // Vektor ?
        if(    ( nRecID == W4WRG_LineTo )
                || ( nRecID == W4WRG_FloodF )
                || ( nRecID == W4WRG_DrMark )
                || ( nRecID == W4WRG_DrMrkP )
                || ( nRecID == W4WRG_DrRect )
                || ( nRecID == W4WRG_DrPLin )
                || ( nRecID == W4WRG_DrPoly )
                || ( nRecID == W4WRG_DrwArc )
                || ( nRecID == W4WRG_DrwPie )
                || ( nRecID == W4WRG_DrCirc )
                || ( nRecID == W4WRG_DrText )
                || ( nRecID == W4WRG_DrChrd ) )
        {
            bRet = TRUE;  break;
        }
        SkipBytes( nVarSize );
    }
    while( ( !nError ) && ( W4WRG_EoFile != nRecID ) );

    rInp.Seek( nFPosMerk );           // FilePos restaurieren
    return bRet;
}


void SwW4WGraf::ReadHexPoint(Point& aPnt)
{
    USHORT xu=GetHexUShort();
    USHORT yu=GetHexUShort();
    if( xu < nVectMinX )
        nVectMinX = xu;
    if( xu > nVectMaxX )
        nVectMaxX = xu;
    if( yu < nVectMinY )
        nVectMinY = yu;
    if( yu > nVectMaxY )
        nVectMaxY = yu;
    long  x=xu;
    long  y=yu;
    Point a(x,y);
    aPnt=a;
}


void DrehePoint(Point& P, long cx, long cy, double sn, double cs)
{
    long  dx,dy;
    double x1,y1;
    dx=P.X()-cx;
    dy=P.Y()-cy;
    x1=dx*cs-dy*sn;
    y1=dy*cs+dx*sn;
    P=Point(cx+long(x1),cy+long(y1));
}


int SwW4WGraf::GetNextVectRec(OutputDevice& rOut)
{
    USHORT nRecID;
    long   nVarSize;

    nRecID   = GetHexUShort();
    nVarSize = GetVarSize();

#ifdef JOEDEBUG
//    ShowW4WGrafRec(nRecID);
#endif

    switch (nRecID) {
//        case W4WRG_DefPal: nRet=ReadPalette(nVarSize); break;
//        case W4WRG_DrBMap: nRet=ReadBitmap(nVarSize);  break;
        case W4WRG_RecdID: {
            /* INT16   nId= */ GetHexUShort();
            /* INT16   nRes= */GetHexUShort();
            /* INT16   nX0= */ GetHexUShort();
            /* INT16   nY0= */ GetHexUShort();
            /* INT16   nX1= */ GetHexUShort();
            /* INT16   nY1= */ GetHexUShort();
        } break;
        case W4WRG_Scalng: {
            SkipBytes(nVarSize);		// Dummy fuer Breakpoint
        } break;
        case W4WRG_MoveTo: {
            ReadHexPoint(aVectPos);
        } break;
        case W4WRG_LineTo: {
            Point   aP;
            ReadHexPoint(aP);
            rOut.DrawLine(aVectPos,aP);
            aVectPos=aP;
        } break;
        case W4WRG_DrPLin: case W4WRG_DrPoly: {
            INT16   nPts=GetHexUShort();
            INT16   i;
            Polygon aPoly(nPts);
            Point P1;
            for (i=0;i<nPts;i++) {
                ReadHexPoint(P1);
                aPoly.SetPoint(P1,i);
            }
            if (nRecID==W4WRG_DrPLin) rOut.DrawPolyLine(aPoly);
            else rOut.DrawPolygon(aPoly);
        } break;
        case W4WRG_DrRect: {
            Point P1,P2;
            ReadHexPoint(P1);
            ReadHexPoint(P2);
            rOut.DrawRect(Rectangle(P1,P2));
        } break;
        case W4WRG_DrCirc: {
            short xc=(short)GetHexUShort();
            short yc=(short)GetHexUShort();
            short xr=(short)GetHexUShort();
            short yr=(short)GetHexUShort();
            rOut.DrawEllipse(Rectangle(Point(xc-xr,yc-yr),Point(xc+xr,yc+yr)));
        } break;
        case W4WRG_DrwPie: case W4WRG_DrwArc: {
            long xc=(short)GetHexUShort(),
                 yc=(short)GetHexUShort(),
                 xr=(short)GetHexUShort(),
                 yr=(short)GetHexUShort(),
                 nAnf=(short)GetHexUShort(),
                 nEnd=(short)GetHexUShort();
            double sn,cs;
            Point aAnf(xc+xr,yc);
            Point aEnd=aAnf;
            sn=sin(double(nAnf)*3.14159265359/18000);
            cs=cos(double(nAnf)*3.14159265359/18000);
            DrehePoint(aAnf,xc,yc,sn,cs);
            sn=sin(double(nEnd)*3.14159265359/18000);
            cs=cos(double(nEnd)*3.14159265359/18000);
            DrehePoint(aEnd,xc,yc,sn,cs);
            if (nRecID==W4WRG_DrwPie)
                 rOut.DrawPie(Rectangle(Point(xc-xr,yc-yr),Point(xc+xr,yc+yr)),aAnf,aEnd);
            else rOut.DrawArc(Rectangle(Point(xc-xr,yc-yr),Point(xc+xr,yc+yr)),aAnf,aEnd);
        } break;
        case W4WRG_DefBrs: {
            USHORT nPat=GetHexUShort();
            USHORT nBlu=GetHexByte();
            USHORT nGrn=GetHexByte();
            USHORT nRed=GetHexByte();
            Color aCol(nRed*0x0100,nGrn*0x0100,nBlu*0x0100);
//			Brush aBrs(aCol);
            switch (nPat) {
                //case 0: Bitmap
//				case 1: aBrs.SetStyle(BRUSH_SOLID);     break;
//				case 2: aBrs.SetStyle(BRUSH_VERT);      break;
//				case 3: aBrs.SetStyle(BRUSH_UPDIAG);    break;  // +45deg
//				case 4: aBrs.SetStyle(BRUSH_HORZ);      break;
//				case 5: aBrs.SetStyle(BRUSH_DOWNDIAG);  break;  // -45deg
//				case 6: aBrs.SetStyle(BRUSH_50);        break;  // Dots
//				case 7: aBrs.SetStyle(BRUSH_CROSS);     break;
//				case 8: aBrs.SetStyle(BRUSH_DIAGCROSS); break;
                case 9: aCol.SetTransparency(0xff);      break;  // BRUSH_NULL Hollow
            }
            rOut.SetFillColor(aCol);
        } break;
        case W4WRG_DefPen: {
            USHORT nPat=GetHexUShort();
            USHORT nBlu=GetHexByte();
            USHORT nGrn=GetHexByte();
            USHORT nRed=GetHexByte();
            /* USHORT nWdt= */ GetHexUShort();
            /* USHORT nHgt= */ GetHexUShort();
            Color aCol(nRed*0x0100,nGrn*0x0100,nBlu*0x0100);
//			Pen   aPen(aCol);
            switch (nPat) {
                case 0: aCol.SetTransparency(0xff);/*aPen.SetStyle(PEN_NULL);*/    break;
//				case 1: aPen.SetStyle(PEN_SOLID);   break;
//				case 2: aPen.SetStyle(PEN_DASH);    break;
//				case 3: aPen.SetStyle(PEN_DOT);     break;
//				case 4: aPen.SetStyle(PEN_DASHDOT); break;
//				case 5: aPen.SetStyle(PEN_DASHDOT); break;
            }
            rOut.SetLineColor(aCol);
        } break;
        case W4WRG_BckCol: {
            USHORT nBlu=GetHexByte();
            USHORT nGrn=GetHexByte();
            USHORT nRed=GetHexByte();
            Color aCol(nRed*0x0100,nGrn*0x0100,nBlu*0x0100);
            rOut.SetBackground(Wallpaper(aCol));
        } break;
        case W4WRG_DefFnt:
        {
            USHORT i;
            USHORT Grad=GetHexByte();
            USHORT nLen=USHORT(nVarSize)-2; // L„nge des Fontnamen inkl. #0
            sal_Char* pC = new sal_Char[ nLen ];
            for (i=0;i<nLen;i++)
                pC[i]=(char)GetHexByte();
            String aStr( pC, RTL_TEXTENCODING_IBM_850 );
            Color aColor=rOut.GetLineColor();
            Font aFont(aStr,Size(0,Grad));
            aFont.SetColor(aColor);
            rOut.SetFont(aFont);
            delete[] pC;
        } break;
        case W4WRG_DrText: {
            USHORT i;
            Point P1;
            ReadHexPoint(P1);
            USHORT nLen=USHORT(nVarSize)-4; // Textl„nge inkl. #0
            char* pC=new char[nLen];
            for (i=0;i<nLen;i++)
                pC[i]=(char)GetHexByte();
            String aStr( pC, RTL_TEXTENCODING_IBM_850 );
            rOut.DrawText(P1,aStr);
            delete[] pC;
        } break;

        /*
            folgende Dummys sind ggfs. noch zu implementieren
        */
        case W4WRG_TextBox: {
            // Dummy-Implementation
            short nDummy;
            for( USHORT i=0; i < nVarSize; i++) nDummy = GetHexByte();
        } break;
        case W4WRG_StClPath: {
            // Dummy-Implementation
            short nDummy;
            for( USHORT i=0; i < nVarSize; i++) nDummy = GetHexByte();
        } break;
        case W4WRG_EoClPath: {
            // Dummy-Implementation
            short nDummy;
            for( USHORT i=0; i < nVarSize; i++) nDummy = GetHexByte();
        } break;
        case W4WRG_Bezier: {
            // Dummy-Implementation
            short nDummy;
            for( USHORT i=0; i < nVarSize; i++) nDummy = GetHexByte();
        } break;
        /*
        case W4WRG_Dr24bitBMap: {
            ;
        } break;
        */
        case W4WRG_Comment: {
            // Dummy-Implementation
            short nDummy;
            for( USHORT i=0; i < nVarSize; i++) nDummy = GetHexByte();
        } break;
        case W4WRG_Spline: {
            // Dummy-Implementation
            short nDummy;
            for( USHORT i=0; i < nVarSize; i++) nDummy = GetHexByte();
        } break;
        case W4WRG_Transform: {
            // Dummy-Implementation
            short nDummy;
            for( USHORT i=0; i < nVarSize; i++) nDummy = GetHexByte();
        } break;
        case W4WRG_LineHead: {
            // Dummy-Implementation
            short nDummy;
            for( USHORT i=0; i < nVarSize; i++) nDummy = GetHexByte();
        } break;
        case W4WRG_LineTail: {
            // Dummy-Implementation
            short nDummy;
            for( USHORT i=0; i < nVarSize; i++) nDummy = GetHexByte();
        } break;
        case W4WRG_CanvasCol: {
            // Dummy-Implementation
            short nDummy;
            for( USHORT i=0; i < nVarSize; i++) nDummy = GetHexByte();
        } break;
        case W4WRG_BmpPos: {
            // Dummy-Implementation
            short nDummy;
            for( USHORT i=0; i < nVarSize; i++) nDummy = GetHexByte();
        } break;

// Bitmap und Vektoren gemischt geht nicht, denn im W4W-BitmapRec steht
// nichts ber die Position der Bitmap in der Grafik drin. Die Gr”áe der
// Bitmap k”nnte man sich mittels Pixelsize und DPI ausrechnen.    Joe M.
//        case W4WRG_DefPal: {
//            ReadPalette(nVarSize);
//        } break;
//        case W4WRG_DrBMap: {
//            ReadBitmap(nVarSize);
//            if (rFileName!=aEmptyStr) {  // BMap im File
//                rFileName=aEmptystr;
//            } else {
//                if ...
//            }
//        } break;
        default: SkipBytes(nVarSize);
    }
    return nRecID;
}


short SwW4WGraf::ReadW4WGrafVect( long, long, long )    // Mastersoft internal Format Vector (Joe)
{
    int nRet;
    GDIMetaFile   aMtf;              // fr Vektorimport des internen W4W-Formats (Joe)
    VirtualDevice aOut;
    aMtf.Record(&aOut);              // aOut ans Metafile binden
    do {
        nRet=GetNextVectRec(aOut);
    } while (nError==0 && nRet!=(int)W4WRG_EoFile && nRet!=-1 && nRet!=-1); //$ EOF
    aMtf.Stop();
    aMtf.WindStart();
    aMtf.SetPrefMapMode( MAP_10TH_MM );
    long nSizX = (long)nVectMaxX - (long)nVectMinX;     // Groesse
    long nRandX = nSizX / 50 + 1;						// Zugabe
    long nSizY = (long)nVectMaxY - (long)nVectMinY;
    long nRandY = nSizY / 50 + 1;						// Zugabe
    aMtf.SetPrefSize( Size( nSizX + 2 * nRandX, nSizY + 2 * nRandY ) );

    // Das Obere zuunterst kehren
    aMtf.Scale( Fraction( 1, 1 ), Fraction( -1, 1 ) );
    aMtf.Move( (long)nVectMinX + nRandX, (long)nVectMaxY + nRandY );
    pGraph=new Graphic(aMtf);

    return nStat;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// W4W Vectorimport von Joe  (ENDE) ////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////



struct WmfFileHd
{                     // Vorsicht: Struktur ist nicht aligned
    USHORT nTyp;            // 0 = Mem, 1 = File
    USHORT nHdSiz;          // 0x009
    USHORT nVersion;        // 0x300
    ULONG  nSize;           // filesize in bytes
    USHORT nNoObj;          // Number of Objects that exist at same time
    ULONG  nMaxRecLen;      // largest record
    USHORT nNoParas;        // not used
};

//#define WMF_FILE_HD_SIZ 18    // Groesse der Struktur WmfFileHd


ULONG SwW4WGraf::ReadWmfHeader( WmfFileHd& rHd )
{
    long nPos = rInp.Tell();

    rHd.nTyp = GetHexUShort();
    rHd.nHdSiz = GetHexUShort();
    rHd.nVersion = GetHexUShort();
    rHd.nSize = GetHexULong();
    rHd.nNoObj = GetHexUShort();
    rHd.nMaxRecLen = GetHexULong();
    rHd.nNoParas = GetHexUShort();

    rInp.Seek( nPos );
    return rHd.nSize * 2;       // FileSize in Bytes
}

struct METAFILEHEADER
{
    UINT32  key;
    UINT16  hmf;
    UINT16  left;
    UINT16  top;
    UINT16  right;
    UINT16  bottom;
    UINT16  inch;
    UINT32  reserved;
    UINT16  checksum;
};

#define METAFILEHEADER_SIZE 22


void WriteWmfPreHd( long nWidth, long nHeight, SvStream& rOStream ) //$ ostream
{
    METAFILEHEADER aHeader;

    aHeader.key = 0x9AC6CDD7L;
    aHeader.hmf = 0;
    aHeader.left = 0;
    aHeader.top = 0;
    aHeader.right =  (USHORT)( nWidth * 100 / 144 );
    aHeader.bottom =  (USHORT)( nHeight * 100 / 144 );
    aHeader.inch= 1000;
    aHeader.reserved = 0;
    aHeader.checksum = 0;
    for( USHORT n=0; n < 10; n++ )
        aHeader.checksum ^= *(((UINT16*)&aHeader)+n);

#ifndef OSL_LITENDIAN
    aHeader.key = SWAPLONG( aHeader.key );
    aHeader.left = 0;
    aHeader.top = 0;
    aHeader.right = SWAPSHORT( aHeader.right );
    aHeader.bottom = SWAPSHORT( aHeader.bottom );
    aHeader.inch = SWAPSHORT( aHeader.inch );
    aHeader.checksum = SWAPSHORT( aHeader.checksum );
#endif // !OSL_LITENDIAN

    rOStream.Write( (char*)&aHeader, METAFILEHEADER_SIZE );
}


static BYTE WpgFileHd[]={ 0xFF, 0x57, 0x50, 0x43, 0x10, 0x00, 0x00, 0x00,
                          0x01, 0x16, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00 };


short SwW4WGraf::ReadGrafFile( long nTyp, long nWidth, long nHeight )   // Grafik in File schreiben
{
    SvCacheStream aOut;
    BOOL bWriteToOut = TRUE;
    switch ( nTyp )
    {
    /*
        Anmerkungen zu den Datei-Formaten, die als
        Hex-Dump im Zwischenkode stehen:

        503 = encapsulated Postscript
                mit Vorschau-Header			--> alles dumpen
              Wir haben hierfuer leider
              noch keinen Import-Filter.

        513 = Bitmap incl. Header			--> alles dumpen

        614 = in AmiPro importierte WPG-
                Files, die fuer uns leider
                unlesbar sind: unbekanntes
                Format                      --> dennoch alles dumpen

        615 = WPG ( WordPerfect )			--> alles dumpen
              Wir haben hierfuer leider
              noch keinen Import-Filter.

    */
    case 1: {                                          // WMF ( z.B. WinWord )
                WmfFileHd aHd;                          // lies Header
                ULONG nSiz = ReadWmfHeader( aHd );      // FileSize in Bytes

                WriteWmfPreHd( nWidth, nHeight, aOut ); // Placeable WMF-Header

                register ULONG i;
                for ( i = 0; i < nSiz && !nError; i++ ) // Schreibe Daten
                    aOut << (BYTE)GetHexByte();
                bWriteToOut = FALSE;
            }
            break;

    case 606:{                                          // WPG ( WordPerfect )
                // Es fehlt der File-Header, es gibt auch keine
                // Info ueber ihn. Ich nehme mal an, dass er immer
                // gleich ist wie in den Beispiel-Dateien von WpWin52

                for ( int i = 0; i < sizeof( WpgFileHd ); i++ )  // Schreibe Header
                    aOut << WpgFileHd[ i ];
            }
            break;

    case 610:{                                          // Metafile
                // diese Metafile-Dateien sind in
                // WinWord eingebunden und
                // haben einen verstuemmelten Header
                // sowie eine andere Dateilaenge als
                // die Original-Dateien
                short s = ReadChar();	// ueberlies " 0 | "
                s = ReadChar();
            }
            break;

    }

    if( bWriteToOut )
    {
        register long i = 0;
        register short s;						// evtl. mit Header
        while( !nError )
        {
            s = GetHexByte();                   // Kopiere Daten
            if ( s < 0 ) break;                 // fertig
            aOut << (BYTE)s;
            i++;
        }
    }

    GraphicFilter& rGF = *GetGrfFilter();	// lies ueber UNSERE Filter ein
    pGraph = new Graphic();

    aOut.Seek( STREAM_SEEK_TO_BEGIN );

    if( 0 != rGF.ImportGraphic( *pGraph, aEmptyStr, aOut ))
        DELETEZ( pGraph );

//	aOut.SwapOut();

    return 0;
}


short SwW4WGraf::Read( long nTyp, long nWidth, long nHeight )
{
    if( pGraph ) DELETEZ( pGraph );
    nStat = 0;
    switch( nTyp ) {
        case 0: {
            if( CheckW4WVector() )
            {
                return ReadW4WGrafVect( nTyp, nWidth, nHeight ); // Mastersoft internal Format Vector
            } else
            {
                return ReadW4WGrafBMap( nTyp, nWidth, nHeight ); // Mastersoft internal Format Bitmap
            }
        }
        default:
        {
            // erweitert um viele Formate (khz, April 1998)
            return ReadGrafFile( nTyp, nWidth, nHeight );
        }
    }
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
