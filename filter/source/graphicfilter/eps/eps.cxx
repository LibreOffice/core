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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"

#include <vcl/sv.h>
#include <tools/stream.hxx>
#include <tools/bigint.hxx>
#include <tools/poly.hxx>
#include <vcl/svapp.hxx>
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
#include <svl/solar.hrc>
#include <svtools/fltcall.hxx>
#include <svtools/FilterConfigItem.hxx>
#include <vcl/graphictools.hxx>
#include "strings.hrc"
#include "dlgeps.hrc"
#include "dlgeps.hxx"

#include <math.h>

#define POSTSCRIPT_BOUNDINGSEARCH	0x1000	// we only try to get the BoundingBox
                                            // in the first 4096 bytes

#define EPS_PREVIEW_TIFF	1
#define EPS_PREVIEW_EPSI	2

#define PS_LINESIZE 		70		// maximum number of characters a line in the output

#define PS_NONE 			0		// formating mode: action which is inserted behind the output
#define PS_SPACE			1
#define PS_RET				2
#define PS_WRAP 			4

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
    struct		StackMember * pSucc;
    Color		aGlobalCol;
    BOOL		bLineCol;
    Color		aLineCol;
    BOOL		bFillCol;
    Color		aFillCol;
    Color		aTextCol;
    BOOL		bTextFillCol;
    Color		aTextFillCol;
    Color		aBackgroundCol;
    Font		aFont;
    TextAlign	eTextAlign;

    double						fLineWidth;
    double						fMiterLimit;
    SvtGraphicStroke::CapType	eLineCap;
    SvtGraphicStroke::JoinType	eJoinType;
    SvtGraphicStroke::DashArray aDashArray;
};

struct PSLZWCTreeNode
{

    PSLZWCTreeNode* 	pBrother;		// naechster Knoten, der den selben Vater hat
    PSLZWCTreeNode* 	pFirstChild;	// erster Sohn
    USHORT				nCode;			// Der Code fuer den String von Pixelwerten, der sich ergibt, wenn
    USHORT				nValue; 		// Der Pixelwert
};

class PSWriter
{
private:
    BOOL				mbStatus;
    ULONG				mnLevelWarning; 	// number of embedded eps files which was not exported
    ULONG				mnLastPercent;		// Mit welcher Zahl pCallback zuletzt aufgerufen wurde.
    UINT32				mnLatestPush;		// offset auf streamposition, an der zuletzt gepusht wurde

    long				mnLevel;			// dialog options
    sal_Bool			mbGrayScale;
    sal_Bool			mbCompression;
    sal_Int32			mnPreview;
    sal_Int32			mnTextMode;

    SvStream*			mpPS;
    const GDIMetaFile*	pMTF;
    GDIMetaFile*		pAMTF;				// only created if Graphics is not a Metafile
    VirtualDevice		aVDev;

    double				nBoundingX1;		// this represents the bounding box
    double				nBoundingY1;
    double				nBoundingX2;
    double				nBoundingY2;
                                            // 
    StackMember*		pGDIStack;
    ULONG				mnCursorPos;		// aktuelle Cursorposition im Output
    Color				aColor; 			// aktuelle Farbe die fuer den Output benutzt wird
    BOOL				bLineColor;
    Color				aLineColor; 		// aktuelle GDIMetafile Farbeinstellungen
    BOOL				bFillColor; 		//
    Color				aFillColor; 		//
    Color				aTextColor; 		//
    BOOL				bTextFillColor; 	//
    Color				aTextFillColor; 	//
    Color				aBackgroundColor;	//
    BOOL				bRegionChanged;
    TextAlign			eTextAlign;			//

    double						fLineWidth;
    double						fMiterLimit;
    SvtGraphicStroke::CapType	eLineCap;
    SvtGraphicStroke::JoinType	eJoinType;
    SvtGraphicStroke::DashArray aDashArray;

    Font				maFont;
    Font				maLastFont;
    BYTE				nChrSet;
    ChrSet* 			pChrSetList;		// Liste der Character-Sets
    BYTE				nNextChrSetId;		// die erste unbenutzte ChrSet-Id

    PSLZWCTreeNode* 	pTable; 			// LZW compression data
    PSLZWCTreeNode* 	pPrefix;			// the compression is as same as the TIFF compression
    USHORT				nDataSize;
    USHORT				nClearCode;
    USHORT				nEOICode;
    USHORT				nTableSize;
    USHORT				nCodeSize;
    ULONG				nOffset;
    ULONG				dwShift;

    com::sun::star::uno::Reference< com::sun::star::task::XStatusIndicator > xStatusIndicator;

    void				ImplWriteProlog( const Graphic* pPreviewEPSI = NULL );
    void				ImplWriteEpilog();
    void				ImplWriteActions( const GDIMetaFile& rMtf, VirtualDevice& rVDev );

                        // this method makes LF's, space inserting and word wrapping as used in all nMode
                        // parameters
    inline void 		ImplExecMode( ULONG nMode );

                        // writes char[] + LF to stream
    inline void 		ImplWriteLine( const char*, ULONG nMode = PS_RET );

                        // writes ( nNumb / 10^nCount ) in ASCII format to stream
    void				ImplWriteF( sal_Int32 nNumb, ULONG nCount = 3, ULONG nMode = PS_SPACE );

                        // writes a double in ASCII format to stream
    void				ImplWriteDouble( double, ULONG nMode = PS_SPACE );

                        // writes a long in ASCII format to stream
    void				ImplWriteLong( sal_Int32 nNumb, ULONG nMode = PS_SPACE );

                        // writes a byte in ASCII format to stream
    void				ImplWriteByte( BYTE nNumb, ULONG nMode = PS_SPACE );

                        // writes a byte in ASCII (hex) format to stream
    void				ImplWriteHexByte( BYTE nNumb, ULONG nMode = PS_WRAP );

                        // writes nNumb as number from 0.000 till 1.000 in ASCII format to stream
    void				ImplWriteB1( BYTE nNumb, ULONG nMode = PS_SPACE );

    inline void 		ImplWritePoint( const Point&, sal_uInt32 nMode = PS_SPACE );
    void				ImplMoveTo( const Point&, sal_uInt32 nMode = PS_SPACE );
    void				ImplLineTo( const Point&, sal_uInt32 nMode = PS_SPACE );
    void				ImplCurveTo( const Point& rP1, const Point& rP2, const Point& rP3, sal_uInt32 nMode = PS_SPACE );
    void				ImplTranslate( const double& fX, const double& fY, sal_uInt32 nMode = PS_RET );
    void				ImplScale( const double& fX, const double& fY, sal_uInt32 nMode = PS_RET );

    void				ImplWriteLine( const Polygon & rPolygon );
    void				ImplAddPath( const Polygon & rPolygon );
    void				ImplWriteLineInfo( double fLineWidth, double fMiterLimit, SvtGraphicStroke::CapType eLineCap,
                                    SvtGraphicStroke::JoinType eJoinType, SvtGraphicStroke::DashArray& rDashArray );
    void				ImplWriteLineInfo( const LineInfo& rLineInfo );
    void				ImplRect( const Rectangle & rRectangle );
    void				ImplRectFill ( const Rectangle & rRectangle );
    void				ImplWriteGradient( const PolyPolygon& rPolyPoly, const Gradient& rGradient, VirtualDevice& rVDev );
    void				ImplIntersect( const PolyPolygon& rPolyPoly );
    void				ImplPolyPoly( const PolyPolygon & rPolyPolygon, sal_Bool bTextOutline = sal_False );
    void				ImplPolyLine( const Polygon & rPolygon );
    
