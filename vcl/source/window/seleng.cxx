/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <vcl/window.hxx>
#include <vcl/seleng.hxx>
#include <tools/debug.hxx>

FunctionSet::~FunctionSet()
{
}


inline bool SelectionEngine::ShouldDeselect( bool bModifierKey1 ) const
{
//  return !( eSelMode == MULTIPLE_SELECTION && bModifierKey1 );
    return eSelMode != MULTIPLE_SELECTION || !bModifierKey1;
}


// TODO: throw out FunctionSet::SelectAtPoint


SelectionEngine::SelectionEngine( Window* pWindow, FunctionSet* pFuncSet,
                                  sal_uLong nAutoRepeatInterval ) :
    pWin( pWindow ),
    nUpdateInterval( nAutoRepeatInterval )
{
    eSelMode = SINGLE_SELECTION;
    pFunctionSet = pFuncSet;
    nFlags = SELENG_EXPANDONMOVE;
    nLockedMods = 0;

    aWTimer.SetTimeoutHdl( LINK( this, SelectionEngine, ImpWatchDog ) );
    aWTimer.SetTimeout( nUpdateInterval );
}


SelectionEngine::~SelectionEngine()
{
    aWTimer.Stop();
}


IMPL_LINK_NOARG(SelectionEngine, ImpWatchDog)
{
    if ( !aArea.IsInside( aLastMove.GetPosPixel() ) )
        SelMouseMove( aLastMove );
    return 0;
}


void SelectionEngine::SetSelectionMode( SelectionMode eMode )
{
    eSelMode = eMode;
}


void SelectionEngine::CursorPosChanging( bool bShift, bool bMod1 )
{
    if ( !pFunctionSet )
        return;

    if ( bShift && eSelMode != SINGLE_SELECTION )
    {
        if ( IsAddMode() )
        {
            if ( !(nFlags & SELENG_HAS_ANCH) )
            {
                pFunctionSet->CreateAnchor();
                nFlags |= SELENG_HAS_ANCH;
            }
        }
        else
        {
            if ( !(nFlags & SELENG_HAS_ANCH) )
            {
                if( ShouldDeselect( bMod1 ) )
                    pFunctionSet->DeselectAll();
                pFunctionSet->CreateAnchor();
                nFlags |= SELENG_HAS_ANCH;
            }
        }
    }
    else
    {
        if ( IsAddMode() )
        {
            if ( nFlags & SELENG_HAS_ANCH )
            {
                // pFunctionSet->CreateCursor();
                pFunctionSet->DestroyAnchor();
                nFlags &= (~SELENG_HAS_ANCH);
            }
        }
        else
        {
            if( ShouldDeselect( bMod1 ) )
                pFunctionSet->DeselectAll();
            else
                pFunctionSet->DestroyAnchor();
            nFlags &= (~SELENG_HAS_ANCH);
        }
    }
}


