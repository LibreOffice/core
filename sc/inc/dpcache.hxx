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
#ifndef SC_DPTABLECACHE_HXX
#define SC_DPTABLECACHE_HXX

#include "global.hxx"
#include "dpnumgroupinfo.hxx"
#include "dpmacros.hxx"
#include "tools/date.hxx"

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/unordered_set.hpp>
#include <mdds/flat_segment_tree.hpp>

#include <vector>
#include <set>

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
    typedef boost::unordered_set<OUString, OUStringHash> StringSetType;

public:
    typedef std::vector<ScDPItemData> ItemsType;
    typedef std::set<ScDPObject*> ObjectSetType;
    typedef std::vector<OUString> LabelsType;
    typedef std::vector<SCROW> IndexArrayType;

    struct GroupItems : boost::noncopyable
    {
        ItemsType maItems;
        ScDPNumGroupInfo maInfo;
        sal_Int32 mnGroupType;

        GroupItems();
        GroupItems(const ScDPNumGroupInfo& rInfo, sal_Int32 nGroupType);
    };

    struct Field : boost::noncopyable
    {
        /**
         * Optional items for grouped field.
         */
        boost::scoped_ptr<GroupItems> mpGroup;

        /**
         * Unique values in the field, stored in ascending order.
         */
        ItemsType maItems;

        /**
         * Original source data represented as indices to the unique value
         * list.  The order of the data is as they appear in the original
         * data source.
         */
        IndexArrayType maData;

        sal_uLong mnNumFormat;

        Field();
    };

    /**
     * Interface for connecting to database source.  Column index is 0-based.
     */
    class DBConnector
    {
    public:
        virtual long getColumnCount() const = 0;
        virtual OUString getColumnLabel(long nCol) const = 0;
        virtual bool first() = 0;
        virtual bool next() = 0;
        virtual void finish() = 0;
        virtual void getValue(long nCol, ScDPItemData& rData, short& rNumType) const = 0;
        virtual ~DBConnector() {}
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
    mutable StringSetType maStringPool;

    LabelsType maLabelNames;    // Stores dimension names.
    mdds::flat_segment_tree<SCROW, bool> maEmptyRows;
    SCROW mnDataSize;
    SCROW mnRowCount;

    bool mbDisposing;

public:
    const OUString* InternString(const OUString& rStr) const;
    void AddReference(ScDPObject* pObj) const;
    void RemoveReference(ScDPObject* pObj) const;
    const ObjectSetType& GetAllReferences() const;

    SCROW GetIdByItemData(long nDim, const ScDPItemData& rItem) const;
    OUString GetFormattedString(long nDim, const ScDPItemData& rItem) const;
    long AppendGroupField();
    void ResetGroupItems(long nDim, const ScDPNumGroupInfo& rNumInfo, sal_Int32 nGroupType);
    SCROW SetGroupItem(long nDim, const ScDPItemData& rData);
    void GetGroupDimMemberIds(long nDim, std::vector<SCROW>& rIds) const;
    void ClearGroupFields();
    const ScDPNumGroupInfo* GetNumGroupInfo(long nDim) const;
    sal_Int32 GetGroupType(long nDim) const;

    SCCOL GetDimensionIndex(const OUString& sName) const;
    sal_uLong GetNumberFormat( long nDim ) const;
    bool  IsDateDimension( long nDim ) const ;
    long GetDimMemberCount(long nDim) const;
    SCROW GetOrder( long nDim, SCROW nIndex ) const;

    const ItemsType& GetDimMemberValues( SCCOL nDim ) const;
    bool InitFromDoc(ScDocument* pDoc, const ScRange& rRange);
    bool InitFromDataBase(DBConnector& rDB);

    SCROW GetRowCount() const;
    SCROW GetDataSize() const;
    SCROW GetItemDataId( sal_uInt16 nDim, SCROW nRow, bool bRepeatIfEmpty ) const;
    OUString GetDimensionName(LabelsType::size_type nDim) const;
    bool IsRowEmpty(SCROW nRow) const;
    bool ValidQuery(SCROW nRow, const ScQueryParam& rQueryParam) const;

    ScDocument* GetDoc() const;
    long GetColumnCount() const;

    const ScDPItemData* GetItemDataById( long nDim, SCROW nId ) const;

    ScDPCache(ScDocument* pDoc);
    ~ScDPCache();

#if DEBUG_PIVOT_TABLE
    void Dump() const;
#endif

private:
    void PostInit();
    void Clear();
    void AddLabel(const OUString& rLabel);
    const GroupItems* GetGroupItems(long nDim) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
