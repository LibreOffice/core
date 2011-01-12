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
#ifndef _BASIC_TTRESHLP_HXX
#include <basic/ttstrhlp.hxx>
#endif
#include "statemnt.hxx"

#ifndef _RETSRTM_HXX
#include "retstrm.hxx"
#endif
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
sal_Bool StatementList::bReadingCommands = sal_False;
sal_Bool StatementList::bIsInReschedule = sal_False;
sal_uInt16 StatementList::nModalCount = 0;
Window *StatementList::pLastFocusWindow = NULL;
sal_Bool StatementList::bWasDragManager = sal_False;
sal_Bool StatementList::bWasPopupMenu = sal_False;
sal_Bool StatementList::bBasicWasRunning = sal_False;
RetStream *StatementList::pRet = NULL;
sal_Bool StatementList::IsError = sal_False;
sal_Bool StatementList::bDying = sal_False;
sal_Bool StatementList::bExecuting = sal_False;
StatementList *StatementList::pCurrentProfileStatement = NULL;
sal_Bool StatementList::bUsePostEvents = sal_True;
#if OSL_DEBUG_LEVEL > 1
EditWindow *StatementList::m_pDbgWin;
#endif


rtl::OString StatementList::aWindowWaitUId = rtl::OString();
Window *StatementList::pWindowWaitPointer = NULL;
rtl::OString StatementList::aWindowWaitOldHelpId = rtl::OString();
rtl::OString StatementList::aWindowWaitOldUniqueId = rtl::OString();
sal_uInt16 StatementList::nUseBindings = 0;

sal_uInt16 StatementList::aSubMenuId1 = 0;  // Untermenüs bei PopupMenus
sal_uInt16 StatementList::aSubMenuId2 = 0;  // erstmal 2-Stufig
sal_uInt16 StatementList::aSubMenuId3 = 0;  // and now even 3 levels #i31512#
SystemWindow *StatementList::pMenuWindow = NULL;
TTProperties *StatementList::pTTProperties = NULL;

sal_uInt16 StatementList::nMinTypeKeysDelay = 0;    // Verzögerung der einzelnen Anschläge für TypeKeys
sal_uInt16 StatementList::nMaxTypeKeysDelay = 0;
sal_Bool StatementList::bDoTypeKeysDelay = sal_False;

Window* StatementList::pFirstDocFrame = NULL;

sal_Bool StatementList::bIsSlotInExecute = sal_False;

sal_Bool StatementList::bCatchGPF = sal_True;


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
        pTTSettings->bToTop = sal_True;
    }

    return pTTSettings;
}




// FIXME: HELPID
#define IS_WINP_CLOSING(pWin) (pWin->GetHelpId().equals( "TT_Win_is_closing_HID" ) && pWin->GetUniqueId().equals( "TT_Win_is_closing_UID" ))

/*
UniString GEN_RES_STR0( sal_uLong nResId ) { return ResString( nResId ); }
UniString GEN_RES_STR1( sal_uLong nResId, const UniString &Text1 ) { return GEN_RES_STR0( nResId ).Append( ArgString( 1, Text1 ) ); }
UniString GEN_RES_STR2( sal_uLong nResId, const UniString &Text1, const UniString &Text2 ) { return GEN_RES_STR1( nResId, Text1 ).Append( ArgString( 2, Text2 ) ); }
UniString GEN_RES_STR3( sal_uLong nResId, const UniString &Text1, const UniString &Text2, const UniString &Text3 ) { return GEN_RES_STR2( nResId, Text1, Text2 ).Append( ArgString( 3, Text3 ) );}
*/
StatementList::StatementList()
: nRetryCount(MAX_RETRIES)
, bStatementInQue(sal_False)
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
            pRet->GenReturn( RET_ProfileInfo, 0, CUniString("InitProfile von anderem Statement gerufen ohne SendProfile\n") );
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
                pRet->GenReturn( RET_ProfileInfo, 0, pProfiler->GetProfileLine( aText ) );

            if ( pProfiler->IsPartitioning() )
                                // FIXME: HELPID
                pRet->GenReturn( RET_ProfileInfo, S_ProfileTime, static_cast<comm_ULONG>(pProfiler->GetPartitioningTime()) ); // GetPartitioningTime() sal_uLong != comm_ULONG on 64bit
        }

        if ( pProfiler->IsAutoProfiling() )
            pRet->GenReturn( RET_ProfileInfo, 0, pProfiler->GetAutoProfiling() );

