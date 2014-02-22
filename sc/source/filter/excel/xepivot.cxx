/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "xepivot.hxx"
#include <com/sun/star/sheet/DataPilotFieldSortInfo.hpp>
#include <com/sun/star/sheet/DataPilotFieldAutoShowInfo.hpp>
#include <com/sun/star/sheet/DataPilotFieldLayoutInfo.hpp>
#include <com/sun/star/sheet/DataPilotFieldReference.hpp>
#include <com/sun/star/sheet/DataPilotFieldGroupBy.hpp>

#include <algorithm>
#include <math.h>

#include <rtl/math.hxx>
#include <tools/date.hxx>
#include <svl/zformat.hxx>
#include <sot/storage.hxx>
#include "document.hxx"
#include "dpobject.hxx"
#include "dpsave.hxx"
#include "dpdimsave.hxx"
#include "dpshttab.hxx"
#include "globstr.hrc"
#include "fapihelper.hxx"
#include "xestring.hxx"
#include "xelink.hxx"
#include "dputil.hxx"

using namespace ::oox;

using ::com::sun::star::sheet::DataPilotFieldOrientation;
using ::com::sun::star::sheet::DataPilotFieldOrientation_HIDDEN;
using ::com::sun::star::sheet::DataPilotFieldOrientation_ROW;
using ::com::sun::star::sheet::DataPilotFieldOrientation_COLUMN;
using ::com::sun::star::sheet::DataPilotFieldOrientation_PAGE;
using ::com::sun::star::sheet::DataPilotFieldOrientation_DATA;
using ::com::sun::star::sheet::GeneralFunction;
using ::com::sun::star::sheet::DataPilotFieldSortInfo;
using ::com::sun::star::sheet::DataPilotFieldAutoShowInfo;
using ::com::sun::star::sheet::DataPilotFieldLayoutInfo;
using ::com::sun::star::sheet::DataPilotFieldReference;






namespace {


const sal_uInt16 EXC_PCITEM_DATA_STRING     = 0x0001;   
const sal_uInt16 EXC_PCITEM_DATA_DOUBLE     = 0x0002;   
const sal_uInt16 EXC_PCITEM_DATA_INTEGER    = 0x0004;   
const sal_uInt16 EXC_PCITEM_DATA_DATE       = 0x0008;   

/** Maps a bitfield consisting of EXC_PCITEM_DATA_* flags above to SXFIELD data type bitfield. */
static const sal_uInt16 spnPCItemFlags[] =
{                               
    EXC_SXFIELD_DATA_NONE,      //
    EXC_SXFIELD_DATA_STR,       
    EXC_SXFIELD_DATA_INT,       
    EXC_SXFIELD_DATA_STR_INT,   
    EXC_SXFIELD_DATA_DBL,       
    EXC_SXFIELD_DATA_STR_DBL,   
    EXC_SXFIELD_DATA_INT,       
    EXC_SXFIELD_DATA_STR_INT,   
    EXC_SXFIELD_DATA_DATE,      
    EXC_SXFIELD_DATA_DATE_STR,  
    EXC_SXFIELD_DATA_DATE_NUM,  
    EXC_SXFIELD_DATA_DATE_STR,  
    EXC_SXFIELD_DATA_DATE_NUM,  
    EXC_SXFIELD_DATA_DATE_STR,  
    EXC_SXFIELD_DATA_DATE_NUM,  
    EXC_SXFIELD_DATA_DATE_STR   
};

} 



XclExpPCItem::XclExpPCItem( const OUString& rText ) :
    XclExpRecord( (!rText.isEmpty()) ? EXC_ID_SXSTRING : EXC_ID_SXEMPTY, 0 ),
    mnTypeFlag( EXC_PCITEM_DATA_STRING )
{
    if( !rText.isEmpty() )
        SetText( rText );
    else
        SetEmpty();
}

XclExpPCItem::XclExpPCItem( double fValue ) :
    XclExpRecord( EXC_ID_SXDOUBLE, 8 )
{
    SetDouble( fValue );
    mnTypeFlag = (fValue - floor( fValue ) == 0.0) ?
        EXC_PCITEM_DATA_INTEGER : EXC_PCITEM_DATA_DOUBLE;
}

XclExpPCItem::XclExpPCItem( const DateTime& rDateTime ) :
    XclExpRecord( EXC_ID_SXDATETIME, 8 )
{
    SetDateTime( rDateTime );
    mnTypeFlag = EXC_PCITEM_DATA_DATE;
}

XclExpPCItem::XclExpPCItem( sal_Int16 nValue ) :
    XclExpRecord( EXC_ID_SXINTEGER, 2 ),
    mnTypeFlag( EXC_PCITEM_DATA_INTEGER )
{
    SetInteger( nValue );
}

XclExpPCItem::XclExpPCItem( bool bValue ) :
    XclExpRecord( EXC_ID_SXBOOLEAN, 2 ),
    mnTypeFlag( EXC_PCITEM_DATA_STRING )
{
    SetBool( bValue );
}



bool XclExpPCItem::EqualsText( const OUString& rText ) const
{
    return rText.isEmpty() ? IsEmpty() : (GetText() && (*GetText() == rText));
}

bool XclExpPCItem::EqualsDouble( double fValue ) const
{
    return GetDouble() && (*GetDouble() == fValue);
}

bool XclExpPCItem::EqualsDateTime( const DateTime& rDateTime ) const
{
    return GetDateTime() && (*GetDateTime() == rDateTime);
}

bool XclExpPCItem::EqualsBool( bool bValue ) const
{
    return GetBool() && (*GetBool() == bValue);
}



void XclExpPCItem::WriteBody( XclExpStream& rStrm )
{
    if( const OUString* pText = GetText() )
    {
        rStrm << XclExpString( *pText );
    }
    else if( const double* pfValue = GetDouble() )
    {
        rStrm << *pfValue;
    }
    else if( const sal_Int16* pnValue = GetInteger() )
    {
        rStrm << *pnValue;
    }
    else if( const DateTime* pDateTime = GetDateTime() )
    {
        sal_uInt16 nYear = static_cast< sal_uInt16 >( pDateTime->GetYear() );
        sal_uInt16 nMonth = static_cast< sal_uInt16 >( pDateTime->GetMonth() );
        sal_uInt8 nDay = static_cast< sal_uInt8 >( pDateTime->GetDay() );
        sal_uInt8 nHour = static_cast< sal_uInt8 >( pDateTime->GetHour() );
        sal_uInt8 nMin = static_cast< sal_uInt8 >( pDateTime->GetMin() );
        sal_uInt8 nSec = static_cast< sal_uInt8 >( pDateTime->GetSec() );
        if( nYear < 1900 ) { nYear = 1900; nMonth = 1; nDay = 0; }
        rStrm << nYear << nMonth << nDay << nHour << nMin << nSec;
    }
    else if( const bool* pbValue = GetBool() )
    {
        rStrm << static_cast< sal_uInt16 >( *pbValue ? 1 : 0 );
    }
    else
    {
        
        OSL_ENSURE( IsEmpty(), "XclExpPCItem::WriteBody - no data found" );
    }
}



XclExpPCField::XclExpPCField(
        const XclExpRoot& rRoot, const XclExpPivotCache& rPCache, sal_uInt16 nFieldIdx,
        const ScDPObject& rDPObj, const ScRange& rRange ) :
    XclExpRecord( EXC_ID_SXFIELD ),
    XclPCField( EXC_PCFIELD_STANDARD, nFieldIdx ),
    XclExpRoot( rRoot ),
    mrPCache( rPCache ),
    mnTypeFlags( 0 )
{
    
    InitStandardField( rRange );

    
    if( const ScDPSaveData* pSaveData = rDPObj.GetSaveData() )
    {
        if( const ScDPDimensionSaveData* pSaveDimData = pSaveData->GetExistingDimensionData() )
        {
            if( const ScDPSaveNumGroupDimension* pNumGroupDim = pSaveDimData->GetNumGroupDim( GetFieldName() ) )
            {
                const ScDPNumGroupInfo& rNumInfo = pNumGroupDim->GetInfo();
                const ScDPNumGroupInfo& rDateInfo = pNumGroupDim->GetDateInfo();
                OSL_ENSURE( !rNumInfo.mbEnable || !rDateInfo.mbEnable,
                    "XclExpPCField::XclExpPCField - numeric and date grouping enabled" );

                if( rNumInfo.mbEnable )
                    InitNumGroupField( rDPObj, rNumInfo );
                else if( rDateInfo.mbEnable )
                    InitDateGroupField( rDPObj, rDateInfo, pNumGroupDim->GetDatePart() );
            }
        }
    }

    
    Finalize();
}

XclExpPCField::XclExpPCField(
        const XclExpRoot& rRoot, const XclExpPivotCache& rPCache, sal_uInt16 nFieldIdx,
        const ScDPObject& rDPObj, const ScDPSaveGroupDimension& rGroupDim, const XclExpPCField& rBaseField ) :
    XclExpRecord( EXC_ID_SXFIELD ),
    XclPCField( EXC_PCFIELD_STDGROUP, nFieldIdx ),
    XclExpRoot( rRoot ),
    mrPCache( rPCache ),
    mnTypeFlags( 0 )
{
    
    OSL_ENSURE( rBaseField.GetFieldName() == rGroupDim.GetSourceDimName(),
        "XclExpPCField::FillFromGroup - wrong base cache field" );
    maFieldInfo.maName = rGroupDim.GetGroupDimName();
    maFieldInfo.mnGroupBase = rBaseField.GetFieldIndex();

    
    const ScDPNumGroupInfo& rDateInfo = rGroupDim.GetDateInfo();
    if( rDateInfo.mbEnable && (rGroupDim.GetDatePart() != 0) )
        InitDateGroupField( rDPObj, rDateInfo, rGroupDim.GetDatePart() );
    else
        InitStdGroupField( rBaseField, rGroupDim );

    
    Finalize();
}

XclExpPCField::~XclExpPCField()
{
}

