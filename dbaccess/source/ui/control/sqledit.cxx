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

#include "sal/config.h"

#include <cassert>

#include "com/sun/star/beans/XMultiPropertySet.hpp"
#include "com/sun/star/beans/XPropertiesChangeListener.hpp"
#include "officecfg/Office/Common.hxx"
#include "sqledit.hxx"
#include "QueryTextView.hxx"
#include "querycontainerwindow.hxx"
#include <tools/debug.hxx>
#include "dbaccess_helpid.hrc"
#include "browserids.hxx"
#include "querycontroller.hxx"
#include "undosqledit.hxx"
#include "QueryDesignView.hxx"
#include <svl/smplhint.hxx>
#include <vcl/settings.hxx>
#include <cppuhelper/implbase.hxx>

using namespace dbaui;

class OSqlEdit::ChangesListener:
    public cppu::WeakImplHelper< css::beans::XPropertiesChangeListener >
{
public:
    explicit ChangesListener(OSqlEdit & editor): editor_(editor) {}

private:
    virtual ~ChangesListener() {}

    virtual void SAL_CALL disposing(css::lang::EventObject const &)
        throw (css::uno::RuntimeException, std::exception) override
    {
        osl::MutexGuard g(editor_.m_mutex);
        editor_.m_notifier.clear();
    }

    virtual void SAL_CALL propertiesChange(
        css::uno::Sequence< css::beans::PropertyChangeEvent > const &)
        throw (css::uno::RuntimeException, std::exception) override
    {
        SolarMutexGuard g;
        editor_.ImplSetFont();
    }

    OSqlEdit & editor_;
};

OSqlEdit::OSqlEdit( OQueryTextView* pParent,  WinBits nWinStyle ) :
    MultiLineEditSyntaxHighlight( pParent, nWinStyle )
    ,m_pView(pParent)
    ,m_bAccelAction( false )
    ,m_bStopTimer(false )
{
    SetHelpId( HID_CTL_QRYSQLEDIT );
    SetModifyHdl( LINK(this, OSqlEdit, ModifyHdl) );

    m_timerUndoActionCreation.SetTimeout(1000);
    m_timerUndoActionCreation.SetTimeoutHdl(LINK(this, OSqlEdit, OnUndoActionTimer));

    m_timerInvalidate.SetTimeout(200);
    m_timerInvalidate.SetTimeoutHdl(LINK(this, OSqlEdit, OnInvalidateTimer));
    m_timerInvalidate.Start();

    ImplSetFont();
    // Listen for change of Font and Color Settings:
    // Using "this" in ctor is a little fishy, but should work here at least as
    // long as there are no derivations:
    m_listener = new ChangesListener(*this);
    css::uno::Reference< css::beans::XMultiPropertySet > n(
        officecfg::Office::Common::Font::SourceViewFont::get(),
        css::uno::UNO_QUERY_THROW);
    {
        osl::MutexGuard g(m_mutex);
        m_notifier = n;
    }
    css::uno::Sequence< OUString > s(2);
    s[0] = "FontHeight";
    s[1] = "FontName";
    n->addPropertiesChangeListener(s, m_listener.get());
    m_ColorConfig.AddListener(this);

    //#i97044#
    EnableFocusSelectionHide( false );
}

OSqlEdit::~OSqlEdit()
{
    disposeOnce();
}

void OSqlEdit::dispose()
{
    if (m_timerUndoActionCreation.IsActive())
        m_timerUndoActionCreation.Stop();
    css::uno::Reference< css::beans::XMultiPropertySet > n;
    {
        osl::MutexGuard g(m_mutex);
        n = m_notifier;
    }
    if (n.is()) {
        n->removePropertiesChangeListener(m_listener.get());
    }
    m_ColorConfig.RemoveListener(this);
    m_pView.clear();
    MultiLineEditSyntaxHighlight::dispose();
}

void OSqlEdit::KeyInput( const KeyEvent& rKEvt )
{
    OJoinController& rController = m_pView->getContainerWindow()->getDesignView()->getController();
    rController.InvalidateFeature(SID_CUT);
    rController.InvalidateFeature(SID_COPY);

    // Is this a cut, copy, paste event?
    KeyFuncType aKeyFunc = rKEvt.GetKeyCode().GetFunction();
    if( (aKeyFunc==KeyFuncType::CUT)||(aKeyFunc==KeyFuncType::COPY)||(aKeyFunc==KeyFuncType::PASTE) )
        m_bAccelAction = true;

    MultiLineEditSyntaxHighlight::KeyInput( rKEvt );

    if( m_bAccelAction )
        m_bAccelAction = false;
}