#if OSL_DEBUG_LEVEL > 1
        if ( pCurrentProfileStatement == NULL )
            pRet->GenReturn( RET_ProfileInfo, 0, CUniString("SendProfile ohne InitProfile\n") );
#endif
        pCurrentProfileStatement = NULL;
    }
}

void StatementList::QueStatement(StatementList *pAfterThis)
{
    DBG_ASSERT(!bStatementInQue,"QueStatement für bereits eingetragenes Statement -> Abgebrochen");
    if ( bStatementInQue )
        return;

    bStatementInQue = sal_True;
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
    bStatementInQue = sal_False;
    pNext = NULL;
}


StatementList::~StatementList()
{
#if OSL_DEBUG_LEVEL > 1
    m_pDbgWin->AddText( "Deleting \n" );
#endif
    DBG_ASSERT(!bReadingCommands,"Deleting commands while reading them!");
}

Window* StatementList::GetDocWin( sal_uInt16 nNr )
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

sal_uInt16 StatementList::GetDocFrameCount()
{
    Window* pBase = Application::GetFirstTopLevelWindow();
    sal_uInt16 nCount = 0;

    while ( pBase )
    {
        if ( IsDocFrame( pBase ) )
            nCount++;
        pBase = Application::GetNextTopLevelWindow( pBase );
    }
    return nCount;
}

sal_uInt16 StatementList::GetDocWinCount()
{
    Window* pBase = Application::GetFirstTopLevelWindow();
    sal_uInt16 nCount = 0;

    while ( pBase )
    {
        if ( IsDocWin( pBase ) )
            nCount++;
        pBase = Application::GetNextTopLevelWindow( pBase );
    }
    return nCount;
}

Window* StatementList::SearchAllWin( Window *pBase, Search &aSearch, sal_Bool MaybeBase )
{

    if ( !pBase && !aSearch.HasSearchFlag( SEARCH_NO_TOPLEVEL_WIN ) )
    {
        sal_Bool bSearchFocusFirst = aSearch.HasSearchFlag( SEARCH_FOCUS_FIRST );

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

//              if ( !IsFirstDocFrame( pPParent ) )
//              {
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
//              }
            }
        }

        pBase = Application::GetFirstTopLevelWindow();

        // Skip FirstDocFrame
//      if ( bSearchFocusFirst && IsFirstDocFrame( pBase ) )
//          pBase = Application::GetNextTopLevelWindow( pBase );

        while ( pBase )
        {
            pControl = SearchAllWin( pBase, aSearch );
            if ( pControl )
                return pControl;

            pBase = Application::GetNextTopLevelWindow( pBase );
            // Skip FirstDocFrame
//          if ( bSearchFocusFirst && IsFirstDocFrame( pBase ) )
//              pBase = Application::GetNextTopLevelWindow( pBase );
        }
        return NULL;
    }


    Window *pResult = NULL;
    pResult = SearchClientWin( pBase, aSearch, MaybeBase );
    if ( pResult )
        return pResult;

//    if ( pBase->GetType() != WINDOW_BORDERWINDOW )
//      return NULL;

    if ( !aSearch.HasSearchFlag( SEARCH_NOOVERLAP ) )
    {
        if ( pBase->GetWindow( WINDOW_FIRSTOVERLAP ) )
            pResult = SearchAllWin( pBase->GetWindow( WINDOW_FIRSTOVERLAP ), aSearch );

        if ( !pResult && pBase->GetWindow( WINDOW_NEXT ) )
            pResult = SearchAllWin( pBase->GetWindow( WINDOW_NEXT ), aSearch );
    }

    return pResult;
}


Window* StatementList::SearchClientWin( Window *pBase, Search &aSearch, sal_Bool MaybeBase )
{
    if ( !pBase )
        return NULL;

    if ( MaybeBase && aSearch.IsWinOK( pBase ) )
        return pBase;

    Window *pResult = NULL;

    sal_uInt16 i;
    for( i = 0 ; i < pBase->GetChildCount() && !pResult; i++ )
        pResult = SearchClientWin( pBase->GetChild(i), aSearch );

    return pResult;
}


