/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <boost/scoped_ptr.hpp>

#include <editeng/editobj.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/eeitem.hxx>

#include "editutil.hxx"

#include "TableFillingAndNavigationTools.hxx"

FormulaTemplate::FormulaTemplate(ScDocument* pDoc) : mpDoc(pDoc) {}

void FormulaTemplate::setTemplate(OUString aTemplate)
{
    mTemplate = aTemplate;
}

void FormulaTemplate::setTemplate(const char* aTemplate)
{
    mTemplate = OUString::createFromAscii(aTemplate);
}

const OUString& FormulaTemplate::getTemplate()
{
    return mTemplate;
}

void FormulaTemplate::applyRange(const OUString& aVariable, const ScRange& aRange, bool b3D)
{
    sal_uInt16 nFlag = b3D ? SCR_ABS_3D : SCR_ABS;
    OUString aString = aRange.Format(nFlag, mpDoc, mpDoc->GetAddressConvention());
    mTemplate = mTemplate.replaceAll(aVariable, aString);
}

void FormulaTemplate::applyRangeList(const OUString& aVariable, const ScRangeList& aRangeList, bool b3D)
{
    sal_uInt16 nFlag = b3D ? SCR_ABS_3D : SCR_ABS;
    OUString aString;
    aRangeList.Format(aString, nFlag, mpDoc, mpDoc->GetAddressConvention());
    mTemplate = mTemplate.replaceAll(aVariable, aString);
}

void FormulaTemplate::applyAddress(const OUString& aVariable, const ScAddress& aAddress, bool b3D)
{
    sal_uInt16 nFlag = b3D ? SCA_ABS_3D : SCA_ABS;
    OUString aString = aAddress.Format(nFlag, mpDoc, mpDoc->GetAddressConvention());
    mTemplate = mTemplate.replaceAll(aVariable, aString);
}

void FormulaTemplate::applyString(OUString aVariable, OUString aValue)
{
    mTemplate = mTemplate.replaceAll(aVariable, aValue);
}

void FormulaTemplate::applyNumber(OUString aVariable, sal_Int32 aValue)
{
    mTemplate = mTemplate.replaceAll(aVariable, OUString::number(aValue));
}

AddressWalker::AddressWalker(ScAddress aInitialAddress, bool aTrackRange) :
    mCurrentAddress(aInitialAddress),
    mMinimumAddress(aInitialAddress),
    mMaximumAddress(aInitialAddress),
    mTrackRange(aTrackRange)
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

    if (mTrackRange)
    {
        if(mMaximumAddress.Col() < mCurrentAddress.Col())
            mMaximumAddress.SetCol(mCurrentAddress.Col());
    }
}

void AddressWalker::nextRow()
{
    mCurrentAddress.IncRow();
    if (mTrackRange)
    {
        if(mMaximumAddress.Row() < mCurrentAddress.Row())
            mMaximumAddress.SetRow(mCurrentAddress.Row());
    }
}

void AddressWalker::push(SCCOL aRelativeCol, SCROW aRelativeRow, SCTAB aRelativeTab)
{
    mCurrentAddress = current(aRelativeCol, aRelativeRow, aRelativeTab);
    mAddressStack.push_back(mCurrentAddress);
}

void AddressWalker::pop()
{
    mCurrentAddress = mAddressStack.back();
    mAddressStack.pop_back();
}

AddressWalkerWriter::AddressWalkerWriter(ScAddress aInitialAddress, ScDocShell* pDocShell, ScDocument* pDocument,
        formula::FormulaGrammar::Grammar eGrammar ) :
    AddressWalker(aInitialAddress, true),
    mpDocShell(pDocShell),
    mpDocument(pDocument),
    meGrammar(eGrammar)
{}

void AddressWalkerWriter::writeFormula(OUString aFormula)
{
    mpDocShell->GetDocFunc().SetFormulaCell(mCurrentAddress,
            new ScFormulaCell(mpDocument, mCurrentAddress, aFormula, meGrammar), true);
}

void AddressWalkerWriter::writeString(OUString aString)
{
    mpDocShell->GetDocFunc().SetStringCell(mCurrentAddress, aString, true);
}

void AddressWalkerWriter::writeString(const char* aCharArray)
{
    writeString(OUString::createFromAscii(aCharArray));
}

void AddressWalkerWriter::writeBoldString(OUString aString)
{
    ScFieldEditEngine& rEngine = mpDocument->GetEditEngine();
    rEngine.SetText(aString);
    SfxItemSet aItemSet = rEngine.GetEmptyItemSet();
    SvxWeightItem aWeight(WEIGHT_BOLD, EE_CHAR_WEIGHT);
    aItemSet.Put(aWeight);
    rEngine.QuickSetAttribs(aItemSet, ESelection(0, 0, 0, aString.getLength()) );
    boost::scoped_ptr<EditTextObject> pEditText(rEngine.CreateTextObject());
    mpDocShell->GetDocFunc().SetEditCell(mCurrentAddress, *pEditText, true);
}

void AddressWalkerWriter::writeValue(double aValue)
{
    mpDocShell->GetDocFunc().SetValueCell(mCurrentAddress, aValue, true);
}

// DataCellIterator

DataCellIterator::DataCellIterator(ScRange aInputRange, bool aByColumn) :
    mInputRange(aInputRange),
    mByColumn(aByColumn)
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

DataRangeIterator::DataRangeIterator(ScRange aInputRange) :
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

DataRangeByColumnIterator::DataRangeByColumnIterator(ScRange aInputRange) :
    DataRangeIterator(aInputRange),
    mCol(aInputRange.aStart.Col())
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

void DataRangeByColumnIterator::reset()
{
    mCol = mInputRange.aStart.Col();
}

DataCellIterator DataRangeByColumnIterator::iterateCells()
{
    return DataCellIterator(get(), false);
}

// DataRangeByRowIterator

DataRangeByRowIterator::DataRangeByRowIterator(ScRange aInputRange) :
    DataRangeIterator(aInputRange),
    mRow(aInputRange.aStart.Row())
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

void DataRangeByRowIterator::reset()
{
    mRow = mInputRange.aStart.Row();
}

DataCellIterator DataRangeByRowIterator::iterateCells()
{
    return DataCellIterator(get(), true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
