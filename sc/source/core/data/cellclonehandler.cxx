/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "cellclonehandler.hxx"
#include "editutil.hxx"
#include "document.hxx"

namespace sc {

CellBlockCloneHandler::CellBlockCloneHandler(
    ScDocument& rSrcDoc, ScDocument& rDestDoc, CellStoreType& rDestCellStore) :
    mrSrcDoc(rSrcDoc), mrDestDoc(rDestDoc), mrDestCellStore(rDestCellStore) {}

CellBlockCloneHandler::~CellBlockCloneHandler() {}

ScDocument& CellBlockCloneHandler::getSrcDoc()
{
    return mrSrcDoc;
}

ScDocument& CellBlockCloneHandler::getDestDoc()
{
    return mrDestDoc;
}

const ScDocument& CellBlockCloneHandler::getDestDoc() const
{
    return mrDestDoc;
}

CellStoreType& CellBlockCloneHandler::getDestCellStore()
{
    return mrDestCellStore;
}

void CellBlockCloneHandler::cloneDoubleBlock(
    CellStoreType::iterator& itPos, const ScAddress& /*rSrcPos*/, const ScAddress& rDestPos,
    const numeric_block::const_iterator& itBegin, const numeric_block::const_iterator& itEnd)
{
    itPos = mrDestCellStore.set(itPos, rDestPos.Row(), itBegin, itEnd);
}

void CellBlockCloneHandler::cloneStringBlock(
    CellStoreType::iterator& itPos, const ScAddress& /*rSrcPos*/, const ScAddress& rDestPos,
    const string_block::const_iterator& itBegin, const string_block::const_iterator& itEnd)
{
    itPos = mrDestCellStore.set(itPos, rDestPos.Row(), itBegin, itEnd);
}

void CellBlockCloneHandler::cloneEditTextBlock(
    CellStoreType::iterator& itPos, const ScAddress& /*rSrcPos*/, const ScAddress& rDestPos,
    const edittext_block::const_iterator& itBegin, const edittext_block::const_iterator& itEnd)
{
    std::vector<EditTextObject*> aCloned;
    aCloned.reserve(std::distance(itBegin, itEnd));
    for (edittext_block::const_iterator it = itBegin; it != itEnd; ++it)
        aCloned.push_back(ScEditUtil::Clone(**it, getDestDoc()));

    itPos = getDestCellStore().set(itPos, rDestPos.Row(), aCloned.begin(), aCloned.end());
}

void CellBlockCloneHandler::cloneFormulaBlock(
    CellStoreType::iterator& itPos, const ScAddress& /*rSrcPos*/, const ScAddress& rDestPos,
    const formula_block::const_iterator& itBegin, const formula_block::const_iterator& itEnd)
{
    std::vector<ScFormulaCell*> aCloned;
    aCloned.reserve(std::distance(itBegin, itEnd));
    ScAddress aDestPos = rDestPos;
    for (formula_block::const_iterator it = itBegin; it != itEnd; ++it, aDestPos.IncRow())
    {
        const ScFormulaCell& rOld = **it;
        if (rOld.GetDirty() && getSrcDoc().GetAutoCalc())
            const_cast<ScFormulaCell&>(rOld).Interpret();

        aCloned.push_back(new ScFormulaCell(rOld, getDestDoc(), aDestPos));
    }

    itPos = getDestCellStore().set(itPos, rDestPos.Row(), aCloned.begin(), aCloned.end());
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
