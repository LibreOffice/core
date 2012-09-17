/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SC_DPCACHETABLE_HXX
#define SC_DPCACHETABLE_HXX

#include "sal/types.h"
#include "osl/mutex.hxx"
#include "global.hxx"
#include "dpitemdata.hxx"

#include <vector>
#include <boost/unordered_set.hpp>
#include <boost/shared_ptr.hpp>

class Date;
class ScDPItemData;
class ScDPCache;
class ScDocument;
class ScRange;
struct ScDPValueData;
struct ScQueryParam;

/**
 * Despite the name, this class is only a wrapper to the actual cache, to
 * provide filtering on the raw data based on the query filter and/or page
 * field filters. I will rename this class to a more appropriate name in the
 * future.
 */
class SC_DLLPUBLIC ScDPCacheTable
{
    struct RowFlag
    {
        bool mbShowByFilter:1;
        bool mbShowByPage:1;
        bool isActive() const;
        RowFlag();
    };
public:

    /** interface class used for filtering of rows. */
    class FilterBase
    {
    public:
        virtual ~FilterBase() {}
        /** returns true if the matching condition is met for a single cell
            value, or false otherwise. */
        virtual bool match( const  ScDPItemData& rCellData ) const = 0;
    };

    /** ordinary single-item filter. */
    class SingleFilter : public FilterBase
    {
    public:
        explicit SingleFilter(const ScDPItemData &rItem);
        virtual ~SingleFilter() {}

        virtual bool match(const ScDPItemData& rCellData) const;

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

    ScDPCacheTable(const ScDPCache* pCache);
    ~ScDPCacheTable();

    sal_Int32 getRowSize() const;
    sal_Int32 getColSize() const;

    const ScDPCache* getCache() const;

    /** Fill the internal table from the cell range provided.  This function
        assumes that the first row is the column header. */
    void fillTable(const ScQueryParam& rQuery, bool bIgnoreEmptyRows, bool bRepeatIfEmpty);

    /** Fill the internal table from database connection object.  This function
        assumes that the first row is the column header. */
    void fillTable();

    /** Check whether a specified row is active or not.  When a row is active,
        it is used in calculation of the results data.  A row becomes inactive
        when it is filtered out by page field. */
    bool isRowActive(sal_Int32 nRow) const;

    /** Set filter on/off flag to each row to control visibility.  The caller
        must ensure that the table is filled before calling this function. */
    void filterByPageDimension(const ::std::vector<Criterion>& rCriteria, const ::boost::unordered_set<sal_Int32>& rRepeatIfEmptyDims);

    /** Get the cell instance at specified location within the data grid. Note
        that the data grid doesn't include the header row.  Don't delete the
        returned object! */
    const ScDPItemData* getCell(SCCOL nCol, SCROW nRow, bool bRepeatIfEmpty) const;
    void  getValue( ScDPValueData& rVal, SCCOL nCol, SCROW nRow, bool bRepeatIfEmpty) const;
    rtl::OUString getFieldName(SCCOL nIndex) const;

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
    bool hasCache() const;

private:
    ScDPCacheTable();
    ScDPCacheTable(const ScDPCacheTable&);

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

    /** Row flags. The first row below the header row has the index of 0. */
    ::std::vector<RowFlag> maRowFlags;

    const ScDPCache* mpCache;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
