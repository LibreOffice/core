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
#include <UndoCore.hxx>
#include <doctxm.hxx>
#include <doc.hxx>
#include <node.hxx>

namespace
{
    sal_uLong GetSectionNodeIndex(SwTOXBaseSection const& rTOX)
    {
        const SwSectionNode* pSectNd = rTOX.GetFormat()->GetSectionNode();
        assert(pSectNd);
        return pSectNd->GetIndex();
    }
}

SwUndoTOXChange::SwUndoTOXChange(const SwDoc& rDoc,
        SwTOXBaseSection const& rTOX, SwTOXBase const& rNew)
    : SwUndo(SwUndoId::TOXCHANGE, &rDoc)
    , m_Old(rTOX)
    , m_New(rNew)
    , m_nNodeIndex(GetSectionNodeIndex(rTOX))
{
}

SwUndoTOXChange::~SwUndoTOXChange()
{
}

// get the current ToXBase, which is not necessarily the same instance that existed there before
static SwTOXBase & GetTOX(SwDoc & rDoc, sal_uLong const nNodeIndex)
{
    SwSectionNode *const pNode(rDoc.GetNodes()[nNodeIndex]->GetSectionNode());
    assert(pNode);
    assert(dynamic_cast<SwTOXBaseSection*>(&pNode->GetSection()));
    auto & rTOX(static_cast<SwTOXBaseSection&>(pNode->GetSection()));
    return rTOX;
}

void SwUndoTOXChange::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc(rContext.GetDoc());
    SwTOXBase & rTOX(GetTOX(rDoc, m_nNodeIndex));
    rTOX = m_Old;
}

void SwUndoTOXChange::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc(rContext.GetDoc());
    SwTOXBase & rTOX(GetTOX(rDoc, m_nNodeIndex));
    rTOX = m_New;
}

void SwUndoTOXChange::RepeatImpl(::sw::RepeatContext & rContext)
{
    SwDoc & rDoc(rContext.GetDoc());
    SwTOXBase *const pTOX(SwDoc::GetCurTOX(*rContext.GetRepeatPaM().GetPoint()));
    if (pTOX)
    {
        rDoc.ChangeTOX(*pTOX, m_New);
        // intentionally limited to not Update because we'd need layout
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