void XclExpPCField::SetGroupChildField( const XclExpPCField& rChildField )
{
    OSL_ENSURE( !::get_flag( maFieldInfo.mnFlags, EXC_SXFIELD_HASCHILD ),
        "XclExpPCField::SetGroupChildIndex - field already has a grouping child field" );
    ::set_flag( maFieldInfo.mnFlags, EXC_SXFIELD_HASCHILD );
    maFieldInfo.mnGroupChild = rChildField.GetFieldIndex();
}

sal_uInt16 XclExpPCField::GetItemCount() const
{
    return static_cast< sal_uInt16 >( GetVisItemList().GetSize() );
}

const XclExpPCItem* XclExpPCField::GetItem( sal_uInt16 nItemIdx ) const
{
    return GetVisItemList().GetRecord( nItemIdx ).get();
}

sal_uInt16 XclExpPCField::GetItemIndex( const OUString& rItemName ) const
{
    const XclExpPCItemList& rItemList = GetVisItemList();
    for( size_t nPos = 0, nSize = rItemList.GetSize(); nPos < nSize; ++nPos )
        if( rItemList.GetRecord( nPos )->ConvertToText() == rItemName )
            return static_cast< sal_uInt16 >( nPos );
    return EXC_PC_NOITEM;
}

sal_Size XclExpPCField::GetIndexSize() const
{
    return Has16BitIndexes() ? 2 : 1;
}

void XclExpPCField::WriteIndex( XclExpStream& rStrm, sal_uInt32 nSrcRow ) const
{
    
    if( nSrcRow < maIndexVec.size() )
    {
        sal_uInt16 nIndex = maIndexVec[ nSrcRow ];
        if( Has16BitIndexes() )
            rStrm << nIndex;
        else
            rStrm << static_cast< sal_uInt8 >( nIndex );
    }
}

void XclExpPCField::Save( XclExpStream& rStrm )
{
    OSL_ENSURE( IsSupportedField(), "XclExpPCField::Save - unknown field type" );
    
    XclExpRecord::Save( rStrm );
    
    XclExpUInt16Record( EXC_ID_SXFDBTYPE, EXC_SXFDBTYPE_DEFAULT ).Save( rStrm );
    
    maGroupItemList.Save( rStrm );
    
    WriteSxgroupinfo( rStrm );
    
    WriteSxnumgroup( rStrm );
    
    maOrigItemList.Save( rStrm );
}



const XclExpPCField::XclExpPCItemList& XclExpPCField::GetVisItemList() const
{
    OSL_ENSURE( IsStandardField() == maGroupItemList.IsEmpty(),
        "XclExpPCField::GetVisItemList - unexpected additional items in standard field" );
    return IsStandardField() ? maOrigItemList : maGroupItemList;
}

void XclExpPCField::InitStandardField( const ScRange& rRange )
{
    OSL_ENSURE( IsStandardField(), "XclExpPCField::InitStandardField - only for standard fields" );
    OSL_ENSURE( rRange.aStart.Col() == rRange.aEnd.Col(), "XclExpPCField::InitStandardField - cell range with multiple columns" );

    ScDocument& rDoc = GetDoc();
    SvNumberFormatter& rFormatter = GetFormatter();

    
    ScAddress aPos( rRange.aStart );
    maFieldInfo.maName = rDoc.GetString(aPos.Col(), aPos.Row(), aPos.Tab());
    
    if (maFieldInfo.maName.getLength() > EXC_PC_MAXSTRLEN)
        maFieldInfo.maName = maFieldInfo.maName.copy(0, EXC_PC_MAXSTRLEN);

    
    for( aPos.IncRow(); (aPos.Row() <= rRange.aEnd.Row()) && (maOrigItemList.GetSize() < EXC_PC_MAXITEMCOUNT); aPos.IncRow() )
    {
        if( rDoc.HasValueData( aPos.Col(), aPos.Row(), aPos.Tab() ) )
        {
            double fValue = rDoc.GetValue( aPos );
            short nFmtType = rFormatter.GetType( rDoc.GetNumberFormat( aPos ) );
            if( nFmtType == NUMBERFORMAT_LOGICAL )
                InsertOrigBoolItem( fValue != 0 );
            else if( nFmtType & NUMBERFORMAT_DATETIME )
                InsertOrigDateTimeItem( GetDateTimeFromDouble( ::std::max( fValue, 0.0 ) ) );
            else
                InsertOrigDoubleItem( fValue );
        }
        else
        {
            OUString aText = rDoc.GetString(aPos.Col(), aPos.Row(), aPos.Tab());
            InsertOrigTextItem( aText );
        }
    }
}

void XclExpPCField::InitStdGroupField( const XclExpPCField& rBaseField, const ScDPSaveGroupDimension& rGroupDim )
{
    OSL_ENSURE( IsGroupField(), "XclExpPCField::InitStdGroupField - only for standard grouping fields" );

    maFieldInfo.mnBaseItems = rBaseField.GetItemCount();
    maGroupOrder.resize( maFieldInfo.mnBaseItems, EXC_PC_NOITEM );

    
    for( long nGroupIdx = 0, nGroupCount = rGroupDim.GetGroupCount(); nGroupIdx < nGroupCount; ++nGroupIdx )
    {
        if( const ScDPSaveGroupItem* pGroupItem = rGroupDim.GetGroupByIndex( nGroupIdx ) )
        {
            
            sal_uInt16 nGroupItemIdx = EXC_PC_NOITEM;
            
            for( size_t nElemIdx = 0, nElemCount = pGroupItem->GetElementCount(); nElemIdx < nElemCount; ++nElemIdx )
            {
                if (const OUString* pElemName = pGroupItem->GetElementByIndex(nElemIdx))
                {
                    
                    sal_uInt16 nBaseItemIdx = rBaseField.GetItemIndex( *pElemName );
                    if( nBaseItemIdx < maFieldInfo.mnBaseItems )
                    {
                        
                        if( nGroupItemIdx == EXC_PC_NOITEM )
                            nGroupItemIdx = InsertGroupItem( new XclExpPCItem( pGroupItem->GetGroupName() ) );
                        maGroupOrder[ nBaseItemIdx ] = nGroupItemIdx;
                    }
                }
            }
        }
    }

    
    for( sal_uInt16 nBaseItemIdx = 0; nBaseItemIdx < maFieldInfo.mnBaseItems; ++nBaseItemIdx )
        
        if( maGroupOrder[ nBaseItemIdx ] == EXC_PC_NOITEM )
            
            if( const XclExpPCItem* pBaseItem = rBaseField.GetItem( nBaseItemIdx ) )
                
                maGroupOrder[ nBaseItemIdx ] = InsertGroupItem( new XclExpPCItem( *pBaseItem ) );
}

void XclExpPCField::InitNumGroupField( const ScDPObject& rDPObj, const ScDPNumGroupInfo& rNumInfo )
{
    OSL_ENSURE( IsStandardField(), "XclExpPCField::InitNumGroupField - only for standard fields" );
    OSL_ENSURE( rNumInfo.mbEnable, "XclExpPCField::InitNumGroupField - numeric grouping not enabled" );

    
    if( rNumInfo.mbDateValues )
    {
        
        meFieldType = EXC_PCFIELD_DATEGROUP;
        maNumGroupInfo.SetScDateType( com::sun::star::sheet::DataPilotFieldGroupBy::DAYS );
        SetDateGroupLimit( rNumInfo, true );
    }
    else
    {
        meFieldType = EXC_PCFIELD_NUMGROUP;
        maNumGroupInfo.SetNumType();
        SetNumGroupLimit( rNumInfo );
    }

    
    InsertNumDateGroupItems( rDPObj, rNumInfo );
}

void XclExpPCField::InitDateGroupField( const ScDPObject& rDPObj, const ScDPNumGroupInfo& rDateInfo, sal_Int32 nDatePart )
{
    OSL_ENSURE( IsStandardField() || IsStdGroupField(), "XclExpPCField::InitDateGroupField - only for standard fields" );
    OSL_ENSURE( rDateInfo.mbEnable, "XclExpPCField::InitDateGroupField - date grouping not enabled" );

    
    meFieldType = IsStandardField() ? EXC_PCFIELD_DATEGROUP : EXC_PCFIELD_DATECHILD;

    
    maNumGroupInfo.SetScDateType( nDatePart );
    SetDateGroupLimit( rDateInfo, false );

    
    InsertNumDateGroupItems( rDPObj, rDateInfo, nDatePart );
}

void XclExpPCField::InsertItemArrayIndex( size_t nListPos )
{
    OSL_ENSURE( IsStandardField(), "XclExpPCField::InsertItemArrayIndex - only for standard fields" );
    maIndexVec.push_back( static_cast< sal_uInt16 >( nListPos ) );
}

void XclExpPCField::InsertOrigItem( XclExpPCItem* pNewItem )
{
    size_t nItemIdx = maOrigItemList.GetSize();
    maOrigItemList.AppendNewRecord( pNewItem );
    InsertItemArrayIndex( nItemIdx );
    mnTypeFlags |= pNewItem->GetTypeFlag();
}

void XclExpPCField::InsertOrigTextItem( const OUString& rText )
{
    size_t nPos = 0;
    bool bFound = false;
    
    OUString aShortText = rText.copy( 0, ::std::min(rText.getLength(), EXC_PC_MAXSTRLEN ) );
    for( size_t nSize = maOrigItemList.GetSize(); !bFound && (nPos < nSize); ++nPos )
        if( (bFound = maOrigItemList.GetRecord( nPos )->EqualsText( aShortText )) == true )
            InsertItemArrayIndex( nPos );
    if( !bFound )
        InsertOrigItem( new XclExpPCItem( aShortText ) );
}

void XclExpPCField::InsertOrigDoubleItem( double fValue )
{
    size_t nPos = 0;
    bool bFound = false;
    for( size_t nSize = maOrigItemList.GetSize(); !bFound && (nPos < nSize); ++nPos )
        if( (bFound = maOrigItemList.GetRecord( nPos )->EqualsDouble( fValue )) == true )
            InsertItemArrayIndex( nPos );
    if( !bFound )
        InsertOrigItem( new XclExpPCItem( fValue ) );
}

