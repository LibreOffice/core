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

    void        AddElement( const ScDPItemData& rName );

    const ScDPItemData& GetName() const     { return aGroupName; }
    bool        HasElement( const ScDPItemData& rData ) const;
    bool        HasCommonElement( const ScDPGroupItem& rOther ) const;

    void        FillGroupFilter( ScDPFilteredCache::GroupFilter& rFilter ) const;
};

class ScDPGroupDimension
{
    tools::Long                        nSourceDim;
    tools::Long                        nGroupDim;
    OUString                    aGroupName;
    std::vector<ScDPGroupItem>  aItems;
    mutable std::vector<SCROW> maMemberEntries;
    bool mbDateDimension;
public:
                ScDPGroupDimension( tools::Long nSource, const OUString& rNewName );
                ScDPGroupDimension( const ScDPGroupDimension& rOther );
                ~ScDPGroupDimension();

    ScDPGroupDimension& operator=( const ScDPGroupDimension& rOther );

    void        AddItem( const ScDPGroupItem& rItem );
    void        SetGroupDim( tools::Long nDim );           // called from AddGroupDimension

    tools::Long        GetSourceDim() const    { return nSourceDim; }
    tools::Long        GetGroupDim() const     { return nGroupDim; }
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

class ScDPGroupTableData final : public ScDPTableData
{
    std::shared_ptr<ScDPTableData> pSourceData;
    sal_Int32                    nSourceCount;
    std::vector<ScDPGroupDimension>
                            aGroups;
    std::unique_ptr<ScDPNumGroupDimension[]>
                            pNumGroups;     // array[nSourceCount]
    ScDocument*             pDoc;

    void FillGroupValues(std::vector<SCROW>& rItems, const std::vector<sal_Int32>& rDims);
    virtual sal_Int32                GetSourceDim( sal_Int32 nDim ) override;

    bool        IsNumGroupDimension( tools::Long nDimension ) const;
    void GetNumGroupInfo(tools::Long nDimension, ScDPNumGroupInfo& rInfo);

    void        ModifyFilterCriteria(::std::vector<ScDPFilteredCache::Criterion>& rCriteria);

public:
                // takes ownership of pSource
                ScDPGroupTableData( const std::shared_ptr<ScDPTableData>& pSource, ScDocument* pDocument );
    virtual     ~ScDPGroupTableData() override;

    const std::shared_ptr<ScDPTableData>& GetSourceTableData() const { return pSourceData;}

    void        AddGroupDimension( const ScDPGroupDimension& rGroup );
    void        SetNumGroupDimension( sal_Int32 nIndex, const ScDPNumGroupDimension& rGroup );
    sal_Int32        GetDimensionIndex( std::u16string_view rName );

    virtual sal_Int32                    GetColumnCount() override;
    virtual sal_Int32                    GetMembersCount( sal_Int32 nDim ) override;
    virtual const std::vector< SCROW >& GetColumnEntries( sal_Int32 nColumn ) override ;
    virtual const ScDPItemData* GetMemberById( sal_Int32 nDim, sal_Int32 nId) override;
    virtual sal_Int32 Compare( sal_Int32 nDim, sal_Int32 nDataId1, sal_Int32 nDataId2) override;

    virtual OUString                getDimensionName(sal_Int32 nColumn) override;
    virtual bool                    getIsDataLayoutDimension(sal_Int32 nColumn) override;
    virtual bool                    IsDateDimension(sal_Int32 nDim) override;
    virtual sal_uInt32              GetNumberFormat(sal_Int32 nDim) override;
    virtual void                    DisposeData() override;
    virtual void                    SetEmptyFlags( bool bIgnoreEmptyRows, bool bRepeatIfEmpty ) override;

    virtual bool                    IsRepeatIfEmpty() override;

    virtual void                    CreateCacheTable() override;
    virtual void                    FilterCacheTable(const std::vector<ScDPFilteredCache::Criterion>& rCriteria, const std::unordered_set<sal_Int32>& rDataDims) override;
    virtual void                    GetDrillDownData(const std::vector<ScDPFilteredCache::Criterion>& rCriteria,
                                                     const std::unordered_set<sal_Int32>& rCatDims,
                                                     css::uno::Sequence< css::uno::Sequence< css::uno::Any > >& rData) override;
    virtual void                    CalcResults(CalcInfo& rInfo, bool bAutoShow) override;
    virtual const ScDPFilteredCache&   GetCacheTable() const override;
    virtual void ReloadCacheTable() override;

    virtual bool                    IsBaseForGroup(sal_Int32 nDim) const override;
    virtual sal_Int32               GetGroupBase(sal_Int32 nGroupDim) const override;
    virtual bool                    IsNumOrDateGroup(sal_Int32 nDim) const override;
    virtual bool                    IsInGroup( const ScDPItemData& rGroupData, sal_Int32 nGroupIndex,
                                               const ScDPItemData& rBaseData, sal_Int32 nBaseIndex ) const override;
    virtual bool                    HasCommonElement( const ScDPItemData& rFirstData, sal_Int32 nFirstIndex,
                                                      const ScDPItemData& rSecondData, sal_Int32 nSecondIndex ) const override;

#if DUMP_PIVOT_TABLE
    virtual void Dump() const override;
#endif
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
