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
#include <tools/time.hxx>
#include <vcl/splitwin.hxx>
#include <vcl/wrkwin.hxx>
#include <basic/ttstrhlp.hxx>
#include "statemnt.hxx"

#include "retstrm.hxx"
#include "rcontrol.hxx"

#if OSL_DEBUG_LEVEL > 1
#include "editwin.hxx"
#endif

#include "profiler.hxx"
#include <vcl/floatwin.hxx>
#include <vcl/toolbox.hxx>

// only needed for dynamic_cast in wintree
#include <svtools/editbrowsebox.hxx>
#include <svtools/valueset.hxx>
#include <svtools/roadmap.hxx>
#include <svtools/extensionlistbox.hxx>
#include <svtools/table/tablecontrol.hxx>

#define WINDOW_ANYTYPE WINDOW_BASE


TTProfiler *StatementList::pProfiler = NULL;
StatementList *StatementList::pFirst = NULL;
BOOL StatementList::bReadingCommands = FALSE;
BOOL StatementList::bIsInReschedule = FALSE;
USHORT StatementList::nModalCount = 0;
Window *StatementList::pLastFocusWindow = NULL;
BOOL StatementList::bWasDragManager = FALSE;
BOOL StatementList::bWasPopupMenu = FALSE;
BOOL StatementList::bBasicWasRunning = FALSE;
RetStream *StatementList::pRet = NULL;
BOOL StatementList::IsError = FALSE;
BOOL StatementList::bDying = FALSE;
BOOL StatementList::bExecuting = FALSE;
StatementList *StatementList::pCurrentProfileStatement = NULL;
BOOL StatementList::bUsePostEvents = TRUE;
#if OSL_DEBUG_LEVEL > 1
EditWindow *StatementList::m_pDbgWin;
#endif


SmartId StatementList::aWindowWaitUId = SmartId();
Window *StatementList::pWindowWaitPointer = NULL;
SmartId StatementList::aWindowWaitOldHelpId = SmartId();
SmartId StatementList::aWindowWaitOldUniqueId = SmartId();
USHORT StatementList::nUseBindings = 0;

SmartId StatementList::aSubMenuId1 = SmartId(); // Untermenüs bei PopupMenus
SmartId StatementList::aSubMenuId2 = SmartId(); // erstmal 2-Stufig
SmartId StatementList::aSubMenuId3 = SmartId(); // and now even 3 levels #i31512#
SystemWindow *StatementList::pMenuWindow = NULL;
TTProperties *StatementList::pTTProperties = NULL;

USHORT StatementList::nMinTypeKeysDelay = 0;    // Verzögerung der einzelnen Anschläge für TypeKeys
USHORT StatementList::nMaxTypeKeysDelay = 0;
BOOL StatementList::bDoTypeKeysDelay = FALSE;

Window* StatementList::pFirstDocFrame = NULL;

BOOL StatementList::bIsSlotInExecute = FALSE;

BOOL StatementList::bCatchGPF = TRUE;


IMPL_GEN_RES_STR;


static TTSettings* pTTSettings = NULL;

TTSettings* GetTTSettings()
{
    if ( !pTTSettings )
    {
        pTTSettings = new TTSettings;

        // DisplayHID
        pTTSettings->pDisplayInstance = NULL;
        pTTSettings->pDisplayHidWin = NULL;
        pTTSettings->Old = NULL;
        pTTSettings->Act = NULL;
        pTTSettings->aOriginalCaption.Erase();

        // Translate
        pTTSettings->pTranslateWin = NULL;
        pTTSettings->bToTop = TRUE;
    }

    return pTTSettings;
}




#define IS_WINP_CLOSING(pWin) (pWin->GetSmartHelpId().Matches( 4321 ) && pWin->GetSmartUniqueId().Matches( 1234 ))

StatementList::StatementList()
: nRetryCount(MAX_RETRIES)
, bStatementInQue(FALSE)
{
    if (!pRet)
        pRet = new RetStream;       // so Spät wie möglich, aber dennoch Zentral und auf jeden Fall rechtzeitig, da pRet private ist.
}

void StatementList::InitProfile()
{
    if ( pProfiler )
    {
        if ( pProfiler->IsProfilingPerCommand() || pProfiler->IsPartitioning() )
            pProfiler->StartProfileInterval( pCurrentProfileStatement != this );

#if OSL_DEBUG_LEVEL > 1
        if ( pCurrentProfileStatement != NULL && pCurrentProfileStatement != this )
            pRet->GenReturn( RET_ProfileInfo, SmartId(), CUniString("InitProfile von anderem Statement gerufen ohne SendProfile\n") );
#endif
        pCurrentProfileStatement = this;
    }
}