    void				ImplSetClipRegion( Region& rRegion );
    void				ImplBmp( Bitmap*, Bitmap*, const Point &, double nWidth, double nHeight );
    void				ImplText( const String& rUniString, const Point& rPos, const INT32* pDXArry, sal_Int32 nWidth, VirtualDevice& rVDev );
    void				ImplSetAttrForText( const Point & rPoint );
    void				ImplWriteCharacter( sal_Char );
    void				ImplWriteString( const ByteString&, VirtualDevice& rVDev, const INT32* pDXArry = NULL, BOOL bStretch = FALSE );
    void				ImplDefineFont( const char*, const char* );

    void				ImplClosePathDraw( ULONG nMode = PS_RET );
    void				ImplPathDraw();

    inline void 		ImplWriteLineColor( ULONG nMode = PS_RET );
    inline void 		ImplWriteFillColor( ULONG nMode = PS_RET );
    inline void 		ImplWriteTextColor( ULONG nMode = PS_RET );
    inline void 		ImplWriteTextFillColor( ULONG nMode = PS_RET );
    void				ImplWriteColor( ULONG nMode );

    double				ImplGetScaling( const MapMode& );
    void				ImplGetMapMode( const MapMode& );
    BOOL				ImplGetBoundingBox( double* nNumb, BYTE* pSource, ULONG nSize );
    BYTE*				ImplSearchEntry( BYTE* pSource, BYTE* pDest, ULONG nComp, ULONG nSize );
                        // LZW methods
    void				StartCompression();
    void				Compress( BYTE nSrc );
    void				EndCompression();
    inline void 		WriteBits( USHORT nCode, USHORT nCodeLen );

public:
    BOOL				WritePS( const Graphic& rGraphic, SvStream& rTargetStream, FilterConfigItem* );
    PSWriter();
    ~PSWriter();
};

//========================== Methoden von PSWriter ==========================

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

