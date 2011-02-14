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
#include "precompiled_editeng.hxx"

#include <eeng_pch.hxx>

#include <editsel.hxx>
#include <impedit.hxx>
#include <editeng/editview.hxx>

//  ----------------------------------------------------------------------
//  class EditSelFunctionSet
//  ----------------------------------------------------------------------
EditSelFunctionSet::EditSelFunctionSet()
{
    pCurView = NULL;
}

void __EXPORT EditSelFunctionSet::CreateAnchor()
{
    if ( pCurView )
        pCurView->pImpEditView->CreateAnchor();
}

void __EXPORT EditSelFunctionSet::DestroyAnchor()
{
    // Nur bei Mehrfachselektion
}

sal_Bool __EXPORT EditSelFunctionSet::SetCursorAtPoint( const Point& rPointPixel, sal_Bool )
{
    if ( pCurView )
        return pCurView->pImpEditView->SetCursorAtPoint( rPointPixel );

    return sal_False;
}

sal_Bool __EXPORT EditSelFunctionSet::IsSelectionAtPoint( const Point& rPointPixel )
{
    if ( pCurView )
        return pCurView->pImpEditView->IsSelectionAtPoint( rPointPixel );

    return sal_False;
}

void __EXPORT EditSelFunctionSet::DeselectAtPoint( const Point& )
{
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !  Implementieren, wenn Mehrfachselektion moeglich  !
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
}

void __EXPORT EditSelFunctionSet::BeginDrag()
{
    // Nur bei Mehrfachselektion
}


void __EXPORT EditSelFunctionSet::DeselectAll()
{
    if ( pCurView )
        pCurView->pImpEditView->DeselectAll();
}

//  ----------------------------------------------------------------------
//  class EditSelectionEngine
//  ----------------------------------------------------------------------
EditSelectionEngine::EditSelectionEngine() : SelectionEngine( (Window*)0 )
{
    // Wegen Bug OV: (1994)
    // 1995: RangeSelection lassen, SingleSelection nur fuer ListBoxen geeignet!
    SetSelectionMode( RANGE_SELECTION );
    EnableDrag( sal_True );
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