void StatementList::SendProfile( String aText )
{
    if ( pProfiler )
    {
        if ( pCurrentProfileStatement == this )
        {
            if ( pProfiler->IsProfilingPerCommand() || pProfiler->IsPartitioning() )
                pProfiler->EndProfileInterval();

            if ( pProfiler->IsProfilingPerCommand() )
                pRet->GenReturn( RET_ProfileInfo, SmartId(), pProfiler->GetProfileLine( aText ) );

            if ( pProfiler->IsPartitioning() )
                pRet->GenReturn( RET_ProfileInfo, SmartId( S_ProfileTime ), static_cast<comm_ULONG>(pProfiler->GetPartitioningTime()) ); // GetPartitioningTime() ULONG != comm_ULONG on 64bit
        }

        if ( pProfiler->IsAutoProfiling() )
            pRet->GenReturn( RET_ProfileInfo, SmartId(), pProfiler->GetAutoProfiling() );

#if OSL_DEBUG_LEVEL > 1
        if ( pCurrentProfileStatement == NULL )
            pRet->GenReturn( RET_ProfileInfo, SmartId(), CUniString("SendProfile ohne InitProfile\n") );
#endif
        pCurrentProfileStatement = NULL;
    }
}

void StatementList::QueStatement(StatementList *pAfterThis)
{
    DBG_ASSERT(!bStatementInQue,"QueStatement für bereits eingetragenes Statement -> Abgebrochen");
    if ( bStatementInQue )
        return;

    bStatementInQue = TRUE;
    if ( pAfterThis )
    {
        if ( pAfterThis->bStatementInQue )
        {
            pNext = pAfterThis->pNext;
            pAfterThis->pNext = this;
        }
        else
        {   // pAfterThis not in que -> already dequed -> add to front of list
            pNext = pFirst;
            pFirst = this;
        }
    }
    else    // am Ende einfügen
    {
        pNext = NULL;
        if( !pFirst )
            pFirst = this;
        else
        {
            StatementList *pList;
            pList = pFirst;
            while( pList->pNext )
                pList = pList->pNext;
            pList->pNext = this;
        }
    }
}

void StatementList::Advance()
{   // pFirst ist static!
    pFirst = pNext;
    bStatementInQue = FALSE;
    pNext = NULL;
}


StatementList::~StatementList()
{
#if OSL_DEBUG_LEVEL > 1
    m_pDbgWin->AddText( "Deleting \n" );
#endif
    DBG_ASSERT(!bReadingCommands,"Deleting commands while reading them!");
}

Window* StatementList::GetDocWin( USHORT nNr )
{
    Window* pBase = Application::GetFirstTopLevelWindow();

    while ( pBase )
    {
        if ( IsDocWin( pBase ) )
        {
            if ( !nNr )
                return pBase;
            nNr--;
        }
        pBase = Application::GetNextTopLevelWindow( pBase );
    }
    return NULL;
}

USHORT StatementList::GetDocFrameCount()
{
    Window* pBase = Application::GetFirstTopLevelWindow();
    USHORT nCount = 0;

    while ( pBase )
    {
        if ( IsDocFrame( pBase ) )
            nCount++;
        pBase = Application::GetNextTopLevelWindow( pBase );
    }
    return nCount;
}

USHORT StatementList::GetDocWinCount()
{
    Window* pBase = Application::GetFirstTopLevelWindow();
    USHORT nCount = 0;

    while ( pBase )
    {
        if ( IsDocWin( pBase ) )
            nCount++;
        pBase = Application::GetNextTopLevelWindow( pBase );
    }
    return nCount;
}

Window* StatementList::SearchAllWin( Window *pBase, Search &aSearch, BOOL MaybeBase )
{

    if ( !pBase && !aSearch.HasSearchFlag( SEARCH_NO_TOPLEVEL_WIN ) )
    {
        BOOL bSearchFocusFirst = aSearch.HasSearchFlag( SEARCH_FOCUS_FIRST );

        Window *pControl = NULL;
        if ( bSearchFocusFirst )
        {
            // first test Parent of Focus Window
            pBase = Application::GetFocusWindow();
            if ( pBase )
            {
                DBG_ASSERT( WinPtrValid( pBase ), "GetFocusWindow is no valid WindowPointer" );
                Window *pPParent = pBase;
                while ( pPParent->GET_REAL_PARENT() )
                    pPParent = pPParent->GET_REAL_PARENT();

                    // get overlap window. Will be dialog else document itself
                    pBase = pBase->GetWindow( WINDOW_OVERLAP );

                    // set flag to find disabled elements.
                    // This is better than an enabled one on another Window
                    aSearch.AddSearchFlags( SEARCH_FIND_DISABLED );

                    // search on current Dialog first
                    pControl = SearchAllWin( pBase, aSearch );

                    // search on current Document
                    if ( !pControl && pBase != pPParent )
                        pControl = SearchAllWin( pPParent, aSearch );

                    aSearch.RemoveSearchFlags( SEARCH_FIND_DISABLED );

                    if ( pControl )
                        return pControl;
            }
        }

        pBase = Application::GetFirstTopLevelWindow();

        while ( pBase )
        {
            pControl = SearchAllWin( pBase, aSearch );
            if ( pControl )
                return pControl;

            pBase = Application::GetNextTopLevelWindow( pBase );
        }
        return NULL;
    }


    Window *pResult = NULL;
    pResult = SearchClientWin( pBase, aSearch, MaybeBase );
    if ( pResult )
        return pResult;

    if ( !aSearch.HasSearchFlag( SEARCH_NOOVERLAP ) )
    {
        if ( pBase->GetWindow( WINDOW_FIRSTOVERLAP ) )
            pResult = SearchAllWin( pBase->GetWindow( WINDOW_FIRSTOVERLAP ), aSearch );

        if ( !pResult && pBase->GetWindow( WINDOW_NEXT ) )
            pResult = SearchAllWin( pBase->GetWindow( WINDOW_NEXT ), aSearch );
    }

    return pResult;
}


