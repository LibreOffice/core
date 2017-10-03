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

#ifndef INCLUDED_SC_INC_DOCITER_HXX
#define INCLUDED_SC_INC_DOCITER_HXX

#include "address.hxx"
#include "formulagroup.hxx"
#include <tools/solar.h>
#include "global.hxx"
#include "scdllapi.h"
#include "cellvalue.hxx"
#include "mtvelements.hxx"
#include <vcl/vclptr.hxx>

#include <memory>
#include <set>
#include <vector>

class ScDocument;
class ScPatternAttr;
class ScAttrArray;
class ScAttrIterator;
class ScRange;
class ScFlatBoolRowSegments;
class ScMatrix;
struct ScDBQueryParamBase;
struct ScQueryParam;
struct ScDBQueryParamInternal;
struct ScDBQueryParamMatrix;
class ScFormulaCell;
struct ScInterpreterContext;

class ScValueIterator            // walk through all values in an area
{
    typedef sc::CellStoreType::const_position_type PositionType;

    ScDocument*     pDoc;
    const ScAttrArray*  pAttrArray;
    sal_uLong           nNumFormat;     // for CalcAsShown
    sal_uLong           nNumFmtIndex;
    ScAddress       maStartPos;
    ScAddress       maEndPos;
    SCCOL           mnCol;
    SCTAB           mnTab;
    SCROW           nAttrEndRow;
    SubtotalFlags   mnSubTotalFlags;
    short           nNumFmtType;
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

    ScValueIterator(
        ScDocument* pDocument, const ScRange& rRange, SubtotalFlags nSubTotalFlags = SubtotalFlags::NONE,
        bool bTextAsZero = false );

    void GetCurNumFmtInfo( const ScInterpreterContext& rContext, short& nType, sal_uLong& nIndex );

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

    class DataAccessInternal : public DataAccess
    {
        typedef std::pair<sc::CellStoreType::const_iterator,size_t> PositionType;
    public:
        DataAccessInternal(ScDBQueryParamInternal* pParam, ScDocument* pDoc, const ScInterpreterContext& rContext);
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
        ScDocument*         mpDoc;
        const ScInterpreterContext& mrContext;
        const ScAttrArray*  pAttrArray;
        sal_uLong               nNumFormat;     // for CalcAsShown
        sal_uLong               nNumFmtIndex;
        SCCOL               nCol;
        SCROW               nRow;
        SCROW               nAttrEndRow;
        SCTAB               nTab;
        short               nNumFmtType;
        bool                bCalcAsShown;
    };

    class DataAccessMatrix : public DataAccess
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
                    ScDBQueryDataIterator(ScDocument* pDocument, const ScInterpreterContext& rContext, ScDBQueryParamBase* pParam);
    /// Does NOT reset rValue if no value found!
    bool            GetFirst(Value& rValue);
    /// Does NOT reset rValue if no value found!
    bool            GetNext(Value& rValue);
};

class ScFormulaGroupIterator
{
private:
    ScDocument* mpDoc;
    SCTAB mnTab;
    SCCOL mnCol;
    bool mbNullCol;
    size_t mnIndex;
    std::vector<sc::FormulaGroupEntry> maEntries;

public:
    ScFormulaGroupIterator( ScDocument* pDoc );

    sc::FormulaGroupEntry* first();
    sc::FormulaGroupEntry* next();
};

/**
 * Walk through all cells in an area. For SubTotal and Aggregate depending on mnSubTotalFlags.
 **/
class ScCellIterator
{
    typedef std::pair<sc::CellStoreType::const_iterator, size_t> PositionType;

    ScDocument*   mpDoc;
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
    ScCellIterator( ScDocument* pDoc, const ScRange& rRange, SubtotalFlags nSubTotalFlags = SubtotalFlags::NONE );

    const ScAddress& GetPos() const { return maCurPos; }

    CellType getType() const { return maCurCell.meType;}
    OUString getString();
    const EditTextObject* getEditText() const { return maCurCell.mpEditText;}
    ScFormulaCell* getFormulaCell() { return maCurCell.mpFormula;}
    const ScFormulaCell* getFormulaCell() const { return maCurCell.mpFormula;}
    ScCellValue getCellValue() const;
    const ScRefCellValue& getRefCellValue() const { return maCurCell;}