void XclExpPCField::InsertOrigDateTimeItem( const DateTime& rDateTime )
{
    size_t nPos = 0;
    bool bFound = false;
    for( size_t nSize = maOrigItemList.GetSize(); !bFound && (nPos < nSize); ++nPos )
        if( (bFound = maOrigItemList.GetRecord( nPos )->EqualsDateTime( rDateTime )) == true )
            InsertItemArrayIndex( nPos );
    if( !bFound )
        InsertOrigItem( new XclExpPCItem( rDateTime ) );
}

void XclExpPCField::InsertOrigBoolItem( bool bValue )
{
    size_t nPos = 0;
    bool bFound = false;
    for( size_t nSize = maOrigItemList.GetSize(); !bFound && (nPos < nSize); ++nPos )
        if( (bFound = maOrigItemList.GetRecord( nPos )->EqualsBool( bValue )) == true )
            InsertItemArrayIndex( nPos );
    if( !bFound )
        InsertOrigItem( new XclExpPCItem( bValue ) );
}

sal_uInt16 XclExpPCField::InsertGroupItem( XclExpPCItem* pNewItem )
{
    maGroupItemList.AppendNewRecord( pNewItem );
    return static_cast< sal_uInt16 >( maGroupItemList.GetSize() - 1 );
}

void XclExpPCField::InsertNumDateGroupItems( const ScDPObject& rDPObj, const ScDPNumGroupInfo& rNumInfo, sal_Int32 nDatePart )
{
    OSL_ENSURE( rDPObj.GetSheetDesc(), "XclExpPCField::InsertNumDateGroupItems - cannot generate element list" );
    if( const ScSheetSourceDesc* pSrcDesc = rDPObj.GetSheetDesc() )
    {
        
        const ScDPSaveData* pSaveData = rDPObj.GetSaveData();
        const ScDPDimensionSaveData* pDimData = NULL;
        if (pSaveData)
            pDimData = pSaveData->GetExistingDimensionData();

        const ScDPCache* pCache = pSrcDesc->CreateCache(pDimData);
        if (!pCache)
            return;

        ScSheetDPData aDPData(GetDocPtr(), *pSrcDesc, *pCache);
        long nDim = GetFieldIndex();
        
        ScDPNumGroupDimension aTmpDim( rNumInfo );
        if( nDatePart != 0 )
            aTmpDim.SetDateDimension();
        const std::vector<SCROW>& aMemberIds = aTmpDim.GetNumEntries(
            static_cast<SCCOL>(nDim), pCache);
        for ( size_t  nIdx = 0 ; nIdx < aMemberIds.size(); nIdx++ )
        {
            const ScDPItemData* pData = aDPData.GetMemberById(nDim , aMemberIds[nIdx]);
            if ( pData )
            {
                OUString aStr = pCache->GetFormattedString(nDim, *pData);
                InsertGroupItem(new XclExpPCItem(aStr));
            }
        }
    }
}

void XclExpPCField::SetNumGroupLimit( const ScDPNumGroupInfo& rNumInfo )
{
    ::set_flag( maNumGroupInfo.mnFlags, EXC_SXNUMGROUP_AUTOMIN, rNumInfo.mbAutoStart );
    ::set_flag( maNumGroupInfo.mnFlags, EXC_SXNUMGROUP_AUTOMAX, rNumInfo.mbAutoEnd );
    maNumGroupLimits.AppendNewRecord( new XclExpPCItem( rNumInfo.mfStart ) );
    maNumGroupLimits.AppendNewRecord( new XclExpPCItem( rNumInfo.mfEnd ) );
    maNumGroupLimits.AppendNewRecord( new XclExpPCItem( rNumInfo.mfStep ) );
}

void XclExpPCField::SetDateGroupLimit( const ScDPNumGroupInfo& rDateInfo, bool bUseStep )
{
    ::set_flag( maNumGroupInfo.mnFlags, EXC_SXNUMGROUP_AUTOMIN, rDateInfo.mbAutoStart );
    ::set_flag( maNumGroupInfo.mnFlags, EXC_SXNUMGROUP_AUTOMAX, rDateInfo.mbAutoEnd );
    maNumGroupLimits.AppendNewRecord( new XclExpPCItem( GetDateTimeFromDouble( rDateInfo.mfStart ) ) );
    maNumGroupLimits.AppendNewRecord( new XclExpPCItem( GetDateTimeFromDouble( rDateInfo.mfEnd ) ) );
    sal_Int16 nStep = bUseStep ? limit_cast< sal_Int16 >( rDateInfo.mfStep, 1, SAL_MAX_INT16 ) : 1;
    maNumGroupLimits.AppendNewRecord( new XclExpPCItem( nStep ) );
}

void XclExpPCField::Finalize()
{
    
    ::set_flag( maFieldInfo.mnFlags, EXC_SXFIELD_HASITEMS, !GetVisItemList().IsEmpty() );
    
    ::set_flag( maFieldInfo.mnFlags, EXC_SXFIELD_16BIT, maOrigItemList.GetSize() >= 0x0100 );
    ::set_flag( maFieldInfo.mnFlags, EXC_SXFIELD_NUMGROUP, IsNumGroupField() || IsDateGroupField() );
    /*  mnTypeFlags is updated in all Insert***Item() functions. Now the flags
        for the current combination of item types is added to the flags. */
    ::set_flag( maFieldInfo.mnFlags, spnPCItemFlags[ mnTypeFlags ] );

    
    maFieldInfo.mnVisItems = static_cast< sal_uInt16 >( GetVisItemList().GetSize() );
    maFieldInfo.mnGroupItems = static_cast< sal_uInt16 >( maGroupItemList.GetSize() );
    
    maFieldInfo.mnOrigItems = static_cast< sal_uInt16 >( maOrigItemList.GetSize() );
}

void XclExpPCField::WriteSxnumgroup( XclExpStream& rStrm )
{
    if( IsNumGroupField() || IsDateGroupField() )
    {
        
        rStrm.StartRecord( EXC_ID_SXNUMGROUP, 2 );
        rStrm << maNumGroupInfo;
        rStrm.EndRecord();

        
        OSL_ENSURE( maNumGroupLimits.GetSize() == 3,
            "XclExpPCField::WriteSxnumgroup - missing numeric grouping limits" );
        maNumGroupLimits.Save( rStrm );
    }
}

void XclExpPCField::WriteSxgroupinfo( XclExpStream& rStrm )
{
    OSL_ENSURE( IsStdGroupField() != maGroupOrder.empty(),
        "XclExpPCField::WriteSxgroupinfo - missing grouping info" );
    if( IsStdGroupField() && !maGroupOrder.empty() )
    {
        rStrm.StartRecord( EXC_ID_SXGROUPINFO, 2 * maGroupOrder.size() );
        for( ScfUInt16Vec::const_iterator aIt = maGroupOrder.begin(), aEnd = maGroupOrder.end(); aIt != aEnd; ++aIt )
            rStrm << *aIt;
        rStrm.EndRecord();
    }
}

void XclExpPCField::WriteBody( XclExpStream& rStrm )
{
    rStrm << maFieldInfo;
}



XclExpPivotCache::XclExpPivotCache( const XclExpRoot& rRoot, const ScDPObject& rDPObj, sal_uInt16 nListIdx ) :
    XclExpRoot( rRoot ),
    mnListIdx( nListIdx ),
    mbValid( false )
{
    
    if( const ScSheetSourceDesc* pSrcDesc = rDPObj.GetSheetDesc() )
    {
        /*  maOrigSrcRange: Range received from the DataPilot object.
            maExpSrcRange: Range written to the DCONREF record.
            maDocSrcRange: Range used to get source data from Calc document.
                This range may be shorter than maExpSrcRange to improve export
                performance (#i22541#). */
        maOrigSrcRange = maExpSrcRange = maDocSrcRange = pSrcDesc->GetSourceRange();
        maSrcRangeName = pSrcDesc->GetRangeName();

        
        SCTAB nScTab = maExpSrcRange.aStart.Tab();
        if( (nScTab == maExpSrcRange.aEnd.Tab()) && GetTabInfo().IsExportTab( nScTab ) )
        {
            
            if( GetAddressConverter().ValidateRange( maExpSrcRange, true ) )
            {
                
                SCCOL nDocCol1, nDocCol2;
                SCROW nDocRow1, nDocRow2;
                GetDoc().GetDataStart( nScTab, nDocCol1, nDocRow1 );
                GetDoc().GetPrintArea( nScTab, nDocCol2, nDocRow2, false );
                SCCOL nSrcCol1 = maExpSrcRange.aStart.Col();
                SCROW nSrcRow1 = maExpSrcRange.aStart.Row();
                SCCOL nSrcCol2 = maExpSrcRange.aEnd.Col();
                SCROW nSrcRow2 = maExpSrcRange.aEnd.Row();

                
                if( 2 * (nDocRow2 - nDocRow1) < (nSrcRow2 - nSrcRow1) )
                    ::set_flag( maPCInfo.mnFlags, EXC_SXDB_SAVEDATA, false );

                
                if( nSrcRow1 < nDocRow1 )
                    nSrcRow1 = nDocRow1 - 1;
                if( nSrcRow2 > nDocRow2 )
                    nSrcRow2 = nDocRow2 + 1;

                maDocSrcRange.aStart.SetCol( ::std::max( nDocCol1, nSrcCol1 ) );
                maDocSrcRange.aStart.SetRow( nSrcRow1 );
                maDocSrcRange.aEnd.SetCol( ::std::min( nDocCol2, nSrcCol2 ) );
                maDocSrcRange.aEnd.SetRow( nSrcRow2 );

                GetDoc().GetName( nScTab, maTabName );
                maPCInfo.mnSrcRecs = static_cast< sal_uInt32 >( maExpSrcRange.aEnd.Row() - maExpSrcRange.aStart.Row() );
                maPCInfo.mnStrmId = nListIdx + 1;
                maPCInfo.mnSrcType = EXC_SXDB_SRC_SHEET;

                AddFields( rDPObj );

                mbValid = true;
            }
        }
    }
}

bool XclExpPivotCache::HasItemIndexList() const
{
    return ::get_flag( maPCInfo.mnFlags, EXC_SXDB_SAVEDATA );
}

sal_uInt16 XclExpPivotCache::GetFieldCount() const
{
    return static_cast< sal_uInt16 >( maFieldList.GetSize() );
}

