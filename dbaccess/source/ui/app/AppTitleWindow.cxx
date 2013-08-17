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

#include "AppTitleWindow.hxx"
#include "moduledbu.hxx"
#include "memory"
#include <vcl/svapp.hxx>
#include <tools/debug.hxx>

namespace dbaui
{

DBG_NAME(OTitleWindow)
OTitleWindow::OTitleWindow(Window* _pParent,sal_uInt16 _nTitleId,WinBits _nBits,sal_Bool _bShift)
: Window(_pParent,_nBits | WB_DIALOGCONTROL)
, m_aSpace1(this)
, m_aSpace2(this)
, m_aTitle(this)
, m_pChild(NULL)
, m_bShift(_bShift)
{
    DBG_CTOR(OTitleWindow,NULL);

    setTitle(_nTitleId);
    SetBorderStyle(WINDOW_BORDER_MONO);
    ImplInitSettings( sal_True, sal_True, sal_True );

    Window* pWindows [] = { &m_aSpace1, &m_aSpace2, &m_aTitle };
    for (size_t i=0; i < sizeof(pWindows)/sizeof(pWindows[0]); ++i)
        pWindows[i]->Show();
}

OTitleWindow::~OTitleWindow()
{
    if ( m_pChild )
    {
        m_pChild->Hide();
        ::std::auto_ptr<Window> aTemp(m_pChild);
        m_pChild = NULL;
    }

    DBG_DTOR(OTitleWindow,NULL);
}

void OTitleWindow::setChildWindow(Window* _pChild)
{
    m_pChild = _pChild;
}

#define SPACE_BORDER    1
void OTitleWindow::Resize()
{
    // parent window dimension
    Size aOutputSize( GetOutputSize() );
    long nOutputWidth   = aOutputSize.Width();
    long nOutputHeight  = aOutputSize.Height();

    Size aTextSize = LogicToPixel( Size( 6, 3 ), MAP_APPFONT );
    sal_Int32 nXOffset = aTextSize.Width();
    sal_Int32 nYOffset = aTextSize.Height();
    sal_Int32 nHeight = GetTextHeight() + 2*nYOffset;

    m_aSpace1.SetPosSizePixel(  Point(SPACE_BORDER, SPACE_BORDER ),
                                Size(nXOffset , nHeight - SPACE_BORDER) );
    m_aSpace2.SetPosSizePixel(  Point(nXOffset + SPACE_BORDER, SPACE_BORDER ),
                                Size(nOutputWidth - nXOffset - 2*SPACE_BORDER, nYOffset) );
    m_aTitle.SetPosSizePixel(   Point(nXOffset + SPACE_BORDER, nYOffset + SPACE_BORDER),
                                Size(nOutputWidth - nXOffset - 2*SPACE_BORDER, nHeight - nYOffset - SPACE_BORDER) );
    if ( m_pChild )
    {
        m_pChild->SetPosSizePixel(  Point(m_bShift ? (nXOffset+SPACE_BORDER) : sal_Int32(SPACE_BORDER), nHeight + nXOffset + SPACE_BORDER),
                                    Size(nOutputWidth - ( m_bShift ? (2*nXOffset - 2*SPACE_BORDER) : sal_Int32(SPACE_BORDER) ), nOutputHeight - nHeight - 2*nXOffset - 2*SPACE_BORDER) );
    }
}

void OTitleWindow::setTitle(sal_uInt16 _nTitleId)
{
    if ( _nTitleId != 0 )
    {
        m_aTitle.SetText(ModuleRes(_nTitleId));
    }
}

void OTitleWindow::GetFocus()
{
    Window::GetFocus();
    if ( m_pChild )
        m_pChild->GrabFocus();
}

long OTitleWindow::GetWidthPixel() const
{
    Size aTextSize = LogicToPixel( Size( 12, 0 ), MAP_APPFONT );
    sal_Int32 nWidth = GetTextWidth(m_aTitle.GetText()) + 2*aTextSize.Width();

    return nWidth;
}

void OTitleWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_FONTS) ||
        (rDCEvt.GetType() == DATACHANGED_DISPLAY) ||
        (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
        ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
        (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
    {
        ImplInitSettings( sal_True, sal_True, sal_True );
        Invalidate();
    }
}

void OTitleWindow::ImplInitSettings( sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground )
{
    AllSettings aAllSettings = GetSettings();
    StyleSettings aStyle = aAllSettings.GetStyleSettings();
    aStyle.SetMonoColor(aStyle.GetActiveBorderColor());//GetMenuBorderColor());
    aAllSettings.SetStyleSettings(aStyle);
    SetSettings(aAllSettings);

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    if( bFont )
    {
        Font aFont;
        aFont = rStyleSettings.GetFieldFont();
        aFont.SetColor( rStyleSettings.GetWindowTextColor() );
        SetPointFont( aFont );
    }

    if( bForeground || bFont )
    {
        SetTextColor( rStyleSettings.GetFieldTextColor() );
        SetTextFillColor();
    }

    if( bBackground )
        SetBackground( rStyleSettings.GetFieldColor() );

    Window* pWindows [] = { &m_aSpace1, &m_aSpace2, &m_aTitle};
    for (size_t i=0; i < sizeof(pWindows)/sizeof(pWindows[0]); ++i)
    {
        Font aFont = pWindows[i]->GetFont();
        aFont.SetWeight(WEIGHT_BOLD);
        pWindows[i]->SetFont(aFont);
        pWindows[i]->SetTextColor( aStyle.GetLightColor() );
        pWindows[i]->SetBackground( Wallpaper( aStyle.GetShadowColor() ) );
    }
}

} // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
