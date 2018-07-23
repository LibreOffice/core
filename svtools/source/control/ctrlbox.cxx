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

#include <config_folders.h>

#include <i18nutil/unicode.hxx>
#include <tools/stream.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/svapp.hxx>
#include <vcl/field.hxx>
#include <vcl/settings.hxx>
#include <sal/macros.h>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <unotools/charclass.hxx>
#include <unotools/fontoptions.hxx>

#include <svtools/sampletext.hxx>
#include <svtools/svtresid.hxx>
#include <svtools/strings.hrc>
#include <svtools/ctrlbox.hxx>
#include <svtools/ctrltool.hxx>
#include <svtools/borderhelper.hxx>

#include <vcl/i18nhelp.hxx>
#include <vcl/fontcapabilities.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <editeng/borderline.hxx>

#include <com/sun/star/table/BorderLineStyle.hpp>

#include <rtl/bootstrap.hxx>

#include <stdio.h>

#define IMGOUTERTEXTSPACE 5
#define EXTRAFONTSIZE 5
#define GAPTOEXTRAPREVIEW 10
#define MAXPREVIEWWIDTH 120
#define MINGAPWIDTH 2

#define FONTNAMEBOXMRUENTRIESFILE "/user/config/fontnameboxmruentries"


BorderWidthImpl::BorderWidthImpl( BorderWidthImplFlags nFlags, double nRate1, double nRate2, double nRateGap ):
    m_nFlags( nFlags ),
    m_nRate1( nRate1 ),
    m_nRate2( nRate2 ),
    m_nRateGap( nRateGap )
{
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
    if ( m_nFlags & BorderWidthImplFlags::CHANGE_LINE1 )
    {
        long const nConstant2 = (m_nFlags & BorderWidthImplFlags::CHANGE_LINE2) ? 0 : m_nRate2;
        long const nConstantD = (m_nFlags & BorderWidthImplFlags::CHANGE_DIST ) ? 0 : m_nRateGap;
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
    if ( m_nFlags & BorderWidthImplFlags::CHANGE_LINE2)
    {
        long const nConstant1 = (m_nFlags & BorderWidthImplFlags::CHANGE_LINE1) ? 0 : m_nRate1;
        long const nConstantD = (m_nFlags & BorderWidthImplFlags::CHANGE_DIST ) ? 0 : m_nRateGap;
        result = std::max<long>(0,
                    static_cast<long>((m_nRate2 * nWidth) + 0.5)
                        - (nConstant1 + nConstantD));
    }
    return result;
}

long BorderWidthImpl::GetGap( long nWidth ) const
{
    long result = static_cast<long>(m_nRateGap);
    if ( m_nFlags & BorderWidthImplFlags::CHANGE_DIST )
    {
        long const nConstant1 = (m_nFlags & BorderWidthImplFlags::CHANGE_LINE1) ? 0 : m_nRate1;
        long const nConstant2 = (m_nFlags & BorderWidthImplFlags::CHANGE_LINE2) ? 0 : m_nRate2;
        result = std::max<long>(0,
                    static_cast<long>((m_nRateGap * nWidth) + 0.5)
                        - (nConstant1 + nConstant2));
    }

    // Avoid having too small distances (less than 0.1pt)
    if ( result < MINGAPWIDTH && m_nRate1 > 0 && m_nRate2 > 0 )
        result = MINGAPWIDTH;

    return result;
}

static double lcl_getGuessedWidth( long nTested, double nRate, bool bChanging )
{
    double nWidth = -1.0;
    if ( bChanging )
        nWidth = double( nTested ) / nRate;
    else
    {
        if ( rtl::math::approxEqual(double( nTested ), nRate) )
            nWidth = nRate;
    }

    return nWidth;
}

long BorderWidthImpl::GuessWidth( long nLine1, long nLine2, long nGap )
{
    std::vector< double > aToCompare;
    bool bInvalid = false;

    bool bLine1Change = bool( m_nFlags & BorderWidthImplFlags::CHANGE_LINE1 );
    double nWidth1 = lcl_getGuessedWidth( nLine1, m_nRate1, bLine1Change );
    if ( bLine1Change )
        aToCompare.push_back( nWidth1 );
    else if ( !bLine1Change && nWidth1 < 0 )
        bInvalid = true;

    bool bLine2Change = bool( m_nFlags & BorderWidthImplFlags::CHANGE_LINE2 );
    double nWidth2 = lcl_getGuessedWidth( nLine2, m_nRate2, bLine2Change );
    if ( bLine2Change )
        aToCompare.push_back( nWidth2 );
    else if ( !bLine2Change && nWidth2 < 0 )
        bInvalid = true;

    bool bGapChange = bool( m_nFlags & BorderWidthImplFlags::CHANGE_DIST );
    double nWidthGap = lcl_getGuessedWidth( nGap, m_nRateGap, bGapChange );
    if ( bGapChange && nGap >= MINGAPWIDTH )
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
        for (auto const& elem : aToCompare)
        {
            bInvalid = ( nWidth != elem );
            if (bInvalid)
                break;
        }
        nWidth = bInvalid ?  0.0 : nLine1 + nLine2 + nGap;
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
    SvxBorderLineStyle m_nStyle;

public:
    ImpLineListData( BorderWidthImpl aWidthImpl, SvxBorderLineStyle nStyle,
            long nMinWidth,
            Color ( *pColor1Fn ) ( Color ),
            Color ( *pColor2Fn ) ( Color ),
            Color ( *pColorDistFn ) ( Color, Color ) );

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
    long   GetMinWidth( ) { return m_nMinWidth;}
    SvxBorderLineStyle GetStyle( ) { return m_nStyle;}
};

ImpLineListData::ImpLineListData( BorderWidthImpl aWidthImpl,
       SvxBorderLineStyle nStyle, long nMinWidth, Color ( *pColor1Fn )( Color ),
       Color ( *pColor2Fn )( Color ), Color ( *pColorDistFn )( Color, Color ) ) :
    m_aWidthImpl( aWidthImpl ),
    m_pColor1Fn( pColor1Fn ),
    m_pColor2Fn( pColor2Fn ),
    m_pColorDistFn( pColorDistFn ),
    m_nMinWidth( nMinWidth ),
    m_nStyle( nStyle )
{
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

SvxBorderLineStyle LineListBox::GetSelectEntryStyle() const
{
    SvxBorderLineStyle nStyle = SvxBorderLineStyle::SOLID;
    sal_Int32 nPos = GetSelectedEntryPos();
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        if (!m_sNone.isEmpty())
            nPos--;
        nStyle = GetEntryStyle( nPos );
    }

    return nStyle;
}


void lclDrawPolygon( OutputDevice& rDev, const basegfx::B2DPolygon& rPolygon, long nWidth, SvxBorderLineStyle nDashing )
{
    AntialiasingFlags nOldAA = rDev.GetAntialiasing();
    rDev.SetAntialiasing( nOldAA & ~AntialiasingFlags::EnableB2dDraw );

    long nPix = rDev.PixelToLogic(Size(1, 1)).Width();
    basegfx::B2DPolyPolygon aPolygons = svtools::ApplyLineDashing(rPolygon, nDashing, nPix);

    // Handle problems of width 1px in Pixel mode: 0.5px gives a 1px line
    if (rDev.GetMapMode().GetMapUnit() == MapUnit::MapPixel && nWidth == nPix)
        nWidth = 0;

    for ( sal_uInt32 i = 0; i < aPolygons.count( ); i++ )
    {
        basegfx::B2DPolygon aDash = aPolygons.getB2DPolygon( i );
        basegfx::B2DPoint aStart = aDash.getB2DPoint( 0 );
        basegfx::B2DPoint aEnd = aDash.getB2DPoint( aDash.count() - 1 );

        basegfx::B2DVector aVector( aEnd - aStart );
        aVector.normalize( );
        const basegfx::B2DVector aPerpendicular(basegfx::getPerpendicular(aVector));

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

namespace svtools {

/**
 * Dashing array must start with a line width and end with a blank width.
 */
std::vector<double> GetDashing( SvxBorderLineStyle nDashing )
{
    std::vector<double> aPattern;
    switch (nDashing)
    {
        case SvxBorderLineStyle::DOTTED:
            aPattern.push_back( 1.0 ); // line
            aPattern.push_back( 2.0 ); // blank
        break;
        case SvxBorderLineStyle::DASHED:
            aPattern.push_back( 16.0 ); // line
            aPattern.push_back( 5.0 );  // blank
        break;
        case SvxBorderLineStyle::FINE_DASHED:
            aPattern.push_back( 6.0 ); // line
            aPattern.push_back( 2.0 ); // blank
        break;
        case SvxBorderLineStyle::DASH_DOT:
            aPattern.push_back( 16.0 ); // line
            aPattern.push_back( 5.0 );  // blank
            aPattern.push_back( 5.0 );  // line
            aPattern.push_back( 5.0 );  // blank
        break;
        case SvxBorderLineStyle::DASH_DOT_DOT:
            aPattern.push_back( 16.0 ); // line
            aPattern.push_back( 5.0 );  // blank
            aPattern.push_back( 5.0 );  // line
            aPattern.push_back( 5.0 );  // blank
            aPattern.push_back( 5.0 );  // line
            aPattern.push_back( 5.0 );  // blank
        break;
        default:
            ;
    }

    return aPattern;
}

namespace {

class ApplyScale
{
    double mfScale;
public:
    explicit ApplyScale( double fScale ) : mfScale(fScale) {}
    void operator() ( double& rVal )
    {
        rVal *= mfScale;
    }
};

}

std::vector<double> GetLineDashing( SvxBorderLineStyle nDashing, double fScale )
{
    std::vector<double> aPattern = GetDashing(nDashing);
    std::for_each(aPattern.begin(), aPattern.end(), ApplyScale(fScale));
    return aPattern;
}

basegfx::B2DPolyPolygon ApplyLineDashing( const basegfx::B2DPolygon& rPolygon, SvxBorderLineStyle nDashing, double fScale )
{
    std::vector<double> aPattern = GetDashing(nDashing);
    std::for_each(aPattern.begin(), aPattern.end(), ApplyScale(fScale));

    basegfx::B2DPolyPolygon aPolygons;

    if (aPattern.empty())
        aPolygons.append(rPolygon);
    else
        basegfx::utils::applyLineDashing(rPolygon, aPattern, &aPolygons);

    return aPolygons;
}

void DrawLine( OutputDevice& rDev, const Point& rP1, const Point& rP2,
    sal_uInt32 nWidth, SvxBorderLineStyle nDashing )
{
    DrawLine( rDev, basegfx::B2DPoint( rP1.X(), rP1.Y() ),
            basegfx::B2DPoint( rP2.X(), rP2.Y( ) ), nWidth, nDashing );
}

void DrawLine( OutputDevice& rDev, const basegfx::B2DPoint& rP1, const basegfx::B2DPoint& rP2,
    sal_uInt32 nWidth, SvxBorderLineStyle nDashing )
{
    basegfx::B2DPolygon aPolygon;
    aPolygon.append( rP1 );
    aPolygon.append( rP2 );
    lclDrawPolygon( rDev, aPolygon, nWidth, nDashing );
}

}

void LineListBox::ImpGetLine( long nLine1, long nLine2, long nDistance,
                            Color aColor1, Color aColor2, Color aColorDist,
                            SvxBorderLineStyle nStyle, BitmapEx& rBmp )
{
    //TODO, rather than including the " " text to force
    //the line height, better would be do drop
    //this calculation and draw a bitmap of height
    //equal to normal text line and center the
    //line within that
    long nMinWidth = GetTextWidth("----------");
    Size aSize = CalcSubEditSize();
    aSize.setWidth( std::max(nMinWidth, aSize.Width()) );
    aSize.AdjustWidth( -(aTxtSize.Width()) );
    aSize.AdjustWidth( -6 );
    aSize.setHeight( aTxtSize.Height() );

    // SourceUnit to Twips
    if ( eSourceUnit == FUNIT_POINT )
    {
        nLine1      /= 5;
        nLine2      /= 5;
        nDistance   /= 5;
    }

    // Paint the lines
    aSize = aVirDev->PixelToLogic( aSize );
    long nPix = aVirDev->PixelToLogic( Size( 0, 1 ) ).Height();
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
        aSize.setHeight( nVirHeight );
    // negative width should not be drawn
    if ( aSize.Width() <= 0 )
        return;

    Size aVirSize = aVirDev->LogicToPixel( aSize );
    if ( aVirDev->GetOutputSizePixel() != aVirSize )
        aVirDev->SetOutputSizePixel( aVirSize );
    aVirDev->SetFillColor( aColorDist );
    aVirDev->DrawRect( tools::Rectangle( Point(), aSize ) );

    aVirDev->SetFillColor( aColor1 );

    double y1 = double( n1 ) / 2;
    svtools::DrawLine( *aVirDev.get(), basegfx::B2DPoint( 0, y1 ), basegfx::B2DPoint( aSize.Width( ), y1 ), n1, nStyle );

    if ( n2 )
    {
        double y2 =  n1 + nDist + double( n2 ) / 2;
        aVirDev->SetFillColor( aColor2 );
        svtools::DrawLine( *aVirDev.get(), basegfx::B2DPoint( 0, y2 ), basegfx::B2DPoint( aSize.Width(), y2 ), n2, SvxBorderLineStyle::SOLID );
    }
    rBmp = aVirDev->GetBitmapEx( Point(), Size( aSize.Width(), n1+nDist+n2 ) );
}

LineListBox::LineListBox( vcl::Window* pParent, WinBits nWinStyle ) :
    ListBox( pParent, nWinStyle ),
    m_nWidth( 5 ),
    m_sNone( ),
    aVirDev( VclPtr<VirtualDevice>::Create() ),
    aColor( COL_BLACK ),
    maPaintCol( COL_BLACK )
{
    aTxtSize.setWidth( GetTextWidth( " " ) );
    aTxtSize.setHeight( GetTextHeight() );
    eSourceUnit = FUNIT_POINT;

    aVirDev->SetLineColor();
    aVirDev->SetMapMode( MapMode( MapUnit::MapTwip ) );

    UpdatePaintLineColor();
}

extern "C" SAL_DLLPUBLIC_EXPORT void makeLineListBox(VclPtr<vcl::Window> & rRet, VclPtr<vcl::Window> & pParent, VclBuilder::stringmap & rMap)
{
    bool bDropdown = BuilderUtils::extractDropdown(rMap);
    WinBits nWinBits = WB_LEFT|WB_VCENTER|WB_3DLOOK|WB_TABSTOP;
    if (bDropdown)
        nWinBits |= WB_DROPDOWN;
    VclPtrInstance<LineListBox> pListBox(pParent, nWinBits);
    if (bDropdown)
        pListBox->EnableAutoSize(true);
    rRet = pListBox;
}

LineListBox::~LineListBox()
{
    disposeOnce();
}

void LineListBox::dispose()
{
    m_vLineList.clear();
    ListBox::dispose();
}

sal_Int32 LineListBox::GetStylePos( sal_Int32 nListPos, long nWidth )
{
    sal_Int32 nPos = LISTBOX_ENTRY_NOTFOUND;
    if (!m_sNone.isEmpty())
        nListPos--;

    sal_Int32 n = 0;
    size_t i = 0;
    size_t nCount = m_vLineList.size();
    while ( nPos == LISTBOX_ENTRY_NOTFOUND && i < nCount )
    {
        auto& pData = m_vLineList[ i ];
        if ( pData->GetMinWidth() <= nWidth )
        {
            if ( nListPos == n )
                nPos = static_cast<sal_Int32>(i);
            n++;
        }
        i++;
    }

    return nPos;
}

void LineListBox::SelectEntry( SvxBorderLineStyle nStyle, bool bSelect )
{
    sal_Int32 nPos = GetEntryPos( nStyle );
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        ListBox::SelectEntryPos( nPos, bSelect );
}

void LineListBox::InsertEntry(
    const BorderWidthImpl& rWidthImpl, SvxBorderLineStyle nStyle, long nMinWidth,
    ColorFunc pColor1Fn, ColorFunc pColor2Fn, ColorDistFunc pColorDistFn )
{
    m_vLineList.emplace_back(new ImpLineListData(
        rWidthImpl, nStyle, nMinWidth, pColor1Fn, pColor2Fn, pColorDistFn));
}

sal_Int32 LineListBox::GetEntryPos( SvxBorderLineStyle nStyle ) const
{
    if(nStyle == SvxBorderLineStyle::NONE && !m_sNone.isEmpty())
        return 0;
    for ( size_t i = 0, n = m_vLineList.size(); i < n; ++i ) {
        auto& pData = m_vLineList[ i ];
        if ( pData->GetStyle() == nStyle )
        {
            size_t nPos = i;
            if (!m_sNone.isEmpty())
                nPos ++;
            return static_cast<sal_Int32>(nPos);
        }
    }
    return LISTBOX_ENTRY_NOTFOUND;
}

SvxBorderLineStyle LineListBox::GetEntryStyle( sal_Int32 nPos ) const
{
    ImpLineListData* pData = (0 <= nPos && static_cast<size_t>(nPos) < m_vLineList.size()) ? m_vLineList[ nPos ].get() : nullptr;
    return pData ? pData->GetStyle() : SvxBorderLineStyle::NONE;
}

void LineListBox::UpdatePaintLineColor()
{
    const StyleSettings&    rSettings = GetSettings().GetStyleSettings();
    Color                   aNewCol( rSettings.GetWindowColor().IsDark()? rSettings.GetLabelTextColor() : aColor );

    bool bRet = aNewCol != maPaintCol;

    if( bRet )
        maPaintCol = aNewCol;
}

void LineListBox::UpdateEntries( long nOldWidth )
{
    SetUpdateMode( false );

    UpdatePaintLineColor( );

    sal_Int32      nSelEntry = GetSelectedEntryPos();
    sal_Int32       nTypePos = GetStylePos( nSelEntry, nOldWidth );

    // Remove the old entries
    while ( GetEntryCount( ) > 0 )
        ListBox::RemoveEntry( 0 );

    // Add the new entries based on the defined width
    if (!m_sNone.isEmpty())
        ListBox::InsertEntry( m_sNone );

    sal_uInt16 n = 0;
    sal_uInt16 nCount = m_vLineList.size( );
    while ( n < nCount )
    {
        auto& pData = m_vLineList[ n ];
        if ( pData->GetMinWidth() <= m_nWidth )
        {
            BitmapEx aBmp;
            ImpGetLine( pData->GetLine1ForWidth( m_nWidth ),
                    pData->GetLine2ForWidth( m_nWidth ),
                    pData->GetDistForWidth( m_nWidth ),
                    GetColorLine1( GetEntryCount( ) ),
                    GetColorLine2( GetEntryCount( ) ),
                    GetColorDist( GetEntryCount( ) ),
                    pData->GetStyle(), aBmp );
            ListBox::InsertEntry(" ", Image(aBmp));
            if ( n == nTypePos )
                SelectEntryPos( GetEntryCount() - 1 );
        }
        else if ( n == nTypePos )
            SetNoSelection();
        n++;
    }

    SetUpdateMode( true );
    Invalidate();
}

Color LineListBox::GetColorLine1( sal_Int32 nPos )
{
    sal_Int32 nStyle = GetStylePos( nPos, m_nWidth );
    if (nStyle == LISTBOX_ENTRY_NOTFOUND)
        return GetPaintColor( );
    auto& pData = m_vLineList[ nStyle ];
    return pData->GetColorLine1( GetColor( ) );
}

Color LineListBox::GetColorLine2( sal_Int32 nPos )
{
    sal_Int32 nStyle = GetStylePos( nPos, m_nWidth );
    if (nStyle == LISTBOX_ENTRY_NOTFOUND)
        return GetPaintColor( );
    auto& pData = m_vLineList[ nStyle ];
    return pData->GetColorLine2( GetColor( ) );
}

Color LineListBox::GetColorDist( sal_Int32 nPos )
{
    Color rResult = GetSettings().GetStyleSettings().GetFieldColor();

    sal_Int32 nStyle = GetStylePos( nPos, m_nWidth );
    if (nStyle == LISTBOX_ENTRY_NOTFOUND)
        return rResult;
    auto& pData = m_vLineList[ nStyle ];
    return pData->GetColorDist( GetColor( ), rResult );
}

void LineListBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    ListBox::DataChanged( rDCEvt );

    if( ( rDCEvt.GetType() == DataChangedEventType::SETTINGS ) && ( rDCEvt.GetFlags() & AllSettingsFlags::STYLE ) )
        UpdateEntries( m_nWidth );
}

FontNameBox::FontNameBox( vcl::Window* pParent, WinBits nWinStyle ) :
    ComboBox( pParent, nWinStyle )
{
    EnableSelectAll();
    mbWYSIWYG = false;
    InitFontMRUEntriesFile();
}

extern "C" SAL_DLLPUBLIC_EXPORT void makeFontNameBox(VclPtr<vcl::Window> & rRet, VclPtr<vcl::Window> & pParent, VclBuilder::stringmap & rMap)
{
    bool bDropdown = BuilderUtils::extractDropdown(rMap);
    WinBits nWinBits = WB_LEFT|WB_VCENTER|WB_3DLOOK|WB_TABSTOP;
    if (bDropdown)
        nWinBits |= WB_DROPDOWN;
    VclPtrInstance<FontNameBox> pListBox(pParent, nWinBits);
    if (bDropdown)
        pListBox->EnableAutoSize(true);
    rRet = pListBox;
}

FontNameBox::~FontNameBox()
{
    disposeOnce();
}

void FontNameBox::dispose()
{
    if (mpFontList)
    {
        SaveMRUEntries (maFontMRUEntriesFile);
        ImplDestroyFontList();
    }
    ComboBox::dispose();
}

void FontNameBox::SaveMRUEntries( const OUString& aFontMRUEntriesFile ) const
{
    OString aEntries(OUStringToOString(GetMRUEntries(),
        RTL_TEXTENCODING_UTF8));

    if (aEntries.isEmpty() || aFontMRUEntriesFile.isEmpty())
        return;

    SvFileStream aStream;
    aStream.Open( aFontMRUEntriesFile, StreamMode::WRITE | StreamMode::TRUNC );
    if( ! (aStream.IsOpen() && aStream.IsWritable()) )
    {
        SAL_INFO("svtools.control", "FontNameBox::SaveMRUEntries: opening mru entries file " << aFontMRUEntriesFile << " failed");
        return;
    }

    aStream.SetLineDelimiter( LINEEND_LF );
    aStream.WriteLine( aEntries );
    aStream.WriteLine( OString() );
}

void FontNameBox::LoadMRUEntries( const OUString& aFontMRUEntriesFile )
{
    if (aFontMRUEntriesFile.isEmpty())
        return;

    SvtFontOptions aFontOpt;
    if (!aFontOpt.IsFontHistoryEnabled())
        return;

    SvFileStream aStream( aFontMRUEntriesFile, StreamMode::READ );
    if( ! aStream.IsOpen() )
    {
        SAL_INFO("svtools.control", "FontNameBox::LoadMRUEntries: opening mru entries file " << aFontMRUEntriesFile << " failed");
        return;
    }

    OString aLine;
    aStream.ReadLine( aLine );
    OUString aEntries = OStringToOUString(aLine,
        RTL_TEXTENCODING_UTF8);
    SetMRUEntries( aEntries );
}

void FontNameBox::InitFontMRUEntriesFile()
{
    OUString sUserConfigDir("${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE( "bootstrap") "::UserInstallation}");
    rtl::Bootstrap::expandMacros(sUserConfigDir);

    maFontMRUEntriesFile = sUserConfigDir;
    if( !maFontMRUEntriesFile.isEmpty() )
    {
        maFontMRUEntriesFile += FONTNAMEBOXMRUENTRIESFILE;
    }
}

void FontNameBox::ImplDestroyFontList()
{
    mpFontList.reset();
}

void FontNameBox::Fill( const FontList* pList )
{
    // store old text and clear box
    OUString aOldText = GetText();
    OUString rEntries = GetMRUEntries();
    bool bLoadFromFile = rEntries.isEmpty();
    Clear();

    ImplDestroyFontList();
    mpFontList.reset(new ImplFontList);

    // insert fonts
    sal_uInt16 nFontCount = pList->GetFontNameCount();
    for ( sal_uInt16 i = 0; i < nFontCount; i++ )
    {
        const FontMetric& rFontMetric = pList->GetFontName( i );
        sal_uInt32 nIndex = InsertEntry( rFontMetric.GetFamilyName() );
        if ( nIndex != LISTBOX_ERROR )
        {
            if ( nIndex < mpFontList->size() ) {
                ImplFontList::iterator it = mpFontList->begin();
                ::std::advance( it, nIndex );
                mpFontList->insert( it, rFontMetric );
            } else {
                mpFontList->push_back( rFontMetric );
            }
        }
    }

    if ( bLoadFromFile )
        LoadMRUEntries (maFontMRUEntriesFile);
    else
        SetMRUEntries( rEntries );

    ImplCalcUserItemSize();

    // restore text
    if (!aOldText.isEmpty())
        SetText( aOldText );
}

void FontNameBox::EnableWYSIWYG( bool bEnable )
{
    if ( bEnable != mbWYSIWYG )
    {
        mbWYSIWYG = bEnable;
        EnableUserDraw( mbWYSIWYG );
        ImplCalcUserItemSize();
    }
}

void FontNameBox::ImplCalcUserItemSize()
{
    Size aUserItemSz;
    if ( mbWYSIWYG && mpFontList )
    {
        aUserItemSz = Size(MAXPREVIEWWIDTH, GetTextHeight() );
        aUserItemSz.setHeight( aUserItemSz.Height() * 16 );
        aUserItemSz.setHeight( aUserItemSz.Height() / 10 );
    }
    SetUserItemSize( aUserItemSz );
}

namespace
{
    long shrinkFontToFit(OUString const &rSampleText, long nH, vcl::Font &rFont, OutputDevice &rDevice, tools::Rectangle &rTextRect)
    {
        long nWidth = 0;

        Size aSize( rFont.GetFontSize() );

        //Make sure it fits in the available height
        while (aSize.Height() > 0)
        {
            if (!rDevice.GetTextBoundRect(rTextRect, rSampleText))
                break;
            if (rTextRect.GetHeight() <= nH)
            {
                nWidth = rTextRect.GetWidth();
                break;
            }

            aSize.AdjustHeight( -(EXTRAFONTSIZE) );
            rFont.SetFontSize(aSize);
            rDevice.SetFont(rFont);
        }

        return nWidth;
    }
}

void FontNameBox::UserDraw( const UserDrawEvent& rUDEvt )
{
    assert( mpFontList );

    FontMetric& rFontMetric = (*mpFontList)[ rUDEvt.GetItemId() ];
    Point aTopLeft = rUDEvt.GetRect().TopLeft();
    long nX = aTopLeft.X();
    long nH = rUDEvt.GetRect().GetHeight();

    if ( mbWYSIWYG )
    {
        nX += IMGOUTERTEXTSPACE;

        const bool bSymbolFont = isSymbolFont(rFontMetric);
        vcl::RenderContext* pRenderContext = rUDEvt.GetRenderContext();

        Color aTextColor = pRenderContext->GetTextColor();
        vcl::Font aOldFont(pRenderContext->GetFont());
        Size aSize( aOldFont.GetFontSize() );
        aSize.AdjustHeight(EXTRAFONTSIZE );
        vcl::Font aFont( rFontMetric );
        aFont.SetFontSize( aSize );
        pRenderContext->SetFont(aFont);
        pRenderContext->SetTextColor(aTextColor);

        bool bUsingCorrectFont = true;
        tools::Rectangle aTextRect;

        // Preview the font name
        OUString sFontName = rFontMetric.GetFamilyName();

        //If it shouldn't or can't draw its own name because it doesn't have the glyphs
        if (!canRenderNameOfSelectedFont(*pRenderContext))
            bUsingCorrectFont = false;
        else
        {
            //Make sure it fits in the available height, shrinking the font if necessary
            bUsingCorrectFont = shrinkFontToFit(sFontName, nH, aFont, *pRenderContext, aTextRect) != 0;
        }

        if (!bUsingCorrectFont)
        {
            pRenderContext->SetFont(aOldFont);
            pRenderContext->GetTextBoundRect(aTextRect, sFontName);
        }

        long nTextHeight = aTextRect.GetHeight();
        long nDesiredGap = (nH-nTextHeight)/2;
        long nVertAdjust = nDesiredGap - aTextRect.Top();
        Point aPos( nX, aTopLeft.Y() + nVertAdjust );
        pRenderContext->DrawText(aPos, sFontName);
        long nTextX = aPos.X() + aTextRect.GetWidth() + GAPTOEXTRAPREVIEW;

        if (!bUsingCorrectFont)
            pRenderContext->SetFont(aFont);

        OUString sSampleText;

        if (!bSymbolFont)
        {
            const bool bNameBeginsWithLatinText = rFontMetric.GetFamilyName()[0] <= 'z';

            if (bNameBeginsWithLatinText || !bUsingCorrectFont)
                sSampleText = makeShortRepresentativeTextForSelectedFont(*pRenderContext);
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

            for (const UScriptCode& rScript : aScripts)
            {
                OUString sText = makeShortRepresentativeTextForScript(rScript);
                if (!sText.isEmpty())
                {
                    bool bHasSampleTextGlyphs = (-1 == pRenderContext->HasGlyphs(aFont, sText));
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

            for (const UScriptCode& rMinimalScript : aMinimalScripts)
            {
                OUString sText = makeShortMinimalTextForScript(rMinimalScript);
                if (!sText.isEmpty())
                {
                    bool bHasSampleTextGlyphs = (-1 == pRenderContext->HasGlyphs(aFont, sText));
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
            sSampleText = makeShortRepresentativeSymbolTextForSelectedFont(*pRenderContext);

        if (!sSampleText.isEmpty())
        {
            const Size &rItemSize = rUDEvt.GetWindow()->GetOutputSize();

            //leave a little border at the edge
            long nSpace = rItemSize.Width() - nTextX - IMGOUTERTEXTSPACE;
            if (nSpace >= 0)
            {
                //Make sure it fits in the available height, and get how wide that would be
                long nWidth = shrinkFontToFit(sSampleText, nH, aFont, *pRenderContext, aTextRect);
                //Chop letters off until it fits in the available width
                while (nWidth > nSpace || nWidth > MAXPREVIEWWIDTH)
                {
                    sSampleText = sSampleText.copy(0, sSampleText.getLength()-1);
                    nWidth = pRenderContext->GetTextBoundRect(aTextRect, sSampleText) ?
                             aTextRect.GetWidth() : 0;
                }

                //center the text on the line
                if (!sSampleText.isEmpty() && nWidth)
                {
                    nTextHeight = aTextRect.GetHeight();
                    nDesiredGap = (nH-nTextHeight)/2;
                    nVertAdjust = nDesiredGap - aTextRect.Top();
                    aPos = Point(nTextX + nSpace - nWidth, aTopLeft.Y() + nVertAdjust);
                    pRenderContext->DrawText(aPos, sSampleText);
                }
            }
        }

        pRenderContext->SetFont(aOldFont);
        DrawEntry( rUDEvt, false, false);   // draw separator
    }
    else
    {
        DrawEntry( rUDEvt, true, true );
    }
}

FontStyleBox::FontStyleBox(vcl::Window* pParent, WinBits nBits)
    : ComboBox(pParent, nBits)
{
    //Use the standard texts to get an optimal size and stick to that size.
    //That should stop the character dialog dancing around.
    InsertEntry(SvtResId(STR_SVT_STYLE_LIGHT));
    InsertEntry(SvtResId(STR_SVT_STYLE_LIGHT_ITALIC));
    InsertEntry(SvtResId(STR_SVT_STYLE_NORMAL));
    InsertEntry(SvtResId(STR_SVT_STYLE_NORMAL_ITALIC));
    InsertEntry(SvtResId(STR_SVT_STYLE_BOLD));
    InsertEntry(SvtResId(STR_SVT_STYLE_BOLD_ITALIC));
    InsertEntry(SvtResId(STR_SVT_STYLE_BLACK));
    InsertEntry(SvtResId(STR_SVT_STYLE_BLACK_ITALIC));
    aOptimalSize = GetOptimalSize();
    Clear();
}

Size FontStyleBox::GetOptimalSize() const
{
    if (aOptimalSize.Width() || aOptimalSize.Height())
        return aOptimalSize;
    return ComboBox::GetOptimalSize();
}

extern "C" SAL_DLLPUBLIC_EXPORT void makeFontStyleBox(VclPtr<vcl::Window> & rRet, VclPtr<vcl::Window> & pParent, VclBuilder::stringmap & rMap)
{
    bool bDropdown = BuilderUtils::extractDropdown(rMap);
    WinBits nWinBits = WB_LEFT|WB_VCENTER|WB_3DLOOK|WB_TABSTOP;
    if (bDropdown)
        nWinBits |= WB_DROPDOWN;
    VclPtrInstance<FontStyleBox> pListBox(pParent, nWinBits);
    if (bDropdown)
        pListBox->EnableAutoSize(true);
    rRet = pListBox;
}

void FontStyleBox::Select()
{
    // keep text over fill operation
    aLastStyle = GetText();
    ComboBox::Select();
}

void FontStyleBox::LoseFocus()
{
    // keep text over fill operation
    aLastStyle = GetText();
    ComboBox::LoseFocus();
}

void FontStyleBox::Modify()
{
    CharClass   aChrCls( ::comphelper::getProcessComponentContext(),
                        GetSettings().GetLanguageTag() );
    OUString   aStr = GetText();
    sal_Int32      nEntryCount = GetEntryCount();

    if ( GetEntryPos( aStr ) == COMBOBOX_ENTRY_NOTFOUND )
    {
        aStr = aChrCls.uppercase(aStr);
        for ( sal_Int32 i = 0; i < nEntryCount; i++ )
        {
            OUString aEntryText = aChrCls.uppercase(GetEntry(i));

            if ( aStr == aEntryText )
            {
                SetText( GetEntry( i ) );
                break;
            }
        }
    }

    ComboBox::Modify();
}

void FontStyleBox::Fill( const OUString& rName, const FontList* pList )
{
    // note: this method must call ComboBox::SetText(),
    //   else aLastStyle will overwritten
    // store prior selection position and clear box
    OUString aOldText = GetText();
    sal_Int32 nPos = GetEntryPos( aOldText );
    Clear();

    // does a font with this name already exist?
    sal_Handle hFontMetric = pList->GetFirstFontMetric( rName );
    if ( hFontMetric )
    {
        OUString aStyleText;
        FontWeight  eLastWeight = WEIGHT_DONTKNOW;
        FontItalic  eLastItalic = ITALIC_NONE;
        FontWidth   eLastWidth = WIDTH_DONTKNOW;
        bool        bNormal = false;
        bool        bItalic = false;
        bool        bBold = false;
        bool        bBoldItalic = false;
        bool        bInsert = false;
        FontMetric    aFontMetric;
        while ( hFontMetric )
        {
            aFontMetric = FontList::GetFontMetric( hFontMetric );

            FontWeight  eWeight = aFontMetric.GetWeight();
            FontItalic  eItalic = aFontMetric.GetItalic();
            FontWidth   eWidth = aFontMetric.GetWidthType();
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
                        bItalic = true;
                    else
                        bNormal = true;
                }
                else
                {
                    if ( eItalic != ITALIC_NONE )
                        bBoldItalic = true;
                    else
                        bBold = true;
                }

                // For wrong StyleNames we replace this with the correct once
                aStyleText = pList->GetStyleName( aFontMetric );
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
                    const OUString& rAttrStyleText = pList->GetStyleName( eWeight, eItalic );
                    if (rAttrStyleText != aStyleText)
                    {
                        OUString aTempStyleText = pList->GetStyleName( aFontMetric );
                        if (rAttrStyleText == aTempStyleText)
                            aStyleText = rAttrStyleText;
                        bInsert = GetEntryPos( aStyleText ) == LISTBOX_ENTRY_NOTFOUND;
                    }
                }
            }

            if ( !bItalic && (aStyleText == pList->GetItalicStr()) )
                bItalic = true;
            else if ( !bBold && (aStyleText == pList->GetBoldStr()) )
                bBold = true;
            else if ( !bBoldItalic && (aStyleText == pList->GetBoldItalicStr()) )
                bBoldItalic = true;

            hFontMetric = FontList::GetNextFontMetric( hFontMetric );
        }

        if ( bInsert )
            InsertEntry( aStyleText );

        // certain style as copy
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
        if (!aOldText.isEmpty())
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
        // insert standard styles if no font
        InsertEntry( pList->GetNormalStr() );
        InsertEntry( pList->GetItalicStr() );
        InsertEntry( pList->GetBoldStr() );
        InsertEntry( pList->GetBoldItalicStr() );
        if (!aOldText.isEmpty())
        {
            if ( nPos > GetEntryCount() )
                ComboBox::SetText( GetEntry( 0 ) );
            else
                ComboBox::SetText( GetEntry( nPos ) );
        }
    }
}

FontSizeBox::FontSizeBox( vcl::Window* pParent, WinBits nWinSize ) :
    MetricBox( pParent, nWinSize )
{
    ImplInit();
}

extern "C" SAL_DLLPUBLIC_EXPORT void makeFontSizeBox(VclPtr<vcl::Window> & rRet, VclPtr<vcl::Window> & pParent, VclBuilder::stringmap & rMap)
{
    bool bDropdown = BuilderUtils::extractDropdown(rMap);
    WinBits nWinBits = WB_LEFT|WB_VCENTER|WB_3DLOOK|WB_TABSTOP;
    if (bDropdown)
        nWinBits |= WB_DROPDOWN;
    VclPtrInstance<FontSizeBox> pListBox(pParent, nWinBits);
    if (bDropdown)
        pListBox->EnableAutoSize(true);
    rRet = pListBox;
}

void FontSizeBox::ImplInit()
{
    EnableAutocomplete( false );

    bRelativeMode   = false;
    bPtRelative     = false;
    bRelative       = false;
    bStdSize        = false;
    pFontList       = nullptr;

    SetShowTrailingZeros( false );
    SetDecimalDigits( 1 );
    SetMin( 20 );
    SetMax( 9999 );
    SetProminentEntryType( ProminentEntry::MIDDLE );
}

void FontSizeBox::Reformat()
{
    FontSizeNames aFontSizeNames( GetSettings().GetUILanguageTag().getLanguageType() );
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

void FontSizeBox::Modify()
{
    MetricBox::Modify();

    if ( !bRelativeMode )
        return;

    OUString aStr = comphelper::string::stripStart(GetText(), ' ');

    bool bNewMode = bRelative;
    bool bOldPtRelMode = bPtRelative;

    if ( bRelative )
    {
        bPtRelative = false;
        const sal_Unicode* pStr = aStr.getStr();
        while ( *pStr )
        {
            if ( ((*pStr < '0') || (*pStr > '9')) && (*pStr != '%') && !unicode::isSpace(*pStr) )
            {
                if ( ('-' == *pStr || '+' == *pStr) && !bPtRelative )
                    bPtRelative = true;
                else if ( bPtRelative && 'p' == *pStr && 't' == *++pStr )
                    ;
                else
                {
                    bNewMode = false;
                    break;
                }
            }
            pStr++;
        }
    }
    else if (!aStr.isEmpty())
    {
        if ( -1 != aStr.indexOf('%') )
        {
            bNewMode = true;
            bPtRelative = false;
        }

        if ( '-' == aStr[0] || '+' == aStr[0] )
        {
            bNewMode = true;
            bPtRelative = true;
        }
    }

    if ( bNewMode != bRelative || bPtRelative != bOldPtRelMode )
        SetRelative( bNewMode );
}

void FontSizeBox::Fill( const FontMetric* pFontMetric, const FontList* pList )
{
    // remember for relative mode
    pFontList = pList;

    // no font sizes need to be set for relative mode
    if ( bRelative )
        return;

    // query font sizes
    const sal_IntPtr* pTempAry;
    const sal_IntPtr* pAry = nullptr;

    if( pFontMetric )
    {
        aFontMetric = *pFontMetric;
        pAry = pList->GetSizeAry( *pFontMetric );
    }
    else
    {
        pAry = FontList::GetStdSizeAry();
    }

    // first insert font size names (for simplified/traditional chinese)
    FontSizeNames aFontSizeNames( GetSettings().GetUILanguageTag().getLanguageType() );
    if ( pAry == FontList::GetStdSizeAry() )
    {
        // for standard sizes we don't need to bother
        if ( bStdSize && GetEntryCount() && aFontSizeNames.IsEmpty() )
            return;
        bStdSize = true;
    }
    else
        bStdSize = false;

    Selection aSelection = GetSelection();
    OUString aStr = GetText();

    Clear();
    sal_Int32 nPos = 0;

    if ( !aFontSizeNames.IsEmpty() )
    {
        if ( pAry == FontList::GetStdSizeAry() )
        {
            // for scalable fonts all font size names
            sal_uInt32 nCount = aFontSizeNames.Count();
            for( sal_uInt32 i = 0; i < nCount; i++ )
            {
                OUString    aSizeName = aFontSizeNames.GetIndexName( i );
                sal_IntPtr  nSize = aFontSizeNames.GetIndexSize( i );
                ComboBox::InsertEntry( aSizeName, nPos );
                ComboBox::SetEntryData( nPos, reinterpret_cast<void*>(-nSize) ); // mark as special
                nPos++;
            }
        }
        else
        {
            // for fixed size fonts only selectable font size names
            pTempAry = pAry;
            while ( *pTempAry )
            {
                OUString aSizeName = aFontSizeNames.Size2Name( *pTempAry );
                if ( !aSizeName.isEmpty() )
                {
                    ComboBox::InsertEntry( aSizeName, nPos );
                    ComboBox::SetEntryData( nPos, reinterpret_cast<void*>(-(*pTempAry)) ); // mark as special
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
        ComboBox::SetEntryData( nPos, reinterpret_cast<void*>(*pTempAry) );
        nPos++;
        pTempAry++;
    }

    SetText( aStr );
    SetSelection( aSelection );
}

void FontSizeBox::EnableRelativeMode( sal_uInt16 nMin, sal_uInt16 nMax, sal_uInt16 nStep )
{
    bRelativeMode = true;
    nRelMin       = nMin;
    nRelMax       = nMax;
    nRelStep      = nStep;
    SetUnit( FUNIT_POINT );
}

void FontSizeBox::EnablePtRelativeMode( short nMin, short nMax, short nStep )
{
    bRelativeMode = true;
    nPtRelMin     = nMin;
    nPtRelMax     = nMax;
    nPtRelStep    = nStep;
    SetUnit( FUNIT_POINT );
}

void FontSizeBox::SetRelative( bool bNewRelative )
{
    if ( !bRelativeMode )
        return;

    Selection aSelection = GetSelection();
    OUString aStr = comphelper::string::stripStart(GetText(), ' ');

    if ( bNewRelative )
    {
        bRelative = true;
        bStdSize = false;

        if ( bPtRelative )
        {
            Clear(); //clear early because SetDecimalDigits is a slow recalc

            SetDecimalDigits( 1 );
            SetMin( nPtRelMin );
            SetMax( nPtRelMax );
            SetUnit( FUNIT_POINT );

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
            Clear(); //clear early because SetDecimalDigits is a slow recalc

            SetDecimalDigits( 0 );
            SetMin( nRelMin );
            SetMax( nRelMax );
            SetUnit( FUNIT_PERCENT );

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
        if (pFontList)
            Clear(); //clear early because SetDecimalDigits is a slow recalc
        bRelative = bPtRelative = false;
        SetDecimalDigits( 1 );
        SetMin( 20 );
        SetMax( 9999 );
        SetUnit( FUNIT_POINT );
        if ( pFontList )
            Fill( &aFontMetric, pFontList );
    }

    SetText( aStr );
    SetSelection( aSelection );
}

OUString FontSizeBox::CreateFieldText( sal_Int64 nValue ) const
{
    OUString sRet( MetricBox::CreateFieldText( nValue ) );
    if ( bRelativeMode && bPtRelative && (0 <= nValue) && !sRet.isEmpty() )
        sRet = "+" + sRet;
    return sRet;
}

void FontSizeBox::SetValue( sal_Int64 nNewValue, FieldUnit eInUnit )
{
    if ( !bRelative )
    {
        sal_Int64 nTempValue = MetricField::ConvertValue( nNewValue, GetBaseValue(), GetDecimalDigits(), eInUnit, GetUnit() );
        FontSizeNames aFontSizeNames( GetSettings().GetUILanguageTag().getLanguageType() );
        // conversion loses precision; however font sizes should
        // never have a problem with that
        OUString aName = aFontSizeNames.Size2Name( static_cast<long>(nTempValue) );
        if ( !aName.isEmpty() && (GetEntryPos( aName ) != LISTBOX_ENTRY_NOTFOUND) )
        {
            mnLastValue = nTempValue;
            SetText( aName );
            mnFieldValue = mnLastValue;
            SetEmptyFieldValueData( false );
            return;
        }
    }

    MetricBox::SetValue( nNewValue, eInUnit );
}

void FontSizeBox::SetValue( sal_Int64 nNewValue )
{
    SetValue( nNewValue, FUNIT_NONE );
}

sal_Int64 FontSizeBox::GetValueFromStringUnit(const OUString& rStr, FieldUnit eOutUnit) const
{
    if ( !bRelative )
    {
        FontSizeNames aFontSizeNames( GetSettings().GetUILanguageTag().getLanguageType() );
        sal_Int64 nValue = aFontSizeNames.Name2Size( rStr );
        if ( nValue )
            return MetricField::ConvertValue( nValue, GetBaseValue(), GetDecimalDigits(), GetUnit(), eOutUnit );
    }

    return MetricBox::GetValueFromStringUnit( rStr, eOutUnit );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
