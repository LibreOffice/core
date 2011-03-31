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

#include <tools/ref.hxx>
#include <vcl/window.hxx>
#include <vcl/seleng.hxx>
#include <tools/debug.hxx>




inline sal_Bool SelectionEngine::ShouldDeselect( sal_Bool bModifierKey1 ) const
{
//  return !( eSelMode == MULTIPLE_SELECTION && bModifierKey1 );
    return eSelMode != MULTIPLE_SELECTION || !bModifierKey1;
}


// TODO: FunctionSet::SelectAtPoint raus

/*************************************************************************
|*
|*    SelectionEngine::SelectionEngine()
|*
|*    Beschreibung      SELENG.SDW
|*    Ersterstellung    OV 10.10.94
|*    Letzte Aenderung  OV 10.10.94
|*
*************************************************************************/

SelectionEngine::SelectionEngine( Window* pWindow, FunctionSet* pFuncSet ) :
                    pWin( pWindow )
{
    eSelMode = SINGLE_SELECTION;
    pFunctionSet = pFuncSet;
    nFlags = SELENG_EXPANDONMOVE;
    nLockedMods = 0;

    aWTimer.SetTimeoutHdl( LINK( this, SelectionEngine, ImpWatchDog ) );
    aWTimer.SetTimeout( SELENG_AUTOREPEAT_INTERVAL );
}

/*************************************************************************
|*
|*    SelectionEngine::~SelectionEngine()
|*
|*    Beschreibung      SELENG.SDW
|*    Ersterstellung    OV 10.10.94
|*    Letzte Aenderung  OV 10.10.94
|*
*************************************************************************/

SelectionEngine::~SelectionEngine()
{
    aWTimer.Stop();
}

/*************************************************************************
|*
|*    SelectionEngine::ImpWatchDog()
|*
|*    Beschreibung      SELENG.SDW
|*    Ersterstellung    OV 10.10.94
|*    Letzte Aenderung  OV 10.10.94
|*
*************************************************************************/

IMPL_LINK( SelectionEngine, ImpWatchDog, Timer*, EMPTYARG )
{
    if ( !aArea.IsInside( aLastMove.GetPosPixel() ) )
        SelMouseMove( aLastMove );
    return 0;
}

/*************************************************************************
|*
|*    SelectionEngine::SetSelectionMode()
|*
|*    Beschreibung      SELENG.SDW
|*    Ersterstellung    OV 10.10.94
|*    Letzte Aenderung  OV 10.10.94
|*
*************************************************************************/

void SelectionEngine::SetSelectionMode( SelectionMode eMode )
{
    eSelMode = eMode;
}

/*************************************************************************
|*
|*    SelectionEngine::ActivateDragMode()
|*
|*    Beschreibung      SELENG.SDW
|*    Ersterstellung    OV 10.10.94
|*    Letzte Aenderung  OV 10.10.94
|*
*************************************************************************/

void SelectionEngine::ActivateDragMode()
{
    DBG_ERRORFILE( "SelectionEngine::ActivateDragMode not implemented" );
}

/*************************************************************************
|*
|*    SelectionEngine::CursorPosChanging()
|*
|*    Beschreibung      SELENG.SDW
|*    Ersterstellung    OV 10.10.94
|*    Letzte Aenderung  GT 2002-04-04
|*
*************************************************************************/

void SelectionEngine::CursorPosChanging( sal_Bool bShift, sal_Bool bMod1 )
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

/*************************************************************************
|*
|*    SelectionEngine::SelMouseButtonDown()
|*
|*    Beschreibung      SELENG.SDW
|*    Ersterstellung    OV 10.10.94
|*    Letzte Aenderung  OV 07.06.95
|*
*************************************************************************/

