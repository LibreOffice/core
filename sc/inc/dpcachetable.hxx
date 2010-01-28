/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dpcachetable.hxx,v $
 *
 * $Revision: 1.4 $
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
#include "collect.hxx"

#include <vector>
#include <hash_map>
#include <hash_set>
#include <boost/shared_ptr.hpp>
#include <com/sun/star/uno/Reference.hxx>

namespace com { namespace sun { namespace star {
    namespace sdbc {
        class XRowSet;
    }
    namespace sheet {
        struct DataPilotFieldFilter;
    }
}}}

class Date;

class ScDocument;
class ScRange;
class ScDPDimension;
class ScDPCollection;
struct ScDPCacheCell;
struct ScDPItemData;
class Date;

// ----------------------------------------------------------------------------

class ScDPCacheTable
{
public:

    struct Cell
    {
        SCROW           mnCategoryRef;
        ScDPCacheCell*  mpContent;

        Cell();
        ~Cell();
    };

    /** individual filter item used in SingleFilter and GroupFilter. */
    struct FilterItem
    {
        sal_Int32   mnMatchStrId;
        double      mfValue;
        bool        mbHasValue;

        FilterItem();
    };

    /** interface class used for filtering of rows. */
    class FilterBase
    {
    public:
        /** returns true if the matching condition is met for a single cell
            value, or false otherwise. */
        virtual bool match(const ScDPCacheCell& rCell) const = 0;
    };

    /** ordinary single-item filter. */
    class SingleFilter : public FilterBase
    {
    public:
        explicit SingleFilter(ScSimpleSharedString& rSharedString,
                              sal_Int32 nMatchStrId, double fValue, bool bHasValue);
        virtual ~SingleFilter(){}

        virtual bool match(const ScDPCacheCell& rCell) const;

        const String    getMatchString();
        double          getMatchValue() const;
        bool            hasValue() const;

    private:
        explicit SingleFilter();

        FilterItem  maItem;
        ScSimpleSharedString mrSharedString;
    };

    /** multi-item (group) filter. */
    class GroupFilter : public FilterBase
    {
    public:
        GroupFilter(ScSimpleSharedString& rSharedString);
        virtual ~GroupFilter(){}
        virtual bool match(const ScDPCacheCell& rCell) const;

        void addMatchItem(const String& rStr, double fVal, bool bHasValue);
        size_t getMatchItemCount() const;

    private:
        GroupFilter();

        ::std::vector<FilterItem> maItems;
        ScSimpleSharedString mrSharedString;
    };

    /** single filtering criterion. */
    struct Criterion
    {
        sal_Int32 mnFieldIndex;
        ::boost::shared_ptr<FilterBase> mpFilter;

        Criterion();
    };

    ScDPCacheTable(ScDPCollection* pCollection);
    ~ScDPCacheTable();

    sal_Int32 getRowSize() const;
    sal_Int32 getColSize() const;

    /** Fill the internal table from the cell range provided.  This function
        assumes that the first row is the column header. */
    void fillTable(ScDocument* pDoc, const ScRange& rRange, const ScQueryParam& rQuery, BOOL* pSpecial,
                   bool bIgnoreEmptyRows);

    /** Fill the internal table from database connection object.  This function
        assumes that the first row is the column header. */
    void fillTable(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xRowSet,
                   const Date& rNullDate);

    /** Check whether a specified row is active or not.  When a row is active,
        it is used in calculation of the results data.  A row becomes inactive
        when it is filtered out by page field. */
    bool isRowActive(sal_Int32 nRow) const;

    /** Set filter on/off flag to each row to control visibility.  The caller
        must ensure that the table is filled before calling this function. */
    void filterByPageDimension(const ::std::vector<Criterion>& rCriteria, const ::std::hash_set<sal_Int32>& rRepeatIfEmptyDims);

    /** Get the cell instance at specified location within the data grid. Note
        that the data grid doesn't include the header row.  Don't delete the
        returned object! */
    const ScDPCacheCell* getCell(SCCOL nCol, SCROW nRow, bool bRepeatIfEmpty) const;

    const String* getFieldName(sal_Int32 nIndex) const;

    /** Get the unique entries for a field specified by index.  The caller must
        make sure that the table is filled before calling function, or it will
        get an empty collection. */
    const TypedScStrCollection& getFieldEntries(sal_Int32 nIndex) const;

    /** Filter the table based on the specified criteria, and copy the
        result to rTabData.  This method is used, for example, to generate
        a drill-down data table. */
    void filterTable(const ::std::vector<Criterion>& rCriteria,
                     ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > >& rTabData,
                     const ::std::hash_set<sal_Int32>& rRepeatIfEmptyDims);

    void clear();
    bool empty() const;

private:
    ScDPCacheTable();
    ScDPCacheTable(const ScDPCacheTable&);

    /**
     * Check if a given row meets all specified criteria.
     *
     * @param nRow index of row to be tested.
     * @param rCriteria a list of criteria
     */
    bool isRowQualified(sal_Int32 nRow, const ::std::vector<Criterion>& rCriteria, const ::std::hash_set<sal_Int32>& rRepeatIfEmptyDims) const;
    void getValueData(ScDocument* pDoc, const ScAddress& rPos, ScDPCacheCell& rCell);

private:
    typedef ::boost::shared_ptr<TypedScStrCollection> TypedScStrCollectionPtr;

    /** main data table. */
    ::std::vector< ::std::vector< ::ScDPCacheTable::Cell > > maTable;

    /** header string IDs */
    ::std::vector<sal_Int32> maHeader;

    /** unique field entires for each field (column). */
    ::std::vector<TypedScStrCollectionPtr> maFieldEntries;

    /** used to track visibility of rows.  The first row below the header row
        has the index of 0. */
    ::std::vector<bool> maRowsVisible;

    ScSimpleSharedString& mrSharedString;
    ScDPCollection* mpCollection;
};


#endif
