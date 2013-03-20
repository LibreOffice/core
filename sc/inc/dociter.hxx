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
#if 1

#include "address.hxx"
#include <tools/solar.h>
#include "global.hxx"
#include "scdllapi.h"

#include <memory>

#include <set>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

class ScDocument;
class ScBaseCell;
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

class ScDocumentIterator                // walk through all non-empty cells
{
private:
    ScDocument*             pDoc;
    SCTAB                   nStartTab;
    SCTAB                   nEndTab;

    const ScPatternAttr*    pDefPattern;

    SCCOL                   nCol;
    SCROW                   nRow;
    SCTAB                   nTab;
    ScBaseCell*             pCell;
    const ScPatternAttr*    pPattern;


    SCSIZE                  nColPos;
    SCSIZE                  nAttrPos;

    bool                    GetThis();
    bool                    GetThisCol();

public:
            ScDocumentIterator( ScDocument* pDocument, SCTAB nStartTable, SCTAB nEndTable );
            ~ScDocumentIterator();

    bool                    GetFirst();
    bool                    GetNext();

    ScBaseCell*             GetCell();
    const ScPatternAttr*    GetPattern();
    void                    GetPos( SCCOL& rCol, SCROW& rRow, SCTAB& rTab );
};

class ScValueIterator            // walk through all values in an area
{
private:
    double          fNextValue;
    ScDocument*     pDoc;
    const ScAttrArray*  pAttrArray;
    sal_uLong           nNumFormat;     // for CalcAsShown
    sal_uLong           nNumFmtIndex;
    SCCOL           nStartCol;
    SCROW           nStartRow;
    SCTAB           nStartTab;
    SCCOL           nEndCol;
    SCROW           nEndRow;
    SCTAB           nEndTab;
    SCCOL           nCol;
    SCROW           nRow;
    SCTAB           nTab;
    SCSIZE          nColRow;
    SCROW           nNextRow;
    SCROW           nAttrEndRow;
    short           nNumFmtType;
    bool            bNumValid;
    bool            bSubTotal;
    bool            bNextValid;
    bool            bCalcAsShown;
    bool            bTextAsZero;

    bool            GetThis(double& rValue, sal_uInt16& rErr);
public:

                    ScValueIterator(ScDocument* pDocument,
                                    const ScRange& rRange, bool bSTotal = false,
                                    bool bTextAsZero = false );
    void            GetCurNumFmtInfo( short& nType, sal_uLong& nIndex );
    /// Does NOT reset rValue if no value found!
    bool            GetFirst(double& rValue, sal_uInt16& rErr);
    /// Does NOT reset rValue if no value found!
    bool            GetNext(double& rValue, sal_uInt16& rErr)
                    {
                        return bNextValid ? ( bNextValid = false, rValue = fNextValue,
                                                rErr = 0, nRow = nNextRow,
                                                ++nColRow, bNumValid = false, true )
                                          : ( ++nRow, GetThis(rValue, rErr) );
                    }
};

class ScDBQueryDataIterator
{
public:
    struct Value
    {
        ::rtl::OUString maString;
        double          mfValue;
        sal_uInt16      mnError;
        bool            mbIsNumber;

        Value();
    };

private:
    static SCROW        GetRowByColEntryIndex(ScDocument& rDoc, SCTAB nTab, SCCOL nCol, SCSIZE nColRow);
    static ScBaseCell*  GetCellByColEntryIndex(ScDocument& rDoc, SCTAB nTab, SCCOL nCol, SCSIZE nColRow);
    static const ScAttrArray* GetAttrArrayByCol(ScDocument& rDoc, SCTAB nTab, SCCOL nCol);
    static bool         IsQueryValid(ScDocument& rDoc, const ScQueryParam& rParam, SCTAB nTab, SCROW nRow, ScBaseCell* pCell);
    static SCSIZE       SearchColEntryIndex(ScDocument& rDoc, SCTAB nTab, SCROW nRow, SCCOL nCol);

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
    public:
        DataAccessInternal(const ScDBQueryDataIterator* pParent, ScDBQueryParamInternal* pParam, ScDocument* pDoc);
        virtual ~DataAccessInternal();
        virtual bool getCurrent(Value& rValue);
        virtual bool getFirst(Value& rValue);
        virtual bool getNext(Value& rValue);

