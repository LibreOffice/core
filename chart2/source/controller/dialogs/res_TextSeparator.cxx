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


#include "res_TextSeparator.hxx"
#include "ResourceIds.hrc"
#include "Strings.hrc"
#include "ResId.hxx"
#include "macros.hxx"

#include <svtools/controldims.hrc>

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

    m_aEntryMap[ " " ] = 0;
    m_aEntryMap[ ", " ] = 1;
    m_aEntryMap[ "; " ] = 2;
    m_aEntryMap[ "\n" ] = 3;

    m_aLB_Separator.SetAccessibleName(m_aFT_Separator.GetText());
    m_aLB_Separator.SetAccessibleRelationLabeledBy(&m_aFT_Separator);
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

void TextSeparatorResources::SetValue( const OUString& rSeparator )
{
    ::std::map< OUString , sal_uInt16 >::iterator aIter( m_aEntryMap.find(rSeparator) );
    if( aIter == m_aEntryMap.end() )
        m_aLB_Separator.SelectEntryPos( m_nDefaultPos );
    else
        m_aLB_Separator.SelectEntryPos( aIter->second );
}

void TextSeparatorResources::SetDefault()
{
    m_aLB_Separator.SelectEntryPos( m_nDefaultPos );
}

 OUString TextSeparatorResources::GetValue() const
{
    sal_uInt16 nPos = m_aLB_Separator.GetSelectEntryPos();
    ::std::map< OUString , sal_uInt16 >::const_iterator aIter( m_aEntryMap.begin() );
    while( aIter != m_aEntryMap.end() )
    {
        if(aIter->second==nPos )
            return aIter->first;
        ++aIter;
    }
    return OUString( " " );
}

//.............................................................................
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