const XclExpPCField* XclExpPivotCache::GetField( sal_uInt16 nFieldIdx ) const
{
    return maFieldList.GetRecord( nFieldIdx ).get();
}

bool XclExpPivotCache::HasAddFields() const
{
    
    return maPCInfo.mnStdFields < maPCInfo.mnTotalFields;
}

bool XclExpPivotCache::HasEqualDataSource( const ScDPObject& rDPObj ) const
{
    /*  For now, only sheet sources are supported, therefore it is enough to
        compare the ScSheetSourceDesc. Later, there should be done more complicated
        comparisons regarding the source type of rDPObj and this cache. */
    if( const ScSheetSourceDesc* pSrcDesc = rDPObj.GetSheetDesc() )
        return pSrcDesc->GetSourceRange() == maOrigSrcRange;
    return false;
}

void XclExpPivotCache::Save( XclExpStream& rStrm )
{
    OSL_ENSURE( mbValid, "XclExpPivotCache::Save - invalid pivot cache" );
    
    XclExpUInt16Record( EXC_ID_SXIDSTM, maPCInfo.mnStrmId ).Save( rStrm );
    
    XclExpUInt16Record( EXC_ID_SXVS, EXC_SXVS_SHEET ).Save( rStrm );

    if (!maSrcRangeName.isEmpty())
        
        WriteDConName(rStrm);
    else
        
        WriteDconref(rStrm);

    
    WriteCacheStream();
}

void XclExpPivotCache::SaveXml( XclExpXmlStream&
#ifdef XLSX_PIVOT_CACHE
                                                 rStrm
#endif
)
{
    OSL_ENSURE( mbValid, "XclExpPivotCache::Save - invalid pivot cache" );
#ifdef XLSX_PIVOT_CACHE /* <pivotCache> without xl/pivotCaches/ cacheStream
                           results in a broken .xlsx */
    sax_fastparser::FSHelperPtr& rWorkbook = rStrm.GetCurrentStream();
    OUString sId = OUStringBuffer()
        .appendAscii("rId")
        .append( rStrm.GetUniqueIdOUString() )
        .makeStringAndClear();
    rWorkbook->startElement( XML_pivotCache,
            XML_cacheId, OString::number( maPCInfo.mnStrmId ).getStr(),
            FSNS( XML_r, XML_id ), XclXmlUtils::ToOString( sId ).getStr(),
            FSEND );
    
    XclExpUInt16Record( EXC_ID_SXIDSTM, maPCInfo.mnStrmId ).SaveXml( rStrm );
    
    XclExpUInt16Record( EXC_ID_SXVS, EXC_SXVS_SHEET ).SaveXml( rStrm );
    
    
    
    
    rWorkbook->endElement( XML_pivotCache );
#endif /* XLSX_PIVOT_CACHE */
}



XclExpPCField* XclExpPivotCache::GetFieldAcc( sal_uInt16 nFieldIdx )
{
    return maFieldList.GetRecord( nFieldIdx ).get();
}

void XclExpPivotCache::AddFields( const ScDPObject& rDPObj )
{
    AddStdFields( rDPObj );
    maPCInfo.mnStdFields = GetFieldCount();
    AddGroupFields( rDPObj );
    AddCalcFields( rDPObj );
    maPCInfo.mnTotalFields = GetFieldCount();
};

void XclExpPivotCache::AddStdFields( const ScDPObject& rDPObj )
{
    
    const ScRange& rRange = HasItemIndexList() ? maExpSrcRange : maDocSrcRange;
    
    for( SCCOL nScCol = rRange.aStart.Col(), nEndScCol = rRange.aEnd.Col(); nScCol <= nEndScCol; ++nScCol )
    {
        ScRange aColRange( rRange );
        aColRange.aStart.SetCol( nScCol );
        aColRange.aEnd.SetCol( nScCol );
        maFieldList.AppendNewRecord( new XclExpPCField(
            GetRoot(), *this, GetFieldCount(), rDPObj, aColRange ) );
    }
}

void XclExpPivotCache::AddGroupFields( const ScDPObject& rDPObj )
{
    if( const ScDPSaveData* pSaveData = rDPObj.GetSaveData() )
    {
        if( const ScDPDimensionSaveData* pSaveDimData = pSaveData->GetExistingDimensionData() )
        {
            
            for( sal_uInt16 nFieldIdx = 0; nFieldIdx < maPCInfo.mnStdFields; ++nFieldIdx )
            {
                if( XclExpPCField* pCurrStdField = GetFieldAcc( nFieldIdx ) )
                {
                    const ScDPSaveGroupDimension* pGroupDim = pSaveDimData->GetGroupDimForBase( pCurrStdField->GetFieldName() );
                    XclExpPCField* pLastGroupField = pCurrStdField;
                    while( pGroupDim )
                    {
                        
                        XclExpPCFieldRef xNewGroupField( new XclExpPCField(
                            GetRoot(), *this, GetFieldCount(), rDPObj, *pGroupDim, *pCurrStdField ) );
                        maFieldList.AppendRecord( xNewGroupField );

                        
                        pLastGroupField->SetGroupChildField( *xNewGroupField );

                        
                        pGroupDim = pSaveDimData->GetGroupDimForBase( pGroupDim->GetGroupDimName() );
                        pLastGroupField = xNewGroupField.get();
                    }
                }
            }
        }
    }
}

void XclExpPivotCache::AddCalcFields( const ScDPObject& /*rDPObj*/ )
{
    
}

void XclExpPivotCache::WriteDconref( XclExpStream& rStrm ) const
{
    XclExpString aRef( XclExpUrlHelper::EncodeUrl( GetRoot(), EMPTY_OUSTRING, &maTabName ) );
    rStrm.StartRecord( EXC_ID_DCONREF, 7 + aRef.GetSize() );
    rStrm   << static_cast< sal_uInt16 >( maExpSrcRange.aStart.Row() )
            << static_cast< sal_uInt16 >( maExpSrcRange.aEnd.Row() )
            << static_cast< sal_uInt8 >( maExpSrcRange.aStart.Col() )
            << static_cast< sal_uInt8 >( maExpSrcRange.aEnd.Col() )
            << aRef
            << sal_uInt8( 0 );
    rStrm.EndRecord();
}

void XclExpPivotCache::WriteDConName( XclExpStream& rStrm ) const
{
    XclExpString aName(maSrcRangeName);
    rStrm.StartRecord(EXC_ID_DCONNAME, aName.GetSize() + 2);
    rStrm << aName << sal_uInt16(0);
    rStrm.EndRecord();
}

void XclExpPivotCache::WriteCacheStream()
{
    SotStorageRef xSvStrg = OpenStorage( EXC_STORAGE_PTCACHE );
    SotStorageStreamRef xSvStrm = OpenStream( xSvStrg, ScfTools::GetHexStr( maPCInfo.mnStrmId ) );
    if( xSvStrm.Is() )
    {
        XclExpStream aStrm( *xSvStrm, GetRoot() );
        
        WriteSxdb( aStrm );
        
        WriteSxdbex( aStrm );
        
        maFieldList.Save( aStrm );
        
        WriteSxindexlistList( aStrm );
        
        XclExpEmptyRecord( EXC_ID_EOF ).Save( aStrm );
    }
}

void XclExpPivotCache::WriteSxdb( XclExpStream& rStrm ) const
{
    rStrm.StartRecord( EXC_ID_SXDB, 21 );
    rStrm << maPCInfo;
    rStrm.EndRecord();
}

void XclExpPivotCache::WriteSxdbex( XclExpStream& rStrm ) const
{
    rStrm.StartRecord( EXC_ID_SXDBEX, 12 );
    rStrm   << EXC_SXDBEX_CREATION_DATE
            << sal_uInt32( 0 );             
    rStrm.EndRecord();
}

void XclExpPivotCache::WriteSxindexlistList( XclExpStream& rStrm ) const
{
    if( HasItemIndexList() )
    {
        sal_Size nRecSize = 0;
        size_t nPos, nSize = maFieldList.GetSize();
        for( nPos = 0; nPos < nSize; ++nPos )
            nRecSize += maFieldList.GetRecord( nPos )->GetIndexSize();

        for( sal_uInt32 nSrcRow = 0; nSrcRow < maPCInfo.mnSrcRecs; ++nSrcRow )
        {
            rStrm.StartRecord( EXC_ID_SXINDEXLIST, nRecSize );
            for( nPos = 0; nPos < nSize; ++nPos )
                maFieldList.GetRecord( nPos )->WriteIndex( rStrm, nSrcRow );
            rStrm.EndRecord();
        }
    }
}





namespace {



/** Returns a display string for a data field containing the field name and aggregation function. */
OUString lclGetDataFieldCaption( const OUString& rFieldName, GeneralFunction eFunc )
{
    OUString aCaption;

    sal_uInt16 nResIdx = 0;
    using namespace ::com::sun::star::sheet;
    switch( eFunc )
    {
        case GeneralFunction_SUM:       nResIdx = STR_FUN_TEXT_SUM;     break;
        case GeneralFunction_COUNT:     nResIdx = STR_FUN_TEXT_COUNT;   break;
        case GeneralFunction_AVERAGE:   nResIdx = STR_FUN_TEXT_AVG;     break;
        case GeneralFunction_MAX:       nResIdx = STR_FUN_TEXT_MAX;     break;
        case GeneralFunction_MIN:       nResIdx = STR_FUN_TEXT_MIN;     break;
        case GeneralFunction_PRODUCT:   nResIdx = STR_FUN_TEXT_PRODUCT; break;
        case GeneralFunction_COUNTNUMS: nResIdx = STR_FUN_TEXT_COUNT;   break;
        case GeneralFunction_STDEV:     nResIdx = STR_FUN_TEXT_STDDEV;  break;
        case GeneralFunction_STDEVP:    nResIdx = STR_FUN_TEXT_STDDEV;  break;
        case GeneralFunction_VAR:       nResIdx = STR_FUN_TEXT_VAR;     break;
        case GeneralFunction_VARP:      nResIdx = STR_FUN_TEXT_VAR;     break;
        default:;
    }
    if( nResIdx )
        aCaption = ScGlobal::GetRscString( nResIdx ) + " - ";
    aCaption += rFieldName;
    return( aCaption );
}



} 