Window* StatementList::SearchClientWin( Window *pBase, Search &aSearch, BOOL MaybeBase )
{
    if ( !pBase )
        return NULL;

    if ( MaybeBase && aSearch.IsWinOK( pBase ) )
        return pBase;

    Window *pResult = NULL;

    USHORT i;
    for( i = 0 ; i < pBase->GetChildCount() && !pResult; i++ )
        pResult = SearchClientWin( pBase->GetChild(i), aSearch );

    return pResult;
}


BOOL SearchUID::IsWinOK( Window *pWin )
{
    if ( aUId.Matches( pWin->GetSmartUniqueOrHelpId() ) )
    {
        if ( ( pWin->IsEnabled() || HasSearchFlag( SEARCH_FIND_DISABLED ) ) && pWin->IsVisible() )
            return TRUE;
        else
        {
            if ( !pMaybeResult )
                pMaybeResult = pWin;
            return FALSE;
        }
    }
    else if ( pWin->GetType() == WINDOW_TOOLBOX )   // Buttons and Controls on ToolBox.
    {
        ToolBox *pTB = ((ToolBox*)pWin);
        USHORT i;
        for ( i = 0; i < pTB->GetItemCount() ; i++ )
        {
            if ( aUId.Matches( pTB->GetItemCommand(pTB->GetItemId( i )) ) || aUId.Matches( pTB->GetHelpId(pTB->GetItemId( i )) ) )
            {       // ID matches.
                Window *pItemWin;
                pItemWin = pTB->GetItemWindow( pTB->GetItemId( i ) );

                if ( bSearchButtonOnToolbox && pTB->GetItemType( i ) == TOOLBOXITEM_BUTTON && !pItemWin )
                {       // We got a Control, see if its valid also.
                        // Same as above.
                    if ( ( pTB->IsEnabled() || HasSearchFlag( SEARCH_FIND_DISABLED ) ) && pTB->IsVisible() )
                    {   // We got a Button, see if its valid also.
                        if ( ( pTB->IsItemEnabled(pTB->GetItemId(i)) || HasSearchFlag( SEARCH_FIND_DISABLED ) )
                         && pTB->IsItemVisible(pTB->GetItemId(i)) )
                            return TRUE;    // We got a Button.
                        else
                        {   // better a disabled Button on a valid ToolBox than an invalid ToolBox as below
                            pMaybeResult = pTB;
                            return FALSE;
                        }
                    }
                    else if ( !pMaybeResult )
                    {   // invalid ToolBox
                        pMaybeResult = pTB;
                        return FALSE;
                    }
                }
                if ( pItemWin )
                {       // We got a Control, see if its valid also.
                        // Same as above.
                    if ( ( pItemWin->IsEnabled() || HasSearchFlag( SEARCH_FIND_DISABLED ) ) && pItemWin->IsVisible() )
                    {
                        if ( !pAlternateResult )    // only take the first found ItemWindow #i35365
                            pAlternateResult = pItemWin;    // since we cannot return a Window here
                        return FALSE;   // continue searching to prefer a window with the right ID #i32292
                    }
                    else if ( !pMaybeResult )
                    {
                        pMaybeResult = pItemWin;
                        return FALSE;
                    }
                }
            }
        }
        return FALSE;
    }
    else
        return FALSE;
}

Window* StatementList::SearchTree( SmartId aUId ,BOOL bSearchButtonOnToolbox )
{
    SearchUID aSearch(aUId,bSearchButtonOnToolbox);

    Window *pResult = SearchAllWin( NULL, aSearch );
    if ( pResult )
        return pResult;
    else if ( aSearch.GetAlternateResultWin() )
        return aSearch.GetAlternateResultWin();
    else
        return aSearch.GetMaybeWin();
}


BOOL SearchWinPtr::IsWinOK( Window *pWin )
{
    return pWin == pTest;
}

BOOL StatementList::WinPtrValid(Window *pTest)
{
    SearchWinPtr aSearch( pTest );
    return SearchAllWin( NULL, aSearch ) != NULL;
}


BOOL SearchRT::IsWinOK( Window *pWin )
{
    if ( pWin->IsVisible() && pWin->GetType() == mnRT )
    {
        mnCount++;
        if ( mnSkip )
        {
            mnSkip--;
            return FALSE;
        }
        else
            return TRUE;
    }
    return FALSE;
}

Window* StatementList::GetWinByRT( Window *pBase, WindowType nRT, BOOL MaybeBase, USHORT nSkip, BOOL bSearchAll )
{
    SearchRT aSearch( nRT, 0, nSkip );
    if ( bSearchAll )
        aSearch.AddSearchFlags( SEARCH_FOCUS_FIRST | SEARCH_FIND_DISABLED );
    else
        aSearch.AddSearchFlags( SEARCH_NOOVERLAP | SEARCH_NO_TOPLEVEL_WIN );

    return SearchAllWin( pBase, aSearch, MaybeBase );
}

