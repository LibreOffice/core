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
#include "precompiled_svtools.hxx"

#define _CTRLBOX_CXX
#include <tools/debug.hxx>
#include <vcl/svapp.hxx>
#include <vcl/field.hxx>
#include <sal/macros.h>
#include <comphelper/processfactory.hxx>
#include <unotools/charclass.hxx>

#include <svtools/svtdata.hxx>
#include <svtools/svtools.hrc>
#include <ctrlbox.hxx>
#include <ctrltool.hxx>
#include <borderhelper.hxx>

#include <vcl/i18nhelp.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

#define IMGINNERTEXTSPACE 2
#define IMGOUTERTEXTSPACE 5
#define EXTRAFONTSIZE 5
#define MAXPREVIEWWIDTH 150

static sal_Unicode aImplSymbolFontText[] = {0xF021,0xF032,0xF043,0xF054,0xF065,0xF076,0xF0B7,0xF0C8,0};
static sal_Unicode aImplStarSymbolText[] = {0x2706,0x2704,0x270D,0xE033,0x2211,0x2288,0};

using namespace ::com::sun::star;

// ========================================================================
// ColorListBox
// ========================================================================

// --------------------
// - ImplColorListData -
// --------------------

struct ImplColorListData
{
    Color       aColor;
    BOOL        bColor;

                ImplColorListData() : aColor( COL_BLACK ) { bColor = FALSE; }
                ImplColorListData( const Color& rColor ) : aColor( rColor ) { bColor = TRUE; }
};

// -----------------------------------------------------------------------

void ColorListBox::ImplInit()
{
    pColorList = new ImpColorList();
    aImageSize.Width()  = GetTextWidth( XubString( RTL_CONSTASCII_USTRINGPARAM( "xxx" ) ) );
    aImageSize.Height() = GetTextHeight();
    aImageSize.Height() -= 2;

    EnableUserDraw( TRUE );
    SetUserItemSize( aImageSize );
}

// -----------------------------------------------------------------------

void ColorListBox::ImplDestroyColorEntries()
{
    for ( size_t n = pColorList->size(); n; )
        delete (*pColorList)[ --n ];
    pColorList->clear();
}

// -----------------------------------------------------------------------

ColorListBox::ColorListBox( Window* pParent, WinBits nWinStyle ) :
    ListBox( pParent, nWinStyle )
{
    ImplInit();
}

// -----------------------------------------------------------------------

ColorListBox::ColorListBox( Window* pParent, const ResId& rResId ) :
    ListBox( pParent, rResId )
{
    ImplInit();
}

// -----------------------------------------------------------------------

ColorListBox::~ColorListBox()
{
    ImplDestroyColorEntries();
    delete pColorList;
}

// -----------------------------------------------------------------------

USHORT ColorListBox::InsertEntry( const XubString& rStr, USHORT nPos )
{
    nPos = ListBox::InsertEntry( rStr, nPos );
    if ( nPos != LISTBOX_ERROR )
    {
        ImplColorListData* pData = new ImplColorListData;
        if ( nPos < pColorList->size() )
        {
            ImpColorList::iterator it = pColorList->begin();
            ::std::advance( it, nPos );
            pColorList->insert( it, pData );
        }
        else
        {
            pColorList->push_back( pData );
            nPos = pColorList->size() - 1;
        }
    }
    return nPos;
}

// -----------------------------------------------------------------------

USHORT ColorListBox::InsertEntry( const Color& rColor, const XubString& rStr,
                                USHORT nPos )
{
    nPos = ListBox::InsertEntry( rStr, nPos );
    if ( nPos != LISTBOX_ERROR )
    {
        ImplColorListData* pData = new ImplColorListData( rColor );
        if ( nPos < pColorList->size() )
        {
            ImpColorList::iterator it = pColorList->begin();
            ::std::advance( it, nPos );
            pColorList->insert( it, pData );
        }
        else
        {
            pColorList->push_back( pData );
            nPos = pColorList->size() - 1;
        }
    }
    return nPos;
}

// -----------------------------------------------------------------------

void ColorListBox::InsertAutomaticEntry()
{
    // insert the "Automatic"-entry always on the first position
    InsertEntry( Color( COL_AUTO ), SvtResId( STR_SVT_AUTOMATIC_COLOR ), 0 );
}

// -----------------------------------------------------------------------

void ColorListBox::RemoveEntry( USHORT nPos )
{
    ListBox::RemoveEntry( nPos );
    if ( nPos < pColorList->size() )
    {
            ImpColorList::iterator it = pColorList->begin();
            ::std::advance( it, nPos );
            delete *it;
            pColorList->erase( it );
    }
}

// -----------------------------------------------------------------------

void ColorListBox::Clear()
{
    ImplDestroyColorEntries();
    ListBox::Clear();
}

// -----------------------------------------------------------------------

void ColorListBox::CopyEntries( const ColorListBox& rBox )
{
    // Liste leeren
    ImplDestroyColorEntries();

    // Daten kopieren
    size_t nCount = rBox.pColorList->size();
    for ( size_t n = 0; n < nCount; n++ )
    {
        ImplColorListData* pData = (*rBox.pColorList)[ n ];
        USHORT nPos = InsertEntry( rBox.GetEntry( n ), LISTBOX_APPEND );
        if ( nPos != LISTBOX_ERROR )
        {
            if ( nPos < pColorList->size() )
            {
                ImpColorList::iterator it = pColorList->begin();
                ::std::advance( it, nPos );
                pColorList->insert( it, new ImplColorListData( *pData ) );
            }
            else
            {
                pColorList->push_back( new ImplColorListData( *pData ) );
            }
        }
    }
}

// -----------------------------------------------------------------------

USHORT ColorListBox::GetEntryPos( const Color& rColor ) const
{
    for( USHORT n = (USHORT) pColorList->size(); n; )
    {
        ImplColorListData* pData = (*pColorList)[ --n ];
        if ( pData->bColor && ( pData->aColor == rColor ) )
            return n;
    }
    return LISTBOX_ENTRY_NOTFOUND;
}

// -----------------------------------------------------------------------

Color ColorListBox::GetEntryColor( USHORT nPos ) const
{
    Color aColor;
    ImplColorListData* pData = ( nPos < pColorList->size() ) ? (*pColorList)[ nPos ] : NULL;
    if ( pData && pData->bColor )
        aColor = pData->aColor;
    return aColor;
}

// -----------------------------------------------------------------------