void OSqlEdit::GetFocus()
{
    m_strOrigText  =GetText();
    MultiLineEditSyntaxHighlight::GetFocus();
}

IMPL_LINK_NOARG_TYPED(OSqlEdit, OnUndoActionTimer, Timer *, void)
{
    OUString aText = GetText();
    if(aText != m_strOrigText)
    {
        OJoinController& rController = m_pView->getContainerWindow()->getDesignView()->getController();
        SfxUndoManager& rUndoMgr = rController.GetUndoManager();
        OSqlEditUndoAct* pUndoAct = new OSqlEditUndoAct( this );

        pUndoAct->SetOriginalText( m_strOrigText );
        rUndoMgr.AddUndoAction( pUndoAct );

        rController.InvalidateFeature(SID_UNDO);
        rController.InvalidateFeature(SID_REDO);

        m_strOrigText  =aText;
    }
}

IMPL_LINK_NOARG_TYPED(OSqlEdit, OnInvalidateTimer, Timer *, void)
{
    OJoinController& rController = m_pView->getContainerWindow()->getDesignView()->getController();
    rController.InvalidateFeature(SID_CUT);
    rController.InvalidateFeature(SID_COPY);
    if(!m_bStopTimer)
        m_timerInvalidate.Start();
}

IMPL_LINK_NOARG_TYPED(OSqlEdit, ModifyHdl, Edit&, void)
{
    if (m_timerUndoActionCreation.IsActive())
        m_timerUndoActionCreation.Stop();
    m_timerUndoActionCreation.Start();

    OJoinController& rController = m_pView->getContainerWindow()->getDesignView()->getController();
    if (!rController.isModified())
        rController.setModified( sal_True );

    rController.InvalidateFeature(SID_SBA_QRY_EXECUTE);
    rController.InvalidateFeature(SID_CUT);
    rController.InvalidateFeature(SID_COPY);
}

void OSqlEdit::SetText(const OUString& rNewText)
{
    if (m_timerUndoActionCreation.IsActive())
    {   // create the trailing undo-actions
        m_timerUndoActionCreation.Stop();
        LINK(this, OSqlEdit, OnUndoActionTimer).Call(nullptr);
    }

    MultiLineEditSyntaxHighlight::SetText(rNewText);
    m_strOrigText  =rNewText;
}

void OSqlEdit::stopTimer()
{
    m_bStopTimer = true;
    if (m_timerInvalidate.IsActive())
        m_timerInvalidate.Stop();
}

void OSqlEdit::startTimer()
{
    m_bStopTimer = false;
    if (!m_timerInvalidate.IsActive())
        m_timerInvalidate.Start();
}

void OSqlEdit::ConfigurationChanged( utl::ConfigurationBroadcaster* pOption, sal_uInt32 )
{
    assert( pOption == &m_ColorConfig );
    (void) pOption; // avoid warnings
    MultiLineEditSyntaxHighlight::UpdateData();
}

void OSqlEdit::ImplSetFont()
{
    AllSettings aSettings = GetSettings();
    StyleSettings aStyleSettings = aSettings.GetStyleSettings();
    OUString sFontName(
        officecfg::Office::Common::Font::SourceViewFont::FontName::get().
        get_value_or( OUString() ) );
    if ( sFontName.isEmpty() )
    {
        vcl::Font aTmpFont( OutputDevice::GetDefaultFont( DefaultFontType::FIXED, Application::GetSettings().GetUILanguageTag().getLanguageType(), GetDefaultFontFlags::NONE, this ) );
        sFontName = aTmpFont.GetFamilyName();
    }
    Size aFontSize(
        0, officecfg::Office::Common::Font::SourceViewFont::FontHeight::get() );
    vcl::Font aFont( sFontName, aFontSize );
    aStyleSettings.SetFieldFont(aFont);
    aSettings.SetStyleSettings(aStyleSettings);
    SetSettings(aSettings);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
