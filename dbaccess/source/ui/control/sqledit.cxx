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
#include "precompiled_dbaccess.hxx"
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

//////////////////////////////////////////////////////////////////////////
// OSqlEdit
//------------------------------------------------------------------------------
using namespace dbaui;

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
    // listen for change of Font and Color Settings
    m_SourceViewConfig.AddListener( this );
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
    m_SourceViewConfig.RemoveListener(this);
    m_ColorConfig.RemoveListener(this);
}
//------------------------------------------------------------------------------
void OSqlEdit::KeyInput( const KeyEvent& rKEvt )
{
    DBG_CHKTHIS(OSqlEdit,NULL);
    OJoinController& rController = m_pView->getContainerWindow()->getDesignView()->getController();
    rController.InvalidateFeature(SID_CUT);
    rController.InvalidateFeature(SID_COPY);

    // Ist dies ein Cut, Copy, Paste Event?
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
    // Das Cut, Copy, Paste per Accel. fuehrt neben der Aktion im Edit im View
    // auch die entsprechenden Slots aus. Die  Aktionen finden also zweimal statt.
    // Um dies zu verhindern, kann im View beim SlotExec diese Funktion
    // aufgerufen werden.

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
IMPL_LINK(OSqlEdit, OnUndoActionTimer, void*, EMPTYARG)
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
IMPL_LINK(OSqlEdit, OnInvalidateTimer, void*, EMPTYARG)
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
    {   // die noch anstehenden Undo-Action erzeugen
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
    if ( pOption == &m_SourceViewConfig )
        ImplSetFont();
    else if ( pOption == &m_ColorConfig )
        MultiLineEditSyntaxHighlight::UpdateData();
}

void OSqlEdit::ImplSetFont()
{
    AllSettings aSettings = GetSettings();
    StyleSettings aStyleSettings = aSettings.GetStyleSettings();
    String sFontName = m_SourceViewConfig.GetFontName();
    if ( !sFontName.Len() )
    {
        Font aTmpFont( OutputDevice::GetDefaultFont( DEFAULTFONT_FIXED, Application::GetSettings().GetUILanguage(), 0 , this ) );
        sFontName = aTmpFont.GetName();
    }
    Size aFontSize( 0, m_SourceViewConfig.GetFontHeight() );
    Font aFont( sFontName, aFontSize );
    aStyleSettings.SetFieldFont(aFont);
    aSettings.SetStyleSettings(aStyleSettings);
    SetSettings(aSettings);
}
//==============================================================================