BOOL PSWriter::WritePS( const Graphic& rGraphic, SvStream& rTargetStream, FilterConfigItem* pFilterConfigItem )
{
    UINT32 nStreamPosition = 0, nPSPosition = 0; // -Wall warning, unset, check

    mbStatus = TRUE;
    mnPreview = 0;
    mnLevelWarning = 0;
    mnLastPercent = 0;
    mnLatestPush = 0xEFFFFFFE;

    if ( pFilterConfigItem )
    {
        xStatusIndicator = pFilterConfigItem->GetStatusIndicator();
        if ( xStatusIndicator.is() )
        {
            rtl::OUString aMsg;
            xStatusIndicator->start( aMsg, 100 );
        }
    }

    mpPS = &rTargetStream;
    mpPS->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

    // default values for the dialog options
    mnLevel = 2;
    mbGrayScale = FALSE;
    mbCompression = TRUE;
    mnTextMode = 0;			// default0 : export glyph outlines

    // try to get the dialog selection
    if ( pFilterConfigItem )
    {
        ByteString	aResMgrName( "eps" );
        ResMgr*		pResMgr;

        pResMgr = ResMgr::CreateResMgr( aResMgrName.GetBuffer(), Application::GetSettings().GetUILocale() );

        if( pResMgr )
        {
            String aPreviewStr( RTL_CONSTASCII_USTRINGPARAM( "Preview" ) );
            String aVersionStr( RTL_CONSTASCII_USTRINGPARAM( "Version" ) );
            String aColorStr( RTL_CONSTASCII_USTRINGPARAM( "ColorFormat" ) );   
            String aComprStr( RTL_CONSTASCII_USTRINGPARAM( "CompressionMode" ) );
            mnPreview = pFilterConfigItem->ReadInt32( aPreviewStr, 1 );
            mnLevel = pFilterConfigItem->ReadInt32( aVersionStr, 2 );
            if ( mnLevel != 1 )
                mnLevel = 2;
            mbGrayScale = pFilterConfigItem->ReadInt32( aColorStr, 1 ) == 2;
            mbCompression = pFilterConfigItem->ReadInt32( aComprStr, 1 ) == 1;	
            String sTextMode( RTL_CONSTASCII_USTRINGPARAM( "TextMode" ) );
            mnTextMode = pFilterConfigItem->ReadInt32( sTextMode, 0 );
            if ( mnTextMode > 2 )
                mnTextMode = 0;
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
            rTargetStream << nPSPosition - 30;	// size of tiff gfx
            rTargetStream.Seek( nPSPosition );
        }
        else
        {
            mnPreview &=~ EPS_PREVIEW_TIFF;
            rTargetStream.Seek( nStreamPosition - 4 );
        }
    }

    // global default value setting
    ChrSet* 		pCS;
    StackMember*	pGS;

    if ( rGraphic.GetType() == GRAPHIC_GDIMETAFILE )
        pMTF = &rGraphic.GetGDIMetaFile();
    else
        pMTF = pAMTF = new GDIMetaFile( rGraphic.GetGDIMetaFile() );
    aVDev.SetMapMode( pMTF->GetPrefMapMode() );
    nBoundingX1 = nBoundingY1 = 0;
    nBoundingX2 = pMTF->GetPrefSize().Width();
    nBoundingY2 = pMTF->GetPrefSize().Height();

    pGDIStack = NULL;
    aColor = Color( COL_TRANSPARENT );
    bLineColor = TRUE;
    aLineColor = Color( COL_BLACK );
    bFillColor = TRUE;
    aFillColor = Color( COL_WHITE );
    bTextFillColor = TRUE;
    aTextFillColor = Color( COL_BLACK );
    fLineWidth = 1;
    fMiterLimit = 15; // use same limit as most graphic systems and basegfx
    eLineCap = SvtGraphicStroke::capButt;
    eJoinType = SvtGraphicStroke::joinMiter;
    aBackgroundColor = Color( COL_WHITE );
    eTextAlign = ALIGN_BASELINE;
    bRegionChanged = FALSE;

    nChrSet = 0x00;
    pChrSetList = NULL;
    nNextChrSetId = 1;

    if( pMTF->GetActionCount() )
    {
        ImplWriteProlog( ( mnPreview & EPS_PREVIEW_EPSI ) ? &rGraphic : NULL );
        mnCursorPos = 0;
        ImplWriteActions( *pMTF, aVDev );
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

    if ( mbStatus && mnLevelWarning && pFilterConfigItem )
    {
        ByteString	aResMgrName( "eps" );
        ResMgr* pResMgr;
        pResMgr = ResMgr::CreateResMgr( aResMgrName.GetBuffer(), Application::GetSettings().GetUILocale() );
        if( pResMgr )
        {
            InfoBox aInfoBox( NULL, String( ResId( KEY_VERSION_CHECK, *pResMgr ) ) );
            aInfoBox.Execute();
            delete pResMgr;
        }
    }

    if ( xStatusIndicator.is() )
        xStatusIndicator->end();

    return mbStatus;
}

//---------------------------------------------------------------------------------

void PSWriter::ImplWriteProlog( const Graphic* pPreview )
{
    ImplWriteLine( "%!PS-Adobe-3.0 EPSF-3.0 " );
    *mpPS << "%%BoundingBox: ";                         // BoundingBox
    ImplWriteLong( 0 );
    ImplWriteLong( 0 );
    Size aSizePoint = Application::GetDefaultDevice()->LogicToLogic( pMTF->GetPrefSize(),
                        pMTF->GetPrefMapMode(), MAP_POINT );
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
            *mpPS << "%%BeginPreview: ";					// BoundingBox
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
    ImplWriteLine( "%%BeginResource: procset SDRes-Prolog 1.0 0" );

//	BEGIN EPSF
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
    ImplWriteLine( "/lc {setlinecap} bdef" );
    ImplWriteLine( "/lj {setlinejoin} bdef" );
    ImplWriteLine( "/lw {setlinewidth} bdef" );
    ImplWriteLine( "/ml {setmiterlimit} bdef" );
    ImplWriteLine( "/ld {setdash} bdef" );
    ImplWriteLine( "/m {neg moveto} bdef" );
    ImplWriteLine( "/ct {6 2 roll neg 6 2 roll neg 6 2 roll neg curveto} bdef" );
    ImplWriteLine( "/r {rotate} bdef" );
    ImplWriteLine( "/t {neg translate} bdef" );
    ImplWriteLine( "/s {scale} bdef" );
    ImplWriteLine( "/sw {show} bdef" );
    ImplWriteLine( "/gs {gsave} bdef" );
    ImplWriteLine( "/gr {grestore} bdef" );

    ImplWriteLine( "/f {findfont dup length dict begin" );  // Setfont
    ImplWriteLine( "{1 index /FID ne {def} {pop pop} ifelse} forall /Encoding ISOLatin1Encoding def" );
    ImplWriteLine( "currentdict end /NFont exch definefont pop /NFont findfont} bdef" );

    ImplWriteLine( "/p {closepath} bdef" );
    ImplWriteLine( "/sf {scalefont setfont} bdef" );

    ImplWriteLine( "/ef {eofill}bdef"			);      // close path and fill
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
    ImplScale( (double)aSizePoint.Width() / (double)pMTF->GetPrefSize().Width(), (double)aSizePoint.Height() / (double)pMTF->GetPrefSize().Height() );
    ImplWriteDouble( 0 );
    ImplWriteDouble( -pMTF->GetPrefSize().Height() );
    ImplWriteLine( "t" );
    ImplWriteLine( "/tm matrix currentmatrix def" );
}

//---------------------------------------------------------------------------------

void PSWriter::ImplWriteEpilog()
{
    ImplTranslate( 0, nBoundingY2 );
    ImplWriteLine( "pom" );
    ImplWriteLine( "count op_count sub {pop} repeat countdictstack dict_count sub {end} repeat b4_inc_state restore" );

    ImplWriteLine( "%%PageTrailer" );
    ImplWriteLine( "%%Trailer" );

    ImplWriteLine( "%%EOF" );
}

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------

void PSWriter::ImplWriteActions( const GDIMetaFile& rMtf, VirtualDevice& rVDev )
{
    PolyPolygon aFillPath;

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
                ImplWriteLineInfo( rLineInfo );
                if ( bLineColor )
                {
                    ImplWriteLineColor( PS_SPACE );
                    ImplMoveTo( ( (const MetaLineAction*) pMA )->GetStartPoint() );
                    ImplLineTo( ( (const MetaLineAction*) pMA )->GetEndPoint() );
                    ImplPathDraw();
                }
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
                Rectangle	aRect = ( ( (const MetaEllipseAction*) pMA )->GetRect() );
                Point		aCenter = aRect.Center();
                Polygon		aPoly( aCenter, aRect.GetWidth() / 2, aRect.GetHeight() / 2 );
                PolyPolygon aPolyPoly( aPoly );
                ImplPolyPoly( aPolyPoly );
            }
            break;

            case META_ARC_ACTION :
            {
                Polygon aPoly( ( (const MetaArcAction*)pMA )->GetRect(), ( (const MetaArcAction*)pMA )->GetStartPoint(),
                    ( (const MetaArcAction*)pMA )->GetEndPoint(), POLY_ARC );
                PolyPolygon	aPolyPoly( aPoly );
                ImplPolyPoly( aPolyPoly );
            }
            break;

            case META_PIE_ACTION :
            {
                Polygon aPoly( ( (const MetaPieAction*)pMA )->GetRect(), ( (const MetaPieAction*)pMA )->GetStartPoint(),
                    ( (const MetaPieAction*)pMA )->GetEndPoint(), POLY_PIE );
                PolyPolygon aPolyPoly( aPoly );
                ImplPolyPoly( aPolyPoly );
            }
            break;

            case META_CHORD_ACTION :
            {
                Polygon aPoly( ( (const MetaChordAction*)pMA )->GetRect(), ( (const MetaChordAction*)pMA )->GetStartPoint(),
                    ( (const MetaChordAction*)pMA )->GetEndPoint(), POLY_CHORD );
                PolyPolygon aPolyPoly( aPoly );
                ImplPolyPoly( aPolyPoly );
            }
            break;

            case META_POLYLINE_ACTION :
            {
                Polygon aPoly( ( (const MetaPolyLineAction*) pMA )->GetPolygon() );
                const LineInfo& rLineInfo = ( ( const MetaPolyLineAction*)pMA )->GetLineInfo();
                ImplWriteLineInfo( rLineInfo );
                
                if(basegfx::B2DLINEJOIN_NONE == rLineInfo.GetLineJoin()
                    && rLineInfo.GetWidth() > 1)
                {
                    // emulate B2DLINEJOIN_NONE by creating single edges
                    const sal_uInt16 nPoints(aPoly.GetSize());
                    const bool bCurve(aPoly.HasFlags());

                    for(sal_uInt16 a(0); a + 1 < nPoints; a++)
                    {
                        if(bCurve 
                            && POLY_NORMAL != aPoly.GetFlags(a + 1)
                            && a + 2 < nPoints
                            && POLY_NORMAL != aPoly.GetFlags(a + 2)
                            && a + 3 < nPoints)
                        {
                            const Polygon aSnippet(4, 
                                aPoly.GetConstPointAry() + a, 
                                aPoly.GetConstFlagAry() + a);
                            ImplPolyLine(aSnippet);
                            a += 2;
                        }
                        else
                        {
                            const Polygon aSnippet(2, 
                                aPoly.GetConstPointAry() + a);
                            ImplPolyLine(aSnippet);
                        }
                    }
                }
                else
                {
                    ImplPolyLine( aPoly );
                }
            }
            break;

            case META_POLYGON_ACTION :
            {
                PolyPolygon aPolyPoly( ( (const MetaPolygonAction*) pMA )->GetPolygon() );
                ImplPolyPoly( aPolyPoly );
            }
            break;

            case META_POLYPOLYGON_ACTION :
            {
                ImplPolyPoly( ( (const MetaPolyPolygonAction*) pMA )->GetPolyPolygon() );
            }
            break;

            case META_TEXT_ACTION:
            {
                const MetaTextAction * pA = (const MetaTextAction*) pMA;

                String	aUniStr( pA->GetText(), pA->GetIndex(), pA->GetLen() );
                Point	aPoint( pA->GetPoint() );

                ImplText( aUniStr, aPoint, NULL, 0, rVDev );
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
                String	aUniStr( pA->GetText(), pA->GetIndex(), pA->GetLen() );
                Point	aPoint( pA->GetPoint() );

                ImplText( aUniStr, aPoint, NULL, pA->GetWidth(), rVDev );
            }
            break;

            case META_TEXTARRAY_ACTION:
            {
                const MetaTextArrayAction* pA = (const MetaTextArrayAction*)pMA;
                String	aUniStr( pA->GetText(), pA->GetIndex(), pA->GetLen() );
                Point	aPoint( pA->GetPoint() );

                ImplText( aUniStr, aPoint, pA->GetDXArray(), 0, rVDev );
            }
            break;

            case META_BMP_ACTION :
            {
                Bitmap aBitmap = ( (const MetaBmpAction*)pMA )->GetBitmap();
                if ( mbGrayScale )
                    aBitmap.Convert( BMP_CONVERSION_8BIT_GREYS );
                Point aPoint = ( (const MetaBmpAction*) pMA )->GetPoint();
                Size aSize = aBitmap.GetSizePixel();
                ImplBmp( &aBitmap, NULL, aPoint, aSize.Width(), aSize.Height() );
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
                Bitmap	aBitmap( ( (const MetaBmpScalePartAction*)pMA )->GetBitmap() );
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
                BitmapEx	aBitmapEx( ( (const MetaBmpExScalePartAction*)pMA )->GetBitmapEx() );
                aBitmapEx.Crop( Rectangle( ( (const MetaBmpExScalePartAction*)pMA )->GetSrcPoint(),
                    ( (const MetaBmpExScalePartAction*)pMA )->GetSrcSize() ) );
                Bitmap		aBitmap( aBitmapEx.GetBitmap() );
                if ( mbGrayScale )
                    aBitmap.Convert( BMP_CONVERSION_8BIT_GREYS );
                Bitmap		aMask( aBitmapEx.GetMask() );
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
                PolyPolygon aPolyPoly( ( (const MetaGradientAction*)pMA)->GetRect() );
                ImplWriteGradient( aPolyPoly, ( (const MetaGradientAction*) pMA )->GetGradient(), rVDev );
            }
            break;

            case META_GRADIENTEX_ACTION :
            {
                PolyPolygon aPolyPoly( ( (const MetaGradientExAction*)pMA)->GetPolyPolygon() );
                ImplWriteGradient( aPolyPoly, ( (const MetaGradientExAction*) pMA )->GetGradient(), rVDev );
            }
            break;

            case META_HATCH_ACTION :
            {
                VirtualDevice	l_aVDev;
                GDIMetaFile 	aTmpMtf;
                
                l_aVDev.SetMapMode( rVDev.GetMapMode() );
                l_aVDev.AddHatchActions( ( (const MetaHatchAction*)pMA)->GetPolyPolygon(), 
                                         ( (const MetaHatchAction*)pMA )->GetHatch(), aTmpMtf );
                ImplWriteActions( aTmpMtf, rVDev );
            }
            break;

            case META_WALLPAPER_ACTION :
            {
                const MetaWallpaperAction* pA = (const MetaWallpaperAction*)pMA;
                Rectangle	aRect = pA->GetRect();
                Wallpaper	aWallpaper = pA->GetWallpaper();

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
                Region aRegion( pA->GetRect() );
                ImplSetClipRegion( aRegion );
            }
            break;

            case META_CLIPREGION_ACTION:
            {
                const MetaClipRegionAction* pA = (const MetaClipRegionAction*) pMA;
                Region aRegion( pA->GetRegion() );
                ImplSetClipRegion( aRegion );
            }
            break;

            case META_ISECTREGIONCLIPREGION_ACTION:
            {
                const MetaISectRegionClipRegionAction* pA = (const MetaISectRegionClipRegionAction*) pMA;
                Region aRegion( pA->GetRegion() );
                ImplSetClipRegion( aRegion );
            }
            break;

            case META_MOVECLIPREGION_ACTION:
            {
/*
                if ( !aClipRegion.IsEmpty() )
                {
                    const MetaMoveClipRegionAction* pA = (const MetaMoveClipRegionAction*) pMA;
                    aClipRegion.Move( pA->GetHorzMove(), pA->GetVertMove() );
                    ImplSetClipRegion();
                }
*/
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
                pMA->Execute( &rVDev );
                ImplGetMapMode( rVDev.GetMapMode() );
            }
            break;

            case META_FONT_ACTION :
            {
                maFont = ((const MetaFontAction*)pMA)->GetFont();
                rVDev.SetFont( maFont );
            }
            break;

            case META_PUSH_ACTION :
            {
                rVDev.Push(((const MetaPushAction*)pMA)->GetFlags() );
                StackMember* pGS = new StackMember;
                pGS->pSucc = pGDIStack;
                pGDIStack = pGS;
                pGS->aDashArray = aDashArray;
                pGS->eJoinType = eJoinType;
                pGS->eLineCap = eLineCap;
                pGS->fLineWidth = fLineWidth;
                pGS->fMiterLimit = fMiterLimit;
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
                bRegionChanged = FALSE;
                pGS->aFont = maFont;
                mnLatestPush = mpPS->Tell();
                ImplWriteLine( "gs" );
            }
            break;

            case META_POP_ACTION :
            {
                rVDev.Pop();
                StackMember* pGS;
                if( pGDIStack )
                {
                    pGS = pGDIStack;
                    pGDIStack = pGS->pSucc;
                    aDashArray = pGS->aDashArray;
                    eJoinType = pGS->eJoinType;
                    eLineCap = pGS->eLineCap;
                    fLineWidth = pGS->fLineWidth;
                    fMiterLimit = pGS->fMiterLimit;
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
                    maFont = pGS->aFont;
                    maLastFont = Font();				// set maLastFont != maFont -> so that
                    delete pGS;
                    sal_uInt32 nCurrentPos = mpPS->Tell();
                    if ( nCurrentPos - 3 == mnLatestPush )
                    {
                        mpPS->Seek( mnLatestPush );
                        ImplWriteLine( "  " );
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
                const GDIMetaFile aSubstitute( ( ( const MetaEPSAction*) pMA )->GetSubstitute() );

                BOOL	bLevelConflict = FALSE;
                BYTE*	pSource = (BYTE*) aGfxLink.GetData();
                ULONG	nSize = aGfxLink.GetDataSize();
                ULONG	nParseThis = POSTSCRIPT_BOUNDINGSEARCH;
                if ( nSize < 64 )						// assuming eps is larger than 64 bytes
                    pSource = NULL;
                if ( nParseThis > nSize )
                    nParseThis = nSize;

                if ( pSource && ( mnLevel == 1 ) )
                {
                    BYTE* pFound = ImplSearchEntry( pSource, (BYTE*)"%%LanguageLevel:", nParseThis - 10, 16 );
                    if ( pFound )
                    {
                        BYTE	k, i = 10;
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
                    double	nBoundingBox[4];
                    if ( pSource && ImplGetBoundingBox( nBoundingBox, pSource, nParseThis ) )
                    {
                        Point	aPoint = ( (const MetaEPSAction*) pMA )->GetPoint();
                        Size	aSize = ( (const MetaEPSAction*) pMA )->GetSize();

                        MapMode	aMapMode( aSubstitute.GetPrefMapMode() );
                        Size aOutSize( rVDev.LogicToLogic( aSize, rVDev.GetMapMode(), aMapMode ) );
                        Point aOrigin( rVDev.LogicToLogic( aPoint, rVDev.GetMapMode(), aMapMode ) );
                        aOrigin.Y() += aOutSize.Height();
                        aMapMode.SetOrigin( aOrigin );
                        aMapMode.SetScaleX( aOutSize.Width() / ( nBoundingBox[ 2 ] - nBoundingBox[ 0 ] ) );
                        aMapMode.SetScaleY( aOutSize.Height() / ( nBoundingBox[ 3 ] - nBoundingBox[ 1 ] ) );
                        ImplWriteLine( "gs" );
                        ImplGetMapMode( aMapMode );
                        ImplWriteLine( "%%BeginDocument:" );
                        mpPS->Write( pSource, aGfxLink.GetDataSize() );
                        ImplWriteLine( "%%EndDocument\ngr" );
                    }
                }
            }
            break;

            case META_TRANSPARENT_ACTION:
            {
//				ImplLine( ( (const MetaTransparentAction*) pMA )->GetPolyPolygon() );
            }
            break;

            case META_RASTEROP_ACTION:
            {
                pMA->Execute( &rVDev );
            }
            break;

            case META_FLOATTRANSPARENT_ACTION:
            {
                const MetaFloatTransparentAction* pA = (const MetaFloatTransparentAction*) pMA;
                
                GDIMetaFile		aTmpMtf( pA->GetGDIMetaFile() );
                Point			aSrcPt( aTmpMtf.GetPrefMapMode().GetOrigin() );
                const Size		aSrcSize( aTmpMtf.GetPrefSize() );
                const Point		aDestPt( pA->GetPoint() );
                const Size		aDestSize( pA->GetSize() );
                const double	fScaleX = aSrcSize.Width() ? (double) aDestSize.Width() / aSrcSize.Width() : 1.0;
                const double	fScaleY = aSrcSize.Height() ? (double) aDestSize.Height() / aSrcSize.Height() : 1.0;
                long			nMoveX, nMoveY;

                if( fScaleX != 1.0 || fScaleY != 1.0 )
                {
                    aTmpMtf.Scale( fScaleX, fScaleY );
                    aSrcPt.X() = FRound( aSrcPt.X() * fScaleX ), aSrcPt.Y() = FRound( aSrcPt.Y() * fScaleY );
                }

                nMoveX = aDestPt.X() - aSrcPt.X(), nMoveY = aDestPt.Y() - aSrcPt.Y();

                if( nMoveX || nMoveY )
                    aTmpMtf.Move( nMoveX, nMoveY );

                ImplWriteActions( aTmpMtf, rVDev );
            }
            break;

            case META_COMMENT_ACTION:
            {									
                const MetaCommentAction* pA = (const MetaCommentAction*) pMA;
                if ( pA->GetComment().CompareIgnoreCaseToAscii( "XGRAD_SEQ_BEGIN" ) == COMPARE_EQUAL )
                {
                    const MetaGradientExAction*	pGradAction = NULL;
                    while( ++nCurAction < nCount )
                    {
                        MetaAction* pAction = rMtf.GetAction( nCurAction );
                        if( pAction->GetType() == META_GRADIENTEX_ACTION )
                            pGradAction = (const MetaGradientExAction*) pAction;
                        else if( ( pAction->GetType() == META_COMMENT_ACTION ) && 
                                 ( ( (const MetaCommentAction*) pAction )->GetComment().CompareIgnoreCaseToAscii( "XGRAD_SEQ_END" ) == COMPARE_EQUAL ) )
                        {
                            break;
                        }
                    }
                    if( pGradAction )
                        ImplWriteGradient( pGradAction->GetPolyPolygon(), pGradAction->GetGradient(), rVDev );				
                }
                else if ( pA->GetComment().Equals( "XPATHFILL_SEQ_END" ) )
                {
                    if ( aFillPath.Count() )
                    {
                        aFillPath = PolyPolygon();
                        ImplWriteLine( "gr" );
                    }
                }
                else
                {
                    const BYTE* pData = pA->GetData();
                    if ( pData )
                    {
                        SvMemoryStream	aMemStm( (void*)pData, pA->GetDataSize(), STREAM_READ );
                        sal_Bool		bSkipSequence = sal_False;					
                        ByteString		sSeqEnd;

                        if( pA->GetComment().Equals( "XPATHSTROKE_SEQ_BEGIN" ) )
                        {
                            sSeqEnd = ByteString( "XPATHSTROKE_SEQ_END" );
                            SvtGraphicStroke aStroke;
                            aMemStm >> aStroke;

                            Polygon aPath;
                            aStroke.getPath( aPath );

                            PolyPolygon aStartArrow;
                            PolyPolygon aEndArrow;
//							double fTransparency( aStroke.getTransparency() );
                            double fStrokeWidth( aStroke.getStrokeWidth() );
                            SvtGraphicStroke::JoinType eJT( aStroke.getJoinType() );
                            SvtGraphicStroke::DashArray l_aDashArray;

                            aStroke.getStartArrow( aStartArrow );
                            aStroke.getEndArrow( aEndArrow );
                            aStroke.getDashArray( l_aDashArray );

                            bSkipSequence = sal_True;
                            if ( l_aDashArray.size() > 11 )	// ps dasharray limit is 11
                                bSkipSequence = sal_False;
                            if ( aStartArrow.Count() || aEndArrow.Count() )
                                bSkipSequence = sal_False;
                            if ( (sal_uInt32)eJT > 2 )
                                bSkipSequence = sal_False;
                            if ( l_aDashArray.size() && ( fStrokeWidth != 0.0 ) )
                                bSkipSequence = sal_False;
                            if ( bSkipSequence )
                            {
                                ImplWriteLineInfo( fStrokeWidth, aStroke.getMiterLimit(),
                                                    aStroke.getCapType(), eJT, l_aDashArray );
                                ImplPolyLine( aPath );
                            }
                        }
                        else if( pA->GetComment().Equals( "XPATHFILL_SEQ_BEGIN" ) )
                        {
                            sSeqEnd = ByteString( "XPATHFILL_SEQ_END" );
                            SvtGraphicFill aFill;
                            aMemStm >> aFill;
                            switch( aFill.getFillType() )
                            {
                                case SvtGraphicFill::fillSolid :
                                {
                                    bSkipSequence = sal_True;
                                    PolyPolygon aPolyPoly;
                                    aFill.getPath( aPolyPoly );
                                    sal_uInt16 i, nPolyCount = aPolyPoly.Count();
                                    if ( nPolyCount )
                                    {
                                        aFillColor = aFill.getFillColor();
                                        ImplWriteFillColor( PS_SPACE );
                                        for ( i = 0; i < nPolyCount; )
                                        {
                                            ImplAddPath( aPolyPoly.GetObject( i ) );
                                            if ( ++i < nPolyCount )
                                            {
                                                *mpPS << "p";
                                                mnCursorPos += 2;
                                                ImplExecMode( PS_RET );
                                            }
                                        }
                                        *mpPS << "p ef";
                                        mnCursorPos += 4;
                                        ImplExecMode( PS_RET );
                                    }
                                }
                                break;

                                case SvtGraphicFill::fillTexture :
                                {
                                    aFill.getPath( aFillPath );

                                    /* normally an object filling is consisting of three MetaActions:
                                        MetaBitmapAction		using RasterOp xor,
                                        MetaPolyPolygonAction	using RasterOp rop_0
                                        MetaBitmapAction		using RasterOp xor

                                        Because RasterOps cannot been used in Postscript, we have to
                                        replace these actions. The MetaComment "XPATHFILL_SEQ_BEGIN" is 
                                        providing the clippath of the object. The following loop is 
                                        trying to find the bitmap that is matching the clippath, so that
                                        only one bitmap is exported, otherwise if the bitmap is not
                                        locatable, all metaactions are played normally.
                                    */
                                    sal_uInt32 nCommentStartAction = nCurAction;
                                    sal_uInt32 nBitmapCount = 0;
                                    sal_uInt32 nBitmapAction = 0;

                                    sal_Bool bOk = sal_True;
                                    while( bOk && ( ++nCurAction < nCount ) )
                                    {
                                        MetaAction* pAction = rMtf.GetAction( nCurAction );
                                        switch( pAction->GetType() )
                                        {
                                            case META_BMPSCALE_ACTION :
                                            case META_BMPSCALEPART_ACTION :
                                            case META_BMPEXSCALE_ACTION :
                                            case META_BMPEXSCALEPART_ACTION :
                                            {
                                                nBitmapCount++;
                                                nBitmapAction = nCurAction;
                                            }
                                            break;
                                            case META_COMMENT_ACTION :
                                            {
                                                if (((const MetaCommentAction*)pAction)->GetComment().Equals( "XPATHFILL_SEQ_END" ))
                                                    bOk = sal_False;
                                            }
                                            break;
                                        }
                                    }
                                    if( nBitmapCount == 2 )
                                    {
                                        ImplWriteLine( "gs" );
                                        ImplIntersect( aFillPath );
                                        GDIMetaFile aTempMtf;
                                        aTempMtf.AddAction( rMtf.GetAction( nBitmapAction )->Clone() );
                                        ImplWriteActions( aTempMtf, rVDev );
                                        ImplWriteLine( "gr" );
                                        aFillPath = PolyPolygon();
                                    }
                                    else
                                        nCurAction = nCommentStartAction + 1;
                                }
                                break;

                                case SvtGraphicFill::fillGradient :
                                    aFill.getPath( aFillPath );
                                break;

                                case SvtGraphicFill::fillHatch :
                                break;
                            }
                            if ( aFillPath.Count() )
                            {
                                ImplWriteLine( "gs" );
                                ImplIntersect( aFillPath );
                            }
                        }
                        if ( bSkipSequence )
                        {
                            while( ++nCurAction < nCount )
                            {
                                pMA = rMtf.GetAction( nCurAction );
                                if ( pMA->GetType() == META_COMMENT_ACTION )
                                {
                                    ByteString sComment( ((MetaCommentAction*)pMA)->GetComment() );	
                                    if ( sComment.Equals( sSeqEnd ) )
                                        break;
                                }
                            }
                        }
                    }
                }
            }
            break;
        }
    }
}



//---------------------------------------------------------------------------------

inline void PSWriter::ImplWritePoint( const Point& rPoint, sal_uInt32 nMode )
{
    ImplWriteDouble( rPoint.X() );
    ImplWriteDouble( rPoint.Y(), nMode );
}

//---------------------------------------------------------------------------------

void PSWriter::ImplMoveTo( const Point& rPoint, sal_uInt32 nMode )
{
    ImplWritePoint( rPoint );
    ImplWriteByte( 'm' );
    ImplExecMode( nMode );
}

//---------------------------------------------------------------------------------

void PSWriter::ImplLineTo( const Point& rPoint, sal_uInt32 nMode )
{
    ImplWritePoint( rPoint );
    ImplWriteByte( 'l' );
    ImplExecMode( nMode );
}

//---------------------------------------------------------------------------------

void PSWriter::ImplCurveTo( const Point& rP1, const Point& rP2, const Point& rP3, sal_uInt32 nMode )
{
    ImplWritePoint( rP1 );
    ImplWritePoint( rP2 );
    ImplWritePoint( rP3 );
    *mpPS << "ct ";
    ImplExecMode( nMode );
}

//---------------------------------------------------------------------------------

void PSWriter::ImplTranslate( const double& fX, const double& fY, sal_uInt32 nMode )
{
    ImplWriteDouble( fX );
    ImplWriteDouble( fY );
    ImplWriteByte( 't' );
    ImplExecMode( nMode );
}

//---------------------------------------------------------------------------------

void PSWriter::ImplScale( const double& fX, const double& fY, sal_uInt32 nMode )
{
    ImplWriteDouble( fX );
    ImplWriteDouble( fY );
    ImplWriteByte( 's' );
    ImplExecMode( nMode );
}

//---------------------------------------------------------------------------------

void PSWriter::ImplRect( const Rectangle & rRect )
{
    if ( bFillColor )
        ImplRectFill( rRect );
    if ( bLineColor )
    {
        double nWidth = rRect.GetWidth();
        double nHeight = rRect.GetHeight();

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
    double nWidth = rRect.GetWidth();
    double nHeight = rRect.GetHeight();

    ImplWriteFillColor( PS_SPACE );
    ImplMoveTo( rRect.TopLeft() );
    ImplWriteDouble( nWidth );
    *mpPS << "0 rl 0 ";
    ImplWriteDouble( nHeight );
    *mpPS << "rl ";
    ImplWriteDouble( nWidth );
    *mpPS << "neg 0 rl ef ";
    *mpPS << "p ef";
    mnCursorPos += 2;
    ImplExecMode( PS_RET );
}

//---------------------------------------------------------------------------------

void PSWriter::ImplAddPath( const Polygon & rPolygon )
{
    USHORT i = 1;
    USHORT nPointCount = rPolygon.GetSize();
    if ( nPointCount > 1 )
    {
        ImplMoveTo( rPolygon.GetPoint( 0 ) );
        while ( i < nPointCount )
        {
            if ( ( rPolygon.GetFlags( i ) == POLY_CONTROL )
                    && ( ( i + 2 ) < nPointCount )
                        && ( rPolygon.GetFlags( i + 1 ) == POLY_CONTROL )
                            && ( rPolygon.GetFlags( i + 2 ) != POLY_CONTROL ) )
            {
                ImplCurveTo( rPolygon[ i ], rPolygon[ i + 1 ], rPolygon[ i + 2 ], PS_WRAP );
                i += 3;
            }
            else
                ImplLineTo( rPolygon.GetPoint( i++ ), PS_SPACE | PS_WRAP );
        }
    }
}

//---------------------------------------------------------------------------------

void PSWriter::ImplIntersect( const PolyPolygon& rPolyPoly )
{
    sal_uInt16 i, nPolyCount = rPolyPoly.Count();
    for ( i = 0; i < nPolyCount; )
    {
        ImplAddPath( rPolyPoly.GetObject( i ) );
        if ( ++i < nPolyCount )
        {
            *mpPS << "p";
            mnCursorPos += 2;
            ImplExecMode( PS_RET );
        }
    }
    ImplWriteLine( "eoclip newpath" );
}

//---------------------------------------------------------------------------------

void PSWriter::ImplWriteGradient( const PolyPolygon& rPolyPoly, const Gradient& rGradient, VirtualDevice& rVDev )
{
    VirtualDevice	l_aVDev;
    GDIMetaFile 	aTmpMtf;
    l_aVDev.SetMapMode( rVDev.GetMapMode() );
    l_aVDev.AddGradientActions( rPolyPoly.GetBoundRect(), rGradient, aTmpMtf );
    ImplWriteActions( aTmpMtf, rVDev );
}

//---------------------------------------------------------------------------------

void PSWriter::ImplPolyPoly( const PolyPolygon & rPolyPoly, sal_Bool bTextOutline )
{
    sal_uInt16 i, nPolyCount = rPolyPoly.Count();
    if ( nPolyCount )
    {
        if ( bFillColor || bTextOutline )
        {
            if ( bTextOutline )
                ImplWriteTextColor( PS_SPACE );
            else
                ImplWriteFillColor( PS_SPACE );
            for ( i = 0; i < nPolyCount; )
            {
                ImplAddPath( rPolyPoly.GetObject( i ) );
                if ( ++i < nPolyCount )
                {
                    *mpPS << "p";
                    mnCursorPos += 2;
                    ImplExecMode( PS_RET );
                }
            }
            *mpPS << "p ef";
            mnCursorPos += 4;
            ImplExecMode( PS_RET );
        }
        if ( bLineColor )
        {
            ImplWriteLineColor( PS_SPACE );		
            for ( i = 0; i < nPolyCount; i++ )
                ImplAddPath( rPolyPoly.GetObject( i ) );
            ImplClosePathDraw( PS_RET );
        }
    }
}

//---------------------------------------------------------------------------------

void PSWriter::ImplPolyLine( const Polygon & rPoly )
{
    if ( bLineColor )
    {
        ImplWriteLineColor( PS_SPACE );
        sal_uInt16 i, nPointCount = rPoly.GetSize();
        if ( nPointCount )
        {
            if ( nPointCount > 1 )
            {							
                ImplMoveTo( rPoly.GetPoint( 0 ) );
                i = 1;
                while ( i < nPointCount )
                {
                    if ( ( rPoly.GetFlags( i ) == POLY_CONTROL )
                            && ( ( i + 2 ) < nPointCount )
                                && ( rPoly.GetFlags( i + 1 ) == POLY_CONTROL )
                                    && ( rPoly.GetFlags( i + 2 ) != POLY_CONTROL ) )
                    {
                        ImplCurveTo( rPoly[ i ], rPoly[ i + 1 ], rPoly[ i + 2 ], PS_WRAP );
                        i += 3;
                    }
                    else
                        ImplLineTo( rPoly.GetPoint( i++ ), PS_SPACE | PS_WRAP );
                }
            }
            
            // #104645# explicitely close path if polygon is closed
            if( rPoly[ 0 ] == rPoly[ nPointCount-1 ] )
                ImplClosePathDraw( PS_RET );
            else
                ImplPathDraw();
        }
    }
}

//---------------------------------------------------------------------------------

void PSWriter::ImplSetClipRegion( Region& rClipRegion )
{
    if ( !rClipRegion.IsEmpty() )
    {
        Rectangle		aRect;
        RegionHandle	hRegionHandle = rClipRegion.BeginEnumRects();

        while ( rClipRegion.GetNextEnumRect( hRegionHandle, aRect ) )
        {
            double nX1 = aRect.Left();
            double nY1 = aRect.Top();
            double nX2 = aRect.Right();
            double nY2 = aRect.Bottom();
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
        rClipRegion.EndEnumRects( hRegionHandle );
        ImplWriteLine( "eoclip newpath" );
    }
}

//---------------------------------------------------------------------------------
// possible gfx formats:
//
// level 1: grayscale	8 bit
//			color	   24 bit
//
// level 2: grayscale	8 bit
//			color		1(pal), 4(pal), 8(pal), 24 Bit
//

void PSWriter::ImplBmp( Bitmap* pBitmap, Bitmap* pMaskBitmap, const Point & rPoint, double nXWidth, double nYHeightOrg )
{
    if ( !pBitmap )
        return;

    INT32	nHeightOrg = pBitmap->GetSizePixel().Height();
    INT32	nHeightLeft = nHeightOrg;
    long	nWidth = pBitmap->GetSizePixel().Width();
    Point	aSourcePos( rPoint );

    while ( nHeightLeft )
    {
        Bitmap	aTileBitmap( *pBitmap );
        long	nHeight = nHeightLeft;
        double	nYHeight = nYHeightOrg;

        BOOL	bDoTrans = FALSE;

        Rectangle	aRect;
        Region		aRegion;

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
            ImplTranslate( aSourcePos.X(), aSourcePos.Y() );
            ImplScale( nXWidth / nWidth,  nYHeight / nHeight );
            RegionHandle	hRegionHandle = aRegion.BeginEnumRects();

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

        ImplTranslate( aSourcePos.X(), aSourcePos.Y() + nYHeight );
        ImplScale( nXWidth, nYHeight );
        if ( mnLevel == 1 ) 				// level 1 is always grayscale !!!
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
        else	// Level 2
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

void PSWriter::ImplWriteString( const ByteString& rString, VirtualDevice& rVDev, const INT32* pDXArry, BOOL bStretch )
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
                    nx = pDXArry[ i - 1 ];
                ImplWriteDouble( ( bStretch ) ? nx : rVDev.GetTextWidth( rString.GetChar( i ) ) );
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
            ImplWriteLine( ") sw" );
        }
    }
}

// ------------------------------------------------------------------------

void PSWriter::ImplText( const String& rUniString, const Point& rPos, const INT32* pDXArry, sal_Int32 nWidth, VirtualDevice& rVDev )
{
    sal_uInt16 nLen = rUniString.Len();
    if ( !nLen )
        return;
    if ( mnTextMode == 0 )	// using glpyh outlines
    {		
        Font	aNotRotatedFont( maFont );
        aNotRotatedFont.SetOrientation( 0 );
    
        VirtualDevice aVirDev( 1 );
        aVirDev.SetMapMode( rVDev.GetMapMode() );
        aVirDev.SetFont( aNotRotatedFont );
        aVirDev.SetTextAlign( eTextAlign );

        sal_Int16 nRotation = maFont.GetOrientation();
        Polygon	aPolyDummy( 1 );
    
        PolyPolygon aPolyPoly;
        Point aPos( rPos );
        if ( nRotation )
        {
            aPolyDummy.SetPoint( aPos, 0 );
            aPolyDummy.Rotate( rPos, nRotation );
            aPos = aPolyDummy.GetPoint( 0 );
        }
        sal_Bool bOldLineColor = bLineColor;
        bLineColor = sal_False;
        std::vector<PolyPolygon> aPolyPolyVec;
        if ( aVirDev.GetTextOutlines( aPolyPolyVec, rUniString, 0, 0, STRING_LEN, TRUE, nWidth, pDXArry ) )
        {
            // always adjust text position to match baseline alignment
            ImplWriteLine( "pum" );
            ImplWriteDouble( aPos.X() );
            ImplWriteDouble( aPos.Y() );
            ImplWriteLine( "t" );
            if ( nRotation )
            {
                ImplWriteF( nRotation, 1 );
                *mpPS << "r ";
            }
            std::vector<PolyPolygon>::iterator aIter( aPolyPolyVec.begin() );
            while ( aIter != aPolyPolyVec.end() )
                ImplPolyPoly( *aIter++, sal_True );
            ImplWriteLine( "pom" );
        }
        bLineColor = bOldLineColor;
    }
    else if ( ( mnTextMode == 1 ) || ( mnTextMode == 2 ) )	// normal text output
    {
        if ( mnTextMode == 2 )	// forcing output one complete text packet, by
            pDXArry = NULL;		// ignoring the kerning array
        ImplSetAttrForText( rPos );
        ByteString aStr( rUniString, maFont.GetCharSet() );
        ImplWriteString( aStr, rVDev, pDXArry, nWidth != 0 );
        if ( maFont.GetOrientation() )
            ImplWriteLine( "gr" );
    }
}

// ------------------------------------------------------------------------

void PSWriter::ImplSetAttrForText( const Point& rPoint )
{
    Point aPoint( rPoint );

    long nRotation = maFont.GetOrientation();
    ImplWriteTextColor();

    Size aSize = maFont.GetSize();

    if ( maLastFont != maFont )
    {
        if ( maFont.GetPitch() == PITCH_FIXED ) 		// a little bit font selection
            ImplDefineFont( "Courier", "Oblique" );
        else if ( maFont.GetCharSet() == RTL_TEXTENCODING_SYMBOL )
            ImplWriteLine( "/Symbol findfont" );
        else if ( maFont.GetFamily() == FAMILY_SWISS )
            ImplDefineFont( "Helvetica", "Oblique" );
        else
            ImplDefineFont( "Times", "Italic" );

        maLastFont = maFont;
        aSize = maFont.GetSize();
        ImplWriteDouble( aSize.Height() );
        *mpPS << "sf ";
    }
    if ( eTextAlign != ALIGN_BASELINE )
    {														// PostScript kennt kein FontAlignment
        if ( eTextAlign == ALIGN_TOP )						// -> ich gehe daher davon aus, dass
            aPoint.Y() += ( aSize.Height() * 4 / 5 );		// der Bereich unter der Baseline
        else if ( eTextAlign == ALIGN_BOTTOM )				// in etwa 20% der Fontsize ausmacht
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

void PSWriter::ImplDefineFont( const char* pOriginalName, const char* pItalic )
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

void PSWriter::ImplClosePathDraw( ULONG nMode )
{
    *mpPS << "pc";
    mnCursorPos += 2;
    ImplExecMode( nMode );
}

void PSWriter::ImplPathDraw()
{
    *mpPS << "ps";
    mnCursorPos += 2;
    ImplExecMode( PS_RET );
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

double PSWriter::ImplGetScaling( const MapMode& rMapMode )
{
    double	nMul;
    switch ( rMapMode.GetMapUnit() )
    {
        case MAP_PIXEL :
        case MAP_SYSFONT :
        case MAP_APPFONT :

        case MAP_100TH_MM :
            nMul = 1;
            break;
        case MAP_10TH_MM :
            nMul = 10;
            break;
        case MAP_MM :
            nMul = 100;
            break;
        case MAP_CM :
            nMul = 1000;
            break;
        case MAP_1000TH_INCH :
            nMul = 2.54;
            break;
        case MAP_100TH_INCH :
            nMul = 25.4;
            break;
        case MAP_10TH_INCH :
            nMul = 254;
            break;
        case MAP_INCH :
            nMul = 2540;
            break;
        case MAP_TWIP :
            nMul = 1.76388889;
            break;
        case MAP_POINT :
            nMul = 35.27777778;
            break;
        default:
            nMul = 1.0;
            break;
    }
    return nMul;
}

//---------------------------------------------------------------------------------

void PSWriter::ImplGetMapMode( const MapMode& rMapMode )
{
    ImplWriteLine( "tm setmatrix" );
    double fMul = ImplGetScaling( rMapMode );
    double fScaleX = (double)rMapMode.GetScaleX() * fMul;
    double fScaleY = (double)rMapMode.GetScaleY() * fMul;
    ImplTranslate( rMapMode.GetOrigin().X() * fScaleX, rMapMode.GetOrigin().Y() * fScaleY );
    ImplScale( fScaleX, fScaleY );
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

inline void PSWriter::ImplWriteLine( const char* pString, ULONG nMode )
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

void PSWriter::ImplWriteLineInfo( double fLWidth, double fMLimit,
                                    SvtGraphicStroke::CapType eLCap,
                                        SvtGraphicStroke::JoinType eJoin,
                                            SvtGraphicStroke::DashArray& rLDash )
{
    if ( fLineWidth != fLWidth )
    {
        fLineWidth = fLWidth;
        ImplWriteDouble( fLineWidth );
        ImplWriteLine( "lw", PS_SPACE );
    }
    if ( eLineCap != eLCap )
    {
        eLineCap = eLCap;
        ImplWriteLong( (sal_Int32)eLineCap, PS_SPACE );
        ImplWriteLine( "lc", PS_SPACE );
    }
    if ( eJoinType != eJoin )
    {
        eJoinType = eJoin;
        ImplWriteLong( (sal_Int32)eJoinType, PS_SPACE );
        ImplWriteLine( "lj", PS_SPACE );
    }
    if ( eJoinType == SvtGraphicStroke::joinMiter )
    {
        if ( fMiterLimit != fMLimit )
        {
            fMiterLimit = fMLimit;
            ImplWriteDouble( fMiterLimit );
            ImplWriteLine( "ml", PS_SPACE );
        }
    }
    if ( aDashArray != rLDash )
    {
        aDashArray = rLDash;
        sal_uInt32 j, i = aDashArray.size();
        ImplWriteLine( "[", PS_SPACE );
        for ( j = 0; j < i; j++ )
            ImplWriteDouble( aDashArray[ j ] );
        ImplWriteLine( "] 0 ld" );
    }
}

//---------------------------------------------------------------------------------

void PSWriter::ImplWriteLineInfo( const LineInfo& rLineInfo )
{
    SvtGraphicStroke::DashArray l_aDashArray;
    if ( rLineInfo.GetStyle() == LINE_DASH )
        l_aDashArray.push_back( 2 );
    const double fLWidth(( ( rLineInfo.GetWidth() + 1 ) + ( rLineInfo.GetWidth() + 1 ) ) * 0.5);
    SvtGraphicStroke::JoinType aJoinType(SvtGraphicStroke::joinMiter);

    switch(rLineInfo.GetLineJoin())
    {
        default: // B2DLINEJOIN_NONE, B2DLINEJOIN_MIDDLE
            // do NOT use SvtGraphicStroke::joinNone here
            // since it will be written as numerical value directly 
            // and is NOT a valid EPS value
            break;
        case basegfx::B2DLINEJOIN_MITER:
            aJoinType = SvtGraphicStroke::joinMiter;
            break;
        case basegfx::B2DLINEJOIN_BEVEL:
            aJoinType = SvtGraphicStroke::joinBevel;
            break;
        case basegfx::B2DLINEJOIN_ROUND:
            aJoinType = SvtGraphicStroke::joinRound;
            break;
    }

    ImplWriteLineInfo( fLWidth, fMiterLimit, SvtGraphicStroke::capButt, aJoinType, l_aDashArray );
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
    sal_Int32 nLength;

    sal_Int32	nPTemp = (sal_Int32)fNumber;
    sal_Int32	nATemp = labs( (sal_Int32)( ( fNumber - nPTemp ) * 100000 ) );

    if ( !nPTemp && nATemp && ( fNumber < 0.0 ) )
        *mpPS << (sal_Char)'-';

    ByteString aNumber1( ByteString::CreateFromInt32( nPTemp ) );
    nLength = aNumber1.Len();
    mnCursorPos += nLength;
    for ( sal_Int32 n = 0; n < nLength; n++ )
        *mpPS << aNumber1.GetChar( (sal_uInt16)n );

    int zCount = 0;
    if ( nATemp )
    {
        *mpPS << (BYTE)'.';
        mnCursorPos++;
        const ByteString aNumber2( ByteString::CreateFromInt32( nATemp ) );

        sal_Int16 n, nLen = aNumber2.Len();
        if ( nLen < 8 )
        {
            mnCursorPos += 6 - nLen;
            for ( n = 0; n < ( 5 - nLen ); n++ )
            {
                *mpPS << (BYTE)'0';
            }
        }
        mnCursorPos += nLen;
        for ( n = 0; n < nLen; n++ )
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
    long nStSize =	( nCount + 1 ) - nLen;
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
    for( USHORT n = 0UL; n < nLen; n++	)
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

    nOffset = 32;						// anzahl freier bits in dwShift
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
    PSLZWCTreeNode* 	p;
    USHORT				i;
    BYTE				nV;

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
    BOOL	bRetValue = FALSE;
    ULONG	nBytesRead;

    if ( nSize < 256 )		// we assume that the file is greater than 256 bytes
        return FALSE;

    if ( nSize < POSTSCRIPT_BOUNDINGSEARCH )
        nBytesRead = nSize;
    else
        nBytesRead = POSTSCRIPT_BOUNDINGSEARCH;

    BYTE* pDest = ImplSearchEntry( pSource, (BYTE*)"%%BoundingBox:", nBytesRead, 14 );
    if ( pDest )
    {
        int 	nSecurityCount = 100;	// only 100 bytes following the bounding box will be checked
        nNumb[0] = nNumb[1] = nNumb[2] = nNumb[3] = 0;
        pDest += 14;
        for ( int i = 0; ( i < 4 ) && nSecurityCount; i++ )
        {
            int 	nDivision = 1;
            BOOL	bDivision = FALSE;
            BOOL	bNegative = FALSE;
            BOOL	bValid = TRUE;

            while ( ( --nSecurityCount ) && ( ( *pDest == ' ' ) || ( *pDest == 0x9 ) ) )
                pDest++;
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
                            nSecurityCount = 1; 	// error parsing the bounding box values
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

extern "C" BOOL __LOADONCALLAPI GraphicExport( SvStream & rStream, Graphic & rGraphic, FilterConfigItem* pFilterConfigItem, BOOL)
{
    PSWriter aPSWriter;
    return aPSWriter.WritePS( rGraphic, rStream, pFilterConfigItem );
}

//---------------------------------------------------------------------------------

extern "C" BOOL __LOADONCALLAPI DoExportDialog( FltCallDialogParameter& rPara )
{
    BOOL bRet = FALSE;

    if ( rPara.pWindow )
    {
        ByteString	aResMgrName( "eps" );
        ResMgr* pResMgr;

        pResMgr = ResMgr::CreateResMgr( aResMgrName.GetBuffer(), Application::GetSettings().GetUILocale() );

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

#ifdef WIN

static HINSTANCE hDLLInst = 0;		// HANDLE der DLL

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
