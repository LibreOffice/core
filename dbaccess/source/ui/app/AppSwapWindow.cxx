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
#include <helpids.h>
#include "AppView.hxx"
#include <vcl/event.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/settings.hxx>
#include "AppController.hxx"

using namespace ::dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;

OApplicationSwapWindow::OApplicationSwapWindow(vcl::Window* pParent, OAppBorderWindow& rBorderWindow)
    : InterimItemWindow(pParent, "dbaccess/ui/appswapwindow.ui", "AppSwapWindow")
    , m_xIconControl(new OApplicationIconControl(m_xBuilder->weld_scrolled_window("scroll")))
    , m_xIconControlWin(new weld::CustomWeld(*m_xBuilder, "valueset", *m_xIconControl))
    , m_eLastType(E_NONE)
    , m_rBorderWin(rBorderWindow)
{
    m_xContainer->set_stack_background();

    ImplInitSettings();

    m_xIconControl->SetHelpId(HID_APP_SWAP_ICONCONTROL);
    m_xIconControl->Fill();
    m_xIconControl->setItemStateHdl(LINK(this, OApplicationSwapWindow, OnContainerSelectHdl));
    m_xIconControl->setControlActionListener( &m_rBorderWin.getView()->getAppController() );
}

void OApplicationSwapWindow::GetFocus()
{
    if (m_xIconControl)
        m_xIconControl->GrabFocus();
    InterimItemWindow::GetFocus();
}

OApplicationSwapWindow::~OApplicationSwapWindow()
{
    disposeOnce();
}

void OApplicationSwapWindow::dispose()
{
    m_xIconControlWin.reset();
    m_xIconControl.reset();
    InterimItemWindow::dispose();
}

void OApplicationSwapWindow::ImplInitSettings()
{
    // FIXME RenderContext
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    vcl::Font aFont = rStyleSettings.GetFieldFont();
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
    m_xIconControl->deselectItems();
    onContainerSelected(E_NONE);
}

void OApplicationSwapWindow::createIconAutoMnemonics(MnemonicGenerator& rMnemonics)
{
    m_xIconControl->createIconAutoMnemonics(rMnemonics);
}

bool OApplicationSwapWindow::interceptKeyInput( const KeyEvent& _rEvent )
{
    const vcl::KeyCode& rKeyCode = _rEvent.GetKeyCode();
    if ( rKeyCode.GetModifier() == KEY_MOD2 )
        return m_xIconControl->DoKeyShortCut( _rEvent );
    // not handled
    return false;
}

ElementType OApplicationSwapWindow::getElementType() const
{
    return m_xIconControl->GetSelectedItem();
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

IMPL_LINK(OApplicationSwapWindow, OnContainerSelectHdl, const ThumbnailViewItem*, pEntry, void)
{
    if (pEntry->mbSelected)
    {
        ElementType eType = static_cast<ElementType>(pEntry->mnId - 1);
        onContainerSelected( eType ); // i87582
    }
}

IMPL_LINK_NOARG(OApplicationSwapWindow, ChangeToLastSelected, void*, void)
{
    selectContainer(m_eLastType);
}

void OApplicationSwapWindow::selectContainer(ElementType eType)
{
    m_xIconControl->deselectItems();
    m_xIconControl->SelectItem(eType + 1); // will trigger onContainerSelected
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
