/*************************************************************************
 *
 *  $RCSfile: dpgroup.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-03 11:28:49 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SC_DPGROUP_HXX
#define SC_DPGROUP_HXX

#include <vector>

#ifndef SC_DPTABDAT_HXX
#include "dptabdat.hxx"
#endif

class ScDocument;
class SvNumberFormatter;

// --------------------------------------------------------------------

//! API constants?
const sal_Int32 SC_DP_DATE_SECONDS  = 1;
const sal_Int32 SC_DP_DATE_MINUTES  = 2;
const sal_Int32 SC_DP_DATE_HOURS    = 4;
const sal_Int32 SC_DP_DATE_DAYS     = 8;
const sal_Int32 SC_DP_DATE_MONTHS   = 16;
const sal_Int32 SC_DP_DATE_QUARTERS = 32;
const sal_Int32 SC_DP_DATE_YEARS    = 64;

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

    void        FillColumnEntries( TypedStrCollection& rEntries, const TypedStrCollection& rOriginal,
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
};

typedef ::std::vector<ScDPGroupItem> ScDPGroupItemVec;

class ScDPGroupDimension
{
    long                        nSourceDim;
    long                        nGroupDim;
    String                      aGroupName;
    ScDPDateGroupHelper*        pDateHelper;
    ScDPGroupItemVec            aItems;
    mutable TypedStrCollection* pCollection;        // collection of item names (cached)

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

    const TypedStrCollection& GetColumnEntries( const TypedStrCollection& rOriginal, ScDocument* pDoc ) const;
    const ScDPGroupItem* GetGroupForData( const ScDPItemData& rData ) const;  // rData = entry in original dim.
    const ScDPGroupItem* GetGroupForName( const ScDPItemData& rName ) const;  // rName = entry in group dim.

    const ScDPDateGroupHelper* GetDateHelper() const    { return pDateHelper; }

    void        MakeDateHelper( const ScDPNumGroupInfo& rInfo, sal_Int32 nPart );

    void        DisposeData();
};

typedef ::std::vector<ScDPGroupDimension> ScDPGroupDimensionVec;

// --------------------------------------------------------------------

class ScDPNumGroupDimension
{
    ScDPNumGroupInfo            aGroupInfo;         // settings
    ScDPDateGroupHelper*        pDateHelper;
    mutable TypedStrCollection* pCollection;        // collection of item names (cached)
    mutable bool                bHasNonInteger;     // initialized in GetNumEntries
    mutable sal_Unicode         cDecSeparator;      // initialized in GetNumEntries

public:
                ScDPNumGroupDimension();
                ScDPNumGroupDimension( const ScDPNumGroupInfo& rInfo );
                ScDPNumGroupDimension( const ScDPNumGroupDimension& rOther );
                ~ScDPNumGroupDimension();

    ScDPNumGroupDimension&  operator=( const ScDPNumGroupDimension& rOther );

    const TypedStrCollection& GetNumEntries( const TypedStrCollection& rOriginal, ScDocument* pDoc ) const;

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
    ScDPTableData*          pSourceData;
    long                    nSourceCount;
    ScDPGroupDimensionVec   aGroups;
    ScDPNumGroupDimension*  pNumGroups;     // array[nSourceCount]
    ScDocument*             pDoc;

    void        FillGroupValues( ScDPItemData* pItemData, long nCount, const long* pDims );
    long*       CopyFields( const long* pSourceDims, long nCount );

    bool        IsNumGroupDimension( long nDimension ) const;
    void        GetNumGroupInfo( long nDimension, ScDPNumGroupInfo& rInfo,
                                    bool& rNonInteger, sal_Unicode& rDecimal );

public:
                // takes ownership of pSource
                ScDPGroupTableData( ScDPTableData* pSource, ScDocument* pDocument );
    virtual     ~ScDPGroupTableData();

    void        AddGroupDimension( const ScDPGroupDimension& rGroup );
    void        SetNumGroupDimension( long nIndex, const ScDPNumGroupDimension& rGroup );
    long        GetDimensionIndex( const String& rName );

    ScDocument* GetDocument()   { return pDoc; }

    virtual long                    GetColumnCount();
    virtual const TypedStrCollection& GetColumnEntries(long nColumn);
    virtual String                  getDimensionName(long nColumn);
    virtual BOOL                    getIsDataLayoutDimension(long nColumn);
    virtual BOOL                    IsDateDimension(long nDim);
    virtual UINT32                  GetNumberFormat(long nDim);
    virtual void                    DisposeData();
    virtual void                    SetEmptyFlags( BOOL bIgnoreEmptyRows, BOOL bRepeatIfEmpty );

    virtual void                    ResetIterator();
    virtual BOOL                    GetNextRow( const ScDPTableIteratorParam& rParam );

    virtual BOOL                    IsBaseForGroup(long nDim) const;
    virtual long                    GetGroupBase(long nGroupDim) const;
    virtual BOOL                    IsNumOrDateGroup(long nDim) const;
    virtual BOOL                    IsInGroup( const ScDPItemData& rGroupData, long nGroupIndex,
                                               const ScDPItemData& rBaseData, long nBaseIndex ) const;
    virtual BOOL                    HasCommonElement( const ScDPItemData& rFirstData, long nFirstIndex,
                                                      const ScDPItemData& rSecondData, long nSecondIndex ) const;
};


#endif

