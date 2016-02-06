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

#include <tools/solar.h>
#include <tools/stream.hxx>
#include <tools/poly.hxx>
#include <tools/fract.hxx>
#include <vcl/svapp.hxx>
#include <vcl/metaact.hxx>
#include <vcl/graphic.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/region.hxx>
#include <vcl/metric.hxx>
#include <vcl/font.hxx>
#include <vcl/virdev.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/gradient.hxx>
#include <vcl/settings.hxx>
#include <unotools/configmgr.hxx>
#include <svl/solar.hrc>
#include <vcl/fltcall.hxx>
#include <vcl/FilterConfigItem.hxx>
#include <vcl/graphictools.hxx>
#include "strings.hrc"

#include <math.h>
#include <memory>

using namespace ::com::sun::star::uno;

#define POSTSCRIPT_BOUNDINGSEARCH   0x1000  // we only try to get the BoundingBox
                                            // in the first 4096 bytes

#define EPS_PREVIEW_TIFF    1
#define EPS_PREVIEW_EPSI    2

#define PS_LINESIZE         70      // maximum number of characters a line in the output

#define PS_NONE             0       // formatting mode: action which is inserted behind the output
#define PS_SPACE            1
#define PS_RET              2
#define PS_WRAP             4

// -----------------------------field-types------------------------------

struct StackMember
{
    struct      StackMember * pSucc;
    Color       aGlobalCol;
    bool        bLineCol;
    Color       aLineCol;
    bool        bFillCol;
    Color       aFillCol;
    Color       aTextCol;
    bool        bTextFillCol;
    Color       aTextFillCol;
    Color       aBackgroundCol;
    vcl::Font   aFont;
    TextAlign   eTextAlign;

    double                      fLineWidth;
    double                      fMiterLimit;
    SvtGraphicStroke::CapType   eLineCap;
    SvtGraphicStroke::JoinType  eJoinType;
    SvtGraphicStroke::DashArray aDashArray;
};

struct PSLZWCTreeNode
{

    PSLZWCTreeNode*     pBrother;       // next node who has the same father
    PSLZWCTreeNode*     pFirstChild;    // first son
    sal_uInt16          nCode;          // The code for the string of pixel values, which arises if... <missing comment>
    sal_uInt16          nValue;         // the pixel value
};

class PSWriter
{
private:
    bool            mbStatus;
    sal_uLong           mnLevelWarning;     // number of embedded eps files which was not exported
    sal_uLong           mnLastPercent;      // the number with which pCallback was called the last time
    sal_uInt32          mnLatestPush;       // offset to streamposition, where last push was done

    long                mnLevel;            // dialog options
    bool            mbGrayScale;
    bool            mbCompression;
    sal_Int32           mnPreview;
    sal_Int32           mnTextMode;

    SvStream*           mpPS;
    const GDIMetaFile*  pMTF;
    GDIMetaFile*        pAMTF;              // only created if Graphics is not a Metafile
    ScopedVclPtrInstance<VirtualDevice> pVDev;

    double              nBoundingX1;        // this represents the bounding box
    double              nBoundingY1;
    double              nBoundingX2;
    double              nBoundingY2;

    StackMember*        pGDIStack;
    sal_uLong           mnCursorPos;        // current cursor position in output
    Color               aColor;             // current color which is used for output
    bool            bLineColor;
    Color               aLineColor;         // current GDIMetafile color settings
    bool            bFillColor;
    Color               aFillColor;
    Color               aTextColor;
    bool            bTextFillColor;
    Color               aTextFillColor;
    Color               aBackgroundColor;
    bool            bRegionChanged;
    TextAlign           eTextAlign;

    double                      fLineWidth;
    double                      fMiterLimit;
    SvtGraphicStroke::CapType   eLineCap;
    SvtGraphicStroke::JoinType  eJoinType;
    SvtGraphicStroke::DashArray aDashArray;

    vcl::Font           maFont;
    vcl::Font           maLastFont;
    sal_uInt8           nChrSet;
    sal_uInt8           nNextChrSetId;      // first unused ChrSet-Id

    PSLZWCTreeNode*     pTable;             // LZW compression data
    PSLZWCTreeNode*     pPrefix;            // the compression is as same as the TIFF compression
    sal_uInt16          nDataSize;
    sal_uInt16          nClearCode;
    sal_uInt16          nEOICode;
    sal_uInt16          nTableSize;
    sal_uInt16          nCodeSize;
    sal_uInt32          nOffset;
    sal_uInt32          dwShift;

    css::uno::Reference< css::task::XStatusIndicator > xStatusIndicator;

    void                ImplWriteProlog( const Graphic* pPreviewEPSI = nullptr );
    void                ImplWriteEpilog();
    void                ImplWriteActions( const GDIMetaFile& rMtf, VirtualDevice& rVDev );

                        // this method makes LF's, space inserting and word wrapping as used in all nMode
                        // parameters
    inline void         ImplExecMode( sal_uLong nMode );

                        // writes char[] + LF to stream
    inline void         ImplWriteLine( const char*, sal_uLong nMode = PS_RET );

                        // writes ( nNumb / 10^nCount ) in ASCII format to stream
    void                ImplWriteF( sal_Int32 nNumb, sal_uLong nCount = 3, sal_uLong nMode = PS_SPACE );

                        // writes a double in ASCII format to stream
    void                ImplWriteDouble( double, sal_uLong nMode = PS_SPACE );

                        // writes a long in ASCII format to stream
    void                ImplWriteLong( sal_Int32 nNumb, sal_uLong nMode = PS_SPACE );

                        // writes a byte in ASCII format to stream
    void                ImplWriteByte( sal_uInt8 nNumb, sal_uLong nMode = PS_SPACE );

                        // writes a byte in ASCII (hex) format to stream
    void                ImplWriteHexByte( sal_uInt8 nNumb, sal_uLong nMode = PS_WRAP );

                        // writes nNumb as number from 0.000 till 1.000 in ASCII format to stream
    void                ImplWriteB1( sal_uInt8 nNumb, sal_uLong nMode = PS_SPACE );

    inline void         ImplWritePoint( const Point&, sal_uInt32 nMode = PS_SPACE );
    void                ImplMoveTo( const Point&, sal_uInt32 nMode = PS_SPACE );
    void                ImplLineTo( const Point&, sal_uInt32 nMode = PS_SPACE );
    void                ImplCurveTo( const Point& rP1, const Point& rP2, const Point& rP3, sal_uInt32 nMode = PS_SPACE );
    void                ImplTranslate( const double& fX, const double& fY, sal_uInt32 nMode = PS_RET );
    void                ImplScale( const double& fX, const double& fY, sal_uInt32 nMode = PS_RET );

    void                ImplAddPath( const tools::Polygon & rPolygon );
    void                ImplWriteLineInfo( double fLineWidth, double fMiterLimit, SvtGraphicStroke::CapType eLineCap,
                                    SvtGraphicStroke::JoinType eJoinType, SvtGraphicStroke::DashArray& rDashArray );
    void                ImplWriteLineInfo( const LineInfo& rLineInfo );
    void                ImplRect( const Rectangle & rRectangle );
    void                ImplRectFill ( const Rectangle & rRectangle );
    void                ImplWriteGradient( const tools::PolyPolygon& rPolyPoly, const Gradient& rGradient, VirtualDevice& rVDev );
    void                ImplIntersect( const tools::PolyPolygon& rPolyPoly );
    void                ImplPolyPoly( const tools::PolyPolygon & rPolyPolygon, bool bTextOutline = false );
    void                ImplPolyLine( const tools::Polygon & rPolygon );

    void                ImplSetClipRegion( vcl::Region& rRegion );
    void                ImplBmp( Bitmap*, Bitmap*, const Point &, double nWidth, double nHeight );
    void                ImplText( const OUString& rUniString, const Point& rPos, const long* pDXArry, sal_Int32 nWidth, VirtualDevice& rVDev );
    void                ImplSetAttrForText( const Point & rPoint );
    void                ImplWriteCharacter( sal_Char );
    void                ImplWriteString( const OString&, VirtualDevice& rVDev, const long* pDXArry = nullptr, bool bStretch = false );
    void                ImplDefineFont( const char*, const char* );

    void                ImplClosePathDraw( sal_uLong nMode = PS_RET );
    void                ImplPathDraw();

    inline void         ImplWriteLineColor( sal_uLong nMode = PS_RET );
    inline void         ImplWriteFillColor( sal_uLong nMode = PS_RET );
    inline void         ImplWriteTextColor( sal_uLong nMode = PS_RET );
    void                ImplWriteColor( sal_uLong nMode );

    void                ImplGetMapMode( const MapMode& );
    static bool         ImplGetBoundingBox( double* nNumb, sal_uInt8* pSource, sal_uLong nSize );
    static sal_uInt8*   ImplSearchEntry( sal_uInt8* pSource, sal_uInt8 const * pDest, sal_uLong nComp, sal_uLong nSize );
                        // LZW methods
    void                StartCompression();
    void                Compress( sal_uInt8 nSrc );
    void                EndCompression();
    inline void         WriteBits( sal_uInt16 nCode, sal_uInt16 nCodeLen );

public:
    bool            WritePS( const Graphic& rGraphic, SvStream& rTargetStream, FilterConfigItem* );
    PSWriter();
    ~PSWriter();
};

//========================== methods from PSWriter ==========================


PSWriter::PSWriter()
    : mbStatus(false)
    , mnLevelWarning(0)
    , mnLastPercent(0)
    , mnLatestPush(0)
    , mnLevel(0)
    , mbGrayScale(false)
    , mbCompression(false)
    , mnPreview(0)
    , mnTextMode(0)
    , mpPS(nullptr)
    , pMTF(nullptr)
    , pAMTF(nullptr)
    , pVDev()
    , nBoundingX1(0)
    , nBoundingY1(0)
    , nBoundingX2(0)
    , nBoundingY2(0)
    , pGDIStack(nullptr)
    , mnCursorPos(0)
    , aColor()
    , bLineColor(false)
    , aLineColor()
    , bFillColor(false)
    , aFillColor()
    , aTextColor()
    , bTextFillColor(false)
    , aTextFillColor()
    , aBackgroundColor()
    , bRegionChanged(false)
    , eTextAlign()
    , fLineWidth(0)
    , fMiterLimit(0)
    , eLineCap()
    , eJoinType()
    , aDashArray()
    , maFont()
    , maLastFont()
    , nChrSet(0)
    , nNextChrSetId(0)
    , pTable(nullptr)
    , pPrefix(nullptr)
    , nDataSize(0)
    , nClearCode(0)
    , nEOICode(0)
    , nTableSize(0)
    , nCodeSize(0)
    , nOffset(0)
    , dwShift(0)
    , xStatusIndicator()
{
    pAMTF = nullptr;
}


