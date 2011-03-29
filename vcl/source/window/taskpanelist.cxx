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
#include "precompiled_vcl.hxx"

#include <tools/rcid.h>

#include <vcl/dockwin.hxx>
#include <vcl/taskpanelist.hxx>

#include <svdata.hxx>

#include <functional>
#include <algorithm>

// can't have static linkage because SUNPRO 5.2 complains
Point ImplTaskPaneListGetPos( const Window *w )
{
    Point pos;
    if( w->ImplIsDockingWindow() )
    {
        pos = ((DockingWindow*)w)->GetPosPixel();
        Window *pF = ((DockingWindow*)w)->GetFloatingWindow();
        if( pF )
            pos = pF->OutputToAbsoluteScreenPixel( pF->ScreenToOutputPixel( pos ) );
        else
            pos = w->OutputToAbsoluteScreenPixel( pos );
    }
    else
        pos = w->OutputToAbsoluteScreenPixel( w->GetPosPixel() );

    return pos;
}

// compares window pos left-to-right
struct LTRSort : public ::std::binary_function< const Window*, const Window*, bool >
{
    bool operator()( const Window* w1, const Window* w2 ) const
    {
        Point pos1(ImplTaskPaneListGetPos( w1 ));
        Point pos2(ImplTaskPaneListGetPos( w2 ));

        if( pos1.X() == pos2.X() )
            return ( pos1.Y() < pos2.Y() );
        else
            return ( pos1.X() < pos2.X() );
    }
};
struct LTRSortBackward : public ::std::binary_function< const Window*, const Window*, bool >
{
    bool operator()( const Window* w2, const Window* w1 ) const
    {
        Point pos1(ImplTaskPaneListGetPos( w1 ));
        Point pos2(ImplTaskPaneListGetPos( w2 ));

        if( pos1.X() == pos2.X() )
            return ( pos1.Y() < pos2.Y() );
        else
            return ( pos1.X() < pos2.X() );
    }
};

// --------------------------------------------------

static void ImplTaskPaneListGrabFocus( Window *pWindow )
{
    // put focus in child of floating windows which is typically a toolbar
    // that can deal with the focus
    if( pWindow->ImplIsFloatingWindow() && pWindow->GetWindow( WINDOW_FIRSTCHILD ) )
        pWindow = pWindow->GetWindow( WINDOW_FIRSTCHILD );
    pWindow->GrabFocus();
}

// --------------------------------------------------

TaskPaneList::TaskPaneList()
{
}

TaskPaneList::~TaskPaneList()
{
}

// --------------------------------------------------

void TaskPaneList::AddWindow( Window *pWindow )
{
#if OSL_DEBUG_LEVEL > 0
    bool bDockingWindow=false;
    bool bToolbox=false;
    bool bDialog=false;
    bool bUnknown=false;
#endif

    if( pWindow )
    {
#if OSL_DEBUG_LEVEL > 0
        if( pWindow->GetType() == RSC_DOCKINGWINDOW )
            bDockingWindow = true;
        else if( pWindow->GetType() == RSC_TOOLBOX )
            bToolbox = true;
        else if( pWindow->IsDialog() )
            bDialog = true;
        else
            bUnknown = true;
#endif

        ::std::vector< Window* >::iterator insertionPos = mTaskPanes.end();
        for ( ::std::vector< Window* >::iterator p = mTaskPanes.begin();
              p != mTaskPanes.end();
              ++p
            )
        {
            if ( *p == pWindow )
                // avoid duplicates
                return;

            // If the new window is the child of an existing pane window, or vice versa,
            // ensure that in our pane list, *first* the child window appears, *then*
            // the ancestor window.
            // This is necessary for HandleKeyEvent: There, the list is traveled from the
            // beginning, until the first window is found which has the ChildPathFocus. Now
            // if this would be the ancestor window of another pane window, this would fudge
            // the result
            // 2004-09-27 - fs@openoffice.org, while fixing #i33573#, which included replacing
            // the original fix for #98916# with this one here.
            if ( pWindow->IsWindowOrChild( *p ) )
            {
                insertionPos = p + 1;
                break;
            }
            if ( (*p)->IsWindowOrChild( pWindow ) )
            {
                insertionPos = p;
                break;
            }
        }

        mTaskPanes.insert( insertionPos, pWindow );
        pWindow->ImplIsInTaskPaneList( sal_True );
    }
}

// --------------------------------------------------

void TaskPaneList::RemoveWindow( Window *pWindow )
{
    ::std::vector< Window* >::iterator p;
    p = ::std::find( mTaskPanes.begin(), mTaskPanes.end(), pWindow );
    if( p != mTaskPanes.end() )
    {
        mTaskPanes.erase( p );
        pWindow->ImplIsInTaskPaneList( sal_False );
    }
}

// --------------------------------------------------

sal_Bool TaskPaneList::IsInList( Window *pWindow )
{
    ::std::vector< Window* >::iterator p;
    p = ::std::find( mTaskPanes.begin(), mTaskPanes.end(), pWindow );
    if( p != mTaskPanes.end() )
        return sal_True;
    else
        return sal_False;
}

// --------------------------------------------------

