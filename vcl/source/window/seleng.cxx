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

#include <vcl/commandevent.hxx>
#include <vcl/window.hxx>
#include <vcl/seleng.hxx>
#include <comphelper/lok.hxx>
#include <sal/log.hxx>

FunctionSet::~FunctionSet()
{
}

inline bool SelectionEngine::ShouldDeselect( bool bModifierKey1 ) const
{
    return eSelMode != SelectionMode::Multiple || !bModifierKey1;
}

// TODO: throw out FunctionSet::SelectAtPoint

SelectionEngine::SelectionEngine( vcl::Window* pWindow, FunctionSet* pFuncSet ) :
    pWin( pWindow ),
    nUpdateInterval( SELENG_AUTOREPEAT_INTERVAL )
{
    eSelMode = SelectionMode::Single;
    pFunctionSet = pFuncSet;
    nFlags = SelectionEngineFlags::EXPANDONMOVE;
    nLockedMods = 0;

    aWTimer.SetInvokeHandler( LINK( this, SelectionEngine, ImpWatchDog ) );
    aWTimer.SetTimeout( nUpdateInterval );
    aWTimer.SetDebugName( "vcl::SelectionEngine aWTimer" );
}

SelectionEngine::~SelectionEngine()
{
    aWTimer.Stop();
}

IMPL_LINK_NOARG(SelectionEngine, ImpWatchDog, Timer *, void)
{
    if ( !aArea.IsInside( aLastMove.GetPosPixel() ) )
        SelMouseMove( aLastMove );
}

void SelectionEngine::SetSelectionMode( SelectionMode eMode )
{
    eSelMode = eMode;
}

void SelectionEngine::CursorPosChanging( bool bShift, bool bMod1 )
{
    if ( !pFunctionSet )
        return;

    if ( bShift && eSelMode != SelectionMode::Single )
    {
        if ( IsAddMode() )
        {
            if ( !(nFlags & SelectionEngineFlags::HAS_ANCH) )
            {
                pFunctionSet->CreateAnchor();
                nFlags |= SelectionEngineFlags::HAS_ANCH;
            }
        }
        else
        {
            if ( !(nFlags & SelectionEngineFlags::HAS_ANCH) )
            {
                if( ShouldDeselect( bMod1 ) )
                    pFunctionSet->DeselectAll();
                pFunctionSet->CreateAnchor();
                nFlags |= SelectionEngineFlags::HAS_ANCH;
            }
        }
    }
    else
    {
        if ( IsAddMode() )
        {
            if ( nFlags & SelectionEngineFlags::HAS_ANCH )
            {
                // pFunctionSet->CreateCursor();
                pFunctionSet->DestroyAnchor();
                nFlags &= ~SelectionEngineFlags::HAS_ANCH;
            }
        }
        else
        {
            if( ShouldDeselect( bMod1 ) )
                pFunctionSet->DeselectAll();
            else
                pFunctionSet->DestroyAnchor();
            nFlags &= ~SelectionEngineFlags::HAS_ANCH;
        }
    }
}

