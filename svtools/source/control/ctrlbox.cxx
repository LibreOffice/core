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
#include <svtools/ctrlbox.hxx>
#include <svtools/ctrltool.hxx>
#include <svtools/borderhelper.hxx>

#include <vcl/i18nhelp.hxx>
#include <vcl/fontcapabilities.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

#define IMGINNERTEXTSPACE 2
#define IMGOUTERTEXTSPACE 5
#define EXTRAFONTSIZE 5
#define MAXPREVIEWWIDTH 100

#define TWIPS_TO_PT100(val) (val * 5)
#define PT100_TO_TWIPS(val) (val / 5)

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
    sal_Bool        bColor;

                ImplColorListData() : aColor( COL_BLACK ) { bColor = sal_False; }
                ImplColorListData( const Color& rColor ) : aColor( rColor ) { bColor = sal_True; }
};

// -----------------------------------------------------------------------

void ColorListBox::ImplInit()
{
    pColorList = new ImpColorList();
    aImageSize.Width()  = GetTextWidth( XubString( RTL_CONSTASCII_USTRINGPARAM( "xxx" ) ) );
    aImageSize.Height() = GetTextHeight();
    aImageSize.Height() -= 2;

    EnableUserDraw( sal_True );
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

sal_uInt16 ColorListBox::InsertEntry( const XubString& rStr, sal_uInt16 nPos )
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

sal_uInt16 ColorListBox::InsertEntry( const Color& rColor, const XubString& rStr,
                                sal_uInt16 nPos )
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

void ColorListBox::RemoveEntry( sal_uInt16 nPos )
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
        sal_uInt16 nPos = InsertEntry( rBox.GetEntry( n ), LISTBOX_APPEND );
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

sal_uInt16 ColorListBox::GetEntryPos( const Color& rColor ) const
{
    for( sal_uInt16 n = (sal_uInt16) pColorList->size(); n; )
    {
        ImplColorListData* pData = (*pColorList)[ --n ];
        if ( pData->bColor && ( pData->aColor == rColor ) )
            return n;
    }
    return LISTBOX_ENTRY_NOTFOUND;
}

// -----------------------------------------------------------------------

Color ColorListBox::GetEntryColor( sal_uInt16 nPos ) const
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
            ListBox::DrawEntry( rUDEvt, sal_False, sal_True, sal_False );
        }
        else
            ListBox::DrawEntry( rUDEvt, sal_False, sal_True, sal_True );
    }
    else
        ListBox::DrawEntry( rUDEvt, sal_True, sal_True, sal_False );
}

// =======================================================================
// LineListBox
// =======================================================================

BorderWidthImpl::BorderWidthImpl( sal_uInt16 nFlags, double nRate1, double nRate2, double nRateGap ):
    m_nFlags( nFlags ),
    m_nRate1( nRate1 ),
    m_nRate2( nRate2 ),
    m_nRateGap( nRateGap )
{
}

BorderWidthImpl& BorderWidthImpl::operator= ( const BorderWidthImpl& r )
{
    m_nFlags = r.m_nFlags;
    m_nRate1 = r.m_nRate1;
    m_nRate2 = r.m_nRate2;
    m_nRateGap = r.m_nRateGap;
    return *this;
}

bool BorderWidthImpl::operator== ( const BorderWidthImpl& r ) const
{
    return ( m_nFlags == r.m_nFlags ) &&
           ( m_nRate1 == r.m_nRate1 ) &&
           ( m_nRate2 == r.m_nRate2 ) &&
           ( m_nRateGap == r.m_nRateGap );
}

long BorderWidthImpl::GetLine1( long nWidth ) const
{
    long result = m_nRate1;
    if ( ( m_nFlags & CHANGE_LINE1 ) > 0 )
        result = m_nRate1 * nWidth;
    return result;
}

long BorderWidthImpl::GetLine2( long nWidth ) const
{
    long result = m_nRate2;
    if ( ( m_nFlags & CHANGE_LINE2 ) > 0 )
        result = m_nRate2 * nWidth;
    return result;
}

long BorderWidthImpl::GetGap( long nWidth ) const
{
    long result = m_nRateGap;
    if ( ( m_nFlags & CHANGE_DIST ) > 0 )
        result = m_nRateGap * nWidth;

    // Avoid having too small distances
    if ( result < 100 && m_nRate1 > 0 && m_nRate2 > 0 )
        result = 100;

    return result;
}

double lcl_getGuessedWidth( long nTested, double nRate, bool nChanging )
{
    double nWidth = -1.0;
    if ( nChanging )
        nWidth = double( nTested ) / nRate;
    else
    {
        if ( double( nTested ) == nRate )
            nWidth = nRate;
    }

    return nWidth;
}

long BorderWidthImpl::GuessWidth( long nLine1, long nLine2, long nGap )
{
    std::vector< double > aToCompare;
    bool bInvalid = false;

    bool bLine1Change = ( m_nFlags & CHANGE_LINE1 ) > 0;
    double nWidth1 = lcl_getGuessedWidth( nLine1, m_nRate1, bLine1Change );
    if ( bLine1Change )
        aToCompare.push_back( nWidth1 );
    else if ( !bLine1Change && nWidth1 < 0 )
        bInvalid = true;

    bool bLine2Change = ( m_nFlags & CHANGE_LINE2 ) > 0;
    double nWidth2 = lcl_getGuessedWidth( nLine2, m_nRate2, bLine2Change );
    if ( bLine2Change )
        aToCompare.push_back( nWidth2 );
    else if ( !bLine2Change && nWidth2 < 0 )
        bInvalid = true;

    bool bGapChange = ( m_nFlags & CHANGE_DIST ) > 0;
    double nWidthGap = lcl_getGuessedWidth( nGap, m_nRateGap, bGapChange );
    if ( bGapChange )
        aToCompare.push_back( nWidthGap );
    else if ( !bGapChange && nWidthGap < 0 )
        bInvalid = true;

    double nWidth = 0.0;
    if ( !bInvalid && aToCompare.size() > 0 )
    {
        nWidth = *aToCompare.begin();
        std::vector< double >::iterator pIt = aToCompare.begin();
        while ( pIt != aToCompare.end() && !bInvalid )
        {
            bInvalid = ( nWidth != *pIt );
            pIt++;
        }
        if ( bInvalid )
            nWidth = 0.0;
    }

    return long( nWidth );
}

/** Utility class storing the border line width, style and colors. The widths
    are defined in Twips.
  */
class ImpLineListData
{
private:
    BorderWidthImpl m_aWidthImpl;

    Color  ( *m_pColor1Fn )( Color );
    Color  ( *m_pColor2Fn )( Color );
    Color  ( *m_pColorDistFn )( Color, Color );

    long   m_nMinWidth;
    sal_uInt16 m_nStyle;

public:
    ImpLineListData( BorderWidthImpl aWidthImpl, sal_uInt16 nStyle,
            long nMinWidth=0, Color ( *pColor1Fn ) ( Color ) = &sameColor,
            Color ( *pColor2Fn ) ( Color ) = &sameColor, Color ( *pColorDistFn ) ( Color, Color ) = &sameDistColor );

    /** Returns the computed width of the line 1 in twips. */
    long GetLine1ForWidth( long nWidth ) { return m_aWidthImpl.GetLine1( nWidth ); }

    /** Returns the computed width of the line 2 in twips. */
    long GetLine2ForWidth( long nWidth ) { return m_aWidthImpl.GetLine2( nWidth ); }

    /** Returns the computed width of the gap in twips. */
    long GetDistForWidth( long nWidth ) { return m_aWidthImpl.GetGap( nWidth ); }

    Color  GetColorLine1( const Color& aMain );
    Color  GetColorLine2( const Color& aMain );
    Color  GetColorDist( const Color& aMain, const Color& rDefault );

    /** Returns the minimum width in twips */
    long   GetMinWidth( );
    sal_uInt16 GetStyle( );
};

ImpLineListData::ImpLineListData( BorderWidthImpl aWidthImpl,
       sal_uInt16 nStyle, long nMinWidth, Color ( *pColor1Fn )( Color ),
       Color ( *pColor2Fn )( Color ), Color ( *pColorDistFn )( Color, Color ) ) :
    m_aWidthImpl( aWidthImpl ),
    m_pColor1Fn( pColor1Fn ),
    m_pColor2Fn( pColor2Fn ),
    m_pColorDistFn( pColorDistFn ),
    m_nMinWidth( nMinWidth ),
    m_nStyle( nStyle )
{
}

long ImpLineListData::GetMinWidth( )
{
    return m_nMinWidth;
}

Color ImpLineListData::GetColorLine1( const Color& rMain )
{
    return ( *m_pColor1Fn )( rMain );
}

Color ImpLineListData::GetColorLine2( const Color& rMain )
{
    return ( *m_pColor2Fn )( rMain );
}

Color ImpLineListData::GetColorDist( const Color& rMain, const Color& rDefault )
{
    return ( *m_pColorDistFn )( rMain, rDefault );
}

sal_uInt16 LineListBox::GetSelectEntryStyle( sal_uInt16 nSelIndex ) const
{
    sal_uInt16 nStyle = STYLE_SOLID;
    sal_uInt16 nPos = GetSelectEntryPos( nSelIndex );
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        if ( m_sNone.Len( ) > 0 )
            nPos--;
        nStyle = GetEntryStyle( nPos );
    }

    return nStyle;
}

sal_uInt16 ImpLineListData::GetStyle( )
{
    return m_nStyle;
}

DECLARE_LIST( ImpLineList, ImpLineListData* )

// -----------------------------------------------------------------------

