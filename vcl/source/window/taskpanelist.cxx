/*************************************************************************
 *
 *  $RCSfile: taskpanelist.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ssa $ $Date: 2002-02-22 09:10:27 $
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

// compares window pos left-to-right
bool LTR( const Window*& w1, const Window*& w2)
{
    Point pos1, pos2;
    if( w1->GetType() == RSC_DOCKINGWINDOW )
        pos1 = ((DockingWindow*)w1)->GetPosPixel();
    else
        pos1 = w1->GetPosPixel();
    if( w2->GetType() == RSC_DOCKINGWINDOW )
        pos2 = ((DockingWindow*)w2)->GetPosPixel();
    else
        pos2 = w2->GetPosPixel();

    if( pos1.X() == pos2.X() )
        return ( pos1.Y() < pos2.Y() );
    else
        return ( pos1.X() < pos2.X() );
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
    if( pWindow )
    {
        if( pWindow->GetType() == RSC_DOCKINGWINDOW )
            bDockingWindow = true;
        else if( pWindow->GetType() == RSC_TOOLBOX )
            bToolbox = true;

        bool bFound = false;
        ::std::list< Window* >::iterator p = mTaskPanes.begin();
        while( p != mTaskPanes.end() )
        {
            if( *p++ == pWindow )
            {
                bFound = true;
                break;
            }
        }

        // avoid duplicates
        if( !bFound )
            mTaskPanes.push_back( pWindow );
    }
}

// --------------------------------------------------

void TaskPaneList::RemoveWindow( Window *pWindow )
{
    mTaskPanes.remove( pWindow );
}

// --------------------------------------------------

BOOL TaskPaneList::HandleKeyEvent( KeyEvent aKeyEvent )
{
    KeyCode aKeyCode = aKeyEvent.GetKeyCode();
    if( (aKeyCode.IsMod1() && aKeyCode.GetCode() == KEY_TAB)    // Ctrl-TAB
        || aKeyCode.GetCode() == KEY_F6                         // F6
        )
    {
        // is the focus in the list ?
        BOOL bHasFocus = FALSE;
        ::std::list< Window* >::iterator p = mTaskPanes.begin();
        while( p != mTaskPanes.end() )
        {
            Window *pWin = *p;
            if( (*p)->HasChildPathFocus( TRUE ) )
            {
                // activate next task pane
                Window *pNextWin = FindNextPane( *p );
                if( pNextWin != *p )
                {
                    ImplGetSVData()->maWinData.mbNoSaveFocus = TRUE;
                    pNextWin->GrabFocus();
                    ImplGetSVData()->maWinData.mbNoSaveFocus = FALSE;
                }
                return TRUE;
            }
            else
                p++;
        }
    }

    return FALSE;
}

// --------------------------------------------------

Window* TaskPaneList::FindNextPane( Window *pWindow )
{
    mTaskPanes.sort(LTR);

    ::std::list< Window* >::iterator p = mTaskPanes.begin();
    while( p != mTaskPanes.end() )
    {
        if( *p == pWindow )
        {
            unsigned n = mTaskPanes.size();
            while( --n )
            {
                if( ++p == mTaskPanes.end() )
                    p = mTaskPanes.begin();
                if( (*p)->IsVisible() )
                    return *p;
            }
            break;
        }
        else
            ++p;
    }

    return pWindow; // nothing found
}

// --------------------------------------------------

