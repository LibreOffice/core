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
    ScDocument& rSrcDoc, ScDocument& rDestDoc,
    CellStoreType& rDestCellStore, CellTextAttrStoreType& rDestAttrStore) :
    mrSrcDoc(rSrcDoc), mrDestDoc(rDestDoc),
    mrDestCellStore(rDestCellStore), mrDestAttrStore(rDestAttrStore) {}

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

CellTextAttrStoreType& CellBlockCloneHandler::getDestAttrStore()
{
    return mrDestAttrStore;
}

void CellBlockCloneHandler::setDefaultAttrToDest(ColumnBlockPosition& rPos, SCROW nRow)
{
    rPos.miCellTextAttrPos = mrDestAttrStore.set(rPos.miCellTextAttrPos, nRow, CellTextAttr());
}

void CellBlockCloneHandler::setDefaultAttrsToDest(ColumnBlockPosition& rPos, SCROW nRow, size_t nSize)
{
    std::vector<sc::CellTextAttr> aAttrs(nSize); // default values
    rPos.miCellTextAttrPos = mrDestAttrStore.set(
        rPos.miCellTextAttrPos, nRow, aAttrs.begin(), aAttrs.end());
}

void CellBlockCloneHandler::cloneDoubleBlock(
    ColumnBlockPosition& rPos, const ScAddress& /*rSrcPos*/, const ScAddress& rDestPos,
    const numeric_block::const_iterator& itBegin, const numeric_block::const_iterator& itEnd)
{
    rPos.miCellPos = mrDestCellStore.set(rPos.miCellPos, rDestPos.Row(), itBegin, itEnd);
    setDefaultAttrsToDest(rPos, rDestPos.Row(), std::distance(itBegin, itEnd));
}

void CellBlockCloneHandler::cloneStringBlock(
    ColumnBlockPosition& rPos, const ScAddress& /*rSrcPos*/, const ScAddress& rDestPos,
    const string_block::const_iterator& itBegin, const string_block::const_iterator& itEnd)
{
    rPos.miCellPos = mrDestCellStore.set(rPos.miCellPos, rDestPos.Row(), itBegin, itEnd);
    setDefaultAttrsToDest(rPos, rDestPos.Row(), std::distance(itBegin, itEnd));
}

void CellBlockCloneHandler::cloneEditTextBlock(
    ColumnBlockPosition& rPos, const ScAddress& /*rSrcPos*/, const ScAddress& rDestPos,
    const edittext_block::const_iterator& itBegin, const edittext_block::const_iterator& itEnd)
{
    std::vector<EditTextObject*> aCloned;
    aCloned.reserve(std::distance(itBegin, itEnd));
    for (edittext_block::const_iterator it = itBegin; it != itEnd; ++it)
        aCloned.push_back(ScEditUtil::Clone(**it, getDestDoc()));

    rPos.miCellPos = getDestCellStore().set(
        rPos.miCellPos, rDestPos.Row(), aCloned.begin(), aCloned.end());

    setDefaultAttrsToDest(rPos, rDestPos.Row(), std::distance(itBegin, itEnd));
}

void CellBlockCloneHandler::cloneFormulaBlock(
    ColumnBlockPosition& rPos, const ScAddress& /*rSrcPos*/, const ScAddress& rDestPos,
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

    rPos.miCellPos = getDestCellStore().set(
        rPos.miCellPos, rDestPos.Row(), aCloned.begin(), aCloned.end());

    setDefaultAttrsToDest(rPos, rDestPos.Row(), std::distance(itBegin, itEnd));
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
