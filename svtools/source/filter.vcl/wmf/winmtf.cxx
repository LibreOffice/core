/*************************************************************************
 *
 *  $RCSfile: winmtf.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:59:00 $
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


#include "winmtf.hxx"
#include <vcl/metaact.hxx>

// ------------------------------------------------------------------------

WinMtfFontStyle::WinMtfFontStyle( LOGFONTW& rFont )
{
    Size    aFontSize( Size( 0, rFont.lfHeight ) );

    aFont.SetName( UniString( (char*)rFont.lfFaceName, RTL_TEXTENCODING_UTF8 ) );
    aFont.SetSize( aFontSize );

    CharSet eCharSet;
    switch ( rFont.lfCharSet )
    {
        case ANSI_CHARSET:
            eCharSet = RTL_TEXTENCODING_MS_1252;
        break;

        case SYMBOL_CHARSET:
            eCharSet = RTL_TEXTENCODING_SYMBOL;
        break;

        case OEM_CHARSET:
        case DEFAULT_CHARSET:
        case SHIFTJIS_CHARSET:
        case HANGEUL_CHARSET:
        case CHINESEBIG5_CHARSET:
        default:
            eCharSet = gsl_getSystemTextEncoding();
        break;
    }
    aFont.SetCharSet( eCharSet );

    FontFamily eFamily;
    switch ( rFont.lfPitchAndFamily & 0xf0 )
    {
        case FF_ROMAN:
            eFamily = FAMILY_ROMAN;
        break;

        case FF_SWISS:
            eFamily = FAMILY_SWISS;
        break;

        case FF_MODERN:
            eFamily = FAMILY_MODERN;
        break;

        case FF_SCRIPT:
            eFamily = FAMILY_SCRIPT;
        break;

        case FF_DECORATIVE:
             eFamily = FAMILY_DECORATIVE;
        break;

        default:
            eFamily = FAMILY_DONTKNOW;
        break;
    }
    aFont.SetFamily( eFamily );

    FontPitch ePitch;
    switch ( rFont.lfPitchAndFamily & 0x0f )
    {
        case FIXED_PITCH:
            ePitch = PITCH_FIXED;
        break;

        case DEFAULT_PITCH:
        case VARIABLE_PITCH:
        default:
            ePitch = PITCH_VARIABLE;
        break;
    }
    aFont.SetPitch( ePitch );

    FontWeight eWeight;
    if( rFont.lfWeight <= FW_THIN )
        eWeight = WEIGHT_THIN;
    else if( rFont.lfWeight <= FW_ULTRALIGHT )
        eWeight = WEIGHT_ULTRALIGHT;
    else if( rFont.lfWeight <= FW_LIGHT )
        eWeight = WEIGHT_LIGHT;
    else if( rFont.lfWeight <  FW_MEDIUM )
        eWeight = WEIGHT_NORMAL;
    else if( rFont.lfWeight == FW_MEDIUM )
        eWeight = WEIGHT_MEDIUM;
    else if( rFont.lfWeight <= FW_SEMIBOLD )
        eWeight = WEIGHT_SEMIBOLD;
    else if( rFont.lfWeight <= FW_BOLD )
        eWeight = WEIGHT_BOLD;
    else if( rFont.lfWeight <= FW_ULTRABOLD )
        eWeight = WEIGHT_ULTRABOLD;
    else
        eWeight = WEIGHT_BLACK;
    aFont.SetWeight( eWeight );

    if( rFont.lfItalic )
        aFont.SetItalic( ITALIC_NORMAL );

    if( rFont.lfUnderline )
        aFont.SetUnderline( UNDERLINE_SINGLE );

    if( rFont.lfStrikeOut )
        aFont.SetStrikeout( STRIKEOUT_SINGLE );

    if ( rFont.lfOrientation )
        aFont.SetOrientation( (short)rFont.lfOrientation );
    else
        aFont.SetOrientation( (short)rFont.lfEscapement );
};

// ------------------------------------------------------------------------

WinMtf::WinMtf( WinMtfOutput* pWinMtfOutput, SvStream& rStreamWMF, PFilterCallback pcallback, void * pcallerdata ) :
    pOut                ( pWinMtfOutput ),
    pCallback           ( pcallback ),
    pCallerData         ( pcallerdata ),
    pWMF                ( &rStreamWMF )
{
    SvLockBytes *pLB = pWMF->GetLockBytes();
    if ( pLB )
        pLB->SetSynchronMode( TRUE );

    nStartPos = pWMF->Tell();

    pOut->SetDevOrg( Point() );
}

// ------------------------------------------------------------------------

WinMtf::~WinMtf()
{
    delete pOut;
}

// ------------------------------------------------------------------------

BOOL WinMtf::Callback( USHORT nPercent )
{
    if ( pCallback != NULL )
    {
        if( (*pCallback)( pCallerData, nPercent ) )
        {
            pWMF->SetError( SVSTREAM_FILEFORMAT_ERROR );
            return TRUE;
        }
    }
    return FALSE;
}

// ------------------------------------------------------------------------

Color WinMtf::ReadColor()
{
    UINT32 nColor;
    *pWMF >> nColor;
    return Color( (BYTE)nColor, (BYTE)( nColor >> 8 ), (BYTE)( nColor >> 16 ) );
};

//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------

WinMtfOutput::WinMtfOutput() :
    mnActTextAlign      ( TA_LEFT | TA_TOP | TA_NOUPDATECP ),
    mnBkMode            ( OPAQUE ),
    maBkColor           ( COL_WHITE ),
    mbNopMode           ( FALSE ),
    mbFontChanged       ( FALSE ),
    maActPos            ( Point() ),
    meRasterOp          ( ROP_OVERPAINT ),
    mnEntrys            ( 16 )
{
    maFont.SetCharSet( gsl_getSystemTextEncoding() );
    mpGDIObj = new GDIObj*[ mnEntrys ];
    for ( UINT32 i = 0; i < mnEntrys; i++ )
    {
        mpGDIObj[ i ] = NULL;
    }
};

//-----------------------------------------------------------------------------------

WinMtfOutput::~WinMtfOutput()
{
    while( maSaveStack.Count() )
        delete maSaveStack.Pop();

    for ( UINT32 i = 0; i < mnEntrys; i++ )
    {
        delete mpGDIObj[ i ];
    }
    delete mpGDIObj;
};

//-----------------------------------------------------------------------------------

Point WinMtfOutput::ImplMap( const Point& rPt )
{
    if( mnWinExtX && mnWinExtY )
    {
        return Point( FRound( ( ( (double) rPt.X() - mnWinOrgX ) * mnDevWidth / mnWinExtX + mnDevOrgX ) * maXForm.eM11 ),
                      FRound( ( ( (double) rPt.Y() - mnWinOrgY ) * mnDevHeight / mnWinExtY + mnDevOrgY ) * maXForm.eM12 ) );
    }
    else
        return Point();
};

// ------------------------------------------------------------------------

Size WinMtfOutput::ImplMap( const Size& rSz )
{
    if( mnWinExtX && mnWinExtY )
    {
        return Size( FRound( ( (double) rSz.Width() * mnDevWidth / mnWinExtX ) * maXForm.eM11 ),
                     FRound( ( (double) rSz.Height() * mnDevHeight / mnWinExtY ) * maXForm.eM12 ) );
    }
    else
        return Size();
}

//-----------------------------------------------------------------------------------

Rectangle WinMtfOutput::ImplMap( const Rectangle& rRect )
{
    return Rectangle( ImplMap( rRect.TopLeft() ), ImplMap( rRect.GetSize() ) );
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::ImplMap( Font& rFont )
{
    // !!! HACK: Wir setzen die Breite jetzt immer auf Null,
    // da OS die Breite unterschiedlich interpretieren;
    // muss spaeter in SV portabel gemacht werden ( KA 08.02.96 )
    Size  aFontSize = ImplMap ( rFont.GetSize() );

    if( aFontSize.Height() < 0 )
        aFontSize.Height() *= -1;

    rFont.SetSize( Size( 0, aFontSize.Height() ) );

    if( ( mnWinExtX * mnWinExtY ) < 0 )
        rFont.SetOrientation( 3600 - rFont.GetOrientation() );
}

//-----------------------------------------------------------------------------------

Polygon& WinMtfOutput::ImplMap( Polygon& rPolygon )
{
    UINT16 nPoints = rPolygon.GetSize();
    for ( UINT16 i = 0; i < nPoints; i++ )
    {
        rPolygon[ i ] = ImplMap( rPolygon[ i ] );
    }
    return rPolygon;
}

//-----------------------------------------------------------------------------------

PolyPolygon& WinMtfOutput::ImplMap( PolyPolygon& rPolyPolygon )
{
    UINT16 nPolys = rPolyPolygon.Count();
    for ( UINT16 i = 0; i < nPolys; ImplMap( rPolyPolygon[ i++ ] ) );
    return rPolyPolygon;
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::SelectObject( INT32 nIndex )
{
    GDIObj* pGDIObj = NULL;

    if ( nIndex & ENHMETA_STOCK_OBJECT )
        pGDIObj = new GDIObj();
    else
    {
        nIndex &= 0xffff;       // zur Sicherheit: mehr als 65535 nicht zulassen

        if ( (UINT32)nIndex < mnEntrys )
            pGDIObj = mpGDIObj[ nIndex ];
    }

    if( pGDIObj == NULL )
        return;

    if ( nIndex & ENHMETA_STOCK_OBJECT )
    {
        UINT16 nStockId = (BYTE)nIndex;
        switch( nStockId )
        {
            case WHITE_BRUSH :
            {
                pGDIObj->Set( GDI_BRUSH, new WinMtfFillStyle( Color( COL_WHITE ) ) );
            }
            break;
            case LTGRAY_BRUSH :
            {
                pGDIObj->Set( GDI_BRUSH, new WinMtfFillStyle( Color( COL_LIGHTGRAY ) ) );
            }
            break;
            case GRAY_BRUSH :
            case DKGRAY_BRUSH :
            {
                pGDIObj->Set( GDI_BRUSH, new WinMtfFillStyle( Color( COL_GRAY ) ) );
            }
            break;
            case BLACK_BRUSH :
            {
                pGDIObj->Set( GDI_BRUSH, new WinMtfFillStyle( Color( COL_BLACK ) ) );
            }
            break;
            case NULL_BRUSH :
            {
                pGDIObj->Set( GDI_BRUSH, new WinMtfFillStyle( Color( COL_TRANSPARENT ), TRUE ) );
            }
            break;
            case WHITE_PEN :
            {
                pGDIObj->Set( GDI_PEN, new WinMtfLineStyle( Color( COL_WHITE ) ) );
            }
            break;
            case BLACK_PEN :
            {
                pGDIObj->Set( GDI_PEN, new WinMtfLineStyle( Color( COL_BLACK ) ) );
            }
            break;
            case NULL_PEN :
            {
                pGDIObj->Set( GDI_PEN, new WinMtfLineStyle( Color( COL_TRANSPARENT ), TRUE ) );
            }
            break;
            default:
            break;
        }
    }
    if ( pGDIObj->pStyle )
    {
        switch( pGDIObj->eType )
        {
            case GDI_PEN :
                maLineStyle = (WinMtfLineStyle*)pGDIObj->pStyle;
            break;
            case GDI_BRUSH :
                maFillStyle = (WinMtfFillStyle*)pGDIObj->pStyle;
            break;
            case GDI_FONT :
            {
                maFont = ((WinMtfFontStyle*)pGDIObj->pStyle)->aFont;
                if ( ( mnActTextAlign & TA_BASELINE) == TA_BASELINE )
                    maFont.SetAlign( ALIGN_BASELINE );
                else if( ( mnActTextAlign & TA_BOTTOM) == TA_BOTTOM )
                    maFont.SetAlign( ALIGN_BOTTOM );
                else
                    maFont.SetAlign( ALIGN_TOP );

                if( mnBkMode == TRANSPARENT )
                    maFont.SetTransparent( TRUE );
                else
                {
                    maFont.SetFillColor( maBkColor );
                    maFont.SetTransparent( FALSE );
                }
                maFont.SetColor( maTextColor );
                mbFontChanged = TRUE;
            }
            break;
        }
    }
    if ( nIndex & ENHMETA_STOCK_OBJECT )
        delete pGDIObj;
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::Push( BOOL bWinExtSet )
{
    SaveStruct* pSave = new SaveStruct;

    pSave->aActPos = maActPos;
    pSave->nActTextAlign = mnActTextAlign;
    pSave->nBkMode = mnBkMode;
    pSave->aBkColor = maBkColor;
    pSave->bWinExtSet = bWinExtSet;
    pSave->aLineStyle = maLineStyle;
    pSave->aFillStyle = maFillStyle;
    pSave->aTextColor = maTextColor;
    pSave->aFont = maFont;
    pSave->bFontChanged = mbFontChanged;

    if ( bWinExtSet )
    {
        pSave->nWinOrgX = mnWinOrgX;
        pSave->nWinOrgY = mnWinOrgY;
        pSave->nWinExtX = mnWinExtX;
        pSave->nWinExtY = mnWinExtY;
        pSave->nDevOrgX = mnDevOrgX;
        pSave->nDevOrgY = mnDevOrgY;
        pSave->nDevWidth = mnDevWidth;
        pSave->nDevHeight = mnDevHeight;
    }
    maSaveStack.Push( pSave );
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::Pop()
{
    // Die aktuellen Daten vom Stack holen
    if( maSaveStack.Count() )
    {
        // Die aktuelle Daten auf dem Stack sichern
        SaveStruct* pSave = maSaveStack.Pop();

        mnBkMode = pSave->nBkMode;
        maBkColor = pSave->aBkColor;
        maActPos = pSave->aActPos;
        mnActTextAlign = pSave->nActTextAlign;
        maLineStyle = pSave->aLineStyle;
        maFillStyle = pSave->aFillStyle;
        maTextColor = pSave->aTextColor;
        maFont = pSave->aFont;
        mbFontChanged = pSave->bFontChanged;

        if ( pSave->bWinExtSet )
        {
            mnWinOrgX = pSave->nWinOrgX;
            mnWinOrgY = pSave->nWinOrgY;
            mnWinExtX = pSave->nWinExtX;
            mnWinExtY = pSave->nWinExtY;
            mnDevOrgX = pSave->nDevOrgX;
            mnDevOrgY = pSave->nDevOrgY;
            mnDevWidth = pSave->nDevWidth;
            mnDevHeight = pSave->nDevHeight;
        }
        delete pSave;
    }
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::SetBkMode( UINT32 nMode )
{
    maFont.SetTransparent( ( mnBkMode = nMode ) == TRANSPARENT );
    mbFontChanged = TRUE;
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::SetBkColor( const Color& rColor )
{
    maBkColor = rColor;
    maFont.SetFillColor( rColor );
    maFont.SetTransparent( mnBkMode == TRANSPARENT );
    mbFontChanged = TRUE;
}


//-----------------------------------------------------------------------------------

void WinMtfOutput::SetTextColor( const Color& rColor )
{
    maTextColor = rColor;
    maFont.SetColor( rColor );
    mbFontChanged = TRUE;
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::SetTextAlign( UINT32 nAlign )
{
    mnActTextAlign = nAlign;

    if ( ( mnActTextAlign & TA_BASELINE ) == TA_BASELINE )
        maFont.SetAlign( ALIGN_BASELINE );
    else if( ( mnActTextAlign & TA_BOTTOM ) == TA_BOTTOM )
        maFont.SetAlign( ALIGN_BOTTOM );
    else
        maFont.SetAlign( ALIGN_TOP );

    mbFontChanged = TRUE;
}

//-----------------------------------------------------------------------------------

UINT32 WinMtfOutput::SetRasterOp( UINT32 nROP2 )
{
    UINT32 nRetROP = mnRop;
    if ( nROP2 != mnRop )
    {
        mnRop = nROP2;
        static WinMtfFillStyle aNopFillStyle;
        static WinMtfLineStyle aNopLineStyle;

        if ( mbNopMode && ( nROP2 != R2_NOP ) )
        {   // beim uebergang von R2_NOP auf anderen Modus
            // gesetzten Pen und Brush aktivieren
            maFillStyle = aNopFillStyle;
            maLineStyle = aNopLineStyle;
            mbNopMode = FALSE;
        }
        switch( nROP2 )
        {
            case R2_NOT:
                meRasterOp = ROP_INVERT;
            break;

            case R2_XORPEN:
                meRasterOp = ROP_XOR;
            break;

            case R2_NOP:
            {
                meRasterOp = ROP_OVERPAINT;
                if( mbNopMode = FALSE  )
                {
                    aNopFillStyle = maFillStyle;
                    aNopLineStyle = maLineStyle;
                    maFillStyle = WinMtfFillStyle( Color( COL_TRANSPARENT ), TRUE );
                    maLineStyle = WinMtfLineStyle( Color( COL_TRANSPARENT ), TRUE );
                    mbNopMode = TRUE;
                }
            }
            break;

            default:
                meRasterOp = ROP_OVERPAINT;
            break;
        }
    }
    return nRetROP;
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::ImplResizeObjectArry( UINT32 nNewEntrys )
{
    GDIObj** pGDIObj = new GDIObj*[ mnEntrys << 1 ];
    UINT32 nIndex;
    for ( nIndex = 0; nIndex < mnEntrys; nIndex++ )
        pGDIObj[ nIndex ] = mpGDIObj[ nIndex ];
    for ( mnEntrys = nNewEntrys; nIndex < mnEntrys; pGDIObj[ nIndex++ ] = NULL );
    delete mpGDIObj, mpGDIObj = pGDIObj;
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::CreateObject( GDIObjectType eType, void* pStyle )
{
    if ( pStyle )
    {
        if ( eType == GDI_FONT )
            ImplMap( ((WinMtfFontStyle*)pStyle)->aFont );
        else if ( eType == GDI_PEN )
        {
            Size aSize( ((WinMtfLineStyle*)pStyle)->aLineInfo.GetWidth(), 0 );
            ((WinMtfLineStyle*)pStyle)->aLineInfo.SetWidth( ImplMap( aSize ).Width() );
            if ( ((WinMtfLineStyle*)pStyle)->aLineInfo.GetStyle() == LINE_DASH )
            {
                aSize.Width() += 1;
                long nDotLen = ImplMap( aSize ).Width();
                ((WinMtfLineStyle*)pStyle)->aLineInfo.SetDistance( nDotLen );
                ((WinMtfLineStyle*)pStyle)->aLineInfo.SetDotLen( nDotLen );
                ((WinMtfLineStyle*)pStyle)->aLineInfo.SetDashLen( nDotLen * 4 );
            }
        }
    }
    UINT32 nIndex;
    for ( nIndex = 0; nIndex < mnEntrys; nIndex++ )
    {
        if ( mpGDIObj[ nIndex ] == NULL )
            break;
    }
    if ( nIndex == mnEntrys )
        ImplResizeObjectArry( mnEntrys << 1 );

    mpGDIObj[ nIndex ] = new GDIObj( eType, pStyle );
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::CreateObject( INT32 nIndex, GDIObjectType eType, void* pStyle )
{
    if ( ( nIndex & ENHMETA_STOCK_OBJECT ) == 0 )
    {
        nIndex &= 0xffff;       // zur Sicherheit: mehr als 65535 nicht zulassen
        if ( pStyle )
        {
            if ( eType == GDI_FONT )
                ImplMap( ((WinMtfFontStyle*)pStyle)->aFont );
            else if ( eType == GDI_PEN )
            {
                Size aSize( ((WinMtfLineStyle*)pStyle)->aLineInfo.GetWidth(), 0 );
                ((WinMtfLineStyle*)pStyle)->aLineInfo.SetWidth( ImplMap( aSize ).Width() );
                if ( ((WinMtfLineStyle*)pStyle)->aLineInfo.GetStyle() == LINE_DASH )
                {
                    aSize.Width() += 1;
                    long nDotLen = ImplMap( aSize ).Width();
                    ((WinMtfLineStyle*)pStyle)->aLineInfo.SetDistance( nDotLen );
                    ((WinMtfLineStyle*)pStyle)->aLineInfo.SetDotLen( nDotLen );
                    ((WinMtfLineStyle*)pStyle)->aLineInfo.SetDashLen( nDotLen * 4 );
                }
            }
        }
        if ( (UINT32)nIndex >= mnEntrys )
            ImplResizeObjectArry( nIndex + 16 );

        if ( mpGDIObj[ nIndex ] != NULL )
            delete mpGDIObj[ nIndex ];

        mpGDIObj[ nIndex ] = new GDIObj( eType, pStyle );
    }
    else
        delete pStyle;
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::DeleteObject( INT32 nIndex )
{
    if ( ( nIndex & ENHMETA_STOCK_OBJECT ) == 0 )
    {
        nIndex &= 0xffff;       // zur Sicherheit: mehr als 65535 nicht zulassen
        delete mpGDIObj[ nIndex ], mpGDIObj[ nIndex ] = NULL;
    }
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::DrawText( Point& rPosition, String& rText, INT32* pDXArry )
{
    rPosition = ImplMap( rPosition );

    if ( pDXArry )
    {
        INT32 i, nSum, nLen = rText.Len();

        for( i = 0, nSum = 0; i < nLen; i++ )
        {
            INT32 nTemp = ImplMap( Size( pDXArry[ i ], 0 ) ).Width();
            nSum += nTemp;
            pDXArry[ i ] = nSum;
        }
    }
}

//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------

WinMtfMetaOutput::WinMtfMetaOutput( GDIMetaFile& rGDIMetaFile ) : WinMtfOutput()
{
    maLatestLineStyle.aLineColor = Color( 0x12, 0x34, 0x56 );
    maLatestFillStyle.aFillColor = Color( 0x12, 0x34, 0x56 );
    mpGDIMetaFile = &rGDIMetaFile;
    mnPushPopCount = 0;

    mnRop = R2_BLACK + 1;
    SetRasterOp( R2_BLACK );
};

//-----------------------------------------------------------------------------------

WinMtfMetaOutput::~WinMtfMetaOutput()
{
    while( mnPushPopCount > 0 )
    {
        mpGDIMetaFile->AddAction( new MetaPopAction() );
        mnPushPopCount--;
    }
    mpGDIMetaFile->SetPrefMapMode( MAP_100TH_MM );
    mpGDIMetaFile->SetPrefSize( Size( mnDevWidth, mnDevHeight ) );
};

//-----------------------------------------------------------------------------------

void WinMtfMetaOutput::UpdateLineStyle()
{
    if (!( maLatestLineStyle == maLineStyle ) )
    {
        maLatestLineStyle = maLineStyle;
        mpGDIMetaFile->AddAction( new MetaLineColorAction( maLineStyle.aLineColor, !maLineStyle.bTransparent ) );
    }
}

//-----------------------------------------------------------------------------------

void WinMtfMetaOutput::UpdateFillStyle()
{
    if (!( maLatestFillStyle == maFillStyle ) )
    {
        maLatestFillStyle = maFillStyle;
        mpGDIMetaFile->AddAction( new MetaFillColorAction( maFillStyle.aFillColor, !maFillStyle.bTransparent ) );
    }
}

//-----------------------------------------------------------------------------------

UINT32 WinMtfMetaOutput::SetRasterOp( UINT32 nRasterOp )
{
    UINT32 nRetROP = WinMtfOutput::SetRasterOp( nRasterOp );
    if ( nRetROP != nRasterOp )
        mpGDIMetaFile->AddAction( new MetaRasterOpAction( meRasterOp ) );
    return nRetROP;
};


//-----------------------------------------------------------------------------------

void WinMtfMetaOutput::DrawPixel( const Point& rSource, const Color& rColor )
{
    mpGDIMetaFile->AddAction( new MetaPixelAction( ImplMap( rSource), rColor ) );
}

//-----------------------------------------------------------------------------------

void WinMtfMetaOutput::LineTo( const Point& rPoint )
{
    Point aDest( ImplMap( rPoint ) );
    UpdateLineStyle();
    mpGDIMetaFile->AddAction( new MetaLineAction( maActPos, aDest, maLineStyle.aLineInfo ) );
    maActPos = aDest;
}

//-----------------------------------------------------------------------------------

void WinMtfMetaOutput::DrawLine( const Point& rSource, const Point& rDest )
{
    UpdateLineStyle();
    mpGDIMetaFile->AddAction( new MetaLineAction( ImplMap( rSource), ImplMap( rDest ), maLineStyle.aLineInfo ) );
}

//-----------------------------------------------------------------------------------

void WinMtfMetaOutput::DrawRect( const Rectangle& rRect, BOOL bEdge )
{
    UpdateFillStyle();
    if ( bEdge )
    {
        if ( maLineStyle.aLineInfo.GetWidth() || ( maLineStyle.aLineInfo.GetStyle() == LINE_DASH ) )
        {
            mpGDIMetaFile->AddAction( new MetaPushAction( PUSH_LINECOLOR ) );
            mpGDIMetaFile->AddAction( new MetaLineColorAction( maLineStyle.aLineColor, FALSE ) );
            mpGDIMetaFile->AddAction( new MetaRectAction( ImplMap( rRect ) ) );
            mpGDIMetaFile->AddAction( new MetaLineColorAction( maLineStyle.aLineColor, !maLineStyle.bTransparent ) );
            mpGDIMetaFile->AddAction( new MetaPolyLineAction( Polygon( ImplMap( rRect ) ),maLineStyle.aLineInfo ) );
            mpGDIMetaFile->AddAction( new MetaPopAction() );
        }
        else
        {
            UpdateLineStyle();
            mpGDIMetaFile->AddAction( new MetaRectAction( ImplMap( rRect ) ) );
        }
    }
    else
    {
        mpGDIMetaFile->AddAction( new MetaPushAction( PUSH_LINECOLOR ) );
        mpGDIMetaFile->AddAction( new MetaLineColorAction( Color(), FALSE ) );
        mpGDIMetaFile->AddAction( new MetaRectAction( ImplMap( rRect ) ) );
        mpGDIMetaFile->AddAction( new MetaPopAction() );
    }
}

//-----------------------------------------------------------------------------------

void WinMtfMetaOutput::DrawRoundRect( const Rectangle& rRect, const Size& rSize )
{
    UpdateLineStyle();
    UpdateFillStyle();
    mpGDIMetaFile->AddAction( new MetaRoundRectAction( ImplMap( rRect ), labs( ImplMap( rSize ).Width() ), labs( ImplMap( rSize ).Height() ) ) );
}

//-----------------------------------------------------------------------------------

void WinMtfMetaOutput::DrawEllipse( const Rectangle& rRect )
{
    UpdateFillStyle();

    if ( maLineStyle.aLineInfo.GetWidth() || ( maLineStyle.aLineInfo.GetStyle() == LINE_DASH ) )
    {
        Point aCenter( ImplMap( rRect.Center() ) );
        Size  aRad( ImplMap( Size( rRect.GetWidth() / 2, rRect.GetHeight() / 2 ) ) );

        mpGDIMetaFile->AddAction( new MetaPushAction( PUSH_LINECOLOR ) );
        mpGDIMetaFile->AddAction( new MetaLineColorAction( maLineStyle.aLineColor, FALSE ) );
        mpGDIMetaFile->AddAction( new MetaEllipseAction( ImplMap( rRect ) ) );
        mpGDIMetaFile->AddAction( new MetaLineColorAction( maLineStyle.aLineColor, !maLineStyle.bTransparent ) );
        mpGDIMetaFile->AddAction( new MetaPolyLineAction( Polygon( aCenter, aRad.Width(), aRad.Height() ), maLineStyle.aLineInfo ) );
        mpGDIMetaFile->AddAction( new MetaPopAction() );
    }
    else
    {
        UpdateLineStyle();
        mpGDIMetaFile->AddAction( new MetaEllipseAction( ImplMap( rRect ) ) );
    }
}

//-----------------------------------------------------------------------------------

void WinMtfMetaOutput::DrawArc( const Rectangle& rRect, const Point& rStart, const Point& rEnd, BOOL bTo )
{
    UpdateLineStyle();
    UpdateFillStyle();

    Rectangle   aRect( ImplMap( rRect ) );
    Point       aStart( ImplMap( rStart ) );
    Point       aEnd( ImplMap( rEnd ) );

    if ( maLineStyle.aLineInfo.GetWidth() || ( maLineStyle.aLineInfo.GetStyle() == LINE_DASH ) )
        mpGDIMetaFile->AddAction( new MetaPolyLineAction( Polygon( aRect, aStart, aEnd, POLY_ARC ), maLineStyle.aLineInfo ) );
    else
        mpGDIMetaFile->AddAction( new MetaArcAction( aRect, aStart, aEnd ) );

    if ( bTo )
        maActPos = aEnd;
}

//-----------------------------------------------------------------------------------

void WinMtfMetaOutput::DrawPie( const Rectangle& rRect, const Point& rStart, const Point& rEnd )
{
    UpdateFillStyle();

    Rectangle   aRect( ImplMap( rRect ) );
    Point       aStart( ImplMap( rStart ) );
    Point       aEnd( ImplMap( rEnd ) );

    if ( maLineStyle.aLineInfo.GetWidth() || ( maLineStyle.aLineInfo.GetStyle() == LINE_DASH ) )
    {
        mpGDIMetaFile->AddAction( new MetaPushAction( PUSH_LINECOLOR ) );
        mpGDIMetaFile->AddAction( new MetaLineColorAction( maLineStyle.aLineColor, FALSE ) );
        mpGDIMetaFile->AddAction( new MetaPieAction( aRect, aStart, aEnd ) );
        mpGDIMetaFile->AddAction( new MetaLineColorAction( maLineStyle.aLineColor, !maLineStyle.bTransparent ) );
        mpGDIMetaFile->AddAction( new MetaPolyLineAction( Polygon( aRect, aStart, aEnd, POLY_PIE ), maLineStyle.aLineInfo ) );
        mpGDIMetaFile->AddAction( new MetaPopAction() );
    }
    else
    {
        UpdateLineStyle();
        mpGDIMetaFile->AddAction( new MetaPieAction( aRect, aStart, aEnd ) );
    }
}

//-----------------------------------------------------------------------------------

void WinMtfMetaOutput::DrawChord( const Rectangle& rRect, const Point& rStart, const Point& rEnd )
{
    UpdateFillStyle();

    Rectangle   aRect( ImplMap( rRect ) );
    Point       aStart( ImplMap( rStart ) );
    Point       aEnd( ImplMap( rEnd ) );

    if ( maLineStyle.aLineInfo.GetWidth() || ( maLineStyle.aLineInfo.GetStyle() == LINE_DASH ) )
    {
        mpGDIMetaFile->AddAction( new MetaPushAction( PUSH_LINECOLOR ) );
        mpGDIMetaFile->AddAction( new MetaLineColorAction( maLineStyle.aLineColor, FALSE ) );
        mpGDIMetaFile->AddAction( new MetaChordAction( aRect, aStart, aEnd ) );
        mpGDIMetaFile->AddAction( new MetaLineColorAction( maLineStyle.aLineColor, !maLineStyle.bTransparent ) );
        mpGDIMetaFile->AddAction( new MetaPolyLineAction( Polygon( aRect, aStart, aEnd, POLY_CHORD ), maLineStyle.aLineInfo ) );
        mpGDIMetaFile->AddAction( new MetaPopAction() );
    }
    else
    {
        UpdateLineStyle();
        mpGDIMetaFile->AddAction( new MetaChordAction( aRect, aStart, aEnd ) );
    }
}

//-----------------------------------------------------------------------------------

void WinMtfMetaOutput::DrawPolygon( Polygon& rPolygon )
{
    UpdateFillStyle();

    if ( maLineStyle.aLineInfo.GetWidth() || ( maLineStyle.aLineInfo.GetStyle() == LINE_DASH ) )
    {
        USHORT nCount = rPolygon.GetSize();
        if ( nCount )
        {
            if ( rPolygon[ nCount - 1 ] != rPolygon[ 0 ] )
            {
                Point aPoint( rPolygon[ 0 ] );
                rPolygon.Insert( nCount, aPoint );
            }
        }
        mpGDIMetaFile->AddAction( new MetaPushAction( PUSH_LINECOLOR ) );
        mpGDIMetaFile->AddAction( new MetaLineColorAction( maLineStyle.aLineColor, FALSE ) );
        mpGDIMetaFile->AddAction( new MetaPolygonAction( ImplMap( rPolygon ) ) );
        mpGDIMetaFile->AddAction( new MetaLineColorAction( maLineStyle.aLineColor, !maLineStyle.bTransparent ) );
        mpGDIMetaFile->AddAction( new MetaPolyLineAction( rPolygon, maLineStyle.aLineInfo ) );
        mpGDIMetaFile->AddAction( new MetaPopAction() );
    }
    else
    {
        UpdateLineStyle();
        mpGDIMetaFile->AddAction( new MetaPolygonAction( ImplMap( rPolygon ) ) );
    }
}

//-----------------------------------------------------------------------------------

void WinMtfMetaOutput::DrawPolyPolygon( PolyPolygon& rPolyPolygon )
{
    UpdateLineStyle();
    UpdateFillStyle();
    mpGDIMetaFile->AddAction( new MetaPolyPolygonAction( ImplMap( rPolyPolygon ) ) );
}

//-----------------------------------------------------------------------------------

void WinMtfMetaOutput::DrawPolyLine( Polygon& rPolygon, BOOL bTo )
{
    UpdateLineStyle();
    ImplMap( rPolygon );
    if ( bTo )
    {
        rPolygon[ 0 ] = maActPos;
        maActPos = rPolygon[ rPolygon.GetSize() - 1 ];
    }
    mpGDIMetaFile->AddAction( new MetaPolyLineAction( rPolygon, maLineStyle.aLineInfo ) );
}

//-----------------------------------------------------------------------------------

void WinMtfMetaOutput::DrawPolyBezier( Polygon& rPolygon, BOOL bTo )
{
    UpdateLineStyle();
    UINT16 nPoints = rPolygon.GetSize();
    if ( ( nPoints >= 4 ) && ( ( ( nPoints - 4 ) % 3 ) == 0 ) )
    {
        ImplMap( rPolygon );
        if ( bTo )
        {
            rPolygon[ 0 ] = maActPos;
            maActPos = rPolygon[ nPoints - 1 ];
        }
        // create bezier polygon
        const USHORT    nSegPoints = 25;
        const USHORT    nSegments = ( ( nPoints - 4 ) / 3 ) + 1;
        Polygon         aBezPoly( nSegments * nSegPoints );

        USHORT nSeg, nBezPos, nStartPos;
        for( nSeg = 0, nBezPos = 0, nStartPos = 0; nSeg < nSegments; nSeg++, nStartPos += 3 )
        {
            const Polygon aSegPoly( rPolygon[ nStartPos ], rPolygon[ nStartPos + 1 ],
                                    rPolygon[ nStartPos + 3 ], rPolygon[ nStartPos + 2 ],
                                    nSegPoints );
            for( USHORT nSegPos = 0; nSegPos < nSegPoints; )
                aBezPoly[ nBezPos++ ] = aSegPoly[ nSegPos++ ];
        }

        if( nBezPos != aBezPoly.GetSize() )
            aBezPoly.SetSize( nBezPos );
        mpGDIMetaFile->AddAction( new MetaPolyLineAction( aBezPoly, maLineStyle.aLineInfo ) );
    }
}

//-----------------------------------------------------------------------------------

void WinMtfMetaOutput::DrawText( Point& rPosition, String& rText, INT32* pDXArry )
{
    WinMtfOutput::DrawText( rPosition, rText, pDXArry );

    if( mbFontChanged )
    {
        mpGDIMetaFile->AddAction( new MetaFontAction( maFont ) );
        mpGDIMetaFile->AddAction( new MetaTextAlignAction( maFont.GetAlign() ) );
        mpGDIMetaFile->AddAction( new MetaTextColorAction( maFont.GetColor() ) );
        mpGDIMetaFile->AddAction( new MetaTextFillColorAction( maFont.GetFillColor(), !maFont.IsTransparent() ) );
        mbFontChanged = FALSE;
    }

    if( mnActTextAlign & ( TA_UPDATECP | TA_RIGHT_CENTER ) )
    {
        VirtualDevice   aVDev;
        sal_Int32       nTextWidth;

        aVDev.SetMapMode( MapMode( MAP_100TH_MM ) );
        aVDev.SetFont( maFont );

        if( pDXArry )
        {
            UINT32 nLen = rText.Len();
            nTextWidth = aVDev.GetTextWidth( rText.GetChar( nLen - 1 ) );
            if( nLen > 1 )
                nTextWidth += pDXArry[ nLen - 2 ];
        }
        else
            nTextWidth = aVDev.GetTextWidth( rText );

        if( mnActTextAlign & TA_UPDATECP )
            rPosition = maActPos;

        if( mnActTextAlign & TA_RIGHT_CENTER )
            rPosition.X() -= ( ( mnActTextAlign & TA_RIGHT_CENTER ) == TA_RIGHT ) ? nTextWidth : ( nTextWidth >> 1 );

        if( mnActTextAlign & TA_UPDATECP )
            maActPos.X() = rPosition.X() + nTextWidth;
    }

    if( pDXArry )
        mpGDIMetaFile->AddAction( new MetaTextArrayAction( rPosition, rText, pDXArry, 0, STRING_LEN ) );
    else
        mpGDIMetaFile->AddAction( new MetaTextAction( rPosition, rText, 0, STRING_LEN ) );
}

//-----------------------------------------------------------------------------------

void WinMtfMetaOutput::ResolveBitmapActions( List& rSaveList )
{
    for( ULONG i = 0, nCount = rSaveList.Count(); i < nCount; i++ )
    {
        BSaveStruct* pSave1 = (BSaveStruct*) rSaveList.GetObject( i );
        BOOL         bDrawn = FALSE;

        UpdateFillStyle();

        if( i < ( nCount - 1 ) )
        {
            BSaveStruct* pSave2 = (BSaveStruct*) rSaveList.GetObject( i + 1 );

            if( ( pSave1->aOutRect == pSave2->aOutRect ) &&
                ( pSave1->nWinRop == SRCPAINT) && ( pSave2->nWinRop == SRCAND ) )
            {
                Bitmap aMask( pSave1->aBmp ); aMask.Invert();
                BitmapEx aBmpEx( pSave2->aBmp, aMask );
                mpGDIMetaFile->AddAction( new MetaBmpExScaleAction( ImplMap( pSave1->aOutRect.TopLeft() ), ImplMap( pSave1->aOutRect.GetSize() ), aBmpEx ) );
                bDrawn = TRUE;
                i++;
                delete pSave2;
            }
        }
        if( !bDrawn )
        {
            UINT32  nNewROP, nOldROP;
            switch( pSave1->nWinRop )
            {
                case DSTINVERT: nNewROP = R2_NOT; break;
                case SRCINVERT: nNewROP = R2_XORPEN; break;
                default: nNewROP = R2_BLACK; break;
            }
            nOldROP = SetRasterOp( nNewROP );
            mpGDIMetaFile->AddAction( new MetaBmpScaleAction( ImplMap( pSave1->aOutRect.TopLeft() ), ImplMap( pSave1->aOutRect.GetSize() ), pSave1->aBmp ) );
            SetRasterOp( nOldROP );
        }
        delete pSave1;
    }
    rSaveList.Clear();
}

//-----------------------------------------------------------------------------------

void WinMtfMetaOutput::IntersectClipRect( const Rectangle& rRect )
{
    mpGDIMetaFile->AddAction( new MetaISectRectClipRegionAction( ImplMap( rRect ) ) );
}

//-----------------------------------------------------------------------------------

void WinMtfMetaOutput::MoveClipRegion( const Size& rSize )
{
    Size aSize( ImplMap( rSize ) );
    mpGDIMetaFile->AddAction( new MetaMoveClipRegionAction( aSize.Width(), aSize.Height() ) );
}

//-----------------------------------------------------------------------------------

void WinMtfMetaOutput::SetDevOrg( const Point& rPoint )
{
    mnDevOrgX = rPoint.X();
    mnDevOrgY = rPoint.Y();
}

//-----------------------------------------------------------------------------------

void WinMtfMetaOutput::SetDevOrgOffset( INT32 nXAdd, INT32 nYAdd )
{
    mnDevOrgX += nXAdd;
    mnDevOrgY += nYAdd;
}

//-----------------------------------------------------------------------------------

void WinMtfMetaOutput::SetDevExt( const Size& rSize )
{
    mnDevWidth = rSize.Width();
    mnDevHeight = rSize.Height();
}

//-----------------------------------------------------------------------------------

void WinMtfMetaOutput::ScaleDevExt( double fX, double fY )
{
    mnDevWidth = FRound( mnDevWidth * fX );
    mnDevHeight = FRound( mnDevHeight * fY );
}

//-----------------------------------------------------------------------------------

void WinMtfMetaOutput::SetWinOrg( const Point& rPoint )
{
    mnWinOrgX = rPoint.X();
    mnWinOrgY = rPoint.Y();
}

//-----------------------------------------------------------------------------------

void WinMtfMetaOutput::SetWinOrgOffset( INT32 nXAdd, INT32 nYAdd )
{
    mnWinOrgX += nXAdd;
    mnWinOrgY += nYAdd;
}

//-----------------------------------------------------------------------------------

void WinMtfMetaOutput::SetWinExt( const Size& rSize )
{
    mnWinExtX = rSize.Width();
    mnWinExtY = rSize.Height();
}

//-----------------------------------------------------------------------------------

void WinMtfMetaOutput::ScaleWinExt( double fX, double fY )
{
    mnWinExtX = FRound( mnWinExtX * fX );
    mnWinExtY = FRound( mnWinExtY * fY );
}

//-----------------------------------------------------------------------------------

void WinMtfMetaOutput::SetWorldTransform( const XForm& rXForm )
{
    maXForm.eM11 = rXForm.eM11;
    maXForm.eM12 = rXForm.eM12;
    maXForm.eM21 = rXForm.eM21;
    maXForm.eM22 = rXForm.eM22;
    maXForm.eDx = rXForm.eDx;
    maXForm.eDy = rXForm.eDy;
}

//-----------------------------------------------------------------------------------

void WinMtfMetaOutput::ModifyWorldTransform( const XForm& rXForm, UINT32 nMode )
{
    switch( nMode )
    {
        case MWT_IDENTITY :
        {
            maXForm.eM11 = maXForm.eM12 = maXForm.eM21 = maXForm.eM22 = 1.0f;
            maXForm.eDx = maXForm.eDx = 0.0f;
        }
        break;

        case MWT_LEFTMULTIPLY :
        case MWT_RIGHTMULTIPLY :
        break;
    }
 }

//-----------------------------------------------------------------------------------

void WinMtfMetaOutput::Push( BOOL bExtSet )
{
    WinMtfOutput::Push( bExtSet );
    // bei SaveDC muessen wir die verzoegerte Selektion
    // von Objekten umgehen, damit beim RestoreDC wieder
    // die richtigen Objekte selektiert werden
    UpdateLineStyle();
    UpdateFillStyle();
    if( mbFontChanged )
    {
        mpGDIMetaFile->AddAction( new MetaFontAction( maFont ) );
        mpGDIMetaFile->AddAction( new MetaTextAlignAction( maFont.GetAlign() ) );
        mpGDIMetaFile->AddAction( new MetaTextColorAction( maFont.GetColor() ) );
        mpGDIMetaFile->AddAction( new MetaTextFillColorAction( maFont.GetFillColor(), !maFont.IsTransparent() ) );
        mbFontChanged = FALSE;
    }
    mpGDIMetaFile->AddAction( new MetaPushAction( PUSH_ALL ) );
    mnPushPopCount++;
}

//-----------------------------------------------------------------------------------

void WinMtfMetaOutput::Pop()
{
    WinMtfOutput::Pop();
    maLatestLineStyle = maLineStyle;
    maLatestFillStyle = maFillStyle;
    if( mnPushPopCount > 0 )
    {
        mpGDIMetaFile->AddAction( new MetaPopAction() );
        mnPushPopCount--;
    }
}

