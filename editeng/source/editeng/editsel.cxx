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

#include <vcl/wrkwin.hxx>
#include <vcl/dialog.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <editsel.hxx>
#include <impedit.hxx>
#include <editeng/editview.hxx>




EditSelFunctionSet::EditSelFunctionSet()
{
    pCurView = NULL;
}

void EditSelFunctionSet::CreateAnchor()
{
    if ( pCurView )
        pCurView->pImpEditView->CreateAnchor();
}

void EditSelFunctionSet::DestroyAnchor()
{
    
}

bool EditSelFunctionSet::SetCursorAtPoint( const Point& rPointPixel, sal_Bool )
{
    if ( pCurView )
        return pCurView->pImpEditView->SetCursorAtPoint( rPointPixel );

    return false;
}

bool EditSelFunctionSet::IsSelectionAtPoint( const Point& rPointPixel )
{
    if ( pCurView )
        return pCurView->pImpEditView->IsSelectionAtPoint( rPointPixel );

    return false;
}

void EditSelFunctionSet::DeselectAtPoint( const Point& )
{



}

void EditSelFunctionSet::BeginDrag()
{
    
}


void EditSelFunctionSet::DeselectAll()
{
    if ( pCurView )
        pCurView->pImpEditView->DeselectAll();
}




EditSelectionEngine::EditSelectionEngine() : SelectionEngine( (Window*)0 )
{
    SetSelectionMode( RANGE_SELECTION );
    EnableDrag( true );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
