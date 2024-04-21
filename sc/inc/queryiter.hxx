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
#include "global.hxx"
#include "scdllapi.h"
#include "queryparam.hxx"
#include "mtvelements.hxx"
#include "types.hxx"

struct ScComplexRefData;
class ScSortedRangeCache;

/*
Query-related iterators. There is one template class ScQueryCellIteratorBase
that implements most of the shared functionality, specific parts are done
by specializing the templates and then subclassing as the actual class to use.
A template is used for maximum performance, as that allows fast code specializing,
inlining, etc.
There are two template arguments:
* ScQueryCellIteratorAccess specifies how cells are accessed:
  + Direct - direct access to cells using mdds.
  + SortedCache - for accessing unsorted cells in a sorted way using ScSortedRangeCache.
* ScQueryCellIteratorType specifies the type of the query operation:
  + Generic - the generic lookup, used e.g. by VLOOKUP.
  + CountIf - faster implementation for COUNTIF(S).

Specific data should be in specific templated base classes, otherwise adding data
members would mean specializing the entire ScQueryCellIteratorBase. Some specific
functionality may also be implemented in the base classes or depending on the template
parameter.
*/

// Data and functionality for accessing cells in a specific way.
// Needs specialization, see ScQueryCellIteratorAccess::Direct for what is needed.
template< ScQueryCellIteratorAccess accessType >
class ScQueryCellIteratorAccessSpecific
{
};

// The implementation using linear direct mdds access.
template<>
class ScQueryCellIteratorAccessSpecific< ScQueryCellIteratorAccess::Direct >
{
protected:
    ScQueryCellIteratorAccessSpecific( ScDocument& rDocument, ScInterpreterContext& rContext,
        const ScQueryParam& rParam, bool bReverseSearch );
    // Initialize position for new column.
    void InitPos();
    // Increase position (next row).
    void IncPos();
    // Next mdds block. If access is not direct/linear, then
    // should call IncPos().
    void IncBlock();

    // Decrease position (prev row).
    void DecPos();
    // Prev mdds block. If access is not direct/linear, then
    // should call DecPos().
    void DecBlock();

    // These members needs to be available already in the base class.
    typedef sc::CellStoreType::const_position_type PositionType;
    PositionType maCurPos;
    ScQueryParam    maParam;
    ScDocument&     rDoc;
    ScInterpreterContext& mrContext;
    bool            mbReverseSearch;
    SCTAB           nTab;
    SCCOL           nCol;
    SCROW           nRow;

    class NonEmptyCellIndexer;
    typedef std::pair<ScRefCellValue, SCROW> BinarySearchCellType;
    static NonEmptyCellIndexer MakeBinarySearchIndexer(const sc::CellStoreType& rCells,
        SCROW nStartRow, SCROW nEndRow);
};

// The implementation using ScSortedRangeCache, which allows sorted iteration
// of unsorted cells.
template<>
class ScQueryCellIteratorAccessSpecific< ScQueryCellIteratorAccess::SortedCache >
{
public:
    void SetSortedRangeCache( const ScSortedRangeCache& cache );
    template<bool fast>
    bool IncPosImpl();
protected:
    ScQueryCellIteratorAccessSpecific( ScDocument& rDocument, ScInterpreterContext& rContext,
        const ScQueryParam& rParam, bool bReverseSearch );
    void InitPosStart(sal_uInt8 nSortedBinarySearch = 0x00);
    void InitPosFinish( SCROW beforeRow, SCROW lastRow, bool bFirstMatch );
    void IncPos() { IncPosImpl<false>(); }
    bool IncPosFast() { return IncPosImpl<true>(); }
    void IncBlock() { IncPos(); } // Cannot skip entire block, not linear.

    // Initialize for backward search. (no need for SortedCache)
    static void DecPos() {};
    static void DecBlock() {};

    // These members needs to be available already in the base class.
    typedef sc::CellStoreType::const_position_type PositionType;
    PositionType maCurPos;
    ScQueryParam    maParam;
    ScDocument&     rDoc;
    ScInterpreterContext& mrContext;
    bool            mbReverseSearch;
    SCTAB           nTab;
    SCCOL           nCol;
    SCROW           nRow;
    const ScColumn* pColumn; // matching nCol, set by InitPos()

    const ScSortedRangeCache* sortedCache;
    size_t sortedCachePos;
    size_t sortedCachePosLast;

    class SortedCacheIndexer;
    typedef std::pair<ScRefCellValue, SCROW> BinarySearchCellType;
    SortedCacheIndexer MakeBinarySearchIndexer(const sc::CellStoreType& rCells,
        SCROW nStartRow, SCROW nEndRow);
};

