/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_sd.hxx"
#include <tools/debug.hxx>
#include "undo/undomanager.hxx"

using namespace sd;

UndoManager::UndoManager( sal_uInt16 nMaxUndoActionCount /* = 20 */ )
: SfxUndoManager( nMaxUndoActionCount )
, mpLinkedUndoManager(NULL)
{
}

void UndoManager::EnterListAction(const UniString &rComment, const UniString& rRepeatComment, sal_uInt16 nId /* =0 */)
{
    if( !IsDoing() )
    {
        ClearLinkedRedoActions();
        SfxUndoManager::EnterListAction( rComment, rRepeatComment, nId );
    }
}

void UndoManager::AddUndoAction( SfxUndoAction *pAction, sal_Bool bTryMerg /* = sal_False */ )
{
    if( !IsDoing() )
    {
        ClearLinkedRedoActions();
        SfxUndoManager::AddUndoAction( pAction, bTryMerg );
    }
    else
    {
        delete pAction;
    }
}


void UndoManager::SetLinkedUndoManager (::svl::IUndoManager* pLinkedUndoManager)
{
    mpLinkedUndoManager = pLinkedUndoManager;
}




void UndoManager::ClearLinkedRedoActions (void)
{
    if (mpLinkedUndoManager != NULL)
        mpLinkedUndoManager->ClearRedo();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
