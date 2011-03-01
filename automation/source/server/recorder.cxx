/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_automation.hxx"
#include <osl/mutex.hxx>

#include <vcl/window.hxx>
#include <vcl/vclevent.hxx>
#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <vcl/spinfld.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/sound.hxx>
#include <vcl/combobox.hxx>
#include <vcl/floatwin.hxx>
#include <basic/ttstrhlp.hxx>
#include "statemnt.hxx"
#include "retstrm.hxx"
#include "rcontrol.hxx"
#include "recorder.hxx"

#include <comphelper/uieventslogger.hxx>

MacroRecorder* MacroRecorder::pMacroRecorder = NULL;

MacroRecorder::MacroRecorder()
: pLastWin( NULL )
, pEditModify( NULL )
, pActionParent( NULL )
, aKeyUniqueID()
, pKeyWin( NULL )
, bKeyFollowFocus( FALSE )
, m_bRecord( FALSE )
, m_bLog( FALSE )
{
    aHookRefresh.SetTimeout( 500 );
    aHookRefresh.SetTimeoutHdl( LINK( this, MacroRecorder, HookRefreshHdl) );
    aHookRefresh.Start();
    aEventListenerHdl = LINK( this, MacroRecorder, EventListener );
    AddEventHooks();
}

MacroRecorder::~MacroRecorder()
{
    aHookRefresh.Stop();
    RemoveEventHooks();
}

void MacroRecorder::AddEventHooks()
{
    Window *pTopLevel = Application::GetFirstTopLevelWindow();
    while ( pTopLevel )
    {
        Window *pParent = pTopLevel;
        while ( pParent->GetParent() )
            pParent = pParent->GetParent();

        pParent->RemoveChildEventListener( aEventListenerHdl );     // might be instrumented already
        pParent->AddChildEventListener( aEventListenerHdl );

        pTopLevel = Application::GetNextTopLevelWindow( pTopLevel );
    }
}

void MacroRecorder::RemoveEventHooks()
{
    Window *pTopLevel = Application::GetFirstTopLevelWindow();
    while ( pTopLevel )
    {
        pTopLevel->RemoveChildEventListener( aEventListenerHdl );
        pTopLevel = Application::GetNextTopLevelWindow( pTopLevel );
    }
}

IMPL_LINK( MacroRecorder, HookRefreshHdl, void*, EMPTYARG )
{
    AddEventHooks();
    return 0;
}

void MacroRecorder::LogVCL( SmartId aParentID, USHORT nVCLWindowType, SmartId aID, String aMethod, USHORT nParam )
{
    ::comphelper::UiEventsLogger::logVcl( aParentID.GetText(), nVCLWindowType, aID.GetText(), aMethod, nParam );
}

void MacroRecorder::LogVCL( SmartId aParentID, USHORT nVCLWindowType, SmartId aID, String aMethod )
{
    ::comphelper::UiEventsLogger::logVcl( aParentID.GetText(), nVCLWindowType, aID.GetText(), aMethod );
}

Window* MacroRecorder::GetParentWithID( Window* pThis )
{
    Window *pOverlap = pThis->GetWindow( WINDOW_OVERLAP );
    while ( pOverlap != pThis && !pThis->GetSmartUniqueOrHelpId().HasAny() && pThis->GET_REAL_PARENT() )
        pThis = pThis->GET_REAL_PARENT();
    return pThis;
}

SmartId MacroRecorder::GetParentID( Window* pThis )
{
    if ( pThis->GetParent() )
        return pThis->GetParent()->GetSmartUniqueOrHelpId();
    else
        return SmartId();
}