USHORT StatementList::CountWinByRT( Window *pBase, WindowType nRT, BOOL MaybeBase )
{
    SearchRT aSearch( nRT, SEARCH_NOOVERLAP | SEARCH_NO_TOPLEVEL_WIN, 0xFFFF );

    SearchAllWin( pBase, aSearch, MaybeBase );
    return aSearch.GetCount();
}

BOOL SearchScroll::IsWinOK( Window *pWin )
{
    if ( SearchRT::IsWinOK( pWin ) )
    {
        DBG_ASSERT( pWin->GetStyle() & ( WB_HORZ | WB_VERT ), "Nither WB_HORZ nor WB_VERT set on ScrollBar");
        return (( pWin->GetStyle() & WB_HORZ ) && ( nDirection == CONST_ALIGN_BOTTOM ))
            || (( pWin->GetStyle() & WB_VERT ) && ( nDirection == CONST_ALIGN_RIGHT ));
    }
    return FALSE;
}

ScrollBar* StatementList::GetScrollBar( Window *pBase, USHORT nDirection, BOOL MaybeBase )
{
    SearchScroll aSearch( nDirection, SEARCH_NOOVERLAP | SEARCH_NO_TOPLEVEL_WIN );

    return (ScrollBar*)SearchAllWin( pBase, aSearch, MaybeBase );
}


BOOL SearchPopupFloatingWin::IsWinOK( Window *pWin )
{
    return pWin->IsVisible() && pWin->GetType() == WINDOW_FLOATINGWINDOW && ((FloatingWindow*)pWin)->IsInPopupMode();
}

Window* StatementList::GetPopupFloatingWin( BOOL MaybeBase )
{
    SearchPopupFloatingWin aSearch;

    return SearchAllWin( NULL, aSearch, MaybeBase );
}


Menu* StatementList::GetMatchingMenu( Window* pWin, Menu* pBaseMenu )
{
    if ( pBaseMenu )
    {
        if ( pBaseMenu->GetWindow() == pWin )
            return pBaseMenu;

        USHORT i;
            i = 0;
            while ( i < pBaseMenu->GetItemCount() )
            {
                PopupMenu* pPopup = pBaseMenu->GetPopupMenu( pBaseMenu->GetItemId( i ) );
                if ( pPopup && pPopup->GetWindow() )
                {
                    if ( pPopup->GetWindow() == pWin )
                        return pPopup;
                    else
                    {
                        pBaseMenu = pPopup;
                        i = 0;
                    }
                }
                else
                    i++;
            }
    }
    else
    {
        if ( PopupMenu::GetActivePopupMenu() )
        {
            Menu* pMenu = GetMatchingMenu( pWin, PopupMenu::GetActivePopupMenu() );
            if ( pMenu )
                return pMenu;
        }

        USHORT nSkip = 0;
        Window* pMenuBarWin = NULL;
        while ( (pMenuBarWin = GetWinByRT( NULL, WINDOW_MENUBARWINDOW, TRUE, nSkip++, TRUE )) != NULL )
        {
            Window* pParent = pMenuBarWin->GET_REAL_PARENT();
            if ( pParent && pParent->GetType() == WINDOW_BORDERWINDOW && pParent->IsVisible() )
            {
                Menu* pMenu = NULL;
                // find Menu of MenuBarWindow
                USHORT nCount;
                for ( nCount = 0 ; nCount < pParent->GetChildCount() ; nCount++ )
                {
                    if ( pParent->GetChild( nCount )->GetType() == WINDOW_WORKWINDOW )
                        pMenu = ((WorkWindow*)(pParent->GetChild( nCount )))->GetMenuBar();
                }
                if ( pMenu )
                {
                    // check for menu bar in Task Window
                    if ( pMenuBarWin == pWin )
                        return pMenu;

                    // search submenues
                    pMenu = GetMatchingMenu( pWin, pMenu );
                    if ( pMenu )
                        return pMenu;
                }
            }
        }
    }
    return NULL;
}


BOOL SearchActive::IsWinOK( Window *pWin )
{
    // only matches ResID due to problems with UNIX Window Managers
    return pWin->IsVisible() && ( (nRT == WINDOW_ANYTYPE && IsDialog(pWin) ) || pWin->GetType() == nRT );
}

Window* StatementList::GetActive( WindowType nRT, BOOL MaybeBase )
{
    SearchActive aSearch( nRT );

    return SearchAllWin( NULL, aSearch, MaybeBase );
}

BOOL SearchFadeSplitWin::IsWinOK( Window *pWin )
{
#if OSL_DEBUG_LEVEL > 1
    if ( pWin->GetType() == WINDOW_SPLITWINDOW )
    {
        BOOL bResult;
        WindowAlign aAlign;
        bResult = pWin->IsVisible();
        bResult = ((SplitWindow*)pWin)->IsFadeInButtonVisible();
        bResult = ((SplitWindow*)pWin)->IsFadeOutButtonVisible();
        bResult = ((SplitWindow*)pWin)->IsAutoHideButtonVisible();
        aAlign = ((SplitWindow*)pWin)->GetAlign();
    }
#endif
    return pWin->IsVisible() && ( pWin->GetType() == WINDOW_SPLITWINDOW )
        && (((SplitWindow*)pWin)->IsFadeInButtonVisible() || ((SplitWindow*)pWin)->IsFadeOutButtonVisible() )
        /*&& ((SplitWindow*)pWin)->IsAutoHideButtonVisible()*/ && ((SplitWindow*)pWin)->GetAlign() == nAlign;
}

