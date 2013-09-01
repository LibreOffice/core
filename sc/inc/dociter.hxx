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

#ifndef SC_DOCITER_HXX
#define SC_DOCITER_HXX

#include "address.hxx"
#include <tools/solar.h>
#include "global.hxx"
#include "scdllapi.h"
#include "cellvalue.hxx"
#include "mtvelements.hxx"

#include <memory>

#include <set>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

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

class ScValueIterator            // walk through all values in an area
{
    typedef std::pair<sc::CellStoreType::const_iterator, size_t> PositionType;

    ScDocument*     pDoc;
    const ScAttrArray*  pAttrArray;
    sal_uLong           nNumFormat;     // for CalcAsShown
    sal_uLong           nNumFmtIndex;
    ScAddress maStartPos;
    ScAddress maEndPos;
    SCCOL mnCol;
    SCTAB mnTab;
    SCROW           nAttrEndRow;
    short           nNumFmtType;
    bool            bNumValid:1;
    bool            bSubTotal:1;
    bool            bCalcAsShown:1;
    bool            bTextAsZero:1;

    const sc::CellStoreType* mpCells;
    PositionType maCurPos;

    SCROW GetRow() const;
    void IncBlock();
    void IncPos();
    void SetPos(size_t nPos);

    /**
     * See if the cell at the current position is a non-empty cell. If not,
     * move to the next non-empty cell position.
     */
    bool GetThis( double& rValue, sal_uInt16& rErr );

public:

    ScValueIterator(
        ScDocument* pDocument, const ScRange& rRange, bool bSTotal = false,
        bool bTextAsZero = false );

    void GetCurNumFmtInfo( short& nType, sal_uLong& nIndex );

    /// Does NOT reset rValue if no value found!
    bool GetFirst( double& rValue, sal_uInt16& rErr );

    /// Does NOT reset rValue if no value found!
    bool GetNext( double& rValue, sal_uInt16& rErr );
};

class ScDBQueryDataIterator
{
public:
    struct Value
    {
        OUString maString;
        double          mfValue;
        sal_uInt16      mnError;
        bool            mbIsNumber;

        Value();
    };

private:
    static const sc::CellStoreType* GetColumnCellStore(ScDocument& rDoc, SCTAB nTab, SCCOL nCol);
    static const ScAttrArray* GetAttrArrayByCol(ScDocument& rDoc, SCTAB nTab, SCCOL nCol);
    static bool IsQueryValid(ScDocument& rDoc, const ScQueryParam& rParam, SCTAB nTab, SCROW nRow, ScRefCellValue& rCell);

    class DataAccess
    {
    public:
        DataAccess(const ScDBQueryDataIterator* pParent);
        virtual ~DataAccess() = 0;
        virtual bool getCurrent(Value& rValue) = 0;
        virtual bool getFirst(Value& rValue) = 0;
        virtual bool getNext(Value& rValue) = 0;
    protected:
        const ScDBQueryDataIterator* mpParent;
    };

    class DataAccessInternal : public DataAccess
    {
        typedef std::pair<sc::CellStoreType::const_iterator,size_t> PositionType;
    public:
        DataAccessInternal(const ScDBQueryDataIterator* pParent, ScDBQueryParamInternal* pParam, ScDocument* pDoc);
        virtual ~DataAccessInternal();
        virtual bool getCurrent(Value& rValue);
        virtual bool getFirst(Value& rValue);
        virtual bool getNext(Value& rValue);

    private:
        void incBlock();
        void incPos();
        void setPos(size_t nPos);

        const sc::CellStoreType* mpCells;
        PositionType maCurPos;
        ScDBQueryParamInternal* mpParam;
        ScDocument*         mpDoc;
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
        DataAccessMatrix(const ScDBQueryDataIterator* pParent, ScDBQueryParamMatrix* pParam);
        virtual ~DataAccessMatrix();
        virtual bool getCurrent(Value& rValue);
        virtual bool getFirst(Value& rValue);
        virtual bool getNext(Value& rValue);

    private:
        bool isValidQuery(SCROW mnRow, const ScMatrix& rMat) const;

        ScDBQueryParamMatrix* mpParam;
        SCROW mnCurRow;
        SCROW mnRows;
        SCCOL mnCols;
    };

    ::std::auto_ptr<ScDBQueryParamBase> mpParam;
    ::std::auto_ptr<DataAccess>         mpData;

public:
                    ScDBQueryDataIterator(ScDocument* pDocument, ScDBQueryParamBase* pParam);
    /// Does NOT reset rValue if no value found!
    bool            GetFirst(Value& rValue);
    /// Does NOT reset rValue if no value found!
    bool            GetNext(Value& rValue);
};

/**
 * Walk through all cells in an area. For SubTotal no hidden and no
 * sub-total lines.
 **/
class ScCellIterator
{
    typedef std::pair<sc::CellStoreType::const_iterator, size_t> PositionType;

    ScDocument* mpDoc;
    ScAddress maStartPos;
    ScAddress maEndPos;
    ScAddress maCurPos;

    PositionType maCurColPos;
    bool mbSubTotal;

    ScRefCellValue maCurCell;

    void incBlock();
    void incPos();
    void setPos(size_t nPos);

    const ScColumn* getColumn() const;

    void init();
    bool getCurrent();

public:
    ScCellIterator( ScDocument* pDoc, const ScRange& rRange, bool bSTotal = false );

    const ScAddress& GetPos() const { return maCurPos; }

    CellType getType() const;
    OUString getString();
    const EditTextObject* getEditText() const;
    ScFormulaCell* getFormulaCell();
    const ScFormulaCell* getFormulaCell() const;
    double getValue();
    ScCellValue getCellValue() const;
    const ScRefCellValue& getRefCellValue() const;