sal_Bool SelectionEngine::SelMouseButtonDown( const MouseEvent& rMEvt )
{
    nFlags &= (~SELENG_CMDEVT);
    if ( !pFunctionSet || !pWin || rMEvt.GetClicks() > 1 || rMEvt.IsRight() )
        return sal_False;

    sal_uInt16 nModifier = rMEvt.GetModifier() | nLockedMods;
    if ( nModifier & KEY_MOD2 )
        return sal_False;
    // in SingleSelection: Control-Taste filtern (damit auch
    // mit Ctrl-Click ein D&D gestartet werden kann)
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
            sal_Bool bSelAtPoint = pFunctionSet->IsSelectionAtPoint( aPos );
            nFlags &= (~SELENG_IN_ADD);
            if ( (nFlags & SELENG_DRG_ENAB) && bSelAtPoint )
            {
                nFlags |= SELENG_WAIT_UPEVT;
                nFlags &= ~(SELENG_IN_SEL);
                pWin->ReleaseMouse();
                return sal_True;  //auf STARTDRAG-Command-Event warten
            }
            if ( eSelMode != SINGLE_SELECTION )
            {
                if( !IsAddMode() )
                    pFunctionSet->DeselectAll();
                else
                    pFunctionSet->DestroyAnchor();
                   nFlags &= (~SELENG_HAS_ANCH); // bHasAnchor = sal_False;
            }
            pFunctionSet->SetCursorAtPoint( aPos );
            // Sonderbehandlung Single-Selection, damit Select+Drag
            // in einem Zug moeglich ist
            if (eSelMode == SINGLE_SELECTION && (nFlags & SELENG_DRG_ENAB))
                nFlags |= SELENG_WAIT_UPEVT;
            return sal_True;
        }

        case KEY_SHIFT:
            if ( eSelMode == SINGLE_SELECTION )
            {
                pWin->ReleaseMouse();
                nFlags &= (~SELENG_IN_SEL);
                return sal_False;
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
            return sal_True;

        case KEY_MOD1:
            // Control nur bei Mehrfachselektion erlaubt
            if ( eSelMode != MULTIPLE_SELECTION )
            {
                nFlags &= (~SELENG_IN_SEL);
                pWin->ReleaseMouse();
                return sal_True;  // Mausclick verschlucken
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
                pFunctionSet->SetCursorAtPoint( aPos, sal_True );
            }
            else
            {
                pFunctionSet->SetCursorAtPoint( aPos );
            }
            return sal_True;

        case KEY_SHIFT + KEY_MOD1:
            if ( eSelMode != MULTIPLE_SELECTION )
            {
                pWin->ReleaseMouse();
                nFlags &= (~SELENG_IN_SEL);
                return sal_False;
            }
            nFlags |= SELENG_IN_ADD; //bIsInAddMode = sal_True;
            if ( !(nFlags & SELENG_HAS_ANCH) )
            {
                pFunctionSet->CreateAnchor();
                nFlags |= SELENG_HAS_ANCH;
            }
            pFunctionSet->SetCursorAtPoint( aPos );
            return sal_True;
    }

    return sal_False;
}

/*************************************************************************
|*
|*    SelectionEngine::SelMouseButtonUp()
|*
|*    Beschreibung      SELENG.SDW
|*    Ersterstellung    OV 10.10.94
|*    Letzte Aenderung  OV 10.10.94
|*
*************************************************************************/

