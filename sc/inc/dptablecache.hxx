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
#include <svl/zforlist.hxx>
#include "dpglobal.hxx"

#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>

#include <vector>
#include <boost/ptr_container/ptr_vector.hpp>

struct ScQueryParam;

class SC_DLLPUBLIC ScDPTableDataCache
{
    friend class ScDPCacheTable;
public:
    typedef ::boost::ptr_vector<ScDPItemData>           DataListType;
private:
    typedef ::boost::ptr_vector<DataListType>           DataGridType;
    typedef ::boost::ptr_vector< ::std::vector<SCROW> > RowGridType;

    ScDocument* mpDoc;
    long mnColumnCount;

    DataGridType                maTableDataValues; // Data Pilot Table's index - value map
    RowGridType                 maSourceData;      // Data Pilot Table's source data
    RowGridType                 maGlobalOrder;     // Sorted members index
    mutable RowGridType         maIndexOrder;      // Index the sorted numbers
    DataListType                maLabelNames;      // Source label data
    std::vector<bool>           mbEmptyRow;        //If empty row?

    mutable ScDPItemDataPool    maAdditionalData;

public:
    SCROW GetIdByItemData( long nDim,  String sItemData  ) const;
    SCROW GetIdByItemData( long nDim, const ScDPItemData& rData ) const;

    SCROW GetAdditionalItemID ( String sItemData ) const;
    SCROW GetAdditionalItemID( const ScDPItemData& rData ) const;

    SCCOL GetDimensionIndex( String sName) const;
    const ScDPItemData* GetSortedItemData( SCCOL nDim, SCROW nOrder ) const;
    ULONG GetNumType ( ULONG nFormat ) const;
    ULONG GetNumberFormat( long nDim ) const;
    bool  IsDateDimension( long nDim ) const ;
    ULONG GetDimNumType( SCCOL nDim) const;
    SCROW GetDimMemberCount( SCCOL nDim ) const;

    SCROW GetSortedItemDataId( SCCOL nDim, SCROW nOrder ) const;
    const DataListType& GetDimMemberValues( SCCOL nDim ) const;
    bool InitFromDoc(ScDocument* pDoc, const ScRange& rRange);
    bool InitFromDataBase(const  ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet>& xRowSet, const Date& rNullDate);

    SCROW  GetRowCount() const;
    SCROW  GetItemDataId( USHORT nDim, SCROW nRow, bool bRepeatIfEmpty ) const;
    String GetDimensionName( USHORT nColumn ) const;
    bool IsEmptyMember( SCROW nRow, USHORT nColumn ) const;
    bool IsRowEmpty( SCROW nRow ) const;
    bool IsValid() const;
    bool ValidQuery( SCROW nRow, const ScQueryParam& rQueryParam, bool* pSpecial );

    ScDocument* GetDoc() const;//ms-cache-core
    long GetColumnCount() const;

    const ScDPItemData* GetItemDataById( long nDim, SCROW nId ) const;

    bool operator== ( const ScDPTableDataCache& r ) const;

    ScDPTableDataCache(ScDocument* pDoc);
    virtual ~ScDPTableDataCache();

private:
    SCROW GetOrder( long nDim, SCROW nIndex ) const;
    void AddLabel( ScDPItemData* pData);
    bool AddData( long nDim, ScDPItemData* itemData );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