void ColorListBox::UserDraw( const UserDrawEvent& rUDEvt )
{
    size_t nPos = rUDEvt.GetItemId();
    ImplColorListData* pData = ( nPos < pColorList->size() ) ? (*pColorList)[ nPos ] : NULL;
    if ( pData )
    {
        if ( pData->bColor )
        {
            Point aPos( rUDEvt.GetRect().TopLeft() );
            aPos.X() += 2;
            aPos.Y() += ( rUDEvt.GetRect().GetHeight() - aImageSize.Height() ) / 2;
            rUDEvt.GetDevice()->Push();
            rUDEvt.GetDevice()->SetFillColor( pData->aColor );
            rUDEvt.GetDevice()->SetLineColor( rUDEvt.GetDevice()->GetTextColor() );
            rUDEvt.GetDevice()->DrawRect( Rectangle( aPos, aImageSize ) );
            rUDEvt.GetDevice()->Pop();
            ListBox::DrawEntry( rUDEvt, FALSE, TRUE, FALSE );
        }
        else
            ListBox::DrawEntry( rUDEvt, FALSE, TRUE, TRUE );
    }
    else
        ListBox::DrawEntry( rUDEvt, TRUE, TRUE, FALSE );
}

// =======================================================================
// LineListBox
// =======================================================================

// -------------------
// - ImpListListData -
// -------------------

struct ImpLineListData
{
    long    nLine1;
    long    nLine2;
    long    nDistance;
    USHORT  nStyle;
};

// -----------------------------------------------------------------------

inline const Color& LineListBox::GetPaintColor( void ) const
{
    return maPaintCol;
}

// -----------------------------------------------------------------------


inline void lclDrawPolygon( OutputDevice& rDev, const basegfx::B2DPolygon& rPolygon, long nWidth, USHORT nDashing )
{
    USHORT nOldAA = rDev.GetAntialiasing();
    rDev.SetAntialiasing( nOldAA & !ANTIALIASING_ENABLE_B2DDRAW );

    Color aOldColor = rDev.GetFillColor( );
    rDev.SetFillColor( rDev.GetLineColor( ) );

    basegfx::B2DPolyPolygon aPolygons = svtools::ApplyLineDashing( rPolygon, nDashing, rDev.GetMapMode().GetMapUnit() );
    for ( sal_uInt32 i = 0; i < aPolygons.count( ); i++ )
    {
        basegfx::B2DPolygon aDash = aPolygons.getB2DPolygon( i );
        basegfx::B2DPoint aStart = aDash.getB2DPoint( 0 );
        basegfx::B2DPoint aEnd = aDash.getB2DPoint( aDash.count() - 1 );

        basegfx::B2DVector aVector( aEnd - aStart );
        aVector.normalize( );
        const basegfx::B2DVector aPerpendicular(basegfx::getPerpendicular(aVector));

        const basegfx::B2DVector aWidthOffset( nWidth / 2 * aPerpendicular);
        basegfx::B2DPolygon aDashPolygon;
        aDashPolygon.append( aStart + aWidthOffset );
        aDashPolygon.append( aEnd + aWidthOffset );
        aDashPolygon.append( aEnd - aWidthOffset );
        aDashPolygon.append( aStart - aWidthOffset );
        aDashPolygon.setClosed( true );

        rDev.DrawPolygon( aDashPolygon );
    }

    rDev.SetFillColor( aOldColor );
    rDev.SetAntialiasing( nOldAA );
}

namespace svtools
{
    basegfx::B2DPolyPolygon ApplyLineDashing( const basegfx::B2DPolygon& rPolygon, USHORT nDashing, MapUnit eUnit )
    {
        ::std::vector < double >aPattern;
        switch ( nDashing )
        {
            case STYLE_DOTTED:
                if ( eUnit == MAP_TWIP )
                {
                    aPattern.push_back( 30.0 );
                    aPattern.push_back( 110.0 );
                }
                else if ( eUnit == MAP_100TH_MM )
                {
                    aPattern.push_back( 50 );
                    aPattern.push_back( 200 );
                }
                else if ( eUnit == MAP_PIXEL )
                {
                    aPattern.push_back( 1.0 );
                    aPattern.push_back( 3.0 );
                }
                break;
            case STYLE_DASHED:
                if ( eUnit == MAP_TWIP )
                {
                    aPattern.push_back( 110 );
                    aPattern.push_back( 110 );
                }
                else if ( eUnit == MAP_100TH_MM )
                {
                    aPattern.push_back( 200 );
                    aPattern.push_back( 200 );
                }
                else if ( eUnit == MAP_PIXEL )
                {
                    aPattern.push_back( 10 );
                    aPattern.push_back( 20 );
                }
                break;
            default:
                break;
        }

        basegfx::B2DPolyPolygon aPolygons;
        if ( ! aPattern.empty() )
            basegfx::tools::applyLineDashing( rPolygon, aPattern, &aPolygons );
        else
            aPolygons.append( rPolygon );

        return aPolygons;
    }

    void DrawLine( OutputDevice& rDev, const Point& rP1, const Point& rP2,
        sal_uInt32 nWidth, USHORT nDashing )
    {
        basegfx::B2DPolygon aPolygon;
        aPolygon.append( basegfx::B2DPoint( rP1.X(), rP1.Y() ) );
        aPolygon.append( basegfx::B2DPoint( rP2.X(), rP2.Y() ) );
        lclDrawPolygon( rDev, aPolygon, nWidth, nDashing );
    }
}

void LineListBox::ImpGetLine( long nLine1, long nLine2, long nDistance,
                            USHORT nStyle, Bitmap& rBmp, XubString& rStr )
{
    Size aSize = GetOutputSizePixel();
    aSize.Width() -= 20;
    aSize.Width() -= aTxtSize.Width();
    aSize.Height() = aTxtSize.Height();

    // SourceUnit nach Twips
    if ( eSourceUnit == FUNIT_POINT )
    {
        nLine1      *= 20;
        nLine2      *= 20;
        nDistance   *= 20;
    }
    else if ( eSourceUnit == FUNIT_MM )
    {
        nLine1      *= 14440;
        nLine1      /= 254;
        nLine2      *= 14440;
        nLine2      /= 254;
        nDistance   *= 14440;
        nDistance   /= 254;
    }

    // Linien malen
    aSize = aVirDev.PixelToLogic( aSize );
    long nPix = aVirDev.PixelToLogic( Size( 0, 1 ) ).Height();
    sal_uInt32 n1 = nLine1 / 100;
    sal_uInt32 n2 = nLine2 / 100;
    long nDist  = nDistance / 100;
    n1 += nPix-1;
    n1 -= n1%nPix;
    if ( n2 )
    {
        nDist += nPix-1;
        nDist -= nDist%nPix;
        n2    += nPix-1;
        n2    -= n2%nPix;
    }
    long nVirHeight = n1+nDist+n2;
    if ( nVirHeight > aSize.Height() )
        aSize.Height() = nVirHeight;
    // negative Breiten muss und darf man nicht painten
    if ( aSize.Width() > 0 )
    {
        Size aVirSize = aVirDev.LogicToPixel( aSize );
        if ( aVirDev.GetOutputSizePixel() != aVirSize )
            aVirDev.SetOutputSizePixel( aVirSize );
        aVirDev.SetFillColor( GetSettings().GetStyleSettings().GetFieldColor() );
        aVirDev.DrawRect( Rectangle( Point(), aSize ) );

        Color oldColor = aVirDev.GetLineColor( );
        aVirDev.SetLineColor( GetPaintColor( ) );

        double y1 = n1 / 2;
        svtools::DrawLine( aVirDev, Point( 0, y1 ), Point( aSize.Width( ), y1 ), n1, nStyle );

        if ( n2 )
        {
            double y2 =  n1 + nDist + n2 / 2;
            svtools::DrawLine( aVirDev, Point( 0, y2 ), Point( aSize.Width(), y2 ), n2, STYLE_SOLID );
        }
        aVirDev.SetLineColor( oldColor );
        rBmp = aVirDev.GetBitmap( Point(), Size( aSize.Width(), n1+nDist+n2 ) );
    }
    // Twips nach Unit
    if ( eUnit == FUNIT_POINT )
    {
        nLine1      /= 20;
        nLine2      /= 20;
        nDistance   /= 20;
        rStr.AssignAscii( " pt" );
    }
    else if ( eUnit == FUNIT_MM )
    {
        nLine1      *= 254;
        nLine1      /= 14400;
        nLine2      *= 254;
        nLine2      /= 14400;
        nDistance   *= 254;
        nDistance   /= 14400;
        rStr.AssignAscii( " mm" );
    }

    String aNum( GetSettings().GetLocaleI18nHelper().GetNum( nLine1+nLine2+nDistance, 2 ) );
    rStr.Insert( aNum, 0 );
}