IMPL_LINK( MacroRecorder, EventListener, VclSimpleEvent*, pEvent )
{
    BOOL bSendData = FALSE;

    if ( pEvent->ISA( VclWindowEvent ) )
    {
        VclWindowEvent* pWinEvent = ( VclWindowEvent* ) pEvent;
        Window* pWin = pWinEvent->GetWindow();
        ULONG nEventID = pWinEvent->GetId();
#if OSL_DEBUG_LEVEL > 1
        if ( nEventID >= 1001 && nEventID != VCLEVENT_WINDOW_KEYUP )
            nEventID = pWinEvent->GetId();  // Just something to set a breakpoint
        else
            return 0;
#endif

// check for different action after collecting keys
        // send if there_is_something_to_send
        // and eather event_is_not_of_interest
        //     or ( new_window and new_window_is_interesting )    ( ignore interesting events to uninteresting window )

        if (  aKeyString.Len()
            && (  ( nEventID != VCLEVENT_WINDOW_KEYINPUT
                 && nEventID != VCLEVENT_WINDOW_MOUSEMOVE
                 && nEventID != VCLEVENT_WINDOW_COMMAND
                 && nEventID != VCLEVENT_WINDOW_KEYUP )
                || ( pKeyWin != pWin
                    && ( pWin->GetType() == WINDOW_CONTROL || pWin->GetType() == WINDOW_WINDOW )
                   )
               )
           )
        {
            if ( m_bRecord )
            {
                // we cannot access pKeyWin since it might have dissapeared
                if ( bKeyFollowFocus )
                    StatementList::pRet->GenReturn( RET_MacroRecorder, aKeyUniqueID, (USHORT)M_TypeKeys, aKeyString, bKeyFollowFocus );
                else
                    StatementList::pRet->GenReturn( RET_MacroRecorder, aKeyUniqueID, (USHORT)M_TypeKeys, aKeyString );
#if OSL_DEBUG_LEVEL > 1
                StatementList::pRet->GenReturn( RET_MacroRecorder, aKeyUniqueID, (USHORT)M_TypeKeys, String::CreateFromInt32( nEventID ) );
#endif
                bSendData = TRUE;
            }
            // cleanup
            aKeyString.Erase();
            pKeyWin = NULL;
            bKeyFollowFocus = FALSE;
        }

        switch ( pWin->GetType() )
        {
            case WINDOW_TABPAGE:
                switch( nEventID )
                {
                    case VCLEVENT_WINDOW_ACTIVATE:
                        if ( m_bRecord )
                        {
                            StatementList::pRet->GenReturn( RET_MacroRecorder, SmartId(), (comm_USHORT)(M_SetPage|M_RET_NUM_CONTROL), static_cast<comm_ULONG>(pWin->GetSmartUniqueOrHelpId().GetNum()) ); //GetNum() ULONG != comm_ULONG on 64bit
                            bSendData = TRUE;
                        }
                        if ( m_bLog )
                        {
                            LogVCL( SmartId(), pWin->GetType(), pWin->GetSmartUniqueOrHelpId(), CUniString("SetPage") );
                        }
                        break;
                }
                break;
            case WINDOW_RADIOBUTTON:
            case WINDOW_IMAGERADIOBUTTON:
                switch( nEventID )
                {
                    case VCLEVENT_BUTTON_CLICK:  // VCLEVENT_RADIOBUTTON_TOGGLE
                        {
                            if ( ((RadioButton*)pWin)->IsChecked() )
                            {
                                if ( m_bRecord )
                                {
                                    StatementList::pRet->GenReturn( RET_MacroRecorder, pWin->GetSmartUniqueOrHelpId(), (comm_USHORT)M_Check );
                                    bSendData = TRUE;
                                }
                                if ( m_bLog )
                                {
                                    LogVCL( GetParentID( pWin ), pWin->GetType(), pWin->GetSmartUniqueOrHelpId(), CUniString("Check") );
                                }
                            }
                        }
                        break;
                }
                break;
            case WINDOW_CHECKBOX:
            case WINDOW_TRISTATEBOX:
                switch( nEventID )
                {
                    case VCLEVENT_BUTTON_CLICK: //VCLEVENT_CHECKBOX_TOGGLE:
                        {
                            comm_USHORT nMethod;
                            String aMethod;
                            switch ( ((TriStateBox*)pWin)->GetState() )
                            {
                                case STATE_CHECK: nMethod = M_Check; aMethod = CUniString("Check"); break;
                                case STATE_NOCHECK: nMethod = M_UnCheck; aMethod = CUniString("UnCheck"); break;
                                case STATE_DONTKNOW: nMethod = M_TriState; aMethod = CUniString("TriState"); break;
                                default: nMethod = M_Check;
                                    OSL_FAIL( "Unknown state in TriStateBox::GetState()" );
                            }
                            if ( m_bRecord )
                            {
                                StatementList::pRet->GenReturn( RET_MacroRecorder, pWin->GetSmartUniqueOrHelpId(), nMethod );
                                bSendData = TRUE;
                            }
                            if ( m_bLog )
                            {
                                LogVCL( GetParentID( pWin ), pWin->GetType(), pWin->GetSmartUniqueOrHelpId(), aMethod );
                            }
                        }
                        break;
                }
                break;
            case WINDOW_EDIT:
            case WINDOW_MULTILINEEDIT:
                switch( nEventID )
                {
                    case VCLEVENT_EDIT_MODIFY:
                        pEditModify = pWin;
                        aEditModifyString = ((Edit*)pWin)->GetText();
                        break;
                }
                break;
            case WINDOW_MULTILISTBOX:
                switch( nEventID )
                {
                    case VCLEVENT_LISTBOX_SELECT:
                        Sound::Beep();
                }
            case WINDOW_LISTBOX:
                switch( nEventID )
                {
                    case VCLEVENT_LISTBOX_SELECT:
                        if ( m_bRecord )
                        {
                            StatementList::pRet->GenReturn( RET_MacroRecorder, pWin->GetSmartUniqueOrHelpId(), (comm_USHORT)M_Select, comm_ULONG( ((ListBox*)pWin)->GetSelectEntryPos() +1 ) );
                            bSendData = TRUE;
                        }
                        if ( m_bLog )
                        {
                            LogVCL( GetParentID( pWin ), pWin->GetType(), pWin->GetSmartUniqueOrHelpId(), CUniString("Select"), ((ListBox*)pWin)->GetSelectEntryPos() );
                        }
                        break;
                }
                break;
            case WINDOW_COMBOBOX:
            case WINDOW_PATTERNBOX:
            case WINDOW_NUMERICBOX:
            case WINDOW_METRICBOX:
            case WINDOW_CURRENCYBOX:
            case WINDOW_DATEBOX:
            case WINDOW_TIMEBOX:
                switch( nEventID )
                {
                    case VCLEVENT_EDIT_MODIFY:
                        pEditModify = pWin;
                        aEditModifyString = ((Edit*)pWin)->GetText();
                        break;
                    case VCLEVENT_COMBOBOX_SELECT:
                        {
                            pEditModify = NULL;
                            aEditModifyString.Erase();

                            USHORT nPos = ((ComboBox*)pWin)->GetEntryPos(((ComboBox*)pWin)->GetText());
                            if ( m_bRecord )
                            {
                                if ( nPos == COMBOBOX_ENTRY_NOTFOUND )
                                    Sound::Beep();
                                else
                                {
                                    StatementList::pRet->GenReturn( RET_MacroRecorder, pWin->GetSmartUniqueOrHelpId(), (comm_USHORT)M_Select, (comm_ULONG) nPos+1 );
                                    bSendData = TRUE;
                                }
                            }
                            if ( m_bLog )
                            {
                                LogVCL( GetParentID( pWin ), pWin->GetType(), pWin->GetSmartUniqueOrHelpId(), CUniString("Select"), nPos );
                            }
                        }
                }
                break;
            case WINDOW_PUSHBUTTON:
            case WINDOW_OKBUTTON:
            case WINDOW_CANCELBUTTON:
            case WINDOW_IMAGEBUTTON:
            case WINDOW_MOREBUTTON:
            case WINDOW_HELPBUTTON:
                switch( nEventID )
                {
                    case VCLEVENT_BUTTON_CLICK:
                        Window* pParent = pWin->GetParent();
                        BOOL bDone = FALSE;
                        if ( pParent->IsDialog() && !pWin->GetSmartUniqueOrHelpId().HasAny() )
                        {
                            switch ( pParent->GetType() )
                            {
                                case WINDOW_MESSBOX:
                                case WINDOW_INFOBOX:
                                case WINDOW_WARNINGBOX:
                                case WINDOW_ERRORBOX:
                                case WINDOW_QUERYBOX:
                                case WINDOW_BUTTONDIALOG:
                                    {
                                        comm_USHORT nMethod;
                                        String aMethod;
                                        ButtonDialog* pBD = (ButtonDialog*)pParent;

                                        // we have to find the current Button ID ourselves since it is not generated at this point :-(
                                        USHORT nCurrentButtonId = 0xffff;    // Some wild value to wak up people
                                        USHORT i;
                                        for ( i = 0; i < pBD->GetButtonCount() ; i++ )
                                        {
                                            if ( pBD->GetPushButton( pBD->GetButtonId(i) ) == pWin )
                                            {
                                                nCurrentButtonId = pBD->GetButtonId(i);
                                                break;
                                            }
                                        }

                                        switch ( nCurrentButtonId )
                                        {
                                            case BUTTONID_OK: nMethod = M_OK; aMethod = CUniString("OK"); break;
                                            case BUTTONID_CANCEL: nMethod = M_Cancel; aMethod = CUniString("Cancel"); break;
                                            case BUTTONID_YES: nMethod = M_Yes; aMethod = CUniString("Yes"); break;
                                            case BUTTONID_NO: nMethod = M_No; aMethod = CUniString("No"); break;
                                            case BUTTONID_RETRY: nMethod = M_Repeat; aMethod = CUniString("Repeat"); break;
                                            case BUTTONID_HELP: nMethod = M_Help; aMethod = CUniString("Help"); break;
                                            default: nMethod = M_Click; aMethod = CUniString("Click");
                                        }
                                        if ( m_bRecord )
                                        {
                                            if ( nMethod != M_Click )
                                                StatementList::pRet->GenReturn( RET_MacroRecorder, SmartId( 0 ), nMethod );
                                            else
                                                StatementList::pRet->GenReturn( RET_MacroRecorder, SmartId( 0 ), nMethod, (comm_ULONG)nCurrentButtonId );
                                            bSendData = TRUE;
                                        }
                                        if ( m_bLog )
                                        {
                                            if ( nMethod != M_Click )
                                                LogVCL( SmartId(), pWin->GetType(), pWin->GetSmartUniqueOrHelpId(), aMethod );
                                            else
                                                LogVCL( SmartId(), pWin->GetType(), pWin->GetSmartUniqueOrHelpId(), aMethod, nCurrentButtonId );
                                            bDone = TRUE;
                                        }
                                    }
                                    break;
                                default:
                                    {
                                        comm_USHORT nMethod;
                                        String aMethod;
                                        switch ( pWin->GetType() )
                                        {
                                            case WINDOW_OKBUTTON: nMethod = M_OK; aMethod = CUniString("OK"); break;
                                            case WINDOW_CANCELBUTTON: nMethod = M_Cancel; aMethod = CUniString("Cancel"); break;
                                            case WINDOW_HELPBUTTON: nMethod = M_Help; aMethod = CUniString("Help"); break;
                                            default: nMethod = M_Default;aMethod = CUniString("Unknown Button");
                                                OSL_FAIL( "Unknown Button" );
                                        }
                                        if ( m_bRecord )
                                        {
                                            StatementList::pRet->GenReturn( RET_MacroRecorder, pParent->GetSmartUniqueOrHelpId(), nMethod );
                                            bSendData = TRUE;
                                        }
                                        if ( m_bLog )
                                        {
                                            LogVCL( GetParentID( pWin ), pWin->GetType(), pWin->GetSmartUniqueOrHelpId(), aMethod );
                                            bDone = TRUE;
                                        }
                                    }
                                    break;
                            }
                        }
                        if ( m_bRecord )
                        {
                            if ( !bSendData && pWin->GetSmartUniqueOrHelpId().HasAny() )
                            {
                                StatementList::pRet->GenReturn( RET_MacroRecorder, pWin->GetSmartUniqueOrHelpId(), (comm_USHORT)M_Click );
                                bSendData = TRUE;
                            }
                        }
                        if ( m_bLog )
                        {
                            if ( !bDone )
                                LogVCL( GetParentID( pWin ), pWin->GetType(), pWin->GetSmartUniqueOrHelpId(), CUniString("Click") );
                        }
                }
                break;
            case WINDOW_SPINFIELD:
            case WINDOW_PATTERNFIELD:
            case WINDOW_NUMERICFIELD:
            case WINDOW_METRICFIELD:
            case WINDOW_CURRENCYFIELD:
            case WINDOW_DATEFIELD:
            case WINDOW_TIMEFIELD:
                switch( nEventID )
                {
                    case VCLEVENT_SPINFIELD_UP:
                    case VCLEVENT_SPINFIELD_DOWN:
                    case VCLEVENT_SPINFIELD_FIRST:
                    case VCLEVENT_SPINFIELD_LAST:
                        {
                            pEditModify = NULL;
                            aEditModifyString.Erase();

                            comm_USHORT nMethod;
                            String aMethod;
                            switch ( nEventID )
                            {
                                case VCLEVENT_SPINFIELD_UP: nMethod = M_More; aMethod = CUniString("More"); break;
                                case VCLEVENT_SPINFIELD_DOWN: nMethod = M_Less; aMethod = CUniString("Less"); break;
                                case VCLEVENT_SPINFIELD_FIRST: nMethod = M_ToMin; aMethod = CUniString("ToMin"); break;
                                case VCLEVENT_SPINFIELD_LAST: nMethod = M_ToMax; aMethod = CUniString("ToMax"); break;
                                default: nMethod = M_ToMin; aMethod = CUniString("Unknown");
                                    OSL_FAIL( "Unknown EventID in Spinfield" );
                            }
                            if ( m_bRecord )
                            {
                                StatementList::pRet->GenReturn( RET_MacroRecorder, pWin->GetSmartUniqueOrHelpId(), nMethod );
                                bSendData = TRUE;
                            }
                            if ( m_bLog )
                            {
                                LogVCL( GetParentID( pWin ), pWin->GetType(), pWin->GetSmartUniqueOrHelpId(), aMethod );
                            }
                        }
                        break;
                    case VCLEVENT_EDIT_MODIFY:
                        pEditModify = pWin;
                        aEditModifyString = ((SpinField*)pWin)->GetText();
                        break;
                }
                break;

            case WINDOW_MENUBUTTON:
                    switch( nEventID )
                    {
                    case VCLEVENT_BUTTON_CLICK:
                        if ( m_bRecord )
                        {
                            StatementList::pRet->GenReturn( RET_MacroRecorder, pWin->GetSmartUniqueOrHelpId(), (comm_USHORT)M_Click );
                            bSendData = TRUE;
                        }
                        if ( m_bLog )
                        {
                            LogVCL( GetParentID( pWin ), pWin->GetType(), pWin->GetSmartUniqueOrHelpId(), CUniString("Click") );
                        }
                        break;
                    }
                break;
            case WINDOW_TOOLBOX:
                {
                    ToolBox *pTB = ((ToolBox*)pWin);
                    switch( nEventID )
                    {
                        case VCLEVENT_TOOLBOX_SELECT:
                            {   // a Button has been clicked
                                // so this cannot be a tearoff or OpenMenu anymore
                                pActionParent = NULL;
                                // compare to 1 for floating ToolBoxes
                                if ( m_bRecord )
                                {
                                    if ( !pWin->GetSmartUniqueOrHelpId().HasAny() || pWin->GetSmartUniqueOrHelpId().Matches( 1 ) )
                                        // generate direct Button access
                                        StatementList::pRet->GenReturn( RET_MacroRecorder, SmartId( pTB->GetItemCommand( pTB->GetCurItemId() ) ), (comm_USHORT)(M_Click) );
                                    else
                                        // access via Toolbox
                                        StatementList::pRet->GenReturn( RET_MacroRecorder, pTB->GetSmartUniqueOrHelpId(), (comm_USHORT)(M_Click|M_RET_NUM_CONTROL), static_cast<comm_ULONG>(pTB->GetHelpId( pTB->GetCurItemId() )) ); // GetHelpId() ULONG != comm_ULONG on 64bit
                                    bSendData = TRUE;
                                }
                            }
                            break;
                        case VCLEVENT_TOOLBOX_CLICK:  /// ATTENTION this is called during initialisation of toolbox. whoever 'invented' this
                            pActionParent = pTB;
                            break;
                        case VCLEVENT_TOOLBOX_DEACTIVATE:
                            pActionParent = NULL;
                            break;
                    }
                }
                break;
            case WINDOW_CONTROL:
            case WINDOW_WINDOW:
                switch( nEventID )
                {
                    case VCLEVENT_WINDOW_COMMAND:
                        break;
                    case VCLEVENT_WINDOW_KEYINPUT:
                        {
                            const KeyEvent *pKeyEvent = ((KeyEvent*)pWinEvent->GetData());
                            const KeyCode aKeyCode = pKeyEvent->GetKeyCode();
                            if ( pKeyEvent )
                            {
                                pKeyWin = pWin;
                                Window *pIdWin = GetParentWithID( pWin );
                                if ( pIdWin != pWin )
                                   bKeyFollowFocus = TRUE;
                                aKeyUniqueID = pIdWin->GetSmartUniqueOrHelpId();
                                if ( ( !aKeyCode.IsMod1() && !aKeyCode.IsMod2() ) &&
                                      (( aKeyCode.GetGroup() == KEYGROUP_NUM)   ||
                                       ( aKeyCode.GetGroup() == KEYGROUP_ALPHA) ||
                                       ( aKeyCode.GetCode() == KEY_SPACE) ||
                                       ( aKeyCode.GetCode() == KEY_ADD) ||
                                       ( aKeyCode.GetCode() == KEY_SUBTRACT) ||
                                       ( aKeyCode.GetCode() == KEY_MULTIPLY) ||
                                       ( aKeyCode.GetCode() == KEY_DIVIDE) ||
                                       ( aKeyCode.GetCode() == KEY_POINT) ||
                                       ( aKeyCode.GetCode() == KEY_COMMA) ||
                                       ( aKeyCode.GetCode() == KEY_EQUAL) ||
                                       ( aKeyCode.GetCode() == 0) ) )
                                {
                                    DBG_ASSERT( pKeyEvent->GetCharCode(), "no charcode found" );
                                    aKeyString += pKeyEvent->GetCharCode();
                                }
                                else
                                {   // not a regular key, transfer KeyCode
                                    aKeyString += sal_Unicode(1);   // mask it
                                    // extra for '>' which is coded as <SHIFT GREATER>
                                    if ( pKeyEvent->GetCharCode() == '>' )
                                        aKeyString += sal_Unicode( KEY_GREATER | (aKeyCode.GetAllModifier() & ~KEY_SHIFT) );
                                    else
                                        aKeyString += sal_Unicode( aKeyCode.GetCode() | aKeyCode.GetAllModifier() );
                                }
                            }
                        }
                        break;

                    case VCLEVENT_WINDOW_MOUSEMOVE:
                    case VCLEVENT_WINDOW_MOUSEBUTTONDOWN:
                    case VCLEVENT_WINDOW_MOUSEBUTTONUP:
                        {
                        }
                        break;


                }
                break;
            case WINDOW_DOCKINGWINDOW:
                {
//                  case 1 .. 0xffff:
    DBG_TRACE3( "TT_VCLMessage %u %u  %X",nEventID, pWin->GetType(), pWin );
//                      BOOL bx = ((DockingWindow*)pWin)->IsFloatingMode();
//                      break;
                }
                break;
            case WINDOW_FLOATINGWINDOW:
                {
    DBG_TRACE3( "TT_VCLMessage %u %u  %X",nEventID, pWin->GetType(), pWin );
//                    FloatingWindow *pFW = ((FloatingWindow*)pWin);
                }
                break;
        }

        switch( nEventID )
        {
            case VCLEVENT_CONTROL_LOSEFOCUS:
                if ( pEditModify == pWin )
                {
                    if ( m_bRecord )
                    {
                        StatementList::pRet->GenReturn( RET_MacroRecorder, pWin->GetSmartUniqueOrHelpId(), M_SetText, aEditModifyString );
                        bSendData = TRUE;
                    }
                    if ( m_bLog )
                    {
                        LogVCL( GetParentID( pWin ), pWin->GetType(), pWin->GetSmartUniqueOrHelpId(), CUniString("Modify") );
                    }
                    pEditModify = NULL;
                    aEditModifyString.Erase();  //could be somewhat lengthy
                }
                break;
        }

        pLastWin = pWin;
    }  // if

    if ( bSendData )
        new StatementFlow( NULL, F_EndCommandBlock );   // Kommando zum Senden erzeugen und in que eintragen

    return 0;
}



static ::osl::Mutex * getRecorderMutex()
{
    static ::osl::Mutex * pMutex  = NULL;
    if(pMutex==NULL)
    {
        ::osl::MutexGuard aGuard(::osl::Mutex::getGlobalMutex());
        if(pMutex==NULL)
            pMutex = new ::osl::Mutex();
    }
    return pMutex;
}


void MacroRecorder::CheckDelete()
{
    ::osl::MutexGuard aGuard( getRecorderMutex() );
    if ( !m_bRecord && !m_bLog )
    {
        pMacroRecorder = NULL;
        delete this;
    }
}


MacroRecorder* MacroRecorder::GetMacroRecorder()
{
    ::osl::MutexGuard aGuard( getRecorderMutex() );
    if ( !pMacroRecorder )
        pMacroRecorder = new MacroRecorder;

    return pMacroRecorder;
}

BOOL MacroRecorder::HasMacroRecorder()
{
    ::osl::MutexGuard aGuard( getRecorderMutex() );
    return pMacroRecorder != NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
