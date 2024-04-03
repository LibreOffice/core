/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once

#include "address.hxx"
#include "formulagroup.hxx"
#include "global.hxx"
#include "scdllapi.h"
#include "cellvalue.hxx"
#include "mtvelements.hxx"
#include "queryparam.hxx"
#include "attarray.hxx"
#include <vcl/outdev.hxx>
#include <vcl/vclptr.hxx>

#include <memory>
#include <set>
#include <vector>
#include <optional>

class ScDocument;
class ScPatternAttr;
class ScAttrArray;
class ScAttrIterator;
class ScFlatBoolRowSegments;
class ScMatrix;
struct ScDBQueryParamBase;
struct ScQueryParam;
struct ScDBQueryParamInternal;
struct ScDBQueryParamMatrix;
class ScFormulaCell;
class OutputDevice;
struct ScInterpreterContext;
enum class SvNumFormatType : sal_Int16;

class ScValueIterator            // walk through all values in an area
{
    typedef sc::CellStoreType::const_position_type PositionType;

    const ScDocument& mrDoc;
    ScInterpreterContext& mrContext;
    const ScAttrArray*  pAttrArray;
    sal_uInt32      nNumFormat;     // for CalcAsShown
    sal_uInt32      nNumFmtIndex;
    ScAddress       maStartPos;
    ScAddress       maEndPos;
    SCCOL           mnCol;
    SCTAB           mnTab;
    SCROW           nAttrEndRow;
    SubtotalFlags   mnSubTotalFlags;
    SvNumFormatType nNumFmtType;
    bool            bNumValid;
    bool            bCalcAsShown;
    bool            bTextAsZero;

    const sc::CellStoreType* mpCells;
    PositionType maCurPos;

    SCROW GetRow() const;
    void IncBlock();
    void IncPos();

    /**
     * See if the cell at the current position is a non-empty cell. If not,
     * move to the next non-empty cell position.
     */
    bool GetThis( double& rValue, FormulaError& rErr );

public:

    ScValueIterator(ScInterpreterContext& rContext,
        const ScRange& rRange, SubtotalFlags nSubTotalFlags = SubtotalFlags::NONE,
        bool bTextAsZero = false );

    void GetCurNumFmtInfo( SvNumFormatType& nType, sal_uInt32& nIndex );

    /// Does NOT reset rValue if no value found!
    bool GetFirst( double& rValue, FormulaError& rErr );

    /// Does NOT reset rValue if no value found!
    bool GetNext( double& rValue, FormulaError& rErr );
};

class ScDBQueryDataIterator
{
public:
    struct Value
    {
        OUString        maString;
        double          mfValue;
        FormulaError    mnError;
        bool            mbIsNumber;

        Value();
    };

private:
    static const sc::CellStoreType* GetColumnCellStore(ScDocument& rDoc, SCTAB nTab, SCCOL nCol);
    static const ScAttrArray* GetAttrArrayByCol(ScDocument& rDoc, SCTAB nTab, SCCOL nCol);
    static bool IsQueryValid(ScDocument& rDoc, const ScQueryParam& rParam, SCTAB nTab, SCROW nRow, const ScRefCellValue* pCell);

    class DataAccess
    {
    public:
        DataAccess();
        virtual ~DataAccess() = 0;
        virtual bool getCurrent(Value& rValue) = 0;
        virtual bool getFirst(Value& rValue) = 0;
        virtual bool getNext(Value& rValue) = 0;
    };

    class DataAccessInternal final : public DataAccess
    {
        typedef std::pair<sc::CellStoreType::const_iterator,size_t> PositionType;
    public:
        DataAccessInternal(ScDBQueryParamInternal* pParam, ScDocument& rDoc, const ScInterpreterContext& rContext);
        virtual ~DataAccessInternal() override;
        virtual bool getCurrent(Value& rValue) override;
        virtual bool getFirst(Value& rValue) override;
        virtual bool getNext(Value& rValue) override;

    private:
        void incBlock();
        void incPos();

        const sc::CellStoreType* mpCells;
        PositionType maCurPos;
        ScDBQueryParamInternal* mpParam;
        ScDocument&         mrDoc;
        const ScInterpreterContext& mrContext;
        const ScAttrArray*  pAttrArray;
        sal_uInt32          nNumFormat;     // for CalcAsShown
        sal_uInt32          nNumFmtIndex;
        SCCOL               nCol;
        SCROW               nRow;
        SCROW               nAttrEndRow;
        SCTAB               nTab;
        SvNumFormatType     nNumFmtType;
        bool                bCalcAsShown;
    };

    class DataAccessMatrix final : public DataAccess
    {
    public:
        DataAccessMatrix(ScDBQueryParamMatrix* pParam);
        virtual ~DataAccessMatrix() override;
        virtual bool getCurrent(Value& rValue) override;
        virtual bool getFirst(Value& rValue) override;
        virtual bool getNext(Value& rValue) override;

