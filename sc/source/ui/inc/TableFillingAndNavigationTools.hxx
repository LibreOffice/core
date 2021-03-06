/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <address.hxx>
#include <rangelst.hxx>

#include <map>
#include <vector>

class FormulaTemplate
{
private:
    OUString            mTemplate;
    ScDocument*         mpDoc;
    bool                mbUse3D;

    typedef std::map<OUString, ScRange>   RangeReplacementMap;
    typedef std::map<OUString, ScAddress> AddressReplacementMap;

    AddressReplacementMap mAddressReplacementMap;
    RangeReplacementMap   mRangeReplacementMap;

public:
    FormulaTemplate(ScDocument* pDoc);

    void      setTemplate(const OUString& aTemplate);
    void      setTemplate(const char* aTemplate);
    const OUString& getTemplate();

    void      autoReplaceRange(const OUString& aVariable, const ScRange& rRange);
    void      autoReplaceAddress(const OUString& aVariable, ScAddress const & aAddress);
    void      autoReplaceUses3D(bool bUse3D) { mbUse3D = bUse3D; }

    void      applyRange(std::u16string_view aVariable, const ScRange& aRange, bool b3D = true);
    void      applyRangeList(std::u16string_view aVariable, const ScRangeList& aRangeList, sal_Unicode cDelimiter );
    void      applyAddress(std::u16string_view aVariable, const ScAddress& aAddress, bool b3D = true);
    void      applyString(std::u16string_view aVariable, std::u16string_view aValue);
    void      applyNumber(std::u16string_view aVariable, sal_Int32 aValue);
};

class AddressWalker
{
public:
    std::vector<ScAddress> mAddressStack;

    ScAddress mCurrentAddress;
    ScAddress mMinimumAddress;
    ScAddress mMaximumAddress;

    AddressWalker(const ScAddress& aInitialAddress);

    ScAddress current(SCCOL aRelativeCol = 0, SCROW aRelativeRow = 0, SCTAB aRelativeTab = 0);

    void reset();
    void resetColumn();
    void resetRow();
    void nextColumn();
    void nextRow();
    void newLine();
    void push(SCCOL aRelativeCol = 0, SCROW aRelativeRow = 0, SCTAB aRelativeTab = 0);
};

class AddressWalkerWriter : public AddressWalker
{
public:
    ScDocShell*                         mpDocShell;
    ScDocument&                         mrDocument;
    formula::FormulaGrammar::Grammar    meGrammar;

    AddressWalkerWriter(const ScAddress& aInitialAddress, ScDocShell* pDocShell, ScDocument& rDocument,
            formula::FormulaGrammar::Grammar eGrammar );

    void writeFormula(const OUString& aFormula);
    void writeFormulas(const std::vector<OUString>& rFormulas);
    void writeMatrixFormula(const OUString& aFormula, SCCOL nCols = 1, SCROW nRows = 1);
    void writeString(const OUString& aString);
    void writeString(const char* aCharArray);
    void writeBoldString(const OUString& aString);
    void writeValue(double aValue);
};

class DataCellIterator final
{
private:
    ScRange mInputRange;
    bool    mByColumn;
    SCCOL   mCol;
    SCROW   mRow;

public:
    DataCellIterator(const ScRange& aInputRange, bool aByColumn);
    ~DataCellIterator();

    bool hasNext() const;
    ScAddress get();
    void next();
    ScAddress getRelative(int aDelta);
};

class DataRangeIterator
{
protected:
    ScRange   mInputRange;
    sal_Int32 mIndex;

public:
    DataRangeIterator(const ScRange& aInputRange);
    virtual ~DataRangeIterator();

    virtual bool hasNext() = 0;
    virtual ScRange get() = 0;
    virtual size_t size() = 0;
    virtual void next() = 0;
    virtual void reset() = 0;

    sal_Int32 index();

    virtual DataCellIterator iterateCells() = 0;
};

class DataRangeByColumnIterator final : public DataRangeIterator
{
    SCCOL mCol;

public:
    DataRangeByColumnIterator(const ScRange& aInputRange);

    virtual bool hasNext() override;
    virtual void next() override;
    virtual ScRange get() override;
    virtual size_t size() override;
    virtual void reset() override;
    virtual DataCellIterator iterateCells() override;
};

class DataRangeByRowIterator final : public DataRangeIterator
{
    SCROW mRow;

public:
    DataRangeByRowIterator(const ScRange& aInputRange);

    virtual bool hasNext() override;
    virtual void next() override;
    virtual ScRange get() override;
    virtual size_t size() override;
    virtual void reset() override;
    virtual DataCellIterator iterateCells() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
