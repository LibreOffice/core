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
#include "types.hxx"

// Query-related iterators. There is one template class ScQueryCellIteratorBase
// that implements most of the shared functionality, specific parts are done
// by specializing the templates and then subclassing as the actual class to use.

// Specific data should be in ScQueryCellIteratorSpecific (otherwise adding data
// members here would mean specializing the entire ScQueryCellIteratorBase).
template< ScQueryCellIteratorType iteratorType >
class ScQueryCellIteratorSpecific
{
};

// Shared code for query-based iterators.
template< ScQueryCellIteratorType iteratorType >
class ScQueryCellIteratorBase : public ScQueryCellIteratorSpecific< iteratorType >
{
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

    typedef sc::CellStoreType::const_position_type PositionType;
    PositionType maCurPos;

    ScQueryParam    maParam;
    ScDocument&     rDoc;
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

    // The actual query function. It will call HandleItemFound() for any matching type
    // and return if HandleItemFound() returns true.
    void PerformQuery();
    bool HandleItemFound(); // not implemented, needs specialization

    SCCOL           GetCol() const { return nCol; }
    SCROW           GetRow() const { return nRow; }

public:
                    ScQueryCellIteratorBase(ScDocument& rDocument, const ScInterpreterContext& rContext, SCTAB nTable,
                                            const ScQueryParam& aParam, bool bMod);
                                        // when !bMod, the QueryParam has to be filled
                                        // (bIsString)

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
};

template<>
class ScQueryCellIteratorSpecific< ScQueryCellIteratorType::Generic >
{
protected:
    bool getThisResult;
};

// The generic query iterator, used e.g. by VLOOKUP.
class ScQueryCellIterator : public ScQueryCellIteratorBase< ScQueryCellIteratorType::Generic >
{
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

    bool GetThis();

public:
    using ScQueryCellIteratorBase::ScQueryCellIteratorBase;
    bool GetFirst();
    bool GetNext();
    using ScQueryCellIteratorBase::GetCol;
    using ScQueryCellIteratorBase::GetRow;

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


template<>
class ScQueryCellIteratorSpecific< ScQueryCellIteratorType::CountIf >
{
protected:
    sal_uInt64 countIfCount;
};

// Used by ScInterpreter::ScCountIf.
class ScCountIfCellIterator : public ScQueryCellIteratorBase< ScQueryCellIteratorType::CountIf >
{
public:
    using ScQueryCellIteratorBase::ScQueryCellIteratorBase;
    sal_uInt64 GetCount();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
