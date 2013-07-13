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

#ifndef SC_DPCACHETABLE_HXX
#define SC_DPCACHETABLE_HXX

#include "sal/types.h"
#include "osl/mutex.hxx"
#include "global.hxx"
#include "dpitemdata.hxx"
#include "calcmacros.hxx"

#include <vector>
#include <boost/unordered_set.hpp>
#include <boost/shared_ptr.hpp>

#include <mdds/flat_segment_tree.hpp>

class ScDPItemData;
class ScDPCache;
class ScDocument;
class ScRange;
struct ScDPValue;
struct ScQueryParam;

/**
 * This class is only a wrapper to the actual cache, to provide filtering on
 * the raw data based on the query filter and/or page field filters.
 */
class SC_DLLPUBLIC ScDPFilteredCache
{
    typedef mdds::flat_segment_tree<SCROW, bool> RowFlagType;

public:
    /** interface class used for filtering of rows. */
    class FilterBase
    {
    public:
        virtual ~FilterBase() {}
        /** returns true if the matching condition is met for a single cell
            value, or false otherwise. */
        virtual bool match( const  ScDPItemData& rCellData ) const = 0;

        virtual std::vector<ScDPItemData> getMatchValues() const = 0;
    };

    /** ordinary single-item filter. */
    class SingleFilter : public FilterBase
    {
    public:
        explicit SingleFilter(const ScDPItemData &rItem);
        virtual ~SingleFilter() {}

        virtual bool match(const ScDPItemData& rCellData) const;
        virtual std::vector<ScDPItemData> getMatchValues() const;
        const ScDPItemData& getMatchValue() const;

    private:
        explicit SingleFilter();

        ScDPItemData maItem;
    };

    /** multi-item (group) filter. */
    class GroupFilter : public FilterBase
    {
    public:
        GroupFilter();
        virtual ~GroupFilter() {}
        virtual bool match(const ScDPItemData& rCellData) const;
        virtual std::vector<ScDPItemData> getMatchValues() const;
        void addMatchItem(const ScDPItemData& rItem);
        size_t getMatchItemCount() const;

    private:
        ::std::vector<ScDPItemData> maItems;
    };

    /** single filtering criterion. */
    struct Criterion
    {
        sal_Int32 mnFieldIndex;
        ::boost::shared_ptr<FilterBase> mpFilter;

        Criterion();
    };

    ScDPFilteredCache(const ScDPCache& rCache);
    ~ScDPFilteredCache();

    sal_Int32 getRowSize() const;
    sal_Int32 getColSize() const;

    const ScDPCache* getCache() const;

    void fillTable(const ScQueryParam& rQuery, bool bIgnoreEmptyRows, bool bRepeatIfEmpty);

    void fillTable();

    /** Check whether a specified row is active or not.  When a row is active,
        it is used in calculation of the results data.  A row becomes inactive
        when it is filtered out by page field. */
    bool isRowActive(sal_Int32 nRow, sal_Int32* pLastRow = NULL) const;

    /** Set filter on/off flag to each row to control visibility.  The caller
        must ensure that the table is filled before calling this function. */
    void filterByPageDimension(const ::std::vector<Criterion>& rCriteria, const ::boost::unordered_set<sal_Int32>& rRepeatIfEmptyDims);

    /** Get the cell instance at specified location within the data grid. Note
        that the data grid doesn't include the header row.  Don't delete the
        returned object! */
    const ScDPItemData* getCell(SCCOL nCol, SCROW nRow, bool bRepeatIfEmpty) const;
    void  getValue( ScDPValue& rVal, SCCOL nCol, SCROW nRow, bool bRepeatIfEmpty) const;
    OUString getFieldName(SCCOL nIndex) const;

   /** Get the unique entries for a field specified by index.  The caller must
       make sure that the table is filled before calling function, or it will
       get an empty collection. */
    const ::std::vector<SCROW>& getFieldEntries( sal_Int32 nColumn ) const;

    /** Filter the table based on the specified criteria, and copy the
        result to rTabData.  This method is used, for example, to generate
        a drill-down data table. */
    void filterTable(const ::std::vector<Criterion>& rCriteria,
                     ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > >& rTabData,
                     const ::boost::unordered_set<sal_Int32>& rRepeatIfEmptyDims);

    SCROW getOrder(long nDim, SCROW nIndex) const;
    void clear();
    bool empty() const;

#if DEBUG_PIVOT_TABLE
    void dumpRowFlag(const RowFlagType& rFlag) const;
    void dump() const;
#endif

private:
    ScDPFilteredCache();
    ScDPFilteredCache(const ScDPFilteredCache&);

    /**
     * Check if a given row meets all specified criteria.
     *
     * @param nRow index of row to be tested.
     * @param rCriteria a list of criteria
     */
    bool isRowQualified(sal_Int32 nRow, const ::std::vector<Criterion>& rCriteria, const ::boost::unordered_set<sal_Int32>& rRepeatIfEmptyDims) const;

private:

    /** unique field entires for each field (column). */
    ::std::vector< ::std::vector<SCROW> > maFieldEntries;

    /** Rows visible by standard filter query. */
    RowFlagType maShowByFilter;
    /** Rows visible by page dimension filtering. */
    RowFlagType maShowByPage;

    const ScDPCache& mrCache;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