    bool hasString() const;
    bool hasNumeric() const;
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

    typedef std::pair<sc::CellStoreType::const_iterator, size_t> PositionType;
    PositionType maCurPos;

    boost::scoped_ptr<ScQueryParam> mpParam;
    ScDocument*     pDoc;
    const ScAttrArray*  pAttrArray;
    sal_uLong           nNumFormat;
    SCTAB           nTab;
    SCCOL           nCol;
    SCROW           nRow;
    SCROW           nAttrEndRow;
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
                    ScQueryCellIterator(ScDocument* pDocument, SCTAB nTable,
                                        const ScQueryParam& aParam, bool bMod = true);
                                        // for bMod = FALSE the QueryParam has to be filled
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

                        @param bSearchForEqualAfterMismatch
                            Continue searching for an equal entry even if the
                            last entry matching the range was found, in case
                            the data is not sorted. Is always done if regular
                            expressions are involved.

                        @param bIgnoreMismatchOnLeadingStrings
                            Normally strings are sorted behind numerical
                            values. If this parameter is true, the search does
                            not stop when encountering a string and does not
                            assume that no values follow anymore.
                            If querying for a string a mismatch on the first
                            entry, e.g. column header, is ignored.

                        @ATTENTION! StopOnMismatch, TestEqualCondition and
                        the internal IgnoreMismatchOnLeadingStrings and query
                        params are in an undefined state upon return! The
                        iterator is not usable anymore except for obtaining the
                        number format!
                      */
    bool            FindEqualOrSortedLastInRange( SCCOL& nFoundCol,
                        SCROW& nFoundRow, bool bSearchForEqualAfterMismatch = false,
                        bool bIgnoreMismatchOnLeadingStrings = true );
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
    ScAttrIterator* pColIter;

public:
                    ScDocAttrIterator(ScDocument* pDocument, SCTAB nTable,
                                    SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2);
                    ~ScDocAttrIterator();

    const ScPatternAttr*    GetNext( SCCOL& rCol, SCROW& rRow1, SCROW& rRow2 );
};

class ScAttrRectIterator            // all attribute areas, including areas stretching
                                    // across more then one column
{
private:
    ScDocument*     pDoc;
    SCTAB           nTab;
    SCCOL           nEndCol;
    SCROW           nStartRow;
    SCROW           nEndRow;
    SCCOL           nIterStartCol;
    SCCOL           nIterEndCol;
    ScAttrIterator* pColIter;

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
    };

    std::vector<ColParam> maColPositions;

    ScDocument*     pDoc;
    SCTAB           mnTab;
    SCCOL           nStartCol;
    SCCOL           nEndCol;
    SCROW           nStartRow;
    SCROW           nEndRow;
    SCROW*          pNextRows;
    SCSIZE*         pNextIndices;
    SCCOL           mnCol;
    SCROW           mnRow;
    ScRefCellValue  maCurCell;
    bool            bMore;

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
};


/** Row-wise value iterator. */
class ScHorizontalValueIterator
{
private:
    ScDocument               *pDoc;
    const ScAttrArray        *pAttrArray;
    ScHorizontalCellIterator *pCellIter;
    sal_uLong                 nNumFormat;     // for CalcAsShown
    sal_uLong                 nNumFmtIndex;
    SCTAB                     nEndTab;
    SCCOL                     nCurCol;
    SCROW                     nCurRow;
    SCTAB                     nCurTab;
    SCROW                     nAttrEndRow;
    short                     nNumFmtType;
    bool                      bNumValid;
    bool                      bSubTotal;
    bool                      bCalcAsShown;
    bool                      bTextAsZero;

public:

                    ScHorizontalValueIterator( ScDocument* pDocument,
                                               const ScRange& rRange,
                                               bool bSTotal = false,
                                               bool bTextAsZero = false );
                    ~ScHorizontalValueIterator();
    /// Does NOT reset rValue if no value found!
    bool            GetNext( double& rValue, sal_uInt16& rErr );
};


//
//  returns all areas with non-default formatting (horizontal)
//

class ScHorizontalAttrIterator
{
private:
    ScDocument*             pDoc;
    SCTAB                   nTab;
    SCCOL                   nStartCol;
    SCROW                   nStartRow;
    SCCOL                   nEndCol;
    SCROW                   nEndRow;

    SCROW*                  pNextEnd;
    SCSIZE*                 pIndices;
    const ScPatternAttr**   ppPatterns;
    SCCOL                   nCol;
    SCROW                   nRow;
    bool                    bRowEmpty;

public:
            ScHorizontalAttrIterator( ScDocument* pDocument, SCTAB nTable,
                                    SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 );
            ~ScHorizontalAttrIterator();

    const ScPatternAttr*    GetNext( SCCOL& rCol1, SCCOL& rCol2, SCROW& rRow );
};

//
//  returns non-empty cells and areas with formatting (horizontal)
//

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
    const ScRefCellValue&   GetCell() const;
};

class ScRowBreakIterator
{
public:
    static SCROW NOT_FOUND;

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
        ::boost::shared_ptr<ScFlatBoolRowSegments> mpRanges;

        TabRanges(SCTAB nTab);
    };

    /**
     * Passing a NULL pointer to pTabRangesArray forces the heights of all
     * rows in all tables to be updated.
     */
    explicit ScDocRowHeightUpdater(
        ScDocument& rDoc, OutputDevice* pOutDev, double fPPTX, double fPPTY,
        const ::std::vector<TabRanges>* pTabRangesArray = NULL);

    void update();

private:
    void updateAll();

private:
    ScDocument& mrDoc;
    OutputDevice* mpOutDev;
    double mfPPTX;
    double mfPPTY;
    const ::std::vector<TabRanges>* mpTabRangesArray;
};


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
