/*************************************************************************
 *
 *  $RCSfile: seleng.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:40 $
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

#include <tools/ref.hxx>
#include <window.hxx>
#include <seleng.hxx>
#include <tools/debug.hxx>

#pragma hdrstop

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
|*    Letzte Aenderung  OV 10.10.94
|*
*************************************************************************/

void SelectionEngine::CursorPosChanging( BOOL bShift, BOOL /* bMod1 */ )
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
            pFunctionSet->DeselectAll();
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

BOOL SelectionEngine::SelMouseButtonDown( const MouseEvent& rMEvt )
{
    nFlags &= (~SELENG_CMDEVT);
    if ( !pFunctionSet || !pWin || rMEvt.GetClicks() > 1 || rMEvt.IsRight() )
        return FALSE;

    USHORT nModifier = rMEvt.GetModifier() | nLockedMods;
    if ( nModifier & KEY_MOD2 )
        return FALSE;
    // in SingleSelection: Control-Taste filtern (damit auch
    // mit Ctrl-Click ein D&D gestartet werden kann)
    if ( nModifier == KEY_MOD1 && eSelMode == SINGLE_SELECTION )
        nModifier = 0;

    Point aPos = rMEvt.GetPosPixel();
    aLastMove = rMEvt;

    pWin->CaptureMouse();
    nFlags |= SELENG_IN_SEL;

    switch ( nModifier )
    {
        case 0:     // KEY_NO_KEY
        {
            BOOL bSelAtPoint = pFunctionSet->IsSelectionAtPoint( aPos );
            nFlags &= (~SELENG_IN_ADD);
            if ( (nFlags & SELENG_DRG_ENAB) && bSelAtPoint )
            {
                nFlags |= SELENG_WAIT_UPEVT;
                nFlags &= ~(SELENG_IN_SEL);
                pWin->ReleaseMouse();
                return TRUE;  //auf STARTDRAG-Command-Event warten
            }
            if ( eSelMode != SINGLE_SELECTION )
            {
                if( !IsAddMode() )
                    pFunctionSet->DeselectAll();
                else
                    pFunctionSet->DestroyAnchor();
                   nFlags &= (~SELENG_HAS_ANCH); // bHasAnchor = FALSE;
            }
            pFunctionSet->SetCursorAtPoint( aPos );
            // Sonderbehandlung Single-Selection, damit Select+Drag
            // in einem Zug moeglich ist
            if (eSelMode == SINGLE_SELECTION && (nFlags & SELENG_DRG_ENAB))
                nFlags |= SELENG_WAIT_UPEVT;
            return TRUE;
        }

        case KEY_SHIFT:
            if ( eSelMode == SINGLE_SELECTION )
            {
                pWin->ReleaseMouse();
                nFlags &= (~SELENG_IN_SEL);
                return FALSE;
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
            return TRUE;

        case KEY_MOD1:
            // Control nur bei Mehrfachselektion erlaubt
            if ( eSelMode != MULTIPLE_SELECTION )
            {
                nFlags &= (~SELENG_IN_SEL);
                pWin->ReleaseMouse();
                return TRUE;  // Mausclick verschlucken
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
                pFunctionSet->SetCursorAtPoint( aPos, TRUE );
            }
            else
            {
                pFunctionSet->SetCursorAtPoint( aPos );
            }
            return TRUE;

        case KEY_SHIFT + KEY_MOD1:
            if ( eSelMode != MULTIPLE_SELECTION )
            {
                pWin->ReleaseMouse();
                nFlags &= (~SELENG_IN_SEL);
                return FALSE;
            }
            nFlags |= SELENG_IN_ADD; //bIsInAddMode = TRUE;
            if ( !(nFlags & SELENG_HAS_ANCH) )
            {
                pFunctionSet->CreateAnchor();
                nFlags |= SELENG_HAS_ANCH;
            }
            pFunctionSet->SetCursorAtPoint( aPos );
            return TRUE;
    }

    return FALSE;
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

BOOL SelectionEngine::SelMouseButtonUp( const MouseEvent& /* rMEvt */ )
{
    aWTimer.Stop();
    //DbgOut("Up");
    if( !pFunctionSet || !pWin )
    {
        nFlags &= ~(SELENG_CMDEVT | SELENG_WAIT_UPEVT | SELENG_IN_SEL);
        return FALSE;
    }
    pWin->ReleaseMouse();

    if( (nFlags & SELENG_WAIT_UPEVT) && !(nFlags & SELENG_CMDEVT) &&
        eSelMode != SINGLE_SELECTION)
    {
        // MouseButtonDown in Sel aber kein CommandEvent eingetrudelt
        // ==> deselektieren
        USHORT nModifier = aLastMove.GetModifier() | nLockedMods;
        if( nModifier == KEY_MOD1 || IsAlwaysAdding() )
        {
            if( !(nModifier & KEY_SHIFT) )
            {
                pFunctionSet->DestroyAnchor();
                nFlags &= (~SELENG_HAS_ANCH); // nix Anker
            }
            pFunctionSet->DeselectAtPoint( aLastMove.GetPosPixel() );
            nFlags &= (~SELENG_HAS_ANCH); // nix Anker
            pFunctionSet->SetCursorAtPoint( aLastMove.GetPosPixel(), TRUE );
        }
        else
        {
            pFunctionSet->DeselectAll();
            nFlags &= (~SELENG_HAS_ANCH); // nix Anker
            pFunctionSet->SetCursorAtPoint( aLastMove.GetPosPixel() );
        }
    }

    nFlags &= ~(SELENG_CMDEVT | SELENG_WAIT_UPEVT | SELENG_IN_SEL);
    return TRUE;
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

BOOL SelectionEngine::SelMouseMove( const MouseEvent& rMEvt )
{

    if ( !pFunctionSet || !(nFlags & SELENG_IN_SEL) ||
         (nFlags & (SELENG_CMDEVT | SELENG_WAIT_UPEVT)) )
        return FALSE;

    if( !(nFlags & SELENG_EXPANDONMOVE) )
        return FALSE; // auf DragEvent warten!

    aLastMove = rMEvt;
    // wenn die Maus ausserhalb der Area steht, dann wird die
    // Frequenz des SetCursorAtPoint() nur durch den Timer bestimmt
    if( aWTimer.IsActive() && !aArea.IsInside( rMEvt.GetPosPixel() ))
        return TRUE;


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

    return TRUE;
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