    private:
        bool isValidQuery(SCROW mnRow, const ScMatrix& rMat) const;

        ScDBQueryParamMatrix* mpParam;
        SCROW mnCurRow;
        SCROW mnRows;
    };

    ::std::unique_ptr<ScDBQueryParamBase> mpParam;
    ::std::unique_ptr<DataAccess>         mpData;

public:
                    ScDBQueryDataIterator(ScDocument& rDocument, const ScInterpreterContext& rContext, std::unique_ptr<ScDBQueryParamBase> pParam);
    /// Does NOT reset rValue if no value found!
    bool            GetFirst(Value& rValue);
    /// Does NOT reset rValue if no value found!
    bool            GetNext(Value& rValue);
};

class ScFormulaGroupIterator
{
private:
    ScDocument& mrDoc;
    SCTAB mnTab;
    SCCOL mnCol;
    bool mbNullCol;
    size_t mnIndex;
    std::vector<sc::FormulaGroupEntry> maEntries;

public:
    ScFormulaGroupIterator( ScDocument& rDoc );

    sc::FormulaGroupEntry* first();
    sc::FormulaGroupEntry* next();
};

/**
 * Walk through all cells in an area. For SubTotal and Aggregate depending on mnSubTotalFlags.
 **/
class ScCellIterator
{
    typedef std::pair<sc::CellStoreType::const_iterator, size_t> PositionType;

    ScDocument&   mrDoc;
    ScAddress     maStartPos;
    ScAddress     maEndPos;
    ScAddress     maCurPos;

    PositionType  maCurColPos;
    SubtotalFlags mnSubTotalFlags;

    ScRefCellValue maCurCell;

    void incBlock();
    void incPos();
    void setPos(size_t nPos);

    const ScColumn* getColumn() const;

    void init();
    bool getCurrent();

public:
    ScCellIterator( ScDocument& rDoc, const ScRange& rRange, SubtotalFlags nSubTotalFlags = SubtotalFlags::NONE );

    const ScAddress& GetPos() const { return maCurPos; }

    CellType getType() const { return maCurCell.getType();}
    OUString getString() const;
    const EditTextObject* getEditText() const { return maCurCell.getEditText();}
    ScFormulaCell* getFormulaCell() { return maCurCell.getFormula();}
    const ScFormulaCell* getFormulaCell() const { return maCurCell.getFormula();}
    ScCellValue getCellValue() const;
    const ScRefCellValue& getRefCellValue() const { return maCurCell;}

    bool hasString() const;
    bool isEmpty() const;
    bool equalsWithoutFormat( const ScAddress& rPos ) const;

    bool first();
    bool next();
};

class ScDocAttrIterator             // all attribute areas
{
private:
    ScDocument&     rDoc;
    SCTAB           nTab;
    SCCOL           nEndCol;
    SCROW           nStartRow;
    SCROW           nEndRow;
    SCCOL           nCol;
    std::optional<ScAttrIterator> moColIter;

public:
                    ScDocAttrIterator(ScDocument& rDocument, SCTAB nTable,
                                    SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2);

    const ScPatternAttr*    GetNext( SCCOL& rCol, SCROW& rRow1, SCROW& rRow2 );
};

class ScAttrRectIterator            // all attribute areas, including areas stretching
                                    // across more than one column
{
private:
    ScDocument&     rDoc;
    SCTAB           nTab;
    SCCOL           nEndCol;
    SCROW           nStartRow;
    SCROW           nEndRow;
    SCCOL           nIterStartCol;
    SCCOL           nIterEndCol;
    std::optional<ScAttrIterator>
                    moColIter;

public:
                    ScAttrRectIterator(ScDocument& rDocument, SCTAB nTable,
                                    SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2);

    void                    DataChanged();
    const ScPatternAttr*    GetNext( SCCOL& rCol1, SCCOL& rCol2, SCROW& rRow1, SCROW& rRow2 );
};

class ScHorizontalCellIterator      // walk through all non empty cells in an area
{                                   // row by row
    struct ColParam
    {
        sc::CellStoreType::const_iterator maPos;
        sc::CellStoreType::const_iterator maEnd;
        SCCOL mnCol;
    };

    std::vector<ColParam>::iterator maColPos;
    std::vector<ColParam> maColPositions;

    ScDocument&     rDoc;
    SCTAB           mnTab;
    SCCOL           nStartCol;
    SCCOL           nEndCol;
    SCROW           nStartRow;
    SCROW           nEndRow;
    SCCOL           mnCol;
    SCROW           mnRow;
    ScRefCellValue  maCurCell;
    bool            mbMore;

public:
    ScHorizontalCellIterator(ScDocument& rDocument, SCTAB nTable,
                    SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2);
    ~ScHorizontalCellIterator();