PSWriter::~PSWriter()
{
    delete pAMTF;
}


bool PSWriter::WritePS( const Graphic& rGraphic, SvStream& rTargetStream, FilterConfigItem* pFilterConfigItem )
{
    sal_uInt32 nStreamPosition = 0, nPSPosition = 0; // -Wall warning, unset, check

    mbStatus = true;
    mnPreview = 0;
    mnLevelWarning = 0;
    mnLastPercent = 0;
    mnLatestPush = 0xEFFFFFFE;

    if ( pFilterConfigItem )
    {
        xStatusIndicator = pFilterConfigItem->GetStatusIndicator();
        if ( xStatusIndicator.is() )
        {
            OUString aMsg;
            xStatusIndicator->start( aMsg, 100 );
        }
    }

    mpPS = &rTargetStream;
    mpPS->SetEndian( SvStreamEndian::LITTLE );

    // default values for the dialog options
    mnLevel = 2;
    mbGrayScale = false;
#ifdef UNX // don't compress by default on unix as ghostscript is unable to read LZW compressed eps
    mbCompression = false;
#else
    mbCompression = sal_True;
#endif
    mnTextMode = 0;         // default0 : export glyph outlines

    // try to get the dialog selection
    if ( pFilterConfigItem )
    {
        std::unique_ptr<ResMgr> pResMgr(ResMgr::CreateResMgr( "eps", Application::GetSettings().GetUILanguageTag() ));

        if( pResMgr )
        {
#ifdef UNX // don't put binary tiff preview ahead of postscript code by default on unix as ghostscript is unable to read it
            mnPreview = pFilterConfigItem->ReadInt32( "Preview", 0 );
#else
            mnPreview = pFilterConfigItem->ReadInt32( "Preview", 1 );
#endif
            mnLevel = pFilterConfigItem->ReadInt32( "Version", 2 );
            if ( mnLevel != 1 )
                mnLevel = 2;
            mbGrayScale = pFilterConfigItem->ReadInt32( "ColorFormat", 1 ) == 2;
#ifdef UNX // don't compress by default on unix as ghostscript is unable to read LZW compressed eps
            mbCompression = pFilterConfigItem->ReadInt32( "CompressionMode", 0 ) != 0;
#else
            mbCompression = pFilterConfigItem->ReadInt32( "CompressionMode", 1 ) == 1;
#endif
            mnTextMode = pFilterConfigItem->ReadInt32( "TextMode", 0 );
            if ( mnTextMode > 2 )
                mnTextMode = 0;
        }
    }

    // compression is not available for Level 1
    if ( mnLevel == 1 )
    {
        mbGrayScale = true;
        mbCompression = false;
    }

    if ( mnPreview & EPS_PREVIEW_TIFF )
    {
        rTargetStream.WriteUInt32( 0xC6D3D0C5 );
        nStreamPosition = rTargetStream.Tell();
        rTargetStream.WriteUInt32( 0 ).WriteUInt32( 0 ).WriteUInt32( 0 ).WriteUInt32( 0 )
           .WriteUInt32( nStreamPosition + 26 ).WriteUInt32( 0 ).WriteUInt16( 0xffff );

        sal_uInt32 nErrCode;
        if ( mbGrayScale )
        {
            BitmapEx aTempBitmapEx( rGraphic.GetBitmapEx() );
            aTempBitmapEx.Convert( BMP_CONVERSION_8BIT_GREYS );
            nErrCode = GraphicConverter::Export( rTargetStream, aTempBitmapEx, ConvertDataFormat::TIF ) ;
        }
        else
            nErrCode = GraphicConverter::Export( rTargetStream, rGraphic, ConvertDataFormat::TIF ) ;

        if ( nErrCode == ERRCODE_NONE )
        {
            rTargetStream.Seek( STREAM_SEEK_TO_END );
            nPSPosition = rTargetStream.Tell();
            rTargetStream.Seek( nStreamPosition + 20 );
            rTargetStream.WriteUInt32( nPSPosition - 30 );  // size of tiff gfx
            rTargetStream.Seek( nPSPosition );
        }
        else
        {
            mnPreview &=~ EPS_PREVIEW_TIFF;
            rTargetStream.Seek( nStreamPosition - 4 );
        }
    }

    // global default value setting
    StackMember*    pGS;

    if (rGraphic.GetType() == GRAPHIC_GDIMETAFILE)
        pMTF = &rGraphic.GetGDIMetaFile();
    else if (rGraphic.GetGDIMetaFile().GetActionSize())
        pMTF = pAMTF = new GDIMetaFile( rGraphic.GetGDIMetaFile() );
    else
    {
        Bitmap aBmp( rGraphic.GetBitmap() );
        pAMTF = new GDIMetaFile();
        ScopedVclPtrInstance< VirtualDevice > pTmpVDev;
        pAMTF->Record( pTmpVDev );
        pTmpVDev->DrawBitmap( Point(), aBmp );
        pAMTF->Stop();
        pAMTF->SetPrefSize( aBmp.GetSizePixel() );
        pMTF = pAMTF;
    }
    pVDev->SetMapMode( pMTF->GetPrefMapMode() );
    nBoundingX1 = nBoundingY1 = 0;
    nBoundingX2 = pMTF->GetPrefSize().Width();
    nBoundingY2 = pMTF->GetPrefSize().Height();

    pGDIStack = nullptr;
    aColor = Color( COL_TRANSPARENT );
    bLineColor = true;
    aLineColor = Color( COL_BLACK );
    bFillColor = true;
    aFillColor = Color( COL_WHITE );
    bTextFillColor = true;
    aTextFillColor = Color( COL_BLACK );
    fLineWidth = 1;
    fMiterLimit = 15; // use same limit as most graphic systems and basegfx
    eLineCap = SvtGraphicStroke::capButt;
    eJoinType = SvtGraphicStroke::joinMiter;
    aBackgroundColor = Color( COL_WHITE );
    eTextAlign = ALIGN_BASELINE;
    bRegionChanged = false;

    nChrSet = 0x00;
    nNextChrSetId = 1;

    if( pMTF->GetActionSize() )
    {
        ImplWriteProlog( ( mnPreview & EPS_PREVIEW_EPSI ) ? &rGraphic : nullptr );
        mnCursorPos = 0;
        ImplWriteActions( *pMTF, *pVDev.get() );
        ImplWriteEpilog();
        if ( mnPreview & EPS_PREVIEW_TIFF )
        {
            sal_uInt32 nPosition = rTargetStream.Tell();
            rTargetStream.Seek( nStreamPosition );
            rTargetStream.WriteUInt32( nPSPosition );
            rTargetStream.WriteUInt32( nPosition - nPSPosition );
            rTargetStream.Seek( nPosition );
        }
        while( pGDIStack )
        {
            pGS=pGDIStack;
            pGDIStack=pGS->pSucc;
            delete pGS;
        }
    }
    else
        mbStatus = false;

    if ( mbStatus && mnLevelWarning && pFilterConfigItem )
    {
        ResMgr* pResMgr;
        pResMgr = ResMgr::CreateResMgr( "eps", Application::GetSettings().GetUILanguageTag() );
        if( pResMgr )
        {
            ScopedVclPtrInstance< InfoBox > aInfoBox( nullptr, ResId(KEY_VERSION_CHECK, *pResMgr).toString() );
            aInfoBox->Execute();
            delete pResMgr;
        }
    }

    if ( xStatusIndicator.is() )
        xStatusIndicator->end();

    return mbStatus;
}


