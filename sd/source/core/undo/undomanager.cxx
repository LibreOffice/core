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

#include <undo/undomanager.hxx>

using namespace sd;

UndoManager::UndoManager()
  : mpLinkedUndoManager(nullptr)
{
}

void UndoManager::EnterListAction(const OUString &rComment, const OUString& rRepeatComment, sal_uInt16 nId, ViewShellId nViewShellId)
{
    if( !IsDoing() )
    {
        ClearLinkedRedoActions();
        SdrUndoManager::EnterListAction( rComment, rRepeatComment, nId, nViewShellId );
    }
}

void UndoManager::AddUndoAction( std::unique_ptr<SfxUndoAction> pAction, bool bTryMerg /* = sal_False */ )
{
    if( !IsDoing() )
    {
        ClearLinkedRedoActions();
        SdrUndoManager::AddUndoAction( std::move(pAction), bTryMerg );
    }
}

void UndoManager::SetLinkedUndoManager (SfxUndoManager* pLinkedUndoManager)
{
    mpLinkedUndoManager = pLinkedUndoManager;
}

void UndoManager::ClearLinkedRedoActions()
{
    if (mpLinkedUndoManager != nullptr)
        mpLinkedUndoManager->ClearRedo();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