    ScRefCellValue* GetNext( SCCOL& rCol, SCROW& rRow );
    bool            GetPos( SCCOL& rCol, SCROW& rRow );
    /// Set a(nother) sheet and (re)init.
    void            SetTab( SCTAB nTab );

private:
    void            Advance();
    void            SkipInvalid();
    bool            SkipInvalidInRow();
    SCROW           FindNextNonEmptyRow();
};

/** Row-wise value iterator. */
class ScHorizontalValueIterator
{
private:
    ScDocument&               rDoc;
    const ScAttrArray*        pAttrArray;
    std::unique_ptr<ScHorizontalCellIterator>
                              pCellIter;
    sal_uInt32                nNumFormat;     // for CalcAsShown
    SCTAB                     nEndTab;
    SCCOL                     nCurCol;
    SCROW                     nCurRow;
    SCTAB                     nCurTab;
    SCROW                     nAttrEndRow;
    bool                      bCalcAsShown;

public:

                    ScHorizontalValueIterator( ScDocument& rDocument,
                                               const ScRange& rRange );
                    ~ScHorizontalValueIterator();
    /// Does NOT reset rValue if no value found!
    bool            GetNext( double& rValue, FormulaError& rErr );
};

class ScHorizontalAttrIterator
{
private:
    ScDocument&             rDoc;
    SCTAB                   nTab;
    SCCOL                   nStartCol;
    SCROW                   nStartRow;
    SCCOL                   nEndCol;
    SCROW                   nEndRow;

    std::unique_ptr<SCROW[]>  pNextEnd;
    std::unique_ptr<SCCOL[]>  pHorizEnd;
    std::unique_ptr<SCSIZE[]> pIndices;
    std::unique_ptr<const ScPatternAttr*[]>
                              ppPatterns;
    SCCOL                   nCol;
    SCROW                   nRow;
    SCROW                   nMinNextEnd;

    void InitForNextRow(bool bInitialization);

public:
            ScHorizontalAttrIterator( ScDocument& rDocument, SCTAB nTable,
                                    SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 );
            ~ScHorizontalAttrIterator();

    const ScPatternAttr*    GetNext( SCCOL& rCol1, SCCOL& rCol2, SCROW& rRow );
};

//  returns non-empty cells and areas with formatting (horizontal)

class SC_DLLPUBLIC ScUsedAreaIterator
{
private:
    ScHorizontalCellIterator    aCellIter;
    ScHorizontalAttrIterator    aAttrIter;

    SCCOL                   nNextCol;
    SCROW                   nNextRow;

    SCCOL                   nCellCol;
    SCROW                   nCellRow;
    ScRefCellValue*         pCell;
    SCCOL                   nAttrCol1;
    SCCOL                   nAttrCol2;
    SCROW                   nAttrRow;
    const ScPatternAttr*    pPattern;

    SCCOL                   nFoundStartCol;         // results after GetNext
    SCCOL                   nFoundEndCol;
    SCROW                   nFoundRow;
    const ScPatternAttr*    pFoundPattern;

    ScRefCellValue maFoundCell;

public:
            ScUsedAreaIterator( ScDocument& rDocument, SCTAB nTable,
                                SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 );
            ~ScUsedAreaIterator();

    bool    GetNext();

    SCCOL                   GetStartCol() const     { return nFoundStartCol; }
    SCCOL                   GetEndCol() const       { return nFoundEndCol; }
    SCROW                   GetRow() const          { return nFoundRow; }
    const ScPatternAttr*    GetPattern() const      { return pFoundPattern; }
    const ScRefCellValue&   GetCell() const { return maFoundCell;}
};

class ScRowBreakIterator
{
public:
    static constexpr SCROW NOT_FOUND = -1;

    explicit ScRowBreakIterator(::std::set<SCROW>& rBreaks);
    SCROW first();
    SCROW next();

private:
    ::std::set<SCROW>& mrBreaks;
    ::std::set<SCROW>::const_iterator maItr;
    ::std::set<SCROW>::const_iterator maEnd;
};

class ScDocRowHeightUpdater
{
public:
    struct TabRanges
    {
        SCTAB mnTab;
        ScFlatBoolRowSegments maRanges;

        TabRanges(SCTAB nTab, SCROW nMaxRow);
    };

    /**
     * Passing a NULL pointer to pTabRangesArray forces the heights of all
     * rows in all tables to be updated.
     */
    explicit ScDocRowHeightUpdater(
        ScDocument& rDoc, OutputDevice* pOutDev, double fPPTX, double fPPTY,
        const ::std::vector<TabRanges>* pTabRangesArray);

    void update(const bool bOnlyUsedRows = false);

private:
    void updateAll(const bool bOnlyUsedRows);

private:
    ScDocument& mrDoc;
    VclPtr<OutputDevice> mpOutDev;
    double mfPPTX;
    double mfPPTY;
    const ::std::vector<TabRanges>* mpTabRangesArray;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