// Data and functionality for specific types of query.
template< ScQueryCellIteratorType iteratorType >
class ScQueryCellIteratorTypeSpecific
{
protected:
    bool HandleItemFound(); // not implemented, needs specialization
};

// Shared code for query-based iterators. The main class.
template< ScQueryCellIteratorAccess accessType, ScQueryCellIteratorType queryType >
class ScQueryCellIteratorBase
    : public ScQueryCellIteratorAccessSpecific< accessType >
    , public ScQueryCellIteratorTypeSpecific< queryType >
{
    typedef ScQueryCellIteratorAccessSpecific< accessType > AccessBase;
    typedef ScQueryCellIteratorTypeSpecific< queryType > TypeBase;
protected:
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

    enum SortedBinarySearchBits
    {
        nBinarySearchDisabled = 0x00,
        nSearchbAscd = 0x01,
        nSearchbDesc = 0x02,
    };

    sal_uInt8            nStopOnMismatch;
    sal_uInt8            nTestEqualCondition;
    sal_uInt8            nSortedBinarySearch;
    bool            bAdvanceQuery;
    bool            bIgnoreMismatchOnLeadingStrings;
    sal_uInt16      nSearchOpCode;
    SCCOL           nBestFitCol;
    SCROW           nBestFitRow;

    // Make base members directly visible here (templated bases need 'this->').
    using AccessBase::maCurPos;
    using AccessBase::maParam;
    using AccessBase::rDoc;
    using AccessBase::mrContext;
    using AccessBase::mbReverseSearch;
    using AccessBase::nTab;
    using AccessBase::nCol;
    using AccessBase::nRow;
    using AccessBase::IncPos;
    using AccessBase::IncBlock;
    using AccessBase::DecPos;
    using AccessBase::DecBlock;
    using typename AccessBase::BinarySearchCellType;
    using AccessBase::MakeBinarySearchIndexer;
    using TypeBase::HandleItemFound;

    void InitPos();

    // The actual query function. It will call HandleItemFound() for any matching type
    // and return if HandleItemFound() returns true.
    void PerformQuery();

    /* Only works if no regular expression is involved, only searches for rows in one column,
       and only the first query entry is considered with simple conditions SC_LESS,SC_LESS_EQUAL,
       SC_EQUAL (sorted ascending) or SC_GREATER,SC_GREATER_EQUAL (sorted descending). It
       delivers a starting point set to nRow, i.e. the last row that either matches the searched
       for value, or the last row that matches the condition. Continue with e.g. GetThis() and
       GetNext() afterwards. Returns false if the searched for value is not in the search range
       or if the range is not properly sorted, with nRow in that case set to the first row or after
       the last row. In that case use GetFirst().
    */
    bool BinarySearch( SCCOL col, bool forEqual = false );

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

    void            HandleBestFitItemFound(SCCOL nBFitCol, SCROW nBFitRow)
                        {
                            nBestFitCol = nBFitCol;
                            nBestFitRow = nBFitRow;
                        }
public:
                    ScQueryCellIteratorBase(ScDocument& rDocument, ScInterpreterContext& rContext, SCTAB nTable,
                                            const ScQueryParam& aParam, bool bMod, bool bReverse);
                                        // when !bMod, the QueryParam has to be filled
                                        // (bIsString)

                    // increments all Entry.nField, if column
                    // changes, for ScInterpreter ScHLookup()
    void            SetAdvanceQueryParamEntryField( bool bVal )
                        { bAdvanceQuery = bVal; }
    void            AdvanceQueryParamEntryField();

    void            SetSortedBinarySearchMode( sal_Int8 nSearchMode )
                        {
                            nSortedBinarySearch = sal::static_int_cast<sal_uInt8>(nSearchMode == 2 ?
                                nSearchbAscd : (nSearchMode == -2 ? nSearchbDesc : nBinarySearchDisabled));
                        }

    void            SetLookupMode( sal_uInt16 nVal )
                        { nSearchOpCode = nVal; }
};


template<>
class ScQueryCellIteratorTypeSpecific< ScQueryCellIteratorType::Generic >
{
protected:
    bool HandleItemFound();
    bool getThisResult;
};