// -----------------------------------------------------------------------

void LineListBox::ImplInit()
{
    aTxtSize.Width()  = GetTextWidth( XubString( RTL_CONSTASCII_USTRINGPARAM( "99,99 mm" ) ) );
    aTxtSize.Height() = GetTextHeight();
    pLineList   = new ImpLineList();
    eUnit       = FUNIT_POINT;
    eSourceUnit = FUNIT_POINT;

    aVirDev.SetLineColor();
    aVirDev.SetMapMode( MapMode( MAP_TWIP ) );

    UpdatePaintLineColor();
}

// -----------------------------------------------------------------------

LineListBox::LineListBox( Window* pParent, WinBits nWinStyle ) :
    ListBox( pParent, nWinStyle ),
    aColor( COL_BLACK ),
    maPaintCol( COL_BLACK )
{
    ImplInit();
}

// -----------------------------------------------------------------------

LineListBox::LineListBox( Window* pParent, const ResId& rResId ) :
    ListBox( pParent, rResId ),
    aColor( COL_BLACK ),
    maPaintCol( COL_BLACK )
{
    ImplInit();
}

// -----------------------------------------------------------------------

LineListBox::~LineListBox()
{
    for ( size_t i = 0, n = pLineList->size(); i < n; ++i ) {
        if ( (*pLineList)[ i ] ) {
            delete (*pLineList)[ i ];
        }
    }
    pLineList->clear();
    delete pLineList;
}

// -----------------------------------------------------------------------

USHORT LineListBox::InsertEntry( const XubString& rStr, USHORT nPos )
{
    nPos = ListBox::InsertEntry( rStr, nPos );
    if ( nPos != LISTBOX_ERROR ) {
        if ( nPos < pLineList->size() ) {
            ImpLineList::iterator it = pLineList->begin();
            ::std::advance( it, nPos );
            pLineList->insert( it, NULL );
        } else {
            pLineList->push_back( NULL );
        }
    }
    return nPos;
}

// -----------------------------------------------------------------------

USHORT LineListBox::InsertEntry( long nLine1, long nLine2, long nDistance,
                                USHORT nStyle, USHORT nPos )
{
    XubString   aStr;
    Bitmap      aBmp;
    ImpGetLine( nLine1, nLine2, nDistance, nStyle, aBmp, aStr );
    nPos = ListBox::InsertEntry( aStr, aBmp, nPos );
    if ( nPos != LISTBOX_ERROR )
    {
        ImpLineListData* pData = new ImpLineListData;
        pData->nLine1    = nLine1;
        pData->nLine2    = nLine2;
        pData->nDistance = nDistance;
        pData->nStyle    = nStyle;
        if ( nPos < pLineList->size() ) {
            ImpLineList::iterator it = pLineList->begin();
            ::std::advance( it, nPos );
            pLineList->insert( it, pData );
        } else {
            pLineList->push_back( pData );
        }
    }

    return nPos;
}

// -----------------------------------------------------------------------

void LineListBox::RemoveEntry( USHORT nPos )
{
    ListBox::RemoveEntry( nPos );

    if ( nPos < pLineList->size() ) {
        ImpLineList::iterator it = pLineList->begin();
        ::std::advance( it, nPos );
        if ( *it ) delete *it;
        pLineList->erase( it );
    }
}

// -----------------------------------------------------------------------

void LineListBox::Clear()
{
    for ( size_t i = 0, n = pLineList->size(); i < n; ++i ) {
        if ( (*pLineList)[ i ] ) {
            delete (*pLineList)[ i ];
        }
    }
    pLineList->clear();

    ListBox::Clear();
}

// -----------------------------------------------------------------------

USHORT LineListBox::GetEntryPos( long nLine1, long nLine2,
                                long nDistance, USHORT nStyle ) const
{
    for ( size_t i = 0, n = pLineList->size(); i < n; ++i ) {
        ImpLineListData* pData = (*pLineList)[ n ];
        if ( pData )
        {
            if (  (pData->nLine1    == nLine1)
               && (pData->nLine2    == nLine2)
               && (pData->nDistance == nDistance)
               && (pData->nStyle    == nStyle)
            ) {
                return (USHORT)i;
            }
        }
    }
    return LISTBOX_ENTRY_NOTFOUND;
}

// -----------------------------------------------------------------------

long LineListBox::GetEntryLine1( USHORT nPos ) const
{
    ImpLineListData* pData = (nPos < pLineList->size()) ? (*pLineList)[ nPos ] : NULL;
    return ( pData ) ? pData->nLine1 : 0;
}

// -----------------------------------------------------------------------

long LineListBox::GetEntryLine2( USHORT nPos ) const
{
    ImpLineListData* pData = (nPos < pLineList->size()) ? (*pLineList)[ nPos ] : NULL;
    return ( pData ) ? pData->nLine2 : 0;
}

// -----------------------------------------------------------------------

long LineListBox::GetEntryDistance( USHORT nPos ) const
{
    ImpLineListData* pData = (nPos < pLineList->size()) ? (*pLineList)[ nPos ] : NULL;
    return ( pData ) ? pData->nDistance : 0;
}

// -----------------------------------------------------------------------

USHORT LineListBox::GetEntryStyle( USHORT nPos ) const
{
    ImpLineListData* pData = (nPos < pLineList->size()) ? (*pLineList)[ nPos ] : NULL;
    return ( pData ) ? pData->nStyle : STYLE_SOLID;
}

// -----------------------------------------------------------------------