XclExpPTItem::XclExpPTItem( const XclExpPCField& rCacheField, sal_uInt16 nCacheIdx ) :
    XclExpRecord( EXC_ID_SXVI, 8 ),
    mpCacheItem( rCacheField.GetItem( nCacheIdx ) )
{
    maItemInfo.mnType = EXC_SXVI_TYPE_DATA;
    maItemInfo.mnCacheIdx = nCacheIdx;
    maItemInfo.maVisName.mbUseCache = mpCacheItem != 0;
}

XclExpPTItem::XclExpPTItem( sal_uInt16 nItemType, sal_uInt16 nCacheIdx, bool bUseCache ) :
    XclExpRecord( EXC_ID_SXVI, 8 ),
    mpCacheItem( 0 )
{
    maItemInfo.mnType = nItemType;
    maItemInfo.mnCacheIdx = nCacheIdx;
    maItemInfo.maVisName.mbUseCache = bUseCache;
}

OUString XclExpPTItem::GetItemName() const
{
    return mpCacheItem ? mpCacheItem->ConvertToText() : OUString();
}

void XclExpPTItem::SetPropertiesFromMember( const ScDPSaveMember& rSaveMem )
{
    
    ::set_flag( maItemInfo.mnFlags, EXC_SXVI_HIDDEN, rSaveMem.HasIsVisible() && !rSaveMem.GetIsVisible() );
    
    ::set_flag( maItemInfo.mnFlags, EXC_SXVI_HIDEDETAIL, rSaveMem.HasShowDetails() && !rSaveMem.GetShowDetails() );

    
    const OUString* pVisName = rSaveMem.GetLayoutName();
    if (pVisName && !pVisName->equals(GetItemName()))
        maItemInfo.SetVisName(*pVisName);
}

void XclExpPTItem::WriteBody( XclExpStream& rStrm )
{
    rStrm << maItemInfo;
}



XclExpPTField::XclExpPTField( const XclExpPivotTable& rPTable, sal_uInt16 nCacheIdx ) :
    mrPTable( rPTable ),
    mpCacheField( rPTable.GetCacheField( nCacheIdx ) )
{
    maFieldInfo.mnCacheIdx = nCacheIdx;

    
    if( mpCacheField )
        for( sal_uInt16 nItemIdx = 0, nItemCount = mpCacheField->GetItemCount(); nItemIdx < nItemCount; ++nItemIdx )
            maItemList.AppendNewRecord( new XclExpPTItem( *mpCacheField, nItemIdx ) );
    maFieldInfo.mnItemCount = static_cast< sal_uInt16 >( maItemList.GetSize() );
}



OUString XclExpPTField::GetFieldName() const
{
    return mpCacheField ? mpCacheField->GetFieldName() : OUString();
}

sal_uInt16 XclExpPTField::GetFieldIndex() const
{
    
    return maFieldInfo.mnCacheIdx;
}

sal_uInt16 XclExpPTField::GetLastDataInfoIndex() const
{
    OSL_ENSURE( !maDataInfoVec.empty(), "XclExpPTField::GetLastDataInfoIndex - no data info found" );
    
    return static_cast< sal_uInt16 >( maDataInfoVec.size() - 1 );
}

sal_uInt16 XclExpPTField::GetItemIndex( const OUString& rName, sal_uInt16 nDefaultIdx ) const
{
    for( size_t nPos = 0, nSize = maItemList.GetSize(); nPos < nSize; ++nPos )
        if( maItemList.GetRecord( nPos )->GetItemName() == rName )
            return static_cast< sal_uInt16 >( nPos );
    return nDefaultIdx;
}



/**
 * Calc's subtotal names are escaped with backslashes ('\'), while Excel's
 * are not escaped at all.
 */
static OUString lcl_convertCalcSubtotalName(const OUString& rName)
{
    OUStringBuffer aBuf;
    const sal_Unicode* p = rName.getStr();
    sal_Int32 n = rName.getLength();
    bool bEscaped = false;
    for (sal_Int32 i = 0; i < n; ++i)
    {
        const sal_Unicode c = p[i];
        if (!bEscaped && c == '\\')
        {
            bEscaped = true;
            continue;
        }

        aBuf.append(c);
        bEscaped = false;
    }
    return aBuf.makeStringAndClear();
}

void XclExpPTField::SetPropertiesFromDim( const ScDPSaveDimension& rSaveDim )
{
    
    DataPilotFieldOrientation eOrient = static_cast< DataPilotFieldOrientation >( rSaveDim.GetOrientation() );
    OSL_ENSURE( eOrient != DataPilotFieldOrientation_DATA, "XclExpPTField::SetPropertiesFromDim - called for data field" );
    maFieldInfo.AddApiOrient( eOrient );

    
    ::set_flag( maFieldExtInfo.mnFlags, EXC_SXVDEX_SHOWALL, rSaveDim.HasShowEmpty() && rSaveDim.GetShowEmpty() );

    
    const OUString* pLayoutName = rSaveDim.GetLayoutName();
    if (pLayoutName && !pLayoutName->equals(GetFieldName()))
        maFieldInfo.SetVisName(*pLayoutName);

    const OUString* pSubtotalName = rSaveDim.GetSubtotalName();
    if (pSubtotalName)
    {
        OUString aSubName = lcl_convertCalcSubtotalName(*pSubtotalName);
        maFieldExtInfo.mpFieldTotalName.reset(new OUString(aSubName));
    }

    
    XclPTSubtotalVec aSubtotals;
    aSubtotals.reserve( static_cast< size_t >( rSaveDim.GetSubTotalsCount() ) );
    for( long nSubtIdx = 0, nSubtCount = rSaveDim.GetSubTotalsCount(); nSubtIdx < nSubtCount; ++nSubtIdx )
        aSubtotals.push_back( rSaveDim.GetSubTotalFunc( nSubtIdx ) );
    maFieldInfo.SetSubtotals( aSubtotals );

    
    if( const DataPilotFieldSortInfo* pSortInfo = rSaveDim.GetSortInfo() )
    {
        maFieldExtInfo.SetApiSortMode( pSortInfo->Mode );
        if( pSortInfo->Mode == ::com::sun::star::sheet::DataPilotFieldSortMode::DATA )
            maFieldExtInfo.mnSortField = mrPTable.GetDataFieldIndex( pSortInfo->Field, EXC_SXVDEX_SORT_OWN );
        ::set_flag( maFieldExtInfo.mnFlags, EXC_SXVDEX_SORT_ASC, pSortInfo->IsAscending );
    }

    
    if( const DataPilotFieldAutoShowInfo* pShowInfo = rSaveDim.GetAutoShowInfo() )
    {
        ::set_flag( maFieldExtInfo.mnFlags, EXC_SXVDEX_AUTOSHOW, pShowInfo->IsEnabled );
        maFieldExtInfo.SetApiAutoShowMode( pShowInfo->ShowItemsMode );
        maFieldExtInfo.SetApiAutoShowCount( pShowInfo->ItemCount );
        maFieldExtInfo.mnShowField = mrPTable.GetDataFieldIndex( pShowInfo->DataField, EXC_SXVDEX_SHOW_NONE );
    }

    
    if( const DataPilotFieldLayoutInfo* pLayoutInfo = rSaveDim.GetLayoutInfo() )
    {
        maFieldExtInfo.SetApiLayoutMode( pLayoutInfo->LayoutMode );
        ::set_flag( maFieldExtInfo.mnFlags, EXC_SXVDEX_LAYOUT_BLANK, pLayoutInfo->AddEmptyLines );
    }

    
    if( eOrient == DataPilotFieldOrientation_PAGE )
    {
        maPageInfo.mnField = GetFieldIndex();
        maPageInfo.mnSelItem = EXC_SXPI_ALLITEMS;
    }

    
    const ScDPSaveDimension::MemberList &rMembers = rSaveDim.GetMembers();
    for (ScDPSaveDimension::MemberList::const_iterator i=rMembers.begin(); i != rMembers.end() ; ++i)
        if( XclExpPTItem* pItem = GetItemAcc( (*i)->GetName() ) )
            pItem->SetPropertiesFromMember( **i );
}

void XclExpPTField::SetDataPropertiesFromDim( const ScDPSaveDimension& rSaveDim )
{
    maDataInfoVec.push_back( XclPTDataFieldInfo() );
    XclPTDataFieldInfo& rDataInfo = maDataInfoVec.back();
    rDataInfo.mnField = GetFieldIndex();

    
    maFieldInfo.AddApiOrient( DataPilotFieldOrientation_DATA );

    
    GeneralFunction eFunc = static_cast< GeneralFunction >( rSaveDim.GetFunction() );
    rDataInfo.SetApiAggFunc( eFunc );

    
    const OUString* pVisName = rSaveDim.GetLayoutName();
    if (pVisName)
        rDataInfo.SetVisName(*pVisName);
    else
        rDataInfo.SetVisName( lclGetDataFieldCaption( GetFieldName(), eFunc ) );

    
    if( const DataPilotFieldReference* pFieldRef = rSaveDim.GetReferenceValue() )
    {
        rDataInfo.SetApiRefType( pFieldRef->ReferenceType );
        rDataInfo.SetApiRefItemType( pFieldRef->ReferenceItemType );
        if( const XclExpPTField* pRefField = mrPTable.GetField( pFieldRef->ReferenceField ) )
        {
            rDataInfo.mnRefField = pRefField->GetFieldIndex();
            if( pFieldRef->ReferenceItemType == ::com::sun::star::sheet::DataPilotFieldReferenceItemType::NAMED )
                rDataInfo.mnRefItem = pRefField->GetItemIndex( pFieldRef->ReferenceItemName, 0 );
        }
    }
}