    private:
        ScDBQueryParamInternal* mpParam;
        ScDocument*         mpDoc;
        const ScAttrArray*  pAttrArray;
        sal_uLong               nNumFormat;     // for CalcAsShown
        sal_uLong               nNumFmtIndex;
        SCCOL               nCol;
        SCROW               nRow;
        SCSIZE              nColRow;
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

class ScCellIterator            // walk through all cells in an area
{                               // for SubTotal no hidden and no sub-total lines
private:
    ScDocument*     pDoc;
    SCCOL           nStartCol;
    SCROW           nStartRow;
    SCTAB           nStartTab;
    SCCOL           nEndCol;
    SCROW           nEndRow;
    SCTAB           nEndTab;
    SCCOL           nCol;
    SCROW           nRow;
    SCTAB           nTab;
    SCSIZE          nColRow;
    bool            bSubTotal;

    ScBaseCell*     GetThis();
public:
                    ScCellIterator(ScDocument* pDocument,
                                   SCCOL nSCol, SCROW nSRow, SCTAB nSTab,
                                   SCCOL nECol, SCROW nERow, SCTAB nETab,
                                   bool bSTotal = false);
                    ScCellIterator(ScDocument* pDocument,
                                   const ScRange& rRange, bool bSTotal = false);
    ScBaseCell*     GetFirst();
    ScBaseCell*     GetNext();
    SCCOL           GetCol() const { return nCol; }
    SCROW           GetRow() const { return nRow; }
    SCTAB           GetTab() const { return nTab; }
    ScAddress       GetPos() const { return ScAddress( nCol, nRow, nTab ); }
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

private:
    boost::scoped_ptr<ScQueryParam> mpParam;
    ScDocument*     pDoc;
    const ScAttrArray*  pAttrArray;
    sal_uLong           nNumFormat;
    SCTAB           nTab;
    SCCOL           nCol;
    SCROW           nRow;
    SCSIZE          nColRow;
    SCROW           nAttrEndRow;
    sal_uInt8            nStopOnMismatch;
    sal_uInt8            nTestEqualCondition;
    bool            bAdvanceQuery;
    bool            bIgnoreMismatchOnLeadingStrings;

    ScBaseCell*     GetThis();

                    /* Only works if no regular expression is involved, only
                       searches for rows in one column, and only the first
                       query entry is considered with simple conditions
                       SC_LESS_EQUAL (sorted ascending) or SC_GREATER_EQUAL
                       (sorted descending). Check these things before
                       invocation! Delivers a starting point, continue with
                       GetThis() and GetNext() afterwards. Introduced for
                       FindEqualOrSortedLastInRange()
                     */
    ScBaseCell*     BinarySearch();

public:
                    ScQueryCellIterator(ScDocument* pDocument, SCTAB nTable,
                                        const ScQueryParam& aParam, bool bMod = true);
                                        // for bMod = FALSE the QueryParam has to be filled
                                        // (bIsString)
    ScBaseCell*     GetFirst();
    ScBaseCell*     GetNext();
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
private:
    ScDocument*     pDoc;
    SCTAB           nTab;
    SCCOL           nStartCol;
    SCCOL           nEndCol;
   SCROW           nStartRow;
    SCROW           nEndRow;
    SCROW*          pNextRows;
    SCSIZE*         pNextIndices;
    SCCOL           nCol;
    SCROW           nRow;
    bool            bMore;

public:
                    ScHorizontalCellIterator(ScDocument* pDocument, SCTAB nTable,
                                    SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2);
                    ~ScHorizontalCellIterator();

    ScBaseCell*     GetNext( SCCOL& rCol, SCROW& rRow );
    bool            ReturnNext( SCCOL& rCol, SCROW& rRow );
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
    const ScBaseCell*       pCell;
    SCCOL                   nAttrCol1;
    SCCOL                   nAttrCol2;
    SCROW                   nAttrRow;
    const ScPatternAttr*    pPattern;

    SCCOL                   nFoundStartCol;         // results after GetNext
    SCCOL                   nFoundEndCol;
    SCROW                   nFoundRow;
    const ScPatternAttr*    pFoundPattern;
    const ScBaseCell*       pFoundCell;

public:
            ScUsedAreaIterator( ScDocument* pDocument, SCTAB nTable,
                                SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 );
            ~ScUsedAreaIterator();

    bool    GetNext();

    SCCOL                   GetStartCol() const     { return nFoundStartCol; }
    SCCOL                   GetEndCol() const       { return nFoundEndCol; }
    SCROW                   GetRow() const          { return nFoundRow; }
    const ScPatternAttr*    GetPattern() const      { return pFoundPattern; }
    const ScBaseCell*       GetCell() const         { return pFoundCell; }
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
