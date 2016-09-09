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

#include <comphelper/lok.hxx>
#include <sfx2/viewsh.hxx>
#include <undo/undomanager.hxx>

using namespace sd;

UndoManager::UndoManager()
  : SdrUndoManager( 20/*nMaxUndoActionCount*/ )
  , mpLinkedUndoManager(nullptr)
  , mpViewShell(nullptr)
{
}

void UndoManager::EnterListAction(const OUString &rComment, const OUString& rRepeatComment, sal_uInt16 nId, sal_Int32 nViewShellId)
{
    if( !IsDoing() )
    {
        ClearLinkedRedoActions();
        SdrUndoManager::EnterListAction( rComment, rRepeatComment, nId, nViewShellId );
    }
}

void UndoManager::AddUndoAction( SfxUndoAction *pAction, bool bTryMerg /* = sal_False */ )
{
    if( !IsDoing() )
    {
        ClearLinkedRedoActions();
        SdrUndoManager::AddUndoAction( pAction, bTryMerg );
    }
    else
    {
        delete pAction;
    }
}

size_t UndoManager::GetUndoActionCount(const bool bCurrentLevel) const
{
    size_t nRet = SdrUndoManager::GetUndoActionCount(bCurrentLevel);
    if (!comphelper::LibreOfficeKit::isActive() || !mpViewShell)
        return nRet;

    if (!nRet || !SdrUndoManager::GetUndoActionCount())
        return nRet;

    const SfxUndoAction* pAction = SdrUndoManager::GetUndoAction();
    if (!pAction)
        return nRet;

    // If an other view created the last undo action, prevent undoing it from this view.
    sal_Int32 nViewShellId = mpViewShell->GetViewShellId();
    if (pAction->GetViewShellId() != nViewShellId)
        nRet = 0;

    return nRet;
}

size_t UndoManager::GetRedoActionCount(const bool bCurrentLevel) const
{
    size_t nRet = SdrUndoManager::GetRedoActionCount(bCurrentLevel);
    if (!comphelper::LibreOfficeKit::isActive() || !mpViewShell)
        return nRet;

    if (!nRet || !SdrUndoManager::GetRedoActionCount())
        return nRet;

    const SfxUndoAction* pAction = SdrUndoManager::GetRedoAction();
    if (!pAction)
        return nRet;

    // If an other view created the first redo action, prevent redoing it from this view.
    sal_Int32 nViewShellId = mpViewShell->GetViewShellId();
    if (pAction->GetViewShellId() != nViewShellId)
        nRet = 0;

    return nRet;
}

void UndoManager::SetViewShell(SfxViewShell* pViewShell)
{
    mpViewShell = pViewShell;
}

void UndoManager::SetLinkedUndoManager (::svl::IUndoManager* pLinkedUndoManager)
{
    mpLinkedUndoManager = pLinkedUndoManager;
}

void UndoManager::ClearLinkedRedoActions()
{
    if (mpLinkedUndoManager != nullptr)
        mpLinkedUndoManager->ClearRedo();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
