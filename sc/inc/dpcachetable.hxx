/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
struct ScQueryParam;
// Wang Xu Ming -- 2009-8-17
// DataPilot Migration - Cache&&Performance
class ScDPItemData;
// End Comments
class Date;

// Wang Xu Ming -- 2009-8-17
// DataPilot Migration - Cache&&Performance
class ScDPTableDataCache;
struct ScDPValueData;
// End Comments
// ----------------------------------------------------------------------------

class SC_DLLPUBLIC ScDPCacheTable
{
public:
    /** individual filter item used in SingleFilter and GroupFilter. */
    struct FilterItem
    {
        // Wang Xu Ming -- 2009-8-17
        // DataPilot Migration - Cache&&Performance
        String       maString;
        // End Comments
        double      mfValue;
        bool        mbHasValue;

        FilterItem();
// Wang Xu Ming -- 2009-8-17
// DataPilot Migration - Cache&&Performance
    bool  match( const  ScDPItemData& rCellData ) const;
// End Comments
    };

    /** interface class used for filtering of rows. */
    class FilterBase
    {
    public:
        /** returns true if the matching condition is met for a single cell
            value, or false otherwise. */
// Wang Xu Ming -- 2009-8-17
// DataPilot Migration - Cache&&Performance
        virtual bool match( const  ScDPItemData& rCellData ) const = 0;
// End Comments
    };

    /** ordinary single-item filter. */
    class SingleFilter : public FilterBase
    {
    public:
        // Wang Xu Ming -- 2009-8-17
        // DataPilot Migration - Cache&&Performance
        explicit SingleFilter(String aString, double fValue, bool bHasValue);
        // End Comments
        virtual ~SingleFilter(){}

       // Wang Xu Ming -- 2009-8-17
        // DataPilot Migration - Cache&&Performance
         virtual bool match(const ScDPItemData& rCellData) const;
         // End Comments
        const String    getMatchString();
        double          getMatchValue() const;
        bool            hasValue() const;

    private:
        explicit SingleFilter();

        FilterItem  maItem;
    };

    /** multi-item (group) filter. */
    class GroupFilter : public FilterBase
    {
    public:
        // Wang Xu Ming -- 2009-8-17
        // DataPilot Migration - Cache&&Performance
        GroupFilter();
        // End Comments
        virtual ~GroupFilter(){}
        // Wang Xu Ming -- 2009-8-17
        // DataPilot Migration - Cache&&Performance
        virtual bool match(  const  ScDPItemData& rCellData ) const;
        // End Comments
        void addMatchItem(const String& rStr, double fVal, bool bHasValue);
        size_t getMatchItemCount() const;

    private:

        ::std::vector<FilterItem> maItems;
    };

    /** single filtering criterion. */
    struct Criterion
    {
        sal_Int32 mnFieldIndex;
        ::boost::shared_ptr<FilterBase> mpFilter;

        Criterion();
    };
    // Wang Xu Ming -- 2009-8-17
    // DataPilot Migration - Cache&&Performance
    ScDPCacheTable( ScDocument* pDoc,long nId );
    // End Comments
    ~ScDPCacheTable();

    sal_Int32 getRowSize() const;
    sal_Int32 getColSize() const;

    // Wang Xu Ming -- 2009-8-17
    // DataPilot Migration - Cache&&Performance
    ScDPTableDataCache* GetCache() const;
    /** Fill the internal table from the cell range provided.  This function
    assumes that the first row is the column header. */
    void fillTable( const ScQueryParam& rQuery, sal_Bool* pSpecial,
        bool bIgnoreEmptyRows, bool bRepeatIfEmpty );
    /** Fill the internal table from database connection object.  This function
        assumes that the first row is the column header. */
    void fillTable();
    // End Comments

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
    const ScDPItemData* getCell(SCCOL nCol, SCROW nRow, bool bRepeatIfEmpty) const;
    void  getValue( ScDPValueData& rVal, SCCOL nCol, SCROW nRow, bool bRepeatIfEmpty) const;
    String getFieldName( SCCOL  nIndex) const;
    //End Comments

    /** Get the field index (i.e. column ID in the original data source) based
        on the string value that corresponds with the column title.  It returns
        -1 if no field matching the string value exists. */
    sal_Int32 getFieldIndex(const String& rStr) const;

  // Wang Xu Ming -- 2009-8-17
  // DataPilot Migration - Cache&&Performance
   /** Get the unique entries for a field specified by index.  The caller must
        make sure that the table is filled before calling function, or it will
        get an empty collection. */
    const ::std::vector<SCROW>& getFieldEntries( sal_Int32 nColumn ) const;
    // End Comments
    /** Filter the table based on the specified criteria, and copy the
        result to rTabData.  This method is used, for example, to generate
        a drill-down data table. */
    void filterTable(const ::std::vector<Criterion>& rCriteria,
                     ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > >& rTabData,
                     const ::std::hash_set<sal_Int32>& rRepeatIfEmptyDims);

    void clear();
    void swap(ScDPCacheTable& rOther);
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
   // Wang Xu Ming -- 2009-8-17
    // DataPilot Migration - Cache&&Performance
   void InitNoneCache( ScDocument* pDoc );
    // End Comments
private:
    // Wang Xu Ming -- 2009-8-17
    // DataPilot Migration - Cache&&Performance
    /** unique field entires for each field (column). */
    ::std::vector< ::std::vector<SCROW> > maFieldEntries;
    // End Comments
    /** used to track visibility of rows.  The first row below the header row
        has the index of 0. */
    ::std::vector<bool> maRowsVisible;
    // Wang Xu Ming -- 2009-8-17
    // DataPilot Migration - Cache&&Performance
    ScDPTableDataCache* mpCache;
    ScDPTableDataCache* mpNoneCache;
    // End Comments
};
#endif
