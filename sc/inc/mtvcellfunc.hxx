/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "mtvelements.hxx"
#include "mtvfunctions.hxx"

class ScFormulaCell;

namespace sc {

template<typename Func>
void ProcessFormula(CellStoreType& rStore, Func& rFunc)
{
    FuncElseNoOp<size_t> aElse;
    ProcessElements1<CellStoreType, formula_block, Func, FuncElseNoOp<size_t> >(rStore, rFunc, aElse);
}

template<typename FuncElem>
typename CellStoreType::iterator
ProcessFormula(
    const CellStoreType::iterator& it, CellStoreType& rStore, SCROW nRow1, SCROW nRow2, FuncElem& rFuncElem)
{
    FuncElseNoOp<size_t> aElse;
    return ProcessElements1<
        CellStoreType, formula_block, FuncElem, FuncElseNoOp<size_t> >(it, rStore, nRow1, nRow2, rFuncElem, aElse);
}

/**
 * Process formula cells found within specified row range. This function
 * allows modifications of the states of the formula function objects.
 */
CellStoreType::iterator ProcessFormula(
    const CellStoreType::iterator& it, CellStoreType& rStore, SCROW nRow1, SCROW nRow2,
    std::function<void(size_t,ScFormulaCell*)> aFuncElem );

template<typename FuncElem, typename FuncElse>
typename CellStoreType::iterator
ProcessFormula(
    const CellStoreType::iterator& it, CellStoreType& rStore, SCROW nRow1, SCROW nRow2, FuncElem& rFuncElem, FuncElse& rFuncElse)
{
    return ProcessElements1<
        CellStoreType, formula_block, FuncElem, FuncElse>(it, rStore, nRow1, nRow2, rFuncElem, rFuncElse);
}

template<typename Func>
CellStoreType::iterator
ProcessEditText(const CellStoreType::iterator& itPos, CellStoreType& rStore, SCROW nRow1, SCROW nRow2, Func& rFunc)
{
    FuncElseNoOp<size_t> aElse;
    return ProcessElements1<CellStoreType, edittext_block, Func, FuncElseNoOp<size_t> >(
        itPos, rStore, nRow1, nRow2, rFunc, aElse);
}

template<typename Func>
void ParseFormula(
    const CellStoreType& rStore, Func& rFunc)
{
    FuncElseNoOp<size_t> aElse;
    ParseElements1<CellStoreType, formula_block, Func, FuncElseNoOp<size_t> >(rStore, rFunc, aElse);
}

template<typename Func>
typename CellStoreType::const_iterator
ParseFormula(
    const CellStoreType::const_iterator& itPos, const CellStoreType& rStore,
    SCROW nStart, SCROW nEnd, Func& rFunc)
{
    FuncElseNoOp<size_t> aElse;
    return ParseElements1<CellStoreType, formula_block, Func, FuncElseNoOp<size_t> >(
        itPos, rStore, nStart, nEnd, rFunc, aElse);
}

template<typename FuncElem, typename FuncElse>
typename CellStoreType::const_iterator
ParseAll(
    const typename CellStoreType::const_iterator& itPos, const CellStoreType& rCells,
    SCROW nRow1, SCROW nRow2, FuncElem& rFuncElem, FuncElse& rFuncElse)
{
    return ParseElements4<CellStoreType,
        numeric_block, string_block, edittext_block, formula_block,
        FuncElem, FuncElse>(
            itPos, rCells, nRow1, nRow2, rFuncElem, rFuncElse);
}

template<typename Func>
typename CellStoreType::const_iterator
ParseAllNonEmpty(
    const typename CellStoreType::const_iterator& itPos, const CellStoreType& rCells,
    SCROW nRow1, SCROW nRow2, Func& rFunc)
{
    FuncElseNoOp<size_t> aElse;
    return ParseElements4<CellStoreType,
        numeric_block, string_block, edittext_block, formula_block,
        Func, FuncElseNoOp<size_t> >(
            itPos, rCells, nRow1, nRow2, rFunc, aElse);
}

template<typename Func>
typename CellStoreType::const_iterator
ParseFormulaNumeric(
    const CellStoreType::const_iterator& itPos, const CellStoreType& rCells,
    SCROW nRow1, SCROW nRow2, Func& rFunc)
{
    FuncElseNoOp<size_t> aElse;
    return ParseElements2<CellStoreType,
        numeric_block, formula_block, Func, FuncElseNoOp<size_t> >(
            itPos, rCells, nRow1, nRow2, rFunc, aElse);
}

template<typename Func>
void ProcessFormulaEditText(CellStoreType& rStore, Func& rFunc)
{
    // Walk backwards through the data - this helps when the FuncElem will be deleting
    // stuff, so we don't continually move block data around.
    FuncElseNoOp<size_t> aElse;
    ProcessElements2Reverse<CellStoreType, edittext_block, formula_block, Func, FuncElseNoOp<size_t> >(rStore, rFunc, aElse);
}

template<typename Func>
std::pair<CellStoreType::const_iterator, size_t>
FindFormula(const CellStoreType& rStore, SCROW nRow1, SCROW nRow2, Func& rFunc)
{
    typedef std::pair<size_t,bool> ElseRetType;
    FuncElseNoOp<size_t, ElseRetType> aElse;
    return FindElement1<CellStoreType, formula_block, Func, FuncElseNoOp<size_t, ElseRetType> >(rStore, nRow1, nRow2, rFunc, aElse);
}

template<typename Func>
std::pair<CellStoreType::const_iterator, size_t>
FindFormulaEditText(const CellStoreType& rStore, SCROW nRow1, SCROW nRow2, Func& rFunc)
{
    return FindElement2<CellStoreType, edittext_block, formula_block, Func, Func>(rStore, nRow1, nRow2, rFunc, rFunc);
}

template<typename Func>
void ProcessNote(CellNoteStoreType& rStore, Func& rFunc)
{
    FuncElseNoOp<size_t> aElse;
    ProcessElements1<CellNoteStoreType, cellnote_block, Func, FuncElseNoOp<size_t> >(rStore, rFunc, aElse);
}

template<typename Func>
typename CellNoteStoreType::const_iterator
ParseNote(
    const CellNoteStoreType::const_iterator& itPos, const CellNoteStoreType& rStore,
    SCROW nStart, SCROW nEnd, Func& rFunc)
{
    FuncElseNoOp<size_t> aElse;
    return ParseElements1<CellNoteStoreType, cellnote_block, Func, FuncElseNoOp<size_t> >(
        itPos, rStore, nStart, nEnd, rFunc, aElse);
}

template<typename FuncElem>
typename CellNoteStoreType::iterator
ProcessNote(
    const CellNoteStoreType::iterator& it, CellNoteStoreType& rStore, SCROW nRow1, SCROW nRow2, FuncElem& rFuncElem)
{
    FuncElseNoOp<size_t> aElse;
    return ProcessElements1<
        CellNoteStoreType, cellnote_block, FuncElem, FuncElseNoOp<size_t> >(it, rStore, nRow1, nRow2, rFuncElem, aElse);
}

template<typename FuncElem>
typename BroadcasterStoreType::iterator
ProcessBroadcaster(
    const BroadcasterStoreType::iterator& it, BroadcasterStoreType& rStore, SCROW nRow1, SCROW nRow2, FuncElem& rFuncElem)
{
    FuncElseNoOp<size_t> aElse;
    return ProcessElements1<
        BroadcasterStoreType, broadcaster_block, FuncElem, FuncElseNoOp<size_t> >(it, rStore, nRow1, nRow2, rFuncElem, aElse);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
