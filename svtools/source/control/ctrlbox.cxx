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


#define _CTRLBOX_CXX
#include <tools/stream.hxx>
#include <vcl/svapp.hxx>
#include <vcl/field.hxx>
#include <vcl/helper.hxx>
#include <sal/macros.h>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <unotools/charclass.hxx>

#include <svtools/sampletext.hxx>
#include <svtools/svtresid.hxx>
#include <svtools/svtools.hrc>
#include <svtools/ctrlbox.hxx>
#include <svtools/ctrltool.hxx>
#include <svtools/borderhelper.hxx>

#include <vcl/i18nhelp.hxx>
#include <vcl/fontcapabilities.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

#include <rtl/bootstrap.hxx>

#if OSL_DEBUG_LEVEL > 1
#include <cstdio>
#endif

#include <stdio.h>

#define IMGINNERTEXTSPACE 2
#define IMGOUTERTEXTSPACE 5
#define EXTRAFONTSIZE 5
#define GAPTOEXTRAPREVIEW 10
#define MAXPREVIEWWIDTH 120
#define MINGAPWIDTH 2

#define FONTNAMEBOXMRUENTRIESFILE "/user/config/fontnameboxmruentries"

using namespace ::com::sun::star;

// ========================================================================
// ColorListBox
// ========================================================================

// --------------------
// - ImplColorListData -
// --------------------

class ImplColorListData
{
public:
    Color       aColor;
    sal_Bool        bColor;

                ImplColorListData() : aColor( COL_BLACK ) { bColor = sal_False; }
                ImplColorListData( const Color& rColor ) : aColor( rColor ) { bColor = sal_True; }
};

// -----------------------------------------------------------------------

