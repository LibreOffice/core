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
#include "dpglobal.hxx"
class ScDocument;
class SvNumberFormatter;

// --------------------------------------------------------------------

//! API struct?
struct ScDPNumGroupInfo
{
    sal_Bool Enable;
    sal_Bool DateValues;
    sal_Bool AutoStart;
    sal_Bool AutoEnd;
    double   Start;
    double   End;
    double   Step;

    ScDPNumGroupInfo() : Enable(false), DateValues(false), AutoStart(false), AutoEnd(false),
                         Start(0.0), End(0.0), Step(0.0) {}
};

// --------------------------------------------------------------------

//  ScDPDateGroupHelper is used as part of ScDPGroupDimension (additional dim.)
//  or ScDPNumGroupDimension (innermost, replaces the original dim.).
//  Source index, name and result collection are stored at the parent.

class ScDPDateGroupHelper
{
    ScDPNumGroupInfo    aNumInfo;       // only start and end (incl. auto flags) are used
    sal_Int32           nDatePart;      // single part

public:
                ScDPDateGroupHelper( const ScDPNumGroupInfo& rInfo, sal_Int32 nPart );
                ~ScDPDateGroupHelper();

    sal_Int32   GetDatePart() const { return nDatePart; }
    const ScDPNumGroupInfo& GetNumInfo() const { return aNumInfo; }

    void FillColumnEntries(
        SCCOL nSourceDim, const ScDPCache* pCahe , std::vector<SCROW>& rEntries,
        const std::vector<SCROW>& rOriginal) const;
};

// --------------------------------------------------------------------

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

    void        FillGroupFilter( ScDPCacheTable::GroupFilter& rFilter ) const;
};

typedef ::std::vector<ScDPGroupItem> ScDPGroupItemVec;

class ScDPGroupDimension
{
    long                        nSourceDim;
    long                        nGroupDim;
    String                      aGroupName;
    ScDPDateGroupHelper*        pDateHelper;
    ScDPGroupItemVec            aItems;
   mutable  ::std::vector< SCROW >            maMemberEntries;
public:
                ScDPGroupDimension( long nSource, const String& rNewName );
                ScDPGroupDimension( const ScDPGroupDimension& rOther );
                ~ScDPGroupDimension();

    ScDPGroupDimension& operator=( const ScDPGroupDimension& rOther );

    void        AddItem( const ScDPGroupItem& rItem );
    void        SetGroupDim( long nDim );           // called from AddGroupDimension

    long        GetSourceDim() const    { return nSourceDim; }
    long        GetGroupDim() const     { return nGroupDim; }
    const      String& GetName() const       { return aGroupName; }

    const std::vector< SCROW >&  GetColumnEntries( const ScDPCacheTable&  rCacheTable, const std::vector< SCROW >& rOriginal ) const;
    const ScDPGroupItem* GetGroupForData( const ScDPItemData& rData ) const;  // rData = entry in original dim.
    const ScDPGroupItem* GetGroupForName( const ScDPItemData& rName ) const;  // rName = entry in group dim.
    const ScDPGroupItem* GetGroupByIndex( size_t nIndex ) const;

    const ScDPDateGroupHelper* GetDateHelper() const    { return pDateHelper; }

    void        MakeDateHelper( const ScDPNumGroupInfo& rInfo, sal_Int32 nPart );

    void        DisposeData();

    size_t      GetItemCount() const { return aItems.size(); }
};

typedef ::std::vector<ScDPGroupDimension> ScDPGroupDimensionVec;

// --------------------------------------------------------------------

class SC_DLLPUBLIC ScDPNumGroupDimension
{
    ScDPNumGroupInfo            aGroupInfo;         // settings
    ScDPDateGroupHelper*        pDateHelper;
    mutable  ::std::vector< SCROW >            maMemberEntries;
    mutable bool                bHasNonInteger;     // initialized in GetNumEntries
    mutable sal_Unicode         cDecSeparator;      // initialized in GetNumEntries

public:
                ScDPNumGroupDimension();
                ScDPNumGroupDimension( const ScDPNumGroupInfo& rInfo );
                ScDPNumGroupDimension( const ScDPNumGroupDimension& rOther );
                ~ScDPNumGroupDimension();