void PSWriter::ImplWriteProlog( const Graphic* pPreview )
{
    ImplWriteLine( "%!PS-Adobe-3.0 EPSF-3.0 " );
    mpPS->WriteCharPtr( "%%BoundingBox: " );                         // BoundingBox
    ImplWriteLong( 0 );
    ImplWriteLong( 0 );
    Size aSizePoint = OutputDevice::LogicToLogic( pMTF->GetPrefSize(),
                        pMTF->GetPrefMapMode(), MAP_POINT );
    ImplWriteLong( aSizePoint.Width() );
    ImplWriteLong( aSizePoint.Height() ,PS_RET );
    ImplWriteLine( "%%Pages: 0" );
    OUString aCreator( "%%Creator: " + utl::ConfigManager::getProductName() + " " +
                       utl::ConfigManager::getProductVersion() );
    ImplWriteLine( OUStringToOString( aCreator, RTL_TEXTENCODING_UTF8 ).getStr() );
    ImplWriteLine( "%%Title: none" );
    ImplWriteLine( "%%CreationDate: none" );

// defaults

    mpPS->WriteCharPtr( "%%LanguageLevel: " );                       // Language level
    ImplWriteLong( mnLevel, PS_RET );
    if ( !mbGrayScale && mnLevel == 1 )
        ImplWriteLine( "%%Extensions: CMYK" );          // CMYK extension is to set in color mode in level 1
    ImplWriteLine( "%%EndComments" );
    if ( pPreview && aSizePoint.Width() && aSizePoint.Height() )
    {
        Size aSizeBitmap( ( aSizePoint.Width() + 7 ) & ~7, aSizePoint.Height() );
        Bitmap aTmpBitmap( pPreview->GetBitmap() );
        aTmpBitmap.Scale( aSizeBitmap, BmpScaleFlag::BestQuality );
        aTmpBitmap.Convert( BMP_CONVERSION_1BIT_THRESHOLD );
        BitmapReadAccess* pAcc = aTmpBitmap.AcquireReadAccess();
        if ( pAcc )
        {
            mpPS->WriteCharPtr( "%%BeginPreview: " );                    // BoundingBox
            ImplWriteLong( aSizeBitmap.Width() );
            ImplWriteLong( aSizeBitmap.Height() );
            mpPS->WriteCharPtr( "1 " );
            sal_Int32 nLines = aSizeBitmap.Width() / 312;
            if ( ( nLines * 312 ) != aSizeBitmap.Width() )
                nLines++;
            nLines *= aSizeBitmap.Height();
            ImplWriteLong( nLines );
            sal_Int32 nX, nY, nCount2, nCount = 4;
            const BitmapColor aBlack( pAcc->GetBestMatchingColor( Color( COL_BLACK ) ) );
            for ( nY = 0; nY < aSizeBitmap.Height(); nY++ )
            {
                nCount2 = 0;
                char nVal = 0;
                for ( nX = 0; nX < aSizeBitmap.Width(); nX++ )
                {
                    if ( !nCount2 )
                    {
                        ImplExecMode( PS_RET );
                        mpPS->WriteCharPtr( "%" );
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
                        mpPS->WriteChar( nVal );
                        nVal = 0;
                        nCount += 4;
                    }
                    nCount2--;
                }
            }
            Bitmap::ReleaseAccess( pAcc );
            ImplExecMode( PS_RET );
            ImplWriteLine( "%%EndPreview" );
        }
    }
    ImplWriteLine( "%%BeginProlog" );
    ImplWriteLine( "%%BeginResource: procset SDRes-Prolog 1.0 0" );

//  BEGIN EPSF
    ImplWriteLine( "/b4_inc_state save def\n/dict_count countdictstack def\n/op_count count 1 sub def\nuserdict begin" );
    ImplWriteLine( "0 setgray 0 setlinecap 1 setlinewidth 0 setlinejoin 10 setmiterlimit[] 0 setdash newpath" );
    ImplWriteLine( "/languagelevel where {pop languagelevel 1 ne {false setstrokeadjust false setoverprint} if} if" );

    ImplWriteLine( "/bdef {bind def} bind def" );       // the new operator bdef is created
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

    ImplWriteLine( "/ef {eofill}bdef"           );      // close path and fill
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


void PSWriter::ImplWriteEpilog()
{
    ImplTranslate( 0, nBoundingY2 );
    ImplWriteLine( "pom" );
    ImplWriteLine( "count op_count sub {pop} repeat countdictstack dict_count sub {end} repeat b4_inc_state restore" );

    ImplWriteLine( "%%PageTrailer" );
    ImplWriteLine( "%%Trailer" );

    ImplWriteLine( "%%EOF" );
}


void PSWriter::ImplWriteActions( const GDIMetaFile& rMtf, VirtualDevice& rVDev )
{
    tools::PolyPolygon aFillPath;

    for( size_t nCurAction = 0, nCount = rMtf.GetActionSize(); nCurAction < nCount; nCurAction++ )
    {
        MetaAction* pMA = rMtf.GetAction( nCurAction );

        switch( pMA->GetType() )
        {
            case MetaActionType::NONE :
            break;

            case MetaActionType::PIXEL :
            {
                Color aOldLineColor( aLineColor );
                aLineColor = static_cast<const MetaPixelAction*>(pMA)->GetColor();
                ImplWriteLineColor( PS_SPACE );
                ImplMoveTo( static_cast<const MetaPixelAction*>(pMA)->GetPoint() );
                ImplLineTo( static_cast<const MetaPixelAction*>(pMA)->GetPoint() );
                ImplPathDraw();
                aLineColor = aOldLineColor;
            }
            break;

            case MetaActionType::POINT :
            {
                ImplWriteLineColor( PS_SPACE );
                ImplMoveTo( static_cast<const MetaPointAction*>(pMA)->GetPoint() );
                ImplLineTo( static_cast<const MetaPointAction*>(pMA)->GetPoint() );
                ImplPathDraw();
            }
            break;

            case MetaActionType::LINE :
            {
                const LineInfo& rLineInfo = static_cast<const MetaLineAction*>(pMA)->GetLineInfo();
                ImplWriteLineInfo( rLineInfo );
                if ( bLineColor )
                {
                    ImplWriteLineColor( PS_SPACE );
                    ImplMoveTo( static_cast<const MetaLineAction*>(pMA)->GetStartPoint() );
                    ImplLineTo( static_cast<const MetaLineAction*>(pMA )->GetEndPoint() );
                    ImplPathDraw();
                }
            }
            break;

            case MetaActionType::RECT :
            {
                ImplRect( static_cast<const MetaRectAction*>(pMA)->GetRect() );
            }
            break;

            case MetaActionType::ROUNDRECT :
                ImplRect( static_cast<const MetaRoundRectAction*>(pMA)->GetRect() );
            break;

            case MetaActionType::ELLIPSE :
            {
                Rectangle   aRect = static_cast<const MetaEllipseAction*>(pMA)->GetRect();
                Point       aCenter = aRect.Center();
                tools::Polygon aPoly( aCenter, aRect.GetWidth() / 2, aRect.GetHeight() / 2 );
                tools::PolyPolygon aPolyPoly( aPoly );
                ImplPolyPoly( aPolyPoly );
            }
            break;

            case MetaActionType::ARC :
            {
                tools::Polygon aPoly( static_cast<const MetaArcAction*>(pMA)->GetRect(), static_cast<const MetaArcAction*>(pMA)->GetStartPoint(),
                    static_cast<const MetaArcAction*>(pMA)->GetEndPoint(), POLY_ARC );
                tools::PolyPolygon aPolyPoly( aPoly );
                ImplPolyPoly( aPolyPoly );
            }
            break;

            case MetaActionType::PIE :
            {
                tools::Polygon aPoly( static_cast<const MetaPieAction*>(pMA)->GetRect(), static_cast<const MetaPieAction*>(pMA)->GetStartPoint(),
                    static_cast<const MetaPieAction*>(pMA)->GetEndPoint(), POLY_PIE );
                tools::PolyPolygon aPolyPoly( aPoly );
                ImplPolyPoly( aPolyPoly );
            }
            break;

            case MetaActionType::CHORD :
            {
                tools::Polygon aPoly( static_cast<const MetaChordAction*>(pMA)->GetRect(), static_cast<const MetaChordAction*>(pMA)->GetStartPoint(),
                    static_cast<const MetaChordAction*>(pMA)->GetEndPoint(), POLY_CHORD );
                tools::PolyPolygon aPolyPoly( aPoly );
                ImplPolyPoly( aPolyPoly );
            }
            break;

            case MetaActionType::POLYLINE :
            {
                tools::Polygon aPoly( static_cast<const MetaPolyLineAction*>(pMA)->GetPolygon() );
                const LineInfo& rLineInfo = static_cast<const MetaPolyLineAction*>(pMA)->GetLineInfo();
                ImplWriteLineInfo( rLineInfo );

                if(basegfx::B2DLineJoin::NONE == rLineInfo.GetLineJoin()
                    && rLineInfo.GetWidth() > 1)
                {
                    // emulate B2DLineJoin::NONE by creating single edges
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
                            const tools::Polygon aSnippet(4,
                                aPoly.GetConstPointAry() + a,
                                aPoly.GetConstFlagAry() + a);
                            ImplPolyLine(aSnippet);
                            a += 2;
                        }
                        else
                        {
                            const tools::Polygon aSnippet(2,
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

            case MetaActionType::POLYGON :
            {
                tools::PolyPolygon aPolyPoly( static_cast<const MetaPolygonAction*>(pMA)->GetPolygon() );
                ImplPolyPoly( aPolyPoly );
            }
            break;

            case MetaActionType::POLYPOLYGON :
            {
                ImplPolyPoly( static_cast<const MetaPolyPolygonAction*>(pMA)->GetPolyPolygon() );
            }
            break;

            case MetaActionType::TEXT:
            {
                const MetaTextAction * pA = static_cast<const MetaTextAction*>(pMA);

                OUString  aUniStr = pA->GetText().copy( pA->GetIndex(), pA->GetLen() );
                Point     aPoint( pA->GetPoint() );

                ImplText( aUniStr, aPoint, nullptr, 0, rVDev );
            }
            break;

            case MetaActionType::TEXTRECT:
            {
                OSL_FAIL( "Unsupported action: TextRect...Action!" );
            }
            break;

            case MetaActionType::STRETCHTEXT :
            {
                const MetaStretchTextAction* pA = static_cast<const MetaStretchTextAction*>(pMA);
                OUString  aUniStr = pA->GetText().copy( pA->GetIndex(), pA->GetLen() );
                Point     aPoint( pA->GetPoint() );

                ImplText( aUniStr, aPoint, nullptr, pA->GetWidth(), rVDev );
            }
            break;

            case MetaActionType::TEXTARRAY:
            {
                const MetaTextArrayAction* pA = static_cast<const MetaTextArrayAction*>(pMA);
                OUString  aUniStr = pA->GetText().copy( pA->GetIndex(), pA->GetLen() );
                Point     aPoint( pA->GetPoint() );

                ImplText( aUniStr, aPoint, pA->GetDXArray(), 0, rVDev );
            }
            break;

            case MetaActionType::BMP :
            {
                Bitmap aBitmap = static_cast<const MetaBmpAction*>(pMA)->GetBitmap();
                if ( mbGrayScale )
                    aBitmap.Convert( BMP_CONVERSION_8BIT_GREYS );
                Point aPoint = static_cast<const MetaBmpAction*>(pMA)->GetPoint();
                Size aSize( rVDev.PixelToLogic( aBitmap.GetSizePixel() ) );
                ImplBmp( &aBitmap, nullptr, aPoint, aSize.Width(), aSize.Height() );
            }
            break;

            case MetaActionType::BMPSCALE :
            {
                Bitmap aBitmap = static_cast<const MetaBmpScaleAction*>(pMA)->GetBitmap();
                if ( mbGrayScale )
                    aBitmap.Convert( BMP_CONVERSION_8BIT_GREYS );
                Point aPoint = static_cast<const MetaBmpScaleAction*>(pMA)->GetPoint();
                Size aSize = static_cast<const MetaBmpScaleAction*>(pMA)->GetSize();
                ImplBmp( &aBitmap, nullptr, aPoint, aSize.Width(), aSize.Height() );
            }
            break;

            case MetaActionType::BMPSCALEPART :
            {
                Bitmap  aBitmap( static_cast<const MetaBmpScalePartAction*>(pMA)->GetBitmap() );
                aBitmap.Crop( Rectangle( static_cast<const MetaBmpScalePartAction*>(pMA)->GetSrcPoint(),
                    static_cast<const MetaBmpScalePartAction*>(pMA)->GetSrcSize() ) );
                if ( mbGrayScale )
                    aBitmap.Convert( BMP_CONVERSION_8BIT_GREYS );
                Point aPoint = static_cast<const MetaBmpScalePartAction*>(pMA)->GetDestPoint();
                Size aSize = static_cast<const MetaBmpScalePartAction*>(pMA)->GetDestSize();
                ImplBmp( &aBitmap, nullptr, aPoint, aSize.Width(), aSize.Height() );
            }
            break;

            case MetaActionType::BMPEX :
            {
                BitmapEx aBitmapEx( static_cast<MetaBmpExAction*>(pMA)->GetBitmapEx() );
                Bitmap aBitmap( aBitmapEx.GetBitmap() );
                if ( mbGrayScale )
                    aBitmap.Convert( BMP_CONVERSION_8BIT_GREYS );
                Bitmap aMask( aBitmapEx.GetMask() );
                Point aPoint( static_cast<const MetaBmpExAction*>(pMA)->GetPoint() );
                Size aSize( rVDev.PixelToLogic( aBitmap.GetSizePixel() ) );
                ImplBmp( &aBitmap, &aMask, aPoint, aSize.Width(), aSize.Height() );
            }
            break;

            case MetaActionType::BMPEXSCALE :
            {
                BitmapEx aBitmapEx( static_cast<MetaBmpExScaleAction*>(pMA)->GetBitmapEx() );
                Bitmap aBitmap( aBitmapEx.GetBitmap() );
                if ( mbGrayScale )
                    aBitmap.Convert( BMP_CONVERSION_8BIT_GREYS );
                Bitmap aMask( aBitmapEx.GetMask() );
                Point aPoint = static_cast<const MetaBmpExScaleAction*>(pMA)->GetPoint();
                Size aSize( static_cast<const MetaBmpExScaleAction*>(pMA)->GetSize() );
                ImplBmp( &aBitmap, &aMask, aPoint, aSize.Width(), aSize.Height() );
            }
            break;

            case MetaActionType::BMPEXSCALEPART :
            {
                BitmapEx    aBitmapEx( static_cast<const MetaBmpExScalePartAction*>(pMA)->GetBitmapEx() );
                aBitmapEx.Crop( Rectangle( static_cast<const MetaBmpExScalePartAction*>(pMA)->GetSrcPoint(),
                    static_cast<const MetaBmpExScalePartAction*>(pMA)->GetSrcSize() ) );
                Bitmap      aBitmap( aBitmapEx.GetBitmap() );
                if ( mbGrayScale )
                    aBitmap.Convert( BMP_CONVERSION_8BIT_GREYS );
                Bitmap      aMask( aBitmapEx.GetMask() );
                Point aPoint = static_cast<const MetaBmpExScalePartAction*>(pMA)->GetDestPoint();
                Size aSize = static_cast<const MetaBmpExScalePartAction*>(pMA)->GetDestSize();
                ImplBmp( &aBitmap, &aMask, aPoint, aSize.Width(), aSize.Height() );
            }
            break;

            // Unsupported Actions
            case MetaActionType::MASK:
            case MetaActionType::MASKSCALE:
            case MetaActionType::MASKSCALEPART:
            {
                OSL_FAIL( "Unsupported action: MetaMask...Action!" );
            }
            break;

            case MetaActionType::GRADIENT :
            {
                tools::PolyPolygon aPolyPoly( static_cast<const MetaGradientAction*>(pMA)->GetRect() );
                ImplWriteGradient( aPolyPoly, static_cast<const MetaGradientAction*>(pMA)->GetGradient(), rVDev );
            }
            break;

            case MetaActionType::GRADIENTEX :
            {
                tools::PolyPolygon aPolyPoly( static_cast<const MetaGradientExAction*>(pMA)->GetPolyPolygon() );
                ImplWriteGradient( aPolyPoly, static_cast<const MetaGradientExAction*>(pMA)->GetGradient(), rVDev );
            }
            break;

            case MetaActionType::HATCH :
            {
                ScopedVclPtrInstance< VirtualDevice > l_pVirDev;
                GDIMetaFile     aTmpMtf;

                l_pVirDev->SetMapMode( rVDev.GetMapMode() );
                l_pVirDev->AddHatchActions( static_cast<const MetaHatchAction*>(pMA)->GetPolyPolygon(),
                                            static_cast<const MetaHatchAction*>(pMA)->GetHatch(), aTmpMtf );
                ImplWriteActions( aTmpMtf, rVDev );
            }
            break;

            case MetaActionType::WALLPAPER :
            {
                const MetaWallpaperAction* pA = static_cast<const MetaWallpaperAction*>(pMA);
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
                        ImplBmp( &aBitmap, nullptr, Point( aRect.Left(), aRect.Top() ), aRect.GetWidth(), aRect.GetHeight() );

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

            case MetaActionType::ISECTRECTCLIPREGION:
            {
                const MetaISectRectClipRegionAction* pA = static_cast<const MetaISectRectClipRegionAction*>(pMA);
                vcl::Region aRegion( pA->GetRect() );
                ImplSetClipRegion( aRegion );
            }
            break;

            case MetaActionType::CLIPREGION:
            {
                const MetaClipRegionAction* pA = static_cast<const MetaClipRegionAction*>(pMA);
                vcl::Region aRegion( pA->GetRegion() );
                ImplSetClipRegion( aRegion );
            }
            break;

            case MetaActionType::ISECTREGIONCLIPREGION:
            {
                const MetaISectRegionClipRegionAction* pA = static_cast<const MetaISectRegionClipRegionAction*>(pMA);
                vcl::Region aRegion( pA->GetRegion() );
                ImplSetClipRegion( aRegion );
            }
            break;

            case MetaActionType::MOVECLIPREGION:
            {
/*
                if ( !aClipRegion.IsEmpty() )
                {
                    const MetaMoveClipRegionAction* pA = static_cast<const MetaMoveClipRegionAction*>(pMA);
                    aClipRegion.Move( pA->GetHorzMove(), pA->GetVertMove() );
                    ImplSetClipRegion();
                }
*/
            }
            break;

            case MetaActionType::LINECOLOR :
            {
                if ( static_cast<const MetaLineColorAction*>(pMA)->IsSetting() )
                {
                    bLineColor = true;
                    aLineColor = static_cast<const MetaLineColorAction*>(pMA)->GetColor();
                }
                else
                    bLineColor = false;
            }
            break;

            case MetaActionType::FILLCOLOR :
            {
                if ( static_cast<const MetaFillColorAction*>(pMA)->IsSetting() )
                {
                    bFillColor = true;
                    aFillColor =  static_cast<const MetaFillColorAction*>(pMA)->GetColor();
                }
                else
                    bFillColor = false;
            }
            break;

            case MetaActionType::TEXTCOLOR :
            {
                aTextColor = static_cast<const MetaTextColorAction*>(pMA)->GetColor();
            }
            break;

            case MetaActionType::TEXTFILLCOLOR :
            {
                if ( static_cast<const MetaTextFillColorAction*>(pMA)->IsSetting() )
                {
                    bTextFillColor = true;
                    aTextFillColor = static_cast<const MetaTextFillColorAction*>(pMA)->GetColor();
                }
                else
                    bTextFillColor = false;
            }
            break;

            case MetaActionType::TEXTALIGN :
            {
                eTextAlign = static_cast<const MetaTextAlignAction*>(pMA)->GetTextAlign();
            }
            break;

            case MetaActionType::MAPMODE :
            {
                pMA->Execute( &rVDev );
                ImplGetMapMode( rVDev.GetMapMode() );
            }
            break;

            case MetaActionType::FONT :
            {
                maFont = static_cast<const MetaFontAction*>(pMA)->GetFont();
                rVDev.SetFont( maFont );
            }
            break;

            case MetaActionType::PUSH :
            {
                rVDev.Push(static_cast<const MetaPushAction*>(pMA)->GetFlags() );
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
                bRegionChanged = false;
                pGS->aFont = maFont;
                mnLatestPush = mpPS->Tell();
                ImplWriteLine( "gs" );
            }
            break;

            case MetaActionType::POP :
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
                    maLastFont = vcl::Font();                // set maLastFont != maFont -> so that
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

            case MetaActionType::EPS :
            {
                GfxLink aGfxLink = static_cast<const MetaEPSAction*>(pMA)->GetLink();
                const GDIMetaFile aSubstitute( static_cast<const MetaEPSAction*>(pMA)->GetSubstitute() );

                bool    bLevelConflict = false;
                sal_uInt8*  pSource = const_cast<sal_uInt8*>(aGfxLink.GetData());
                sal_uLong   nSize = aGfxLink.GetDataSize();
                sal_uLong   nParseThis = POSTSCRIPT_BOUNDINGSEARCH;
                if ( nSize < 64 )                       // assuming eps is larger than 64 bytes
                    pSource = nullptr;
                if ( nParseThis > nSize )
                    nParseThis = nSize;

                if ( pSource && ( mnLevel == 1 ) )
                {
                    sal_uInt8* pFound = ImplSearchEntry( pSource, reinterpret_cast<sal_uInt8 const *>("%%LanguageLevel:"), nParseThis - 10, 16 );
                    if ( pFound )
                    {
                        sal_uInt8   k, i = 10;
                        pFound += 16;
                        while ( --i )
                        {
                            k = *pFound++;
                            if ( ( k > '0' ) && ( k <= '9' ) )
                            {
                                if ( k != '1' )
                                {
                                    bLevelConflict = true;
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
                        Point   aPoint = static_cast<const MetaEPSAction*>(pMA)->GetPoint();
                        Size    aSize = static_cast<const MetaEPSAction*>(pMA)->GetSize();

                        MapMode aMapMode( aSubstitute.GetPrefMapMode() );
                        Size aOutSize( OutputDevice::LogicToLogic( aSize, rVDev.GetMapMode(), aMapMode ) );
                        Point aOrigin( OutputDevice::LogicToLogic( aPoint, rVDev.GetMapMode(), aMapMode ) );
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

            case MetaActionType::Transparent:
            {
//              ImplLine( ( (const MetaTransparentAction*) pMA )->GetPolyPolygon() );
            }
            break;

            case MetaActionType::RASTEROP:
            {
                pMA->Execute( &rVDev );
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

                ImplWriteActions( aTmpMtf, rVDev );
            }
            break;

            case MetaActionType::COMMENT:
            {
                const MetaCommentAction* pA = static_cast<const MetaCommentAction*>(pMA);
                if ( pA->GetComment().equalsIgnoreAsciiCase("XGRAD_SEQ_BEGIN") )
                {
                    const MetaGradientExAction* pGradAction = nullptr;
                    while( ++nCurAction < nCount )
                    {
                        MetaAction* pAction = rMtf.GetAction( nCurAction );
                        if( pAction->GetType() == MetaActionType::GRADIENTEX )
                            pGradAction = static_cast<const MetaGradientExAction*>(pAction);
                        else if( ( pAction->GetType() == MetaActionType::COMMENT ) &&
                                 ( static_cast<const MetaCommentAction*>(pAction)->GetComment().equalsIgnoreAsciiCase("XGRAD_SEQ_END") ) )
                        {
                            break;
                        }
                    }
                    if( pGradAction )
                        ImplWriteGradient( pGradAction->GetPolyPolygon(), pGradAction->GetGradient(), rVDev );
                }
                else if ( pA->GetComment().equals("XPATHFILL_SEQ_END") )
                {
                    if ( aFillPath.Count() )
                    {
                        aFillPath = tools::PolyPolygon();
                        ImplWriteLine( "gr" );
                    }
                }
                else
                {
                    const sal_uInt8* pData = pA->GetData();
                    if ( pData )
                    {
                        SvMemoryStream  aMemStm( const_cast<sal_uInt8 *>(pData), pA->GetDataSize(), StreamMode::READ );
                        bool        bSkipSequence = false;
                        OString sSeqEnd;

                        if( pA->GetComment().equals( "XPATHSTROKE_SEQ_BEGIN" ) )
                        {
                            sSeqEnd = "XPATHSTROKE_SEQ_END";
                            SvtGraphicStroke aStroke;
                            ReadSvtGraphicStroke( aMemStm, aStroke );

                            tools::Polygon aPath;
                            aStroke.getPath( aPath );

                            tools::PolyPolygon aStartArrow;
                            tools::PolyPolygon aEndArrow;
//                          double fTransparency( aStroke.getTransparency() );
                            double fStrokeWidth( aStroke.getStrokeWidth() );
                            SvtGraphicStroke::JoinType eJT( aStroke.getJoinType() );
                            SvtGraphicStroke::DashArray l_aDashArray;

                            aStroke.getStartArrow( aStartArrow );
                            aStroke.getEndArrow( aEndArrow );
                            aStroke.getDashArray( l_aDashArray );

                            bSkipSequence = true;
                            if ( l_aDashArray.size() > 11 ) // ps dasharray limit is 11
                                bSkipSequence = false;
                            if ( aStartArrow.Count() || aEndArrow.Count() )
                                bSkipSequence = false;
                            if ( (sal_uInt32)eJT > 2 )
                                bSkipSequence = false;
                            if ( l_aDashArray.size() && ( fStrokeWidth != 0.0 ) )
                                bSkipSequence = false;
                            if ( bSkipSequence )
                            {
                                ImplWriteLineInfo( fStrokeWidth, aStroke.getMiterLimit(),
                                                    aStroke.getCapType(), eJT, l_aDashArray );
                                ImplPolyLine( aPath );
                            }
                        }
                        else if (pA->GetComment().equals("XPATHFILL_SEQ_BEGIN"))
                        {
                            sSeqEnd = "XPATHFILL_SEQ_END";
                            SvtGraphicFill aFill;
                            ReadSvtGraphicFill( aMemStm, aFill );
                            switch( aFill.getFillType() )
                            {
                                case SvtGraphicFill::fillSolid :
                                {
                                    bSkipSequence = true;
                                    tools::PolyPolygon aPolyPoly;
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
                                                mpPS->WriteCharPtr( "p" );
                                                mnCursorPos += 2;
                                                ImplExecMode( PS_RET );
                                            }
                                        }
                                        mpPS->WriteCharPtr( "p ef" );
                                        mnCursorPos += 4;
                                        ImplExecMode( PS_RET );
                                    }
                                }
                                break;

                                case SvtGraphicFill::fillTexture :
                                {
                                    aFill.getPath( aFillPath );

                                    /* normally an object filling is consisting of three MetaActions:
                                        MetaBitmapAction        using RasterOp xor,
                                        MetaPolyPolygonAction   using RasterOp rop_0
                                        MetaBitmapAction        using RasterOp xor

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

                                    bool bOk = true;
                                    while( bOk && ( ++nCurAction < nCount ) )
                                    {
                                        MetaAction* pAction = rMtf.GetAction( nCurAction );
                                        switch( pAction->GetType() )
                                        {
                                            case MetaActionType::BMPSCALE :
                                            case MetaActionType::BMPSCALEPART :
                                            case MetaActionType::BMPEXSCALE :
                                            case MetaActionType::BMPEXSCALEPART :
                                            {
                                                nBitmapCount++;
                                                nBitmapAction = nCurAction;
                                            }
                                            break;
                                            case MetaActionType::COMMENT :
                                            {
                                                if (static_cast<const MetaCommentAction*>(pAction)->GetComment().equals("XPATHFILL_SEQ_END"))
                                                    bOk = false;
                                            }
                                            break;
                                            default: break;
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
                                        aFillPath = tools::PolyPolygon();
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
                                if ( pMA->GetType() == MetaActionType::COMMENT )
                                {
                                    OString sComment( static_cast<MetaCommentAction*>(pMA)->GetComment() );
                                    if ( sComment.equals( sSeqEnd ) )
                                        break;
                                }
                            }
                        }
                    }
                }
            }
            break;
            default: break;
        }
    }
}


inline void PSWriter::ImplWritePoint( const Point& rPoint, sal_uInt32 nMode )
{
    ImplWriteDouble( rPoint.X() );
    ImplWriteDouble( rPoint.Y(), nMode );
}


void PSWriter::ImplMoveTo( const Point& rPoint, sal_uInt32 nMode )
{
    ImplWritePoint( rPoint );
    ImplWriteByte( 'm' );
    ImplExecMode( nMode );
}


void PSWriter::ImplLineTo( const Point& rPoint, sal_uInt32 nMode )
{
    ImplWritePoint( rPoint );
    ImplWriteByte( 'l' );
    ImplExecMode( nMode );
}


void PSWriter::ImplCurveTo( const Point& rP1, const Point& rP2, const Point& rP3, sal_uInt32 nMode )
{
    ImplWritePoint( rP1 );
    ImplWritePoint( rP2 );
    ImplWritePoint( rP3 );
    mpPS->WriteCharPtr( "ct " );
    ImplExecMode( nMode );
}


void PSWriter::ImplTranslate( const double& fX, const double& fY, sal_uInt32 nMode )
{
    ImplWriteDouble( fX );
    ImplWriteDouble( fY );
    ImplWriteByte( 't' );
    ImplExecMode( nMode );
}


void PSWriter::ImplScale( const double& fX, const double& fY, sal_uInt32 nMode )
{
    ImplWriteDouble( fX );
    ImplWriteDouble( fY );
    ImplWriteByte( 's' );
    ImplExecMode( nMode );
}


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
        mpPS->WriteCharPtr( "0 rl 0 " );
        ImplWriteDouble( nHeight );
        mpPS->WriteCharPtr( "rl " );
        ImplWriteDouble( nWidth );
        mpPS->WriteCharPtr( "neg 0 rl " );
        ImplClosePathDraw();
    }
    mpPS->WriteUChar( 10 );
    mnCursorPos = 0;
}


void PSWriter::ImplRectFill( const Rectangle & rRect )
{
    double nWidth = rRect.GetWidth();
    double nHeight = rRect.GetHeight();

    ImplWriteFillColor( PS_SPACE );
    ImplMoveTo( rRect.TopLeft() );
    ImplWriteDouble( nWidth );
    mpPS->WriteCharPtr( "0 rl 0 " );
    ImplWriteDouble( nHeight );
    mpPS->WriteCharPtr( "rl " );
    ImplWriteDouble( nWidth );
    mpPS->WriteCharPtr( "neg 0 rl ef " );
    mpPS->WriteCharPtr( "p ef" );
    mnCursorPos += 2;
    ImplExecMode( PS_RET );
}


void PSWriter::ImplAddPath( const tools::Polygon & rPolygon )
{
    sal_uInt16 nPointCount = rPolygon.GetSize();
    if ( nPointCount > 1 )
    {
        sal_uInt16 i = 1;
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


void PSWriter::ImplIntersect( const tools::PolyPolygon& rPolyPoly )
{
    sal_uInt16 i, nPolyCount = rPolyPoly.Count();
    for ( i = 0; i < nPolyCount; )
    {
        ImplAddPath( rPolyPoly.GetObject( i ) );
        if ( ++i < nPolyCount )
        {
            mpPS->WriteCharPtr( "p" );
            mnCursorPos += 2;
            ImplExecMode( PS_RET );
        }
    }
    ImplWriteLine( "eoclip newpath" );
}


void PSWriter::ImplWriteGradient( const tools::PolyPolygon& rPolyPoly, const Gradient& rGradient, VirtualDevice& rVDev )
{
    ScopedVclPtrInstance< VirtualDevice > l_pVDev;
    GDIMetaFile     aTmpMtf;
    l_pVDev->SetMapMode( rVDev.GetMapMode() );
    l_pVDev->AddGradientActions( rPolyPoly.GetBoundRect(), rGradient, aTmpMtf );
    ImplWriteActions( aTmpMtf, rVDev );
}


void PSWriter::ImplPolyPoly( const tools::PolyPolygon & rPolyPoly, bool bTextOutline )
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
                    mpPS->WriteCharPtr( "p" );
                    mnCursorPos += 2;
                    ImplExecMode( PS_RET );
                }
            }
            mpPS->WriteCharPtr( "p ef" );
            mnCursorPos += 4;
            ImplExecMode( PS_RET );
        }
        if ( bLineColor )
        {
            ImplWriteLineColor( PS_SPACE );
            for ( i = 0; i < nPolyCount; i++ )
                ImplAddPath( rPolyPoly.GetObject( i ) );
            ImplClosePathDraw();
        }
    }
}


void PSWriter::ImplPolyLine( const tools::Polygon & rPoly )
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

            // #104645# explicitly close path if polygon is closed
            if( rPoly[ 0 ] == rPoly[ nPointCount-1 ] )
                ImplClosePathDraw();
            else
                ImplPathDraw();
        }
    }
}


void PSWriter::ImplSetClipRegion( vcl::Region& rClipRegion )
{
    if ( !rClipRegion.IsEmpty() )
    {
        RectangleVector aRectangles;
        rClipRegion.GetRegionRectangles(aRectangles);

        for(RectangleVector::const_iterator aRectIter(aRectangles.begin()); aRectIter != aRectangles.end(); ++aRectIter)
        {
            double nX1(aRectIter->Left());
            double nY1(aRectIter->Top());
            double nX2(aRectIter->Right());
            double nY2(aRectIter->Bottom());

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
        }

        //Rectangle     aRect;
        //RegionHandle  hRegionHandle = rClipRegion.BeginEnumRects();
        //
        //while ( rClipRegion.GetEnumRects( hRegionHandle, aRect ) )
        //{
        //  double nX1 = aRect.Left();
        //  double nY1 = aRect.Top();
        //  double nX2 = aRect.Right();
        //  double nY2 = aRect.Bottom();
        //  ImplWriteDouble( nX1 );
        //  ImplWriteDouble( nY1 );
        //  ImplWriteByte( 'm' );
        //  ImplWriteDouble( nX2 );
        //  ImplWriteDouble( nY1 );
        //  ImplWriteByte( 'l' );
        //  ImplWriteDouble( nX2 );
        //  ImplWriteDouble( nY2 );
        //  ImplWriteByte( 'l' );
        //  ImplWriteDouble( nX1 );
        //  ImplWriteDouble( nY2 );
        //  ImplWriteByte( 'l' );
        //  ImplWriteDouble( nX1 );
        //  ImplWriteDouble( nY1 );
        //  ImplWriteByte( 'l', PS_SPACE | PS_WRAP );
        //};
        //rClipRegion.EndEnumRects( hRegionHandle );
        ImplWriteLine( "eoclip newpath" );
    }
}


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

    sal_Int32   nHeightOrg = pBitmap->GetSizePixel().Height();
    sal_Int32   nHeightLeft = nHeightOrg;
    long    nWidth = pBitmap->GetSizePixel().Width();
    Point   aSourcePos( rPoint );

    while ( nHeightLeft )
    {
        Bitmap  aTileBitmap( *pBitmap );
        long    nHeight = nHeightLeft;
        double  nYHeight = nYHeightOrg;

        bool    bDoTrans = false;

        Rectangle   aRect;
        vcl::Region      aRegion;

        if ( pMaskBitmap )
        {
            bDoTrans = true;
            while (true)
            {
                if ( mnLevel == 1 )
                {
                    if ( nHeight > 10 )
                        nHeight = 8;
                }
                aRect = Rectangle( Point( 0, nHeightOrg - nHeightLeft ), Size( (long)nWidth, (long)nHeight ) );
                aRegion = vcl::Region( pMaskBitmap->CreateRegion( COL_BLACK, aRect ) );

                if( mnLevel == 1 )
                {
                    RectangleVector aRectangleVector;
                    aRegion.GetRegionRectangles(aRectangleVector);

                    if ( aRectangleVector.size() * 5 > 1000 )
                    {
                        nHeight >>= 1;
                        if ( nHeight < 2 )
                            return;
                        continue;
                    }
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

            RectangleVector aRectangles;
            aRegion.GetRegionRectangles(aRectangles);
            const long nMoveVertical(nHeightLeft - nHeightOrg);

            for(RectangleVector::iterator aRectIter(aRectangles.begin()); aRectIter != aRectangles.end(); ++aRectIter)
            {
                aRectIter->Move(0, nMoveVertical);

                ImplWriteLong( aRectIter->Left() );
                ImplWriteLong( aRectIter->Top() );
                ImplWriteByte( 'm' );
                ImplWriteLong( aRectIter->Right() + 1 );
                ImplWriteLong( aRectIter->Top() );
                ImplWriteByte( 'l' );
                ImplWriteLong( aRectIter->Right() + 1 );
                ImplWriteLong( aRectIter->Bottom() + 1 );
                ImplWriteByte( 'l' );
                ImplWriteLong( aRectIter->Left() );
                ImplWriteLong( aRectIter->Bottom() + 1 );
                ImplWriteByte( 'l' );
                ImplWriteByte( 'p', PS_SPACE | PS_WRAP );
            }

            //RegionHandle  hRegionHandle = aRegion.BeginEnumRects();
            //
            //while ( aRegion.GetEnumRects( hRegionHandle, aRect ) )
            //{
            //  aRect.Move( 0, - ( nHeightOrg - nHeightLeft ) );
            //  ImplWriteLong( aRect.Left() );
            //  ImplWriteLong( aRect.Top() );
            //  ImplWriteByte( 'm' );
            //  ImplWriteLong( aRect.Right() + 1 );
            //  ImplWriteLong( aRect.Top() );
            //  ImplWriteByte( 'l' );
            //  ImplWriteLong( aRect.Right() + 1 );
            //  ImplWriteLong( aRect.Bottom() + 1 );
            //  ImplWriteByte( 'l' );
            //  ImplWriteLong( aRect.Left() );
            //  ImplWriteLong( aRect.Bottom() + 1 );
            //  ImplWriteByte( 'l' );
            //  ImplWriteByte( 'p', PS_SPACE | PS_WRAP );
            //};
            //aRegion.EndEnumRects( hRegionHandle );
            ImplWriteLine( "eoclip newpath" );
            ImplWriteLine( "pom" );
        }
        BitmapReadAccess* pAcc = aTileBitmap.AcquireReadAccess();

        if (!bDoTrans )
            ImplWriteLine( "pum" );

        ImplTranslate( aSourcePos.X(), aSourcePos.Y() + nYHeight );
        ImplScale( nXWidth, nYHeight );
        if ( mnLevel == 1 )                 // level 1 is always grayscale !!!
        {
            ImplWriteLong( nWidth );
            ImplWriteLong( nHeight );
            mpPS->WriteCharPtr( "8 [" );
            ImplWriteLong( nWidth );
            mpPS->WriteCharPtr( "0 0 " );
            ImplWriteLong( -nHeight );
            ImplWriteLong( 0 );
            ImplWriteLong( nHeight );
            ImplWriteLine( "]" );
            mpPS->WriteCharPtr( "{currentfile " );
            ImplWriteLong( nWidth );
            ImplWriteLine( "string readhexstring pop}" );
            ImplWriteLine( "image" );
            for ( long y = 0; y < nHeight; y++ )
            {
                for ( long x = 0; x < nWidth; x++ )
                {
                    ImplWriteHexByte( pAcc->GetPixelIndex( y, x ) );
                }
            }
            mpPS->WriteUChar( 10 );
        }
        else    // Level 2
        {
            if ( mbGrayScale )
            {
                ImplWriteLine( "/DeviceGray setcolorspace" );
                ImplWriteLine( "<<" );
                ImplWriteLine( "/ImageType 1" );
                mpPS->WriteCharPtr( "/Width " );
                ImplWriteLong( nWidth, PS_RET );
                mpPS->WriteCharPtr( "/Height " );
                ImplWriteLong( nHeight, PS_RET );
                ImplWriteLine( "/BitsPerComponent 8" );
                ImplWriteLine( "/Decode[0 1]" );
                mpPS->WriteCharPtr( "/ImageMatrix[" );
                ImplWriteLong( nWidth );
                mpPS->WriteCharPtr( "0 0 " );
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
                            Compress( pAcc->GetPixelIndex( y, x ) );
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
                            ImplWriteHexByte( pAcc->GetPixelIndex( y, x ) );
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
                    for ( sal_uInt16 i = 0; i < pAcc->GetPaletteEntryCount(); i++ )
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
                    mpPS->WriteCharPtr( "/Width " );
                    ImplWriteLong( nWidth, PS_RET );
                    mpPS->WriteCharPtr( "/Height " );
                    ImplWriteLong( nHeight, PS_RET );
                    ImplWriteLine( "/BitsPerComponent 8" );
                    ImplWriteLine( "/Decode[0 255]" );
                    mpPS->WriteCharPtr( "/ImageMatrix[" );
                    ImplWriteLong( nWidth );
                    mpPS->WriteCharPtr( "0 0 " );
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
                                Compress( pAcc->GetPixelIndex( y, x ) );
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
                                ImplWriteHexByte( pAcc->GetPixelIndex( y, x ) );
                            }
                        }
                    }
                }
                else // 24 bit color
                {
                    ImplWriteLine( "/DeviceRGB setcolorspace" );
                    ImplWriteLine( "<<" );
                    ImplWriteLine( "/ImageType 1" );
                    mpPS->WriteCharPtr( "/Width " );
                    ImplWriteLong( nWidth, PS_RET );
                    mpPS->WriteCharPtr( "/Height " );
                    ImplWriteLong( nHeight, PS_RET );
                    ImplWriteLine( "/BitsPerComponent 8" );
                    ImplWriteLine( "/Decode[0 1 0 1 0 1]" );
                    mpPS->WriteCharPtr( "/ImageMatrix[" );
                    ImplWriteLong( nWidth );
                    mpPS->WriteCharPtr( "0 0 " );
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

        Bitmap::ReleaseAccess( pAcc );
        nHeightLeft -= nHeight;
        if ( nHeightLeft )
        {
            nHeightLeft++;
            aSourcePos.Y() = (long) ( rPoint.Y() + ( nYHeightOrg * ( nHeightOrg - nHeightLeft ) ) / nHeightOrg );
        }
    }
}


void PSWriter::ImplWriteCharacter( sal_Char nChar )
{
    switch( nChar )
    {
        case '(' :
        case ')' :
        case '\\' :
            ImplWriteByte( (sal_uInt8)'\\', PS_NONE );
    }
    ImplWriteByte( (sal_uInt8)nChar, PS_NONE );
}


void PSWriter::ImplWriteString( const OString& rString, VirtualDevice& rVDev, const long* pDXArry, bool bStretch )
{
    sal_Int32 nLen = rString.getLength();
    if ( nLen )
    {
        if ( pDXArry )
        {
            double nx = 0;

            for (sal_Int32 i = 0; i < nLen; ++i)
            {
                if ( i > 0 )
                    nx = pDXArry[ i - 1 ];
                ImplWriteDouble( ( bStretch ) ? nx : rVDev.GetTextWidth( OUString(rString[i]) ) );
                ImplWriteDouble( nx );
                ImplWriteLine( "(", PS_NONE );
                ImplWriteCharacter( rString[i] );
                ImplWriteLine( ") bs" );
            }
        }
        else
        {
            ImplWriteByte( '(', PS_NONE );
            for (sal_Int32 i = 0; i < nLen; ++i)
                ImplWriteCharacter( rString[i] );
            ImplWriteLine( ") sw" );
        }
    }
}


void PSWriter::ImplText( const OUString& rUniString, const Point& rPos, const long* pDXArry, sal_Int32 nWidth, VirtualDevice& rVDev )
{
    if ( rUniString.isEmpty() )
        return;
    if ( mnTextMode == 0 )  // using glyph outlines
    {
        vcl::Font    aNotRotatedFont( maFont );
        aNotRotatedFont.SetOrientation( 0 );

        ScopedVclPtrInstance< VirtualDevice > pVirDev(DeviceFormat::BITMASK);
        pVirDev->SetMapMode( rVDev.GetMapMode() );
        pVirDev->SetFont( aNotRotatedFont );
        pVirDev->SetTextAlign( eTextAlign );

        sal_Int16 nRotation = maFont.GetOrientation();
        tools::Polygon aPolyDummy( 1 );

        Point aPos( rPos );
        if ( nRotation )
        {
            aPolyDummy.SetPoint( aPos, 0 );
            aPolyDummy.Rotate( rPos, nRotation );
            aPos = aPolyDummy.GetPoint( 0 );
        }
        bool bOldLineColor = bLineColor;
        bLineColor = false;
        std::vector<tools::PolyPolygon> aPolyPolyVec;
        if ( pVirDev->GetTextOutlines( aPolyPolyVec, rUniString, 0, 0, -1, true, nWidth, pDXArry ) )
        {
            // always adjust text position to match baseline alignment
            ImplWriteLine( "pum" );
            ImplWriteDouble( aPos.X() );
            ImplWriteDouble( aPos.Y() );
            ImplWriteLine( "t" );
            if ( nRotation )
            {
                ImplWriteF( nRotation, 1 );
                mpPS->WriteCharPtr( "r " );
            }
            std::vector<tools::PolyPolygon>::iterator aIter( aPolyPolyVec.begin() );
            while ( aIter != aPolyPolyVec.end() )
                ImplPolyPoly( *aIter++, true );
            ImplWriteLine( "pom" );
        }
        bLineColor = bOldLineColor;
    }
    else if ( ( mnTextMode == 1 ) || ( mnTextMode == 2 ) )  // normal text output
    {
        if ( mnTextMode == 2 )  // forcing output one complete text packet, by
            pDXArry = nullptr;     // ignoring the kerning array
        ImplSetAttrForText( rPos );
        OString aStr(OUStringToOString(rUniString,
            maFont.GetCharSet()));
        ImplWriteString( aStr, rVDev, pDXArry, nWidth != 0 );
        if ( maFont.GetOrientation() )
            ImplWriteLine( "gr" );
    }
}


void PSWriter::ImplSetAttrForText( const Point& rPoint )
{
    Point aPoint( rPoint );

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
        ImplWriteDouble( aSize.Height() );
        mpPS->WriteCharPtr( "sf " );
    }
    if ( eTextAlign != ALIGN_BASELINE )
    {                                                       // PostScript does not know about FontAlignment
        if ( eTextAlign == ALIGN_TOP )                      // -> so I assume that
            aPoint.Y() += ( aSize.Height() * 4 / 5 );       // the area under the baseline
        else if ( eTextAlign == ALIGN_BOTTOM )              // is about 20% of the font size
            aPoint.Y() -= ( aSize.Height() / 5 );
    }
    ImplMoveTo( aPoint );
    if ( nRotation )
    {
        mpPS->WriteCharPtr( "gs " );
        ImplWriteF( nRotation, 1 );
        mpPS->WriteCharPtr( "r " );
    }
}


void PSWriter::ImplDefineFont( const char* pOriginalName, const char* pItalic )
{
    mpPS->WriteUChar( '/' );             //convert the font pOriginalName using ISOLatin1Encoding
    mpPS->WriteCharPtr( pOriginalName );
    switch ( maFont.GetWeight() )
    {
        case WEIGHT_SEMIBOLD :
        case WEIGHT_BOLD :
        case WEIGHT_ULTRABOLD :
        case WEIGHT_BLACK :
            mpPS->WriteCharPtr( "-Bold" );
            if ( maFont.GetItalic() != ITALIC_NONE )
                mpPS->WriteCharPtr( pItalic );
            break;
        default:
            if ( maFont.GetItalic() != ITALIC_NONE )
                mpPS->WriteCharPtr( pItalic );
            break;
    }
    ImplWriteLine( " f" );
}


void PSWriter::ImplClosePathDraw( sal_uLong nMode )
{
    mpPS->WriteCharPtr( "pc" );
    mnCursorPos += 2;
    ImplExecMode( nMode );
}

void PSWriter::ImplPathDraw()
{
    mpPS->WriteCharPtr( "ps" );
    mnCursorPos += 2;
    ImplExecMode( PS_RET );
}


inline void PSWriter::ImplWriteLineColor( sal_uLong nMode )
{
    if ( aColor != aLineColor )
    {
        aColor = aLineColor;
        ImplWriteColor( nMode );
    }
}
inline void PSWriter::ImplWriteFillColor( sal_uLong nMode )
{
    if ( aColor != aFillColor )
    {
        aColor = aFillColor;
        ImplWriteColor( nMode );
    }
}
inline void PSWriter::ImplWriteTextColor( sal_uLong nMode )
{
    if ( aColor != aTextColor )
    {
        aColor = aTextColor;
        ImplWriteColor( nMode );
    }
}

void PSWriter::ImplWriteColor( sal_uLong nMode )
{
    if ( mbGrayScale )
    {
        // writes the Color (grayscale) as a Number from 0.000 up to 1.000

        ImplWriteF( 1000 * ( (sal_uInt8)aColor.GetRed() * 77 + (sal_uInt8)aColor.GetGreen() * 151 +
            (sal_uInt8)aColor.GetBlue() * 28 + 1 ) / 65536, 3, nMode );
    }
    else
    {
        ImplWriteB1 ( (sal_uInt8)aColor.GetRed() );
        ImplWriteB1 ( (sal_uInt8)aColor.GetGreen() );
        ImplWriteB1 ( (sal_uInt8)aColor.GetBlue() );
    }
    mpPS->WriteCharPtr( "c" );                               // ( c is defined as setrgbcolor or setgray )
    ImplExecMode( nMode );
}

void PSWriter::ImplGetMapMode( const MapMode& rMapMode )
{
    ImplWriteLine( "tm setmatrix" );
    double fMul = rMapMode.GetUnitMultiplier();
    double fScaleX = (double)rMapMode.GetScaleX() * fMul;
    double fScaleY = (double)rMapMode.GetScaleY() * fMul;
    ImplTranslate( rMapMode.GetOrigin().X() * fScaleX, rMapMode.GetOrigin().Y() * fScaleY );
    ImplScale( fScaleX, fScaleY );
}


inline void PSWriter::ImplExecMode( sal_uLong nMode )
{
    if ( nMode & PS_WRAP )
    {
        if ( mnCursorPos >= PS_LINESIZE )
        {
            mnCursorPos = 0;
            mpPS->WriteUChar( 0xa );
            return;
        }
    }
    if ( nMode & PS_SPACE )
    {
            mpPS->WriteUChar( 32 );
            mnCursorPos++;
    }
    if ( nMode & PS_RET )
    {
        mpPS->WriteUChar( 0xa );
        mnCursorPos = 0;
    }
}


inline void PSWriter::ImplWriteLine( const char* pString, sal_uLong nMode )
{
    sal_uLong i = 0;
    while ( pString[ i ] )
    {
        mpPS->WriteUChar( pString[ i++ ] );
    }
    mnCursorPos += i;
    ImplExecMode( nMode );
}


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
        ImplWriteLong( (sal_Int32)eLineCap );
        ImplWriteLine( "lc", PS_SPACE );
    }
    if ( eJoinType != eJoin )
    {
        eJoinType = eJoin;
        ImplWriteLong( (sal_Int32)eJoinType );
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


void PSWriter::ImplWriteLineInfo( const LineInfo& rLineInfo )
{
    SvtGraphicStroke::DashArray l_aDashArray;
    if ( rLineInfo.GetStyle() == LINE_DASH )
        l_aDashArray.push_back( 2 );
    const double fLWidth(( ( rLineInfo.GetWidth() + 1 ) + ( rLineInfo.GetWidth() + 1 ) ) * 0.5);
    SvtGraphicStroke::JoinType aJoinType(SvtGraphicStroke::joinMiter);
    SvtGraphicStroke::CapType aCapType(SvtGraphicStroke::capButt);

    switch(rLineInfo.GetLineJoin())
    {
        default: // B2DLineJoin::NONE, B2DLineJoin::Middle
            // do NOT use SvtGraphicStroke::joinNone here
            // since it will be written as numerical value directly
            // and is NOT a valid EPS value
            break;
        case basegfx::B2DLineJoin::Miter:
            aJoinType = SvtGraphicStroke::joinMiter;
            break;
        case basegfx::B2DLineJoin::Bevel:
            aJoinType = SvtGraphicStroke::joinBevel;
            break;
        case basegfx::B2DLineJoin::Round:
            aJoinType = SvtGraphicStroke::joinRound;
            break;
    }
    switch(rLineInfo.GetLineCap())
    {
        default: /* css::drawing::LineCap_BUTT */
        {
            aCapType = SvtGraphicStroke::capButt;
            break;
        }
        case css::drawing::LineCap_ROUND:
        {
            aCapType = SvtGraphicStroke::capRound;
            break;
        }
        case css::drawing::LineCap_SQUARE:
        {
            aCapType = SvtGraphicStroke::capSquare;
            break;
        }
    }

    ImplWriteLineInfo( fLWidth, fMiterLimit, aCapType, aJoinType, l_aDashArray );
}


void PSWriter::ImplWriteLong(sal_Int32 nNumber, sal_uLong nMode)
{
    const OString aNumber(OString::number(nNumber));
    mnCursorPos += aNumber.getLength();
    mpPS->WriteCharPtr( aNumber.getStr() );
    ImplExecMode(nMode);
}


void PSWriter::ImplWriteDouble( double fNumber, sal_uLong nMode )
{
    sal_Int32   nPTemp = (sal_Int32)fNumber;
    sal_Int32   nATemp = labs( (sal_Int32)( ( fNumber - nPTemp ) * 100000 ) );

    if ( !nPTemp && nATemp && ( fNumber < 0.0 ) )
        mpPS->WriteChar( '-' );

    const OString aNumber1(OString::number(nPTemp));
    mpPS->WriteCharPtr( aNumber1.getStr() );
    mnCursorPos += aNumber1.getLength();

    if ( nATemp )
    {
        int zCount = 0;
        mpPS->WriteUChar( '.' );
        mnCursorPos++;
        const OString aNumber2(OString::number(nATemp));

        sal_Int16 n, nLen = aNumber2.getLength();
        if ( nLen < 8 )
        {
            mnCursorPos += 6 - nLen;
            for ( n = 0; n < ( 5 - nLen ); n++ )
            {
                mpPS->WriteUChar( '0' );
            }
        }
        mnCursorPos += nLen;
        for ( n = 0; n < nLen; n++ )
        {
            mpPS->WriteChar( aNumber2[n] );
            zCount--;
            if ( aNumber2[n] != '0' )
                zCount = 0;
        }
        if ( zCount )
            mpPS->SeekRel( zCount );
    }
    ImplExecMode( nMode );
}


// writes the number to stream: nNumber / ( 10^nCount )

void PSWriter::ImplWriteF( sal_Int32 nNumber, sal_uLong nCount, sal_uLong nMode )
{
    if ( nNumber < 0 )
    {
        mpPS->WriteUChar( '-' );
        nNumber = -nNumber;
        mnCursorPos++;
    }
    const OString aScaleFactor(OString::number(nNumber));
    sal_uLong nLen = aScaleFactor.getLength();
    long nStSize =  ( nCount + 1 ) - nLen;
    if ( nStSize >= 1 )
    {
        mpPS->WriteUChar( '0' );
        mnCursorPos++;
    }
    if ( nStSize >= 2 )
    {
        mpPS->WriteUChar( '.' );
        for ( long i = 1; i < nStSize; i++ )
        {
            mpPS->WriteUChar( '0' );
            mnCursorPos++;
        }
    }
    mnCursorPos += nLen;
    for( sal_uLong n = 0; n < nLen; n++  )
    {
        if ( n == nLen - nCount )
        {
            mpPS->WriteUChar( '.' );
            mnCursorPos++;
        }
        mpPS->WriteChar( aScaleFactor[n] );
    }
    ImplExecMode( nMode );
}


void PSWriter::ImplWriteByte( sal_uInt8 nNumb, sal_uLong nMode )
{
    mpPS->WriteUChar( nNumb );
    mnCursorPos++;
    ImplExecMode( nMode );
}


void PSWriter::ImplWriteHexByte( sal_uInt8 nNumb, sal_uLong nMode )
{
    if ( ( nNumb >> 4 ) > 9 )
        mpPS->WriteUChar( ( nNumb >> 4 ) + 'A' - 10 );
    else
        mpPS->WriteUChar( ( nNumb >> 4 ) + '0' );

    if ( ( nNumb & 0xf ) > 9 )
        mpPS->WriteUChar( ( nNumb & 0xf ) + 'A' - 10 );
    else
        mpPS->WriteUChar( ( nNumb & 0xf ) + '0' );
    mnCursorPos += 2;
    ImplExecMode( nMode );
}


// writes the sal_uInt8 nNumb as a Number from 0.000 up to 1.000

void PSWriter::ImplWriteB1( sal_uInt8 nNumb, sal_uLong nMode )
{
    ImplWriteF( 1000 * ( nNumb + 1 ) / 256 , 3, nMode );
}


inline void PSWriter::WriteBits( sal_uInt16 nCode, sal_uInt16 nCodeLen )
{
    dwShift |= ( nCode << ( nOffset - nCodeLen ) );
    nOffset -= nCodeLen;
    while ( nOffset < 24 )
    {
        ImplWriteHexByte( (sal_uInt8)( dwShift >> 24 ) );
        dwShift <<= 8;
        nOffset += 8;
    }
    if ( nCode == 257 && nOffset != 32 )
        ImplWriteHexByte( (sal_uInt8)( dwShift >> 24 ) );
}


void PSWriter::StartCompression()
{
    sal_uInt16 i;
    nDataSize = 8;

    nClearCode = 1 << nDataSize;
    nEOICode = nClearCode + 1;
    nTableSize = nEOICode + 1;
    nCodeSize = nDataSize + 1;

    nOffset = 32;                       // number of free unused in dwShift
    dwShift = 0;

    pTable = new PSLZWCTreeNode[ 4096 ];

    for ( i = 0; i < 4096; i++ )
    {
        pTable[ i ].pBrother = pTable[ i ].pFirstChild = nullptr;
        pTable[ i ].nValue = (sal_uInt8)( pTable[ i ].nCode = i );
    }
    pPrefix = nullptr;
    WriteBits( nClearCode, nCodeSize );
}


void PSWriter::Compress( sal_uInt8 nCompThis )
{
    PSLZWCTreeNode*     p;
    sal_uInt16              i;
    sal_uInt8               nV;

    if( !pPrefix )
    {
        pPrefix = pTable + nCompThis;
    }
    else
    {
        nV = nCompThis;
        for( p = pPrefix->pFirstChild; p != nullptr; p = p->pBrother )
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
                    pTable[ i ].pFirstChild = nullptr;

                nCodeSize = nDataSize + 1;
                nTableSize = nEOICode + 1;
            }
            else
            {
                if( nTableSize == (sal_uInt16)( ( 1 << nCodeSize ) - 1 ) )
                    nCodeSize++;

                p = pTable + ( nTableSize++ );
                p->pBrother = pPrefix->pFirstChild;
                pPrefix->pFirstChild = p;
                p->nValue = nV;
                p->pFirstChild = nullptr;
            }

            pPrefix = pTable + nV;
        }
    }
}


void PSWriter::EndCompression()
{
    if( pPrefix )
        WriteBits( pPrefix->nCode, nCodeSize );

    WriteBits( nEOICode, nCodeSize );
    delete[] pTable;
}


sal_uInt8* PSWriter::ImplSearchEntry( sal_uInt8* pSource, sal_uInt8 const * pDest, sal_uLong nComp, sal_uLong nSize )
{
    while ( nComp-- >= nSize )
    {
        sal_uLong i;
        for ( i = 0; i < nSize; i++ )
        {
            if ( ( pSource[i]&~0x20 ) != ( pDest[i]&~0x20 ) )
                break;
        }
        if ( i == nSize )
            return pSource;
        pSource++;
    }
    return nullptr;
}


bool PSWriter::ImplGetBoundingBox( double* nNumb, sal_uInt8* pSource, sal_uLong nSize )
{
    bool    bRetValue = false;
    sal_uLong   nBytesRead;

    if ( nSize < 256 )      // we assume that the file is greater than 256 bytes
        return false;

    if ( nSize < POSTSCRIPT_BOUNDINGSEARCH )
        nBytesRead = nSize;
    else
        nBytesRead = POSTSCRIPT_BOUNDINGSEARCH;

    sal_uInt8* pDest = ImplSearchEntry( pSource, reinterpret_cast<sal_uInt8 const *>("%%BoundingBox:"), nBytesRead, 14 );
    if ( pDest )
    {
        int     nSecurityCount = 100;   // only 100 bytes following the bounding box will be checked
        nNumb[0] = nNumb[1] = nNumb[2] = nNumb[3] = 0;
        pDest += 14;
        for ( int i = 0; ( i < 4 ) && nSecurityCount; i++ )
        {
            int     nDivision = 1;
            bool    bDivision = false;
            bool    bNegative = false;
            bool    bValid = true;

            while ( ( --nSecurityCount ) && ( ( *pDest == ' ' ) || ( *pDest == 0x9 ) ) )
                pDest++;
            sal_uInt8 nByte = *pDest;
            while ( nSecurityCount && ( nByte != ' ' ) && ( nByte != 0x9 ) && ( nByte != 0xd ) && ( nByte != 0xa ) )
            {
                switch ( nByte )
                {
                    case '.' :
                        if ( bDivision )
                            bValid = false;
                        else
                            bDivision = true;
                        break;
                    case '-' :
                        bNegative = true;
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
            bRetValue = true;
    }
    return bRetValue;
}

//================== GraphicExport - the exported function ===================

extern "C" SAL_DLLPUBLIC_EXPORT bool SAL_CALL
epsGraphicExport( SvStream & rStream, Graphic & rGraphic, FilterConfigItem* pFilterConfigItem )
{
    PSWriter aPSWriter;
    return aPSWriter.WritePS( rGraphic, rStream, pFilterConfigItem );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