sal_Bool SelectionEngine::SelMouseButtonUp( const MouseEvent& rMEvt )
{
    aWTimer.Stop();
    //DbgOut("Up");
    if( !pFunctionSet || !pWin )
    {
        nFlags &= ~(SELENG_CMDEVT | SELENG_WAIT_UPEVT | SELENG_IN_SEL);
        return sal_False;
    }

    if( !rMEvt.IsRight() )
    {
       pWin->ReleaseMouse();
    }

    if( (nFlags & SELENG_WAIT_UPEVT) && !(nFlags & SELENG_CMDEVT) &&
        eSelMode != SINGLE_SELECTION)
    {
        // MouseButtonDown in Sel aber kein CommandEvent eingetrudelt
        // ==> deselektieren
        sal_uInt16 nModifier = aLastMove.GetModifier() | nLockedMods;
        if( nModifier == KEY_MOD1 || IsAlwaysAdding() )
        {
            if( !(nModifier & KEY_SHIFT) )
            {
                pFunctionSet->DestroyAnchor();
                nFlags &= (~SELENG_HAS_ANCH); // nix Anker
            }
            pFunctionSet->DeselectAtPoint( aLastMove.GetPosPixel() );
            nFlags &= (~SELENG_HAS_ANCH); // nix Anker
            pFunctionSet->SetCursorAtPoint( aLastMove.GetPosPixel(), sal_True );
        }
        else
        {
            pFunctionSet->DeselectAll();
            nFlags &= (~SELENG_HAS_ANCH); // nix Anker
            pFunctionSet->SetCursorAtPoint( aLastMove.GetPosPixel() );
        }
    }

    nFlags &= ~(SELENG_CMDEVT | SELENG_WAIT_UPEVT | SELENG_IN_SEL);
    return sal_True;
}

/*************************************************************************
|*
|*    SelectionEngine::SelMouseMove()
|*
|*    Beschreibung      SELENG.SDW
|*    Ersterstellung    OV 10.10.94
|*    Letzte Aenderung  OV 10.10.94
|*
*************************************************************************/

sal_Bool SelectionEngine::SelMouseMove( const MouseEvent& rMEvt )
{

    if ( !pFunctionSet || !(nFlags & SELENG_IN_SEL) ||
         (nFlags & (SELENG_CMDEVT | SELENG_WAIT_UPEVT)) )
        return sal_False;

    if( !(nFlags & SELENG_EXPANDONMOVE) )
        return sal_False; // auf DragEvent warten!

    aLastMove = rMEvt;
    // wenn die Maus ausserhalb der Area steht, dann wird die
    // Frequenz des SetCursorAtPoint() nur durch den Timer bestimmt
    if( aWTimer.IsActive() && !aArea.IsInside( rMEvt.GetPosPixel() ))
        return sal_True;


    aWTimer.Start();
    if ( eSelMode != SINGLE_SELECTION )
    {
        if ( !(nFlags & SELENG_HAS_ANCH) )
        {
            pFunctionSet->CreateAnchor();
            //DbgOut("Move:Creating anchor");
            nFlags |= SELENG_HAS_ANCH;
        }
    }

    //DbgOut("Move:SetCursor");
    pFunctionSet->SetCursorAtPoint( rMEvt.GetPosPixel() );

    return sal_True;
}

/*************************************************************************
|*
|*    SelectionEngine::SetWindow()
|*
|*    Beschreibung      SELENG.SDW
|*    Ersterstellung    OV 10.10.94
|*    Letzte Aenderung  OV 10.10.94
|*
*************************************************************************/

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

/*************************************************************************
|*
|*    SelectionEngine::Reset()
|*
|*    Beschreibung      SELENG.SDW
|*    Ersterstellung    OV 07.07.95
|*    Letzte Aenderung  OV 07.07.95
|*
*************************************************************************/

void SelectionEngine::Reset()
{
    aWTimer.Stop();
    if ( nFlags & SELENG_IN_SEL )
        pWin->ReleaseMouse();
    nFlags &= ~(SELENG_HAS_ANCH | SELENG_IN_SEL);
    nLockedMods = 0;
}

/*************************************************************************
|*
|*    SelectionEngine::Command()
|*
|*    Beschreibung      SELENG.SDW
|*    Ersterstellung    OV 07.07.95
|*    Letzte Aenderung  OV 07.07.95
|*
*************************************************************************/

void SelectionEngine::Command( const CommandEvent& rCEvt )
{
    // Timer aWTimer ist beim Aufspannen einer Selektion aktiv
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