void XclExpPTField::AppendSubtotalItems()
{
    if( maFieldInfo.mnSubtotals & EXC_SXVD_SUBT_DEFAULT )   AppendSubtotalItem( EXC_SXVI_TYPE_DEFAULT );
    if( maFieldInfo.mnSubtotals & EXC_SXVD_SUBT_SUM )       AppendSubtotalItem( EXC_SXVI_TYPE_SUM );
    if( maFieldInfo.mnSubtotals & EXC_SXVD_SUBT_COUNT )     AppendSubtotalItem( EXC_SXVI_TYPE_COUNT );
    if( maFieldInfo.mnSubtotals & EXC_SXVD_SUBT_AVERAGE )   AppendSubtotalItem( EXC_SXVI_TYPE_AVERAGE );
    if( maFieldInfo.mnSubtotals & EXC_SXVD_SUBT_MAX )       AppendSubtotalItem( EXC_SXVI_TYPE_MAX );
    if( maFieldInfo.mnSubtotals & EXC_SXVD_SUBT_MIN )       AppendSubtotalItem( EXC_SXVI_TYPE_MIN );
    if( maFieldInfo.mnSubtotals & EXC_SXVD_SUBT_PROD )      AppendSubtotalItem( EXC_SXVI_TYPE_PROD );
    if( maFieldInfo.mnSubtotals & EXC_SXVD_SUBT_COUNTNUM )  AppendSubtotalItem( EXC_SXVI_TYPE_COUNTNUM );
    if( maFieldInfo.mnSubtotals & EXC_SXVD_SUBT_STDDEV )    AppendSubtotalItem( EXC_SXVI_TYPE_STDDEV );
    if( maFieldInfo.mnSubtotals & EXC_SXVD_SUBT_STDDEVP )   AppendSubtotalItem( EXC_SXVI_TYPE_STDDEVP );
    if( maFieldInfo.mnSubtotals & EXC_SXVD_SUBT_VAR )       AppendSubtotalItem( EXC_SXVI_TYPE_VAR );
    if( maFieldInfo.mnSubtotals & EXC_SXVD_SUBT_VARP )      AppendSubtotalItem( EXC_SXVI_TYPE_VARP );
}



void XclExpPTField::WriteSxpiEntry( XclExpStream& rStrm ) const
{
    rStrm << maPageInfo;
}

void XclExpPTField::WriteSxdi( XclExpStream& rStrm, sal_uInt16 nDataInfoIdx ) const
{
    OSL_ENSURE( nDataInfoIdx < maDataInfoVec.size(), "XclExpPTField::WriteSxdi - data field not found" );
    if( nDataInfoIdx < maDataInfoVec.size() )
    {
        rStrm.StartRecord( EXC_ID_SXDI, 12 );
        rStrm << maDataInfoVec[ nDataInfoIdx ];
        rStrm.EndRecord();
    }
}

void XclExpPTField::Save( XclExpStream& rStrm )
{
    
    WriteSxvd( rStrm );
    
    maItemList.Save( rStrm );
    
    WriteSxvdex( rStrm );
}



XclExpPTItem* XclExpPTField::GetItemAcc( const OUString& rName )
{
    XclExpPTItem* pItem = 0;
    for( size_t nPos = 0, nSize = maItemList.GetSize(); !pItem && (nPos < nSize); ++nPos )
        if( maItemList.GetRecord( nPos )->GetItemName() == rName )
            pItem = maItemList.GetRecord( nPos ).get();
    return pItem;
}

void XclExpPTField::AppendSubtotalItem( sal_uInt16 nItemType )
{
    maItemList.AppendNewRecord( new XclExpPTItem( nItemType, EXC_SXVI_DEFAULT_CACHE, true ) );
    ++maFieldInfo.mnItemCount;
}

void XclExpPTField::WriteSxvd( XclExpStream& rStrm ) const
{
    rStrm.StartRecord( EXC_ID_SXVD, 10 );
    rStrm << maFieldInfo;
    rStrm.EndRecord();
}

void XclExpPTField::WriteSxvdex( XclExpStream& rStrm ) const
{
    rStrm.StartRecord( EXC_ID_SXVDEX, 20 );
    rStrm << maFieldExtInfo;
    rStrm.EndRecord();
}



XclExpPivotTable::XclExpPivotTable( const XclExpRoot& rRoot, const ScDPObject& rDPObj, const XclExpPivotCache& rPCache, size_t nId ) :
    XclExpRoot( rRoot ),
    mrPCache( rPCache ),
    maDataOrientField( *this, EXC_SXIVD_DATA ),
    mnOutScTab( 0 ),
    mbValid( false ),
    mbFilterBtn( false ),
    mnId( nId )
{
    const ScRange& rOutScRange = rDPObj.GetOutRange();
    if( GetAddressConverter().ConvertRange( maPTInfo.maOutXclRange, rOutScRange, true ) )
    {
        

        
        mnOutScTab = rOutScRange.aStart.Tab();
        maPTInfo.maTableName = rDPObj.GetName();
        maPTInfo.mnCacheIdx = mrPCache.GetCacheIndex();

        maPTViewEx9Info.Init( rDPObj );

        if( const ScDPSaveData* pSaveData = rDPObj.GetSaveData() )
        {
            
            SetPropertiesFromDP( *pSaveData );

            

            /*  1)  Default-construct all pivot table fields for all pivot cache fields. */
            for( sal_uInt16 nFieldIdx = 0, nFieldCount = mrPCache.GetFieldCount(); nFieldIdx < nFieldCount; ++nFieldIdx )
                maFieldList.AppendNewRecord( new XclExpPTField( *this, nFieldIdx ) );

            boost::ptr_vector<ScDPSaveDimension>::const_iterator iter;
            const ScDPSaveData::DimsType& rDimList = pSaveData->GetDimensions();

            /*  2)  First process all data dimensions, they are needed for extended
                    settings of row/column/page fields (sorting/auto show). */
            for (iter = rDimList.begin(); iter != rDimList.end(); ++iter)
                if (iter->GetOrientation() == DataPilotFieldOrientation_DATA)
                    SetDataFieldPropertiesFromDim(*iter);

            /*  3)  Row/column/page/hidden fields. */
            for (iter = rDimList.begin(); iter != rDimList.end(); ++iter)
                if (iter->GetOrientation() != DataPilotFieldOrientation_DATA)
                    SetFieldPropertiesFromDim(*iter);

            

            Finalize();
            mbValid = true;
        }
    }
}

const XclExpPCField* XclExpPivotTable::GetCacheField( sal_uInt16 nCacheIdx ) const
{
    return mrPCache.GetField( nCacheIdx );
}

const XclExpPTField* XclExpPivotTable::GetField( sal_uInt16 nFieldIdx ) const
{
    return (nFieldIdx == EXC_SXIVD_DATA) ? &maDataOrientField : maFieldList.GetRecord( nFieldIdx ).get();
}

const XclExpPTField* XclExpPivotTable::GetField( const OUString& rName ) const
{
    return const_cast< XclExpPivotTable* >( this )->GetFieldAcc( rName );
}

sal_uInt16 XclExpPivotTable::GetDataFieldIndex( const OUString& rName, sal_uInt16 nDefaultIdx ) const
{
    for( XclPTDataFieldPosVec::const_iterator aIt = maDataFields.begin(), aEnd = maDataFields.end(); aIt != aEnd; ++aIt )
        if( const XclExpPTField* pField = GetField( aIt->first ) )
            if( pField->GetFieldName() == rName )
                return static_cast< sal_uInt16 >( aIt - maDataFields.begin() );
    return nDefaultIdx;
}

void XclExpPivotTable::Save( XclExpStream& rStrm )
{
    if( mbValid )
    {
        
        WriteSxview( rStrm );
        
        maFieldList.Save( rStrm );
        
        WriteSxivd( rStrm, maRowFields );
        WriteSxivd( rStrm, maColFields );
        
        WriteSxpi( rStrm );
        
        WriteSxdiList( rStrm );
        
        WriteSxli( rStrm, maPTInfo.mnDataRows, maPTInfo.mnRowFields );
        WriteSxli( rStrm, maPTInfo.mnDataCols, maPTInfo.mnColFields );
        
        WriteSxex( rStrm );
        
        WriteQsiSxTag( rStrm );
        
        WriteSxViewEx9( rStrm );
    }
}