    bool hasString() const;
    bool hasEmptyData() const;
    bool isEmpty() const;
    bool equalsWithoutFormat( const ScAddress& rPos ) const;

    bool first();
    bool next();
};

class ScQueryCellIterator           // walk through all non-empty cells in an area
{
    enum StopOnMismatchBits
    {
        nStopOnMismatchDisabled = 0x00,
        nStopOnMismatchEnabled  = 0x01,
        nStopOnMismatchOccurred  = 0x02,
        nStopOnMismatchExecuted = nStopOnMismatchEnabled | nStopOnMismatchOccurred
    };

    enum TestEqualConditionBits
    {
        nTestEqualConditionDisabled = 0x00,
        nTestEqualConditionEnabled  = 0x01,
        nTestEqualConditionMatched  = 0x02,
        nTestEqualConditionFulfilled = nTestEqualConditionEnabled | nTestEqualConditionMatched
    };

    typedef sc::CellStoreType::const_position_type PositionType;
    PositionType maCurPos;

    std::unique_ptr<ScQueryParam> mpParam;
    ScDocument*     pDoc;
    const ScInterpreterContext& mrContext;
    SCTAB           nTab;
    SCCOL           nCol;
    SCROW           nRow;
    sal_uInt8            nStopOnMismatch;
    sal_uInt8            nTestEqualCondition;
    bool            bAdvanceQuery;
    bool            bIgnoreMismatchOnLeadingStrings;

    /** Initialize position for new column. */
    void InitPos();
    void IncPos();
    void IncBlock();
    bool GetThis();

                    /* Only works if no regular expression is involved, only
                       searches for rows in one column, and only the first
                       query entry is considered with simple conditions
                       SC_LESS_EQUAL (sorted ascending) or SC_GREATER_EQUAL
                       (sorted descending). Check these things before
                       invocation! Delivers a starting point, continue with
                       GetThis() and GetNext() afterwards. Introduced for
                       FindEqualOrSortedLastInRange()
                     */
    bool BinarySearch();

public:
                    ScQueryCellIterator(ScDocument* pDocument, const ScInterpreterContext& rContext, SCTAB nTable,
                                        const ScQueryParam& aParam, bool bMod);
                                        // when !bMod, the QueryParam has to be filled
                                        // (bIsString)
    bool GetFirst();
    bool GetNext();
    SCCOL           GetCol() { return nCol; }
    SCROW           GetRow() { return nRow; }

                    // increments all Entry.nField, if column
                    // changes, for ScInterpreter ScHLookup()
    void            SetAdvanceQueryParamEntryField( bool bVal )
                        { bAdvanceQuery = bVal; }
    void            AdvanceQueryParamEntryField();

                    /** If set, iterator stops on first non-matching cell
                        content. May be used in SC_LESS_EQUAL queries where a
                        cell range is assumed to be sorted; stops on first
                        value being greater than the queried value and
                        GetFirst()/GetNext() return NULL. StoppedOnMismatch()
                        returns true then.
                        However, the iterator's conditions are not set to end
                        all queries, GetCol() and GetRow() return values for
                        the non-matching cell, further GetNext() calls may be
                        executed. */
    void            SetStopOnMismatch( bool bVal )
                        {
                            nStopOnMismatch = sal::static_int_cast<sal_uInt8>(bVal ? nStopOnMismatchEnabled :
                                nStopOnMismatchDisabled);
                        }
    bool            StoppedOnMismatch() const
                        { return nStopOnMismatch == nStopOnMismatchExecuted; }

                    /** If set, an additional test for SC_EQUAL condition is
                        executed in ScTable::ValidQuery() if SC_LESS_EQUAL or
                        SC_GREATER_EQUAL conditions are to be tested. May be
                        used where a cell range is assumed to be sorted to stop
                        if an equal match is found. */
    void            SetTestEqualCondition( bool bVal )
                        {
                            nTestEqualCondition = sal::static_int_cast<sal_uInt8>(bVal ?
                                nTestEqualConditionEnabled :
                                nTestEqualConditionDisabled);
                        }
    bool            IsEqualConditionFulfilled() const
                        { return nTestEqualCondition == nTestEqualConditionFulfilled; }