sal_Bool SearchUID::IsWinOK( Window *pWin )
{
    if ( aUId.equals( pWin->GetUniqueOrHelpId() ) )
    {
        if ( ( pWin->IsEnabled() || HasSearchFlag( SEARCH_FIND_DISABLED ) ) && pWin->IsVisible() )
            return sal_True;
        else
        {
            if ( !pMaybeResult )
                pMaybeResult = pWin;
            return sal_False;
        }
    }
    else if ( pWin->GetType() == WINDOW_TOOLBOX )   // Buttons and Controls on ToolBox.
    {
        ToolBox *pTB = ((ToolBox*)pWin);
        sal_uInt16 i;
        for ( i = 0; i < pTB->GetItemCount() ; i++ )
        {
            if ( aUId.equals( Str2Id( pTB->GetItemCommand(pTB->GetItemId( i )) ) ) || aUId.equals( pTB->GetHelpId(pTB->GetItemId( i )) ) )
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
                            return sal_True;    // We got a Button.
                        else
                        {   // better a disabled Button on a valid ToolBox than an invalid ToolBox as below
                            pMaybeResult = pTB;
                            return sal_False;
                        }
                    }
                    else if ( !pMaybeResult )
                    {   // invalid ToolBox
                        pMaybeResult = pTB;
                        return sal_False;
                    }
                }
                if ( pItemWin )
                {       // We got a Control, see if its valid also.
                        // Same as above.
                    if ( ( pItemWin->IsEnabled() || HasSearchFlag( SEARCH_FIND_DISABLED ) ) && pItemWin->IsVisible() )
                    {
                        if ( !pAlternateResult )    // only take the first found ItemWindow #i35365
                            pAlternateResult = pItemWin;    // since we cannot return a Window here
                        return sal_False;   // continue searching to prefer a window with the right ID #i32292
                    }
                    else if ( !pMaybeResult )
                    {
                        pMaybeResult = pItemWin;
                        return sal_False;
                    }
                }
            }
        }
        return sal_False;
    }
    else
        return sal_False;
}

Window* StatementList::SearchTree( rtl::OString aUId ,sal_Bool bSearchButtonOnToolbox )
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


sal_Bool SearchWinPtr::IsWinOK( Window *pWin )
{
    return pWin == pTest;
}

sal_Bool StatementList::WinPtrValid(Window *pTest)
{
    SearchWinPtr aSearch( pTest );
    return SearchAllWin( NULL, aSearch ) != NULL;
}


sal_Bool SearchRT::IsWinOK( Window *pWin )
{
    if ( pWin->IsVisible() && pWin->GetType() == mnRT )
    {
        mnCount++;
        if ( mnSkip )
        {
            mnSkip--;
            return sal_False;
        }
        else
            return sal_True;
    }
    return sal_False;
}

Window* StatementList::GetWinByRT( Window *pBase, WindowType nRT, sal_Bool MaybeBase, sal_uInt16 nSkip, sal_Bool bSearchAll )
{
    SearchRT aSearch( nRT, 0, nSkip );
    if ( bSearchAll )
        aSearch.AddSearchFlags( SEARCH_FOCUS_FIRST | SEARCH_FIND_DISABLED );
    else
        aSearch.AddSearchFlags( SEARCH_NOOVERLAP | SEARCH_NO_TOPLEVEL_WIN );

    return SearchAllWin( pBase, aSearch, MaybeBase );
}

sal_uInt16 StatementList::CountWinByRT( Window *pBase, WindowType nRT, sal_Bool MaybeBase )
{
    SearchRT aSearch( nRT, SEARCH_NOOVERLAP | SEARCH_NO_TOPLEVEL_WIN, 0xFFFF );

    SearchAllWin( pBase, aSearch, MaybeBase );
    return aSearch.GetCount();
}

sal_Bool SearchScroll::IsWinOK( Window *pWin )
{
    if ( SearchRT::IsWinOK( pWin ) )
    {
        DBG_ASSERT( pWin->GetStyle() & ( WB_HORZ | WB_VERT ), "Nither WB_HORZ nor WB_VERT set on ScrollBar");
        return (( pWin->GetStyle() & WB_HORZ ) && ( nDirection == CONST_ALIGN_BOTTOM ))
            || (( pWin->GetStyle() & WB_VERT ) && ( nDirection == CONST_ALIGN_RIGHT ));
    }
    return sal_False;
}