void lclDrawPolygon( OutputDevice& rDev, const basegfx::B2DPolygon& rPolygon, long nWidth, sal_uInt16 nDashing )
{
    sal_uInt16 nOldAA = rDev.GetAntialiasing();
    rDev.SetAntialiasing( nOldAA & !ANTIALIASING_ENABLE_B2DDRAW );

    basegfx::B2DPolyPolygon aPolygons = svtools::ApplyLineDashing( rPolygon, nDashing, rDev.GetMapMode().GetMapUnit() );
    for ( sal_uInt32 i = 0; i < aPolygons.count( ); i++ )
    {
        basegfx::B2DPolygon aDash = aPolygons.getB2DPolygon( i );
        basegfx::B2DPoint aStart = aDash.getB2DPoint( 0 );
        basegfx::B2DPoint aEnd = aDash.getB2DPoint( aDash.count() - 1 );

        basegfx::B2DVector aVector( aEnd - aStart );
        aVector.normalize( );
        const basegfx::B2DVector aPerpendicular(basegfx::getPerpendicular(aVector));

        // Handle problems of width 1px in Pixel mode: 0.5px gives a 1px line
        long nPix = rDev.PixelToLogic( Size( 0, 1 ) ).Height();
        if ( rDev.GetMapMode().GetMapUnit() == MAP_PIXEL && nWidth == nPix )
            nWidth = 0;

        const basegfx::B2DVector aWidthOffset( double( nWidth ) / 2 * aPerpendicular);
        basegfx::B2DPolygon aDashPolygon;
        aDashPolygon.append( aStart + aWidthOffset );
        aDashPolygon.append( aEnd + aWidthOffset );
        aDashPolygon.append( aEnd - aWidthOffset );
        aDashPolygon.append( aStart - aWidthOffset );
        aDashPolygon.setClosed( true );

        rDev.DrawPolygon( aDashPolygon );
    }

    rDev.SetAntialiasing( nOldAA );
}

namespace svtools
{
    std::vector < double > GetDashing( sal_uInt16 nDashing, MapUnit eUnit )
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

        return aPattern;
    }

    basegfx::B2DPolyPolygon ApplyLineDashing( const basegfx::B2DPolygon& rPolygon, sal_uInt16 nDashing, MapUnit eUnit )
    {
        std::vector< double > aPattern = GetDashing( nDashing, eUnit );
        basegfx::B2DPolyPolygon aPolygons;
        if ( ! aPattern.empty() )
            basegfx::tools::applyLineDashing( rPolygon, aPattern, &aPolygons );
        else
            aPolygons.append( rPolygon );

        return aPolygons;
    }

    void DrawLine( OutputDevice& rDev, const Point& rP1, const Point& rP2,
        sal_uInt32 nWidth, sal_uInt16 nDashing )
    {
        DrawLine( rDev, basegfx::B2DPoint( rP1.X(), rP1.Y() ),
                basegfx::B2DPoint( rP2.X(), rP2.Y( ) ), nWidth, nDashing );
    }

    void DrawLine( OutputDevice& rDev, const basegfx::B2DPoint& rP1, const basegfx::B2DPoint& rP2,
        sal_uInt32 nWidth, sal_uInt16 nDashing )
    {
        basegfx::B2DPolygon aPolygon;
        aPolygon.append( rP1 );
        aPolygon.append( rP2 );
        lclDrawPolygon( rDev, aPolygon, nWidth, nDashing );
    }
}

