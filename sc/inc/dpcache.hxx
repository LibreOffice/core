/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright IBM Corporation 2009.
 * Copyright 2009 by Sun Microsystems, Inc.
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
#ifndef SC_DPTABLECACHE_HXX
#define SC_DPTABLECACHE_HXX

#include "global.hxx"
#include "dpnumgroupinfo.hxx"
#include "tools/date.hxx"

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <mdds/flat_segment_tree.hpp>

#include <vector>
#include <set>

namespace com { namespace sun { namespace star {
    namespace sdbc {
        class XRowSet;
    }
}}}

struct ScQueryParam;
class ScDPObject;
class ScDPItemData;
struct ScDPNumGroupInfo;

/**
 * This class represents the cached data part of the datapilot cache table
 * implementation.
 */
class SC_DLLPUBLIC ScDPCache : boost::noncopyable
{
public:
    typedef std::vector<ScDPItemData> DataListType;
    typedef std::set<ScDPObject*> ObjectSetType;
    typedef std::vector<rtl::OUString> LabelsType;
    typedef std::vector<SCROW> IndexArrayType;

    struct GroupItems : boost::noncopyable
    {
        DataListType maItems;
        ScDPNumGroupInfo maInfo;

        GroupItems();
        GroupItems(const ScDPNumGroupInfo& rInfo);
    };

    struct Field : boost::noncopyable
    {
        /**
         * Optional items for grouped field.
         */
        boost::scoped_ptr<GroupItems> mpGroup;

        DataListType maItems; /// Unique values in the field.

        /**
         * Original source data represented as indices to the unique value
         * list.  The order of the data is as they appear in the original
         * data source.
         */
        IndexArrayType maData;

        /**
         * Ascending order of field items.
         */
        IndexArrayType maGlobalOrder;

        /**
         * Ranks of each unique data represented by their index.  It's a
         * reverse mapping of item index to global order index.
         */
        mutable IndexArrayType maIndexOrder;

        sal_uLong mnNumFormat;

        Field();
    };

private:

    ScDocument* mpDoc;
    long mnColumnCount;

    /**
     * All pivot table objects that references this cache.
     */
    mutable ObjectSetType maRefObjects;

    typedef boost::ptr_vector<Field> FieldsType;
    typedef boost::ptr_vector<GroupItems> GroupFieldsType;

    FieldsType maFields;
    GroupFieldsType maGroupFields;

    LabelsType maLabelNames;    // Stores dimension names.
    mdds::flat_segment_tree<SCROW, bool> maEmptyRows;

    bool mbDisposing;

public:
    void AddReference(ScDPObject* pObj) const;
    void RemoveReference(ScDPObject* pObj) const;
    const ObjectSetType& GetAllReferences() const;

    SCROW GetIdByItemData(long nDim, const ScDPItemData& rItem) const;
    rtl::OUString GetFormattedString(long nDim, const ScDPItemData& rItem) const;
    long AppendGroupField();
    void ResetGroupItems(long nDim, const ScDPNumGroupInfo& rNumInfo);
    SCROW SetGroupItem(long nDim, const ScDPItemData& rData);
    void GetGroupDimMemberIds(long nDim, std::vector<SCROW>& rIds) const;
    void ClearGroupFields();

    SCCOL GetDimensionIndex(const rtl::OUString& sName) const;
    sal_uLong GetNumberFormat( long nDim ) const;
    bool  IsDateDimension( long nDim ) const ;
    SCROW GetDimMemberCount( SCCOL nDim ) const;
    SCROW GetOrder( long nDim, SCROW nIndex ) const;

    const DataListType& GetDimMemberValues( SCCOL nDim ) const;
    bool InitFromDoc(ScDocument* pDoc, const ScRange& rRange);
    bool InitFromDataBase(const  ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet>& xRowSet, const Date& rNullDate);

    SCROW  GetRowCount() const;
    SCROW  GetItemDataId( sal_uInt16 nDim, SCROW nRow, bool bRepeatIfEmpty ) const;
    rtl::OUString GetDimensionName(LabelsType::size_type nDim) const;
    bool IsRowEmpty(SCROW nRow) const;
    bool ValidQuery(SCROW nRow, const ScQueryParam& rQueryParam) const;

    ScDocument* GetDoc() const;
    long GetColumnCount() const;

    const ScDPItemData* GetItemDataById( long nDim, SCROW nId ) const;

    bool operator== ( const ScDPCache& r ) const;

    ScDPCache(ScDocument* pDoc);
    ~ScDPCache();

private:
    void PostInit();
    void Clear();
    void AddLabel(const rtl::OUString& rLabel);
    bool AddData(long nDim, const ScDPItemData& rData, sal_uLong nNumFormat);
    const GroupItems* GetGroupItems(long nDim) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
