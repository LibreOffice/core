/*************************************************************************
 *
 *  $RCSfile: eps.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:11 $
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

#include <math.h>
#include <vcl/sv.h>
#include <tools/stream.hxx>
#include <tools/bigint.hxx>
#include <vcl/poly.hxx>
#include <vcl/svapp.hxx>
#include <vcl/config.hxx>
#include <vcl/metaact.hxx>
#include <vcl/graph.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/region.hxx>
#include <vcl/metric.hxx>
#include <vcl/font.hxx>
#include <vcl/virdev.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/gradient.hxx>
#include <svtools/solar.hrc>
#include <svtools/fltcall.hxx>
#include "strings.hrc"
#include "dlgeps.hrc"
#include "dlgeps.hxx"

#define POSTSCRIPT_BOUNDINGSEARCH   0x1000  // we only try to get the BoundingBox
                                            // in the first 4096 bytes

#define EPS_PREVIEW_TIFF    1
#define EPS_PREVIEW_EPSI    2

#define EPS_SCALING_FAKTOR  0.1

#define PS_LINESIZE         70      // maximum number of characters a line in the output

#define PS_NONE             0       // formating mode: action which is inserted behind the output
#define PS_SPACE            1
#define PS_RET              2
#define PS_WRAP             4

// -----------------------------Feld-Typen-------------------------------

struct ChrSet
{
    struct ChrSet * pSucc;
    BYTE nSet;
    String aName;
    FontWeight eWeight;
};

struct StackMember
{
    struct      StackMember * pSucc;
    Color       aGlobalCol;
    BOOL        bLineCol;
    Color       aLineCol;
    BOOL        bFillCol;
    Color       aFillCol;
    Color       aTextCol;
    BOOL        bTextFillCol;
    Color       aTextFillCol;
    Color       aBackgroundCol;
    Font        aFont;
    MapMode     aMapMode;
    double      nXScale;
    double      nYScale;
    double      nXOrig;
    double      nYOrig;
    BOOL        bRegionChanged;
    Region      bClipRegion;
    TextAlign   eTextAlign;
    Region      aClipReg;
};

struct PSLZWCTreeNode
{

    PSLZWCTreeNode*     pBrother;       // naechster Knoten, der den selben Vater hat
    PSLZWCTreeNode*     pFirstChild;    // erster Sohn
    USHORT              nCode;          // Der Code fuer den String von Pixelwerten, der sich ergibt, wenn
    USHORT              nValue;         // Der Pixelwert
};

class PSWriter
{
private:
    BOOL                mbStatus;
    ULONG               mnLevelWarning;     // number of embedded eps files which was not exported
    PFilterCallback     mpCallback;
    void*               mpCallerData;
    ULONG               mnLastPercent;      // Mit welcher Zahl pCallback zuletzt aufgerufen wurde.
    UINT32              mnLatestPush;       // offset auf streamposition, an der zuletzt gepusht wurde

    long                mnLevel;            // dialog options
    BOOL                mbGrayScale;
    BOOL                mbCompression;
    sal_Int32           mnPreview;

    SvStream*           mpPS;
    const GDIMetaFile*  pMTF;
    GDIMetaFile*        pAMTF;              // only created if Graphics is not a Metafile
    MapMode             aMapMode;
    VirtualDevice       aOutputDevice;

    double              nBoundingX1;        // this represents the bounding box
    double              nBoundingY1;
    double              nBoundingX2;
    double              nBoundingY2;

    double              nXScaling;          // represents the factor of the current ( MapMode to 100THmm )
    double              nYScaling;
    double              nXOrigin;           // this points to the origin ( in 100THmm )
    double              nYOrigin;           // ( eg. is the BoundingBox ( 100, 200 ... )
                                            // nXOrigin and nYOrigin starts with -100 -200;
    StackMember*        pGDIStack;
    ULONG               mnCursorPos;        // aktuelle Cursorposition im Output
    Color               aColor;             // aktuelle Farbe die fuer den Output benutzt wird
    BOOL                bLineColor;
    Color               aLineColor;         // aktuelle GDIMetafile Farbeinstellungen
    BOOL                bFillColor;         //
    Color               aFillColor;         //
    Color               aTextColor;         //
    BOOL                bTextFillColor;     //
    Color               aTextFillColor;     //
    Color               aBackgroundColor;   //
    BOOL                bRegionChanged;
    Region              aClipRegion;
    TextAlign           eTextAlign;

    Font                maFont;
    Font                maLastFont;
    BYTE                nChrSet;
    ChrSet*             pChrSetList;        // Liste der Character-Sets
    BYTE                nNextChrSetId;      // die erste unbenutzte ChrSet-Id

    PSLZWCTreeNode*     pTable;             // LZW compression data
    PSLZWCTreeNode*     pPrefix;            // the compression is as same as the TIFF compression
    USHORT              nDataSize;
    USHORT              nClearCode;
    USHORT              nEOICode;
    USHORT              nTableSize;
    USHORT              nCodeSize;
    ULONG               nOffset;
    ULONG               dwShift;

    void                MayCallback( ULONG nPercent );
    void                ImplWriteProlog( const Graphic* pPreviewEPSI = NULL );
    void                ImplWriteEpilog();
    void                ImplWriteActions( const GDIMetaFile& rMtf);

                        // this method makes LF's, space inserting and word wrapping as used in all nMode
                        // parameters
    inline void         ImplExecMode( ULONG nMode );

                        // writes char[] + LF to stream
    inline void         ImplWriteLine( char[], ULONG nMode = PS_RET );

                        // writes ( nNumb / 10^nCount ) in ASCII format to stream
    void                ImplWriteF( sal_Int32 nNumb, ULONG nCount = 3, ULONG nMode = PS_SPACE );

                        // writes a double in ASCII format to stream
    void                ImplWriteDouble( double, ULONG nMode = PS_SPACE );

                        // writes a long in ASCII format to stream
    void                ImplWriteLong( sal_Int32 nNumb, ULONG nMode = PS_SPACE );

                        // writes a byte in ASCII format to stream
    void                ImplWriteByte( BYTE nNumb, ULONG nMode = PS_SPACE );

                        // writes a byte in ASCII (hex) format to stream
    void                ImplWriteHexByte( BYTE nNumb, ULONG nMode = PS_WRAP );

                        // writes nNumb as number from 0.000 till 1.000 in ASCII format to stream
    void                ImplWriteB1( BYTE nNumb, ULONG nMode = PS_SPACE );

    inline void         ImplWritePoint( Point, ULONG nMode = PS_SPACE );
    void                ImplMoveTo( Point, ULONG nMode = PS_SPACE );
    void                ImplLineTo( Point, ULONG nMode = PS_SPACE );
    void                ImplLine( const Polygon & rPolygon );
    void                ImplLine( const PolyPolygon & rPolyPolygon );
    void                ImplWriteLineInfo( const LineInfo& rLineInfo );
    void                ImplRect( const Rectangle & rRectangle );
    void                ImplRectFill ( const Rectangle & rRectangle );
    void                ImplPoly( const Polygon & rPolygon );

    void                ImplSetClipRegion();
    void                ImplBmp( Bitmap*, Bitmap*, const Point &, double nWidth, double nHeight );
    void                ImplSetAttrForText( Point & rPoint );
    void                ImplWriteCharacter( sal_Char );
    void                ImplWriteString( const ByteString&, const INT32* pDXArry = NULL, BOOL bStretch = FALSE );
    void                ImplDefineFont( char*, char* );

    void                ImplClosePathFill( ULONG nMode = PS_RET );
    void                ImplClosePathDraw( ULONG nMode = PS_RET );
    void                ImplPathDraw( ULONG nMode = PS_RET );

    inline void         ImplWriteLineColor( ULONG nMode = PS_RET );
    inline void         ImplWriteFillColor( ULONG nMode = PS_RET );
    inline void         ImplWriteTextColor( ULONG nMode = PS_RET );
    inline void         ImplWriteTextFillColor( ULONG nMode = PS_RET );
    void                ImplWriteColor( ULONG nMode );

    void                ImplGetMapMode( const MapMode& );
    BOOL                ImplGetBoundingBox( double* nNumb, BYTE* pSource, ULONG nSize );
    BYTE*               ImplSearchEntry( BYTE* pSource, BYTE* pDest, ULONG nComp, ULONG nSize );
                        // LZW methods
    void                StartCompression();
    void                Compress( BYTE nSrc );
    void                EndCompression();
    inline void         WriteBits( USHORT nCode, USHORT nCodeLen );

public:
    BOOL                WritePS( const Graphic& rGraphic, SvStream& rTargetStream,
                                    PFilterCallback, void*, Config* );
    PSWriter();
    ~PSWriter();
};

//========================== Methoden von PSWriter ==========================

void PSWriter::MayCallback( ULONG nPercent )
{

    if ( nPercent >= mnLastPercent + 3 )
    {
        mnLastPercent = nPercent;
        if( mpCallback && nPercent <= 100 )
        {
            if ( ( (*mpCallback)( mpCallerData, (USHORT)nPercent ) ) == TRUE )
                mbStatus = FALSE;
        }
    }
}

//---------------------------------------------------------------------------------

PSWriter::PSWriter()
{
    pAMTF = NULL;
}


PSWriter::~PSWriter()
{
    delete pAMTF;
}

//---------------------------------------------------------------------------------

BOOL PSWriter::WritePS( const Graphic& rGraphic, SvStream& rTargetStream,
                          PFilterCallback pcallback, void* pcallerdata, Config* pOptionsConfig )
{
    UINT32 nStreamPosition, nPSPosition;

    mbStatus = TRUE;
    mnPreview = 0;
    mnLevelWarning = 0;
    mpCallback = pcallback;
    mpCallerData = pcallerdata;
    mnLastPercent = 0;
    mnLatestPush = 0xEFFFFFFE;

    mpPS = &rTargetStream;
    mpPS->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

    // default values for the dialog options
    mnLevel = 2;
    mbGrayScale = FALSE;
    mbCompression = TRUE;

    // try to get the dialog selection
    if ( pOptionsConfig )
    {
        ByteString  aResMgrName( "eps" );
        ResMgr*     pResMgr;

        aResMgrName.Append( ByteString::CreateFromInt32( SOLARUPD ) );
        pResMgr = ResMgr::CreateResMgr( aResMgrName.GetBuffer(), Application::GetAppInternational().GetLanguage() );

        if( pResMgr )
        {
            String aPreviewStr( ResId( KEY_PREVIEW, pResMgr ) );
            String aVersionStr( ResId( KEY_VERSION, pResMgr ) );
            String aColorStr( ResId( KEY_COLOR, pResMgr ) );
            String aComprStr( ResId( KEY_COMPR, pResMgr ) );

            mnPreview = pOptionsConfig->ReadKey( ByteString( aPreviewStr, RTL_TEXTENCODING_UTF8 ), "1" ).ToInt32();
            mnLevel = pOptionsConfig->ReadKey( ByteString( aVersionStr, RTL_TEXTENCODING_UTF8 ), "2" ).ToInt32();
            if ( mnLevel != 1 )
                mnLevel = 2;
            mbGrayScale = pOptionsConfig->ReadKey( ByteString( aColorStr, RTL_TEXTENCODING_UTF8 ), "1" ).ToInt32() == 2;
            mbCompression = pOptionsConfig->ReadKey( ByteString( aComprStr,  RTL_TEXTENCODING_UTF8 ), "1" ).ToInt32() == 1;
            delete pResMgr;
        }
    }

    // compression is not available for Level 1
    if ( mnLevel == 1 )
    {
        mbGrayScale = TRUE;
        mbCompression = FALSE;
    }

    if ( mnPreview & EPS_PREVIEW_TIFF )
    {
        rTargetStream << (UINT32)0xC6D3D0C5;
        nStreamPosition = rTargetStream.Tell();
        rTargetStream << (UINT32)0 << (UINT32)0 << (UINT32)0 << (UINT32)0
            << nStreamPosition + 26 << (UINT32)0 << (UINT16)0xffff;

        UINT32 nErrCode;
        if ( mbGrayScale )
        {
            BitmapEx aTempBitmapEx( rGraphic.GetBitmapEx() );
            aTempBitmapEx.Convert( BMP_CONVERSION_8BIT_GREYS );
            nErrCode = GraphicConverter::Export( rTargetStream, aTempBitmapEx, CVT_TIF ) ;
        }
        else
            nErrCode = GraphicConverter::Export( rTargetStream, rGraphic, CVT_TIF ) ;

        if ( nErrCode == ERRCODE_NONE )
        {
            rTargetStream.Seek( STREAM_SEEK_TO_END );
            nPSPosition = rTargetStream.Tell();
            rTargetStream.Seek( nStreamPosition + 20 );
            rTargetStream << nPSPosition - 30;  // size of tiff gfx
            rTargetStream.Seek( nPSPosition );
        }
        else
        {
            mnPreview &=~ EPS_PREVIEW_TIFF;
            rTargetStream.Seek( nStreamPosition - 4 );
        }
    }

    // global default value setting
    ChrSet*         pCS;
    StackMember*    pGS;

    if ( rGraphic.GetType() == GRAPHIC_GDIMETAFILE )
        pMTF = &rGraphic.GetGDIMetaFile();
    else
        pMTF = pAMTF = new GDIMetaFile( rGraphic.GetGDIMetaFile() );

    ImplGetMapMode( pMTF->GetPrefMapMode() );

    nBoundingX1 = nBoundingY1 = 0;
    nBoundingX2 = pMTF->GetPrefSize().Width() * nXScaling;
    nBoundingY2 = pMTF->GetPrefSize().Height() * nYScaling;

    pGDIStack = NULL;
    aColor = Color( COL_TRANSPARENT );
    bLineColor = TRUE;
    aLineColor = Color( COL_BLACK );
    bFillColor = TRUE;
    aFillColor = Color( COL_WHITE );
    aBackgroundColor = Color( COL_WHITE );
    bRegionChanged = FALSE;
    aClipRegion.SetEmpty();

    nChrSet = 0x00;
    pChrSetList = NULL;
    nNextChrSetId = 1;

    if( pMTF->GetActionCount() )
    {
        ImplWriteProlog( ( mnPreview & EPS_PREVIEW_EPSI ) ? &rGraphic : NULL );
        mnCursorPos = 0;
        ImplWriteActions( *pMTF );
        ImplWriteEpilog();
        if ( mnPreview & EPS_PREVIEW_TIFF )
        {
            UINT32 nPosition = rTargetStream.Tell();
            rTargetStream.Seek( nStreamPosition );
            rTargetStream << nPSPosition;
            rTargetStream << nPosition - nPSPosition;
            rTargetStream.Seek( nPosition );
        }
        while( pChrSetList )
        {
            pCS=pChrSetList;
            pChrSetList=pCS->pSucc;
            delete pCS;
        }
        while( pGDIStack )
        {
            pGS=pGDIStack;
            pGDIStack=pGS->pSucc;
            delete pGS;
        }
    }
    else
        mbStatus = FALSE;

    if ( mbStatus && mnLevelWarning && pOptionsConfig )
    {
        ByteString  aResMgrName( "eps" );
        ResMgr* pResMgr;
        aResMgrName.Append( ByteString::CreateFromInt32( SOLARUPD ) );
        pResMgr = ResMgr::CreateResMgr( aResMgrName.GetBuffer(), Application::GetAppInternational().GetLanguage() );
        if( pResMgr )
        {
            InfoBox aInfoBox( NULL, String( ResId( KEY_VERSION_CHECK, pResMgr ) ) );
            aInfoBox.Execute();
            delete pResMgr;
        }
    }
    return mbStatus;
}

//---------------------------------------------------------------------------------

void PSWriter::ImplWriteProlog( const Graphic* pPreview )
{
    ImplWriteLine( "%!PS-Adobe-3.0 EPSF-3.0 " );
    *mpPS << "%%BoundingBox: ";                         // BoundingBox
    ImplWriteLong( 0 );
    ImplWriteLong( 0 );
    aMapMode = MapMode( pMTF->GetPrefMapMode() );
    Size aSizePoint = Application::GetDefaultDevice()->LogicToLogic( pMTF->GetPrefSize(), aMapMode, MAP_POINT );
    ImplWriteLong( aSizePoint.Width() );
    ImplWriteLong( aSizePoint.Height() ,PS_RET );
    ImplWriteLine( "%%Pages: 0" );
    ImplWriteLine( "%%Creator: Sun Microsystems, Inc." );
    ImplWriteLine( "%%Title: none" );
    ImplWriteLine( "%%CreationDate: none" );

// defaults

    *mpPS << "%%LanguageLevel: ";                       // Language level
    ImplWriteLong( mnLevel, PS_RET );
    if ( !mbGrayScale && mnLevel == 1 )
        ImplWriteLine( "%%Extensions: CMYK" );          // CMYK extension is to set in color mode in level 1
    ImplWriteLine( "%%EndComments" );
    if ( pPreview && aSizePoint.Width() && aSizePoint.Height() )
    {
        Size aSizeBitmap( ( aSizePoint.Width() + 7 ) & ~7, aSizePoint.Height() );
        Bitmap aTmpBitmap( pPreview->GetBitmap() );
        aTmpBitmap.Scale( aSizeBitmap, BMP_SCALE_INTERPOLATE );
        aTmpBitmap.Convert( BMP_CONVERSION_1BIT_THRESHOLD );
        BitmapReadAccess* pAcc = aTmpBitmap.AcquireReadAccess();
        if ( pAcc )
        {
            *mpPS << "%%BeginPreview: ";                    // BoundingBox
            ImplWriteLong( aSizeBitmap.Width() );
            ImplWriteLong( aSizeBitmap.Height() );
            *mpPS << "1 ";
            INT32 nLines = aSizeBitmap.Width() / 312;
            if ( ( nLines * 312 ) != aSizeBitmap.Width() )
                nLines++;
            nLines *= aSizeBitmap.Height();
            ImplWriteLong( nLines );
            char  nVal;
            INT32 nX, nY, nCount2, nCount = 4;
            const BitmapColor aBlack( pAcc->GetBestMatchingColor( Color( COL_BLACK ) ) );
            for ( nY = 0; nY < aSizeBitmap.Height(); nY++ )
            {
                nCount2 = 0;
                nVal = 0;
                for ( nX = 0; nX < aSizeBitmap.Width(); nX++ )
                {
                    if ( !nCount2 )
                    {
                        ImplExecMode( PS_RET );
                        *mpPS << "%";
                        nCount2 = 312;
                    }
                    nVal <<= 1;
                    if ( pAcc->GetPixel( nY, nX ) == aBlack )
                        nVal |= 1;
                    if ( ! ( --nCount ) )
                    {
                        if ( nVal > 9 )
                            nVal += 'A' - 10;
                        else
                            nVal += '0';
                        *mpPS << nVal;
                        nVal = 0;
                        nCount += 4;
                    }
                    nCount2--;
                }
            }
            aTmpBitmap.ReleaseAccess( pAcc );
            ImplExecMode( PS_RET );
            ImplWriteLine( "%%EndPreview" );
        }
    }
    ImplWriteLine( "%%BeginProlog" );
    ImplWriteLine( "%%BeginResource: SDRes" );

//  BEGIN EPSF
    ImplWriteLine( "/b4_inc_state save def\n/dict_count countdictstack def\n/op_count count 1 sub def\nuserdict begin" );
    ImplWriteLine( "0 setgray 0 setlinecap 1 setlinewidth 0 setlinejoin 10 setmiterlimit[] 0 setdash newpath" );
    ImplWriteLine( "/languagelevel where {pop languagelevel 1 ne {false setstrokeadjust false setoverprint} if} if" );

    ImplWriteLine( "/bdef {bind def} bind def" );       // der neue operator bdef wird erzeugt
    if ( mbGrayScale )
        ImplWriteLine( "/c {setgray} bdef" );
    else
        ImplWriteLine( "/c {setrgbcolor} bdef" );
    ImplWriteLine( "/l {neg lineto} bdef" );
    ImplWriteLine( "/rl {neg rlineto} bdef" );
    ImplWriteLine( "/cl {currentlinewidth currentdash currentlinecap 2 setlinecap} bdef" );
    ImplWriteLine( "/lc {setlinecap} bdef" );
    ImplWriteLine( "/lw {setlinewidth} bdef" );
    ImplWriteLine( "/ld {setdash} bdef" );
    ImplWriteLine( "/m {neg moveto} bdef" );
    ImplWriteLine( "/r {rotate} bdef" );
    ImplWriteLine( "/t {translate} bdef" );
    ImplWriteLine( "/gs {gsave} bdef" );
    ImplWriteLine( "/gr {grestore} bdef" );

    ImplWriteLine( "/f {findfont dup length dict begin" );  // Setfont
    ImplWriteLine( "{1 index /FID ne {def} {pop pop} ifelse} forall /Encoding ISOLatin1Encoding def" );
    ImplWriteLine( "currentdict end /NFont exch definefont pop /NFont findfont} bdef" );

    ImplWriteLine( "/s {show} bdef" );
    ImplWriteLine( "/p {closepath} bdef" );
    ImplWriteLine( "/sf {scalefont setfont} bdef" );

    ImplWriteLine( "/pf {closepath fill}bdef" );        // close path and fill
    ImplWriteLine( "/pc {closepath stroke}bdef" );      // close path and draw
    ImplWriteLine( "/ps {stroke}bdef" );                // draw current path
    ImplWriteLine( "/pum {matrix currentmatrix}bdef" ); // pushes the current matrix
    ImplWriteLine( "/pom {setmatrix}bdef" );            // pops the matrix
    ImplWriteLine( "/bs {/aString exch def /nXOfs exch def /nWidth exch def currentpoint nXOfs 0 rmoveto pum nWidth aString stringwidth pop div 1 scale aString show pom moveto} bdef" );
    ImplWriteLine( "%%EndResource" );
    ImplWriteLine( "%%EndProlog" );
    ImplWriteLine( "%%BeginSetup" );
    ImplWriteLine( "%%EndSetup" );
    ImplWriteLine( "%%Page: 1 1" );
    ImplWriteLine( "%%BeginPageSetup" );
    ImplWriteLine( "%%EndPageSetup" );

    ImplWriteLine( "pum" );
    ImplWriteDouble( (double)aSizePoint.Width() / (double)nBoundingX2 );
    ImplWriteDouble( (double)aSizePoint.Height() / (double)nBoundingY2 );
    ImplWriteLine( "scale" );
    ImplWriteLong( 0 );
    ImplWriteDouble( nBoundingY2 );
    ImplWriteLine( "t" );
}

//---------------------------------------------------------------------------------

void PSWriter::ImplWriteEpilog()
{
    ImplWriteLong( 0 );
    ImplWriteDouble( - nBoundingY2 );
    ImplWriteLine( "t" );
    ImplWriteLine( "pom" );
    ImplWriteLine( "%%PageTrailer" );
    ImplWriteLine( "%%Trailer" );
    ImplWriteLine( "count op_count sub {pop} repeat countdictstack dict_count sub {end} repeat b4_inc_state restore" );
    ImplWriteLine( "%%EOF" );
}

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------

void PSWriter::ImplWriteActions( const GDIMetaFile& rMtf )
{
    for( ULONG nCurAction = 0, nCount = rMtf.GetActionCount(); nCurAction < nCount; nCurAction++ )
    {
        MetaAction* pMA = rMtf.GetAction( nCurAction );

        switch( pMA->GetType() )
        {
            case META_NULL_ACTION :
            break;

            case META_PIXEL_ACTION :
            {
                Color aOldLineColor( aLineColor );
                aLineColor = ( (const MetaPixelAction*) pMA )->GetColor();
                ImplWriteLineColor( PS_SPACE );
                ImplMoveTo( ( (const MetaPixelAction*)pMA )->GetPoint() );
                ImplLineTo( ( (const MetaPixelAction*)pMA )->GetPoint() );
                ImplPathDraw();
                aLineColor = aOldLineColor;
            }
            break;

            case META_POINT_ACTION :
            {
                ImplWriteLineColor( PS_SPACE );
                ImplMoveTo( ( (const MetaPointAction*)pMA )->GetPoint() );
                ImplLineTo( ( (const MetaPointAction*)pMA )->GetPoint() );
                ImplPathDraw();
            }
            break;

            case META_LINE_ACTION :
            {
                const LineInfo& rLineInfo = ( ( const MetaLineAction*)pMA )->GetLineInfo();
                if ( !rLineInfo.IsDefault() )
                    ImplWriteLineInfo( rLineInfo );

                if ( bLineColor )
                {
                    ImplWriteLineColor( PS_SPACE );
                    ImplMoveTo( ( (const MetaLineAction*) pMA )->GetStartPoint() );
                    ImplLineTo( ( (const MetaLineAction*) pMA )->GetEndPoint() );
                    ImplPathDraw();
                }
                if ( !rLineInfo.IsDefault() )
                    ImplWriteLine( "lc ld lw" ); // LineWidth, LineDash, LineCap zuruecksetzen
            }
            break;

            case META_RECT_ACTION :
            {
                ImplRect( ( (const MetaRectAction*) pMA )->GetRect() );
            }
            break;

            case META_ROUNDRECT_ACTION :
                ImplRect( ( (const MetaRoundRectAction*) pMA )->GetRect() );
            break;

            case META_ELLIPSE_ACTION :
            {
                Rectangle aRect = ( ( (const MetaEllipseAction*) pMA )->GetRect() );
                Point     aCenter = aRect.Center();
                Polygon   aPoly( aCenter, aRect.GetWidth() / 2, aRect.GetHeight() / 2 );
                ImplPoly( aPoly );
            }
            break;

            case META_ARC_ACTION :
            {
                Polygon aPoly( ( (const MetaArcAction*)pMA )->GetRect(), ( (const MetaArcAction*)pMA )->GetStartPoint(),
                    ( (const MetaArcAction*)pMA )->GetEndPoint(), POLY_ARC );
                ImplPoly( aPoly );
            }
            break;

            case META_PIE_ACTION :
            {
                Polygon aPoly( ( (const MetaPieAction*)pMA )->GetRect(), ( (const MetaPieAction*)pMA )->GetStartPoint(),
                    ( (const MetaPieAction*)pMA )->GetEndPoint(), POLY_PIE );
                ImplPoly( aPoly );
            }
            break;

            case META_CHORD_ACTION :
            {
                Polygon aPoly( ( (const MetaChordAction*)pMA )->GetRect(), ( (const MetaChordAction*)pMA )->GetStartPoint(),
                    ( (const MetaChordAction*)pMA )->GetEndPoint(), POLY_CHORD );
                ImplPoly( aPoly );
            }
            break;

            case META_POLYLINE_ACTION :
            {
                if ( bLineColor )
                {
                    const LineInfo& rLineInfo = ( ( const MetaPolyLineAction*)pMA )->GetLineInfo();
                    ImplWriteLineColor( PS_SPACE );
                    if ( !rLineInfo.IsDefault() )
                        ImplWriteLineInfo( rLineInfo );
                    ImplLine( ( (const MetaPolyLineAction*) pMA )->GetPolygon() );
                    ImplPathDraw();
                    if ( !rLineInfo.IsDefault() )
                        ImplWriteLine( "lc ld lw" ); // LineWidth, LineDash, LineCap zuruecksetzen
                }
            }
            break;

            case META_POLYGON_ACTION :
            {
                ImplPoly( ( (const MetaPolygonAction*) pMA )->GetPolygon() );
            }
            break;

            case META_POLYPOLYGON_ACTION :
            {
                ImplLine( ( (const MetaPolyPolygonAction*) pMA )->GetPolyPolygon() );
            }
            break;

            case META_TEXT_ACTION:
            {
                const MetaTextAction * pA = (const MetaTextAction*) pMA;

                String  aUniStr( pA->GetText(), pA->GetIndex(), pA->GetLen() );
                Point   aPoint( pA->GetPoint() );

                ImplSetAttrForText( aPoint );
                ByteString aStr( aUniStr, maFont.GetCharSet() );
                ImplWriteString( aStr );
                if ( maFont.GetOrientation() )
                    ImplWriteLine( "gr" );
            }
            break;

            case META_TEXTRECT_ACTION:
            {
                DBG_ERROR( "Unsupported action: TextRect...Action!" );
            }
            break;

            case META_STRETCHTEXT_ACTION :
            {
                const MetaStretchTextAction* pA = (const MetaStretchTextAction*)pMA;
                String  aUniStr( pA->GetText(), pA->GetIndex(), pA->GetLen() );
                Point   aPoint( pA->GetPoint() );

                ImplSetAttrForText( aPoint );

                ByteString aStr( aUniStr, maFont.GetCharSet() );
                ImplWriteString( aStr, NULL, TRUE );

                if ( maFont.GetOrientation() )
                    ImplWriteLine( "gr" );
            }
            break;

            case META_TEXTARRAY_ACTION:
            {
                const MetaTextArrayAction* pA = (const MetaTextArrayAction*)pMA;
                String  aUniStr( pA->GetText(), pA->GetIndex(), pA->GetLen() );
                Point   aPoint( pA->GetPoint() );

                ImplSetAttrForText( aPoint );

                ByteString aStr( aUniStr, maFont.GetCharSet() );
                ImplWriteString( aStr, pA->GetDXArray(), FALSE );

                if ( maFont.GetOrientation() )
                    ImplWriteLine( "gr" );
            }
            break;

            case META_BMP_ACTION :
            {
                Bitmap aBitmap = ( (const MetaBmpAction*)pMA )->GetBitmap();
                if ( mbGrayScale )
                    aBitmap.Convert( BMP_CONVERSION_8BIT_GREYS );
                Point aPoint = ( (const MetaBmpAction*) pMA )->GetPoint();
                ImplBmp( &aBitmap, NULL, aPoint, nBoundingX2, nBoundingY2 );
            }
            break;

            case META_BMPSCALE_ACTION :
            {
                Bitmap aBitmap = ( (const MetaBmpScaleAction*)pMA )->GetBitmap();
                if ( mbGrayScale )
                    aBitmap.Convert( BMP_CONVERSION_8BIT_GREYS );
                Point aPoint = ( (const MetaBmpScaleAction*) pMA )->GetPoint();
                Size aSize = ( (const MetaBmpScaleAction*)pMA )->GetSize();
                ImplBmp( &aBitmap, NULL, aPoint, aSize.Width(), aSize.Height() );
            }
            break;

            case META_BMPSCALEPART_ACTION :
            {
                Bitmap  aBitmap( ( (const MetaBmpScalePartAction*)pMA )->GetBitmap() );
                aBitmap.Crop( Rectangle( ( (const MetaBmpScalePartAction*)pMA )->GetSrcPoint(),
                    ( (const MetaBmpScalePartAction*)pMA )->GetSrcSize() ) );
                if ( mbGrayScale )
                    aBitmap.Convert( BMP_CONVERSION_8BIT_GREYS );
                Point aPoint = ( (const MetaBmpScalePartAction*) pMA)->GetDestPoint();
                Size aSize = ( (const MetaBmpScalePartAction*)pMA )->GetDestSize();
                ImplBmp( &aBitmap, NULL, aPoint, aSize.Width(), aSize.Height() );
            }
            break;

            case META_BMPEX_ACTION :
            {
                BitmapEx aBitmapEx( ( (MetaBmpExAction*)pMA)->GetBitmapEx() );
                Bitmap aBitmap( aBitmapEx.GetBitmap() );
                if ( mbGrayScale )
                    aBitmap.Convert( BMP_CONVERSION_8BIT_GREYS );
                Bitmap aMask( aBitmapEx.GetMask() );
                Point aPoint = ( (const MetaBmpExAction*) pMA)->GetPoint();
                Size aSize = ( aBitmap.GetSizePixel() );
                ImplBmp( &aBitmap, &aMask, aPoint, aSize.Width(), aSize.Height() );
            }
            break;

            case META_BMPEXSCALE_ACTION :
            {
                BitmapEx aBitmapEx( ( (MetaBmpExScaleAction*)pMA)->GetBitmapEx() );
                Bitmap aBitmap( aBitmapEx.GetBitmap() );
                if ( mbGrayScale )
                    aBitmap.Convert( BMP_CONVERSION_8BIT_GREYS );
                Bitmap aMask( aBitmapEx.GetMask() );
                Point aPoint = ( (const MetaBmpExScaleAction*) pMA)->GetPoint();
                Size aSize( ( (const MetaBmpExScaleAction*)pMA )->GetSize() );
                ImplBmp( &aBitmap, &aMask, aPoint, aSize.Width(), aSize.Height() );
            }
            break;

            case META_BMPEXSCALEPART_ACTION :
            {
                BitmapEx    aBitmapEx( ( (const MetaBmpExScalePartAction*)pMA )->GetBitmapEx() );
                aBitmapEx.Crop( Rectangle( ( (const MetaBmpExScalePartAction*)pMA )->GetSrcPoint(),
                    ( (const MetaBmpExScalePartAction*)pMA )->GetSrcSize() ) );
                Bitmap      aBitmap( aBitmapEx.GetBitmap() );
                if ( mbGrayScale )
                    aBitmap.Convert( BMP_CONVERSION_8BIT_GREYS );
                Bitmap      aMask( aBitmapEx.GetMask() );
                Point aPoint = ( (const MetaBmpExScalePartAction*) pMA)->GetDestPoint();
                Size aSize = ( (const MetaBmpExScalePartAction*)pMA )->GetDestSize();
                ImplBmp( &aBitmap, &aMask, aPoint, aSize.Width(), aSize.Height() );
            }
            break;

            // Unsupported Actions
            case META_MASK_ACTION:
            case META_MASKSCALE_ACTION:
            case META_MASKSCALEPART_ACTION:
            {
                DBG_ERROR( "Unsupported action: MetaMask...Action!" );
            }
            break;

            case META_GRADIENT_ACTION :
            {
                VirtualDevice   aVDev;
                GDIMetaFile     aTmpMtf;

                aVDev.SetMapMode( aMapMode );
                aVDev.AddGradientActions( ( (const MetaGradientAction*)pMA)->GetRect(), ( (const MetaGradientAction*) pMA )->GetGradient(), aTmpMtf );
                ImplWriteActions( aTmpMtf );
            }
            break;

            case META_HATCH_ACTION :
            {
                VirtualDevice   aVDev;
                GDIMetaFile     aTmpMtf;

                aVDev.SetMapMode( aMapMode );
                aVDev.AddHatchActions( ( (const MetaHatchAction*)pMA)->GetPolyPolygon(),
                                       ( (const MetaHatchAction*)pMA )->GetHatch(), aTmpMtf );
                ImplWriteActions( aTmpMtf );
            }
            break;

            case META_WALLPAPER_ACTION :
            {
                const MetaWallpaperAction* pA = (const MetaWallpaperAction*)pMA;
                Rectangle   aRect = pA->GetRect();
                Wallpaper   aWallpaper = pA->GetWallpaper();

                if ( aWallpaper.IsBitmap() )
                {
                    BitmapEx aBitmapEx = aWallpaper.GetBitmap();
                    Bitmap aBitmap( aBitmapEx.GetBitmap() );
                    if ( aBitmapEx.IsTransparent() )
                    {
                        if ( aWallpaper.IsGradient() )
                        {

                        // gradient action

                        }
                        Bitmap aMask( aBitmapEx.GetMask() );
                        ImplBmp( &aBitmap, &aMask, Point( aRect.Left(), aRect.Top() ), aRect.GetWidth(), aRect.GetHeight() );
                    }
                    else
                        ImplBmp( &aBitmap, NULL, Point( aRect.Left(), aRect.Top() ), aRect.GetWidth(), aRect.GetHeight() );

                        // wallpaper Style

                }
                else if ( aWallpaper.IsGradient() )
                {

                // gradient action

                }
                else
                {
                    aColor = aWallpaper.GetColor();
                    ImplRectFill( aRect );
                }
            }
            break;

            case META_ISECTRECTCLIPREGION_ACTION:
            {
                const MetaISectRectClipRegionAction* pA = (const MetaISectRectClipRegionAction*) pMA;
                if ( aClipRegion.IsEmpty() )
                    aClipRegion = pA->GetRect();
                else
                    aClipRegion.Intersect( pA->GetRect() );
                bRegionChanged = FALSE;
                ImplSetClipRegion();
            }
            break;

            case META_CLIPREGION_ACTION:
            {
                const MetaClipRegionAction* pA = (const MetaClipRegionAction*) pMA;
                bRegionChanged = TRUE;
                aClipRegion = pA->GetRegion();
                ImplSetClipRegion();
            }
            break;

            case META_ISECTREGIONCLIPREGION_ACTION:
            {
                const MetaISectRegionClipRegionAction* pA = (const MetaISectRegionClipRegionAction*) pMA;
                if ( aClipRegion.IsEmpty() )
                    aClipRegion = pA->GetRegion();
                else
                    aClipRegion.Intersect( pA->GetRegion() );
                bRegionChanged = TRUE;
                ImplSetClipRegion();
            }
            break;

            case META_MOVECLIPREGION_ACTION:
            {
                if ( !aClipRegion.IsEmpty() )
                {
                    const MetaMoveClipRegionAction* pA = (const MetaMoveClipRegionAction*) pMA;
                    aClipRegion.Move( pA->GetHorzMove(), pA->GetVertMove() );
                    ImplSetClipRegion();
                }
            }
            break;

            case META_LINECOLOR_ACTION :
            {
                if ( ( (const MetaLineColorAction*) pMA)->IsSetting() )
                {
                    bLineColor = TRUE;
                    aLineColor = ( (const MetaLineColorAction*) pMA )->GetColor();
                }
                else
                    bLineColor = FALSE;
            }
            break;

            case META_FILLCOLOR_ACTION :
            {
                if ( ( (const MetaFillColorAction*) pMA )->IsSetting() )
                {
                    bFillColor = TRUE;
                    aFillColor =  ( (const MetaFillColorAction*) pMA )->GetColor();
                }
                else
                    bFillColor = FALSE;
            }
            break;

            case META_TEXTCOLOR_ACTION :
            {
                aTextColor = ( (const MetaTextColorAction*) pMA )->GetColor();
            }
            break;

            case META_TEXTFILLCOLOR_ACTION :
            {
                if ( ( (const MetaTextFillColorAction*) pMA )->IsSetting() )
                {
                    bTextFillColor = TRUE;
                    aTextFillColor = ( (const MetaTextFillColorAction*) pMA )->GetColor();
                }
                else
                    bTextFillColor = FALSE;
            }
            break;

            case META_TEXTALIGN_ACTION :
            {
                eTextAlign = ( (const MetaTextAlignAction*) pMA )->GetTextAlign();
            }
            break;

            case META_MAPMODE_ACTION :
            {
                aMapMode = ( (const MetaMapModeAction*) pMA )->GetMapMode();

                if( aMapMode.GetMapUnit() == MAP_RELATIVE )
                {
                    nXScaling *= (double)aMapMode.GetScaleX();
                    nYScaling *= (double)aMapMode.GetScaleY();
                    nXOrigin  += (double)aMapMode.GetOrigin().X() * nXScaling;
                    nYOrigin  += (double)aMapMode.GetOrigin().Y() * nYScaling;
                }
                else
                    ImplGetMapMode( aMapMode );
            }
            break;

            case META_FONT_ACTION :
            {
                maFont = ( (const MetaFontAction*) pMA )->GetFont();
                aOutputDevice.SetFont( maFont );
            }
            break;

            case META_PUSH_ACTION :
            {
                StackMember* pGS = new StackMember;
                pGS->pSucc = pGDIStack;
                pGDIStack = pGS;
                pGS->eTextAlign = eTextAlign;
                pGS->aGlobalCol = aColor;
                pGS->bLineCol = bLineColor;
                pGS->aLineCol = aLineColor;
                pGS->bFillCol = bFillColor;
                pGS->aFillCol = aFillColor;
                pGS->aTextCol = aTextColor;
                pGS->bTextFillCol = bTextFillColor;
                pGS->aTextFillCol = aTextFillColor;
                pGS->aBackgroundCol = aBackgroundColor;
                pGS->bRegionChanged = bRegionChanged;
                pGS->aClipReg = aClipRegion;
                pGS->aMapMode = aMapMode;
                bRegionChanged = FALSE;
                pGS->aFont = maFont;
                pGS->nXScale = nXScaling;
                pGS->nYScale = nYScaling;
                pGS->nXOrig = nXOrigin;
                pGS->nYOrig = nYOrigin;
                mnLatestPush = mpPS->Tell();
                ImplWriteLine( "gs" );
            }
            break;

            case META_POP_ACTION :
            {
                StackMember* pGS;
                if( pGDIStack )
                {
                    pGS = pGDIStack;
                    pGDIStack = pGS->pSucc;
                    if ( aMapMode != pGS->aMapMode )
                    {
                        aMapMode = pGS->aMapMode;
                        ImplGetMapMode( aMapMode );
                    }
                    eTextAlign = pGS->eTextAlign;
                    aColor = pGS->aGlobalCol;
                    bLineColor = pGS->bLineCol;
                    aLineColor = pGS->aLineCol;
                    bFillColor = pGS->bFillCol;
                    aFillColor = pGS->aFillCol;
                    aTextColor = pGS->aTextCol;
                    bTextFillColor = pGS->bTextFillCol;
                    aTextFillColor = pGS->aTextFillCol;
                    aBackgroundColor = pGS->aBackgroundCol;
                    if ( bRegionChanged )
                    {
                        aClipRegion = pGS->aClipReg;
                        ImplSetClipRegion();
                    }
                    bRegionChanged = pGS->bRegionChanged;
                    maFont = pGS->aFont;
                    maLastFont = Font();                // set maLastFont != maFont -> so that
                    nXScaling = pGS->nXScale;
                    nYScaling = pGS->nYScale;
                    nXOrigin = pGS->nXOrig;
                    nYOrigin = pGS->nYOrig;
                    delete pGS;
                    UINT32 nCurrentPos = mpPS->Tell();
                    if ( nCurrentPos - 6 == mnLatestPush )
                    {
                        mpPS->Seek( mnLatestPush );
                        ImplWriteLine( "     " );
                        mpPS->Seek( mnLatestPush );
                    }
                    else
                        ImplWriteLine( "gr" );
                }
            }
            break;

            case META_EPS_ACTION :
            {
                GfxLink aGfxLink = ( (const MetaEPSAction*) pMA )->GetLink();
                BOOL    bLevelConflict = FALSE;
                BYTE*   pSource = (BYTE*) aGfxLink.GetData();
                ULONG   nSize = aGfxLink.GetDataSize();
                ULONG   nParseThis = POSTSCRIPT_BOUNDINGSEARCH;
                if ( nSize < 64 )                       // assuming eps is larger than 64 bytes
                    pSource = NULL;
                if ( nParseThis > nSize )
                    nParseThis = nSize;

                if ( pSource && ( mnLevel == 1 ) )
                {
                    BYTE* pFound = ImplSearchEntry( pSource, (BYTE*)"%%LanguageLevel:", nParseThis - 10, 16 );
                    if ( pFound )
                    {
                        BYTE    k, i = 10;
                        pFound += 16;
                        while ( --i )
                        {
                            k = *pFound++;
                            if ( ( k > '0' ) && ( k <= '9' ) )
                            {
                                if ( k != '1' )
                                {
                                    bLevelConflict = TRUE;
                                    mnLevelWarning++;
                                }
                                break;
                            }
                        }
                    }
                }
                if ( !bLevelConflict )
                {
                    double  nBoundingBox[4];
                    if ( pSource && ImplGetBoundingBox( nBoundingBox, pSource, nParseThis ) )
                    {
                        Point   aPoint = ( (const MetaEPSAction*) pMA )->GetPoint();
                        Size    aSize = ( (const MetaEPSAction*) pMA )->GetSize();
                        double nXScale = (double)aSize.Width() * (double)nXScaling / ( nBoundingBox[ 2 ] - nBoundingBox[ 0 ] );
                        double nYScale = (double)aSize.Height() * (double)nYScaling / ( nBoundingBox[ 3 ] - nBoundingBox[ 1 ] );
                        ImplWriteLine( "gs\n%%BeginDocument:" );
                        ImplWriteDouble( aPoint.X() * nXScaling + nXOrigin );
                        ImplWriteDouble( - ( aPoint.Y() * nYScaling + nYOrigin + nBoundingBox[ 3 ] * nYScale ) );
                        ImplWriteLine( "t" );
                        ImplWriteDouble( nXScale );
                        ImplWriteDouble( nYScale );
                        ImplWriteLine( "scale" );
                        mpPS->Write( pSource, aGfxLink.GetDataSize() );
                        ImplWriteLine( "%%EndDocument\ngr" );
                    }
                }
            }
            break;

            case META_TRANSPARENT_ACTION:
            {
                ImplLine( ( (const MetaTransparentAction*) pMA )->GetPolyPolygon() );
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

                ImplWriteActions( aTmpMtf );
            }
            break;
        }
    }
}



//---------------------------------------------------------------------------------

inline void PSWriter::ImplWritePoint( Point nPoint, ULONG nMode )
{
    ImplWriteDouble( nPoint.X() * nXScaling + nXOrigin );
    ImplWriteDouble( nPoint.Y() * nYScaling + nYOrigin, nMode );
}

//---------------------------------------------------------------------------------

void PSWriter::ImplMoveTo( Point nPoint, ULONG nMode )
{
    ImplWritePoint( nPoint );
    ImplWriteByte( 'm' );
    ImplExecMode( nMode );
}

//---------------------------------------------------------------------------------

void PSWriter::ImplLineTo( Point nPoint, ULONG nMode )
{
    ImplWritePoint( nPoint );
    ImplWriteByte( 'l' );
    ImplExecMode( nMode );
}

//---------------------------------------------------------------------------------

void PSWriter::ImplLine( const Polygon &rPolygon )
{
    USHORT i = 1;
    USHORT nPointCount = rPolygon.GetSize();
    if ( nPointCount > 1 )
    {
        ImplMoveTo( rPolygon.GetPoint( 0 ) );
        while ( i < nPointCount )
        {
            ImplLineTo( rPolygon.GetPoint( i++ ), PS_SPACE | PS_WRAP );
        }
    }
}

//---------------------------------------------------------------------------------

void PSWriter::ImplLine( const PolyPolygon &rPolyPolygon )
{
    USHORT i, nCount = rPolyPolygon.Count();
    for ( i = 0; i < nCount; i++ )
    {
        if ( bFillColor )
        {
            ImplWriteFillColor( PS_SPACE );
            ImplLine( rPolyPolygon.GetObject( i ) );
            ImplClosePathFill();
        }
        if ( bLineColor )
        {
            ImplWriteLineColor( PS_SPACE );
            ImplLine( rPolyPolygon.GetObject( i ) );
            ImplClosePathDraw();
        }
    }
}

//---------------------------------------------------------------------------------

void PSWriter::ImplRect( const Rectangle & rRect )
{
    if ( bFillColor )
        ImplRectFill( rRect );
    if ( bLineColor )
    {

        double nWidth = rRect.GetWidth() * nXScaling;
        double nHeight = rRect.GetHeight() * nYScaling;

        ImplWriteLineColor( PS_SPACE );
        ImplMoveTo( rRect.TopLeft() );
        ImplWriteDouble( nWidth );
        *mpPS << "0 rl 0 ";
        ImplWriteDouble( nHeight );
        *mpPS << "rl ";
        ImplWriteDouble( nWidth );
        *mpPS << "neg 0 rl ";
        ImplClosePathDraw();
    }
    *mpPS << (BYTE)10;
    mnCursorPos = 0;
}

//---------------------------------------------------------------------------------

void PSWriter::ImplRectFill( const Rectangle & rRect )
{
    double nWidth = rRect.GetWidth() * nXScaling;
    double nHeight = rRect.GetHeight() * nYScaling;

    ImplWriteFillColor( PS_SPACE );
    ImplMoveTo( rRect.TopLeft() );
    ImplWriteDouble( nWidth );
    *mpPS << "0 rl 0 ";
    ImplWriteDouble( nHeight );
    *mpPS << "rl ";
    ImplWriteDouble( nWidth );
    *mpPS << "neg 0 rl ";
    ImplClosePathFill();
}

//---------------------------------------------------------------------------------

void PSWriter::ImplPoly( const Polygon & rPoly )
{
    if ( rPoly.GetSize() > 1 )
    {
        if ( bFillColor )
        {
            ImplWriteFillColor( PS_SPACE );
            ImplLine( rPoly );
            ImplClosePathFill();
        }
        if ( bLineColor )
        {
            ImplWriteLineColor( PS_SPACE );
            ImplLine( rPoly );
            ImplClosePathDraw();
        }
    }
}

//---------------------------------------------------------------------------------

void PSWriter::ImplSetClipRegion()
{
    if ( !aClipRegion.IsEmpty() )
    {
        Rectangle       aRect;
        RegionHandle    hRegionHandle = aClipRegion.BeginEnumRects();

        while ( aClipRegion.GetNextEnumRect( hRegionHandle, aRect ) )
        {
            double nX1 = aRect.Left() * nXScaling + nXOrigin;
            double nY1 = aRect.Top() * nYScaling + nYOrigin;
            double nX2 = aRect.Right() * nXScaling + nXOrigin;
            double nY2 = aRect.Bottom() * nYScaling + nYOrigin;
            ImplWriteDouble( nX1 );
            ImplWriteDouble( nY1 );
            ImplWriteByte( 'm' );
            ImplWriteDouble( nX2 );
            ImplWriteDouble( nY1 );
            ImplWriteByte( 'l' );
            ImplWriteDouble( nX2 );
            ImplWriteDouble( nY2 );
            ImplWriteByte( 'l' );
            ImplWriteDouble( nX1 );
            ImplWriteDouble( nY2 );
            ImplWriteByte( 'l' );
            ImplWriteDouble( nX1 );
            ImplWriteDouble( nY1 );
            ImplWriteByte( 'l', PS_SPACE | PS_WRAP );
        };
        aClipRegion.EndEnumRects( hRegionHandle );
        ImplWriteLine( "eoclip newpath" );
    }
}

//---------------------------------------------------------------------------------
// possible gfx formats:
//
// level 1: grayscale   8 bit
//          color      24 bit
//
// level 2: grayscale   8 bit
//          color       1(pal), 4(pal), 8(pal), 24 Bit
//

void PSWriter::ImplBmp( Bitmap* pBitmap, Bitmap* pMaskBitmap, const Point & rPoint, double nXWidth, double nYHeightOrg )
{
    if ( !pBitmap )
        return;

    INT32   nHeightOrg = pBitmap->GetSizePixel().Height();
    INT32   nHeightLeft = nHeightOrg;
    long    nWidth = pBitmap->GetSizePixel().Width();
    Point   aSourcePos( rPoint );

    while ( nHeightLeft )
    {
        Bitmap  aTileBitmap( *pBitmap );
        long    nHeight = nHeightLeft;
        double  nYHeight = nYHeightOrg;

        BOOL    bDoTrans = FALSE;

        Rectangle   aRect;
        Region      aRegion;

        if ( pMaskBitmap )
        {
            bDoTrans = TRUE;
            while (TRUE)
            {
                if ( mnLevel == 1 )
                {
                    if ( nHeight > 10 )
                        nHeight = 8;
                }
                aRect = Rectangle( Point( 0, nHeightOrg - nHeightLeft ), Size( (long)nWidth, (long)nHeight ) );
                aRegion = Region( pMaskBitmap->CreateRegion( COL_BLACK, aRect ) );

                if ( ( mnLevel == 1 ) && ( aRegion.GetRectCount() * 5 > 1000 ) )
                {
                    nHeight >>= 1;
                    if ( nHeight < 2 )
                        return;
                    continue;
                }
                break;
            }
        }
        if ( nHeight != nHeightOrg )
        {
            nYHeight = nYHeightOrg * nHeight / nHeightOrg;
            aTileBitmap.Crop( Rectangle( Point( 0, nHeightOrg - nHeightLeft ), Size( nWidth, nHeight ) ) );
        }
        if ( bDoTrans )
        {
            ImplWriteLine( "gs\npum" );
            ImplWriteDouble( aSourcePos.X() * nXScaling + nXOrigin );
            ImplWriteDouble( -aSourcePos.Y() * nYScaling - nYOrigin );
            ImplWriteLine( "t" );
            ImplWriteDouble( nXWidth * nXScaling / nWidth );
            ImplWriteDouble( nYHeight * nYScaling / nHeight );
            ImplWriteLine( "scale" );

            if ( !aClipRegion.IsEmpty() )
            {
    //          aRegion.Intersect( aClipRegion );
            }
            RegionHandle    hRegionHandle = aRegion.BeginEnumRects();

            while ( aRegion.GetNextEnumRect( hRegionHandle, aRect ) )
            {
                aRect.Move( 0, - ( nHeightOrg - nHeightLeft ) );
                ImplWriteLong( aRect.Left() );
                ImplWriteLong( aRect.Top() );
                ImplWriteByte( 'm' );
                ImplWriteLong( aRect.Right() + 1 );
                ImplWriteLong( aRect.Top() );
                ImplWriteByte( 'l' );
                ImplWriteLong( aRect.Right() + 1 );
                ImplWriteLong( aRect.Bottom() + 1 );
                ImplWriteByte( 'l' );
                ImplWriteLong( aRect.Left() );
                ImplWriteLong( aRect.Bottom() + 1 );
                ImplWriteByte( 'l' );
                ImplWriteByte( 'p', PS_SPACE | PS_WRAP );
            };
            aRegion.EndEnumRects( hRegionHandle );
            ImplWriteLine( "eoclip newpath" );
            ImplWriteLine( "pom" );
        }
        BitmapReadAccess* pAcc = aTileBitmap.AcquireReadAccess();

        if (!bDoTrans )
            ImplWriteLine( "pum" );

        ImplWriteDouble( aSourcePos.X() * nXScaling + nXOrigin );
        ImplWriteDouble( -aSourcePos.Y() * nYScaling - nYOrigin - nYHeight * nYScaling );
        ImplWriteLine( "t" );
        ImplWriteDouble( nXWidth * nXScaling );
        ImplWriteDouble( nYHeight * nYScaling );
        ImplWriteLine( "scale" );
        if ( mnLevel == 1 )                 // level 1 is always grayscale !!!
        {
            ImplWriteLong( nWidth );
            ImplWriteLong( nHeight );
            *mpPS << "8 [";
            ImplWriteLong( nWidth );
            *mpPS << "0 0 ";
            ImplWriteLong( -nHeight );
            ImplWriteLong( 0 );
            ImplWriteLong( nHeight );
            ImplWriteLine( "]" );
            *mpPS << "{currentfile ";
            ImplWriteLong( nWidth );
            ImplWriteLine( "string readhexstring pop}" );
            ImplWriteLine( "image" );
            for ( long y = 0; y < nHeight; y++ )
            {
                for ( long x = 0; x < nWidth; x++ )
                {
                    ImplWriteHexByte( (BYTE)pAcc->GetPixel( y, x ) );
                }
            }
            *mpPS << (BYTE)10;
        }
        else    // Level 2
        {
            if ( mbGrayScale )
            {
                ImplWriteLine( "/DeviceGray setcolorspace" );
                ImplWriteLine( "<<" );
                ImplWriteLine( "/ImageType 1" );
                *mpPS << "/Width ";
                ImplWriteLong( nWidth, PS_RET );
                *mpPS << "/Height ";
                ImplWriteLong( nHeight, PS_RET );
                ImplWriteLine( "/BitsPerComponent 8" );
                ImplWriteLine( "/Decode[0 1]" );
                *mpPS << "/ImageMatrix[";
                ImplWriteLong( nWidth );
                *mpPS << "0 0 ";
                ImplWriteLong( -nHeight );
                ImplWriteLong( 0 );
                ImplWriteLong( nHeight, PS_NONE );
                ImplWriteByte( ']', PS_RET );
                ImplWriteLine( "/DataSource currentfile" );
                ImplWriteLine( "/ASCIIHexDecode filter" );
                if ( mbCompression )
                    ImplWriteLine( "/LZWDecode filter" );
                ImplWriteLine( ">>" );
                ImplWriteLine( "image" );
                if ( mbCompression )
                {
                    StartCompression();
                    for ( long y = 0; y < nHeight; y++ )
                    {
                        for ( long x = 0; x < nWidth; x++ )
                        {
                            Compress( (BYTE)pAcc->GetPixel( y, x ) );
                        }
                    }
                    EndCompression();
                }
                else
                {
                    for ( long y = 0; y < nHeight; y++ )
                    {
                        for ( long x = 0; x < nWidth; x++ )
                        {
                            ImplWriteHexByte( (BYTE)pAcc->GetPixel( y, x ) );
                        }
                    }
                }
            }
            else
            {
                // have we to write a palette ?

                if ( pAcc->HasPalette() )
                {
                    ImplWriteLine( "[/Indexed /DeviceRGB " );
                    ImplWriteLong( pAcc->GetPaletteEntryCount() - 1, PS_RET );
                    ImplWriteByte( '<', PS_NONE );
                    for ( USHORT i = 0; i < pAcc->GetPaletteEntryCount(); i++ )
                    {
                        BitmapColor aBitmapColor = pAcc->GetPaletteColor( i );
                        ImplWriteHexByte( aBitmapColor.GetRed(), PS_NONE );
                        ImplWriteHexByte( aBitmapColor.GetGreen(), PS_NONE );
                        ImplWriteHexByte( aBitmapColor.GetBlue(), PS_SPACE | PS_WRAP );
                    }
                    ImplWriteByte( '>', PS_RET );

                    ImplWriteLine( "] setcolorspace" );
                    ImplWriteLine( "<<" );
                    ImplWriteLine( "/ImageType 1" );
                    *mpPS << "/Width ";
                    ImplWriteLong( nWidth, PS_RET );
                    *mpPS << "/Height ";
                    ImplWriteLong( nHeight, PS_RET );
                    ImplWriteLine( "/BitsPerComponent 8" );
                    ImplWriteLine( "/Decode[0 255]" );
                    *mpPS << "/ImageMatrix[";
                    ImplWriteLong( nWidth );
                    *mpPS << "0 0 ";
                    ImplWriteLong( -nHeight );
                    ImplWriteLong( 0);
                    ImplWriteLong( nHeight, PS_NONE );
                    ImplWriteByte( ']', PS_RET );
                    ImplWriteLine( "/DataSource currentfile" );
                    ImplWriteLine( "/ASCIIHexDecode filter" );
                    if ( mbCompression )
                        ImplWriteLine( "/LZWDecode filter" );
                    ImplWriteLine( ">>" );
                    ImplWriteLine( "image" );
                    if ( mbCompression )
                    {
                        StartCompression();
                        for ( long y = 0; y < nHeight; y++ )
                        {
                            for ( long x = 0; x < nWidth; x++ )
                            {
                                Compress( (BYTE)pAcc->GetPixel( y, x ) );
                            }
                        }
                        EndCompression();
                    }
                    else
                    {
                        for ( long y = 0; y < nHeight; y++ )
                        {
                            for ( long x = 0; x < nWidth; x++ )
                            {
                                ImplWriteHexByte( (BYTE)pAcc->GetPixel( y, x ) );
                            }
                        }
                    }
                }
                else // 24 bit color
                {
                    ImplWriteLine( "/DeviceRGB setcolorspace" );
                    ImplWriteLine( "<<" );
                    ImplWriteLine( "/ImageType 1" );
                    *mpPS << "/Width ";
                    ImplWriteLong( nWidth, PS_RET );
                    *mpPS << "/Height ";
                    ImplWriteLong( nHeight, PS_RET );
                    ImplWriteLine( "/BitsPerComponent 8" );
                    ImplWriteLine( "/Decode[0 1 0 1 0 1]" );
                    *mpPS << "/ImageMatrix[";
                    ImplWriteLong( nWidth );
                    *mpPS << "0 0 ";
                    ImplWriteLong( -nHeight );
                    ImplWriteLong( 0 );
                    ImplWriteLong( nHeight, PS_NONE );
                    ImplWriteByte( ']', PS_RET );
                    ImplWriteLine( "/DataSource currentfile" );
                    ImplWriteLine( "/ASCIIHexDecode filter" );
                    if ( mbCompression )
                        ImplWriteLine( "/LZWDecode filter" );
                    ImplWriteLine( ">>" );
                    ImplWriteLine( "image" );
                    if ( mbCompression )
                    {
                        StartCompression();
                        for ( long y = 0; y < nHeight; y++ )
                        {
                            for ( long x = 0; x < nWidth; x++ )
                            {
                                const BitmapColor aBitmapColor( pAcc->GetPixel( y, x ) );
                                Compress( aBitmapColor.GetRed() );
                                Compress( aBitmapColor.GetGreen() );
                                Compress( aBitmapColor.GetBlue() );
                            }
                        }
                        EndCompression();
                    }
                    else
                    {
                        for ( long y = 0; y < nHeight; y++ )
                        {
                            for ( long x = 0; x < nWidth; x++ )
                            {
                                const BitmapColor aBitmapColor( pAcc->GetPixel( y, x ) );
                                ImplWriteHexByte( aBitmapColor.GetRed() );
                                ImplWriteHexByte( aBitmapColor.GetGreen() );
                                ImplWriteHexByte( aBitmapColor.GetBlue() );
                            }
                        }
                    }
                }
            }
            ImplWriteLine( ">" );       // in Level 2 the dictionary needs to be closed (eod)
        }
        if ( bDoTrans )
            ImplWriteLine( "gr" );
        else
            ImplWriteLine( "pom" );

        aTileBitmap.ReleaseAccess( pAcc );
        nHeightLeft -= nHeight;
        if ( nHeightLeft )
        {
            nHeightLeft++;
            aSourcePos.Y() = (long) ( rPoint.Y() + ( nYHeightOrg * ( nHeightOrg - nHeightLeft ) ) / nHeightOrg );
        }
    }
}

//---------------------------------------------------------------------------------

void PSWriter::ImplWriteCharacter( sal_Char nChar )
{
    switch( nChar )
    {
        case '(' :
        case ')' :
        case '\\' :
            ImplWriteByte( (BYTE)'\\', PS_NONE );
    }
    ImplWriteByte( (BYTE)nChar, PS_NONE );
}

//---------------------------------------------------------------------------------

void PSWriter::ImplWriteString( const ByteString& rString, const INT32* pDXArry, BOOL bStretch )
{
    USHORT nLen = rString.Len();
    if ( nLen )
    {
        USHORT i;
        if ( pDXArry )
        {
            double nx = 0;

            for( i = 0; i < nLen; i++ )
            {
                if ( i > 0 )
                    nx = pDXArry[ i - 1 ] * nXScaling;
                ImplWriteDouble( ( bStretch ) ? nx : aOutputDevice.GetTextWidth( rString.GetChar( i ) ) * nXScaling );
                ImplWriteDouble( nx );
                ImplWriteLine( "(", PS_NONE );
                ImplWriteCharacter( rString.GetChar( i ) );
                ImplWriteLine( ") bs" );
            }
        }
        else
        {
            ImplWriteByte( '(', PS_NONE );
            for ( i = 0; i < nLen; i++ )
                ImplWriteCharacter( rString.GetChar( i ) );
            ImplWriteLine( ") s" );
        }
    }
}

// ------------------------------------------------------------------------

void PSWriter::ImplSetAttrForText( Point & aPoint )
{
    long nRotation = maFont.GetOrientation();
    ImplWriteTextColor();

    Size aSize = maFont.GetSize();

    if ( maLastFont != maFont )
    {
        if ( maFont.GetPitch() == PITCH_FIXED )         // a little bit font selection
            ImplDefineFont( "Courier", "Oblique" );
        else if ( maFont.GetCharSet() == RTL_TEXTENCODING_SYMBOL )
            ImplWriteLine( "/Symbol findfont" );
        else if ( maFont.GetFamily() == FAMILY_SWISS )
            ImplDefineFont( "Helvetica", "Oblique" );
        else
            ImplDefineFont( "Times", "Italic" );

        maLastFont = maFont;
        aSize = maFont.GetSize();
        ImplWriteDouble( aSize.Height() * nYScaling );      ///???????????????????
        *mpPS << "sf ";
    }
    if ( eTextAlign != ALIGN_BASELINE )
    {                                                       // PostScript kennt kein FontAlignment
        if ( eTextAlign == ALIGN_TOP )                      // -> ich gehe daher davon aus, dass
            aPoint.Y() += ( aSize.Height() * 4 / 5 );       // der Bereich unter der Baseline
        else if ( eTextAlign == ALIGN_BOTTOM )              // in etwa 20% der Fontsize ausmacht
            aPoint.Y() -= ( aSize.Height() / 5 );
    }
    ImplMoveTo( aPoint );
    if ( nRotation )
    {
        *mpPS << "gs ";
        ImplWriteF( nRotation, 1 );
        *mpPS << "r ";
    }
}

//---------------------------------------------------------------------------------

void PSWriter::ImplDefineFont( char* pOriginalName, char* pItalic )
{
    *mpPS << (BYTE)'/';             //convert the font pOriginalName using ISOLatin1Encoding
    *mpPS << pOriginalName;
    switch ( maFont.GetWeight() )
    {
        case WEIGHT_SEMIBOLD :
        case WEIGHT_BOLD :
        case WEIGHT_ULTRABOLD :
        case WEIGHT_BLACK :
            *mpPS << "-Bold";
            if ( maFont.GetItalic() != ITALIC_NONE )
                *mpPS << pItalic;
            break;
        default:
            if ( maFont.GetItalic() != ITALIC_NONE )
                *mpPS << pItalic;
            break;
    }
    ImplWriteLine( " f" );
}

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------

void PSWriter::ImplClosePathFill( ULONG nMode )
{
    *mpPS << "pf";
    mnCursorPos += 2;
    ImplExecMode( nMode );
}

void PSWriter::ImplClosePathDraw( ULONG nMode )
{
    *mpPS << "pc";
    mnCursorPos += 2;
    ImplExecMode( nMode );
}

void PSWriter::ImplPathDraw( ULONG nMode )
{
    *mpPS << "ps";
    mnCursorPos += 2;
    ImplExecMode( nMode );
}

//---------------------------------------------------------------------------------

inline void PSWriter::ImplWriteLineColor( ULONG nMode )
{
    if ( aColor != aLineColor )
    {
        aColor = aLineColor;
        ImplWriteColor( nMode );
    }
}
inline void PSWriter::ImplWriteFillColor( ULONG nMode )
{
    if ( aColor != aFillColor )
    {
        aColor = aFillColor;
        ImplWriteColor( nMode );
    }
}
inline void PSWriter::ImplWriteTextColor( ULONG nMode )
{
    if ( aColor != aTextColor )
    {
        aColor = aTextColor;
        ImplWriteColor( nMode );
    }
}
inline void PSWriter::ImplWriteTextFillColor( ULONG nMode )
{
    if ( aColor != aTextFillColor )
    {
        aColor = aTextFillColor;
        ImplWriteColor( nMode );
    }
}

//---------------------------------------------------------------------------------

void PSWriter::ImplWriteColor( ULONG nMode )
{
    if ( mbGrayScale )
    {
        // writes the Color (grayscale) as a Number from 0.000 up to 1.000

        ImplWriteF( 1000 * ( (BYTE)aColor.GetRed() * 77 + (BYTE)aColor.GetGreen() * 151 +
            (BYTE)aColor.GetBlue() * 28 + 1 ) / 65536, 3, nMode );
    }
    else
    {
        ImplWriteB1 ( (BYTE)aColor.GetRed() );
        ImplWriteB1 ( (BYTE)aColor.GetGreen() );
        ImplWriteB1 ( (BYTE)aColor.GetBlue() );
    }
    *mpPS << "c";                               // ( c is defined as setrgbcolor or setgray )
    ImplExecMode( nMode );
}

//---------------------------------------------------------------------------------

void PSWriter::ImplGetMapMode( const MapMode& aMapMode )
{
    aOutputDevice.SetMapMode( aMapMode );
    double  nMul;
    switch ( aMapMode.GetMapUnit() )
    {
        case MAP_PIXEL :
        case MAP_SYSFONT :
        case MAP_APPFONT :

        case MAP_100TH_MM :
            nMul = 1 * EPS_SCALING_FAKTOR;
            break;
        case MAP_10TH_MM :
            nMul = 10 * EPS_SCALING_FAKTOR;
            break;
        case MAP_MM :
            nMul = 100 * EPS_SCALING_FAKTOR;
            break;
        case MAP_CM :
            nMul = 1000 * EPS_SCALING_FAKTOR;
            break;
        case MAP_1000TH_INCH :
            nMul = 2.54 * EPS_SCALING_FAKTOR;
            break;
        case MAP_100TH_INCH :
            nMul = 25.4 * EPS_SCALING_FAKTOR;
            break;
        case MAP_10TH_INCH :
            nMul = 254 * EPS_SCALING_FAKTOR;
            break;
        case MAP_INCH :
            nMul = 2540 * EPS_SCALING_FAKTOR;
            break;
        case MAP_TWIP :
            nMul = 1,76388889 * EPS_SCALING_FAKTOR;
            break;
        case MAP_POINT :
            nMul = 35,27777778 * EPS_SCALING_FAKTOR;
            break;
        default:
            // that does not look right
            break;
    }
    nXOrigin = aMapMode.GetOrigin().X() * nMul;
    nYOrigin = aMapMode.GetOrigin().Y() * nMul;
    double nScale = aMapMode.GetScaleX();
    nXScaling = nMul * nScale;
    nScale = aMapMode.GetScaleY();
    nYScaling = nMul * nScale;
}

//---------------------------------------------------------------------------------

inline void PSWriter::ImplExecMode( ULONG nMode )
{
    if ( nMode & PS_WRAP )
    {
        if ( mnCursorPos >= PS_LINESIZE )
        {
            mnCursorPos = 0;
            *mpPS << (BYTE)0xa;
            return;
        }
    }
    if ( nMode & PS_SPACE )
    {
            *mpPS << (BYTE)32;
            mnCursorPos++;
    }
    if ( nMode & PS_RET )
    {
        *mpPS << (BYTE)0xa;
        mnCursorPos = 0;
    }
}

//---------------------------------------------------------------------------------

inline void PSWriter::ImplWriteLine( char pString[], ULONG nMode )
{
    ULONG i = 0;
    while ( pString[ i ] )
    {
        *mpPS << (BYTE)pString[ i++ ];
    }
    mnCursorPos += i;
    ImplExecMode( nMode );
}

//---------------------------------------------------------------------------------

void PSWriter::ImplWriteLineInfo( const LineInfo& rLineInfo )
{
    double fLineWidth = ( ( rLineInfo.GetWidth() + 1 ) * nXScaling + ( rLineInfo.GetWidth() + 1 ) * nYScaling ) * 0.5;
    ImplWriteLine( "cl", PS_SPACE );    // currentLineWidth & currentDash auf den Stack
    ImplWriteDouble( fLineWidth );
    ImplWriteLine( " lw", PS_SPACE );
    if ( rLineInfo.GetStyle() == LINE_DASH )
    {
        ImplWriteLine( "[ 2 ] 1 ld" );
    }
}

//---------------------------------------------------------------------------------

void PSWriter::ImplWriteLong( sal_Int32 nNumber, ULONG nMode )
{
    const ByteString aNumber( ByteString::CreateFromInt32( nNumber ) );
    ULONG nLen = aNumber.Len();
    mnCursorPos += nLen;
    for ( USHORT n = 0; n < nLen; n++ )
        *mpPS << aNumber.GetChar( n );
    ImplExecMode( nMode );
}

//---------------------------------------------------------------------------------

void PSWriter::ImplWriteDouble( double fNumber, ULONG nMode )
{
    sal_Int32 n, nLen;

    sal_Int32   nPTemp = (sal_Int32)fNumber;
    sal_Int32   nATemp = abs( ( fNumber - nPTemp ) * 100000 );

    if ( !nPTemp && nATemp && ( fNumber < 0.0 ) )
        *mpPS << (sal_Char)'-';

    ByteString aNumber1( ByteString::CreateFromInt32( nPTemp ) );
    nLen = aNumber1.Len();
    mnCursorPos += nLen;
    for ( n = 0; n < nLen; n++ )
        *mpPS << aNumber1.GetChar( n );

    int zCount = 0;
    if ( nATemp )
    {
        *mpPS << (BYTE)'.';
        mnCursorPos++;
        const ByteString aNumber2( ByteString::CreateFromInt32( nATemp ) );

        ULONG nLen = aNumber2.Len();
        if ( nLen < 8 )
        {
            mnCursorPos += 6 - nLen;
            for ( n = 0; n < ( 5 - nLen ); n++ )
            {
                *mpPS << (BYTE)'0';
            }
        }
        mnCursorPos += nLen;
        for ( USHORT n = 0; n < nLen; n++ )
        {
            *mpPS << aNumber2.GetChar( n );
            zCount--;
            if ( aNumber2.GetChar( n ) != '0' )
                zCount = 0;
        }
        if ( zCount )
            mpPS->SeekRel( zCount );
    }
    ImplExecMode( nMode );
}

//---------------------------------------------------------------------------------

// writes the number to stream: nNumber / ( 10^nCount )

void PSWriter::ImplWriteF( sal_Int32 nNumber, ULONG nCount, ULONG nMode )
{
    if ( nNumber < 0 )
    {
        *mpPS << (BYTE)'-';
        nNumber = -nNumber;
        mnCursorPos++;
    }
    const ByteString aScaleFactor( ByteString::CreateFromInt32( nNumber ) );
    ULONG nLen = aScaleFactor.Len();
    long nStSize =  ( nCount + 1 ) - nLen;
    if ( nStSize >= 1 )
    {
        *mpPS << (BYTE)'0';
        mnCursorPos++;
    }
    if ( nStSize >= 2 )
    {
        *mpPS << (BYTE)'.';
        for ( long i = 1; i < nStSize; i++ )
        {
            *mpPS << (BYTE)'0';
            mnCursorPos++;
        }
    }
    mnCursorPos += nLen;
    for( USHORT n = 0UL; n < nLen; n++  )
    {
        if ( n == nLen - nCount )
        {
            *mpPS << (BYTE)'.';
            mnCursorPos++;
        }
        *mpPS << aScaleFactor.GetChar( n );
    }
    ImplExecMode( nMode );
}

//---------------------------------------------------------------------------------

void PSWriter::ImplWriteByte( BYTE nNumb, ULONG nMode )
{
    *mpPS << ( nNumb );
    mnCursorPos++;
    ImplExecMode( nMode );
}

//---------------------------------------------------------------------------------

void PSWriter::ImplWriteHexByte( BYTE nNumb, ULONG nMode )
{
    if ( ( nNumb >> 4 ) > 9 )
        *mpPS << (BYTE)( ( nNumb >> 4 ) + 'A' - 10 );
    else
        *mpPS << (BYTE)( ( nNumb >> 4 ) + '0' );

    if ( ( nNumb & 0xf ) > 9 )
        *mpPS << (BYTE)( ( nNumb & 0xf ) + 'A' - 10 );
    else
        *mpPS << (BYTE)( ( nNumb & 0xf ) + '0' );
    mnCursorPos += 2;
    ImplExecMode( nMode );
}

//---------------------------------------------------------------------------------

// writes the BYTE nNumb as a Number from 0.000 up to 1.000

void PSWriter::ImplWriteB1( BYTE nNumb, ULONG nMode )
{
    ImplWriteF( 1000 * ( nNumb + 1 ) / 256 , 3, nMode );
}


// ------------------------------------------------------------------------

inline void PSWriter::WriteBits( USHORT nCode, USHORT nCodeLen )
{
    dwShift |= ( nCode << ( nOffset - nCodeLen ) );
    nOffset -= nCodeLen;
    while ( nOffset < 24 )
    {
        ImplWriteHexByte( (BYTE)( dwShift >> 24 ) );
        dwShift <<= 8;
        nOffset += 8;
    }
    if ( nCode == 257 && nOffset != 32 )
        ImplWriteHexByte( (BYTE)( dwShift >> 24 ) );
}

// ------------------------------------------------------------------------

void PSWriter::StartCompression()
{
    USHORT i;
    nDataSize = 8;

    nClearCode = 1 << nDataSize;
    nEOICode = nClearCode + 1;
    nTableSize = nEOICode + 1;
    nCodeSize = nDataSize + 1;

    nOffset = 32;                       // anzahl freier bits in dwShift
    dwShift = 0;

    pTable = new PSLZWCTreeNode[ 4096 ];

    for ( i = 0; i < 4096; i++ )
    {
        pTable[ i ].pBrother = pTable[ i ].pFirstChild = NULL;
        pTable[ i ].nValue = (BYTE)( pTable[ i ].nCode = i );
    }
    pPrefix = NULL;
    WriteBits( nClearCode, nCodeSize );
}

// ------------------------------------------------------------------------

void PSWriter::Compress( BYTE nCompThis )
{
    PSLZWCTreeNode*     p;
    USHORT              i;
    BYTE                nV;

    if( !pPrefix )
    {
        pPrefix = pTable + nCompThis;
    }
    else
    {
        nV = nCompThis;
        for( p = pPrefix->pFirstChild; p != NULL; p = p->pBrother )
        {
            if ( p->nValue == nV )
                break;
        }

        if( p )
            pPrefix = p;
        else
        {
            WriteBits( pPrefix->nCode, nCodeSize );

            if ( nTableSize == 409 )
            {
                WriteBits( nClearCode, nCodeSize );

                for ( i = 0; i < nClearCode; i++ )
                    pTable[ i ].pFirstChild = NULL;

                nCodeSize = nDataSize + 1;
                nTableSize = nEOICode + 1;
            }
            else
            {
                if( nTableSize == (USHORT)( ( 1 << nCodeSize ) - 1 ) )
                    nCodeSize++;

                p = pTable + ( nTableSize++ );
                p->pBrother = pPrefix->pFirstChild;
                pPrefix->pFirstChild = p;
                p->nValue = nV;
                p->pFirstChild = NULL;
            }

            pPrefix = pTable + nV;
        }
    }
}

// ------------------------------------------------------------------------

void PSWriter::EndCompression()
{
    if( pPrefix )
        WriteBits( pPrefix->nCode, nCodeSize );

    WriteBits( nEOICode, nCodeSize );
    delete[] pTable;
}

// ------------------------------------------------------------------------

BYTE* PSWriter::ImplSearchEntry( BYTE* pSource, BYTE* pDest, ULONG nComp, ULONG nSize )
{
    while ( nComp-- >= nSize )
    {
        ULONG i;
        for ( i = 0; i < nSize; i++ )
        {
            if ( ( pSource[i]&~0x20 ) != ( pDest[i]&~0x20 ) )
                break;
        }
        if ( i == nSize )
            return pSource;
        pSource++;
    }
    return NULL;
}

// ------------------------------------------------------------------------

BOOL PSWriter::ImplGetBoundingBox( double* nNumb, BYTE* pSource, ULONG nSize )
{
    BOOL    bRetValue = FALSE;
    ULONG   nBytesRead;

    if ( nSize < 256 )      // we assume that the file is greater than 256 bytes
        return FALSE;

    if ( nSize < POSTSCRIPT_BOUNDINGSEARCH )
        nBytesRead = nSize;
    else
        nBytesRead = POSTSCRIPT_BOUNDINGSEARCH;

    BYTE* pDest = ImplSearchEntry( pSource, (BYTE*)"%%BoundingBox:", nBytesRead, 14 );
    if ( pDest )
    {
        int     nSecurityCount = 100;   // only 100 bytes following the bounding box will be checked
        nNumb[0] = nNumb[1] = nNumb[2] = nNumb[3] = 0;
        pDest += 14;
        for ( int i = 0; ( i < 4 ) && nSecurityCount; i++ )
        {
            int     nDivision = 1;
            BOOL    bDivision = FALSE;
            BOOL    bNegative = FALSE;
            BOOL    bValid = TRUE;

            while ( ( --nSecurityCount ) && ( *pDest == ' ' ) || ( *pDest == 0x9 ) ) pDest++;
            BYTE nByte = *pDest;
            while ( nSecurityCount && ( nByte != ' ' ) && ( nByte != 0x9 ) && ( nByte != 0xd ) && ( nByte != 0xa ) )
            {
                switch ( nByte )
                {
                    case '.' :
                        if ( bDivision )
                            bValid = FALSE;
                        else
                            bDivision = TRUE;
                        break;
                    case '-' :
                        bNegative = TRUE;
                        break;
                    default :
                        if ( ( nByte < '0' ) || ( nByte > '9' ) )
                            nSecurityCount = 1;     // error parsing the bounding box values
                        else if ( bValid )
                        {
                            if ( bDivision )
                                nDivision*=10;
                            nNumb[i] *= 10;
                            nNumb[i] += nByte - '0';
                        }
                        break;
                }
                nSecurityCount--;
                nByte = *(++pDest);
            }
            if ( bNegative )
                nNumb[i] = -nNumb[i];
            if ( bDivision && ( nDivision != 1 ) )
                nNumb[i] /= nDivision;
        }
        if ( nSecurityCount)
            bRetValue = TRUE;
    }
    return bRetValue;
}

//================== GraphicExport - die exportierte Funktion ================

#ifdef WNT
extern "C" BOOL _cdecl GraphicExport(SvStream & rStream, Graphic & rGraphic,
                              PFilterCallback pCallback, void * pCallerData,
                              Config* pOptionsConfig, BOOL)
#else
extern "C" BOOL GraphicExport(SvStream & rStream, Graphic & rGraphic,
                              PFilterCallback pCallback, void * pCallerData,
                              Config* pOptionsConfig, BOOL)
#endif
{
    PSWriter aPSWriter;
    return aPSWriter.WritePS( rGraphic, rStream, pCallback, pCallerData, pOptionsConfig );
}

//---------------------------------------------------------------------------------

extern "C" BOOL __LOADONCALLAPI DoExportDialog( FltCallDialogParameter& rPara )
{
    BOOL bRet = FALSE;

    if ( rPara.pWindow && rPara.pCfg )
    {
        ByteString  aResMgrName( "eps" );
        ResMgr* pResMgr;

        aResMgrName.Append( ByteString::CreateFromInt32( SOLARUPD ) );
        pResMgr = ResMgr::CreateResMgr( aResMgrName.GetBuffer(), Application::GetAppInternational().GetLanguage() );

        if( pResMgr )
        {
            rPara.pResMgr = pResMgr;
            bRet = ( DlgExportEPS( rPara ).Execute() == RET_OK );
            delete pResMgr;
        }
        else
            bRet = TRUE;
    }

    return bRet;
}

//================== ein bischen Muell fuer Windows ==========================

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