void LineListBox::UpdateLineColors( void )
{
    if( UpdatePaintLineColor() )
    {
        size_t nCount = pLineList->size();
        if( !nCount )
            return;

        XubString   aStr;
        Bitmap      aBmp;

        // exchange entries which containing lines
        SetUpdateMode( FALSE );

        USHORT nSelEntry = GetSelectEntryPos();
        for( size_t n = 0 ; n < nCount ; ++n )
        {
            ImpLineListData* pData = (*pLineList)[ n ];
            if( pData )
            {
                // exchange listbox data
                ListBox::RemoveEntry( USHORT( n ) );
                ImpGetLine( pData->nLine1, pData->nLine2, pData->nDistance,
                        pData->nStyle, aBmp, aStr );
                ListBox::InsertEntry( aStr, aBmp, USHORT( n ) );
            }
        }

        if( nSelEntry != LISTBOX_ENTRY_NOTFOUND )
            SelectEntryPos( nSelEntry );

        SetUpdateMode( TRUE );
        Invalidate();
    }
}

// -----------------------------------------------------------------------

BOOL LineListBox::UpdatePaintLineColor( void )
{
    BOOL                    bRet = TRUE;
    const StyleSettings&    rSettings = GetSettings().GetStyleSettings();
    Color                   aNewCol( rSettings.GetWindowColor().IsDark()? rSettings.GetLabelTextColor() : aColor );

    bRet = aNewCol != maPaintCol;

    if( bRet )
        maPaintCol = aNewCol;

    return bRet;
}

// -----------------------------------------------------------------------

void LineListBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    ListBox::DataChanged( rDCEvt );

    if( ( rDCEvt.GetType() == DATACHANGED_SETTINGS ) && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
        UpdateLineColors();
}

// ===================================================================
// FontNameBox
// ===================================================================

struct ImplFontNameListData
{
    FontInfo    maInfo;
    USHORT      mnType;

                ImplFontNameListData( const FontInfo& rInfo,
                                    USHORT nType ) :
                    maInfo( rInfo ),
                    mnType( nType )
                {}
};

// -------------------------------------------------------------------

FontNameBox::FontNameBox( Window* pParent, WinBits nWinStyle ) :
    ComboBox( pParent, nWinStyle )
{
    InitBitmaps();
    mpFontList = NULL;
    mbWYSIWYG = FALSE;
    mbSymbols = FALSE;
}

// -------------------------------------------------------------------

FontNameBox::FontNameBox( Window* pParent, const ResId& rResId ) :
    ComboBox( pParent, rResId )
{
    InitBitmaps();
    mpFontList = NULL;
    mbWYSIWYG = FALSE;
    mbSymbols = FALSE;
}

// -------------------------------------------------------------------

FontNameBox::~FontNameBox()
{
    ImplDestroyFontList();
}

// -------------------------------------------------------------------

void FontNameBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    ComboBox::DataChanged( rDCEvt );

    if( rDCEvt.GetType() == DATACHANGED_SETTINGS && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
        InitBitmaps();
}

// -------------------------------------------------------------------

void FontNameBox::InitBitmaps( void )
{
    maImagePrinterFont = Image( SvtResId( RID_IMG_PRINTERFONT ) );
    maImageBitmapFont = Image( SvtResId( RID_IMG_BITMAPFONT ) );
    maImageScalableFont = Image( SvtResId( RID_IMG_SCALABLEFONT ) );
}

// -------------------------------------------------------------------

void FontNameBox::ImplDestroyFontList()
{
    if ( mpFontList )
    {
        for ( size_t i = 0, n = mpFontList->size(); i < n; ++i ) {
            delete (*mpFontList)[ i ];
        }
        mpFontList->clear();
        delete mpFontList;
    }
}

// -------------------------------------------------------------------

void FontNameBox::Fill( const FontList* pList )
{
    // store old text and clear box
    XubString aOldText = GetText();
    Clear();

    ImplDestroyFontList();
    mpFontList = new ImplFontList;

    // insert fonts
    USHORT nFontCount = pList->GetFontNameCount();
    for ( USHORT i = 0; i < nFontCount; i++ )
    {
        const FontInfo& rFontInfo = pList->GetFontName( i );
        ULONG nIndex = InsertEntry( rFontInfo.GetName() );
        if ( nIndex != LISTBOX_ERROR )
        {
            USHORT nType = pList->GetFontNameType( i );
            ImplFontNameListData* pData = new ImplFontNameListData( rFontInfo, nType );
            if ( nIndex < mpFontList->size() ) {
                ImplFontList::iterator it = mpFontList->begin();
                ::std::advance( it, nIndex );
                mpFontList->insert( it, pData );
            } else {
                mpFontList->push_back( pData );
            }
        }
    }

    ImplCalcUserItemSize();

    // restore text
    if ( aOldText.Len() )
        SetText( aOldText );
}

// -------------------------------------------------------------------

void FontNameBox::EnableWYSIWYG( BOOL bEnable )
{
    if ( bEnable != mbWYSIWYG )
    {
        mbWYSIWYG = bEnable;
        EnableUserDraw( mbWYSIWYG | mbSymbols );
        ImplCalcUserItemSize();
    }
}

// -------------------------------------------------------------------

void FontNameBox::EnableSymbols( BOOL bEnable )
{
    if ( bEnable != mbSymbols )
    {
        mbSymbols = bEnable;
        EnableUserDraw( mbWYSIWYG | mbSymbols );
        ImplCalcUserItemSize();
    }
}

// -------------------------------------------------------------------

void FontNameBox::ImplCalcUserItemSize()
{
    Size aUserItemSz;
    if ( mbWYSIWYG && mpFontList )
    {
        aUserItemSz = Size(MAXPREVIEWWIDTH, GetTextHeight() );
        aUserItemSz.Height() *= 16;
        aUserItemSz.Height() /= 10;
    }
    if ( mbSymbols )
    {
        Size aSz = maImageScalableFont.GetSizePixel();
        aUserItemSz.Width() += aSz.Width() + IMGINNERTEXTSPACE;

        if ( mbWYSIWYG && mpFontList )
            aUserItemSz.Width() += IMGOUTERTEXTSPACE;

        if ( aSz.Height() > aUserItemSz.Height() )
            aUserItemSz.Height() = aSz.Height();
    }
    SetUserItemSize( aUserItemSz );
}

#define MKTAG(s) ((((((s[0]<<8)+s[1])<<8)+s[2])<<8)+s[3])

