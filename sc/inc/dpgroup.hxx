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

#ifndef INCLUDED_SC_INC_DPGROUP_HXX
#define INCLUDED_SC_INC_DPGROUP_HXX

#include <memory>
#include <unordered_set>
#include <vector>

#include "dptabdat.hxx"
#include "scdllapi.h"
#include "dpitemdata.hxx"
#include "dpnumgroupinfo.hxx"

class ScDocument;

typedef ::std::vector<ScDPItemData> ScDPItemDataVec;

class ScDPGroupItem
{
    ScDPItemData    aGroupName;     // name of group item
    ScDPItemDataVec aElements;      // names of items in original dimension

public:
                ScDPGroupItem( const ScDPItemData& rName );
                ~ScDPGroupItem();

    void        AddElement( const ScDPItemData& rName );

    const ScDPItemData& GetName() const     { return aGroupName; }
    bool        HasElement( const ScDPItemData& rData ) const;
    bool        HasCommonElement( const ScDPGroupItem& rOther ) const;

    void        FillGroupFilter( ScDPFilteredCache::GroupFilter& rFilter ) const;
};

typedef ::std::vector<ScDPGroupItem> ScDPGroupItemVec;

class ScDPGroupDimension
{
    long                        nSourceDim;
    long                        nGroupDim;
    OUString               aGroupName;
    ScDPGroupItemVec            aItems;
    mutable std::vector<SCROW> maMemberEntries;
    bool mbDateDimension;
public:
                ScDPGroupDimension( long nSource, const OUString& rNewName );
                ScDPGroupDimension( const ScDPGroupDimension& rOther );
                ~ScDPGroupDimension();

    ScDPGroupDimension& operator=( const ScDPGroupDimension& rOther );

    void        AddItem( const ScDPGroupItem& rItem );
    void        SetGroupDim( long nDim );           // called from AddGroupDimension

    long        GetSourceDim() const    { return nSourceDim; }
    long        GetGroupDim() const     { return nGroupDim; }
    const OUString& GetName() const { return aGroupName; }

    const std::vector< SCROW >&  GetColumnEntries( const ScDPFilteredCache&  rCacheTable ) const;
    const ScDPGroupItem* GetGroupForData( const ScDPItemData& rData ) const;  // rData = entry in original dim.
    const ScDPGroupItem* GetGroupForName( const ScDPItemData& rName ) const;  // rName = entry in group dim.
    const ScDPGroupItem* GetGroupByIndex( size_t nIndex ) const;

    void        DisposeData();

    size_t      GetItemCount() const { return aItems.size(); }

    void SetDateDimension();
    bool IsDateDimension() const { return mbDateDimension;}
};

typedef ::std::vector<ScDPGroupDimension> ScDPGroupDimensionVec;

class SC_DLLPUBLIC ScDPNumGroupDimension
{
    mutable ScDPNumGroupInfo    aGroupInfo;         // settings
    mutable std::vector<SCROW>  maMemberEntries;
    bool mbDateDimension;

public:
                ScDPNumGroupDimension();
                ScDPNumGroupDimension( const ScDPNumGroupInfo& rInfo );
                ScDPNumGroupDimension( const ScDPNumGroupDimension& rOther );
                ~ScDPNumGroupDimension();

    ScDPNumGroupDimension&  operator=( const ScDPNumGroupDimension& rOther );

    const ScDPNumGroupInfo& GetInfo() const     { return aGroupInfo; }

    const std::vector<SCROW>& GetNumEntries(SCCOL nSourceDim, const ScDPCache* pCache) const;

    void SetDateDimension();

    void        DisposeData();

    bool IsDateDimension() const { return mbDateDimension;}
};

//  proxy implementation of ScDPTableData to add grouped items

class ScDPGroupTableData : public ScDPTableData
{
    typedef std::unordered_set< OUString, OUStringHash, ::std::equal_to< OUString > > StringHashSet;