ScrollBar* StatementList::GetScrollBar( Window *pBase, sal_uInt16 nDirection, sal_Bool MaybeBase )
{
    SearchScroll aSearch( nDirection, SEARCH_NOOVERLAP | SEARCH_NO_TOPLEVEL_WIN );

    return (ScrollBar*)SearchAllWin( pBase, aSearch, MaybeBase );
}


sal_Bool SearchPopupFloatingWin::IsWinOK( Window *pWin )
{
    return pWin->IsVisible() && pWin->GetType() == WINDOW_FLOATINGWINDOW && ((FloatingWindow*)pWin)->IsInPopupMode();
}

Window* StatementList::GetPopupFloatingWin( sal_Bool MaybeBase )
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

        sal_uInt16 i;
//        while ( pBaseMenu )
//        {
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
//        }
    }
    else
    {
        if ( PopupMenu::GetActivePopupMenu() )
        {
            Menu* pMenu = GetMatchingMenu( pWin, PopupMenu::GetActivePopupMenu() );
            if ( pMenu )
                return pMenu;
        }

        sal_uInt16 nSkip = 0;
        Window* pMenuBarWin = NULL;
        while ( (pMenuBarWin = GetWinByRT( NULL, WINDOW_MENUBARWINDOW, sal_True, nSkip++, sal_True )) != NULL )
        {
            Window* pParent = pMenuBarWin->GET_REAL_PARENT();
            if ( pParent && pParent->GetType() == WINDOW_BORDERWINDOW && pParent->IsVisible() )
            {
                Menu* pMenu = NULL;
                // find Menu of MenuBarWindow
                sal_uInt16 nCount;
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


sal_Bool SearchActive::IsWinOK( Window *pWin )
{
//  return pWin->IsVisible() && ( (nRT == WINDOW_ANYTYPE && IsDialog(pWin) ) || pWin->GetType() == nRT )  && (nRT == WINDOW_FILEDIALOG || nRT == WINDOW_PATHDIALOG || nRT == WINDOW_PRINTDIALOG || nRT == WINDOW_PRINTERSETUPDIALOG || nRT == WINDOW_COLORDIALOG || ((SystemWindow*)pWin)->IsActive());
    // only matches ResID due to problems with UNIX Window Managers
    return pWin->IsVisible() && ( (nRT == WINDOW_ANYTYPE && IsDialog(pWin) ) || pWin->GetType() == nRT );
}

Window* StatementList::GetActive( WindowType nRT, sal_Bool MaybeBase )
{
    SearchActive aSearch( nRT );

    return SearchAllWin( NULL, aSearch, MaybeBase );
}

sal_Bool SearchFadeSplitWin::IsWinOK( Window *pWin )
{
#if OSL_DEBUG_LEVEL > 1
    if ( pWin->GetType() == WINDOW_SPLITWINDOW )
    {
        sal_Bool bResult;
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

Window* StatementList::GetFadeSplitWin( Window *pBase, WindowAlign nAlign, sal_Bool MaybeBase )
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

Window* StatementList::GetFocus( WindowType nRT, sal_Bool MaybeBase )
{

    if ( nRT == WINDOW_TABCONTROL )
    {
        Window *pResult = GetActive( WINDOW_TABDIALOG, MaybeBase);
        for( sal_uInt16 i = 0 ; pResult && i < pResult->GetChildCount(); i++ )
            if ( pResult->GetChild(i)->GetType() == nRT )
                return pResult->GetChild(i);
    }

    return NULL;
}

Window* StatementList::GetAnyActive( sal_Bool MaybeBase )
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

sal_Bool StatementList::IsFirstDocFrame( Window* pWin )
{
    return pWin && ( pWin == GetFirstDocFrame() || ( GetFirstDocFrame() && pWin == GetFirstDocFrame()->GetWindow( WINDOW_CLIENT ) ) ) && ( GetFirstDocFrame() && IsDocFrame( GetFirstDocFrame() ) );
}

MenuBar* StatementList::GetDocFrameMenuBar( Window* pWin )
{
    if ( pWin && pWin->IsVisible() && pWin->GetType() == WINDOW_BORDERWINDOW )
    {
        sal_uInt16 nCount;
        for ( nCount = 0 ; nCount < pWin->GetChildCount() ; nCount++ )
        {
            if ( pWin->GetChild( nCount )->GetType() == WINDOW_WORKWINDOW )
                return ((WorkWindow*)(pWin->GetChild( nCount )))->GetMenuBar();
        }
    }
    return NULL;
}

// a Doc Frame is a Document or the Backing Window
sal_Bool StatementList::IsDocFrame( Window* pWin )
{
    if ( pWin && pWin->IsVisible() && pWin->GetType() == WINDOW_BORDERWINDOW )
    {
        sal_uInt16 nCount;
        sal_Bool bHasWorkWindow = sal_False;
        sal_Bool bHasMenuBar = sal_False;
        // #91724# it is now necessary to sort out the IME WIndow in Solaris as well.
        // so now we check for existence of WINDOW_WORKWINDOW and newly for
        // WINDOW_MENUBARWINDOW which contains the Menu and the close/min/max buttons
        for ( nCount = 0 ; nCount < pWin->GetChildCount() ; nCount++ )
        {
            if ( pWin->GetChild( nCount )->GetType() == WINDOW_WORKWINDOW )
                bHasWorkWindow = sal_True;
            if ( pWin->GetChild( nCount )->GetType() == WINDOW_MENUBARWINDOW )
                bHasMenuBar = sal_True;
        }
        return bHasWorkWindow && bHasMenuBar;
    }
    return sal_False;
}

// a Doc Win is a real document (not the Backing Window)
sal_Bool StatementList::IsDocWin( Window* pWin )
{
    if ( pWin && IsDocFrame( pWin ) )
    {
        if ( GetDocFrameCount() != 1 )
            return sal_True;
        else
        {
            // check for the close button to see if we are the last one or only the backing Window
            if ( GetDocFrameMenuBar( pWin ) )
                return GetDocFrameMenuBar( pWin )->HasCloser();
        }
    }
    return sal_False;
}

sal_Bool StatementList::IsIMEWin( Window* pWin )    // Input Window for CJK under Solaris
{
    if ( pWin && pWin->IsVisible() && pWin->GetType() == WINDOW_BORDERWINDOW )
    {
        sal_uInt16 nCount;
        sal_Bool bHasWorkWindow = sal_False;
        sal_Bool bHasWindow = sal_False;
        // #91724# it is now necessary to sort out the IME WIndow in Solaris as well.
        // so now we check for existence of WINDOW_WORKWINDOW and newly for
        // WINDOW_WINDOW which contains the Menu and the close/min/max buttons
        for ( nCount = 0 ; nCount < pWin->GetChildCount() ; nCount++ )
            if ( pWin->GetChild( nCount )->GetType() == WINDOW_WORKWINDOW )
                bHasWorkWindow = sal_True;
        for ( nCount = 0 ; nCount < pWin->GetChildCount() ; nCount++ )
            if ( pWin->GetChild( nCount )->GetType() == WINDOW_WINDOW )
                bHasWindow = sal_True;
        return bHasWorkWindow && !bHasWindow;
    }
    return sal_False;
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
    WRITE(Id2Str(pBase->GetUniqueOrHelpId()));
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
    sal_uInt16 i;
    for (i = 0 ; i < pBase->GetChildCount() ; i++)
    {
        aReturn += ClientTree(pBase->GetChild(i),Indent+1);
    }
    return aReturn;
}


sal_Bool StatementList::CheckWindowWait()
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
            m_pDbgWin->AddText( Id2Str(aWindowWaitUId).AppendAscii(" Still Open. RType=") );
            m_pDbgWin->AddText( String::CreateFromInt32( pWindowWaitPointer->GetType() ).AppendAscii("\n") );
#endif

            // Ist die Zeit schonn abgelaufen?
            if ( StartTime + Time(0,0,10) < Time() )    // 10 Sekunden reichen wohl
            {
#if OSL_DEBUG_LEVEL > 1
                m_pDbgWin->AddText( "Close timed out. Going on!! " );
#endif
                pWindowWaitPointer->SetHelpId(aWindowWaitOldHelpId);
                pWindowWaitPointer->SetUniqueId(aWindowWaitOldUniqueId);

                aWindowWaitUId = rtl::OString();
                pWindowWaitPointer = NULL;
                StartTime = Time(0L);
                return sal_True;
            }

            return sal_False;
        }
        pWindowWaitPointer = NULL;
        aWindowWaitUId = rtl::OString();
#if OSL_DEBUG_LEVEL > 1
        m_pDbgWin->AddText( "Closed, Going on.\n" );
#endif
    }
    StartTime = Time(0L);
    return sal_True;
}

void StatementList::ReportError(String aMessage)
{
    ReportError ( rtl::OString(), aMessage );
}

void StatementList::ReportError(rtl::OString aUId, String aMessage)
{
    pRet->GenError ( aUId, aMessage );
    IsError = sal_True;
}

void StatementList::ReportError(String aMessage, sal_uLong nWhatever)
{
    ReportError ( aMessage.AppendAscii(" ").Append(UniString::CreateFromInt32(nWhatever)));
}

void StatementList::DirectLog( sal_uLong nType, String aMessage )
{
    if ( pRet )
        pRet->GenReturn( RET_DirectLoging, (sal_uInt16) nType, aMessage );
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

void ImplKeyInput( Window* pWin, KeyEvent &aKEvnt, sal_Bool bForceDirect )
{

    if ( StatementList::bUsePostEvents && !bForceDirect )
    {
        if ( StatementList::WinPtrValid( pWin ) )
        {
            sal_uLong nID1;
            sal_uLong nID2;
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

                    CommandEvent aEvent( aPos, COMMAND_CONTEXTMENU, sal_False );
                    ImplCommand( pWin, aEvent );
                }
            }
        }

        CALL_EVENT_WITH_NOTIFY( EVENT_KEYUP, aKEvnt, pWin, KeyUp )
    }
};

void ImplMouseMove( Window* pWin, MouseEvent &aMEvnt, sal_Bool bForceDirect )
{
    if ( StatementList::bUsePostEvents && !bForceDirect )
    {
        if ( StatementList::WinPtrValid( pWin ) )
        {
            sal_uLong nID;
            nID = Application::PostMouseEvent( VCLEVENT_WINDOW_MOUSEMOVE, pWin, &aMEvnt );
            ImplEventWait( nID );
        }
    }
    else
    {
    //  DragManager* pDragManager = DragManager::GetDragManager();
    //  if ( pDragManager )
    //      pDragManager->MouseMove( aMEvnt, pWin );
    //  else
            if ( pWin->IsTracking() )
        {
            TrackingEvent   aTEvt( aMEvnt );
            pWin->Tracking( aTEvt );
        }
        else
            CALL_EVENT_WITH_NOTIFY( EVENT_MOUSEMOVE, aMEvnt, pWin, MouseMove )
    }
};

void ImplMouseButtonDown( Window* pWin, MouseEvent &aMEvnt, sal_Bool bForceDirect )
{
    if ( StatementList::bUsePostEvents && !bForceDirect )
    {
        if ( StatementList::WinPtrValid( pWin ) )
        {
            sal_uLong nID;
            nID = Application::PostMouseEvent( VCLEVENT_WINDOW_MOUSEBUTTONDOWN, pWin, &aMEvnt );
            ImplEventWait( nID );
        }
    }
    else
    {
        CALL_EVENT_WITH_NOTIFY( EVENT_MOUSEBUTTONDOWN, aMEvnt, pWin, MouseButtonDown )
    }
};

void ImplMouseButtonUp( Window* pWin, MouseEvent &aMEvnt, sal_Bool bForceDirect )
{
    if ( StatementList::bUsePostEvents && !bForceDirect )
    {
        if ( StatementList::WinPtrValid( pWin ) )
        {
            sal_uLong nID;
            nID = Application::PostMouseEvent( VCLEVENT_WINDOW_MOUSEBUTTONUP, pWin, &aMEvnt );
            ImplEventWait( nID );
        }
    }
    else
    {
    //      DragManager* pDragManager = DragManager::GetDragManager();
    //  if ( pDragManager )
    //      pDragManager->ButtonUp( aMEvnt, pWin );
    //  else
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

void ImplEventWait( sal_uLong nID )
{
    while ( !Application::IsProcessedMouseOrKeyEvent( nID ) )
        Application::Yield();
}

void ImplCommand( Window* pWin, CommandEvent &aCmdEvnt )
{
    CALL_EVENT_WITH_NOTIFY( EVENT_COMMAND, aCmdEvnt, pWin, Command )
};