bool SelectionEngine::SelMouseButtonDown( const MouseEvent& rMEvt )
{
    nFlags &= (~SELENG_CMDEVT);
    if ( !pFunctionSet || !pWin || rMEvt.GetClicks() > 1 || rMEvt.IsRight() )
        return false;

    sal_uInt16 nModifier = rMEvt.GetModifier() | nLockedMods;
    if ( nModifier & KEY_MOD2 )
        return false;
    // in SingleSelection: filter Control-Key,
    // so that a D&D can be also started with a Ctrl-Click
    if ( nModifier == KEY_MOD1 && eSelMode == SINGLE_SELECTION )
        nModifier = 0;

    Point aPos = rMEvt.GetPosPixel();
    aLastMove = rMEvt;

    if( !rMEvt.IsRight() )
    {
        pWin->CaptureMouse();
        nFlags |= SELENG_IN_SEL;
    }
    else
    {
        nModifier = 0;
    }

    switch ( nModifier )
    {
        case 0:     // KEY_NO_KEY
        {
            bool bSelAtPoint = pFunctionSet->IsSelectionAtPoint( aPos );
            nFlags &= (~SELENG_IN_ADD);
            if ( (nFlags & SELENG_DRG_ENAB) && bSelAtPoint )
            {
                nFlags |= SELENG_WAIT_UPEVT;
                nFlags &= ~(SELENG_IN_SEL);
                pWin->ReleaseMouse();
                return true;  // wait for STARTDRAG-Command-Event
            }
            if ( eSelMode != SINGLE_SELECTION )
            {
                if( !IsAddMode() )
                    pFunctionSet->DeselectAll();
                else
                    pFunctionSet->DestroyAnchor();
                   nFlags &= (~SELENG_HAS_ANCH); // bHasAnchor = false;
            }
            pFunctionSet->SetCursorAtPoint( aPos );
            // special case Single-Selection, to enable simple Select+Drag
            if (eSelMode == SINGLE_SELECTION && (nFlags & SELENG_DRG_ENAB))
                nFlags |= SELENG_WAIT_UPEVT;
            return true;
        }

        case KEY_SHIFT:
            if ( eSelMode == SINGLE_SELECTION )
            {
                pWin->ReleaseMouse();
                nFlags &= (~SELENG_IN_SEL);
                return false;
            }
            if ( nFlags & SELENG_ADD_ALW )
                nFlags |= SELENG_IN_ADD;
            else
                nFlags &= (~SELENG_IN_ADD);

            if( !(nFlags & SELENG_HAS_ANCH) )
            {
                if ( !(nFlags & SELENG_IN_ADD) )
                    pFunctionSet->DeselectAll();
                pFunctionSet->CreateAnchor();
                nFlags |= SELENG_HAS_ANCH;
            }
            pFunctionSet->SetCursorAtPoint( aPos );
            return true;

        case KEY_MOD1:
            // allow Control only for Multi-Select
            if ( eSelMode != MULTIPLE_SELECTION )
            {
                nFlags &= (~SELENG_IN_SEL);
                pWin->ReleaseMouse();
                return true;  // skip Mouse-Click
            }
            if ( nFlags & SELENG_HAS_ANCH )
            {
                // pFunctionSet->CreateCursor();
                pFunctionSet->DestroyAnchor();
                nFlags &= (~SELENG_HAS_ANCH);
            }
            if ( pFunctionSet->IsSelectionAtPoint( aPos ) )
            {
                pFunctionSet->DeselectAtPoint( aPos );
                pFunctionSet->SetCursorAtPoint( aPos, true );
            }
            else
            {
                pFunctionSet->SetCursorAtPoint( aPos );
            }
            return true;

        case KEY_SHIFT + KEY_MOD1:
            if ( eSelMode != MULTIPLE_SELECTION )
            {
                pWin->ReleaseMouse();
                nFlags &= (~SELENG_IN_SEL);
                return false;
            }
            nFlags |= SELENG_IN_ADD; //bIsInAddMode = true;
            if ( !(nFlags & SELENG_HAS_ANCH) )
            {
                pFunctionSet->CreateAnchor();
                nFlags |= SELENG_HAS_ANCH;
            }
            pFunctionSet->SetCursorAtPoint( aPos );
            return true;
    }

    return false;
}


bool SelectionEngine::SelMouseButtonUp( const MouseEvent& rMEvt )
{
    aWTimer.Stop();
    if( !pFunctionSet || !pWin )
    {
        nFlags &= ~(SELENG_CMDEVT | SELENG_WAIT_UPEVT | SELENG_IN_SEL);
        return false;
    }

    if( !rMEvt.IsRight() )
    {
       pWin->ReleaseMouse();
    }

    if( (nFlags & SELENG_WAIT_UPEVT) && !(nFlags & SELENG_CMDEVT) &&
        eSelMode != SINGLE_SELECTION)
    {
        // MouseButtonDown in Sel but no CommandEvent yet
        // ==> deselektieren
        sal_uInt16 nModifier = aLastMove.GetModifier() | nLockedMods;
        if( nModifier == KEY_MOD1 || IsAlwaysAdding() )
        {
            if( !(nModifier & KEY_SHIFT) )
            {
                pFunctionSet->DestroyAnchor();
                nFlags &= (~SELENG_HAS_ANCH); // uncheck anchor
            }
            pFunctionSet->DeselectAtPoint( aLastMove.GetPosPixel() );
            nFlags &= (~SELENG_HAS_ANCH); // uncheck anchor
            pFunctionSet->SetCursorAtPoint( aLastMove.GetPosPixel(), true );
        }
        else
        {
            pFunctionSet->DeselectAll();
            nFlags &= (~SELENG_HAS_ANCH); // uncheck anchor
            pFunctionSet->SetCursorAtPoint( aLastMove.GetPosPixel() );
        }
    }

    nFlags &= ~(SELENG_CMDEVT | SELENG_WAIT_UPEVT | SELENG_IN_SEL);
    return true;
}


