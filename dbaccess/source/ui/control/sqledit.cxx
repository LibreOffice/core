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

namespace css = ::com::sun::star;

//////////////////////////////////////////////////////////////////////////
// OSqlEdit
//------------------------------------------------------------------------------
using namespace dbaui;

class OSqlEdit::ChangesListener:
    public cppu::WeakImplHelper1< css::beans::XPropertiesChangeListener >
{
public:
    ChangesListener(OSqlEdit & editor): editor_(editor) {}

private:
    virtual ~ChangesListener() {}

    virtual void SAL_CALL disposing(css::lang::EventObject const &)
        throw (css::uno::RuntimeException)
    {
        osl::MutexGuard g(editor_.m_mutex);
        editor_.m_notifier.clear();
    }

    virtual void SAL_CALL propertiesChange(
        css::uno::Sequence< css::beans::PropertyChangeEvent > const &)
        throw (css::uno::RuntimeException)
    {
        SolarMutexGuard g;
        editor_.ImplSetFont();
    }

    OSqlEdit & editor_;
};

DBG_NAME(OSqlEdit)
OSqlEdit::OSqlEdit( OQueryTextView* pParent,  WinBits nWinStyle ) :
    MultiLineEditSyntaxHighlight( pParent, nWinStyle )
    ,m_pView(pParent)
    ,m_bAccelAction( sal_False )
    ,m_bStopTimer(sal_False )
{
    DBG_CTOR(OSqlEdit,NULL);
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
    css::uno::Sequence< rtl::OUString > s(2);
    s[0] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FontHeight"));
    s[1] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FontName"));
    n->addPropertiesChangeListener(s, m_listener.get());
    m_ColorConfig.AddListener(this);

    //#i97044#
    EnableFocusSelectionHide( sal_False );
}

//------------------------------------------------------------------------------
OSqlEdit::~OSqlEdit()
{
    DBG_DTOR(OSqlEdit,NULL);
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
}
//------------------------------------------------------------------------------
void OSqlEdit::KeyInput( const KeyEvent& rKEvt )
{
    DBG_CHKTHIS(OSqlEdit,NULL);
    OJoinController& rController = m_pView->getContainerWindow()->getDesignView()->getController();
    rController.InvalidateFeature(SID_CUT);
    rController.InvalidateFeature(SID_COPY);

    // Is this a cut, copy, paste event?
    KeyFuncType aKeyFunc = rKEvt.GetKeyCode().GetFunction();
    if( (aKeyFunc==KEYFUNC_CUT)||(aKeyFunc==KEYFUNC_COPY)||(aKeyFunc==KEYFUNC_PASTE) )
        m_bAccelAction = sal_True;

    MultiLineEditSyntaxHighlight::KeyInput( rKEvt );

    if( m_bAccelAction )
        m_bAccelAction = sal_False;
}

//------------------------------------------------------------------------------
sal_Bool OSqlEdit::IsInAccelAct()
{
    DBG_CHKTHIS(OSqlEdit,NULL);
    // Cut, Copy, Paste by Accel. runs the action in the Edit but also the
    // corresponding slot in the View. Therefore, the action occurs twice.
    // To prevent this, SlotExec in View can call this function.

    return m_bAccelAction;
}

//------------------------------------------------------------------------------
void OSqlEdit::GetFocus()
{
    DBG_CHKTHIS(OSqlEdit,NULL);
    m_strOrigText  =GetText();
    MultiLineEditSyntaxHighlight::GetFocus();
}

//------------------------------------------------------------------------------
IMPL_LINK_NOARG(OSqlEdit, OnUndoActionTimer)
{
    String aText  =GetText();
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

    return 0L;
}
//------------------------------------------------------------------------------
IMPL_LINK_NOARG(OSqlEdit, OnInvalidateTimer)
{
    OJoinController& rController = m_pView->getContainerWindow()->getDesignView()->getController();
    rController.InvalidateFeature(SID_CUT);
    rController.InvalidateFeature(SID_COPY);
    if(!m_bStopTimer)
        m_timerInvalidate.Start();
    return 0L;
}
//------------------------------------------------------------------------------
IMPL_LINK(OSqlEdit, ModifyHdl, void*, /*EMPTYTAG*/)
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

    m_lnkTextModifyHdl.Call(NULL);
    return 0;
}

//------------------------------------------------------------------------------
void OSqlEdit::SetText(const String& rNewText)
{
    DBG_CHKTHIS(OSqlEdit,NULL);
    if (m_timerUndoActionCreation.IsActive())
    {   // create the trailing undo-actions
        m_timerUndoActionCreation.Stop();
        LINK(this, OSqlEdit, OnUndoActionTimer).Call(NULL);
    }

    MultiLineEditSyntaxHighlight::SetText(rNewText);
    m_strOrigText  =rNewText;
}
// -----------------------------------------------------------------------------
void OSqlEdit::stopTimer()
{
    m_bStopTimer = sal_True;
    if (m_timerInvalidate.IsActive())
        m_timerInvalidate.Stop();
}
// -----------------------------------------------------------------------------
void OSqlEdit::startTimer()
{
    m_bStopTimer = sal_False;
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
    rtl::OUString sFontName(
        officecfg::Office::Common::Font::SourceViewFont::FontName::get().
        get_value_or( rtl::OUString() ) );
    if ( sFontName.isEmpty() )
    {
        Font aTmpFont( OutputDevice::GetDefaultFont( DEFAULTFONT_FIXED, Application::GetSettings().GetUILanguage(), 0 , this ) );
        sFontName = aTmpFont.GetName();
    }
    Size aFontSize(
        0, officecfg::Office::Common::Font::SourceViewFont::FontHeight::get() );
    Font aFont( sFontName, aFontSize );
    aStyleSettings.SetFieldFont(aFont);
    aSettings.SetStyleSettings(aStyleSettings);
    SetSettings(aSettings);
}
//==============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