                    /** In a range assumed to be sorted find either the last of
                        a sequence of equal entries or the last being less than
                        (or greater than) the queried value. Used by the
                        interpreter for [HV]?LOOKUP() and MATCH(). Column and
                        row position of the found entry are returned, otherwise
                        invalid.

                        The search does not stop when encountering a string and does not
                        assume that no values follow anymore.
                        If querying for a string a mismatch on the first
                        entry, e.g. column header, is ignored.

                        @ATTENTION! StopOnMismatch, TestEqualCondition and
                        the internal IgnoreMismatchOnLeadingStrings and query
                        params are in an undefined state upon return! The
                        iterator is not usable anymore except for obtaining the
                        number format!
                      */
    bool            FindEqualOrSortedLastInRange( SCCOL& nFoundCol, SCROW& nFoundRow );
};

class ScDocAttrIterator             // all attribute areas
{
private:
    ScDocument*     pDoc;
    SCTAB           nTab;
    SCCOL           nEndCol;
    SCROW           nStartRow;
    SCROW           nEndRow;
    SCCOL           nCol;
    std::unique_ptr<ScAttrIterator>
                    pColIter;

public:
                    ScDocAttrIterator(ScDocument* pDocument, SCTAB nTable,
                                    SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2);
                    ~ScDocAttrIterator();

    const ScPatternAttr*    GetNext( SCCOL& rCol, SCROW& rRow1, SCROW& rRow2 );
};

class ScAttrRectIterator            // all attribute areas, including areas stretching
                                    // across more than one column
{
private:
    ScDocument*     pDoc;
    SCTAB           nTab;
    SCCOL           nEndCol;
    SCROW           nStartRow;
    SCROW           nEndRow;
    SCCOL           nIterStartCol;
    SCCOL           nIterEndCol;
    std::unique_ptr<ScAttrIterator>
                    pColIter;

public:
                    ScAttrRectIterator(ScDocument* pDocument, SCTAB nTable,
                                    SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2);
                    ~ScAttrRectIterator();

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

    ScDocument*     pDoc;
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
    ScHorizontalCellIterator(ScDocument* pDocument, SCTAB nTable,
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
    ScDocument               *pDoc;
    const ScAttrArray        *pAttrArray;
    std::unique_ptr<ScHorizontalCellIterator>
                              pCellIter;
    sal_uLong                 nNumFormat;     // for CalcAsShown
    SCTAB                     nEndTab;
    SCCOL                     nCurCol;
    SCROW                     nCurRow;
    SCTAB                     nCurTab;
    SCROW                     nAttrEndRow;
    bool                      bCalcAsShown;

public:

                    ScHorizontalValueIterator( ScDocument* pDocument,
                                               const ScRange& rRange );
                    ~ScHorizontalValueIterator();
    /// Does NOT reset rValue if no value found!
    bool            GetNext( double& rValue, FormulaError& rErr );
};

//  returns all areas with non-default formatting (horizontal)

class ScHorizontalAttrIterator
{
private:
    ScDocument*             pDoc;
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
    bool                    bRowEmpty;
    SCROW                   nMinNextEnd;

    void InitForNextRow(bool bInitialization);
    bool InitForNextAttr();

public:
            ScHorizontalAttrIterator( ScDocument* pDocument, SCTAB nTable,
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
            ScUsedAreaIterator( ScDocument* pDocument, SCTAB nTable,
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
        std::shared_ptr<ScFlatBoolRowSegments> mpRanges;

        TabRanges(SCTAB nTab);
    };

    /**
     * Passing a NULL pointer to pTabRangesArray forces the heights of all
     * rows in all tables to be updated.
     */
    explicit ScDocRowHeightUpdater(
        ScDocument& rDoc, OutputDevice* pOutDev, double fPPTX, double fPPTY,
        const ::std::vector<TabRanges>* pTabRangesArray);

    void update();

private:
    void updateAll();

private:
    ScDocument& mrDoc;
    VclPtr<OutputDevice> mpOutDev;
    double mfPPTX;
    double mfPPTY;
    const ::std::vector<TabRanges>* mpTabRangesArray;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