Window* StatementList::GetFadeSplitWin( Window *pBase, WindowAlign nAlign, BOOL MaybeBase )
{
    SearchFadeSplitWin aSearch( nAlign );

    if ( GetpApp()->GetAppWindow() == pBase && pBase->GetType() != WINDOW_BORDERWINDOW )
        pBase = pBase->GetWindow( WINDOW_OVERLAP );

    return SearchAllWin( pBase, aSearch, MaybeBase );
}

Window* StatementList::GetMouseWin()
{
    Window *pBase = Application::GetFirstTopLevelWindow();
    Window *pControl = NULL;
    while ( pBase )
    {
        Window *pBaseFrame = pBase->GetWindow( WINDOW_OVERLAP );

        Point aP = pBaseFrame->GetPointerPosPixel();
        pControl = pBaseFrame->FindWindow( aP );
        if ( pControl )
            return pControl;

        pBase = Application::GetNextTopLevelWindow( pBase );
    }
    return NULL;
}

Window* StatementList::GetFocus( WindowType nRT, BOOL MaybeBase )
{

    if ( nRT == WINDOW_TABCONTROL )
    {
        Window *pResult = GetActive( WINDOW_TABDIALOG, MaybeBase);
        for( USHORT i = 0 ; pResult && i < pResult->GetChildCount(); i++ )
            if ( pResult->GetChild(i)->GetType() == nRT )
                return pResult->GetChild(i);
    }

    return NULL;
}

Window* StatementList::GetAnyActive( BOOL MaybeBase )
{
    Window *pControl;

    pControl = GetActive( WINDOW_MESSBOX, MaybeBase);
    if ( !pControl )
    {
        pControl = GetActive( WINDOW_INFOBOX, MaybeBase);
    }
    if ( !pControl )
    {
        pControl = GetActive( WINDOW_WARNINGBOX, MaybeBase);
    }
    if ( !pControl )
    {
        pControl = GetActive( WINDOW_ERRORBOX, MaybeBase);
    }
    if ( !pControl )
    {
        pControl = GetActive( WINDOW_QUERYBOX, MaybeBase);
    }
    if ( !pControl )
    {
        pControl = GetActive( WINDOW_BUTTONDIALOG, MaybeBase);
    }
    if ( !pControl )
    {
        pControl = GetActive( WINDOW_FILEDIALOG, MaybeBase);
    }
    if ( !pControl )
    {
        pControl = GetActive( WINDOW_PATHDIALOG, MaybeBase);
    }
    if ( !pControl )
    {
        pControl = GetActive( WINDOW_PRINTDIALOG, MaybeBase);
    }
    if ( !pControl )
    {
        pControl = GetActive( WINDOW_PRINTERSETUPDIALOG, MaybeBase);
    }
    if ( !pControl )
    {
        pControl = GetActive( WINDOW_COLORDIALOG, MaybeBase);
    }
    if ( !pControl )
    {
        pControl = GetFocus( WINDOW_TABCONTROL, MaybeBase);
    }

    return pControl;
}

void StatementList::SetFirstDocFrame( Window* pWin )
{
    DBG_ASSERT( IsDocFrame( pWin ), "Non Document Frame set as first Document Frame" );
    pFirstDocFrame = pWin;
}

Window* StatementList::GetFirstDocFrame()
{

    if ( pFirstDocFrame && !WinPtrValid( pFirstDocFrame ) )
        pFirstDocFrame = NULL;
    if ( pFirstDocFrame && !pFirstDocFrame->IsVisible() )
        pFirstDocFrame = NULL;
    if ( pFirstDocFrame && !IsDocFrame( pFirstDocFrame ) )
        pFirstDocFrame = NULL;
    if ( !pFirstDocFrame )
    {
        Window* pBase = Application::GetFirstTopLevelWindow();
        while ( pBase && !IsDocFrame( pBase ) )
            pBase = Application::GetNextTopLevelWindow( pBase );

        if ( pBase )
            SetFirstDocFrame( pBase );

        if ( !pBase )   // find just something
        {
            pBase = Application::GetFirstTopLevelWindow();
            while ( pBase && !pBase->IsVisible() )
                pBase = Application::GetNextTopLevelWindow( pBase );

            return pBase;   // just for now, later we will hopefully have a Window
        }
    }
    return pFirstDocFrame;
}

BOOL StatementList::IsFirstDocFrame( Window* pWin )
{
    return pWin && ( pWin == GetFirstDocFrame() || ( GetFirstDocFrame() && pWin == GetFirstDocFrame()->GetWindow( WINDOW_CLIENT ) ) ) && ( GetFirstDocFrame() && IsDocFrame( GetFirstDocFrame() ) );
}

MenuBar* StatementList::GetDocFrameMenuBar( Window* pWin )
{
    if ( pWin && pWin->IsVisible() && pWin->GetType() == WINDOW_BORDERWINDOW )
    {
        USHORT nCount;
        for ( nCount = 0 ; nCount < pWin->GetChildCount() ; nCount++ )
        {
            if ( pWin->GetChild( nCount )->GetType() == WINDOW_WORKWINDOW )
                return ((WorkWindow*)(pWin->GetChild( nCount )))->GetMenuBar();
        }
    }
    return NULL;
}

