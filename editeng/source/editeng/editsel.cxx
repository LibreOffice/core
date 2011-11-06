/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

