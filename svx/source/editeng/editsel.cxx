/*************************************************************************
 *
 *  $RCSfile: editsel.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:13 $
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

#include <eeng_pch.hxx>

#pragma hdrstop

#include <editsel.hxx>
#include <impedit.hxx>
#include <editview.hxx>

// ----------------------------------------------------------------------
//  class EditSelFunctionSet
//  ----------------------------------------------------------------------
EditSelFunctionSet::EditSelFunctionSet()
{
    pCurView = 0;
}

void __EXPORT EditSelFunctionSet::BeginDrag()
{
    if ( pCurView )
        pCurView->GetImpEditEngine()->BeginDrag( pCurView );
}

void __EXPORT EditSelFunctionSet::CreateAnchor()
{
    if ( pCurView )
        pCurView->GetImpEditEngine()->CreateAnchor( pCurView );
}

void __EXPORT EditSelFunctionSet::DestroyAnchor()
{
    // Nur bei Mehrfachselektion
}

BOOL __EXPORT EditSelFunctionSet::SetCursorAtPoint( const Point& rPointPixel, BOOL )
{
    if ( pCurView )
        return pCurView->GetImpEditEngine()->SetCursorAtPoint( rPointPixel, pCurView );

    return FALSE;
}

BOOL __EXPORT EditSelFunctionSet::IsSelectionAtPoint( const Point& rPointPixel )
{
    if ( pCurView )
        return pCurView->GetImpEditEngine()->IsSelectionAtPoint( rPointPixel, pCurView );
    return FALSE;
}

void __EXPORT EditSelFunctionSet::DeselectAtPoint( const Point& )
{
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !  Implementieren, wenn Mehrfachselektion moeglich  !
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
}

void __EXPORT EditSelFunctionSet::DeselectAll()
{
    if ( pCurView )
        pCurView->GetImpEditEngine()->DeselectAll( pCurView );
}

// ----------------------------------------------------------------------
//  class EditSelectionEngine
//  ----------------------------------------------------------------------
EditSelectionEngine::EditSelectionEngine() : SelectionEngine( (Window*)0 )
{
    // Wegen Bug OV: (1994)
    // 1995: RangeSelection lassen, SingleSelection nur fuer ListBoxen geeignet!
    SetSelectionMode( RANGE_SELECTION );
    EnableDrag( TRUE );
}

void EditSelectionEngine::SetCurView( EditView* pNewView )
{
    if ( GetFunctionSet() )
        ((EditSelFunctionSet*)GetFunctionSet())->SetCurView( pNewView );

    if ( pNewView )
        SetWindow( pNewView->GetWindow() );
    else
        SetWindow( (Window*)0 );
}

EditView* EditSelectionEngine::GetCurView()
{
    EditView* pView = 0;
    if ( GetFunctionSet() )
        pView = ((EditSelFunctionSet*)GetFunctionSet())->GetCurView();

    return pView;
}