// a Doc Frame is a Document or the Backing Window
BOOL StatementList::IsDocFrame( Window* pWin )
{
    if ( pWin && pWin->IsVisible() && pWin->GetType() == WINDOW_BORDERWINDOW )
    {
        USHORT nCount;
        BOOL bHasWorkWindow = FALSE;
        BOOL bHasMenuBar = FALSE;
        // #91724# it is now necessary to sort out the IME WIndow in Solaris as well.
        // so now we check for existence of WINDOW_WORKWINDOW and newly for
        // WINDOW_MENUBARWINDOW which contains the Menu and the close/min/max buttons
        for ( nCount = 0 ; nCount < pWin->GetChildCount() ; nCount++ )
        {
            if ( pWin->GetChild( nCount )->GetType() == WINDOW_WORKWINDOW )
                bHasWorkWindow = TRUE;
            if ( pWin->GetChild( nCount )->GetType() == WINDOW_MENUBARWINDOW )
                bHasMenuBar = TRUE;
        }
        return bHasWorkWindow && bHasMenuBar;
    }
    return FALSE;
}

// a Doc Win is a real document (not the Backing Window)
BOOL StatementList::IsDocWin( Window* pWin )
{
    if ( pWin && IsDocFrame( pWin ) )
    {
        if ( GetDocFrameCount() != 1 )
            return TRUE;
        else
        {
            // check for the close button to see if we are the last one or only the backing Window
            if ( GetDocFrameMenuBar( pWin ) )
                return GetDocFrameMenuBar( pWin )->HasCloser();
        }
    }
    return FALSE;
}

BOOL StatementList::IsIMEWin( Window* pWin )    // Input Window for CJK under Solaris
{
    if ( pWin && pWin->IsVisible() && pWin->GetType() == WINDOW_BORDERWINDOW )
    {
        USHORT nCount;
        BOOL bHasWorkWindow = FALSE;
        BOOL bHasWindow = FALSE;
        // #91724# it is now necessary to sort out the IME WIndow in Solaris as well.
        // so now we check for existence of WINDOW_WORKWINDOW and newly for
        // WINDOW_WINDOW which contains the Menu and the close/min/max buttons
        for ( nCount = 0 ; nCount < pWin->GetChildCount() ; nCount++ )
            if ( pWin->GetChild( nCount )->GetType() == WINDOW_WORKWINDOW )
                bHasWorkWindow = TRUE;
        for ( nCount = 0 ; nCount < pWin->GetChildCount() ; nCount++ )
            if ( pWin->GetChild( nCount )->GetType() == WINDOW_WINDOW )
                bHasWindow = TRUE;
        return bHasWorkWindow && !bHasWindow;
    }
    return FALSE;
}

UniString StatementList::Tree(Window *pBase, int Indent)
{

    String aReturn, aSep;
    if ( !pBase )
    {
        aSep.AssignAscii("============================\n");
        aSep.ConvertLineEnd();
        pBase = Application::GetFirstTopLevelWindow();
        while ( pBase )
        {
            Window *pBaseFrame = pBase->GetWindow( WINDOW_OVERLAP );

            aReturn += aSep;
            aReturn += Tree( pBaseFrame, Indent+1 );

            pBase = Application::GetNextTopLevelWindow( pBase );
        }
        return aReturn;
    }


    aSep.AssignAscii("----------------------------\n");
    aSep.ConvertLineEnd();

    aReturn += ClientTree( pBase, Indent );

    if ( pBase->GetWindow( WINDOW_FIRSTOVERLAP ) )
    {
        aReturn += aSep;
        aReturn += Tree( pBase->GetWindow( WINDOW_FIRSTOVERLAP ), Indent+1 );
    }

    if ( pBase->GetWindow( WINDOW_NEXT ) )
    {
        aReturn += aSep;
        aReturn += Tree( pBase->GetWindow( WINDOW_NEXT ), Indent );
    }

    return aReturn;
}