bool SelectionEngine::SelMouseButtonDown( const MouseEvent& rMEvt )
{
    nFlags &= ~SelectionEngineFlags::CMDEVT;
    if ( !pFunctionSet || !pWin || rMEvt.GetClicks() > 1 || rMEvt.IsRight() )
        return false;

    sal_uInt16 nModifier = rMEvt.GetModifier() | nLockedMods;
    if ( nModifier & KEY_MOD2 )
        return false;
    // in SingleSelection: filter Control-Key,
    // so that a D&D can be also started with a Ctrl-Click
    if ( nModifier == KEY_MOD1 && eSelMode == SelectionMode::Single )
        nModifier = 0;

    Point aPos = rMEvt.GetPosPixel();
    aLastMove = rMEvt;

    if( !rMEvt.IsRight() )
    {
        pWin->CaptureMouse();
        nFlags |= SelectionEngineFlags::IN_SEL;
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
            nFlags &= ~SelectionEngineFlags::IN_ADD;
            if ( (nFlags & SelectionEngineFlags::DRG_ENAB) && bSelAtPoint )
            {
                nFlags |= SelectionEngineFlags::WAIT_UPEVT;
                nFlags &= ~SelectionEngineFlags::IN_SEL;
                pWin->ReleaseMouse();
                return true;  // wait for STARTDRAG-Command-Event
            }
            if ( eSelMode != SelectionMode::Single )
            {
                if( !IsAddMode() )
                    pFunctionSet->DeselectAll();
                else
                    pFunctionSet->DestroyAnchor();
                nFlags &= ~SelectionEngineFlags::HAS_ANCH; // bHasAnchor = false;
            }
            pFunctionSet->SetCursorAtPoint( aPos );
            // special case Single-Selection, to enable simple Select+Drag
            if (eSelMode == SelectionMode::Single && (nFlags & SelectionEngineFlags::DRG_ENAB))
                nFlags |= SelectionEngineFlags::WAIT_UPEVT;
            return true;
        }

        case KEY_SHIFT:
            if ( eSelMode == SelectionMode::Single )
            {
                pWin->ReleaseMouse();
                nFlags &= ~SelectionEngineFlags::IN_SEL;
                return false;
            }
            if ( nFlags & SelectionEngineFlags::ADD_ALW )
                nFlags |= SelectionEngineFlags::IN_ADD;
            else
                nFlags &= ~SelectionEngineFlags::IN_ADD;

            if( !(nFlags & SelectionEngineFlags::HAS_ANCH) )
            {
                if ( !(nFlags & SelectionEngineFlags::IN_ADD) )
                    pFunctionSet->DeselectAll();
                pFunctionSet->CreateAnchor();
                nFlags |= SelectionEngineFlags::HAS_ANCH;
            }
            pFunctionSet->SetCursorAtPoint( aPos );
            return true;

        case KEY_MOD1:
            // allow Control only for Multi-Select
            if ( eSelMode != SelectionMode::Multiple )
            {
                nFlags &= ~SelectionEngineFlags::IN_SEL;
                pWin->ReleaseMouse();
                return true;  // skip Mouse-Click
            }
            if ( nFlags & SelectionEngineFlags::HAS_ANCH )
            {
                // pFunctionSet->CreateCursor();
                pFunctionSet->DestroyAnchor();
                nFlags &= ~SelectionEngineFlags::HAS_ANCH;
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
            if ( eSelMode != SelectionMode::Multiple )
            {
                pWin->ReleaseMouse();
                nFlags &= ~SelectionEngineFlags::IN_SEL;
                return false;
            }
            nFlags |= SelectionEngineFlags::IN_ADD; //bIsInAddMode = true;
            if ( !(nFlags & SelectionEngineFlags::HAS_ANCH) )
            {
                pFunctionSet->CreateAnchor();
                nFlags |= SelectionEngineFlags::HAS_ANCH;
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
        const SelectionEngineFlags nMask = SelectionEngineFlags::CMDEVT | SelectionEngineFlags::WAIT_UPEVT | SelectionEngineFlags::IN_SEL;
        nFlags &= ~nMask;
        return false;
    }

    if( !rMEvt.IsRight() )
    {
       ReleaseMouse();
    }

    if( (nFlags & SelectionEngineFlags::WAIT_UPEVT) && !(nFlags & SelectionEngineFlags::CMDEVT) &&
        eSelMode != SelectionMode::Single)
    {
        // MouseButtonDown in Sel but no CommandEvent yet
        // ==> deselect
        sal_uInt16 nModifier = aLastMove.GetModifier() | nLockedMods;
        if( nModifier == KEY_MOD1 || IsAlwaysAdding() )
        {
            if( !(nModifier & KEY_SHIFT) )
            {
                pFunctionSet->DestroyAnchor();
                nFlags &= ~SelectionEngineFlags::HAS_ANCH; // uncheck anchor
            }
            pFunctionSet->DeselectAtPoint( aLastMove.GetPosPixel() );
            nFlags &= ~SelectionEngineFlags::HAS_ANCH; // uncheck anchor
            pFunctionSet->SetCursorAtPoint( aLastMove.GetPosPixel(), true );
        }
        else
        {
            pFunctionSet->DeselectAll();
            nFlags &= ~SelectionEngineFlags::HAS_ANCH; // uncheck anchor
            pFunctionSet->SetCursorAtPoint( aLastMove.GetPosPixel() );
        }
    }

    const SelectionEngineFlags nMask = SelectionEngineFlags::CMDEVT | SelectionEngineFlags::WAIT_UPEVT | SelectionEngineFlags::IN_SEL;
    nFlags &= ~nMask;
    return true;
}

void SelectionEngine::ReleaseMouse()
{
    if (!pWin || !pWin->IsMouseCaptured())
        return;
    pWin->ReleaseMouse();
}

bool SelectionEngine::SelMouseMove( const MouseEvent& rMEvt )
{

    if ( !pFunctionSet || !(nFlags & SelectionEngineFlags::IN_SEL) ||
         (nFlags & (SelectionEngineFlags::CMDEVT | SelectionEngineFlags::WAIT_UPEVT)) )
        return false;

    if( !(nFlags & SelectionEngineFlags::EXPANDONMOVE) )
        return false; // wait for DragEvent!

    aLastMove = rMEvt;
    // if the mouse is outside the area, the frequency of
    // SetCursorAtPoint() is only set by the Timer
    if( aWTimer.IsActive() && !aArea.IsInside( rMEvt.GetPosPixel() ))
        return true;

    aWTimer.SetTimeout( nUpdateInterval );
    if (!comphelper::LibreOfficeKit::isActive())
        // Generating fake mouse moves does not work with LOK.
        aWTimer.Start();
    if ( eSelMode != SelectionMode::Single )
    {
        if ( !(nFlags & SelectionEngineFlags::HAS_ANCH) )
        {
            pFunctionSet->CreateAnchor();
            nFlags |= SelectionEngineFlags::HAS_ANCH;
        }
    }

    pFunctionSet->SetCursorAtPoint( rMEvt.GetPosPixel() );

    return true;
}

void SelectionEngine::SetWindow( vcl::Window* pNewWin )
{
    if( pNewWin != pWin )
    {
        if ( pWin && (nFlags & SelectionEngineFlags::IN_SEL) )
            pWin->ReleaseMouse();
        pWin = pNewWin;
        if ( pWin && ( nFlags & SelectionEngineFlags::IN_SEL ) )
            pWin->CaptureMouse();
    }
}

void SelectionEngine::Reset()
{
    aWTimer.Stop();
    if ( nFlags & SelectionEngineFlags::IN_SEL )
        pWin->ReleaseMouse();
    nFlags &= ~SelectionEngineFlags(SelectionEngineFlags::HAS_ANCH | SelectionEngineFlags::IN_SEL);
    nLockedMods = 0;
}

void SelectionEngine::Command( const CommandEvent& rCEvt )
{
    // Timer aWTimer is active during enlarging a selection
    if ( !pFunctionSet || !pWin || aWTimer.IsActive() )
        return;
    aWTimer.Stop();
    if ( rCEvt.GetCommand() == CommandEventId::StartDrag )
    {
        nFlags |= SelectionEngineFlags::CMDEVT;
        if ( nFlags & SelectionEngineFlags::DRG_ENAB )
        {
            SAL_WARN_IF( !rCEvt.IsMouseEvent(), "vcl", "STARTDRAG: Not a MouseEvent" );
            if ( pFunctionSet->IsSelectionAtPoint( rCEvt.GetMousePosPixel() ) )
            {
                aLastMove = MouseEvent( rCEvt.GetMousePosPixel(),
                               aLastMove.GetClicks(), aLastMove.GetMode(),
                               aLastMove.GetButtons(), aLastMove.GetModifier() );
                pFunctionSet->BeginDrag();
                const SelectionEngineFlags nMask = SelectionEngineFlags::CMDEVT|SelectionEngineFlags::WAIT_UPEVT|SelectionEngineFlags::IN_SEL;
                nFlags &= ~nMask;
            }
            else
                nFlags &= ~SelectionEngineFlags::CMDEVT;
        }
        else
            nFlags &= ~SelectionEngineFlags::CMDEVT;
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
