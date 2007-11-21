/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sqledit.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-21 15:51:26 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
#ifndef DBAUI_SQLEDIT_HXX
#include "sqledit.hxx"
#endif
#ifndef DBAUI_QUERYVIEW_TEXT_HXX
#include "QueryTextView.hxx"
#endif
#ifndef DBAUI_QUERYCONTAINERWINDOW_HXX
#include "querycontainerwindow.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#include "dbaccess_helpid.hrc"
#ifndef DBACCESS_UI_BROWSER_ID_HXX
#include "browserids.hxx"
#endif
#ifndef DBAUI_QUERYCONTROLLER_HXX
#include "querycontroller.hxx"
#endif
#ifndef DBAUI_UNDOSQLEDIT_HXX
#include "undosqledit.hxx"
#endif
#ifndef DBAUI_QUERYDESIGNVIEW_HXX
#include "QueryDesignView.hxx"
#endif
//////////////////////////////////////////////////////////////////////////
// OSqlEdit
//------------------------------------------------------------------------------
using namespace dbaui;

DBG_NAME(OSqlEdit)
OSqlEdit::OSqlEdit( OQueryTextView* pParent,  WinBits nWinStyle ) :
    MultiLineEdit( pParent, nWinStyle )
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
}

//------------------------------------------------------------------------------
OSqlEdit::~OSqlEdit()
{
    DBG_DTOR(OSqlEdit,NULL);
    if (m_timerUndoActionCreation.IsActive())
        m_timerUndoActionCreation.Stop();
}
//------------------------------------------------------------------------------
void OSqlEdit::KeyInput( const KeyEvent& rKEvt )
{
    DBG_CHKTHIS(OSqlEdit,NULL);
    m_pView->getContainerWindow()->getDesignView()->getController()->InvalidateFeature(SID_CUT);
    m_pView->getContainerWindow()->getDesignView()->getController()->InvalidateFeature(SID_COPY);

    // Ist dies ein Cut, Copy, Paste Event?
    KeyFuncType aKeyFunc = rKEvt.GetKeyCode().GetFunction();
    if( (aKeyFunc==KEYFUNC_CUT)||(aKeyFunc==KEYFUNC_COPY)||(aKeyFunc==KEYFUNC_PASTE) )
        m_bAccelAction = sal_True;

    MultiLineEdit::KeyInput( rKEvt );

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
    MultiLineEdit::GetFocus();
}

//------------------------------------------------------------------------------
IMPL_LINK(OSqlEdit, OnUndoActionTimer, void*, EMPTYARG)
{
    String aText  =GetText();
    if(aText != m_strOrigText)
    {
        SfxUndoManager* pUndoMgr = m_pView->getContainerWindow()->getDesignView()->getController()->getUndoMgr();
        OSqlEditUndoAct* pUndoAct = new OSqlEditUndoAct( this );

        pUndoAct->SetOriginalText( m_strOrigText );
        pUndoMgr->AddUndoAction( pUndoAct );

        m_pView->getContainerWindow()->getDesignView()->getController()->InvalidateFeature(SID_UNDO);
        m_pView->getContainerWindow()->getDesignView()->getController()->InvalidateFeature(SID_REDO);

        m_strOrigText  =aText;
    }

    return 0L;
}
//------------------------------------------------------------------------------
IMPL_LINK(OSqlEdit, OnInvalidateTimer, void*, EMPTYARG)
{
    m_pView->getContainerWindow()->getDesignView()->getController()->InvalidateFeature(SID_CUT);
    m_pView->getContainerWindow()->getDesignView()->getController()->InvalidateFeature(SID_COPY);
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

    if (!m_pView->getContainerWindow()->getDesignView()->getController()->isModified())
        m_pView->getContainerWindow()->getDesignView()->getController()->setModified( sal_True );

    m_pView->getContainerWindow()->getDesignView()->getController()->InvalidateFeature(SID_SBA_QRY_EXECUTE);
    m_pView->getContainerWindow()->getDesignView()->getController()->InvalidateFeature(SID_CUT);
    m_pView->getContainerWindow()->getDesignView()->getController()->InvalidateFeature(SID_COPY);

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

    MultiLineEdit::SetText(rNewText);
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

//==============================================================================