String StatementList::ClientTree(Window *pBase, int Indent)
{
#if OSL_DEBUG_LEVEL > 1
#define WRITE(Text) { m_pDbgWin->AddText(Text); aReturn += Text; }
#define WRITEc(Text) { m_pDbgWin->AddText(Text); aReturn.AppendAscii(Text); }
#else
#define WRITE(Text) { aReturn += Text; }
#define WRITEc(Text) { aReturn.AppendAscii(Text); }
#endif

    String sIndent,aText,aReturn;
    sIndent.Expand(sal::static_int_cast< xub_StrLen >(2*Indent));

    aText = pBase->GetText();


    UniString t1,t2;t1 = CUniString("\n"); t2 = CUniString("\\n");
    aText.SearchAndReplaceAll(t1,t2 );

    WRITE(sIndent);

    if (pBase->IsDialog())
    {
        WRITEc("*(Dialog(TH))");
    }
    if (IsDialog( pBase ))
    {
        WRITEc("*(Dialog(GH))");
    }
    if (pBase->HasFocus())
    {
        WRITEc("*(Focus)");
    }
    if (!pBase->IsEnabled())
    {
        WRITEc("*(Disab)");
    }
    if (pBase->IsVisible())
    {
        WRITEc("*(Visible)");
    }
    if ( IsDialog(pBase) && ((SystemWindow*)pBase)->IsActive() )
    {
        WRITEc("*(Active)");
    }
    if ( pBase->GetStyle() & WB_CLOSEABLE )
    {
        WRITEc("*(Closable)");
    }
    if ( pBase->GetType() == WINDOW_DOCKINGWINDOW &&
            ((((DockingWindow*)pBase)->GetFloatStyle()) & WB_CLOSEABLE) )
    {
        WRITEc("*(Closable Docking in Floatingstyle)");
    }
    if ( pBase->GetStyle() & WB_DOCKABLE )
    {
        WRITEc("*(Dockable)");
    }
    if ( pBase->GetType() == WINDOW_SPLITWINDOW &&
            (((SplitWindow*)pBase)->IsFadeInButtonVisible() || ((SplitWindow*)pBase)->IsFadeOutButtonVisible()) )
    {
        WRITEc("*(FadeIn/Out)");
    }
    WRITEc("Text: ");
    WRITE(aText);
    WRITEc("\n");

    WRITE(sIndent);
    WRITEc("UId : ");
    WRITE(UIdString(pBase->GetSmartUniqueOrHelpId()));
    WRITEc(":0x");
    WRITE(
        String::CreateFromInt64(
            sal::static_int_cast< sal_Int64 >(
                reinterpret_cast< sal_IntPtr >(pBase)),
            16 ));
    WRITEc(":");
    WRITE(pBase->GetQuickHelpText());
    WRITEc(":");
    WRITE(pBase->GetHelpText());
    WRITEc("\n");

    WRITE(sIndent);
    WRITEc("RTyp: ");
    WRITE(MakeStringNumber(TypeKenn,pBase->GetType()));
    if ( pBase->GetType() == WINDOW_CONTROL )
    {
        if ( dynamic_cast< svt::EditBrowseBox* >(pBase) )
            WRITEc("/BrowseBox")
        else if ( dynamic_cast< ValueSet* >(pBase) )
            WRITEc("/ValueSet")
        else if ( dynamic_cast< svt::ORoadmap* >(pBase) )
            WRITEc("/RoadMap")
        else if ( dynamic_cast< svt::IExtensionListBox* >(pBase) )
            WRITEc("/ExtensionListBox")
        else if ( dynamic_cast< svt::table::TableControl* >(pBase) )
            WRITEc("/TableControl")
        else
            WRITEc("/Unknown")
    }
    WRITEc("\n");

    aReturn.ConvertLineEnd();
    USHORT i;
    for (i = 0 ; i < pBase->GetChildCount() ; i++)
    {
        aReturn += ClientTree(pBase->GetChild(i),Indent+1);
    }
    return aReturn;
}


BOOL StatementList::CheckWindowWait()
{
    static Time StartTime = Time(0L);   // Abbruch wenn Fenster absolut nicht schliesst.
    if ( StartTime == Time(0L) )
        StartTime = Time();

    if ( pWindowWaitPointer )
    {
#if OSL_DEBUG_LEVEL > 1
        m_pDbgWin->AddText( "Waiting for Window to close ... " );
#endif
        if ( WinPtrValid(pWindowWaitPointer) && IS_WINP_CLOSING(pWindowWaitPointer) )
        {
#if OSL_DEBUG_LEVEL > 1
            m_pDbgWin->AddText( aWindowWaitUId.GetText().AppendAscii(" Still Open. RType=") );
            m_pDbgWin->AddText( String::CreateFromInt32( pWindowWaitPointer->GetType() ).AppendAscii("\n") );
#endif

            // Ist die Zeit schonn abgelaufen?
            if ( StartTime + Time(0,0,10) < Time() )    // 10 Sekunden reichen wohl
            {
#if OSL_DEBUG_LEVEL > 1
                m_pDbgWin->AddText( "Close timed out. Going on!! " );
#endif
                pWindowWaitPointer->SetSmartHelpId(aWindowWaitOldHelpId, SMART_SET_ALL);
                pWindowWaitPointer->SetSmartUniqueId(aWindowWaitOldUniqueId, SMART_SET_ALL);

                aWindowWaitUId = SmartId();
                pWindowWaitPointer = NULL;
                StartTime = Time(0L);
                return TRUE;
            }

            return FALSE;
        }
        pWindowWaitPointer = NULL;
        aWindowWaitUId = SmartId();
#if OSL_DEBUG_LEVEL > 1
        m_pDbgWin->AddText( "Closed, Going on.\n" );
#endif
    }
    StartTime = Time(0L);
    return TRUE;
}

void StatementList::ReportError(String aMessage)
{
    ReportError ( SmartId(), aMessage );
}

void StatementList::ReportError(SmartId aUId, String aMessage)
{
    pRet->GenError ( aUId, aMessage );
    IsError = TRUE;
}

void StatementList::ReportError(String aMessage, ULONG nWhatever)
{
    ReportError ( aMessage.AppendAscii(" ").Append(UniString::CreateFromInt32(nWhatever)));
}

