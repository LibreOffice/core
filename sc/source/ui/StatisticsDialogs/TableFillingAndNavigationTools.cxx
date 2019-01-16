/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <memory>

#include <editeng/editobj.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/eeitem.hxx>

#include <editutil.hxx>

#include <TableFillingAndNavigationTools.hxx>
#include <formulacell.hxx>
#include <docfunc.hxx>
#include <docsh.hxx>

FormulaTemplate::FormulaTemplate(ScDocument* pDoc)
    : mpDoc(pDoc)
    , mbUse3D(true)
{}

void FormulaTemplate::setTemplate(const OUString& aTemplate)
{
    mTemplate = aTemplate;
}

void FormulaTemplate::setTemplate(const char* aTemplate)
{
    mTemplate = OUString::createFromAscii(aTemplate);
}

const OUString& FormulaTemplate::getTemplate()
{
    for (const auto& [rVariable, rRange] : mRangeReplacementMap)
    {
        applyRange(rVariable, rRange, mbUse3D);
    }
    for (const auto& [rVariable, rAddress] : mAddressReplacementMap)
    {
        applyAddress(rVariable, rAddress, mbUse3D);
    }
    return mTemplate;
}

void FormulaTemplate::autoReplaceRange(const OUString& aVariable, const ScRange& rRange)
{
    mRangeReplacementMap[aVariable] = rRange;
}

void FormulaTemplate::autoReplaceAddress(const OUString& aVariable, ScAddress const & aAddress)
{

    mAddressReplacementMap[aVariable] = aAddress;
}

void FormulaTemplate::applyRange(const OUString& aVariable, const ScRange& aRange, bool b3D)
{
    ScRefFlags nFlag = b3D ? ScRefFlags::RANGE_ABS_3D : ScRefFlags::RANGE_ABS;
    OUString aString = aRange.Format(nFlag, mpDoc, mpDoc->GetAddressConvention());
    mTemplate = mTemplate.replaceAll(aVariable, aString);
}

void FormulaTemplate::applyRangeList(const OUString& aVariable, const ScRangeList& aRangeList, sal_Unicode cDelimiter)
{
    OUString aString;
    aRangeList.Format(aString, ScRefFlags::RANGE_ABS_3D, mpDoc, mpDoc->GetAddressConvention(), cDelimiter);
    mTemplate = mTemplate.replaceAll(aVariable, aString);
}

void FormulaTemplate::applyAddress(const OUString& aVariable, const ScAddress& aAddress, bool b3D)
{
    ScRefFlags nFlag = b3D ? ScRefFlags::ADDR_ABS_3D : ScRefFlags::ADDR_ABS;
    OUString aString = aAddress.Format(nFlag, mpDoc, mpDoc->GetAddressConvention());
    mTemplate = mTemplate.replaceAll(aVariable, aString);
}

void FormulaTemplate::applyString(const OUString& aVariable, const OUString& aValue)
{
    mTemplate = mTemplate.replaceAll(aVariable, aValue);
}

void FormulaTemplate::applyNumber(const OUString& aVariable, sal_Int32 aValue)
{
    mTemplate = mTemplate.replaceAll(aVariable, OUString::number(aValue));
}

AddressWalker::AddressWalker(const ScAddress& aInitialAddress) :
    mCurrentAddress(aInitialAddress),
    mMinimumAddress(aInitialAddress),
    mMaximumAddress(aInitialAddress)
{
    mAddressStack.push_back(mCurrentAddress);
}

void AddressWalker::resetColumn()
{
    mCurrentAddress.SetCol(mAddressStack.back().Col());
}

void AddressWalker::resetRow()
{
    mCurrentAddress.SetRow(mAddressStack.back().Row());
}

void AddressWalker::reset()
{
    mCurrentAddress = mAddressStack.back();
}

void AddressWalker::newLine()
{
    resetColumn();
    nextRow();
}

ScAddress AddressWalker::current(SCCOL aRelCol, SCROW aRelRow, SCTAB aRelTab)
{
    return ScAddress(
            mCurrentAddress.Col() + aRelCol,
            mCurrentAddress.Row() + aRelRow,
            mCurrentAddress.Tab() + aRelTab);
}

void AddressWalker::nextColumn()
{
    mCurrentAddress.IncCol();

    if(mMaximumAddress.Col() < mCurrentAddress.Col())
        mMaximumAddress.SetCol(mCurrentAddress.Col());
}

void AddressWalker::nextRow()
{
    mCurrentAddress.IncRow();
    if(mMaximumAddress.Row() < mCurrentAddress.Row())
        mMaximumAddress.SetRow(mCurrentAddress.Row());
}

void AddressWalker::push(SCCOL aRelativeCol, SCROW aRelativeRow, SCTAB aRelativeTab)
{
    mCurrentAddress = current(aRelativeCol, aRelativeRow, aRelativeTab);
    mAddressStack.push_back(mCurrentAddress);
}

AddressWalkerWriter::AddressWalkerWriter(const ScAddress& aInitialAddress, ScDocShell* pDocShell, ScDocument* pDocument,
        formula::FormulaGrammar::Grammar eGrammar ) :
    AddressWalker(aInitialAddress),
    mpDocShell(pDocShell),
    mpDocument(pDocument),
    meGrammar(eGrammar)
{}

void AddressWalkerWriter::writeFormula(const OUString& aFormula)
{
    mpDocShell->GetDocFunc().SetFormulaCell(mCurrentAddress,
            new ScFormulaCell(mpDocument, mCurrentAddress, aFormula, meGrammar), true);
}

