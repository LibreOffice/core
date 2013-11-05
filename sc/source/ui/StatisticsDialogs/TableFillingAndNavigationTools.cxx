/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include "TableFillingAndNavigationTools.hxx"

FormulaTemplate::FormulaTemplate(ScDocument* aDocument, ScAddress::Details aAddressDetails) :
    mDocument(aDocument),
    mAddressDetails(aAddressDetails)
{}

void FormulaTemplate::setTemplate(OUString aTemplate)
{
    mTemplate = aTemplate;
}

void FormulaTemplate::setTemplate(const char* aTemplate)
{
    mTemplate = OUString::createFromAscii(aTemplate);
}

OUString& FormulaTemplate::getTemplate()
{
    return mTemplate;
}

void FormulaTemplate::applyRange(OUString aVariable, ScRange aRange)
{
    OUString aString = aRange.Format(SCR_ABS, mDocument, mAddressDetails);
    mTemplate = mTemplate.replaceAll(aVariable, aString);
}

void FormulaTemplate::applyRangeList(OUString aVariable, ScRangeList aRangeList)
{
    OUString aString;
    aRangeList.Format(aString, SCR_ABS, mDocument);
    mTemplate = mTemplate.replaceAll(aVariable, aString);
}

void FormulaTemplate::applyAddress(OUString aVariable, ScAddress aAddress)
{
    OUString aString = aAddress.Format(SCR_ABS, mDocument, mAddressDetails);
    mTemplate = mTemplate.replaceAll(aVariable, aString);
}

void FormulaTemplate::applyString(OUString aVariable, OUString aValue)
{
    mTemplate = mTemplate.replaceAll(aVariable, aValue);
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

AddressWalkerWriter::AddressWalkerWriter(ScAddress aInitialAddress, ScDocShell* pDocShell, ScDocument* pDocument) :
    AddressWalker(aInitialAddress, true),
    mpDocShell(pDocShell),
    mpDocument(pDocument)
{}

void AddressWalkerWriter::writeFormula(OUString aFormula)
{
    mpDocShell->GetDocFunc().SetFormulaCell(mCurrentAddress, new ScFormulaCell(mpDocument, mCurrentAddress, aFormula), true);
}

void AddressWalkerWriter::writeString(OUString aString)
{
    mpDocShell->GetDocFunc().SetStringCell(mCurrentAddress, aString, true);
}

void AddressWalkerWriter::writeString(const char* aCharArray)
{
    mpDocShell->GetDocFunc().SetStringCell(mCurrentAddress, OUString::createFromAscii(aCharArray), true);
}

void AddressWalkerWriter::writeValue(double aValue)
{
    mpDocShell->GetDocFunc().SetValueCell(mCurrentAddress, aValue, true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
