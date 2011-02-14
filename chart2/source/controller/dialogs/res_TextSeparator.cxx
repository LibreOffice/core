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
#include "precompiled_chart2.hxx"

#include "res_TextSeparator.hxx"
#include "ResourceIds.hrc"
#include "Strings.hrc"
#include "ResId.hxx"
#include "macros.hxx"

#ifndef _SVT_CONTROLDIMS_HRC_
#include <svtools/controldims.hrc>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

TextSeparatorResources::TextSeparatorResources( Window* pWindow )
    : m_aFT_Separator( pWindow, pWindow->GetStyle() )
    , m_aLB_Separator( pWindow , SchResId( LB_TEXT_SEPARATOR ) )
    , m_aEntryMap()
    , m_nDefaultPos(0)
{
    m_aFT_Separator.SetText( String( SchResId( STR_TEXT_SEPARATOR )) );
    m_aFT_Separator.SetSizePixel( m_aFT_Separator.CalcMinimumSize() );

    m_aLB_Separator.SetDropDownLineCount(m_aLB_Separator.GetEntryCount());
    m_aLB_Separator.SetSizePixel( m_aLB_Separator.CalcMinimumSize() );

    m_aEntryMap[ C2U( " " ) ] = 0;
    m_aEntryMap[ C2U( ", " ) ] = 1;
    m_aEntryMap[ C2U( "; " ) ] = 2;
    m_aEntryMap[ C2U( "\n" ) ] = 3;
}
TextSeparatorResources::~TextSeparatorResources()
{
}
void TextSeparatorResources::Show( bool bShow )
{
    m_aFT_Separator.Show( bShow );
    m_aLB_Separator.Show( bShow );
}
void TextSeparatorResources::Enable( bool bEnable )
{
    m_aFT_Separator.Enable( bEnable );
    m_aLB_Separator.Enable( bEnable );
}
void TextSeparatorResources::PositionBelowControl( const Window& rWindow )
{
    Point aPoint( rWindow.GetPosPixel() );
    Size aSize( rWindow.GetSizePixel() );
    aPoint.Y() += aSize.Height();
    Size aBigDistanceSize( rWindow.LogicToPixel( Size(0,RSC_SP_CTRL_Y), MapMode(MAP_APPFONT) ) );
    aPoint.Y() += aBigDistanceSize.Height();

    Size aDistanceSize( rWindow.LogicToPixel( Size(RSC_SP_CTRL_DESC_X, (RSC_CD_DROPDOWN_HEIGHT-RSC_CD_FIXEDTEXT_HEIGHT)/2), MapMode(MAP_APPFONT) ) );
    aPoint.Y() += aDistanceSize.Height();

    m_aFT_Separator.SetPosPixel( aPoint );
    m_aLB_Separator.SetPosPixel( Point( aPoint.X()+m_aFT_Separator.GetSizePixel().Width()+aDistanceSize.Width(), aPoint.Y()-aDistanceSize.Height()-1) );
}

void TextSeparatorResources::AlignListBoxWidthAndXPos( long nWantedLeftBorder /*use -1 to indicate that this can be automatic*/
                                                     , long nWantedRightBorder /*use -1 to indicate that this can be automatic*/
                                                     , long nMinimumListBoxWidth /*use -1 to indicate that this can be automatic*/ )
{
    long nMinPossibleLeftBorder = m_aFT_Separator.GetPosPixel().X() + m_aFT_Separator.GetSizePixel().Width() + 1 ;
    if( nWantedLeftBorder >= 0 && nWantedLeftBorder>nMinPossibleLeftBorder )
    {
        Point aPos( m_aLB_Separator.GetPosPixel() );
        aPos.X() = nWantedLeftBorder;
        m_aLB_Separator.SetPosPixel( aPos );
    }

    long nMinPossibleRightBorder = m_aLB_Separator.GetPosPixel().X() + m_aLB_Separator.CalcMinimumSize().Width() - 1 ;
    if( nWantedRightBorder < m_aLB_Separator.GetPosPixel().X() + nMinimumListBoxWidth )
        nWantedRightBorder = m_aLB_Separator.GetPosPixel().X() + nMinimumListBoxWidth;

    if( nWantedRightBorder >= 0 && nWantedRightBorder > nMinPossibleRightBorder )
    {
        Size aSize( m_aLB_Separator.GetSizePixel() );
        aSize.Width() = nWantedRightBorder-m_aLB_Separator.GetPosPixel().X();
        m_aLB_Separator.SetSizePixel(aSize);
    }
}

Point TextSeparatorResources::GetCurrentListBoxPosition() const
{
    return m_aLB_Separator.GetPosPixel();
}

Size TextSeparatorResources::GetCurrentListBoxSize() const
{
    return m_aLB_Separator.GetSizePixel();
}

void TextSeparatorResources::SetValue( const rtl::OUString& rSeparator )
{
    ::std::map< ::rtl::OUString, sal_uInt16 >::iterator aIter( m_aEntryMap.find(rSeparator) );
    if( aIter == m_aEntryMap.end() )
        m_aLB_Separator.SelectEntryPos( m_nDefaultPos );
    else
        m_aLB_Separator.SelectEntryPos( aIter->second );
}

void TextSeparatorResources::SetDefault()
{
    m_aLB_Separator.SelectEntryPos( m_nDefaultPos );
}

rtl::OUString TextSeparatorResources::GetValue() const
{
    sal_uInt16 nPos = m_aLB_Separator.GetSelectEntryPos();
    ::std::map< ::rtl::OUString, sal_uInt16 >::const_iterator aIter( m_aEntryMap.begin() );
    while( aIter != m_aEntryMap.end() )
    {
        if(aIter->second==nPos )
            return aIter->first;
        ++aIter;
    }
    return C2U( " " );
}

//.............................................................................
} //namespace chart
//.............................................................................