void LineListBox::ImpGetLine( long nLine1, long nLine2, long nDistance,
                            Color aColor1, Color aColor2, Color aColorDist,
                            sal_uInt16 nStyle, Bitmap& rBmp, XubString& rStr )
{
    Size aSize = GetOutputSizePixel();
    aSize.Width() -= 20;
    aSize.Width() -= aTxtSize.Width();
    aSize.Height() = aTxtSize.Height();

    // SourceUnit nach Twips
    if ( eSourceUnit == FUNIT_POINT )
    {
        nLine1      /= 5;
        nLine2      /= 5;
        nDistance   /= 5;
    }

    // Linien malen
    aSize = aVirDev.PixelToLogic( aSize );
    long nPix = aVirDev.PixelToLogic( Size( 0, 1 ) ).Height();
    sal_uInt32 n1 = nLine1;
    sal_uInt32 n2 = nLine2;
    long nDist  = nDistance;
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
        aVirDev.SetFillColor( aColorDist );
        aVirDev.DrawRect( Rectangle( Point(), aSize ) );

        aVirDev.SetFillColor( aColor1 );

        double y1 = double( n1 ) / 2;
        svtools::DrawLine( aVirDev, basegfx::B2DPoint( 0, y1 ), basegfx::B2DPoint( aSize.Width( ), y1 ), n1, nStyle );

        if ( n2 )
        {
            double y2 =  n1 + nDist + double( n2 ) / 2;
            aVirDev.SetFillColor( aColor2 );
            svtools::DrawLine( aVirDev, basegfx::B2DPoint( 0, y2 ), basegfx::B2DPoint( aSize.Width(), y2 ), n2, STYLE_SOLID );
        }
        rBmp = aVirDev.GetBitmap( Point(), Size( aSize.Width(), n1+nDist+n2 ) );
    }

    // Twips nach Unit
    if ( eUnit == FUNIT_POINT )
    {
        nLine1      *= 5;
        nLine2      *= 5;
        nDistance   *= 5;
        rStr.AssignAscii( " pt" );
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
    m_nWidth( 5 ),
    m_sNone( ),
    aColor( COL_BLACK ),
    maPaintCol( COL_BLACK )
{
    ImplInit();
}

// -----------------------------------------------------------------------

LineListBox::LineListBox( Window* pParent, const ResId& rResId ) :
    ListBox( pParent, rResId ),
    m_nWidth( 5 ),
    m_sNone( ),
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

sal_uInt16 LineListBox::GetStylePos( sal_uInt16 nListPos, long nWidth )
{
    sal_uInt16 nPos = LISTBOX_ENTRY_NOTFOUND;
    if ( m_sNone.Len( ) > 0 )
        nListPos--;

    sal_uInt16 i = 0;
    sal_uInt16 n = 0;
    sal_uInt16 nCount = pLineList->Count( );
    while ( nPos == LISTBOX_ENTRY_NOTFOUND && i < nCount )
    {
        ImpLineListData* pData = pLineList->GetObject( i );
        if ( pData && pData->GetMinWidth() <= nWidth )
        {
            if ( nListPos == n )
                nPos = i;
            n++;
        }
        i++;
    }

    return nPos;
}


void LineListBox::SelectEntry( sal_uInt16 nStyle, sal_Bool bSelect )
{
    sal_uInt16 nPos = GetEntryPos( nStyle );
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        ListBox::SelectEntryPos( nPos, bSelect );
}

// -----------------------------------------------------------------------

sal_uInt16 LineListBox::InsertEntry( const XubString& rStr, sal_uInt16 nPos )
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

void LineListBox::InsertEntry(
        BorderWidthImpl aWidthImpl,
        sal_uInt16 nStyle, long nMinWidth,
        Color ( *pColor1Fn )( Color ), Color ( *pColor2Fn )( Color ),
        Color ( *pColorDistFn )( Color, Color ) )
{
    ImpLineListData* pData = new ImpLineListData(
            aWidthImpl, nStyle, nMinWidth,
           pColor1Fn, pColor2Fn, pColorDistFn );
    pLineList->push_back( pData );
}

// -----------------------------------------------------------------------

void LineListBox::RemoveEntry( sal_uInt16 nPos )
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

sal_uInt16 LineListBox::GetEntryPos( sal_uInt16 nStyle ) const
{
    for ( size_t i = 0, n = pLineList->size(); i < n; ++i ) {
        ImpLineListData* pData = (*pLineList)[ i ];
        if ( pData )
        {
            if ( GetEntryStyle( i ) == nStyle )
            {
                size_t nPos = i;
                if ( m_sNone.Len() > 0 )
                    nPos ++;
                return (sal_uInt16)nPos;
            }
        }
    }
    return LISTBOX_ENTRY_NOTFOUND;
}

// -----------------------------------------------------------------------

long LineListBox::GetEntryLine1( sal_uInt16 nPos ) const
{
    ImpLineListData* pData = (nPos < pLineList->size()) ? (*pLineList)[ nPos ] : NULL;
    return ( pData ) ? pData->GetLine1ForWidth( m_nWidth ) : 0;
}

// -----------------------------------------------------------------------

long LineListBox::GetEntryLine2( sal_uInt16 nPos ) const
{
    ImpLineListData* pData = (nPos < pLineList->size()) ? (*pLineList)[ nPos ] : NULL;
    return ( pData ) ? pData->GetLine2ForWidth( m_nWidth ) : 0;
}

// -----------------------------------------------------------------------

long LineListBox::GetEntryDistance( sal_uInt16 nPos ) const
{
    ImpLineListData* pData = (nPos < pLineList->size()) ? (*pLineList)[ nPos ] : NULL;
    return ( pData ) ? pData->GetDistForWidth( m_nWidth ) : 0;
}

// -----------------------------------------------------------------------

sal_uInt16 LineListBox::GetEntryStyle( sal_uInt16 nPos ) const
{
    ImpLineListData* pData = (nPos < pLineList->size()) ? (*pLineList)[ nPos ] : NULL;
    return ( pData ) ? pData->GetStyle() : STYLE_SOLID;
}

// -----------------------------------------------------------------------

sal_Bool LineListBox::UpdatePaintLineColor( void )
{
    sal_Bool                    bRet = sal_True;
    const StyleSettings&    rSettings = GetSettings().GetStyleSettings();
    Color                   aNewCol( rSettings.GetWindowColor().IsDark()? rSettings.GetLabelTextColor() : aColor );

    bRet = aNewCol != maPaintCol;

    if( bRet )
        maPaintCol = aNewCol;

    return bRet;
}

void LineListBox::UpdateEntries( long nOldWidth )
{
    SetUpdateMode( sal_False );

    UpdatePaintLineColor( );

    sal_uInt16      nSelEntry = GetSelectEntryPos();
    sal_uInt16       nTypePos = GetStylePos( nSelEntry, nOldWidth );

    // Remove the old entries
    while ( GetEntryCount( ) > 0 )
        ListBox::RemoveEntry( 0 );

    // Add the new entries based on the defined width
    if ( m_sNone.Len( ) > 0 )
        ListBox::InsertEntry( m_sNone, LISTBOX_APPEND );

    sal_uInt16 n = 0;
    sal_uInt16 nCount = pLineList->Count( );
    while ( n < nCount )
    {
        ImpLineListData* pData = pLineList->GetObject( n );
        if ( pData && pData->GetMinWidth() <= m_nWidth )
        {
            XubString   aStr;
            Bitmap      aBmp;
            ImpGetLine( pData->GetLine1ForWidth( m_nWidth ),
                    pData->GetLine2ForWidth( m_nWidth ),
                    pData->GetDistForWidth( m_nWidth ),
                    GetColorLine1( GetEntryCount( ) ),
                    GetColorLine2( GetEntryCount( ) ),
                    GetColorDist( GetEntryCount( ) ),
                    pData->GetStyle(), aBmp, aStr );
            ListBox::InsertEntry( aStr, aBmp, LISTBOX_APPEND );
            if ( n == nTypePos )
                SelectEntryPos( GetEntryCount() - 1 );
        }
        else if ( n == nTypePos )
            SetNoSelection();
        n++;
    }

    SetUpdateMode( sal_True );
    Invalidate();
}

// -----------------------------------------------------------------------

Color LineListBox::GetColorLine1( sal_uInt16 nPos )
{
    Color rResult = GetPaintColor( );

    sal_uInt16 nStyle = GetStylePos( nPos, m_nWidth );
    ImpLineListData* pData = pLineList->GetObject( nStyle );
    if ( pData )
        rResult = pData->GetColorLine1( GetColor( ) );

    return rResult;
}

Color LineListBox::GetColorLine2( sal_uInt16 nPos )
{
    Color rResult = GetPaintColor( );

    sal_uInt16 nStyle = GetStylePos( nPos, m_nWidth );
    ImpLineListData* pData = pLineList->GetObject( nStyle );
    if ( pData )
        rResult = pData->GetColorLine2( GetColor( ) );

    return rResult;
}

Color LineListBox::GetColorDist( sal_uInt16 nPos )
{
    Color rResult = GetSettings().GetStyleSettings().GetFieldColor();

    sal_uInt16 nStyle = GetStylePos( nPos, m_nWidth );
    ImpLineListData* pData = pLineList->GetObject( nStyle );
    if ( pData )
        rResult = pData->GetColorDist( GetColor( ), rResult );

    return rResult;
}

// -----------------------------------------------------------------------

void LineListBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    ListBox::DataChanged( rDCEvt );

    if( ( rDCEvt.GetType() == DATACHANGED_SETTINGS ) && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
        UpdateEntries( m_nWidth );
}


// ===================================================================
// FontNameBox
// ===================================================================

struct ImplFontNameListData
{
    FontInfo    maInfo;
    sal_uInt16      mnType;

                ImplFontNameListData( const FontInfo& rInfo,
                                    sal_uInt16 nType ) :
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
    mbWYSIWYG = sal_False;
    mbSymbols = sal_False;
}

// -------------------------------------------------------------------

FontNameBox::FontNameBox( Window* pParent, const ResId& rResId ) :
    ComboBox( pParent, rResId )
{
    InitBitmaps();
    mpFontList = NULL;
    mbWYSIWYG = sal_False;
    mbSymbols = sal_False;
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
    sal_uInt16 nFontCount = pList->GetFontNameCount();
    for ( sal_uInt16 i = 0; i < nFontCount; i++ )
    {
        const FontInfo& rFontInfo = pList->GetFontName( i );
        sal_uLong nIndex = InsertEntry( rFontInfo.GetName() );
        if ( nIndex != LISTBOX_ERROR )
        {
            sal_uInt16 nType = pList->GetFontNameType( i );
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

void FontNameBox::EnableWYSIWYG( sal_Bool bEnable )
{
    if ( bEnable != mbWYSIWYG )
    {
        mbWYSIWYG = bEnable;
        EnableUserDraw( mbWYSIWYG | mbSymbols );
        ImplCalcUserItemSize();
    }
}

// -------------------------------------------------------------------

void FontNameBox::EnableSymbols( sal_Bool bEnable )
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

#define MKTAG(s) sal_uInt32((((((s[0]<<8)+s[1])<<8)+s[2])<<8)+s[3])
#define TRADITIONAL_CHINESE 0x01000000
#define SIMPLIFIED_CHINESE  0x02000000
#define JAPANESE            0x04000000
#define KOREAN              0x08000000
#define HEBREW_MINIMAL      0x10000000
#define GREEK_MINIMAL       0x20000000

namespace
{
    rtl::OUString getRepresentativeText(size_t nScript)
    {
        rtl::OUString sSampleText;
        switch (nScript)
        {
            case vcl::UnicodeCoverage::GREEK_AND_COPTIC:
            {
                const sal_Unicode aGrek[] = {
                    0x0391, 0x03BB, 0x03C6, 0x03AC, 0x03B2, 0x03B7, 0x03C4, 0x03BF
                };
                sSampleText = rtl::OUString(aGrek, SAL_N_ELEMENTS(aGrek));
                break;
            }
            case GREEK_MINIMAL:
            {
                const sal_Unicode aGrek[] = {
                    0x0391, 0x0392
                };
                sSampleText = rtl::OUString(aGrek, SAL_N_ELEMENTS(aGrek));
                break;
            }
            case vcl::UnicodeCoverage::HEBREW:
            {
                const sal_Unicode aHebr[] = {
                    0x05D0, 0x05B8, 0x05DC, 0x05B6, 0x05E3, 0x05D1, 0x05B5, 0x05BC,
                    0x05D9, 0x05EA, 0x0020, 0x05E2, 0x05B4, 0x05D1, 0x05B0, 0x05E8,
                    0x05B4, 0x05D9
                };
                sSampleText = rtl::OUString(aHebr, SAL_N_ELEMENTS(aHebr));
                break;
            }
            case HEBREW_MINIMAL:
            {
                const sal_Unicode aHebr[] = {
                    0x05D0, 0x05D1
                };
                sSampleText = rtl::OUString(aHebr, SAL_N_ELEMENTS(aHebr));
                break;
            }
            case vcl::UnicodeCoverage::ARABIC:
            {
                const sal_Unicode aArab[] = {
                    0x0623, 0x0628, 0x062C, 0x062F, 0x064A, 0x0629, 0x0020, 0x0639,
                    0x0631, 0x0628, 0x064A, 0x0629
                };
                sSampleText = rtl::OUString(aArab, SAL_N_ELEMENTS(aArab));
                break;
            }
            case vcl::UnicodeCoverage::DEVANAGARI:
            {
                const sal_Unicode aDeva[] = {
                    0x0926, 0x0947, 0x0935, 0x0928, 0x093E, 0x0917, 0x0930, 0x0940
                };
                sSampleText = rtl::OUString(aDeva, SAL_N_ELEMENTS(aDeva));
                break;
            }
            case vcl::UnicodeCoverage::BENGALI:
            {
                const sal_Unicode aBeng[] = {
                    0x09AC, 0x09BE, 0x0982, 0x09B2, 0x09BE, 0x0020, 0x09B2, 0x09BF,
                    0x09AA, 0x09BF
                };
                sSampleText = rtl::OUString(aBeng, SAL_N_ELEMENTS(aBeng));
                break;
            }
            case vcl::UnicodeCoverage::GURMUKHI:
            {
                const sal_Unicode aGuru[] = {
                    0x0A17, 0x0A41, 0x0A30, 0x0A2E, 0x0A41, 0x0A16, 0x0A40
                };
                sSampleText = rtl::OUString(aGuru, SAL_N_ELEMENTS(aGuru));
                break;
            }
            case vcl::UnicodeCoverage::GUJARATI:
            {
                const sal_Unicode aGujr[] = {
                    0x0A97, 0x0AC1, 0x0A9C, 0x0AB0, 0x0ABE, 0x0AA4, 0x0aC0, 0x0020,
                    0x0AB2, 0x0ABF, 0x0AAA, 0x0ABF
                };
                sSampleText = rtl::OUString(aGujr, SAL_N_ELEMENTS(aGujr));
                break;
            }
            case vcl::UnicodeCoverage::ORIYA:
            {
                const sal_Unicode aOrya[] = {
                    0x0B09, 0x0B24, 0x0B4D, 0x0B15, 0x0B33, 0x0020, 0x0B32, 0x0B3F,
                    0x0B2A, 0x0B3F
                };
                sSampleText = rtl::OUString(aOrya, SAL_N_ELEMENTS(aOrya));
                break;
            }
            case vcl::UnicodeCoverage::TAMIL:
            {
                const sal_Unicode aTaml[] = {
                    0x0B85, 0x0BB0, 0x0BBF, 0x0B9A, 0x0BCD, 0x0B9A, 0x0BC1, 0x0BB5,
                    0x0B9F, 0x0BBF
                };
                sSampleText = rtl::OUString(aTaml, SAL_N_ELEMENTS(aTaml));
                break;
            }
            case vcl::UnicodeCoverage::TELUGU:
            {
                const sal_Unicode aTelu[] = {
                    0x0C24, 0x0C46, 0x0C32, 0x0C41, 0x0C17, 0x0C41
                };
                sSampleText = rtl::OUString(aTelu, SAL_N_ELEMENTS(aTelu));
                break;
            }
            case vcl::UnicodeCoverage::KANNADA:
            {
                const sal_Unicode aKnda[] = {
                    0x0C95, 0x0CA8, 0x0CCD, 0x0CA8, 0x0CA1, 0x0020, 0x0CB2, 0x0CBF,
                    0x0CAA, 0x0CBF
                };
                sSampleText = rtl::OUString(aKnda, SAL_N_ELEMENTS(aKnda));
                break;
            }
            case vcl::UnicodeCoverage::MALAYALAM:
            {
                const sal_Unicode aMlym[] = {
                    0x0D2E, 0x0D32, 0x0D2F, 0x0D3E, 0x0D33, 0x0D32, 0x0D3F, 0x0D2A,
                    0x0D3F
                };
                sSampleText = rtl::OUString(aMlym, SAL_N_ELEMENTS(aMlym));
                break;
            }
            case vcl::UnicodeCoverage::THAI:
            {
                const sal_Unicode aThai[] = {
                    0x0E2D, 0x0E31, 0x0E01, 0x0E29, 0x0E23, 0x0E44, 0x0E17, 0x0E22
                };
                sSampleText = rtl::OUString(aThai, SAL_N_ELEMENTS(aThai));
                break;
            }
            case vcl::UnicodeCoverage::LAO:
            {
                const sal_Unicode aLao[] = {
                    0x0EAD, 0x0EB1, 0x0E81, 0x0EAA, 0x0EAD, 0x0E99, 0x0EA5, 0x0EB2,
                    0x0EA7
                };
                sSampleText = rtl::OUString(aLao, SAL_N_ELEMENTS(aLao));
                break;
            }
            case vcl::UnicodeCoverage::GEORGIAN:
            {
                const sal_Unicode aGeorgian[] = {
                    0x10D3, 0x10D0, 0x10DB, 0x10EC, 0x10D4, 0x10E0, 0x10DA, 0x10DD,
                    0x10D1, 0x10D0
                };
                sSampleText = rtl::OUString(aGeorgian, SAL_N_ELEMENTS(aGeorgian));
                break;
            }
            case vcl::UnicodeCoverage::HANGUL_JAMO:
            case KOREAN:
            {
                const sal_Unicode aHang[] = {
                    0xD55C, 0xAE00
                };
                sSampleText = rtl::OUString(aHang, SAL_N_ELEMENTS(aHang));
                break;
            }
            case vcl::UnicodeCoverage::TIBETAN:
            {
                const sal_Unicode aTibt[] = {
                    0x0F51, 0x0F56, 0x0F74, 0x0F0B, 0x0F45, 0x0F53, 0x0F0B
                };
                sSampleText = rtl::OUString(aTibt, SAL_N_ELEMENTS(aTibt));
                break;
            }
            case vcl::UnicodeCoverage::SYRIAC:
            {
                const sal_Unicode aSyri[] = {
                    0x0723, 0x071B, 0x072A, 0x0722, 0x0713, 0x0720, 0x0710
                };
                sSampleText = rtl::OUString(aSyri, SAL_N_ELEMENTS(aSyri));
                break;
            }
            case vcl::UnicodeCoverage::THAANA:
            {
                const sal_Unicode aThaa[] = {
                    0x078C, 0x07A7, 0x0782, 0x07A6
                };
                sSampleText = rtl::OUString(aThaa, SAL_N_ELEMENTS(aThaa));
                break;
            }
            case vcl::UnicodeCoverage::SINHALA:
            {
                const sal_Unicode aSinh[] = {
                    0x0DC1, 0x0DD4, 0x0DAF, 0x0DCA, 0x0DB0, 0x0020, 0x0DC3, 0x0DD2,
                    0x0D82, 0x0DC4, 0x0DBD
                };
                sSampleText = rtl::OUString(aSinh, SAL_N_ELEMENTS(aSinh));
                break;
            }
            case vcl::UnicodeCoverage::MYANMAR:
            {
                const sal_Unicode aMymr[] = {
                    0x1019, 0x103C, 0x1014, 0x103A, 0x1019, 0x102C, 0x1021, 0x1000,
                    0x1039, 0x1001, 0x101B, 0x102C
                };
                sSampleText = rtl::OUString(aMymr, SAL_N_ELEMENTS(aMymr));
                break;
            }
            case vcl::UnicodeCoverage::ETHIOPIC:
            {
                const sal_Unicode aEthi[] = {
                    0x130D, 0x12D5, 0x12DD
                };
                sSampleText = rtl::OUString(aEthi, SAL_N_ELEMENTS(aEthi));
                break;
            }
            case vcl::UnicodeCoverage::CHEROKEE:
            {
                const sal_Unicode aCher[] = {
                    0x13D7, 0x13AA, 0x13EA, 0x13B6, 0x13D9, 0x13D7
                };
                sSampleText = rtl::OUString(aCher, SAL_N_ELEMENTS(aCher));
                break;
            }
            case vcl::UnicodeCoverage::KHMER:
            {
                const sal_Unicode aKhmr[] = {
                    0x17A2, 0x1780, 0x17D2, 0x1781, 0x179A, 0x1780, 0x17D2, 0x179A,
                    0x1798, 0x1781, 0x17C1, 0x1798, 0x179A, 0x1797, 0x17B6, 0x179F,
                    0x17B6
                };
                sSampleText = rtl::OUString(aKhmr, SAL_N_ELEMENTS(aKhmr));
                break;
            }
            case vcl::UnicodeCoverage::MONGOLIAN:
            {
                const sal_Unicode aMongolian[] = {
                    0x182A, 0x1822, 0x1834, 0x1822, 0x182D, 0x180C
                };
                sSampleText = rtl::OUString(aMongolian, SAL_N_ELEMENTS(aMongolian));
                break;
            }
            case vcl::UnicodeCoverage::TAGALOG:
            {
                const sal_Unicode aTagalog[] = {
                    0x170A, 0x170A, 0x170C, 0x1712
                };
                sSampleText = rtl::OUString(aTagalog, SAL_N_ELEMENTS(aTagalog));
                break;
            }
            case vcl::UnicodeCoverage::NEW_TAI_LUE:
            {
                const sal_Unicode aTalu[] = {
                    0x1991, 0x19BA, 0x199F, 0x19B9, 0x19C9
                };
                sSampleText = rtl::OUString(aTalu, SAL_N_ELEMENTS(aTalu));
                break;
            }
            case TRADITIONAL_CHINESE:
            {
                const sal_Unicode aTraditionalChinese[] = {
                    0x7E41
                };
                sSampleText = rtl::OUString(aTraditionalChinese, SAL_N_ELEMENTS(aTraditionalChinese));
                break;
            }
            case SIMPLIFIED_CHINESE:
            {
                const sal_Unicode aSimplifiedChinese[] = {
                    0x7B80
                };
                sSampleText = rtl::OUString(aSimplifiedChinese, SAL_N_ELEMENTS(aSimplifiedChinese));
                break;
            }
            case TRADITIONAL_CHINESE|SIMPLIFIED_CHINESE:
            {
                const sal_Unicode aSimplifiedAndTraditionalChinese[] = {
                    0x7B80, 0x7E41
                };
                sSampleText = rtl::OUString(aSimplifiedAndTraditionalChinese, SAL_N_ELEMENTS(aSimplifiedAndTraditionalChinese));
                break;
            }
            case JAPANESE:
            {
                const sal_Unicode aJapanese[] = {
                    0x65E5, 0x672C, 0x8A9E
                };
                sSampleText = rtl::OUString(aJapanese, SAL_N_ELEMENTS(aJapanese));
                break;
            }
            default:
                break;
        }
        return sSampleText;
    }

#if OSL_DEBUG_LEVEL > 2
    void lcl_dump_unicode_coverage(const boost::dynamic_bitset<sal_uInt32> &rIn)
    {
        if (rIn[vcl::UnicodeCoverage::BASIC_LATIN])
            fprintf(stderr, "BASIC_LATIN\n");
        if (rIn[vcl::UnicodeCoverage::LATIN_1_SUPPLEMENT])
            fprintf(stderr, "LATIN_1_SUPPLEMENT\n");
        if (rIn[vcl::UnicodeCoverage::LATIN_EXTENDED_A])
            fprintf(stderr, "LATIN_EXTENDED_A\n");
        if (rIn[vcl::UnicodeCoverage::LATIN_EXTENDED_B])
            fprintf(stderr, "LATIN_EXTENDED_B\n");
        if (rIn[vcl::UnicodeCoverage::IPA_EXTENSIONS])
            fprintf(stderr, "IPA_EXTENSIONS\n");
        if (rIn[vcl::UnicodeCoverage::SPACING_MODIFIER_LETTERS])
            fprintf(stderr, "SPACING_MODIFIER_LETTERS\n");
        if (rIn[vcl::UnicodeCoverage::COMBINING_DIACRITICAL_MARKS])
            fprintf(stderr, "COMBINING_DIACRITICAL_MARKS\n");
        if (rIn[vcl::UnicodeCoverage::GREEK_AND_COPTIC])
            fprintf(stderr, "GREEK_AND_COPTIC\n");
        if (rIn[vcl::UnicodeCoverage::COPTIC])
            fprintf(stderr, "COPTIC\n");
        if (rIn[vcl::UnicodeCoverage::CYRILLIC])
            fprintf(stderr, "CYRILLIC\n");
        if (rIn[vcl::UnicodeCoverage::ARMENIAN])
            fprintf(stderr, "ARMENIAN\n");
        if (rIn[vcl::UnicodeCoverage::HEBREW])
            fprintf(stderr, "HEBREW\n");
        if (rIn[vcl::UnicodeCoverage::VAI])
            fprintf(stderr, "VAI\n");
        if (rIn[vcl::UnicodeCoverage::ARABIC])
            fprintf(stderr, "ARABIC\n");
        if (rIn[vcl::UnicodeCoverage::NKO])
            fprintf(stderr, "NKO\n");
        if (rIn[vcl::UnicodeCoverage::DEVANAGARI])
            fprintf(stderr, "DEVANAGARI\n");
        if (rIn[vcl::UnicodeCoverage::BENGALI])
            fprintf(stderr, "BENGALI\n");
        if (rIn[vcl::UnicodeCoverage::GURMUKHI])
            fprintf(stderr, "GURMUKHI\n");
        if (rIn[vcl::UnicodeCoverage::GUJARATI])
            fprintf(stderr, "GUJARATI\n");
        if (rIn[vcl::UnicodeCoverage::ORIYA])
            fprintf(stderr, "ORIYA\n");
        if (rIn[vcl::UnicodeCoverage::TAMIL])
            fprintf(stderr, "TAMIL\n");
        if (rIn[vcl::UnicodeCoverage::TELUGU])
            fprintf(stderr, "TELUGU\n");
        if (rIn[vcl::UnicodeCoverage::KANNADA])
            fprintf(stderr, "KANNADA\n");
        if (rIn[vcl::UnicodeCoverage::MALAYALAM])
            fprintf(stderr, "MALAYALAM\n");
        if (rIn[vcl::UnicodeCoverage::THAI])
            fprintf(stderr, "THAI\n");
        if (rIn[vcl::UnicodeCoverage::LAO])
            fprintf(stderr, "LAO\n");
        if (rIn[vcl::UnicodeCoverage::GEORGIAN])
            fprintf(stderr, "GEORGIAN\n");
        if (rIn[vcl::UnicodeCoverage::BALINESE])
            fprintf(stderr, "BALINESE\n");
        if (rIn[vcl::UnicodeCoverage::HANGUL_JAMO])
            fprintf(stderr, "HANGUL_JAMO\n");
        if (rIn[vcl::UnicodeCoverage::LATIN_EXTENDED_ADDITIONAL])
            fprintf(stderr, "LATIN_EXTENDED_ADDITIONAL\n");
        if (rIn[vcl::UnicodeCoverage::GREEK_EXTENDED])
            fprintf(stderr, "GREEK_EXTENDED\n");
        if (rIn[vcl::UnicodeCoverage::GENERAL_PUNCTUATION])
            fprintf(stderr, "GENERAL_PUNCTUATION\n");
        if (rIn[vcl::UnicodeCoverage::SUPERSCRIPTS_AND_SUBSCRIPTS])
            fprintf(stderr, "SUPERSCRIPTS_AND_SUBSCRIPTS\n");
        if (rIn[vcl::UnicodeCoverage::CURRENCY_SYMBOLS])
            fprintf(stderr, "CURRENCY_SYMBOLS\n");
        if (rIn[vcl::UnicodeCoverage::COMBINING_DIACRITICAL_MARKS_FOR_SYMBOLS])
            fprintf(stderr, "COMBINING_DIACRITICAL_MARKS_FOR_SYMBOLS\n");
        if (rIn[vcl::UnicodeCoverage::LETTERLIKE_SYMBOLS])
            fprintf(stderr, "LETTERLIKE_SYMBOLS\n");
        if (rIn[vcl::UnicodeCoverage::NUMBER_FORMS])
            fprintf(stderr, "NUMBER_FORMS\n");
        if (rIn[vcl::UnicodeCoverage::ARROWS])
            fprintf(stderr, "ARROWS\n");
        if (rIn[vcl::UnicodeCoverage::MATHEMATICAL_OPERATORS])
            fprintf(stderr, "MATHEMATICAL_OPERATORS\n");
        if (rIn[vcl::UnicodeCoverage::MISCELLANEOUS_TECHNICAL])
            fprintf(stderr, "MISCELLANEOUS_TECHNICAL\n");
        if (rIn[vcl::UnicodeCoverage::CONTROL_PICTURES])
            fprintf(stderr, "CONTROL_PICTURES\n");
        if (rIn[vcl::UnicodeCoverage::OPTICAL_CHARACTER_RECOGNITION])
            fprintf(stderr, "OPTICAL_CHARACTER_RECOGNITION\n");
        if (rIn[vcl::UnicodeCoverage::ENCLOSED_ALPHANUMERICS])
            fprintf(stderr, "ENCLOSED_ALPHANUMERICS\n");
        if (rIn[vcl::UnicodeCoverage::BOX_DRAWING])
            fprintf(stderr, "BOX_DRAWING\n");
        if (rIn[vcl::UnicodeCoverage::BLOCK_ELEMENTS])
            fprintf(stderr, "BLOCK_ELEMENTS\n");
        if (rIn[vcl::UnicodeCoverage::GEOMETRIC_SHAPES])
            fprintf(stderr, "GEOMETRIC_SHAPES\n");
        if (rIn[vcl::UnicodeCoverage::MISCELLANEOUS_SYMBOLS])
            fprintf(stderr, "MISCELLANEOUS_SYMBOLS\n");
        if (rIn[vcl::UnicodeCoverage::DINGBATS])
            fprintf(stderr, "DINGBATS\n");
        if (rIn[vcl::UnicodeCoverage::CJK_SYMBOLS_AND_PUNCTUATION])
            fprintf(stderr, "CJK_SYMBOLS_AND_PUNCTUATION\n");
        if (rIn[vcl::UnicodeCoverage::HIRAGANA])
            fprintf(stderr, "HIRAGANA\n");
        if (rIn[vcl::UnicodeCoverage::KATAKANA])
            fprintf(stderr, "KATAKANA\n");
        if (rIn[vcl::UnicodeCoverage::BOPOMOFO])
            fprintf(stderr, "BOPOMOFO\n");
        if (rIn[vcl::UnicodeCoverage::HANGUL_COMPATIBILITY_JAMO])
            fprintf(stderr, "HANGUL_COMPATIBILITY_JAMO\n");
        if (rIn[vcl::UnicodeCoverage::PHAGS_PA])
            fprintf(stderr, "PHAGS_PA\n");
        if (rIn[vcl::UnicodeCoverage::ENCLOSED_CJK_LETTERS_AND_MONTHS])
            fprintf(stderr, "ENCLOSED_CJK_LETTERS_AND_MONTHS\n");
        if (rIn[vcl::UnicodeCoverage::CJK_COMPATIBILITY])
            fprintf(stderr, "CJK_COMPATIBILITY\n");
        if (rIn[vcl::UnicodeCoverage::HANGUL_SYLLABLES])
            fprintf(stderr, "HANGUL_SYLLABLES\n");
        if (rIn[vcl::UnicodeCoverage::NONPLANE_0])
            fprintf(stderr, "NONPLANE_0\n");
        if (rIn[vcl::UnicodeCoverage::PHOENICIAN])
            fprintf(stderr, "PHOENICIAN\n");
        if (rIn[vcl::UnicodeCoverage::CJK_UNIFIED_IDEOGRAPHS])
            fprintf(stderr, "CJK_UNIFIED_IDEOGRAPHS\n");
        if (rIn[vcl::UnicodeCoverage::PRIVATE_USE_AREA_PLANE_0])
            fprintf(stderr, "PRIVATE_USE_AREA_PLANE_0\n");
        if (rIn[vcl::UnicodeCoverage::CJK_STROKES])
            fprintf(stderr, "CJK_STROKES\n");
        if (rIn[vcl::UnicodeCoverage::ALPHABETIC_PRESENTATION_FORMS])
            fprintf(stderr, "ALPHABETIC_PRESENTATION_FORMS\n");
        if (rIn[vcl::UnicodeCoverage::ARABIC_PRESENTATION_FORMS_A])
            fprintf(stderr, "ARABIC_PRESENTATION_FORMS_A\n");
        if (rIn[vcl::UnicodeCoverage::COMBINING_HALF_MARKS])
            fprintf(stderr, "COMBINING_HALF_MARKS\n");
        if (rIn[vcl::UnicodeCoverage::VERTICAL_FORMS])
            fprintf(stderr, "VERTICAL_FORMS\n");
        if (rIn[vcl::UnicodeCoverage::SMALL_FORM_VARIANTS])
            fprintf(stderr, "SMALL_FORM_VARIANTS\n");
        if (rIn[vcl::UnicodeCoverage::ARABIC_PRESENTATION_FORMS_B])
            fprintf(stderr, "ARABIC_PRESENTATION_FORMS_B\n");
        if (rIn[vcl::UnicodeCoverage::HALFWIDTH_AND_FULLWIDTH_FORMS])
            fprintf(stderr, "HALFWIDTH_AND_FULLWIDTH_FORMS\n");
        if (rIn[vcl::UnicodeCoverage::SPECIALS])
            fprintf(stderr, "SPECIALS\n");
        if (rIn[vcl::UnicodeCoverage::TIBETAN])
            fprintf(stderr, "TIBETAN\n");
        if (rIn[vcl::UnicodeCoverage::SYRIAC])
            fprintf(stderr, "SYRIAC\n");
        if (rIn[vcl::UnicodeCoverage::THAANA])
            fprintf(stderr, "THAANA\n");
        if (rIn[vcl::UnicodeCoverage::SINHALA])
            fprintf(stderr, "SINHALA\n");
        if (rIn[vcl::UnicodeCoverage::MYANMAR])
            fprintf(stderr, "MYANMAR\n");
        if (rIn[vcl::UnicodeCoverage::ETHIOPIC])
            fprintf(stderr, "ETHIOPIC\n");
        if (rIn[vcl::UnicodeCoverage::CHEROKEE])
            fprintf(stderr, "CHEROKEE\n");
        if (rIn[vcl::UnicodeCoverage::UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS])
            fprintf(stderr, "UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS\n");
        if (rIn[vcl::UnicodeCoverage::OGHAM])
            fprintf(stderr, "OGHAM\n");
        if (rIn[vcl::UnicodeCoverage::RUNIC])
            fprintf(stderr, "RUNIC\n");
        if (rIn[vcl::UnicodeCoverage::KHMER])
            fprintf(stderr, "KHMER\n");
        if (rIn[vcl::UnicodeCoverage::MONGOLIAN])
            fprintf(stderr, "MONGOLIAN\n");
        if (rIn[vcl::UnicodeCoverage::BRAILLE_PATTERNS])
            fprintf(stderr, "BRAILLE_PATTERNS\n");
        if (rIn[vcl::UnicodeCoverage::YI_SYLLABLES])
            fprintf(stderr, "YI_SYLLABLES\n");
        if (rIn[vcl::UnicodeCoverage::TAGALOG])
            fprintf(stderr, "TAGALOG\n");
        if (rIn[vcl::UnicodeCoverage::OLD_ITALIC])
            fprintf(stderr, "OLD_ITALIC\n");
        if (rIn[vcl::UnicodeCoverage::GOTHIC])
            fprintf(stderr, "GOTHIC\n");
        if (rIn[vcl::UnicodeCoverage::DESERET])
            fprintf(stderr, "DESERET\n");
        if (rIn[vcl::UnicodeCoverage::BYZANTINE_MUSICAL_SYMBOLS])
            fprintf(stderr, "BYZANTINE_MUSICAL_SYMBOLS\n");
        if (rIn[vcl::UnicodeCoverage::MATHEMATICAL_ALPHANUMERIC_SYMBOLS])
            fprintf(stderr, "MATHEMATICAL_ALPHANUMERIC_SYMBOLS\n");
        if (rIn[vcl::UnicodeCoverage::PRIVATE_USE_PLANE_15])
            fprintf(stderr, "PRIVATE_USE_PLANE_15\n");
        if (rIn[vcl::UnicodeCoverage::VARIATION_SELECTORS])
            fprintf(stderr, "VARIATION_SELECTORS\n");
        if (rIn[vcl::UnicodeCoverage::TAGS])
            fprintf(stderr, "TAGS\n");
        if (rIn[vcl::UnicodeCoverage::LIMBU])
            fprintf(stderr, "LIMBU\n");
        if (rIn[vcl::UnicodeCoverage::TAI_LE])
            fprintf(stderr, "TAI_LE\n");
        if (rIn[vcl::UnicodeCoverage::NEW_TAI_LUE])
            fprintf(stderr, "NEW_TAI_LUE\n");
        if (rIn[vcl::UnicodeCoverage::BUGINESE])
            fprintf(stderr, "BUGINESE\n");
        if (rIn[vcl::UnicodeCoverage::GLAGOLITIC])
            fprintf(stderr, "GLAGOLITIC\n");
        if (rIn[vcl::UnicodeCoverage::TIFINAGH])
            fprintf(stderr, "TIFINAGH\n");
        if (rIn[vcl::UnicodeCoverage::YIJING_HEXAGRAM_SYMBOLS])
            fprintf(stderr, "YIJING_HEXAGRAM_SYMBOLS\n");
        if (rIn[vcl::UnicodeCoverage::SYLOTI_NAGRI])
            fprintf(stderr, "SYLOTI_NAGRI\n");
        if (rIn[vcl::UnicodeCoverage::LINEAR_B_SYLLABARY])
            fprintf(stderr, "LINEAR_B_SYLLABARY\n");
        if (rIn[vcl::UnicodeCoverage::ANCIENT_GREEK_NUMBERS])
            fprintf(stderr, "ANCIENT_GREEK_NUMBERS\n");
        if (rIn[vcl::UnicodeCoverage::UGARITIC])
            fprintf(stderr, "UGARITIC\n");
        if (rIn[vcl::UnicodeCoverage::OLD_PERSIAN])
            fprintf(stderr, "OLD_PERSIAN\n");
        if (rIn[vcl::UnicodeCoverage::SHAVIAN])
            fprintf(stderr, "SHAVIAN\n");
        if (rIn[vcl::UnicodeCoverage::OSMANYA])
            fprintf(stderr, "OSMANYA\n");
        if (rIn[vcl::UnicodeCoverage::CYPRIOT_SYLLABARY])
            fprintf(stderr, "CYPRIOT_SYLLABARY\n");
        if (rIn[vcl::UnicodeCoverage::KHAROSHTHI])
            fprintf(stderr, "KHAROSHTHI\n");
        if (rIn[vcl::UnicodeCoverage::TAI_XUAN_JING_SYMBOLS])
            fprintf(stderr, "TAI_XUAN_JING_SYMBOLS\n");
        if (rIn[vcl::UnicodeCoverage::CUNEIFORM])
            fprintf(stderr, "CUNEIFORM\n");
        if (rIn[vcl::UnicodeCoverage::COUNTING_ROD_NUMERALS])
            fprintf(stderr, "COUNTING_ROD_NUMERALS\n");
        if (rIn[vcl::UnicodeCoverage::SUNDANESE])
            fprintf(stderr, "SUNDANESE\n");
        if (rIn[vcl::UnicodeCoverage::LEPCHA])
            fprintf(stderr, "LEPCHA\n");
        if (rIn[vcl::UnicodeCoverage::OL_CHIKI])
            fprintf(stderr, "OL_CHIKI\n");
        if (rIn[vcl::UnicodeCoverage::SAURASHTRA])
            fprintf(stderr, "SAURASHTRA\n");
        if (rIn[vcl::UnicodeCoverage::KAYAH_LI])
            fprintf(stderr, "KAYAH_LI\n");
        if (rIn[vcl::UnicodeCoverage::REJANG])
            fprintf(stderr, "REJANG\n");
        if (rIn[vcl::UnicodeCoverage::CHAM])
            fprintf(stderr, "CHAM\n");
        if (rIn[vcl::UnicodeCoverage::ANCIENT_SYMBOLS])
            fprintf(stderr, "ANCIENT_SYMBOLS\n");
        if (rIn[vcl::UnicodeCoverage::PHAISTOS_DISC])
            fprintf(stderr, "PHAISTOS_DISC\n");
        if (rIn[vcl::UnicodeCoverage::CARIAN])
            fprintf(stderr, "CARIAN\n");
        if (rIn[vcl::UnicodeCoverage::DOMINO_TILES])
            fprintf(stderr, "DOMINO_TILES\n");
        if (rIn[vcl::UnicodeCoverage::RESERVED1])
            fprintf(stderr, "RESERVED1\n");
        if (rIn[vcl::UnicodeCoverage::RESERVED2])
            fprintf(stderr, "RESERVED2\n");
        if (rIn[vcl::UnicodeCoverage::RESERVED3])
            fprintf(stderr, "RESERVED3\n");
        if (rIn[vcl::UnicodeCoverage::RESERVED4])
            fprintf(stderr, "RESERVED4\n");
        if (rIn[vcl::UnicodeCoverage::RESERVED5])
            fprintf(stderr, "RESERVED5\n");
    }

    void lcl_dump_codepage_coverage(const boost::dynamic_bitset<sal_uInt32> &rIn)
    {
        if (rIn[vcl::CodePageCoverage::CP1252])
            fprintf(stderr, "CP1252\n");
        if (rIn[vcl::CodePageCoverage::CP1250])
            fprintf(stderr, "CP1250\n");
        if (rIn[vcl::CodePageCoverage::CP1251])
            fprintf(stderr, "CP1251\n");
        if (rIn[vcl::CodePageCoverage::CP1253])
            fprintf(stderr, "CP1253\n");
        if (rIn[vcl::CodePageCoverage::CP1254])
            fprintf(stderr, "CP1254\n");
        if (rIn[vcl::CodePageCoverage::CP1255])
            fprintf(stderr, "CP1255\n");
        if (rIn[vcl::CodePageCoverage::CP1256])
            fprintf(stderr, "CP1256\n");
        if (rIn[vcl::CodePageCoverage::CP1257])
            fprintf(stderr, "CP1257\n");
        if (rIn[vcl::CodePageCoverage::CP1258])
            fprintf(stderr, "CP1258\n");
        if (rIn[vcl::CodePageCoverage::CP874])
            fprintf(stderr, "CP874\n");
        if (rIn[vcl::CodePageCoverage::CP932])
            fprintf(stderr, "CP932\n");
        if (rIn[vcl::CodePageCoverage::CP936])
            fprintf(stderr, "CP936\n");
        if (rIn[vcl::CodePageCoverage::CP949])
            fprintf(stderr, "CP949\n");
        if (rIn[vcl::CodePageCoverage::CP950])
            fprintf(stderr, "CP950\n");
        if (rIn[vcl::CodePageCoverage::CP1361])
            fprintf(stderr, "CP1361\n");
        if (rIn[vcl::CodePageCoverage::CP869])
            fprintf(stderr, "CP869\n");
        if (rIn[vcl::CodePageCoverage::CP866])
            fprintf(stderr, "CP866\n");
        if (rIn[vcl::CodePageCoverage::CP865])
            fprintf(stderr, "CP865\n");
        if (rIn[vcl::CodePageCoverage::CP864])
            fprintf(stderr, "CP864\n");
        if (rIn[vcl::CodePageCoverage::CP863])
            fprintf(stderr, "CP863\n");
        if (rIn[vcl::CodePageCoverage::CP862])
            fprintf(stderr, "CP862\n");
        if (rIn[vcl::CodePageCoverage::CP861])
            fprintf(stderr, "CP861\n");
        if (rIn[vcl::CodePageCoverage::CP860])
            fprintf(stderr, "CP860\n");
        if (rIn[vcl::CodePageCoverage::CP857])
            fprintf(stderr, "CP857\n");
        if (rIn[vcl::CodePageCoverage::CP855])
            fprintf(stderr, "CP855\n");
        if (rIn[vcl::CodePageCoverage::CP852])
            fprintf(stderr, "CP852\n");
        if (rIn[vcl::CodePageCoverage::CP775])
            fprintf(stderr, "CP775\n");
        if (rIn[vcl::CodePageCoverage::CP737])
            fprintf(stderr, "CP737\n");
        if (rIn[vcl::CodePageCoverage::CP780])
            fprintf(stderr, "CP780\n");
        if (rIn[vcl::CodePageCoverage::CP850])
            fprintf(stderr, "CP850\n");
        if (rIn[vcl::CodePageCoverage::CP437])
            fprintf(stderr, "CP437\n");
    }
#endif

    size_t getScript(const vcl::FontCapabilities &rFontCapabilities)
    {
        boost::dynamic_bitset<sal_uInt32> aGenericMask(vcl::UnicodeCoverage::MAX_UC_ENUM);
        aGenericMask.set();
        aGenericMask.set(vcl::UnicodeCoverage::BASIC_LATIN, false);
        aGenericMask.set(vcl::UnicodeCoverage::LATIN_1_SUPPLEMENT, false);
        aGenericMask.set(vcl::UnicodeCoverage::LATIN_EXTENDED_A, false);
        aGenericMask.set(vcl::UnicodeCoverage::LATIN_EXTENDED_B, false);
        aGenericMask.set(vcl::UnicodeCoverage::IPA_EXTENSIONS, false);
        aGenericMask.set(vcl::UnicodeCoverage::SPACING_MODIFIER_LETTERS, false);
        aGenericMask.set(vcl::UnicodeCoverage::COMBINING_DIACRITICAL_MARKS, false);
        aGenericMask.set(vcl::UnicodeCoverage::COMBINING_DIACRITICAL_MARKS_FOR_SYMBOLS, false);
        aGenericMask.set(vcl::UnicodeCoverage::LATIN_EXTENDED_ADDITIONAL, false);
        aGenericMask.set(vcl::UnicodeCoverage::GENERAL_PUNCTUATION, false);
        aGenericMask.set(vcl::UnicodeCoverage::GEOMETRIC_SHAPES, false);
        aGenericMask.set(vcl::UnicodeCoverage::SUPERSCRIPTS_AND_SUBSCRIPTS, false);
        aGenericMask.set(vcl::UnicodeCoverage::CURRENCY_SYMBOLS, false);
        aGenericMask.set(vcl::UnicodeCoverage::LETTERLIKE_SYMBOLS, false);
        aGenericMask.set(vcl::UnicodeCoverage::DINGBATS, false);
        aGenericMask.set(vcl::UnicodeCoverage::PRIVATE_USE_AREA_PLANE_0, false);
        aGenericMask.set(vcl::UnicodeCoverage::ALPHABETIC_PRESENTATION_FORMS, false);
        aGenericMask.set(vcl::UnicodeCoverage::NUMBER_FORMS, false);
        aGenericMask.set(vcl::UnicodeCoverage::ARROWS, false);
        aGenericMask.set(vcl::UnicodeCoverage::MATHEMATICAL_OPERATORS, false);
        aGenericMask.set(vcl::UnicodeCoverage::MATHEMATICAL_ALPHANUMERIC_SYMBOLS, false);
        aGenericMask.set(vcl::UnicodeCoverage::MISCELLANEOUS_TECHNICAL, false);
        aGenericMask.set(vcl::UnicodeCoverage::CONTROL_PICTURES, false);
        aGenericMask.set(vcl::UnicodeCoverage::ENCLOSED_ALPHANUMERICS, false);
        aGenericMask.set(vcl::UnicodeCoverage::BOX_DRAWING, false);
        aGenericMask.set(vcl::UnicodeCoverage::BLOCK_ELEMENTS, false);
        aGenericMask.set(vcl::UnicodeCoverage::MISCELLANEOUS_SYMBOLS, false);
        aGenericMask.set(vcl::UnicodeCoverage::SPECIALS, false);
        aGenericMask.set(vcl::UnicodeCoverage::NONPLANE_0, false);
        aGenericMask.set(vcl::UnicodeCoverage::PRIVATE_USE_PLANE_15, false);
        aGenericMask.set(vcl::UnicodeCoverage::CJK_SYMBOLS_AND_PUNCTUATION, false);

        boost::dynamic_bitset<sal_uInt32> aMasked = rFontCapabilities.maUnicodeRange & aGenericMask;

        if (aMasked.count() == 1)
            return aMasked.find_first();


        if (aMasked[vcl::UnicodeCoverage::ARABIC])
        {
            aMasked.set(vcl::UnicodeCoverage::ARABIC_PRESENTATION_FORMS_A, false);
            aMasked.set(vcl::UnicodeCoverage::ARABIC_PRESENTATION_FORMS_B, false);
            aMasked.set(vcl::UnicodeCoverage::NKO, false);
            //Probably strongly tuned for Arabic
            if (aMasked.count() == 1)
                return vcl::UnicodeCoverage::ARABIC;
            if (aMasked.count() == 2 && aMasked[vcl::UnicodeCoverage::SYRIAC])
                return vcl::UnicodeCoverage::SYRIAC;
        }

        if (aMasked[vcl::UnicodeCoverage::DEVANAGARI])
        {
            aMasked.set(vcl::UnicodeCoverage::DEVANAGARI, false);
            //Probably strongly tuned for a single Indic script
            if (aMasked.count() == 1)
                return aMasked.find_first();
        }

        aMasked.set(vcl::UnicodeCoverage::GREEK_EXTENDED, false);
        aMasked.set(vcl::UnicodeCoverage::GREEK_AND_COPTIC, false);
        if (aMasked.count() == 1)
            return aMasked.find_first();

        if (aMasked[vcl::UnicodeCoverage::CYRILLIC])
        {
            //Probably strongly tuned for Georgian
            if (aMasked.count() == 2 && aMasked[vcl::UnicodeCoverage::GEORGIAN])
                return vcl::UnicodeCoverage::GEORGIAN;
        }

        boost::dynamic_bitset<sal_uInt32> aCJKMask(vcl::UnicodeCoverage::MAX_UC_ENUM);
        aCJKMask.set();
        aCJKMask.set(vcl::UnicodeCoverage::CJK_SYMBOLS_AND_PUNCTUATION, false);
        aCJKMask.set(vcl::UnicodeCoverage::HIRAGANA, false);
        aCJKMask.set(vcl::UnicodeCoverage::KATAKANA, false);
        aCJKMask.set(vcl::UnicodeCoverage::HANGUL_JAMO, false);
        aCJKMask.set(vcl::UnicodeCoverage::HANGUL_SYLLABLES, false);
        aCJKMask.set(vcl::UnicodeCoverage::HANGUL_COMPATIBILITY_JAMO, false);
        aCJKMask.set(vcl::UnicodeCoverage::ENCLOSED_CJK_LETTERS_AND_MONTHS, false);
        aCJKMask.set(vcl::UnicodeCoverage::CJK_COMPATIBILITY, false);
        aCJKMask.set(vcl::UnicodeCoverage::CJK_UNIFIED_IDEOGRAPHS, false);
        aCJKMask.set(vcl::UnicodeCoverage::CJK_STROKES, false);
        aCJKMask.set(vcl::UnicodeCoverage::HALFWIDTH_AND_FULLWIDTH_FORMS, false);
        aCJKMask.set(vcl::UnicodeCoverage::VERTICAL_FORMS, false);
        aCJKMask.set(vcl::UnicodeCoverage::BOPOMOFO, false);
        aCJKMask.set(vcl::UnicodeCoverage::SMALL_FORM_VARIANTS, false);
        aCJKMask.set(vcl::UnicodeCoverage::PHAGS_PA, false);
        aCJKMask.set(vcl::UnicodeCoverage::CYRILLIC, false);
        aCJKMask.set(vcl::UnicodeCoverage::THAI, false);
        aCJKMask.set(vcl::UnicodeCoverage::DESERET, false);

        aMasked = aMasked & aCJKMask;

        //So, apparently a CJK font
        if (!aMasked.count())
        {
            boost::dynamic_bitset<sal_uInt32> aCJKCodePageMask(vcl::CodePageCoverage::MAX_CP_ENUM);
            aCJKCodePageMask.set(vcl::CodePageCoverage::CP932);
            aCJKCodePageMask.set(vcl::CodePageCoverage::CP936);
            aCJKCodePageMask.set(vcl::CodePageCoverage::CP949);
            aCJKCodePageMask.set(vcl::CodePageCoverage::CP950);
            aCJKCodePageMask.set(vcl::CodePageCoverage::CP1361);
            boost::dynamic_bitset<sal_uInt32> aMaskedCodePage =
                rFontCapabilities.maCodePageRange & aCJKCodePageMask;
            //fold Korean
            if (aMaskedCodePage[vcl::CodePageCoverage::CP1361])
            {
                aMaskedCodePage.set(vcl::CodePageCoverage::CP949);
                aMaskedCodePage.set(vcl::CodePageCoverage::CP1361, false);
            }

            sal_uInt32 nRet = 0;
            if (aMaskedCodePage[vcl::CodePageCoverage::CP936])
                nRet |= SIMPLIFIED_CHINESE;
            if (aMaskedCodePage[vcl::CodePageCoverage::CP950])
                nRet |= TRADITIONAL_CHINESE;
            if (aMaskedCodePage[vcl::CodePageCoverage::CP932])
                nRet |= JAPANESE;
            if (aMaskedCodePage[vcl::CodePageCoverage::CP949])
                nRet |= KOREAN;
            return nRet;
        }

        return 0;
    }

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
            static sal_Unicode aImplSymbolFontText[] = {
                0xF021,0xF032,0xF043,0xF054,0xF065,0xF076,0xF0B7,0xF0C8,0};
            const sal_Unicode* pText = aImplSymbolFontText;
            if( bOpenSymbol )
            {
                static sal_Unicode aImplStarSymbolText[] = {
                    0x2706,0x2704,0x270D,0xE033,0x2211,0x2288,0};
                pText = aImplStarSymbolText;
            }
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
    sal_uInt16                  nType = pData->mnType;
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

        if (!bSymbolFont)
        {
            const bool bNameBeginsWithLatinText = rInfo.GetName().GetChar(0) <= 'z';
            vcl::FontCapabilities aFontCapabilities;
#if OSL_DEBUG_LEVEL > 2
            lcl_dump_unicode_coverage(aFontCapabilities.maUnicodeRange);
            lcl_dump_codepage_coverage(aFontCapabilities.maCodePageRange);
#endif
            if (bNameBeginsWithLatinText && rUDEvt.GetDevice()->GetFontCapabilities(aFontCapabilities))
            {
                //If this font is probably tuned to display a single non-Latin
                //script and the font name is itself in Latin, then show a small
                //chunk of representative text for that script
                size_t nScript = getScript(aFontCapabilities);
                if (nScript)
                {
                    //If we're a CJK font, see if we seem to be tuned
                    //for C, J or K
                    if (nScript & JAPANESE && nScript ^ JAPANESE)
                    {
                        const sal_Unicode aJapanese[] = { 0x3007, 0x9F9D };
                        rtl::OUString sJapanese(aJapanese, SAL_N_ELEMENTS(aJapanese));
                        if (STRING_LEN != rUDEvt.GetDevice()->HasGlyphs(aFont, sJapanese))
                            nScript ^= JAPANESE;
                    }

                    if (nScript & TRADITIONAL_CHINESE && nScript ^ TRADITIONAL_CHINESE)
                    {
                        const sal_Unicode aTraditionalChinese[] = { 0xFA0D };
                        rtl::OUString sTraditionalChinese(aTraditionalChinese, SAL_N_ELEMENTS(aTraditionalChinese));
                        if (STRING_LEN != rUDEvt.GetDevice()->HasGlyphs(aFont, sTraditionalChinese))
                            nScript ^= TRADITIONAL_CHINESE;
                    }

                    if (nScript & SIMPLIFIED_CHINESE && nScript ^ SIMPLIFIED_CHINESE)
                    {
                        const sal_Unicode aSimplifiedChinese[] = { 0x9FA0 };
                        rtl::OUString sSimplifiedChinese(aSimplifiedChinese, SAL_N_ELEMENTS(aSimplifiedChinese));
                        if (STRING_LEN != rUDEvt.GetDevice()->HasGlyphs(aFont, sSimplifiedChinese))
                            nScript ^= SIMPLIFIED_CHINESE;
                    }

                    sSampleText = getRepresentativeText(nScript);
                    bHasSampleTextGlyphs = (STRING_LEN == rUDEvt.GetDevice()->HasGlyphs(aFont, sSampleText));
                }
            }
        }

        //If we're not a symbol font, but could neither render our own name and
        //we can't determine what script it would like to render, then try a
        //few well known scripts
        if (!sSampleText.getLength() && !bUsingCorrectFont)
        {
            static const sal_Int32 aScripts[] =
            {
                vcl::UnicodeCoverage::ARABIC,
                vcl::UnicodeCoverage::HEBREW,

                vcl::UnicodeCoverage::BENGALI,
                vcl::UnicodeCoverage::GURMUKHI,
                vcl::UnicodeCoverage::GUJARATI,
                vcl::UnicodeCoverage::ORIYA,
                vcl::UnicodeCoverage::TAMIL,
                vcl::UnicodeCoverage::TELUGU,
                vcl::UnicodeCoverage::KANNADA,
                vcl::UnicodeCoverage::MALAYALAM,
                vcl::UnicodeCoverage::SINHALA,
                vcl::UnicodeCoverage::DEVANAGARI,

                vcl::UnicodeCoverage::THAI,
                vcl::UnicodeCoverage::LAO,
                vcl::UnicodeCoverage::GEORGIAN,
                vcl::UnicodeCoverage::TIBETAN,
                vcl::UnicodeCoverage::SYRIAC,
                vcl::UnicodeCoverage::MYANMAR,
                vcl::UnicodeCoverage::ETHIOPIC,
                vcl::UnicodeCoverage::KHMER,
                vcl::UnicodeCoverage::MONGOLIAN,

                KOREAN,
                JAPANESE,
                TRADITIONAL_CHINESE|SIMPLIFIED_CHINESE,

                vcl::UnicodeCoverage::GREEK_AND_COPTIC,

                HEBREW_MINIMAL,
                GREEK_MINIMAL
            };

            for (size_t i = 0; i < SAL_N_ELEMENTS(aScripts); ++i)
            {
                sSampleText = getRepresentativeText(aScripts[i]);
                if (sSampleText.getLength())
                {
                    bHasSampleTextGlyphs = (STRING_LEN == rUDEvt.GetDevice()->HasGlyphs(aFont, sSampleText));
                    if (bHasSampleTextGlyphs)
                        break;
                }
            }
        }

        //If we're a symbol font, or for some reason the font still couldn't
        //render something representative of what it would like to render then
        //make up some semi-random text that it *can* display
        if (bSymbolFont || (!bUsingCorrectFont && !bHasSampleTextGlyphs))
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
        DrawEntry( rUDEvt, sal_False, sal_False);   // draw seperator
    }
    else
    {
        DrawEntry( rUDEvt, sal_True, sal_True );
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
    sal_uInt16      nEntryCount = GetEntryCount();

    if ( GetEntryPos( aStr ) == COMBOBOX_ENTRY_NOTFOUND )
    {
        aChrCls.toUpper( aStr );
        for ( sal_uInt16 i = 0; i < nEntryCount; i++ )
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
    sal_uInt16 nPos = GetEntryPos( aOldText );
    Clear();

    // does a font with this name already exist?
    sal_Handle hFontInfo = pList->GetFirstFontInfo( rName );
    if ( hFontInfo )
    {
        XubString   aStyleText;
        FontWeight  eLastWeight = WEIGHT_DONTKNOW;
        FontItalic  eLastItalic = ITALIC_NONE;
        FontWidth   eLastWidth = WIDTH_DONTKNOW;
        sal_Bool        bNormal = sal_False;
        sal_Bool        bItalic = sal_False;
        sal_Bool        bBold = sal_False;
        sal_Bool        bBoldItalic = sal_False;
        sal_Bool        bInsert = sal_False;
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
                        bItalic = sal_True;
                    else
                        bNormal = sal_True;
                }
                else
                {
                    if ( eItalic != ITALIC_NONE )
                        bBoldItalic = sal_True;
                    else
                        bBold = sal_True;
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
                bItalic = sal_True;
            else if ( !bBold && (aStyleText == pList->GetBoldStr()) )
                bBold = sal_True;
            else if ( !bBoldItalic && (aStyleText == pList->GetBoldItalicStr()) )
                bBoldItalic = sal_True;

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
    EnableAutocomplete( sal_False );

    bRelativeMode   = sal_False;
    bPtRelative     = sal_False;
    bRelative       = sal_False;
    bStdSize        = sal_False;
    pFontList       = NULL;

    SetShowTrailingZeros( sal_False );
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

        sal_Bool bNewMode = bRelative;
        sal_Bool bOldPtRelMode = bPtRelative;

        if ( bRelative )
        {
            bPtRelative = sal_False;
            const xub_Unicode* pStr = aStr.GetBuffer();
            while ( *pStr )
            {
                if ( ((*pStr < '0') || (*pStr > '9')) && (*pStr != '%') )
                {
                    if ( ('-' == *pStr || '+' == *pStr) && !bPtRelative )
                        bPtRelative = sal_True;
                    else if ( bPtRelative && 'p' == *pStr && 't' == *++pStr )
                        ;
                    else
                    {
                        bNewMode = sal_False;
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
                bNewMode = sal_True;
                bPtRelative = sal_False;
            }

            if ( '-' == aStr.GetChar( 0 ) || '+' == aStr.GetChar( 0 ) )
            {
                bNewMode = sal_True;
                bPtRelative = sal_True;
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
        bStdSize = sal_True;
    }
    else
        bStdSize = sal_False;

    Selection aSelection = GetSelection();
    XubString aStr = GetText();

    Clear();
    sal_uInt16 nPos = 0;

    if ( !aFontSizeNames.IsEmpty() )
    {
        if ( pAry == pList->GetStdSizeAry() )
        {
            // for scalable fonts all font size names
            sal_uLong nCount = aFontSizeNames.Count();
            for( sal_uLong i = 0; i < nCount; i++ )
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

void FontSizeBox::EnableRelativeMode( sal_uInt16 nMin, sal_uInt16 nMax, sal_uInt16 nStep )
{
    bRelativeMode = sal_True;
    nRelMin       = nMin;
    nRelMax       = nMax;
    nRelStep      = nStep;
    SetUnit( FUNIT_POINT );
}

// -----------------------------------------------------------------------

void FontSizeBox::EnablePtRelativeMode( short nMin, short nMax, short nStep )
{
    bRelativeMode = sal_True;
    nPtRelMin     = nMin;
    nPtRelMax     = nMax;
    nPtRelStep    = nStep;
    SetUnit( FUNIT_POINT );
}

// -----------------------------------------------------------------------

void FontSizeBox::SetRelative( sal_Bool bNewRelative )
{
    if ( bRelativeMode )
    {
        Selection aSelection = GetSelection();
        XubString  aStr = GetText();
        aStr.EraseLeadingChars();

        if ( bNewRelative )
        {
            bRelative = sal_True;
            bStdSize = sal_False;

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
                sal_uInt16 i = nRelMin;
                while ( i <= nRelMax )
                {
                    InsertValue( i );
                    i = i + nRelStep;
                }
            }
        }
        else
        {
            bRelative = bPtRelative = sal_False;
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
            SetEmptyFieldValueData( sal_False );
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

sal_Int64 FontSizeBox::GetValue( sal_uInt16 nPos, FieldUnit eOutUnit ) const
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
