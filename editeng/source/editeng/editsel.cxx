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

#include "editsel.hxx"
#include "impedit.hxx"
#include <editeng/editview.hxx>



EditSelFunctionSet::EditSelFunctionSet()
{
    pCurView = nullptr;
}

void EditSelFunctionSet::CreateAnchor()
{
    if ( pCurView )
        pCurView->getImpl().CreateAnchor();
}

void EditSelFunctionSet::DestroyAnchor()
{
    // Only with multiple selection
}

void EditSelFunctionSet::SetCursorAtPoint( const Point& rPointPixel, bool )
{
    if ( pCurView )
        pCurView->getImpl().SetCursorAtPoint( rPointPixel );
}

bool EditSelFunctionSet::IsSelectionAtPoint( const Point& rPointPixel )
{
    if ( pCurView )
        return pCurView->getImpl().IsSelectionAtPoint( rPointPixel );

    return false;
}

void EditSelFunctionSet::DeselectAtPoint( const Point& )
{
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !   Implement when multiple selection is possible   !
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
}

void EditSelFunctionSet::BeginDrag()
{
    // Only with multiple selection
}


void EditSelFunctionSet::DeselectAll()
{
    if ( pCurView )
        pCurView->getImpl().DeselectAll();
}



EditSelectionEngine::EditSelectionEngine() : SelectionEngine( nullptr )
{
    SetSelectionMode( SelectionMode::Range );
    EnableDrag( true );
}

void EditSelectionEngine::SetCurView( EditView* pNewView )
{
    if ( GetFunctionSet() )
        const_cast<EditSelFunctionSet*>(static_cast<const EditSelFunctionSet*>(GetFunctionSet()))->SetCurView( pNewView );

    if ( pNewView )
        SetWindow( pNewView->GetWindow() );
    else
        SetWindow( nullptr );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