void XclExpPivotTable::SaveXml( XclExpXmlStream& rStrm )
{
    if( !mbValid )
        return;
    sax_fastparser::FSHelperPtr aPivotTableDefinition = rStrm.CreateOutputStream(
            XclXmlUtils::GetStreamName( "xl/", "pivotTables/pivotTable", mnId + 1),
            XclXmlUtils::GetStreamName( "../", "pivotTables/pivotTable", mnId + 1),
            rStrm.GetCurrentStream()->getOutputStream(),
            "application/vnd.openxmlformats-officedocument.spreadsheetml.pivotTable+xml",
            "http:
    rStrm.PushStream( aPivotTableDefinition );

    aPivotTableDefinition->startElement( XML_pivotTableDefinition,
            XML_xmlns,                      "http:
            XML_name,                       XclXmlUtils::ToOString( maPTInfo.maTableName ).getStr(),
            XML_cacheId,                    OString::number(  maPTInfo.mnCacheIdx ).getStr(),
            XML_dataOnRows,                 XclXmlUtils::ToPsz( maPTInfo.mnDataAxis == EXC_SXVD_AXIS_COL ),
            XML_dataPosition,               OString::number(  maPTInfo.mnDataPos ).getStr(),
            XML_autoFormatId,               OString::number(  maPTInfo.mnAutoFmtIdx ).getStr(),
            
            
            
            
            
            
            XML_dataCaption,                XclXmlUtils::ToOString( maPTInfo.maDataName ).getStr(),
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            XML_enableDrill,                XclXmlUtils::ToPsz( maPTExtInfo.mnFlags & EXC_SXEX_DRILLDOWN ), 
            
            
            
            
            
            XML_rowGrandTotals,             XclXmlUtils::ToPsz( maPTInfo.mnFlags & EXC_SXVIEW_ROWGRAND ),
            XML_colGrandTotals,             XclXmlUtils::ToPsz( maPTInfo.mnFlags & EXC_SXVIEW_COLGRAND ),
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            FSEND );

    aPivotTableDefinition->singleElement( XML_location,
            XML_ref,            XclXmlUtils::ToOString( maPTInfo.maOutXclRange ).getStr(),
            XML_firstHeaderRow, OString::number(  maPTInfo.mnFirstHeadRow ).getStr(),
            XML_firstDataRow,   OString::number(  maPTInfo.maDataXclPos.mnRow ).getStr(),
            XML_firstDataCol,   OString::number(  maPTInfo.maDataXclPos.mnCol ).getStr(),
            XML_rowPageCount,   OString::number(  maPTInfo.mnDataRows ).getStr(),   
            XML_colPageCount,   OString::number(  maPTInfo.mnDataCols ).getStr(),   
            FSEND );

    

    
    if( maPTInfo.mnRowFields )
    {
        aPivotTableDefinition->startElement( XML_rowFields,
                XML_count,  OString::number(  maPTInfo.mnRowFields ).getStr(),
                FSEND );
        aPivotTableDefinition->endElement( XML_rowFields );
    }

    

    if( maPTInfo.mnColFields )
    {
        aPivotTableDefinition->startElement( XML_colFields,
                XML_count,  OString::number(  maPTInfo.mnColFields ).getStr(),
                FSEND );
        aPivotTableDefinition->endElement( XML_colFields );
    }

    

    if( maPTInfo.mnPageFields )
    {
        aPivotTableDefinition->startElement( XML_pageFields,
                XML_count,  OString::number(  maPTInfo.mnPageFields ).getStr(),
                FSEND );
        aPivotTableDefinition->endElement( XML_pageFields );
    }

    if( maPTInfo.mnDataFields )
    {
        aPivotTableDefinition->startElement( XML_dataFields,
                XML_count,  OString::number(  maPTInfo.mnDataFields ).getStr(),
                FSEND );
        aPivotTableDefinition->endElement( XML_dataFields );
    }

    
    
    

    aPivotTableDefinition->endElement( XML_pivotTableDefinition );

    rStrm.PopStream();
}



XclExpPTField* XclExpPivotTable::GetFieldAcc( const OUString& rName )
{
    XclExpPTField* pField = 0;
    for( size_t nPos = 0, nSize = maFieldList.GetSize(); !pField && (nPos < nSize); ++nPos )
        if( maFieldList.GetRecord( nPos )->GetFieldName() == rName )
            pField = maFieldList.GetRecord( nPos ).get();
    return pField;
}

XclExpPTField* XclExpPivotTable::GetFieldAcc( const ScDPSaveDimension& rSaveDim )
{
    
    if( rSaveDim.IsDataLayout() )
        return &maDataOrientField;

    
    OUString aFieldName = ScDPUtil::getSourceDimensionName(rSaveDim.GetName());
    return aFieldName.isEmpty() ? NULL : GetFieldAcc(aFieldName);
}



void XclExpPivotTable::SetPropertiesFromDP( const ScDPSaveData& rSaveData )
{
    ::set_flag( maPTInfo.mnFlags, EXC_SXVIEW_ROWGRAND, rSaveData.GetRowGrand() );
    ::set_flag( maPTInfo.mnFlags, EXC_SXVIEW_COLGRAND, rSaveData.GetColumnGrand() );
    ::set_flag( maPTExtInfo.mnFlags, EXC_SXEX_DRILLDOWN, rSaveData.GetDrillDown() );
    mbFilterBtn = rSaveData.GetFilterButton();
    const ScDPSaveDimension* pDim = rSaveData.GetExistingDataLayoutDimension();
    if (!pDim)
        return;

    const OUString* pLayoutName = pDim->GetLayoutName();
    if (pLayoutName)
        maPTInfo.maDataName = *pLayoutName;
    else
        maPTInfo.maDataName = ScGlobal::GetRscString(STR_PIVOT_DATA);
}

void XclExpPivotTable::SetFieldPropertiesFromDim( const ScDPSaveDimension& rSaveDim )
{
    if( XclExpPTField* pField = GetFieldAcc( rSaveDim ) )
    {
        
        pField->SetPropertiesFromDim( rSaveDim );

        
        DataPilotFieldOrientation eOrient = static_cast< DataPilotFieldOrientation >( rSaveDim.GetOrientation() );
        sal_uInt16 nFieldIdx = pField->GetFieldIndex();
        bool bDataLayout = nFieldIdx == EXC_SXIVD_DATA;
        bool bMultiData = maDataFields.size() > 1;

        if( !bDataLayout || bMultiData ) switch( eOrient )
        {
            case DataPilotFieldOrientation_ROW:
                maRowFields.push_back( nFieldIdx );
                if( bDataLayout )
                    maPTInfo.mnDataAxis = EXC_SXVD_AXIS_ROW;
            break;
            case DataPilotFieldOrientation_COLUMN:
                maColFields.push_back( nFieldIdx );
                if( bDataLayout )
                    maPTInfo.mnDataAxis = EXC_SXVD_AXIS_COL;
            break;
            case DataPilotFieldOrientation_PAGE:
                maPageFields.push_back( nFieldIdx );
                OSL_ENSURE( !bDataLayout, "XclExpPivotTable::SetFieldPropertiesFromDim - wrong orientation for data fields" );
            break;
            case DataPilotFieldOrientation_DATA:
                OSL_FAIL( "XclExpPivotTable::SetFieldPropertiesFromDim - called for data field" );
            break;
            default:;
        }
    }
}

void XclExpPivotTable::SetDataFieldPropertiesFromDim( const ScDPSaveDimension& rSaveDim )
{
    if( XclExpPTField* pField = GetFieldAcc( rSaveDim ) )
    {
        
        pField->SetDataPropertiesFromDim( rSaveDim );
        
        maDataFields.push_back( XclPTDataFieldPos( pField->GetFieldIndex(), pField->GetLastDataInfoIndex() ) );
    }
}

void XclExpPivotTable::Finalize()
{
    
    maPTInfo.mnFields = static_cast< sal_uInt16 >( maFieldList.GetSize() );
    maPTInfo.mnRowFields = static_cast< sal_uInt16 >( maRowFields.size() );
    maPTInfo.mnColFields = static_cast< sal_uInt16 >( maColFields.size() );
    maPTInfo.mnPageFields = static_cast< sal_uInt16 >( maPageFields.size() );
    maPTInfo.mnDataFields = static_cast< sal_uInt16 >( maDataFields.size() );

    maPTExtInfo.mnPagePerRow = maPTInfo.mnPageFields;
    maPTExtInfo.mnPagePerCol = (maPTInfo.mnPageFields > 0) ? 1 : 0;

    
    for( size_t nPos = 0, nSize = maFieldList.GetSize(); nPos < nSize; ++nPos )
        maFieldList.GetRecord( nPos )->AppendSubtotalItems();

    
    maPTInfo.mnDataPos = EXC_SXVIEW_DATALAST;
    const ScfUInt16Vec* pFieldVec = 0;
    switch( maPTInfo.mnDataAxis )
    {
        case EXC_SXVD_AXIS_ROW: pFieldVec = &maRowFields;   break;
        case EXC_SXVD_AXIS_COL: pFieldVec = &maColFields;   break;
    }

    if( pFieldVec && !pFieldVec->empty() && (pFieldVec->back() != EXC_SXIVD_DATA) )
    {
        ScfUInt16Vec::const_iterator aIt = ::std::find( pFieldVec->begin(), pFieldVec->end(), EXC_SXIVD_DATA );
        if( aIt != pFieldVec->end() )
            maPTInfo.mnDataPos = static_cast< sal_uInt16 >( aIt - pFieldVec->begin() );
    }

    
    if( maPTInfo.mnDataAxis == EXC_SXVD_AXIS_NONE )
        maPTInfo.mnDataAxis = EXC_SXVD_AXIS_ROW;

    
    sal_uInt16& rnXclCol1 = maPTInfo.maOutXclRange.maFirst.mnCol;
    sal_uInt32& rnXclRow1 = maPTInfo.maOutXclRange.maFirst.mnRow;
    sal_uInt16& rnXclCol2 = maPTInfo.maOutXclRange.maLast.mnCol;
    sal_uInt32& rnXclRow2 = maPTInfo.maOutXclRange.maLast.mnRow;
    
    rnXclRow1 = rnXclRow1 + maPTInfo.mnPageFields;
    
    if( mbFilterBtn )
        ++rnXclRow1;
    
    if( mbFilterBtn || maPTInfo.mnPageFields )
        ++rnXclRow1;

    
    sal_uInt16& rnDataXclCol = maPTInfo.maDataXclPos.mnCol;
    sal_uInt32& rnDataXclRow = maPTInfo.maDataXclPos.mnRow;
    rnDataXclCol = rnXclCol1 + maPTInfo.mnRowFields;
    rnDataXclRow = rnXclRow1 + maPTInfo.mnColFields + 1;
    if( maDataFields.empty() )
        ++rnDataXclRow;

    bool bExtraHeaderRow = (0 == maPTViewEx9Info.mnGridLayout && maPTInfo.mnColFields == 0);
    if (bExtraHeaderRow)
        
        ++rnDataXclRow;

    rnXclCol2 = ::std::max( rnXclCol2, rnDataXclCol );
    rnXclRow2 = ::std::max( rnXclRow2, rnDataXclRow );
    maPTInfo.mnDataCols = rnXclCol2 - rnDataXclCol + 1;
    maPTInfo.mnDataRows = rnXclRow2 - rnDataXclRow + 1;

    
    maPTInfo.mnFirstHeadRow = rnXclRow1;
    if (bExtraHeaderRow)
        maPTInfo.mnFirstHeadRow += 2;
}



void XclExpPivotTable::WriteSxview( XclExpStream& rStrm ) const
{
    rStrm.StartRecord( EXC_ID_SXVIEW, 46 + maPTInfo.maTableName.getLength() + maPTInfo.maDataName.getLength() );
    rStrm << maPTInfo;
    rStrm.EndRecord();
}

void XclExpPivotTable::WriteSxivd( XclExpStream& rStrm, const ScfUInt16Vec& rFields ) const
{
    if( !rFields.empty() )
    {
        rStrm.StartRecord( EXC_ID_SXIVD, rFields.size() * 2 );
        for( ScfUInt16Vec::const_iterator aIt = rFields.begin(), aEnd = rFields.end(); aIt != aEnd; ++aIt )
            rStrm << *aIt;
        rStrm.EndRecord();
    }
}

void XclExpPivotTable::WriteSxpi( XclExpStream& rStrm ) const
{
    if( !maPageFields.empty() )
    {
        rStrm.StartRecord( EXC_ID_SXPI, maPageFields.size() * 6 );
        rStrm.SetSliceSize( 6 );
        for( ScfUInt16Vec::const_iterator aIt = maPageFields.begin(), aEnd = maPageFields.end(); aIt != aEnd; ++aIt )
        {
            XclExpPTFieldRef xField = maFieldList.GetRecord( *aIt );
            if( xField )
                xField->WriteSxpiEntry( rStrm );
        }
        rStrm.EndRecord();
    }
}

void XclExpPivotTable::WriteSxdiList( XclExpStream& rStrm ) const
{
    for( XclPTDataFieldPosVec::const_iterator aIt = maDataFields.begin(), aEnd = maDataFields.end(); aIt != aEnd; ++aIt )
    {
        XclExpPTFieldRef xField = maFieldList.GetRecord( aIt->first );
        if( xField )
            xField->WriteSxdi( rStrm, aIt->second );
    }
}

void XclExpPivotTable::WriteSxli( XclExpStream& rStrm, sal_uInt16 nLineCount, sal_uInt16 nIndexCount ) const
{
    if( nLineCount > 0 )
    {
        sal_uInt16 nLineSize = 8 + 2 * nIndexCount;
        rStrm.StartRecord( EXC_ID_SXLI, nLineSize * nLineCount );

        /*  Excel expects the records to be filled completely, do not
            set a segment size... */


        for( sal_uInt16 nLine = 0; nLine < nLineCount; ++nLine )
        {
            
            rStrm   << sal_uInt16( 0 )      
                    << EXC_SXVI_TYPE_DATA
                    << nIndexCount
                    << EXC_SXLI_DEFAULTFLAGS;
            rStrm.WriteZeroBytes( 2 * nIndexCount );
        }
        rStrm.EndRecord();
    }
}

void XclExpPivotTable::WriteSxex( XclExpStream& rStrm ) const
{
    rStrm.StartRecord( EXC_ID_SXEX, 24 );
    rStrm << maPTExtInfo;
    rStrm.EndRecord();
}

void XclExpPivotTable::WriteQsiSxTag( XclExpStream& rStrm ) const
{
    rStrm.StartRecord( 0x0802, 32 );

    sal_uInt16 nRecordType = 0x0802;
    sal_uInt16 nDummyFlags = 0x0000;
    sal_uInt16 nTableType  = 1; 

    rStrm << nRecordType << nDummyFlags << nTableType;

    
    bool bEnableRefresh = true;
    bool bPCacheInvalid = false;
    bool bOlapPTReport  = false;

    sal_uInt16 nFlags = 0x0000;
    if (bEnableRefresh) nFlags |= 0x0001;
    if (bPCacheInvalid) nFlags |= 0x0002;
    if (bOlapPTReport)  nFlags |= 0x0004;
    rStrm << nFlags;

    
    
    sal_uInt32 nOptions = 0x00000000;
    bool bNoStencil = false;
    bool bHideTotal = false;
    bool bEmptyRows = false;
    bool bEmptyCols = false;
    if (bNoStencil) nOptions |= 0x00000001;
    if (bHideTotal) nOptions |= 0x00000002;
    if (bEmptyRows) nOptions |= 0x00000008;
    if (bEmptyCols) nOptions |= 0x00000010;
    rStrm << nOptions;

    enum ExcelVersion
    {
        Excel2000 = 0,
        ExcelXP   = 1,
        Excel2003 = 2,
        Excel2007 = 3
    };
    ExcelVersion eXclVer = Excel2000;
    sal_uInt8 nOffsetBytes = 16;
    rStrm << static_cast<sal_uInt8>(eXclVer)  
          << static_cast<sal_uInt8>(eXclVer)  
          << nOffsetBytes
          << static_cast<sal_uInt8>(eXclVer); 

    rStrm << XclExpString(maPTInfo.maTableName);
    rStrm << static_cast<sal_uInt16>(0x0001); 

    rStrm.EndRecord();
}

void XclExpPivotTable::WriteSxViewEx9( XclExpStream& rStrm ) const
{
    
    
    if ( 0 == maPTViewEx9Info.mnGridLayout )
    {
        rStrm.StartRecord( EXC_ID_SXVIEWEX9, 17 );
        rStrm << maPTViewEx9Info;
        rStrm.EndRecord();
    }
}



namespace {

const SCTAB EXC_PTMGR_PIVOTCACHES = SCTAB_MAX;

/** Record wrapper class to write the pivot caches or pivot tables. */
class XclExpPivotRecWrapper : public XclExpRecordBase
{
public:
    explicit            XclExpPivotRecWrapper( XclExpPivotTableManager& rPTMgr, SCTAB nScTab );
    virtual void        Save( XclExpStream& rStrm );
    virtual void        SaveXml( XclExpXmlStream& rStrm );
private:
    XclExpPivotTableManager& mrPTMgr;
    SCTAB               mnScTab;
};

XclExpPivotRecWrapper::XclExpPivotRecWrapper( XclExpPivotTableManager& rPTMgr, SCTAB nScTab ) :
    mrPTMgr( rPTMgr ),
    mnScTab( nScTab )
{
}

void XclExpPivotRecWrapper::Save( XclExpStream& rStrm )
{
    if( mnScTab == EXC_PTMGR_PIVOTCACHES )
        mrPTMgr.WritePivotCaches( rStrm );
    else
        mrPTMgr.WritePivotTables( rStrm, mnScTab );
}

void XclExpPivotRecWrapper::SaveXml( XclExpXmlStream& rStrm )
{
    if( mnScTab == EXC_PTMGR_PIVOTCACHES )
        mrPTMgr.WritePivotCachesXml( rStrm );
    else
        mrPTMgr.WritePivotTablesXml( rStrm, mnScTab );
}

} 



XclExpPivotTableManager::XclExpPivotTableManager( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot ),
    mbShareCaches( true )
{
}

void XclExpPivotTableManager::CreatePivotTables()
{
    if( ScDPCollection* pDPColl = GetDoc().GetDPCollection() )
        for( size_t nDPObj = 0, nCount = pDPColl->GetCount(); nDPObj < nCount; ++nDPObj )
            if( ScDPObject* pDPObj = (*pDPColl)[ nDPObj ] )
                if( const XclExpPivotCache* pPCache = CreatePivotCache( *pDPObj ) )
                    maPTableList.AppendNewRecord( new XclExpPivotTable( GetRoot(), *pDPObj, *pPCache, nDPObj ) );
}

XclExpRecordRef XclExpPivotTableManager::CreatePivotCachesRecord()
{
    return XclExpRecordRef( new XclExpPivotRecWrapper( *this, EXC_PTMGR_PIVOTCACHES ) );
}

XclExpRecordRef XclExpPivotTableManager::CreatePivotTablesRecord( SCTAB nScTab )
{
    return XclExpRecordRef( new XclExpPivotRecWrapper( *this, nScTab ) );
}

void XclExpPivotTableManager::WritePivotCaches( XclExpStream& rStrm )
{
    maPCacheList.Save( rStrm );
}

void XclExpPivotTableManager::WritePivotCachesXml( XclExpXmlStream&
#ifdef XLSX_PIVOT_CACHE
                                                                    rStrm
#endif
)
{
#ifdef XLSX_PIVOT_CACHE /* <pivotCache> without xl/pivotCaches/ cacheStream
                           results in a broken .xlsx */
    if( maPCacheList.IsEmpty() )
        return;
    sax_fastparser::FSHelperPtr& rWorkbook = rStrm.GetCurrentStream();
    rWorkbook->startElement( XML_pivotCaches, FSEND );
    maPCacheList.SaveXml( rStrm );
    rWorkbook->endElement( XML_pivotCaches );
#endif /* XLSX_PIVOT_CACHE */
}

void XclExpPivotTableManager::WritePivotTables( XclExpStream& rStrm, SCTAB nScTab )
{
    for( size_t nPos = 0, nSize = maPTableList.GetSize(); nPos < nSize; ++nPos )
    {
        XclExpPivotTableRef xPTable = maPTableList.GetRecord( nPos );
        if( xPTable->GetScTab() == nScTab )
            xPTable->Save( rStrm );
    }
}

void XclExpPivotTableManager::WritePivotTablesXml( XclExpXmlStream& rStrm, SCTAB nScTab )
{
    for( size_t nPos = 0, nSize = maPTableList.GetSize(); nPos < nSize; ++nPos )
    {
        XclExpPivotTableRef xPTable = maPTableList.GetRecord( nPos );
        if( xPTable->GetScTab() == nScTab )
            xPTable->SaveXml( rStrm );
    }
}



const XclExpPivotCache* XclExpPivotTableManager::CreatePivotCache( const ScDPObject& rDPObj )
{
    
    /*  #i25110# In Excel, the pivot cache contains additional fields
        (i.e. grouping info, calculated fields). If the passed DataPilot object
        or the found cache contains this data, do not share the cache with
        multiple pivot tables. */
    if( mbShareCaches )
    {
        if( const ScDPSaveData* pSaveData = rDPObj.GetSaveData() )
        {
            const ScDPDimensionSaveData* pDimSaveData = pSaveData->GetExistingDimensionData();
            
            if( !pDimSaveData || !pDimSaveData->HasGroupDimensions() )
            {
                
                for( size_t nPos = 0, nSize = maPCacheList.GetSize(); nPos < nSize; ++nPos )
                {
                    XclExpPivotCacheRef xPCache = maPCacheList.GetRecord( nPos );
                    
                    if( !xPCache->HasAddFields() && xPCache->HasEqualDataSource( rDPObj ) )
                        return xPCache.get();
                }
            }
        }
    }

    
    sal_uInt16 nNewCacheIdx = static_cast< sal_uInt16 >( maPCacheList.GetSize() );
    XclExpPivotCacheRef xNewPCache( new XclExpPivotCache( GetRoot(), rDPObj, nNewCacheIdx ) );
    if( xNewPCache->IsValid() )
    {
        maPCacheList.AppendRecord( xNewPCache );
        return xNewPCache.get();
    }

    return 0;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
