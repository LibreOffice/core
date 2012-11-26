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
#include "precompiled_sw.hxx"

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
    SwTOXBaseSection* pTOXBase = dynamic_cast< SwTOXBaseSection * >(pTOX);

    if (pTOXBase)
    {
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