sal_Bool TaskPaneList::HandleKeyEvent( KeyEvent aKeyEvent )
{

    // F6 cycles through everything and works always

    // MAV, #i104204#
    // The old design was the following one:
    // < Ctrl-TAB cycles through Menubar, Toolbars and Floatingwindows only and is
    // < only active if one of those items has the focus
    //
    // Since the design of Ctrl-Tab looks to be inconsistent ( non-modal dialogs are not reachable
    // and the shortcut conflicts with tab-control shortcut ), it is no more supported
    sal_Bool bSplitterOnly = sal_False;
    sal_Bool bFocusInList = sal_False;
    KeyCode aKeyCode = aKeyEvent.GetKeyCode();
    sal_Bool bForward = !aKeyCode.IsShift();
    if( aKeyCode.GetCode() == KEY_F6 && ! aKeyCode.IsMod2() ) // F6
    {
        bSplitterOnly = aKeyCode.IsMod1() && aKeyCode.IsShift();

        // is the focus in the list ?
        ::std::vector< Window* >::iterator p = mTaskPanes.begin();
        while( p != mTaskPanes.end() )
        {
            Window *pWin = *p;
            if( pWin->HasChildPathFocus( sal_True ) )
            {
                bFocusInList = sal_True;

                // Ctrl-F6 goes directly to the document
                if( !pWin->IsDialog() && aKeyCode.IsMod1() && !aKeyCode.IsShift() )
                {
                    pWin->GrabFocusToDocument();
                    return sal_True;
                }

                // activate next task pane
                Window *pNextWin = NULL;

                if( bSplitterOnly )
                    pNextWin = FindNextSplitter( *p, sal_True );
                else
                    pNextWin = FindNextFloat( *p, bForward );

                if( pNextWin != pWin )
                {
                    ImplGetSVData()->maWinData.mbNoSaveFocus = sal_True;
                    ImplTaskPaneListGrabFocus( pNextWin );
                    ImplGetSVData()->maWinData.mbNoSaveFocus = sal_False;
                }
                else
                {
                    // forward key if no splitter found
                    if( bSplitterOnly )
                        return sal_False;

                    // we did not find another taskpane, so
                    // put focus back into document
                    pWin->GrabFocusToDocument();
                }

                return sal_True;
            }
            else
                p++;
        }

        // the focus is not in the list: activate first float if F6 was pressed
        if( !bFocusInList )
        {
            Window *pWin;
            if( bSplitterOnly )
                pWin = FindNextSplitter( NULL, sal_True );
            else
                pWin = FindNextFloat( NULL, bForward );
            if( pWin )
            {
                ImplTaskPaneListGrabFocus( pWin );
                return sal_True;
            }
        }
    }

    return sal_False;
}

// --------------------------------------------------

//  returns next valid pane
Window* TaskPaneList::FindNextPane( Window *pWindow, sal_Bool bForward )
{
    if( bForward )
        ::std::stable_sort( mTaskPanes.begin(), mTaskPanes.end(), LTRSort() );
    else
        ::std::stable_sort( mTaskPanes.begin(), mTaskPanes.end(), LTRSortBackward() );

    ::std::vector< Window* >::iterator p = mTaskPanes.begin();
    while( p != mTaskPanes.end() )
    {
        if( *p == pWindow )
        {
            unsigned n = mTaskPanes.size();
            while( --n )
            {
                if( ++p == mTaskPanes.end() )
                    p = mTaskPanes.begin();
                if( (*p)->IsReallyVisible() && !(*p)->IsDialog() && !(*p)->ImplIsSplitter() )
                {
                    pWindow = *p;
                    break;
                }
            }
            break;
        }
        else
            ++p;
    }

    return pWindow;
}

// --------------------------------------------------

// returns next splitter
Window* TaskPaneList::FindNextSplitter( Window *pWindow, sal_Bool bForward )
{
    if( bForward )
        ::std::stable_sort( mTaskPanes.begin(), mTaskPanes.end(), LTRSort() );
    else
        ::std::stable_sort( mTaskPanes.begin(), mTaskPanes.end(), LTRSortBackward() );

    ::std::vector< Window* >::iterator p = mTaskPanes.begin();
    while( p != mTaskPanes.end() )
    {
        if( !pWindow || *p == pWindow )
        {
            unsigned n = mTaskPanes.size();
            while( --n )
            {
                if( pWindow )   // increment before test
                    ++p;
                if( p == mTaskPanes.end() )
                    p = mTaskPanes.begin();
                if( (*p)->ImplIsSplitter() && (*p)->IsReallyVisible() && !(*p)->IsDialog() && (*p)->GetParent()->HasChildPathFocus() )
                {
                    pWindow = *p;
                    break;
                }
                if( !pWindow )  // increment after test, otherwise first element is skipped
                    ++p;
            }
            break;
        }
        else
            ++p;
    }

    return pWindow;
}

// --------------------------------------------------

// returns first valid item (regardless of type) if pWindow==0, otherwise returns next valid float
Window* TaskPaneList::FindNextFloat( Window *pWindow, sal_Bool bForward )
{
    if( bForward )
        ::std::stable_sort( mTaskPanes.begin(), mTaskPanes.end(), LTRSort() );
    else
        ::std::stable_sort( mTaskPanes.begin(), mTaskPanes.end(), LTRSortBackward() );

    ::std::vector< Window* >::iterator p = mTaskPanes.begin();
    while( p != mTaskPanes.end() )
    {
        if( !pWindow || *p == pWindow )
        {
            while( p != mTaskPanes.end() )
            {
                if( pWindow )   // increment before test
                    ++p;
                if( p == mTaskPanes.end() )
                    break; // do not wrap, send focus back to document at end of list
                /* #i83908# do not use the menubar if it is native and invisible
                   this relies on MenuBar::ImplCreate setting the height of the menubar
                   to 0 in this case
                */
                if( (*p)->IsReallyVisible() && !(*p)->ImplIsSplitter() &&
                    ( (*p)->GetType() != WINDOW_MENUBARWINDOW || (*p)->GetSizePixel().Height() > 0 )
                    )
                {
                    pWindow = *p;
                    break;
                }
                if( !pWindow )  // increment after test, otherwise first element is skipped
                    ++p;
            }
            break;
        }
        else
            ++p;
    }

    return pWindow;
}

// --------------------------------------------------