    std::shared_ptr<ScDPTableData> pSourceData;
    long                    nSourceCount;
    ScDPGroupDimensionVec   aGroups;
    ScDPNumGroupDimension*  pNumGroups;     // array[nSourceCount]
    ScDocument*             pDoc;
    StringHashSet           aGroupNames;

    void FillGroupValues(std::vector<SCROW>& rItems, const std::vector<long>& rDims);
    virtual long                GetSourceDim( long nDim ) SAL_OVERRIDE;

    bool        IsNumGroupDimension( long nDimension ) const;
    void GetNumGroupInfo(long nDimension, ScDPNumGroupInfo& rInfo);

    void        ModifyFilterCriteria(::std::vector<ScDPFilteredCache::Criterion>& rCriteria);

public:
                // takes ownership of pSource
                ScDPGroupTableData( const std::shared_ptr<ScDPTableData>& pSource, ScDocument* pDocument );
    virtual     ~ScDPGroupTableData();

    std::shared_ptr<ScDPTableData> GetSourceTableData() { return pSourceData;}

    void        AddGroupDimension( const ScDPGroupDimension& rGroup );
    void        SetNumGroupDimension( long nIndex, const ScDPNumGroupDimension& rGroup );
    long        GetDimensionIndex( const OUString& rName );

    virtual long                    GetColumnCount() SAL_OVERRIDE;
    virtual long                    GetMembersCount( long nDim ) SAL_OVERRIDE;
    virtual const std::vector< SCROW >& GetColumnEntries( long nColumn ) SAL_OVERRIDE ;
    virtual const ScDPItemData* GetMemberById( long nDim, long nId) SAL_OVERRIDE;
    virtual long Compare( long nDim, long nDataId1, long nDataId2) SAL_OVERRIDE;

    virtual OUString                getDimensionName(long nColumn) SAL_OVERRIDE;
    virtual bool                    getIsDataLayoutDimension(long nColumn) SAL_OVERRIDE;
    virtual bool                    IsDateDimension(long nDim) SAL_OVERRIDE;
    virtual sal_uLong               GetNumberFormat(long nDim) SAL_OVERRIDE;
    virtual void                    DisposeData() SAL_OVERRIDE;
    virtual void                    SetEmptyFlags( bool bIgnoreEmptyRows, bool bRepeatIfEmpty ) SAL_OVERRIDE;

    virtual bool                    IsRepeatIfEmpty() SAL_OVERRIDE;

    virtual void                    CreateCacheTable() SAL_OVERRIDE;
    virtual void                    FilterCacheTable(const std::vector<ScDPFilteredCache::Criterion>& rCriteria, const std::unordered_set<sal_Int32>& rDataDims) SAL_OVERRIDE;
    virtual void                    GetDrillDownData(const std::vector<ScDPFilteredCache::Criterion>& rCriteria,
                                                     const std::unordered_set<sal_Int32>& rCatDims,
                                                     ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > >& rData) SAL_OVERRIDE;
    virtual void                    CalcResults(CalcInfo& rInfo, bool bAutoShow) SAL_OVERRIDE;
    virtual const ScDPFilteredCache&   GetCacheTable() const SAL_OVERRIDE;
    virtual void ReloadCacheTable() SAL_OVERRIDE;

    virtual bool                    IsBaseForGroup(long nDim) const SAL_OVERRIDE;
    virtual long                    GetGroupBase(long nGroupDim) const SAL_OVERRIDE;
    virtual bool                    IsNumOrDateGroup(long nDim) const SAL_OVERRIDE;
    virtual bool                    IsInGroup( const ScDPItemData& rGroupData, long nGroupIndex,
                                               const ScDPItemData& rBaseData, long nBaseIndex ) const SAL_OVERRIDE;
    virtual bool                    HasCommonElement( const ScDPItemData& rFirstData, long nFirstIndex,
                                                      const ScDPItemData& rSecondData, long nSecondIndex ) const SAL_OVERRIDE;

#if DEBUG_PIVOT_TABLE
    virtual void Dump() const;
#endif
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
