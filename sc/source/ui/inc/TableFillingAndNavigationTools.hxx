/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef TABLEFILLINGANDNAVIGATIONTOOLS_HXX
#define TABLEFILLINGANDNAVIGATIONTOOLS_HXX

#include "address.hxx"
#include "rangelst.hxx"

#include "docsh.hxx"
#include "document.hxx"
#include "docfunc.hxx"
#include "formulacell.hxx"

#include <list>

class FormulaTemplate
{
private:
    OUString            mTemplate;
    ScDocument*         mpDoc;

public:
    FormulaTemplate(ScDocument* pDoc);

    void        setTemplate(OUString aTemplate);
    void        setTemplate(const char* aTemplate);
    const OUString& getTemplate();
    void        applyRange(const OUString& aVariable, const ScRange& aRange, bool b3D = true);
    void        applyRangeList(const OUString& aVariable, const ScRangeList& aRangeList, bool b3D = true);
    void        applyAddress(const OUString& aVariable, const ScAddress& aAddress, bool b3D = true);
    void        applyString(OUString aVariable, OUString aValue);
    void        applyNumber(OUString aVariable, sal_Int32 aValue);
};

class AddressWalker
{
public:
    std::list<ScAddress> mAddressStack;

    ScAddress mCurrentAddress;
    ScAddress mMinimumAddress;
    ScAddress mMaximumAddress;
    bool      mTrackRange;

    AddressWalker(ScAddress aInitialAddress, bool aTrackRange = true);

    ScAddress current(SCCOL aRelativeCol = 0, SCROW aRelativeRow = 0, SCTAB aRelativeTab = 0);
    void      reset();
    void      resetColumn();
    void      resetRow();
    void      nextColumn();
    void      nextRow();
    void      push(SCCOL aRelativeCol = 0, SCROW aRelativeRow = 0, SCTAB aRelativeTab = 0);
    void      pop();
};

class AddressWalkerWriter : public AddressWalker
{
public:
    ScDocShell*                         mpDocShell;
    ScDocument*                         mpDocument;
    formula::FormulaGrammar::Grammar    meGrammar;

    AddressWalkerWriter(ScAddress aInitialAddress, ScDocShell* pDocShell, ScDocument* pDocument,
            formula::FormulaGrammar::Grammar eGrammar );

    void writeFormula(OUString aFormula);
    void writeString(OUString aString);
    void writeString(const char* aCharArray);
    void writeBoldString(OUString aString);
    void writeValue(double aValue);
};

class DataCellIterator
{
private:
    ScRange mInputRange;
    bool    mByColumn;
    SCCOL   mCol;
    SCROW   mRow;

public:
    DataCellIterator(ScRange aInputRange, bool aByColumn);
    virtual ~DataCellIterator();

    virtual bool hasNext();
    virtual ScAddress get();
    virtual void next();
    virtual ScAddress getRelative(int aDelta);
};

class DataRangeIterator
{
protected:
    ScRange   mInputRange;
    sal_Int32 mIndex;

public:
    DataRangeIterator(ScRange aInputRange);
    virtual ~DataRangeIterator();

    virtual bool hasNext() = 0;
    virtual ScRange get() = 0;
    virtual void next() = 0;
    virtual void reset() = 0;
    virtual sal_Int32 index();

    virtual DataCellIterator iterateCells() = 0;
};

class DataRangeByColumnIterator : public DataRangeIterator
{
protected:
    SCCOL mCol;

public:
    DataRangeByColumnIterator(ScRange aInputRange);

    virtual bool hasNext();
    virtual void next();
    virtual ScRange get();
    virtual void reset();
    virtual DataCellIterator iterateCells();
};

class DataRangeByRowIterator : public DataRangeIterator
{
protected:
    SCROW mRow;

public:
    DataRangeByRowIterator(ScRange aInputRange);

    virtual bool hasNext();
    virtual void next();
    virtual ScRange get();
    virtual void reset();
    virtual DataCellIterator iterateCells();
};

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
