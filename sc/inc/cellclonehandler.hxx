/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_CELLCLONEHANDLER_HXX
#define SC_CELLCLONEHANDLER_HXX

#include "rtl/ustring.hxx"
#include "address.hxx"
#include "mtvelements.hxx"

class ScDocument;
class EditTextObject;
class ScFormulaCell;

namespace sc {

class CellBlockCloneHandler
{
    ScDocument& mrSrcDoc;
    ScDocument& mrDestDoc;
    CellStoreType& mrDestCellStore;
    CellTextAttrStoreType& mrDestAttrStore;

protected:
    ScDocument& getSrcDoc();
    ScDocument& getDestDoc();
    const ScDocument& getDestDoc() const;
    CellStoreType& getDestCellStore();
    CellTextAttrStoreType& getDestAttrStore();

    void setDefaultAttrToDest(ColumnBlockPosition& rPos, SCROW nRow);
    void setDefaultAttrsToDest(ColumnBlockPosition& rPos, SCROW nRow, size_t nSize);

public:
    CellBlockCloneHandler(
        ScDocument& rSrcDoc, ScDocument& rDestDoc,
        CellStoreType& rDestCellStore, CellTextAttrStoreType& rDestAttrStore);

    virtual ~CellBlockCloneHandler();

    virtual void cloneDoubleBlock(
        ColumnBlockPosition& rPos, const ScAddress& rSrcPos, const ScAddress& rDestPos,
        const numeric_block::const_iterator& itBegin, const numeric_block::const_iterator& itEnd);

    virtual void cloneStringBlock(
        ColumnBlockPosition& rPos, const ScAddress& rSrcPos, const ScAddress& rDestPos,
        const string_block::const_iterator& itBegin, const string_block::const_iterator& itEnd);

    virtual void cloneEditTextBlock(
        ColumnBlockPosition& rPos, const ScAddress& rSrcPos, const ScAddress& rDestPos,
        const edittext_block::const_iterator& itBegin, const edittext_block::const_iterator& itEnd);

    virtual void cloneFormulaBlock(
        ColumnBlockPosition& rPos, const ScAddress& rSrcPos, const ScAddress& rDestPos,
        const formula_block::const_iterator& itBegin, const formula_block::const_iterator& itEnd);
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