// The generic query iterator, used e.g. by VLOOKUP.
template< ScQueryCellIteratorAccess accessType >
class ScQueryCellIterator
    : public ScQueryCellIteratorBase< accessType, ScQueryCellIteratorType::Generic >
{
    typedef ScQueryCellIteratorBase< accessType, ScQueryCellIteratorType::Generic > Base;
    // Make base members directly visible here (templated bases need 'this->').
    using Base::maParam;
    using Base::rDoc;
    using Base::mrContext;
    using Base::mbReverseSearch;
    using Base::nTab;
    using Base::nCol;
    using Base::nRow;
    using Base::InitPos;
    using Base::IncPos;
    using Base::DecPos;
    using Base::bIgnoreMismatchOnLeadingStrings;
    using Base::SetStopOnMismatch;
    using Base::SetTestEqualCondition;
    using Base::BinarySearch;
    using typename Base::PositionType;
    using Base::maCurPos;
    using Base::IsEqualConditionFulfilled;
    using Base::bAdvanceQuery;
    using Base::StoppedOnMismatch;
    using Base::nStopOnMismatch;
    using Base::nStopOnMismatchEnabled;
    using Base::nTestEqualCondition;
    using Base::nTestEqualConditionEnabled;
    using Base::nSortedBinarySearch;
    using Base::nBinarySearchDisabled;
    using Base::PerformQuery;
    using Base::getThisResult;
    using Base::nBestFitCol;
    using Base::nBestFitRow;
    using Base::nSearchOpCode;

    bool GetThis();

public:
    ScQueryCellIterator(ScDocument& rDocument, ScInterpreterContext& rContext, SCTAB nTable,
                        const ScQueryParam& aParam, bool bMod, bool bReverse)
        : Base( rDocument, rContext, nTable, aParam, bMod, bReverse ) {}
    bool GetFirst();
    bool GetNext();
    SCCOL GetCol() const { return nCol; }
    SCROW GetRow() const { return nRow; }

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

typedef ScQueryCellIterator< ScQueryCellIteratorAccess::Direct > ScQueryCellIteratorDirect;

class ScQueryCellIteratorSortedCache
    : public ScQueryCellIterator< ScQueryCellIteratorAccess::SortedCache >
{
    typedef ScQueryCellIterator< ScQueryCellIteratorAccess::SortedCache > Base;
public:
    ScQueryCellIteratorSortedCache(ScDocument& rDocument, ScInterpreterContext& rContext,
        SCTAB nTable, const ScQueryParam& aParam, bool bMod, bool bReverse )
    : Base( rDocument, rContext, nTable, aParam, bMod, bReverse ) {}
    // Returns true if this iterator can be used for the given query.
    static bool CanBeUsed(ScDocument& rDoc, const ScQueryParam& aParam,
        SCTAB nTab, const ScFormulaCell* cell, const ScComplexRefData* refData,
        ScInterpreterContext& context);
};


template<>
class ScQueryCellIteratorTypeSpecific< ScQueryCellIteratorType::CountIf >
{
protected:
    bool HandleItemFound();
    sal_uInt64 countIfCount;
};

// Used by ScInterpreter::ScCountIf.
template< ScQueryCellIteratorAccess accessType >
class ScCountIfCellIterator
    : public ScQueryCellIteratorBase< accessType, ScQueryCellIteratorType::CountIf >
{
protected:
    typedef ScQueryCellIteratorBase< accessType, ScQueryCellIteratorType::CountIf > Base;
    // Make base members directly visible here (templated bases need 'this->').
    using Base::maParam;
    using Base::rDoc;
    using Base::nTab;
    using Base::nCol;
    using Base::nRow;
    using Base::InitPos;
    using Base::PerformQuery;
    using Base::SetAdvanceQueryParamEntryField;
    using Base::countIfCount;

public:
    ScCountIfCellIterator(ScDocument& rDocument, ScInterpreterContext& rContext, SCTAB nTable,
                          const ScQueryParam& aParam, bool bMod, bool bReverse)
        : Base( rDocument, rContext, nTable, aParam, bMod, bReverse ) {}
    sal_uInt64 GetCount();
};

typedef ScCountIfCellIterator< ScQueryCellIteratorAccess::Direct > ScCountIfCellIteratorDirect;

class ScCountIfCellIteratorSortedCache
    : public ScCountIfCellIterator< ScQueryCellIteratorAccess::SortedCache >
{
    typedef ScCountIfCellIterator< ScQueryCellIteratorAccess::SortedCache > Base;
public:
    ScCountIfCellIteratorSortedCache(ScDocument& rDocument, ScInterpreterContext& rContext,
        SCTAB nTable, const ScQueryParam& aParam, bool bMod, bool bReverse)
    : Base( rDocument, rContext, nTable, aParam, bMod, bReverse ) {}
    // Returns true if this iterator can be used for the given query.
    static bool CanBeUsed(ScDocument& rDoc, const ScQueryParam& aParam,
        SCTAB nTab, const ScFormulaCell* cell, const ScComplexRefData* refData,
        ScInterpreterContext& context);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