namespace
{
    rtl::OUString makeRepresentativeSymbolText(bool bOpenSymbol, OutputDevice &rDevice)
    {
        rtl::OUString sSampleText;

        FontCharMap aFontCharMap;
        bool bHasCharMap = rDevice.GetFontCharMap( aFontCharMap );
        if( bHasCharMap )
        {
            // use some sample characters available in the font
            sal_Unicode aText[8];

            // start just above the PUA used by most symbol fonts
            sal_uInt32 cNewChar = 0xFF00;
    #ifdef QUARTZ
            // on MacOSX there are too many non-presentable symbols above the codepoint 0x0192
            if( !bOpenSymbol )
                cNewChar = 0x0192;
    #endif

            const int nMaxCount = sizeof(aText)/sizeof(*aText) - 1;
            int nSkip = aFontCharMap.GetCharCount() / nMaxCount;
            if( nSkip > 10 )
                nSkip = 10;
            else if( nSkip <= 0 )
                nSkip = 1;
            for( int i = 0; i < nMaxCount; ++i )
            {
                sal_uInt32 cOldChar = cNewChar;
                for( int j = nSkip; --j >= 0; )
                    cNewChar = aFontCharMap.GetPrevChar( cNewChar );
                if( cOldChar == cNewChar )
                    break;
                aText[ i ] = static_cast<sal_Unicode>(cNewChar); // TODO: support UCS4 samples
                aText[ i+1 ] = 0;
            }

            sSampleText = rtl::OUString(aText);
        }
        else
        {
            const sal_Unicode* pText = aImplSymbolFontText;
            if( bOpenSymbol )
                pText = aImplStarSymbolText;
            sSampleText = rtl::OUString(pText);
        }

        return sSampleText;
    }

    long shrinkFontToFit(rtl::OUString &rSampleText, long nH, Font &rFont, OutputDevice &rDevice, Rectangle &rTextRect)
    {
        long nWidth = 0;

        Size aSize( rFont.GetSize() );

        //Make sure it fits in the available height
        while (aSize.Height() > 0)
        {
            if (!rDevice.GetTextBoundRect(rTextRect, rSampleText, 0, 0))
                break;
            if (rTextRect.GetHeight() <= nH)
            {
                nWidth = rTextRect.GetWidth();
                break;
            }

            aSize.Height() -= EXTRAFONTSIZE;
            rFont.SetSize(aSize);
            rDevice.SetFont(rFont);
        }

        return nWidth;
    }
}

// -------------------------------------------------------------------

void FontNameBox::UserDraw( const UserDrawEvent& rUDEvt )
{
    ImplFontNameListData*   pData = (*mpFontList)[ rUDEvt.GetItemId() ];
    const FontInfo&         rInfo = pData->maInfo;
    USHORT                  nType = pData->mnType;
    Point                   aTopLeft = rUDEvt.GetRect().TopLeft();
    long                    nX = aTopLeft.X();
    long                    nH = rUDEvt.GetRect().GetHeight();

    if ( mbSymbols )
    {
        nX += IMGINNERTEXTSPACE;
        Image* pImg = NULL;
        if ( (nType & (FONTLIST_FONTNAMETYPE_PRINTER | FONTLIST_FONTNAMETYPE_SCREEN)) == FONTLIST_FONTNAMETYPE_PRINTER )
            pImg = &maImagePrinterFont;
        else if ( nType & FONTLIST_FONTNAMETYPE_SCALABLE )
            pImg = &maImageScalableFont;
        else
            pImg = &maImageBitmapFont;

        if ( pImg )
        {
            Point aPos( nX, aTopLeft.Y() + (nH-pImg->GetSizePixel().Height())/2 );
            rUDEvt.GetDevice()->DrawImage( aPos, *pImg );
        }

        // X immer um gleiche Breite aendern, auch wenn kein Image ausgegeben.
        nX += maImagePrinterFont.GetSizePixel().Width();
    }

    if ( mbWYSIWYG && mpFontList )
    {
        nX += IMGOUTERTEXTSPACE;

        bool bSymbolFont = (rInfo.GetCharSet() == RTL_TEXTENCODING_SYMBOL)
                              || rInfo.GetName().EqualsIgnoreCaseAscii("cmsy10")
                              || rInfo.GetName().EqualsIgnoreCaseAscii("cmex10")
                              || rInfo.GetName().EqualsIgnoreCaseAscii("feta26")
                              || rInfo.GetName().EqualsIgnoreCaseAscii("jsMath-cmsy10")
                              || rInfo.GetName().EqualsIgnoreCaseAscii("jsMath-cmex10")
                              || rInfo.GetName().EqualsIgnoreCaseAscii("msam10")
                              || rInfo.GetName().EqualsIgnoreCaseAscii("msbm10")
                              || rInfo.GetName().EqualsIgnoreCaseAscii("wasy10")
                              || rInfo.GetName().EqualsIgnoreCaseAscii("Denemo")
                              || rInfo.GetName().EqualsIgnoreCaseAscii("GlyphBasic1")
                              || rInfo.GetName().EqualsIgnoreCaseAscii("GlyphBasic2")
                              || rInfo.GetName().EqualsIgnoreCaseAscii("GlyphBasic3")
                              || rInfo.GetName().EqualsIgnoreCaseAscii("GlyphBasic4")
                              || rInfo.GetName().EqualsIgnoreCaseAscii("Letters Laughing")
                              || rInfo.GetName().EqualsIgnoreCaseAscii("MusiQwik")
                              || rInfo.GetName().EqualsIgnoreCaseAscii("MusiSync");

        // starsymbol is a unicode font, but cannot display its own name
        const bool bOpenSymbol = rInfo.GetName().EqualsIgnoreCaseAscii( "starsymbol" )
                              || rInfo.GetName().EqualsIgnoreCaseAscii( "opensymbol" );
        bSymbolFont |= bOpenSymbol;

        Color aTextColor = rUDEvt.GetDevice()->GetTextColor();
        Font aOldFont( rUDEvt.GetDevice()->GetFont() );
        Size aSize( aOldFont.GetSize() );
        aSize.Height() += EXTRAFONTSIZE;
        Font aFont( rInfo );
        aFont.SetSize( aSize );
        rUDEvt.GetDevice()->SetFont( aFont );
        rUDEvt.GetDevice()->SetTextColor( aTextColor );

        bool bUsingCorrectFont = true;
        Rectangle aTextRect;

        // Preview the font name
        rtl::OUString sFontName = rInfo.GetName();

        //If it shouldn't or can't draw its own name because it doesn't have the glyphs
        if (bSymbolFont || (STRING_LEN != rUDEvt.GetDevice()->HasGlyphs(aFont, sFontName)))
            bUsingCorrectFont = false;
        else
        {
            //Make sure it fits in the available height, shrinking the font if necessary
            bUsingCorrectFont = shrinkFontToFit(sFontName, nH, aFont, *rUDEvt.GetDevice(), aTextRect) != 0;
        }

        if (!bUsingCorrectFont)
        {
            rUDEvt.GetDevice()->SetFont(aOldFont);
            rUDEvt.GetDevice()->GetTextBoundRect(aTextRect, sFontName, 0, 0);
        }

        long nTextHeight = aTextRect.GetHeight();
        long nDesiredGap = (nH-nTextHeight)/2;
        long nVertAdjust = nDesiredGap - aTextRect.Top();
//      long nHortAdjust = 0 - aTextRect.Left();
        Point aPos( nX, aTopLeft.Y() + nVertAdjust );
        rUDEvt.GetDevice()->DrawText( aPos, sFontName );
        Rectangle aHack(aPos.X(), aTopLeft.Y() + nH/2 - 5, aPos.X() + 40, aTopLeft.Y() + nH/2 + 5);
        long nTextX = aPos.X() + aTextRect.GetWidth();

        if (!bUsingCorrectFont)
            rUDEvt.GetDevice()->SetFont( aFont );

        rtl::OUString sSampleText;
        bool bHasSampleTextGlyphs=false;

        //TO-DO: for non-symbol fonts pull GPOS/GSUB features through vcl to
        //and map to suitable strings for Arabic, Hebrew, etc fonts

        //If we're a symbol font, or for some reason the font still couldn't
        //render something representative of what it would like to render then
        //make up some semi-random text that it *can* display
        if (bSymbolFont || !bUsingCorrectFont)
        {
            sSampleText = makeRepresentativeSymbolText(bOpenSymbol, *rUDEvt.GetDevice());
            bHasSampleTextGlyphs = (STRING_LEN == rUDEvt.GetDevice()->HasGlyphs(aFont, sSampleText));
        }

        if (sSampleText.getLength() && bHasSampleTextGlyphs)
        {
            const Size &rItemSize = rUDEvt.GetDevice()->GetOutputSize();
            //leave a little border at the edge
            long nSpace = rItemSize.Width() - nTextX - IMGOUTERTEXTSPACE;
            if (nSpace >= 0)
            {
                //Make sure it fits in the available height, and get how wide that would be
                long nWidth = shrinkFontToFit(sSampleText, nH, aFont, *rUDEvt.GetDevice(), aTextRect);
                //Chop letters off until it fits in the available width
                while (nWidth > nSpace || nWidth > MAXPREVIEWWIDTH)
                {
                    sSampleText = sSampleText.copy(0, sSampleText.getLength()-1);
                    nWidth = rUDEvt.GetDevice()->GetTextBoundRect(aTextRect, sSampleText, 0, 0) ?
                             aTextRect.GetWidth() : 0;
                }

                //center the text on the line
                if (sSampleText.getLength() && nWidth)
                {
                    nTextHeight = aTextRect.GetHeight();
                    nDesiredGap = (nH-nTextHeight)/2;
                    nVertAdjust = nDesiredGap - aTextRect.Top();
                    aPos = Point(nTextX + nSpace - nWidth, aTopLeft.Y() + nVertAdjust);
                    rUDEvt.GetDevice()->DrawText( aPos, sSampleText );
                }
            }
        }

        rUDEvt.GetDevice()->SetFont( aOldFont );
        DrawEntry( rUDEvt, FALSE, FALSE);   // draw seperator
    }
    else
    {
        DrawEntry( rUDEvt, TRUE, TRUE );
    }
}