void AddressWalkerWriter::writeMatrixFormula(const OUString& aFormula, SCCOL nCols, SCROW nRows)
{
    ScRange aRange;
    aRange.aStart = mCurrentAddress;
    aRange.aEnd = mCurrentAddress;
    if (nCols > 1)
        aRange.aEnd.IncCol(nCols - 1);
    if (nRows > 1)
        aRange.aEnd.IncRow(nRows - 1);
    mpDocShell->GetDocFunc().EnterMatrix(aRange, nullptr, nullptr, aFormula, false, false, OUString(), meGrammar );
}

void AddressWalkerWriter::writeString(const OUString& aString)
{
    mpDocShell->GetDocFunc().SetStringCell(mCurrentAddress, aString, true);
}

void AddressWalkerWriter::writeString(const char* aCharArray)
{
    writeString(OUString::createFromAscii(aCharArray));
}

void AddressWalkerWriter::writeBoldString(const OUString& aString)
{
    ScFieldEditEngine& rEngine = mpDocument->GetEditEngine();
    rEngine.SetText(aString);
    SfxItemSet aItemSet = rEngine.GetEmptyItemSet();
    SvxWeightItem aWeight(WEIGHT_BOLD, EE_CHAR_WEIGHT);
    aItemSet.Put(aWeight);
    rEngine.QuickSetAttribs(aItemSet, ESelection(0, 0, 0, aString.getLength()) );
    std::unique_ptr<EditTextObject> pEditText(rEngine.CreateTextObject());
    mpDocShell->GetDocFunc().SetEditCell(mCurrentAddress, *pEditText, true);
}

void AddressWalkerWriter::writeValue(double aValue)
{
    mpDocShell->GetDocFunc().SetValueCell(mCurrentAddress, aValue, true);
}

// DataCellIterator

DataCellIterator::DataCellIterator(const ScRange& aInputRange, bool aByColumn)
    : mInputRange(aInputRange)
    , mByColumn(aByColumn)
    , mCol(0)
    , mRow(0)
{
    if(aByColumn)
        mCol = aInputRange.aStart.Col();
    else
        mRow = aInputRange.aStart.Row();
}

DataCellIterator::~DataCellIterator()
{}

bool DataCellIterator::hasNext()
{
    if(mByColumn)
        return mCol <= mInputRange.aEnd.Col();
    else
        return mRow <= mInputRange.aEnd.Row();
}

void DataCellIterator::next()
{
    if(mByColumn)
        mCol++;
    else
        mRow++;
}

ScAddress DataCellIterator::get()
{
    return getRelative(0);
}

ScAddress DataCellIterator::getRelative(int aDelta)
{
    if(mByColumn)
    {
        SCCOL aNewColumn = mCol + aDelta;
        if(aNewColumn < mInputRange.aStart.Col() || aNewColumn > mInputRange.aEnd.Col())
        {
            ScAddress aResult;
            aResult.SetInvalid();
            return aResult;
        }
        return ScAddress(aNewColumn, mInputRange.aStart.Row(), mInputRange.aStart.Tab());
    }
    else
    {
        SCROW aNewRow = mRow + aDelta;
        if(aNewRow < mInputRange.aStart.Row() || aNewRow > mInputRange.aEnd.Row())
        {
            ScAddress aResult;
            aResult.SetInvalid();
            return aResult;
        }
        return ScAddress(mInputRange.aStart.Col(), aNewRow, mInputRange.aStart.Tab());
    }
}

// DataRangeIterator

DataRangeIterator::DataRangeIterator(const ScRange& aInputRange) :
    mInputRange(aInputRange),
    mIndex(0)
{}

DataRangeIterator::~DataRangeIterator()
{}

sal_Int32 DataRangeIterator::index()
{
    return mIndex;
}

// DataRangeByColumnIterator

DataRangeByColumnIterator::DataRangeByColumnIterator(const ScRange& aInputRange)
    : DataRangeIterator(aInputRange)
    , mCol(aInputRange.aStart.Col())
{}

bool DataRangeByColumnIterator::hasNext()
{
    return mCol <= mInputRange.aEnd.Col();
}

void DataRangeByColumnIterator::next()
{
    mCol++;
    mIndex++;
}

ScRange DataRangeByColumnIterator::get()
{
    return ScRange(
        ScAddress(mCol, mInputRange.aStart.Row(), mInputRange.aStart.Tab()),
        ScAddress(mCol, mInputRange.aEnd.Row(),   mInputRange.aEnd.Tab())
    );
}

size_t DataRangeByColumnIterator::size()
{
    return mInputRange.aEnd.Row() - mInputRange.aStart.Row() + 1;
}

void DataRangeByColumnIterator::reset()
{
    mCol = mInputRange.aStart.Col();
}

DataCellIterator DataRangeByColumnIterator::iterateCells()
{
    return DataCellIterator(get(), false);
}

// DataRangeByRowIterator

DataRangeByRowIterator::DataRangeByRowIterator(const ScRange& aInputRange)
    : DataRangeIterator(aInputRange)
    , mRow(aInputRange.aStart.Row())
{}

bool DataRangeByRowIterator::hasNext()
{
    return mRow <= mInputRange.aEnd.Row();
}

void DataRangeByRowIterator::next()
{
    mRow++;
    mIndex++;
}

ScRange DataRangeByRowIterator::get()
{
    return ScRange(
        ScAddress(mInputRange.aStart.Col(), mRow, mInputRange.aStart.Tab()),
        ScAddress(mInputRange.aEnd.Col(),   mRow, mInputRange.aEnd.Tab())
    );
}

size_t DataRangeByRowIterator::size()
{
    return mInputRange.aEnd.Col() - mInputRange.aStart.Col() + 1;
}

void DataRangeByRowIterator::reset()
{
    mRow = mInputRange.aStart.Row();
}

DataCellIterator DataRangeByRowIterator::iterateCells()
{
    return DataCellIterator(get(), true);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