void ColorListBox::ImplInit()
{
    pColorList = new ImpColorList();
    aImageSize.Width()  = GetTextWidth( rtl::OUString("xxx") );
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

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeColorListBox(Window *pParent)
{
    ColorListBox *pListBox = new ColorListBox(pParent, WB_LEFT|WB_DROPDOWN|WB_VCENTER|WB_3DLOOK);
    pListBox->SetDropDownLineCount(16); //arbitrary
    return pListBox;
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

void ColorListBox::InsertAutomaticEntryColor(const Color &rColor)
{
    // insert the "Automatic"-entry always on the first position
    InsertEntry( rColor, SVT_RESSTR(STR_SVT_AUTOMATIC_COLOR), 0 );
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
    long result = static_cast<long>(m_nRate1);
    if ( ( m_nFlags & CHANGE_LINE1 ) > 0 )
    {
        long const nConstant2 = (m_nFlags & CHANGE_LINE2) ? 0 : m_nRate2;
        long const nConstantD = (m_nFlags & CHANGE_DIST ) ? 0 : m_nRateGap;
        result = std::max<long>(0,
                    static_cast<long>((m_nRate1 * nWidth) + 0.5)
                        - (nConstant2 + nConstantD));
        if (result == 0 && m_nRate1 > 0.0 && nWidth > 0)
        {   // fdo#51777: hack to essentially treat 1 twip DOUBLE border
            result = 1;  // as 1 twip SINGLE border
        }
    }
    return result;
}

long BorderWidthImpl::GetLine2( long nWidth ) const
{
    long result = static_cast<long>(m_nRate2);
    if ( ( m_nFlags & CHANGE_LINE2 ) > 0 )
    {
        long const nConstant1 = (m_nFlags & CHANGE_LINE1) ? 0 : m_nRate1;
        long const nConstantD = (m_nFlags & CHANGE_DIST ) ? 0 : m_nRateGap;
        result = std::max<long>(0,
                    static_cast<long>((m_nRate2 * nWidth) + 0.5)
                        - (nConstant1 + nConstantD));
    }
    return result;
}

long BorderWidthImpl::GetGap( long nWidth ) const
{
    long result = static_cast<long>(m_nRateGap);
    if ( ( m_nFlags & CHANGE_DIST ) > 0 )
    {
        long const nConstant1 = (m_nFlags & CHANGE_LINE1) ? 0 : m_nRate1;
        long const nConstant2 = (m_nFlags & CHANGE_LINE2) ? 0 : m_nRate2;
        result = std::max<long>(0,
                    static_cast<long>((m_nRateGap * nWidth) + 0.5)
                        - (nConstant1 + nConstant2));
    }

    // Avoid having too small distances (less than 0.1pt)
    if ( result < MINGAPWIDTH && m_nRate1 > 0 && m_nRate2 > 0 )
        result = MINGAPWIDTH;

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
    if ( bGapChange && nGap > MINGAPWIDTH )
        aToCompare.push_back( nWidthGap );
    else if ( !bGapChange && nWidthGap < 0 )
        bInvalid = true;

    // non-constant line width factors must sum to 1
    assert((((bLine1Change) ? m_nRate1 : 0) +
            ((bLine2Change) ? m_nRate2 : 0) +
            ((bGapChange) ? m_nRateGap : 0)) - 1.0 < 0.00001 );

    double nWidth = 0.0;
    if ( (!bInvalid) && (!aToCompare.empty()) )
    {
        nWidth = *aToCompare.begin();
        std::vector< double >::iterator pIt = aToCompare.begin();
        while ( pIt != aToCompare.end() && !bInvalid )
        {
            bInvalid = ( nWidth != *pIt );
            ++pIt;
        }
        nWidth = (bInvalid) ?  0.0 : nLine1 + nLine2 + nGap;
    }

    return nWidth;
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

    basegfx::B2DPolyPolygon ApplyLineDashing( const basegfx::B2DPolygon& rPolygon, sal_uInt16 nDashing, MapUnit eUnit, double fScale )
    {
        std::vector< double > aPattern = GetDashing( nDashing, eUnit );
        std::vector< double >::iterator i = aPattern.begin();
        while( i != aPattern.end() ) {
            (*i) *= fScale;
            ++i;
        }

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
                            sal_uInt16 nStyle, Bitmap& rBmp )
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
}

// -----------------------------------------------------------------------

void LineListBox::ImplInit()
{
    aTxtSize.Width()  = GetTextWidth( rtl::OUString( " " ) );
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
    sal_uInt16 nCount = pLineList->size();
    while ( nPos == LISTBOX_ENTRY_NOTFOUND && i < nCount )
    {
        ImpLineListData* pData = (*pLineList)[ i ];
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

sal_uInt16 LineListBox::GetEntryStyle( sal_uInt16 nPos ) const
{
    ImpLineListData* pData = (nPos < pLineList->size()) ? (*pLineList)[ nPos ] : NULL;
    return ( pData ) ? pData->GetStyle() : STYLE_NONE;
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
    sal_uInt16 nCount = pLineList->size( );
    while ( n < nCount )
    {
        ImpLineListData* pData = (*pLineList)[ n ];
        if ( pData && pData->GetMinWidth() <= m_nWidth )
        {
            Bitmap      aBmp;
            ImpGetLine( pData->GetLine1ForWidth( m_nWidth ),
                    pData->GetLine2ForWidth( m_nWidth ),
                    pData->GetDistForWidth( m_nWidth ),
                    GetColorLine1( GetEntryCount( ) ),
                    GetColorLine2( GetEntryCount( ) ),
                    GetColorDist( GetEntryCount( ) ),
                    pData->GetStyle(), aBmp );
            ListBox::InsertEntry( rtl::OUString( " " ), aBmp, LISTBOX_APPEND );
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
    ImpLineListData* pData = (*pLineList)[ nStyle ];
    if ( pData )
        rResult = pData->GetColorLine1( GetColor( ) );

    return rResult;
}

Color LineListBox::GetColorLine2( sal_uInt16 nPos )
{
    Color rResult = GetPaintColor( );

    sal_uInt16 nStyle = GetStylePos( nPos, m_nWidth );
    ImpLineListData* pData = (*pLineList)[ nStyle ];
    if ( pData )
        rResult = pData->GetColorLine2( GetColor( ) );

    return rResult;
}

Color LineListBox::GetColorDist( sal_uInt16 nPos )
{
    Color rResult = GetSettings().GetStyleSettings().GetFieldColor();

    sal_uInt16 nStyle = GetStylePos( nPos, m_nWidth );
    ImpLineListData* pData = (*pLineList)[ nStyle ];
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

class ImplFontNameListData
{
public:
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
    InitFontMRUEntriesFile();
}

// -------------------------------------------------------------------

FontNameBox::FontNameBox( Window* pParent, const ResId& rResId ) :
    ComboBox( pParent, rResId )
{
    InitBitmaps();
    mpFontList = NULL;
    mbWYSIWYG = sal_False;
    mbSymbols = sal_False;
    InitFontMRUEntriesFile();
}

// -------------------------------------------------------------------

FontNameBox::~FontNameBox()
{
    SaveMRUEntries (maFontMRUEntriesFile);
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

void FontNameBox::SaveMRUEntries( const String& aFontMRUEntriesFile, xub_Unicode cSep ) const
{
    rtl::OString aEntries(rtl::OUStringToOString(GetMRUEntries(cSep),
        RTL_TEXTENCODING_UTF8));

    if (aEntries.isEmpty() || !aFontMRUEntriesFile.Len())
        return;

    SvFileStream aStream;
    aStream.Open( aFontMRUEntriesFile, STREAM_WRITE | STREAM_TRUNC );
    if( ! (aStream.IsOpen() && aStream.IsWritable()) )
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "FontNameBox::SaveMRUEntries: opening mru entries file %s failed\n", rtl::OUStringToOString(aFontMRUEntriesFile, RTL_TEXTENCODING_UTF8 ).getStr() );
#endif
        return;
    }

    aStream.SetLineDelimiter( LINEEND_LF );
    aStream.WriteLine( aEntries );
    aStream.WriteLine( rtl::OString() );
}

// -------------------------------------------------------------------

void FontNameBox::LoadMRUEntries( const String& aFontMRUEntriesFile, xub_Unicode cSep )
{
    if( ! aFontMRUEntriesFile.Len() )
        return;

    SvFileStream aStream( aFontMRUEntriesFile, STREAM_READ );
    if( ! aStream.IsOpen() )
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "FontNameBox::LoadMRUEntries: opening mru entries file %s failed\n", rtl::OUStringToOString(aFontMRUEntriesFile, RTL_TEXTENCODING_UTF8).getStr() );
#endif
        return;
    }

    rtl::OString aLine;
    aStream.ReadLine( aLine );
    rtl::OUString aEntries = rtl::OStringToOUString(aLine,
        RTL_TEXTENCODING_UTF8);
    SetMRUEntries( aEntries, cSep );
}

// ------------------------------------------------------------------

void FontNameBox::InitFontMRUEntriesFile()
{
    rtl::OUString sUserConfigDir(RTL_CONSTASCII_USTRINGPARAM("${$BRAND_BASE_DIR/program/bootstrap.ini:UserInstallation}"));
    rtl::Bootstrap::expandMacros(sUserConfigDir);

    maFontMRUEntriesFile = sUserConfigDir;
    if( maFontMRUEntriesFile.Len() )
    {
        maFontMRUEntriesFile.AppendAscii( FONTNAMEBOXMRUENTRIESFILE );
    }
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
    XubString rEntries = GetMRUEntries();
    sal_Bool bLoadFromFile = ! rEntries.Len();
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

    if ( bLoadFromFile )
        LoadMRUEntries (maFontMRUEntriesFile);
    else
        SetMRUEntries( rEntries );

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

namespace
{
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

        const bool bSymbolFont = isSymbolFont(rInfo);

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
        if (!canRenderNameOfSelectedFont(*rUDEvt.GetDevice()))
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
        Point aPos( nX, aTopLeft.Y() + nVertAdjust );
        rUDEvt.GetDevice()->DrawText( aPos, sFontName );
        Rectangle aHack(aPos.X(), aTopLeft.Y() + nH/2 - 5, aPos.X() + 40, aTopLeft.Y() + nH/2 + 5);
        long nTextX = aPos.X() + aTextRect.GetWidth() + GAPTOEXTRAPREVIEW;

        if (!bUsingCorrectFont)
            rUDEvt.GetDevice()->SetFont( aFont );

        rtl::OUString sSampleText;

        if (!bSymbolFont)
        {
            const bool bNameBeginsWithLatinText = rInfo.GetName().GetChar(0) <= 'z';

            if (bNameBeginsWithLatinText || !bUsingCorrectFont)
                sSampleText = makeShortRepresentativeTextForSelectedFont(*rUDEvt.GetDevice());
        }

        //If we're not a symbol font, but could neither render our own name and
        //we can't determine what script it would like to render, then try a
        //few well known scripts
        if (sSampleText.isEmpty() && !bUsingCorrectFont)
        {
            static const UScriptCode aScripts[] =
            {
                USCRIPT_ARABIC,
                USCRIPT_HEBREW,

                USCRIPT_BENGALI,
                USCRIPT_GURMUKHI,
                USCRIPT_GUJARATI,
                USCRIPT_ORIYA,
                USCRIPT_TAMIL,
                USCRIPT_TELUGU,
                USCRIPT_KANNADA,
                USCRIPT_MALAYALAM,
                USCRIPT_SINHALA,
                USCRIPT_DEVANAGARI,

                USCRIPT_THAI,
                USCRIPT_LAO,
                USCRIPT_GEORGIAN,
                USCRIPT_TIBETAN,
                USCRIPT_SYRIAC,
                USCRIPT_MYANMAR,
                USCRIPT_ETHIOPIC,
                USCRIPT_KHMER,
                USCRIPT_MONGOLIAN,

                USCRIPT_KOREAN,
                USCRIPT_JAPANESE,
                USCRIPT_HAN,
                USCRIPT_SIMPLIFIED_HAN,
                USCRIPT_TRADITIONAL_HAN,

                USCRIPT_GREEK
            };

            for (size_t i = 0; i < SAL_N_ELEMENTS(aScripts); ++i)
            {
                rtl::OUString sText = makeShortRepresentativeTextForScript(aScripts[i]);
                if (!sText.isEmpty())
                {
                    bool bHasSampleTextGlyphs = (STRING_LEN == rUDEvt.GetDevice()->HasGlyphs(aFont, sText));
                    if (bHasSampleTextGlyphs)
                    {
                        sSampleText = sText;
                        break;
                    }
                }
            }

            static const UScriptCode aMinimalScripts[] =
            {
                USCRIPT_HEBREW, //e.g. biblical hebrew
                USCRIPT_GREEK
            };

            for (size_t i = 0; i < SAL_N_ELEMENTS(aMinimalScripts); ++i)
            {
                rtl::OUString sText = makeShortMinimalTextForScript(aMinimalScripts[i]);
                if (!sText.isEmpty())
                {
                    bool bHasSampleTextGlyphs = (STRING_LEN == rUDEvt.GetDevice()->HasGlyphs(aFont, sText));
                    if (bHasSampleTextGlyphs)
                    {
                        sSampleText = sText;
                        break;
                    }
                }
            }
        }

        //If we're a symbol font, or for some reason the font still couldn't
        //render something representative of what it would like to render then
        //make up some semi-random text that it *can* display
        if (bSymbolFont || (!bUsingCorrectFont && sSampleText.isEmpty()))
            sSampleText = makeShortRepresentativeSymbolTextForSelectedFont(*rUDEvt.GetDevice());

        if (!sSampleText.isEmpty())
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
                if (!sSampleText.isEmpty() && nWidth)
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
        aStr = aChrCls.uppercase(aStr);
        for ( sal_uInt16 i = 0; i < nEntryCount; i++ )
        {
            XubString aEntryText = aChrCls.uppercase(GetEntry(i));

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
        XubString aStr = comphelper::string::stripStart(GetText(), ' ');

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
        XubString aStr = comphelper::string::stripStart(GetText(), ' ');

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
                SetCustomUnitText(rtl::OUString('%'));
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