// ===================================================================
// FontStyleBox
// ===================================================================

FontStyleBox::FontStyleBox( Window* pParent, WinBits nWinStyle ) :
    ComboBox( pParent, nWinStyle )
{
}

// -------------------------------------------------------------------

FontStyleBox::FontStyleBox( Window* pParent, const ResId& rResId ) :
    ComboBox( pParent, rResId )
{
    aLastStyle = GetText();
}

// -------------------------------------------------------------------

FontStyleBox::~FontStyleBox()
{
}

// -------------------------------------------------------------------

void FontStyleBox::Select()
{
    // keep text over fill operation
    aLastStyle = GetText();
    ComboBox::Select();
}

// -------------------------------------------------------------------

void FontStyleBox::LoseFocus()
{
    // keep text over fill operation
    aLastStyle = GetText();
    ComboBox::LoseFocus();
}

// -------------------------------------------------------------------

void FontStyleBox::Modify()
{
    CharClass   aChrCls( ::comphelper::getProcessServiceFactory(),
                        GetSettings().GetLocale() );
    XubString   aStr = GetText();
    USHORT      nEntryCount = GetEntryCount();

    if ( GetEntryPos( aStr ) == COMBOBOX_ENTRY_NOTFOUND )
    {
        aChrCls.toUpper( aStr );
        for ( USHORT i = 0; i < nEntryCount; i++ )
        {
            XubString aEntryText = GetEntry( i );
            aChrCls.toUpper( aEntryText );

            if ( aStr == aEntryText )
            {
                SetText( GetEntry( i ) );
                break;
            }
        }
    }

    ComboBox::Modify();
}

// -------------------------------------------------------------------

void FontStyleBox::Fill( const XubString& rName, const FontList* pList )
{
    // note: this method must call ComboBox::SetText(),
    //   else aLastStyle will overwritten
    // store prior selection position and clear box
    XubString aOldText = GetText();
    USHORT nPos = GetEntryPos( aOldText );
    Clear();

    // does a font with this name already exist?
    sal_Handle hFontInfo = pList->GetFirstFontInfo( rName );
    if ( hFontInfo )
    {
        XubString   aStyleText;
        FontWeight  eLastWeight = WEIGHT_DONTKNOW;
        FontItalic  eLastItalic = ITALIC_NONE;
        FontWidth   eLastWidth = WIDTH_DONTKNOW;
        BOOL        bNormal = FALSE;
        BOOL        bItalic = FALSE;
        BOOL        bBold = FALSE;
        BOOL        bBoldItalic = FALSE;
        BOOL        bInsert = FALSE;
        FontInfo    aInfo;
        while ( hFontInfo )
        {
            aInfo = pList->GetFontInfo( hFontInfo );

            FontWeight  eWeight = aInfo.GetWeight();
            FontItalic  eItalic = aInfo.GetItalic();
            FontWidth   eWidth = aInfo.GetWidthType();
            // Only if the attributes are different, we insert the
            // Font to avoid double Entries in different languages
            if ( (eWeight != eLastWeight) || (eItalic != eLastItalic) ||
                 (eWidth != eLastWidth) )
            {
                if ( bInsert )
                    InsertEntry( aStyleText );

                if ( eWeight <= WEIGHT_NORMAL )
                {
                    if ( eItalic != ITALIC_NONE )
                        bItalic = TRUE;
                    else
                        bNormal = TRUE;
                }
                else
                {
                    if ( eItalic != ITALIC_NONE )
                        bBoldItalic = TRUE;
                    else
                        bBold = TRUE;
                }

                // For wrong StyleNames we replace this with the correct once
                aStyleText = pList->GetStyleName( aInfo );
                bInsert = GetEntryPos( aStyleText ) == LISTBOX_ENTRY_NOTFOUND;
                if ( !bInsert )
                {
                    aStyleText = pList->GetStyleName( eWeight, eItalic );
                    bInsert = GetEntryPos( aStyleText ) == LISTBOX_ENTRY_NOTFOUND;
                }

                eLastWeight = eWeight;
                eLastItalic = eItalic;
                eLastWidth = eWidth;
            }
            else
            {
                if ( bInsert )
                {
                    // If we have two names for the same attributes
                    // we prefer the translated standard names
                    const XubString& rAttrStyleText = pList->GetStyleName( eWeight, eItalic );
                    if ( rAttrStyleText != aStyleText )
                    {
                        XubString aTempStyleText = pList->GetStyleName( aInfo );
                        if ( rAttrStyleText == aTempStyleText )
                            aStyleText = rAttrStyleText;
                        bInsert = GetEntryPos( aStyleText ) == LISTBOX_ENTRY_NOTFOUND;
                    }
                }
            }

            if ( !bItalic && (aStyleText == pList->GetItalicStr()) )
                bItalic = TRUE;
            else if ( !bBold && (aStyleText == pList->GetBoldStr()) )
                bBold = TRUE;
            else if ( !bBoldItalic && (aStyleText == pList->GetBoldItalicStr()) )
                bBoldItalic = TRUE;

            hFontInfo = pList->GetNextFontInfo( hFontInfo );
        }

        if ( bInsert )
            InsertEntry( aStyleText );

        // Bestimmte Styles als Nachbildung
        if ( bNormal )
        {
            if ( !bItalic )
                InsertEntry( pList->GetItalicStr() );
            if ( !bBold )
                InsertEntry( pList->GetBoldStr() );
        }
        if ( !bBoldItalic )
        {
            if ( bNormal || bItalic || bBold )
                InsertEntry( pList->GetBoldItalicStr() );
        }
        if ( aOldText.Len() )
        {
            if ( GetEntryPos( aLastStyle ) != LISTBOX_ENTRY_NOTFOUND )
                ComboBox::SetText( aLastStyle );
            else
            {
                if ( nPos >= GetEntryCount() )
                    ComboBox::SetText( GetEntry( 0 ) );
                else
                    ComboBox::SetText( GetEntry( nPos ) );
            }
        }
    }
    else
    {
        // Wenn Font nicht, dann Standard-Styles einfuegen
        InsertEntry( pList->GetNormalStr() );
        InsertEntry( pList->GetItalicStr() );
        InsertEntry( pList->GetBoldStr() );
        InsertEntry( pList->GetBoldItalicStr() );
        if ( aOldText.Len() )
        {
            if ( nPos > GetEntryCount() )
                ComboBox::SetText( GetEntry( 0 ) );
            else
                ComboBox::SetText( GetEntry( nPos ) );
        }
    }
}

