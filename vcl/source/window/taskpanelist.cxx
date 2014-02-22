/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <tools/rcid.h>

#include <vcl/dockwin.hxx>
#include <vcl/taskpanelist.hxx>

#include <svdata.hxx>

#include <functional>
#include <algorithm>


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



static void ImplTaskPaneListGrabFocus( Window *pWindow, bool bForward )
{
    
    
    if( pWindow->ImplIsFloatingWindow() && pWindow->GetWindow( WINDOW_FIRSTCHILD ) )
        pWindow = pWindow->GetWindow( WINDOW_FIRSTCHILD );
    pWindow->ImplGrabFocus( GETFOCUS_F6 | (bForward ? GETFOCUS_FORWARD : GETFOCUS_BACKWARD));
}



TaskPaneList::TaskPaneList()
{
}

TaskPaneList::~TaskPaneList()
{
}



void TaskPaneList::AddWindow( Window *pWindow )
{
    if( pWindow )
    {
        ::std::vector< Window* >::iterator insertionPos = mTaskPanes.end();
        for ( ::std::vector< Window* >::iterator p = mTaskPanes.begin();
              p != mTaskPanes.end();
              ++p
            )
        {
            if ( *p == pWindow )
                
                return;

            
            
            
            
            
            
            
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
        pWindow->ImplIsInTaskPaneList( true );
    }
}



void TaskPaneList::RemoveWindow( Window *pWindow )
{
    ::std::vector< Window* >::iterator p;
    p = ::std::find( mTaskPanes.begin(), mTaskPanes.end(), pWindow );
    if( p != mTaskPanes.end() )
    {
        mTaskPanes.erase( p );
        pWindow->ImplIsInTaskPaneList( false );
    }
}



bool TaskPaneList::IsInList( Window *pWindow )
{
    ::std::vector< Window* >::iterator p;
    p = ::std::find( mTaskPanes.begin(), mTaskPanes.end(), pWindow );
    if( p != mTaskPanes.end() )
        return true;
    else
        return false;
}



bool TaskPaneList::HandleKeyEvent( KeyEvent aKeyEvent )
{

    

    
    
    
    
    //
    
    
    bool bSplitterOnly = false;
    bool bFocusInList = false;
    KeyCode aKeyCode = aKeyEvent.GetKeyCode();
    bool bForward = !aKeyCode.IsShift();
    if( aKeyCode.GetCode() == KEY_F6 && ! aKeyCode.IsMod2() ) 
    {
        bSplitterOnly = aKeyCode.IsMod1() && aKeyCode.IsShift();

        
        ::std::vector< Window* >::iterator p = mTaskPanes.begin();
        while( p != mTaskPanes.end() )
        {
            Window *pWin = *p;
            if( pWin->HasChildPathFocus( true ) )
            {
                bFocusInList = true;

                
                if( !pWin->IsDialog() && aKeyCode.IsMod1() && !aKeyCode.IsShift() )
                {
                    pWin->ImplGrabFocusToDocument( GETFOCUS_F6 );
                    return true;
                }

                
                Window *pNextWin = NULL;

                if( bSplitterOnly )
                    pNextWin = FindNextSplitter( *p, true );
                else
                    pNextWin = FindNextFloat( *p, bForward );

                if( pNextWin != pWin )
                {
                    ImplGetSVData()->maWinData.mbNoSaveFocus = true;
                    ImplTaskPaneListGrabFocus( pNextWin, bForward );
                    ImplGetSVData()->maWinData.mbNoSaveFocus = false;
                }
                else
                {
                    
                    if( bSplitterOnly )
                        return false;

                    
                    
                    pWin->ImplGrabFocusToDocument( GETFOCUS_F6 | (bForward ? GETFOCUS_FORWARD : GETFOCUS_BACKWARD));
                }

                return true;
            }
            else
                ++p;
        }

        
        if( !bFocusInList )
        {
            Window *pWin;
            if( bSplitterOnly )
                pWin = FindNextSplitter( NULL, true );
            else
                pWin = FindNextFloat( NULL, bForward );
            if( pWin )
            {
                ImplTaskPaneListGrabFocus( pWin, bForward );
                return true;
            }
        }
    }

    return false;
}




Window* TaskPaneList::FindNextSplitter( Window *pWindow, bool bForward )
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
                if( pWindow )   
                    ++p;
                if( p == mTaskPanes.end() )
                    p = mTaskPanes.begin();
                if( (*p)->ImplIsSplitter() && (*p)->IsReallyVisible() && !(*p)->IsDialog() && (*p)->GetParent()->HasChildPathFocus() )
                {
                    pWindow = *p;
                    break;
                }
                if( !pWindow )  
                    ++p;
            }
            break;
        }
        else
            ++p;
    }

    return pWindow;
}




Window* TaskPaneList::FindNextFloat( Window *pWindow, bool bForward )
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
                if( pWindow )   
                    ++p;
                if( p == mTaskPanes.end() )
                    break; 
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
                if( !pWindow )  
                    ++p;
            }
            break;
        }
        else
            ++p;
    }

    return pWindow;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