void StatementList::DirectLog( ULONG nType, String aMessage )
{
    if ( pRet )
        pRet->GenReturn( RET_DirectLoging, SmartId(nType), aMessage );
}


#define CALL_EVENT_WITH_NOTIFY( EventType, Event, WinP, Method )    \
{                                                                   \
    if ( StatementList::WinPtrValid( WinP ) )                       \
    {                                                               \
        NotifyEvent aNEvt( EventType, WinP, &Event );               \
        if ( !WinP->PreNotify( aNEvt ) )                            \
            WinP->Method( Event );                                  \
    }                                                               \
}

void ImplKeyInput( Window* pWin, KeyEvent &aKEvnt, BOOL bForceDirect )
{

    if ( StatementList::bUsePostEvents && !bForceDirect )
    {
        if ( StatementList::WinPtrValid( pWin ) )
        {
            ULONG nID1;
            ULONG nID2;
            nID1 = Application::PostKeyEvent( VCLEVENT_WINDOW_KEYINPUT, pWin, &aKEvnt );
            nID2 = Application::PostKeyEvent( VCLEVENT_WINDOW_KEYUP, pWin, &aKEvnt );
            // wait after posting both events so deleting pWin will remove the second event also
            ImplEventWait( nID1 );
            ImplEventWait( nID2 );
        }
    }
    else
    {
        if ( !Application::CallAccel( aKEvnt.GetKeyCode() ) )
        {
            CALL_EVENT_WITH_NOTIFY( EVENT_KEYINPUT, aKEvnt, pWin, KeyInput )

            KeyCode aCode = aKEvnt.GetKeyCode();
            if ( (aCode.GetCode() == KEY_CONTEXTMENU) || ((aCode.GetCode() == KEY_F10) && aCode.IsShift()) )
            {
                if ( StatementList::WinPtrValid( pWin ) )
                {
                    Point aPos;
                    // simulate mouseposition at center of window
                    Size aSize = pWin->GetOutputSize();
                    aPos = Point( aSize.getWidth()/2, aSize.getHeight()/2 );

                    CommandEvent aEvent( aPos, COMMAND_CONTEXTMENU, FALSE );
                    ImplCommand( pWin, aEvent );
                }
            }
        }

        CALL_EVENT_WITH_NOTIFY( EVENT_KEYUP, aKEvnt, pWin, KeyUp )
    }
};

void ImplMouseMove( Window* pWin, MouseEvent &aMEvnt, BOOL bForceDirect )
{
    if ( StatementList::bUsePostEvents && !bForceDirect )
    {
        if ( StatementList::WinPtrValid( pWin ) )
        {
            ULONG nID;
            nID = Application::PostMouseEvent( VCLEVENT_WINDOW_MOUSEMOVE, pWin, &aMEvnt );
            ImplEventWait( nID );
        }
    }
    else
    {
            if ( pWin->IsTracking() )
        {
            TrackingEvent   aTEvt( aMEvnt );
            pWin->Tracking( aTEvt );
        }
        else
            CALL_EVENT_WITH_NOTIFY( EVENT_MOUSEMOVE, aMEvnt, pWin, MouseMove )
    }
};

void ImplMouseButtonDown( Window* pWin, MouseEvent &aMEvnt, BOOL bForceDirect )
{
    if ( StatementList::bUsePostEvents && !bForceDirect )
    {
        if ( StatementList::WinPtrValid( pWin ) )
        {
            ULONG nID;
            nID = Application::PostMouseEvent( VCLEVENT_WINDOW_MOUSEBUTTONDOWN, pWin, &aMEvnt );
            ImplEventWait( nID );
        }
    }
    else
    {
        CALL_EVENT_WITH_NOTIFY( EVENT_MOUSEBUTTONDOWN, aMEvnt, pWin, MouseButtonDown )
    }
};

void ImplMouseButtonUp( Window* pWin, MouseEvent &aMEvnt, BOOL bForceDirect )
{
    if ( StatementList::bUsePostEvents && !bForceDirect )
    {
        if ( StatementList::WinPtrValid( pWin ) )
        {
            ULONG nID;
            nID = Application::PostMouseEvent( VCLEVENT_WINDOW_MOUSEBUTTONUP, pWin, &aMEvnt );
            ImplEventWait( nID );
        }
    }
    else
    {
            if ( pWin->IsTracking() )
        {
            // siehe #64693 die Position ist für Toolboxen relevant
            // #60020 Jetzt hoffentlich kein GPF mehr
            // Zuerst Tracking beenden ohne Event
            pWin->EndTracking( ENDTRACK_DONTCALLHDL );
            // dann eigenen Event mit richtigem Maus-Event senden
            TrackingEvent   aTEvt( aMEvnt, ENDTRACK_END );
            pWin->Tracking( aTEvt );
        }
        else
            CALL_EVENT_WITH_NOTIFY( EVENT_MOUSEBUTTONUP, aMEvnt, pWin, MouseButtonUp )
    }
};

void ImplEventWait( ULONG nID )
{
    while ( !Application::IsProcessedMouseOrKeyEvent( nID ) )
        Application::Yield();
}

void ImplCommand( Window* pWin, CommandEvent &aCmdEvnt )
{
    CALL_EVENT_WITH_NOTIFY( EVENT_COMMAND, aCmdEvnt, pWin, Command )
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
