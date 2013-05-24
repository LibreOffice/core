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

protected:
    ScDocument& getSrcDoc();
    ScDocument& getDestDoc();
    const ScDocument& getDestDoc() const;
    CellStoreType& getDestCellStore();

public:
    CellBlockCloneHandler(
        ScDocument& rSrcDoc, ScDocument& rDestDoc, CellStoreType& rDestCellStore);
    virtual ~CellBlockCloneHandler();

    virtual void cloneDoubleBlock(
        CellStoreType::iterator& itPos, const ScAddress& rSrcPos, const ScAddress& rDestPos,
        const numeric_block::const_iterator& itBegin, const numeric_block::const_iterator& itEnd);

    virtual void cloneStringBlock(
        CellStoreType::iterator& itPos, const ScAddress& rSrcPos, const ScAddress& rDestPos,
        const string_block::const_iterator& itBegin, const string_block::const_iterator& itEnd);

    virtual void cloneEditTextBlock(
        CellStoreType::iterator& itPos, const ScAddress& rSrcPos, const ScAddress& rDestPos,
        const edittext_block::const_iterator& itBegin, const edittext_block::const_iterator& itEnd);

    virtual void cloneFormulaBlock(
        CellStoreType::iterator& itPos, const ScAddress& rSrcPos, const ScAddress& rDestPos,
        const formula_block::const_iterator& itBegin, const formula_block::const_iterator& itEnd);
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
