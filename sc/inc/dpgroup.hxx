/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dpgroup.hxx,v $
 * $Revision: 1.7 $
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
#include <hash_set>

#include "dptabdat.hxx"
#include "scdllapi.h"

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

    ScDPNumGroupInfo() : Enable(sal_False), DateValues(sal_False), AutoStart(sal_False), AutoEnd(sal_False),
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

    void        FillColumnEntries( TypedScStrCollection& rEntries, const TypedScStrCollection& rOriginal,
                                    SvNumberFormatter* pFormatter ) const;
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
    mutable TypedScStrCollection* pCollection;        // collection of item names (cached)

public:
                ScDPGroupDimension( long nSource, const String& rNewName );
                ScDPGroupDimension( const ScDPGroupDimension& rOther );
                ~ScDPGroupDimension();

    ScDPGroupDimension& operator=( const ScDPGroupDimension& rOther );

    void        AddItem( const ScDPGroupItem& rItem );
    void        SetGroupDim( long nDim );           // called from AddGroupDimension

    long        GetSourceDim() const    { return nSourceDim; }
    long        GetGroupDim() const     { return nGroupDim; }
    const String& GetName() const       { return aGroupName; }

    const TypedScStrCollection& GetColumnEntries( const TypedScStrCollection& rOriginal, ScDocument* pDoc ) const;
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
    mutable TypedScStrCollection* pCollection;        // collection of item names (cached)
    mutable bool                bHasNonInteger;     // initialized in GetNumEntries
    mutable sal_Unicode         cDecSeparator;      // initialized in GetNumEntries

public:
                ScDPNumGroupDimension();
                ScDPNumGroupDimension( const ScDPNumGroupInfo& rInfo );
                ScDPNumGroupDimension( const ScDPNumGroupDimension& rOther );
                ~ScDPNumGroupDimension();

    ScDPNumGroupDimension&  operator=( const ScDPNumGroupDimension& rOther );

    const TypedScStrCollection& GetNumEntries( const TypedScStrCollection& rOriginal, ScDocument* pDoc ) const;

    const ScDPNumGroupInfo& GetInfo() const     { return aGroupInfo; }
    bool        HasNonInteger() const           { return bHasNonInteger; }
    sal_Unicode GetDecSeparator() const         { return cDecSeparator; }

    const ScDPDateGroupHelper* GetDateHelper() const    { return pDateHelper; }

    void        MakeDateHelper( const ScDPNumGroupInfo& rInfo, sal_Int32 nPart );

    void        DisposeData();
};

// --------------------------------------------------------------------
//
//  proxy implementation of ScDPTableData to add grouped items
//

class ScDPGroupTableData : public ScDPTableData
{
    typedef ::std::hash_set< ::rtl::OUString, ::rtl::OUStringHash, ::std::equal_to< ::rtl::OUString > > StringHashSet;

    ScDPTableData*          pSourceData;
    long                    nSourceCount;
    ScDPGroupDimensionVec   aGroups;
    ScDPNumGroupDimension*  pNumGroups;     // array[nSourceCount]
    ScDocument*             pDoc;
    StringHashSet           aGroupNames;

    void        FillGroupValues( ScDPItemData* pItemData, long nCount, const long* pDims );
    void        CopyFields(const ::std::vector<long>& rFieldDims, ::std::vector<long>& rNewFieldDims);

    bool        IsNumGroupDimension( long nDimension ) const;
    void        GetNumGroupInfo( long nDimension, ScDPNumGroupInfo& rInfo,
                                    bool& rNonInteger, sal_Unicode& rDecimal );

    void        ModifyFilterCriteria(::std::vector<ScDPCacheTable::Criterion>& rCriteria);

public:
                // takes ownership of pSource
                ScDPGroupTableData( ScDPTableData* pSource, ScDocument* pDocument );
    virtual     ~ScDPGroupTableData();

    void        AddGroupDimension( const ScDPGroupDimension& rGroup );
    void        SetNumGroupDimension( long nIndex, const ScDPNumGroupDimension& rGroup );
    long        GetDimensionIndex( const String& rName );

    ScDocument* GetDocument()   { return pDoc; }

    virtual long                    GetColumnCount();
    virtual const TypedScStrCollection& GetColumnEntries(long nColumn);
    virtual String                  getDimensionName(long nColumn);
    virtual BOOL                    getIsDataLayoutDimension(long nColumn);
    virtual BOOL                    IsDateDimension(long nDim);
    virtual UINT32                  GetNumberFormat(long nDim);
    virtual void                    DisposeData();
    virtual void                    SetEmptyFlags( BOOL bIgnoreEmptyRows, BOOL bRepeatIfEmpty );

    virtual bool                    IsRepeatIfEmpty();

    virtual void                    CreateCacheTable();
    virtual void                    FilterCacheTable(const ::std::vector<ScDPCacheTable::Criterion>& rCriteria, const ::std::hash_set<sal_Int32>& rDataDims);
    virtual void                    GetDrillDownData(const ::std::vector<ScDPCacheTable::Criterion>& rCriteria,
                                                     const ::std::hash_set<sal_Int32>& rCatDims,
                                                     ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > >& rData);
    virtual void                    CalcResults(CalcInfo& rInfo, bool bAutoShow);
    virtual const ScDPCacheTable&   GetCacheTable() const;

    virtual BOOL                    IsBaseForGroup(long nDim) const;
    virtual long                    GetGroupBase(long nGroupDim) const;
    virtual BOOL                    IsNumOrDateGroup(long nDim) const;
    virtual BOOL                    IsInGroup( const ScDPItemData& rGroupData, long nGroupIndex,
                                               const ScDPItemData& rBaseData, long nBaseIndex ) const;
    virtual BOOL                    HasCommonElement( const ScDPItemData& rFirstData, long nFirstIndex,
                                                      const ScDPItemData& rSecondData, long nSecondIndex ) const;
};


#endif

