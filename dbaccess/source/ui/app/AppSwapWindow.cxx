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

#include "AppSwapWindow.hxx"
#include <tools/debug.hxx>
#include "dbaccess_helpid.hrc"
#include "dbu_app.hrc"
#include "AppView.hxx"
#include <vcl/svapp.hxx>
#include <vcl/syswin.hxx>
#include <vcl/menu.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/settings.hxx>
#include "AppController.hxx"

using namespace ::dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;

// class OApplicationSwapWindow
OApplicationSwapWindow::OApplicationSwapWindow( vcl::Window* _pParent, OAppBorderWindow& _rBorderWindow )
    :Window(_pParent,WB_DIALOGCONTROL )
    ,m_aIconControl(VclPtr<OApplicationIconControl>::Create(this))
    ,m_eLastType(E_NONE)
    ,m_rBorderWin( _rBorderWindow )
{
    ImplInitSettings();

    m_aIconControl->SetClickHdl(LINK(this, OApplicationSwapWindow, OnContainerSelectHdl));
    m_aIconControl->setControlActionListener( &m_rBorderWin.getView()->getAppController() );
    m_aIconControl->SetHelpId(HID_APP_SWAP_ICONCONTROL);
    m_aIconControl->Show();
}

OApplicationSwapWindow::~OApplicationSwapWindow()
{
    disposeOnce();
}

void OApplicationSwapWindow::dispose()
{
    m_aIconControl.disposeAndClear();
    vcl::Window::dispose();
}

void OApplicationSwapWindow::Resize()
{
    Size aFLSize = LogicToPixel( Size( 8, 0 ), MAP_APPFONT );
    long nX = 0;
    if ( m_aIconControl->GetEntryCount() != 0 )
        nX = m_aIconControl->GetBoundingBox( m_aIconControl->GetEntry(0) ).GetWidth() + aFLSize.Width();

    Size aOutputSize = GetOutputSize();

    m_aIconControl->SetPosSizePixel( Point(static_cast<long>((aOutputSize.Width() - nX)*0.5), 0)  ,Size(nX,aOutputSize.Height()));
    m_aIconControl->ArrangeIcons();
}

void OApplicationSwapWindow::ImplInitSettings()
{
    // FIXME RenderContext
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    vcl::Font aFont;
    aFont = rStyleSettings.GetFieldFont();
    aFont.SetColor( rStyleSettings.GetWindowTextColor() );
    SetPointFont(*this, aFont);

    SetTextColor( rStyleSettings.GetFieldTextColor() );
    SetTextFillColor();

    SetBackground( rStyleSettings.GetFieldColor() );
}

void OApplicationSwapWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );
    if ( (rDCEvt.GetType() == DataChangedEventType::FONTS) ||
        (rDCEvt.GetType() == DataChangedEventType::DISPLAY) ||
        (rDCEvt.GetType() == DataChangedEventType::FONTSUBSTITUTION) ||
        ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
        (rDCEvt.GetFlags() & AllSettingsFlags::STYLE)) )
    {
        ImplInitSettings();
        Invalidate();
    }
}

void OApplicationSwapWindow::clearSelection()
{
    m_aIconControl->SetNoSelection();
    SvxIconChoiceCtrlEntry* pEntry = m_aIconControl->GetSelectedEntry();
    if ( pEntry )
        m_aIconControl->InvalidateEntry(pEntry);
    m_aIconControl->GetClickHdl().Call(m_aIconControl.get());
}

void OApplicationSwapWindow::createIconAutoMnemonics( MnemonicGenerator& _rMnemonics )
{
    m_aIconControl->CreateAutoMnemonics( _rMnemonics );
}

bool OApplicationSwapWindow::interceptKeyInput( const KeyEvent& _rEvent )
{
    const vcl::KeyCode& rKeyCode = _rEvent.GetKeyCode();
    if ( rKeyCode.GetModifier() == KEY_MOD2 )
        return m_aIconControl->DoKeyInput( _rEvent );

    // not handled
    return false;
}

ElementType OApplicationSwapWindow::getElementType() const
{
    SvxIconChoiceCtrlEntry* pEntry = m_aIconControl->GetSelectedEntry();
    return ( pEntry ) ? *static_cast<ElementType*>(pEntry->GetUserData()) : E_NONE;
}

bool OApplicationSwapWindow::onContainerSelected( ElementType _eType )
{
    if ( m_eLastType == _eType )
        return true;

    if ( m_rBorderWin.getView()->getAppController().onContainerSelect( _eType ) )
    {
        if ( _eType != E_NONE )
            m_eLastType = _eType;
        return true;
    }

    PostUserEvent( LINK( this, OApplicationSwapWindow, ChangeToLastSelected ), nullptr, true );
    return false;
}

IMPL_LINK_TYPED(OApplicationSwapWindow, OnContainerSelectHdl, SvtIconChoiceCtrl*, _pControl, void)
{
    SvxIconChoiceCtrlEntry* pEntry = _pControl->GetSelectedEntry();
    ElementType eType = E_NONE;
    if ( pEntry )
    {
        eType = *static_cast<ElementType*>(pEntry->GetUserData());
        onContainerSelected( eType ); // i87582
    }
}

IMPL_LINK_NOARG_TYPED(OApplicationSwapWindow, ChangeToLastSelected, void*, void)
{
    selectContainer(m_eLastType);
}

void OApplicationSwapWindow::selectContainer(ElementType _eType)
{
    sal_uLong nCount = m_aIconControl->GetEntryCount();
    SvxIconChoiceCtrlEntry* pEntry = nullptr;
    for (sal_uLong i=0; i < nCount; ++i)
    {
        pEntry = m_aIconControl->GetEntry(i);
        if ( pEntry && *static_cast<ElementType*>(pEntry->GetUserData()) == _eType )
            break;
        pEntry = nullptr;
    }

    if ( pEntry )
        m_aIconControl->SetCursor(pEntry); // this call also initiates a onContainerSelected call
    else
        onContainerSelected( _eType );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
