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

#include <SwUndoTOXChange.hxx>
#include <swundo.hxx>
#include <doctxm.hxx>

SwUndoTOXChange::SwUndoTOXChange(SwTOXBase * _pTOX, const SwTOXBase & rNew)
    : SwUndo(UNDO_TOXCHANGE), pTOX(_pTOX), aOld(*_pTOX), aNew(rNew)
{
}

SwUndoTOXChange::~SwUndoTOXChange()
{
}

void SwUndoTOXChange::UpdateTOXBaseSection()
{
    if ( dynamic_cast< const SwTOXBaseSection *>( pTOX ) != nullptr )
    {
        SwTOXBaseSection * pTOXBase = static_cast<SwTOXBaseSection *>(pTOX);
        pTOXBase->Update();
        pTOXBase->UpdatePageNum();
    }
}

void SwUndoTOXChange::UndoImpl(::sw::UndoRedoContext &)
{
    *pTOX = aOld;

    UpdateTOXBaseSection();
}

void SwUndoTOXChange::DoImpl()
{
    *pTOX = aNew;

    UpdateTOXBaseSection();
}

void SwUndoTOXChange::RedoImpl(::sw::UndoRedoContext &)
{
    DoImpl();
}

void SwUndoTOXChange::RepeatImpl(::sw::RepeatContext &)
{
    DoImpl();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