    ScDPNumGroupDimension&  operator=( const ScDPNumGroupDimension& rOther );

    const ScDPNumGroupInfo& GetInfo() const     { return aGroupInfo; }
    bool        HasNonInteger() const           { return bHasNonInteger; }
    sal_Unicode GetDecSeparator() const         { return cDecSeparator; }

    const ScDPDateGroupHelper* GetDateHelper() const    { return pDateHelper; }

    const std::vector<SCROW>& GetNumEntries(
        SCCOL nSourceDim, const ScDPCache* pCache,
        const std::vector< SCROW >& rOriginal) const;

    void        MakeDateHelper( const ScDPNumGroupInfo& rInfo, sal_Int32 nPart );

    void        DisposeData();
};

// --------------------------------------------------------------------
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

    void                         FillGroupValues( SCROW* pItemDataIndex, long nCount, const long* pDims );
    virtual long                GetSourceDim( long nDim );

    bool        IsNumGroupDimension( long nDimension ) const;
    void        GetNumGroupInfo( long nDimension, ScDPNumGroupInfo& rInfo,
                                    bool& rNonInteger, sal_Unicode& rDecimal );

    void        ModifyFilterCriteria(::std::vector<ScDPCacheTable::Criterion>& rCriteria);

public:
                // takes ownership of pSource
                ScDPGroupTableData( const ::boost::shared_ptr<ScDPTableData>& pSource, ScDocument* pDocument );
    virtual     ~ScDPGroupTableData();

    void        AddGroupDimension( const ScDPGroupDimension& rGroup );
    void        SetNumGroupDimension( long nIndex, const ScDPNumGroupDimension& rGroup );
    long        GetDimensionIndex( const String& rName );

    ScDocument* GetDocument()   { return pDoc; }

    virtual long                    GetColumnCount();
    virtual   long                                          GetMembersCount( long nDim );
    virtual    const std::vector< SCROW >& GetColumnEntries( long nColumn ) ;
    virtual const ScDPItemData* GetMemberById( long nDim, long nId);
    virtual long Compare( long nDim, long nDataId1, long nDataId2);

    virtual String                  getDimensionName(long nColumn);
    virtual sal_Bool                    getIsDataLayoutDimension(long nColumn);
    virtual sal_Bool                    IsDateDimension(long nDim);
    virtual sal_uLong                  GetNumberFormat(long nDim);
    virtual void                    DisposeData();
    virtual void                    SetEmptyFlags( sal_Bool bIgnoreEmptyRows, sal_Bool bRepeatIfEmpty );

    virtual bool                    IsRepeatIfEmpty();

    virtual void                    CreateCacheTable();
    virtual void                    FilterCacheTable(const ::std::vector<ScDPCacheTable::Criterion>& rCriteria, const ::boost::unordered_set<sal_Int32>& rDataDims);
    virtual void                    GetDrillDownData(const ::std::vector<ScDPCacheTable::Criterion>& rCriteria,
                                                     const ::boost::unordered_set<sal_Int32>& rCatDims,
                                                     ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > >& rData);
    virtual void                    CalcResults(CalcInfo& rInfo, bool bAutoShow);
    virtual const ScDPCacheTable&   GetCacheTable() const;

    virtual sal_Bool                    IsBaseForGroup(long nDim) const;
    virtual long                    GetGroupBase(long nGroupDim) const;
    virtual sal_Bool                    IsNumOrDateGroup(long nDim) const;
    virtual sal_Bool                    IsInGroup( const ScDPItemData& rGroupData, long nGroupIndex,
                                               const ScDPItemData& rBaseData, long nBaseIndex ) const;
    virtual sal_Bool                    HasCommonElement( const ScDPItemData& rFirstData, long nFirstIndex,
                                                      const ScDPItemData& rSecondData, long nSecondIndex ) const;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