// ===================================================================
// FontSizeBox
// ===================================================================

FontSizeBox::FontSizeBox( Window* pParent, WinBits nWinSize ) :
    MetricBox( pParent, nWinSize )
{
    ImplInit();
}

// -----------------------------------------------------------------------

FontSizeBox::FontSizeBox( Window* pParent, const ResId& rResId ) :
    MetricBox( pParent, rResId )
{
    ImplInit();
}

// -----------------------------------------------------------------------

FontSizeBox::~FontSizeBox()
{
}

// -----------------------------------------------------------------------

void FontSizeBox::ImplInit()
{
    EnableAutocomplete( FALSE );

    bRelativeMode   = FALSE;
    bPtRelative     = FALSE;
    bRelative       = FALSE;
    bStdSize        = FALSE;
    pFontList       = NULL;

    SetShowTrailingZeros( FALSE );
    SetDecimalDigits( 1 );
    SetMin( 20 );
    SetMax( 9999 );
    SetProminentEntryType( PROMINENT_MIDDLE );
}

// -----------------------------------------------------------------------

void FontSizeBox::Reformat()
{
    FontSizeNames aFontSizeNames( GetSettings().GetUILanguage() );
    if ( !bRelativeMode || !aFontSizeNames.IsEmpty() )
    {
        long nNewValue = aFontSizeNames.Name2Size( GetText() );
        if ( nNewValue)
        {
            mnLastValue = nNewValue;
            return;
        }
    }

    MetricBox::Reformat();
}

// -----------------------------------------------------------------------

void FontSizeBox::Modify()
{
    MetricBox::Modify();

    if ( bRelativeMode )
    {
        XubString aStr = GetText();
        aStr.EraseLeadingChars();

        BOOL bNewMode = bRelative;
        BOOL bOldPtRelMode = bPtRelative;

        if ( bRelative )
        {
            bPtRelative = FALSE;
            const xub_Unicode* pStr = aStr.GetBuffer();
            while ( *pStr )
            {
                if ( ((*pStr < '0') || (*pStr > '9')) && (*pStr != '%') )
                {
                    if ( ('-' == *pStr || '+' == *pStr) && !bPtRelative )
                        bPtRelative = TRUE;
                    else if ( bPtRelative && 'p' == *pStr && 't' == *++pStr )
                        ;
                    else
                    {
                        bNewMode = FALSE;
                        break;
                    }
                }
                pStr++;
            }
        }
        else
        {
            if ( STRING_NOTFOUND != aStr.Search( '%' ) )
            {
                bNewMode = TRUE;
                bPtRelative = FALSE;
            }

            if ( '-' == aStr.GetChar( 0 ) || '+' == aStr.GetChar( 0 ) )
            {
                bNewMode = TRUE;
                bPtRelative = TRUE;
            }
        }

        if ( bNewMode != bRelative || bPtRelative != bOldPtRelMode )
            SetRelative( bNewMode );
    }
}

// -----------------------------------------------------------------------

void FontSizeBox::Fill( const FontInfo* pInfo, const FontList* pList )
{
    // remember for relative mode
    pFontList = pList;

    // no font sizes need to be set for relative mode
    if ( bRelative )
        return;

    // query font sizes
    const long* pTempAry;
    const long* pAry = 0;

    if( pInfo )
    {
        aFontInfo = *pInfo;
        pAry = pList->GetSizeAry( *pInfo );
    }
    else
    {
        pAry = pList->GetStdSizeAry();
    }

    // first insert font size names (for simplified/traditional chinese)
    FontSizeNames aFontSizeNames( GetSettings().GetUILanguage() );
    if ( pAry == pList->GetStdSizeAry() )
    {
        // for standard sizes we don't need to bother
        if ( bStdSize && GetEntryCount() && aFontSizeNames.IsEmpty() )
            return;
        bStdSize = TRUE;
    }
    else
        bStdSize = FALSE;

    Selection aSelection = GetSelection();
    XubString aStr = GetText();

    Clear();
    USHORT nPos = 0;

    if ( !aFontSizeNames.IsEmpty() )
    {
        if ( pAry == pList->GetStdSizeAry() )
        {
            // for scalable fonts all font size names
            ULONG nCount = aFontSizeNames.Count();
            for( ULONG i = 0; i < nCount; i++ )
            {
                String  aSizeName = aFontSizeNames.GetIndexName( i );
                long    nSize = aFontSizeNames.GetIndexSize( i );
                ComboBox::InsertEntry( aSizeName, nPos );
                ComboBox::SetEntryData( nPos, (void*)(-nSize) ); // mark as special
                nPos++;
            }
        }
        else
        {
            // for fixed size fonts only selectable font size names
            pTempAry = pAry;
            while ( *pTempAry )
            {
                String aSizeName = aFontSizeNames.Size2Name( *pTempAry );
                if ( aSizeName.Len() )
                {
                    ComboBox::InsertEntry( aSizeName, nPos );
                    ComboBox::SetEntryData( nPos, (void*)(-(*pTempAry)) ); // mark as special
                    nPos++;
                }
                pTempAry++;
            }
        }
    }

    // then insert numerical font size values
    pTempAry = pAry;
    while ( *pTempAry )
    {
        InsertValue( *pTempAry, FUNIT_NONE, nPos );
        ComboBox::SetEntryData( nPos, (void*)(*pTempAry) );
        nPos++;
        pTempAry++;
    }

    SetText( aStr );
    SetSelection( aSelection );
}

