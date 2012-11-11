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

#ifndef SC_DPGROUP_HXX
#define SC_DPGROUP_HXX

#include <vector>
#include <boost/unordered_set.hpp>
#include <boost/shared_ptr.hpp>

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
    rtl::OUString               aGroupName;
    ScDPGroupItemVec            aItems;
    mutable std::vector<SCROW> maMemberEntries;
    bool mbDateDimension;
public:
                ScDPGroupDimension( long nSource, const String& rNewName );
                ScDPGroupDimension( const ScDPGroupDimension& rOther );
                ~ScDPGroupDimension();

    ScDPGroupDimension& operator=( const ScDPGroupDimension& rOther );

    void        AddItem( const ScDPGroupItem& rItem );
    void        SetGroupDim( long nDim );           // called from AddGroupDimension

    long        GetSourceDim() const    { return nSourceDim; }
    long        GetGroupDim() const     { return nGroupDim; }
    const rtl::OUString& GetName() const { return aGroupName; }

    const std::vector< SCROW >&  GetColumnEntries( const ScDPFilteredCache&  rCacheTable ) const;
    const ScDPGroupItem* GetGroupForData( const ScDPItemData& rData ) const;  // rData = entry in original dim.
    const ScDPGroupItem* GetGroupForName( const ScDPItemData& rName ) const;  // rName = entry in group dim.
    const ScDPGroupItem* GetGroupByIndex( size_t nIndex ) const;

    void        DisposeData();

    size_t      GetItemCount() const { return aItems.size(); }

    void SetDateDimension();
    bool IsDateDimension() const;
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

    bool IsDateDimension() const;
};

//
//  proxy implementation of ScDPTableData to add grouped items
//

class ScDPGroupTableData : public ScDPTableData
{
    typedef ::boost::unordered_set< ::rtl::OUString, ::rtl::OUStringHash, ::std::equal_to< ::rtl::OUString > > StringHashSet;

    ::boost::shared_ptr<ScDPTableData> pSourceData;
    long                    nSourceCount;
    ScDPGroupDimensionVec   aGroups;
    ScDPNumGroupDimension*  pNumGroups;     // array[nSourceCount]
    ScDocument*             pDoc;
    StringHashSet           aGroupNames;

    void FillGroupValues(std::vector<SCROW>& rItems, const std::vector<long>& rDims);
    virtual long                GetSourceDim( long nDim );

    bool        IsNumGroupDimension( long nDimension ) const;
    void GetNumGroupInfo(long nDimension, ScDPNumGroupInfo& rInfo);

    void        ModifyFilterCriteria(::std::vector<ScDPFilteredCache::Criterion>& rCriteria);

public:
                // takes ownership of pSource
                ScDPGroupTableData( const ::boost::shared_ptr<ScDPTableData>& pSource, ScDocument* pDocument );
    virtual     ~ScDPGroupTableData();

    boost::shared_ptr<ScDPTableData> GetSourceTableData();

    void        AddGroupDimension( const ScDPGroupDimension& rGroup );
    void        SetNumGroupDimension( long nIndex, const ScDPNumGroupDimension& rGroup );
    long        GetDimensionIndex( const rtl::OUString& rName );

    ScDocument* GetDocument()   { return pDoc; }

    virtual long                    GetColumnCount();
    virtual   long                                          GetMembersCount( long nDim );
    virtual    const std::vector< SCROW >& GetColumnEntries( long nColumn ) ;
    virtual const ScDPItemData* GetMemberById( long nDim, long nId);
    virtual long Compare( long nDim, long nDataId1, long nDataId2);

    virtual rtl::OUString           getDimensionName(long nColumn);
    virtual sal_Bool                    getIsDataLayoutDimension(long nColumn);
    virtual sal_Bool                    IsDateDimension(long nDim);
    virtual sal_uLong                  GetNumberFormat(long nDim);
    virtual void                    DisposeData();
    virtual void                    SetEmptyFlags( sal_Bool bIgnoreEmptyRows, sal_Bool bRepeatIfEmpty );

    virtual bool                    IsRepeatIfEmpty();

    virtual void                    CreateCacheTable();
    virtual void                    FilterCacheTable(const ::std::vector<ScDPFilteredCache::Criterion>& rCriteria, const ::boost::unordered_set<sal_Int32>& rDataDims);
    virtual void                    GetDrillDownData(const ::std::vector<ScDPFilteredCache::Criterion>& rCriteria,
                                                     const ::boost::unordered_set<sal_Int32>& rCatDims,
                                                     ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > >& rData);
    virtual void                    CalcResults(CalcInfo& rInfo, bool bAutoShow);
    virtual const ScDPFilteredCache&   GetCacheTable() const;
    virtual void ReloadCacheTable();

    virtual sal_Bool                    IsBaseForGroup(long nDim) const;
    virtual long                    GetGroupBase(long nGroupDim) const;
    virtual sal_Bool                    IsNumOrDateGroup(long nDim) const;
    virtual sal_Bool                    IsInGroup( const ScDPItemData& rGroupData, long nGroupIndex,
                                               const ScDPItemData& rBaseData, long nBaseIndex ) const;
    virtual sal_Bool                    HasCommonElement( const ScDPItemData& rFirstData, long nFirstIndex,
                                                      const ScDPItemData& rSecondData, long nSecondIndex ) const;

#if DEBUG_PIVOT_TABLE
    virtual void Dump() const;
#endif
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
