/*************************************************************************
 *
 *  $RCSfile: taskpanelist.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: ssa $ $Date: 2002-05-23 09:43:15 $
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
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
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
#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _RCID_H
#include <rcid.h>
#endif
#ifndef _SV_DOCKWIN_HXX
#include <dockwin.hxx>
#endif

#include <taskpanelist.hxx>
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
    bool bDockingWindow=false;
    bool bToolbox=false;
    bool bDialog=false;
    bool bUnknown=false;

    if( pWindow )
    {
        if( pWindow->GetType() == RSC_DOCKINGWINDOW )
            bDockingWindow = true;
        else if( pWindow->GetType() == RSC_TOOLBOX )
            bToolbox = true;
        else if( pWindow->IsDialog() )
            bDialog = true;
        else
            bUnknown = true;

        ::std::vector< Window* >::iterator p;
        p = ::std::find( mTaskPanes.begin(), mTaskPanes.end(), pWindow );

        // avoid duplicates
        if( p == mTaskPanes.end() ) // not found
            mTaskPanes.push_back( pWindow );
    }
}

// --------------------------------------------------

void TaskPaneList::RemoveWindow( Window *pWindow )
{
    ::std::vector< Window* >::iterator p;
    p = ::std::find( mTaskPanes.begin(), mTaskPanes.end(), pWindow );
    if( p != mTaskPanes.end() )
        mTaskPanes.erase( p );
}

// --------------------------------------------------

BOOL TaskPaneList::HandleKeyEvent( KeyEvent aKeyEvent )
{
    // F6 cycles through everything and works always
    // Ctrl-TAB cycles through Menubar, Toolbars and Floatingwindows only and is
    // only active if one of those items has the focus
    BOOL bF6 = FALSE;
    BOOL bSplitterOnly = FALSE;
    BOOL bFocusInList = FALSE;
    KeyCode aKeyCode = aKeyEvent.GetKeyCode();
    BOOL bForward = !aKeyCode.IsShift();
    if( ( (aKeyCode.IsMod1() || aKeyCode.IsMod2()) && aKeyCode.GetCode() == KEY_TAB )  // Ctrl-TAB or Alt-TAB
        || ( bF6 = ( aKeyCode.GetCode()) == KEY_F6 )    // F6
        )
    {
        bSplitterOnly = bF6 && aKeyCode.IsMod1() && aKeyCode.IsShift();

        // is the focus in the list ?
        BOOL bHasFocus = FALSE;
        ::std::vector< Window* >::iterator p = mTaskPanes.begin();
        while( p != mTaskPanes.end() )
        {
            Window *pWin = *p;
            if( (*p)->HasChildPathFocus( TRUE ) )
            {
                bFocusInList = TRUE;

                // Ctrl-TAB does not work in Dialogs
                if( !bF6 && (*p)->IsDialog() )
                    return FALSE;

                // activate next task pane
                Window *pNextWin = NULL;

                if( bSplitterOnly )
                    pNextWin = FindNextSplitter( *p, TRUE );
                else
                    pNextWin = bF6 ?  FindNextFloat( *p, bForward ) : FindNextPane( *p, bForward );
                if( pNextWin != pWin )
                {
                    ImplGetSVData()->maWinData.mbNoSaveFocus = TRUE;
                    ImplTaskPaneListGrabFocus( pNextWin );
                    ImplGetSVData()->maWinData.mbNoSaveFocus = FALSE;
                }
                else
                {
                    // forward key if no splitter found
                    if( bSplitterOnly )
                        return FALSE;

                    // we did not find another taskpane, so
                    // put focus back into document: use frame win of topmost parent
                    while( pWin )
                    {
                        if( !pWin->GetParent() )
                        {
                            pWin->ImplGetFrameWindow()->GetWindow( WINDOW_CLIENT )->GrabFocus();
                            break;
                        }
                        pWin = pWin->GetParent();
                    }
                }

                return TRUE;
            }
            else
                p++;
        }

        // the focus is not in the list: activate first float if F6 was pressed
        if( !bFocusInList && bF6 )
        {
            Window *pWin;
            if( bSplitterOnly )
                pWin = FindNextSplitter( NULL, TRUE );
            else
                pWin = FindNextFloat( NULL, bForward );
            if( pWin )
            {
                ImplTaskPaneListGrabFocus( pWin );
                return TRUE;
            }
        }
    }

    return FALSE;
}

// --------------------------------------------------

//  returns next valid pane
Window* TaskPaneList::FindNextPane( Window *pWindow, BOOL bForward )
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
Window* TaskPaneList::FindNextSplitter( Window *pWindow, BOOL bForward )
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
                if( (*p)->ImplIsSplitter() && (*p)->IsReallyVisible() && !(*p)->IsDialog() )
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
Window* TaskPaneList::FindNextFloat( Window *pWindow, BOOL bForward )
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
                if( (*p)->IsReallyVisible() && !(*p)->ImplIsSplitter() )
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