// -----------------------------------------------------------------------

void FontSizeBox::EnableRelativeMode( USHORT nMin, USHORT nMax, USHORT nStep )
{
    bRelativeMode = TRUE;
    nRelMin       = nMin;
    nRelMax       = nMax;
    nRelStep      = nStep;
    SetUnit( FUNIT_POINT );
}

// -----------------------------------------------------------------------

void FontSizeBox::EnablePtRelativeMode( short nMin, short nMax, short nStep )
{
    bRelativeMode = TRUE;
    nPtRelMin     = nMin;
    nPtRelMax     = nMax;
    nPtRelStep    = nStep;
    SetUnit( FUNIT_POINT );
}

// -----------------------------------------------------------------------

void FontSizeBox::SetRelative( BOOL bNewRelative )
{
    if ( bRelativeMode )
    {
        Selection aSelection = GetSelection();
        XubString  aStr = GetText();
        aStr.EraseLeadingChars();

        if ( bNewRelative )
        {
            bRelative = TRUE;
            bStdSize = FALSE;

            if ( bPtRelative )
            {
                SetDecimalDigits( 1 );
                SetMin( nPtRelMin );
                SetMax( nPtRelMax );
                SetUnit( FUNIT_POINT );

                Clear();

                short i = nPtRelMin, n = 0;
                // JP 30.06.98: more than 100 values are not useful
                while ( i <= nPtRelMax && n++ < 100 )
                {
                    InsertValue( i );
                    i = i + nPtRelStep;
                }
            }
            else
            {
                SetDecimalDigits( 0 );
                SetMin( nRelMin );
                SetMax( nRelMax );
                SetCustomUnitText( '%' );
                SetUnit( FUNIT_CUSTOM );

                Clear();
                USHORT i = nRelMin;
                while ( i <= nRelMax )
                {
                    InsertValue( i );
                    i = i + nRelStep;
                }
            }
        }
        else
        {
            bRelative = bPtRelative = FALSE;
            SetDecimalDigits( 1 );
            SetMin( 20 );
            SetMax( 9999 );
            SetUnit( FUNIT_POINT );
            if ( pFontList )
                Fill( &aFontInfo, pFontList );
        }

        SetText( aStr );
        SetSelection( aSelection );
    }
}

// -----------------------------------------------------------------------

XubString FontSizeBox::CreateFieldText( sal_Int64 nValue ) const
{
    XubString sRet( MetricBox::CreateFieldText( nValue ) );
    if ( bRelativeMode && bPtRelative && (0 <= nValue) && sRet.Len() )
        sRet.Insert( '+', 0 );
    return sRet;
}

// -----------------------------------------------------------------------

void FontSizeBox::SetValue( sal_Int64 nNewValue, FieldUnit eInUnit )
{
    if ( !bRelative )
    {
        sal_Int64 nTempValue = MetricField::ConvertValue( nNewValue, GetBaseValue(), GetDecimalDigits(), eInUnit, GetUnit() );
        FontSizeNames aFontSizeNames( GetSettings().GetUILanguage() );
        // conversion loses precision; however font sizes should
        // never have a problem with that
        String aName = aFontSizeNames.Size2Name( static_cast<long>(nTempValue) );
        if ( aName.Len() && (GetEntryPos( aName ) != LISTBOX_ENTRY_NOTFOUND) )
        {
            mnLastValue = nTempValue;
            SetText( aName );
            mnFieldValue = mnLastValue;
            SetEmptyFieldValueData( FALSE );
            return;
        }
    }

    MetricBox::SetValue( nNewValue, eInUnit );
}

// -----------------------------------------------------------------------

void FontSizeBox::SetValue( sal_Int64 nNewValue )
{
    SetValue( nNewValue, FUNIT_NONE );
}

// -----------------------------------------------------------------------

sal_Int64 FontSizeBox::GetValue( USHORT nPos, FieldUnit eOutUnit ) const
{
    if ( !bRelative )
    {
        sal_Int64 nComboVal = static_cast<sal_Int64>(reinterpret_cast<long>(ComboBox::GetEntryData( nPos )));
        if ( nComboVal < 0 )     // marked as special?
        {
            return MetricField::ConvertValue( -nComboVal, mnBaseValue, GetDecimalDigits(),
                                              meUnit, eOutUnit );
        }
    }

    // do normal font size processing
    sal_Int64 nRetValue = MetricBox::GetValue( nPos, eOutUnit );
    return nRetValue;
}

// -----------------------------------------------------------------------

sal_Int64 FontSizeBox::GetValue( FieldUnit eOutUnit ) const
{
    if ( !bRelative )
    {
        FontSizeNames aFontSizeNames( GetSettings().GetUILanguage() );
        sal_Int64 nValue = aFontSizeNames.Name2Size( GetText() );
        if ( nValue)
            return MetricField::ConvertValue( nValue, GetBaseValue(), GetDecimalDigits(), GetUnit(), eOutUnit );
    }

    return MetricBox::GetValue( eOutUnit );
}

// -----------------------------------------------------------------------

sal_Int64 FontSizeBox::GetValue() const
{
    // implementation not inline, because it is a virtual function
    return GetValue( FUNIT_NONE );
}

// -----------------------------------------------------------------------

void FontSizeBox::SetUserValue( sal_Int64 nNewValue, FieldUnit eInUnit )
{
    if ( !bRelative )
    {
        sal_Int64 nTempValue = MetricField::ConvertValue( nNewValue, GetBaseValue(), GetDecimalDigits(), eInUnit, GetUnit() );
        FontSizeNames aFontSizeNames( GetSettings().GetUILanguage() );
        // conversion loses precision
        // however font sizes should never have a problem with that
        String aName = aFontSizeNames.Size2Name( static_cast<long>(nTempValue) );
        if ( aName.Len() && (GetEntryPos( aName ) != LISTBOX_ENTRY_NOTFOUND) )
        {
            mnLastValue = nTempValue;
            SetText( aName );
            return;
        }
    }

    MetricBox::SetUserValue( nNewValue, eInUnit );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
