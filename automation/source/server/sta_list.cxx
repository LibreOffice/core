/*************************************************************************
 *
 *  $RCSfile: sta_list.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:48:44 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2002
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2002 by Sun Microsystems, Inc.
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
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _TIME_HXX //autogen
#include <tools/time.hxx>
#endif
#ifndef _SV_SPLITWIN_HXX //autogen
#include <vcl/splitwin.hxx>
#endif
#ifndef _SV_WRKWIN_HXX //handmade
#include <vcl/wrkwin.hxx>
#endif
#ifndef _BASIC_TTRESHLP_HXX
#include <basic/ttstrhlp.hxx>
#endif

#ifndef _STATEMNT_HXX
#include "statemnt.hxx"
#endif

#ifndef _RETSRTM_HXX
#include "retstrm.hxx"
#endif

#ifndef _RCONTROL_HXX
#include "rcontrol.hxx"
#endif

#if OSL_DEBUG_LEVEL > 1
#ifndef _EDITWIN_HXX
#include "editwin.hxx"
#endif
#endif

#include "profiler.hxx"

#ifndef _SV_FLOATWIN_HXX //autogen
#include <vcl/floatwin.hxx>
#endif

#ifndef _SV_TOOLBOX_HXX //autogen
#include <vcl/toolbox.hxx>
#endif

#define WINDOW_ANYTYPE WINDOW_BASE


TTProfiler *StatementList::pProfiler = NULL;
StatementList *StatementList::pFirst = NULL;
BOOL StatementList::bReadingCommands = FALSE;
BOOL StatementList::bIsInReschedule = FALSE;
Window *StatementList::pLastFocusWindow = NULL;
BOOL StatementList::bWasDragManager = FALSE;
BOOL StatementList::bWasPopupMenu = FALSE;
BOOL StatementList::bBasicWasRunning = FALSE;
RetStream *StatementList::pRet = NULL;
BOOL StatementList::IsError = FALSE;
BOOL StatementList::bDying = FALSE;
BOOL StatementList::bExecuting = FALSE;
StatementList *StatementList::pCurrentProfileStatement = NULL;
USHORT StatementList::nControlType = CONST_CTBrowseBox;
#if OSL_DEBUG_LEVEL > 1
EditWindow *StatementList::m_pDbgWin;
#endif


ULONG StatementList::nWindowWaitUId = 0;
Window *StatementList::pWindowWaitPointer = NULL;
ULONG StatementList::nWindowWaitOldHelpId = 0;
ULONG StatementList::nWindowWaitOldUniqueId = 0;
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

BOOL StatementList::bCatchGPF = TRUE;

#define IS_WINP_CLOSING(pWin) (pWin->GetHelpId() == 4321 && pWin->GetUniqueId() == 1234)

/*
UniString GEN_RES_STR0( ULONG nResId ) { return ResString( nResId ); }
UniString GEN_RES_STR1( ULONG nResId, const UniString &Text1 ) { return GEN_RES_STR0( nResId ).Append( ArgString( 1, Text1 ) ); }
UniString GEN_RES_STR2( ULONG nResId, const UniString &Text1, const UniString &Text2 ) { return GEN_RES_STR1( nResId, Text1 ).Append( ArgString( 2, Text2 ) ); }
UniString GEN_RES_STR3( ULONG nResId, const UniString &Text1, const UniString &Text2, const UniString &Text3 ) { return GEN_RES_STR2( nResId, Text1, Text2 ).Append( ArgString( 3, Text3 ) );}
*/
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
                pRet->GenReturn( RET_ProfileInfo, SmartId( S_ProfileTime ), pProfiler->GetPartitioningTime() );
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
    DBG_ASSERT(!bStatementInQue,"QueStatement für bereits eingetragenes Statement -> Abgebrochen")
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
                DBG_ASSERT( WinPtrValid( pBase ), "GetFocusWindow is no valid WindowPointer" )
                Window *pPParent = pBase;
                while ( pPParent->GET_REAL_PARENT() )
                    pPParent = pPParent->GET_REAL_PARENT();
                if ( !IsFirstDocFrame( pPParent ) )
                {
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
        }

        pBase = Application::GetFirstTopLevelWindow();

        // Skip FirstDocFrame
        if ( bSearchFocusFirst && IsFirstDocFrame( pBase ) )
            pBase = Application::GetNextTopLevelWindow( pBase );

        while ( pBase )
        {
            pControl = SearchAllWin( pBase, aSearch );
            if ( pControl )
                return pControl;

            pBase = Application::GetNextTopLevelWindow( pBase );
            // Skip FirstDocFrame
            if ( bSearchFocusFirst && IsFirstDocFrame( pBase ) )
                pBase = Application::GetNextTopLevelWindow( pBase );
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


Window* StatementList::SearchClientWin( Window *pBase, Search &aSearch, BOOL MaybeBase )
{
    if ( !pBase )
        return NULL;

    if ( MaybeBase && aSearch.IsWinOK( pBase ) )
        return pBase;

    Window *pResult = NULL;

    int i;
    for( i = 0 ; i < pBase->GetChildCount() && !pResult; i++ )
        pResult = SearchClientWin( pBase->GetChild(i), aSearch );

    return pResult;
}


BOOL SearchUID::IsWinOK( Window *pWin )
{
//  #97961# tabpages on non tabdialogs don't set the ID of the dialog and thus have to be acessible by themselves.
//  it is unclear why these were excluded in the first place
//  if ( pWin->GetUniqueOrHelpId() == aUId && ( pWin->GetType() != WINDOW_TABPAGE || pWin->GetWindow( WINDOW_REALPARENT )->GetType() != WINDOW_TABCONTROL ) )
    if ( aUId.Equals( pWin->GetUniqueOrHelpId() ) )
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
    else if ( pWin->GetType() == WINDOW_TOOLBOX )   // Buttons und Controls auf Toolboxen.
    {
        ToolBox *pTB = ((ToolBox*)pWin);
        if ( aUId.Equals( pTB->GetHelpIdAsString() ) )
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
        int i;
        for ( i = 0; i < pTB->GetItemCount() ; i++ )
        {
            if ( aUId.Equals( pTB->GetItemCommand(pTB->GetItemId( i )) ) || aUId.Equals( pTB->GetHelpId(pTB->GetItemId( i )) ) )
            {       // Die ID stimmt.
                Window *pItemWin;
                pItemWin = pTB->GetItemWindow( pTB->GetItemId( i ) );

                if ( bSearchButtonOnToolbox && pTB->GetItemType( i ) == TOOLBOXITEM_BUTTON && !pItemWin )
                {       // Wir haben ein Control. Noch sehen ob Gültig.
                        // Das Gleiche wie oben.
                    if ( ( pTB->IsEnabled() || HasSearchFlag( SEARCH_FIND_DISABLED ) ) && pTB->IsVisible() )
                    {   // Wir haben einen Button. Noch sehen ob Gültig.
                        if ( ( pTB->IsItemEnabled(pTB->GetItemId(i)) || HasSearchFlag( SEARCH_FIND_DISABLED ) )
                         && pTB->IsItemVisible(pTB->GetItemId(i)) )
                            return TRUE;    // Wir haben einen Button.
                        else
                        {   // Lieber der Button auf der Gültigen ToolBox als der Fall unten
                            pMaybeResult = pTB;
                            return FALSE;
                        }
                    }
                    else if ( !pMaybeResult )
                    {   // ToolBox ungültig
                        pMaybeResult = pTB;
                        return FALSE;
                    }
                }
                if ( pItemWin )
                {       // Wir haben ein Control. Noch sehen ob Gültig.
                        // Das Gleiche wie oben.
                    if ( ( pItemWin->IsEnabled() || HasSearchFlag( SEARCH_FIND_DISABLED ) ) && pItemWin->IsVisible() )
                    {
                        pMaybeResult = pItemWin;    // Da wir hier kein Window zurückliefern können
                        return FALSE;
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
    if ( !pResult )
        return aSearch.GetMaybeWin();
    else
        return pResult;
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

        USHORT nSkip = 0;
        Window* pMenuBarWin = NULL;
        while ( pMenuBarWin = GetWinByRT( NULL, WINDOW_MENUBARWINDOW, TRUE, nSkip++, TRUE ) )
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
//  return pWin->IsVisible() && ( (nRT == WINDOW_ANYTYPE && IsDialog(pWin) ) || pWin->GetType() == nRT )  && (nRT == WINDOW_FILEDIALOG || nRT == WINDOW_PATHDIALOG || nRT == WINDOW_PRINTDIALOG || nRT == WINDOW_PRINTERSETUPDIALOG || nRT == WINDOW_COLORDIALOG || ((SystemWindow*)pWin)->IsActive());
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
        && ((SplitWindow*)pWin)->IsAutoHideButtonVisible() && ((SplitWindow*)pWin)->GetAlign() == nAlign;
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

Window* StatementList::GetFocus( WindowType nRT ,BOOL ParentWasRT, BOOL MaybeBase )
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
        pControl = GetFocus( WINDOW_TABCONTROL, FALSE, MaybeBase);
    }

    return pControl;
}

void StatementList::SetFirstDocFrame( Window* pWin )
{
    DBG_ASSERT( IsDocFrame( pWin ), "Non Document Frame set as first Document Frame" )
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
        Window *pControl = NULL;
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
#define WRITE(Text) m_pDbgWin->AddText(Text); aReturn += Text
#define WRITEc(Text) m_pDbgWin->AddText(Text); aReturn.AppendAscii(Text)
#else
#define WRITE(Text) aReturn += Text
#define WRITEc(Text) aReturn.AppendAscii(Text)
#endif

    String sIndent,aText,aReturn;
    sIndent.Expand(2*Indent);

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
    WRITE(MakeStringNumber(UIdKenn,pBase->GetUniqueOrHelpId()));
    WRITEc(":");
    WRITE(pBase->GetQuickHelpText());
    WRITEc(":");
    WRITE(pBase->GetHelpText());
    WRITEc("\n");

    WRITE(sIndent);
    WRITEc("RTyp: ");
    WRITE(MakeStringNumber(TypeKenn,pBase->GetType()));
    WRITEc("\n");

    aReturn.ConvertLineEnd();
    int i;
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
            m_pDbgWin->AddText( String::CreateFromInt64( nWindowWaitUId ).AppendAscii(" Still Open. RType=") );
            m_pDbgWin->AddText( String::CreateFromInt32( pWindowWaitPointer->GetType() ).AppendAscii("\n") );
#endif

            // Ist die Zeit schonn abgelaufen?
            if ( StartTime + Time(0,0,10) < Time() )    // 10 Sekunden reichen wohl
            {
#if OSL_DEBUG_LEVEL > 1
                m_pDbgWin->AddText( "Close timed out. Going on!! " );
#endif
                pWindowWaitPointer->SetHelpId(nWindowWaitOldHelpId);
                pWindowWaitPointer->SetUniqueId(nWindowWaitOldUniqueId);

                nWindowWaitUId = 0;
                pWindowWaitPointer = NULL;
                StartTime = Time(0L);
                return TRUE;
            }

            return FALSE;
        }
        pWindowWaitPointer = NULL;
        nWindowWaitUId = 0;
#if OSL_DEBUG_LEVEL > 1
        m_pDbgWin->AddText( "Closed, Going on.\n" );
#endif
    }
    StartTime = Time(0L);
    return TRUE;
}

void StatementList::ReportError(String aMessage)
{
    ReportError ( SmartId(-1), aMessage );
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

void ImplKeyInput( Window* pWin, KeyEvent &aKEvnt )
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
};

void ImplMouseMove( Window* pWin, MouseEvent &aMEvnt )
{
/*  DragManager* pDragManager = DragManager::GetDragManager();
    if ( pDragManager )
        pDragManager->MouseMove( aMEvnt, pWin );
    else*/ if ( pWin->IsTracking() )
    {
        TrackingEvent   aTEvt( aMEvnt );
        pWin->Tracking( aTEvt );
    }
    else
        CALL_EVENT_WITH_NOTIFY( EVENT_MOUSEMOVE, aMEvnt, pWin, MouseMove )
};

void ImplMouseButtonDown( Window* pWin, MouseEvent &aMEvnt )
{
    CALL_EVENT_WITH_NOTIFY( EVENT_MOUSEBUTTONDOWN, aMEvnt, pWin, MouseButtonDown )
};

void ImplMouseButtonUp( Window* pWin, MouseEvent &aMEvnt )
{
/*  DragManager* pDragManager = DragManager::GetDragManager();
    if ( pDragManager )
        pDragManager->ButtonUp( aMEvnt, pWin );
    else*/ if ( pWin->IsTracking() )
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
};

void ImplCommand( Window* pWin, CommandEvent &aCmdEvnt )
{
    CALL_EVENT_WITH_NOTIFY( EVENT_COMMAND, aCmdEvnt, pWin, Command )
};