bool SelectionEngine::SelMouseMove( const MouseEvent& rMEvt )
{

    if ( !pFunctionSet || !(nFlags & SELENG_IN_SEL) ||
         (nFlags & (SELENG_CMDEVT | SELENG_WAIT_UPEVT)) )
        return false;

    if( !(nFlags & SELENG_EXPANDONMOVE) )
        return false; // wait for DragEvent!

    aLastMove = rMEvt;
    // if the mouse is outside the area, the frequency of
    // SetCursorAtPoint() is only set by the Timer
    if( aWTimer.IsActive() && !aArea.IsInside( rMEvt.GetPosPixel() ))
        return true;

    aWTimer.SetTimeout( nUpdateInterval );
    aWTimer.Start();
    if ( eSelMode != SINGLE_SELECTION )
    {
        if ( !(nFlags & SELENG_HAS_ANCH) )
        {
            pFunctionSet->CreateAnchor();
            nFlags |= SELENG_HAS_ANCH;
        }
    }

    pFunctionSet->SetCursorAtPoint( rMEvt.GetPosPixel() );

    return true;
}


void SelectionEngine::SetWindow( Window* pNewWin )
{
    if( pNewWin != pWin )
    {
        if ( pWin && (nFlags & SELENG_IN_SEL) )
            pWin->ReleaseMouse();
        pWin = pNewWin;
        if ( pWin && ( nFlags & SELENG_IN_SEL ) )
            pWin->CaptureMouse();
    }
}


void SelectionEngine::Reset()
{
    aWTimer.Stop();
    if ( nFlags & SELENG_IN_SEL )
        pWin->ReleaseMouse();
    nFlags &= ~(SELENG_HAS_ANCH | SELENG_IN_SEL);
    nLockedMods = 0;
}


void SelectionEngine::Command( const CommandEvent& rCEvt )
{
    // Timer aWTimer is active during enlarging a selection
    if ( !pFunctionSet || !pWin || aWTimer.IsActive() )
        return;
    aWTimer.Stop();
    nFlags |= SELENG_CMDEVT;
    if ( rCEvt.GetCommand() == COMMAND_STARTDRAG )
    {
        if ( nFlags & SELENG_DRG_ENAB )
        {
            DBG_ASSERT( rCEvt.IsMouseEvent(), "STARTDRAG: Not a MouseEvent" );
            if ( pFunctionSet->IsSelectionAtPoint( rCEvt.GetMousePosPixel() ) )
            {
                aLastMove = MouseEvent( rCEvt.GetMousePosPixel(),
                               aLastMove.GetClicks(), aLastMove.GetMode(),
                               aLastMove.GetButtons(), aLastMove.GetModifier() );
                pFunctionSet->BeginDrag();
                nFlags &= ~(SELENG_CMDEVT|SELENG_WAIT_UPEVT|SELENG_IN_SEL);
            }
            else
                nFlags &= ~SELENG_CMDEVT;
        }
        else
            nFlags &= ~SELENG_CMDEVT;
    }
}

void SelectionEngine::SetUpdateInterval( sal_uLong nInterval )
{
    if (nInterval < SELENG_AUTOREPEAT_INTERVAL_MIN)
        // Set a lower threshold.  On Windows, setting this value too low
        // would cause selection to get updated indefinitely.
        nInterval = SELENG_AUTOREPEAT_INTERVAL_MIN;

    if (nUpdateInterval == nInterval)
        // no update needed.
        return;

    if (aWTimer.IsActive())
    {
        // reset the timer right away on interval change.
        aWTimer.Stop();
        aWTimer.SetTimeout(nInterval);
        aWTimer.Start();
    }
    else
        aWTimer.SetTimeout(nInterval);

    nUpdateInterval = nInterval;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
