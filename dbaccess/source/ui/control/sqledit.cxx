/*************************************************************************
 *
 *  $RCSfile: sqledit.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-28 10:01:54 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef DBAUI_SQLEDIT_HXX
#include "sqledit.hxx"
#endif
#ifndef DBAUI_QUERYVIEW_TEXT_HXX
#include "QueryTextView.hxx"
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

DBG_NAME(OSqlEdit);
OSqlEdit::OSqlEdit( OQueryTextView* pParent,  WinBits nWinStyle ) :
    MultiLineEdit( pParent, nWinStyle )
    ,m_bAccelAction( sal_False )
    ,m_pView(pParent)
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
    if (m_timerUndoActionCreation.IsActive())
        m_timerUndoActionCreation.Stop();
    DBG_DTOR(OSqlEdit,NULL);
}
//------------------------------------------------------------------------------
void OSqlEdit::KeyInput( const KeyEvent& rKEvt )
{
    DBG_CHKTHIS(OSqlEdit,NULL);
    static_cast<OQueryContainerWindow*>(m_pView->GetParent())->getView()->getRealView()->getController()->InvalidateFeature(SID_CUT);
    static_cast<OQueryContainerWindow*>(m_pView->GetParent())->getView()->getRealView()->getController()->InvalidateFeature(SID_COPY);

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
    m_strOrigText = GetText();
    MultiLineEdit::GetFocus();
}

//------------------------------------------------------------------------------
IMPL_LINK(OSqlEdit, OnUndoActionTimer, void*, EMPTYARG)
{
    String aText = GetText();
    if(aText != m_strOrigText)
    {
        SfxUndoManager* pUndoMgr = static_cast<OQueryContainerWindow*>(m_pView->GetParent())->getView()->getRealView()->getController()->getUndoMgr();
        OSqlEditUndoAct* pUndoAct = new OSqlEditUndoAct( this );

        pUndoAct->SetOriginalText( m_strOrigText );
        pUndoMgr->AddUndoAction( pUndoAct );

        static_cast<OQueryContainerWindow*>(m_pView->GetParent())->getView()->getRealView()->getController()->InvalidateFeature(SID_UNDO);
        static_cast<OQueryContainerWindow*>(m_pView->GetParent())->getView()->getRealView()->getController()->InvalidateFeature(SID_REDO);

        m_strOrigText = aText;
    }

    return 0L;
}
//------------------------------------------------------------------------------
IMPL_LINK(OSqlEdit, OnInvalidateTimer, void*, EMPTYARG)
{
    static_cast<OQueryContainerWindow*>(m_pView->GetParent())->getView()->getRealView()->getController()->InvalidateFeature(SID_CUT);
    static_cast<OQueryContainerWindow*>(m_pView->GetParent())->getView()->getRealView()->getController()->InvalidateFeature(SID_COPY);
    m_timerInvalidate.Start();
    return 0L;
}
//------------------------------------------------------------------------------
IMPL_LINK(OSqlEdit, ModifyHdl, void*, EMPTYTAG)
{
    if (m_timerUndoActionCreation.IsActive())
        m_timerUndoActionCreation.Stop();
    m_timerUndoActionCreation.Start();

    if (!static_cast<OQueryContainerWindow*>(m_pView->GetParent())->getView()->getRealView()->getController()->isModified())
        static_cast<OQueryContainerWindow*>(m_pView->GetParent())->getView()->getRealView()->getController()->setModified( sal_True );

    static_cast<OQueryContainerWindow*>(m_pView->GetParent())->getView()->getRealView()->getController()->InvalidateFeature(SID_SBA_QRY_EXECUTE);
    static_cast<OQueryContainerWindow*>(m_pView->GetParent())->getView()->getRealView()->getController()->InvalidateFeature(SID_CUT);
    static_cast<OQueryContainerWindow*>(m_pView->GetParent())->getView()->getRealView()->getController()->InvalidateFeature(SID_COPY);

    m_lnkTextModifyHdl.Call(NULL);
    return 0;
}

//------------------------------------------------------------------------------
void OSqlEdit::OverloadedSetText(const String& rNewText)
{
    DBG_CHKTHIS(OSqlEdit,NULL);
    if (m_timerUndoActionCreation.IsActive())
    {   // die noch anstehenden Undo-Action erzeugen
        m_timerUndoActionCreation.Stop();
        LINK(this, OSqlEdit, OnUndoActionTimer).Call(NULL);
    }

    MultiLineEdit::SetText(rNewText);
    m_strOrigText = rNewText;
    static_cast<OQueryContainerWindow*>(m_pView->GetParent())->getView()->getRealView()->getController()->setModified(sal_True);
}

//==============================================================================
