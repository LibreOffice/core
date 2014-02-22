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


#include "xipivot.hxx"

#include <com/sun/star/sheet/DataPilotFieldSortInfo.hpp>
#include <com/sun/star/sheet/DataPilotFieldAutoShowInfo.hpp>
#include <com/sun/star/sheet/DataPilotFieldLayoutInfo.hpp>
#include <com/sun/star/sheet/DataPilotFieldReference.hpp>

#include <tools/datetime.hxx>
#include <svl/zformat.hxx>
#include <svl/intitem.hxx>

#include "document.hxx"
#include "formulacell.hxx"
#include "dpsave.hxx"
#include "dpdimsave.hxx"
#include "dpobject.hxx"
#include "dpshttab.hxx"
#include "dpoutputgeometry.hxx"
#include "scitems.hxx"
#include "attrib.hxx"

#include "xltracer.hxx"
#include "xistream.hxx"
#include "xihelper.hxx"
#include "xilink.hxx"
#include "xiescher.hxx"


#include "excform.hxx"
#include "xltable.hxx"
#include "documentimport.hxx"

#include <vector>

using namespace com::sun::star;

using ::com::sun::star::sheet::DataPilotFieldOrientation;
using ::com::sun::star::sheet::DataPilotFieldOrientation_DATA;
using ::com::sun::star::sheet::DataPilotFieldSortInfo;
using ::com::sun::star::sheet::DataPilotFieldAutoShowInfo;
using ::com::sun::star::sheet::DataPilotFieldLayoutInfo;
using ::com::sun::star::sheet::DataPilotFieldReference;
using ::std::vector;





XclImpPCItem::XclImpPCItem( XclImpStream& rStrm )
{
    switch( rStrm.GetRecId() )
    {
        case EXC_ID_SXDOUBLE:   ReadSxdouble( rStrm );      break;
        case EXC_ID_SXBOOLEAN:  ReadSxboolean( rStrm );     break;
        case EXC_ID_SXERROR:    ReadSxerror( rStrm );       break;
        case EXC_ID_SXINTEGER:  ReadSxinteger( rStrm );     break;
        case EXC_ID_SXSTRING:   ReadSxstring( rStrm );      break;
        case EXC_ID_SXDATETIME: ReadSxdatetime( rStrm );    break;
        case EXC_ID_SXEMPTY:    ReadSxempty( rStrm );       break;
        default:    OSL_FAIL( "XclImpPCItem::XclImpPCItem - unknown record id" );
    }
}

namespace {

void lclSetValue( XclImpRoot& rRoot, const ScAddress& rScPos, double fValue, short nFormatType )
{
    ScDocumentImport& rDoc = rRoot.GetDocImport();
    rDoc.setNumericCell(rScPos, fValue);
    sal_uInt32 nScNumFmt = rRoot.GetFormatter().GetStandardFormat( nFormatType, rRoot.GetDocLanguage() );
    rDoc.getDoc().ApplyAttr(
        rScPos.Col(), rScPos.Row(), rScPos.Tab(), SfxUInt32Item(ATTR_VALUE_FORMAT, nScNumFmt));
}

} 

void XclImpPCItem::WriteToSource( XclImpRoot& rRoot, const ScAddress& rScPos ) const
{
    ScDocumentImport& rDoc = rRoot.GetDocImport();
    if( const OUString* pText = GetText() )
        rDoc.setStringCell(rScPos, *pText);
    else if( const double* pfValue = GetDouble() )
        rDoc.setNumericCell(rScPos, *pfValue);
    else if( const sal_Int16* pnValue = GetInteger() )
        rDoc.setNumericCell(rScPos, *pnValue);
    else if( const bool* pbValue = GetBool() )
        lclSetValue( rRoot, rScPos, *pbValue ? 1.0 : 0.0, NUMBERFORMAT_LOGICAL );
    else if( const DateTime* pDateTime = GetDateTime() )
    {
        
        double fValue = rRoot.GetDoubleFromDateTime( *pDateTime );
        double fInt = 0.0;
        double fFrac = modf( fValue, &fInt );
        short nFormatType = ((fFrac == 0.0) && (fInt != 0.0)) ? NUMBERFORMAT_DATE :
            ((fInt == 0.0) ? NUMBERFORMAT_TIME : NUMBERFORMAT_DATETIME);
        lclSetValue( rRoot, rScPos, fValue, nFormatType );
    }
    else if( const sal_uInt16* pnError = GetError() )
    {
        double fValue;
        sal_uInt8 nErrCode = static_cast< sal_uInt8 >( *pnError );
        const ScTokenArray* pScTokArr = rRoot.GetOldFmlaConverter().GetBoolErr(
            XclTools::ErrorToEnum( fValue, EXC_BOOLERR_ERROR, nErrCode ) );
        ScFormulaCell* pCell = pScTokArr ? new ScFormulaCell(&rDoc.getDoc(), rScPos, *pScTokArr) : new ScFormulaCell(&rDoc.getDoc(), rScPos);
        pCell->SetHybridDouble( fValue );
        rDoc.setFormulaCell(rScPos, pCell);
    }
}

void XclImpPCItem::ReadSxdouble( XclImpStream& rStrm )
{
    OSL_ENSURE( rStrm.GetRecSize() == 8, "XclImpPCItem::ReadSxdouble - wrong record size" );
    SetDouble( rStrm.ReadDouble() );
}

void XclImpPCItem::ReadSxboolean( XclImpStream& rStrm )
{
    OSL_ENSURE( rStrm.GetRecSize() == 2, "XclImpPCItem::ReadSxboolean - wrong record size" );
    SetBool( rStrm.ReaduInt16() != 0 );
}

void XclImpPCItem::ReadSxerror( XclImpStream& rStrm )
{
    OSL_ENSURE( rStrm.GetRecSize() == 2, "XclImpPCItem::ReadSxerror - wrong record size" );
    SetError( rStrm.ReaduInt16() );
}

void XclImpPCItem::ReadSxinteger( XclImpStream& rStrm )
{
    OSL_ENSURE( rStrm.GetRecSize() == 2, "XclImpPCItem::ReadSxinteger - wrong record size" );
    SetInteger( rStrm.ReadInt16() );
}

void XclImpPCItem::ReadSxstring( XclImpStream& rStrm )
{
    OSL_ENSURE( rStrm.GetRecSize() >= 3, "XclImpPCItem::ReadSxstring - wrong record size" );
    SetText( rStrm.ReadUniString() );
}

void XclImpPCItem::ReadSxdatetime( XclImpStream& rStrm )
{
    OSL_ENSURE( rStrm.GetRecSize() == 8, "XclImpPCItem::ReadSxdatetime - wrong record size" );
    sal_uInt16 nYear, nMonth;
    sal_uInt8 nDay, nHour, nMin, nSec;
    rStrm >> nYear >> nMonth >> nDay >> nHour >> nMin >> nSec;
    SetDateTime( DateTime( Date( nDay, nMonth, nYear ), Time( nHour, nMin, nSec ) ) );
}

void XclImpPCItem::ReadSxempty( XclImpStream& rStrm )
{
    (void)rStrm;    
    OSL_ENSURE( rStrm.GetRecSize() == 0, "XclImpPCItem::ReadSxempty - wrong record size" );
    SetEmpty();
}



XclImpPCField::XclImpPCField( const XclImpRoot& rRoot, XclImpPivotCache& rPCache, sal_uInt16 nFieldIdx ) :
    XclPCField( EXC_PCFIELD_UNKNOWN, nFieldIdx ),
    XclImpRoot( rRoot ),
    mrPCache( rPCache ),
    mnSourceScCol( -1 ),
    mbNumGroupInfoRead( false )
{
}

XclImpPCField::~XclImpPCField()
{
}



const OUString& XclImpPCField::GetFieldName( const ScfStringVec& rVisNames ) const
{
    if( IsGroupChildField() && (mnFieldIdx < rVisNames.size()) )
    {
        const OUString& rVisName = rVisNames[ mnFieldIdx ];
        if (!rVisName.isEmpty())
            return rVisName;
    }
    return maFieldInfo.maName;
}

const XclImpPCField* XclImpPCField::GetGroupBaseField() const
{
    OSL_ENSURE( IsGroupChildField(), "XclImpPCField::GetGroupBaseField - this field type does not have a base field" );
    return IsGroupChildField() ? mrPCache.GetField( maFieldInfo.mnGroupBase ) : 0;
}

const XclImpPCItem* XclImpPCField::GetItem( sal_uInt16 nItemIdx ) const
{
    return (nItemIdx < maItems.size()) ? maItems[ nItemIdx ].get() : 0;
}

const XclImpPCItem* XclImpPCField::GetLimitItem( sal_uInt16 nItemIdx ) const
{
    OSL_ENSURE( nItemIdx < 3, "XclImpPCField::GetLimitItem - invalid item index" );
    OSL_ENSURE( nItemIdx < maNumGroupItems.size(), "XclImpPCField::GetLimitItem - no item found" );
    return (nItemIdx < maNumGroupItems.size()) ? maNumGroupItems[ nItemIdx ].get() : 0;
}

void XclImpPCField::WriteFieldNameToSource( SCCOL nScCol, SCTAB nScTab )
{
    OSL_ENSURE( HasOrigItems(), "XclImpPCField::WriteFieldNameToSource - only for standard fields" );
    GetDocImport().setStringCell(ScAddress(nScCol, 0, nScTab), maFieldInfo.maName);
    mnSourceScCol = nScCol;
}

void XclImpPCField::WriteOrigItemToSource( SCROW nScRow, SCTAB nScTab, sal_uInt16 nItemIdx )
{
    if( nItemIdx < maOrigItems.size() )
        maOrigItems[ nItemIdx ]->WriteToSource( GetRoot(), ScAddress( mnSourceScCol, nScRow, nScTab ) );
}

void XclImpPCField::WriteLastOrigItemToSource( SCROW nScRow, SCTAB nScTab )
{
    if( !maOrigItems.empty() )
        maOrigItems.back()->WriteToSource( GetRoot(), ScAddress( mnSourceScCol, nScRow, nScTab ) );
}



void XclImpPCField::ReadSxfield( XclImpStream& rStrm )
{
    rStrm >> maFieldInfo;

    /*  Detect the type of this field. This is done very restrictive to detect
        any unexpected state. */
    meFieldType = EXC_PCFIELD_UNKNOWN;

    bool bItems  = ::get_flag( maFieldInfo.mnFlags, EXC_SXFIELD_HASITEMS );
    bool bPostp  = ::get_flag( maFieldInfo.mnFlags, EXC_SXFIELD_POSTPONE );
    bool bCalced = ::get_flag( maFieldInfo.mnFlags, EXC_SXFIELD_CALCED );
    bool bChild  = ::get_flag( maFieldInfo.mnFlags, EXC_SXFIELD_HASCHILD );
    bool bNum    = ::get_flag( maFieldInfo.mnFlags, EXC_SXFIELD_NUMGROUP );

    sal_uInt16 nVisC   = maFieldInfo.mnVisItems;
    sal_uInt16 nGroupC = maFieldInfo.mnGroupItems;
    sal_uInt16 nBaseC  = maFieldInfo.mnBaseItems;
    sal_uInt16 nOrigC  = maFieldInfo.mnOrigItems;
    OSL_ENSURE( nVisC > 0, "XclImpPCField::ReadSxfield - field without visible items" );

    sal_uInt16 nType = maFieldInfo.mnFlags & EXC_SXFIELD_DATA_MASK;
    bool bType =
        (nType == EXC_SXFIELD_DATA_STR) ||
        (nType == EXC_SXFIELD_DATA_INT) ||
        (nType == EXC_SXFIELD_DATA_DBL) ||
        (nType == EXC_SXFIELD_DATA_STR_INT) ||
        (nType == EXC_SXFIELD_DATA_STR_DBL) ||
        (nType == EXC_SXFIELD_DATA_DATE) ||
        (nType == EXC_SXFIELD_DATA_DATE_EMP) ||
        (nType == EXC_SXFIELD_DATA_DATE_NUM) ||
        (nType == EXC_SXFIELD_DATA_DATE_STR);
    bool bTypeNone =
        (nType == EXC_SXFIELD_DATA_NONE);
    
    OSL_ENSURE( bCalced || bType || bTypeNone, "XclImpPCField::ReadSxfield - unknown item data type" );

    if( nVisC > 0 || bPostp )
    {
        if( bItems && !bPostp )
        {
            if( !bCalced )
            {
                
                if( !bNum )
                {
                    
                    if( bType && (nGroupC == 0) && (nBaseC == 0) && (nOrigC == nVisC) )
                        meFieldType = EXC_PCFIELD_STANDARD;

                    
                    else if( bTypeNone && (nGroupC == nVisC) && (nBaseC > 0) && (nOrigC == 0) )
                        meFieldType = EXC_PCFIELD_STDGROUP;
                }
                
                else if( (nGroupC == nVisC) && (nBaseC == 0) )
                {
                    
                    if( !bChild && bType && (nOrigC > 0) )
                    {
                        switch( nType )
                        {
                            case EXC_SXFIELD_DATA_INT:
                            case EXC_SXFIELD_DATA_DBL:  meFieldType = EXC_PCFIELD_NUMGROUP;     break;
                            case EXC_SXFIELD_DATA_DATE: meFieldType = EXC_PCFIELD_DATEGROUP;    break;
                            default:    OSL_FAIL( "XclImpPCField::ReadSxfield - numeric group with wrong data type" );
                        }
                    }

                    
                    else if( bChild && (nType == EXC_SXFIELD_DATA_DATE) && (nOrigC > 0) )
                        meFieldType = EXC_PCFIELD_DATEGROUP;

                    
                    else if( bTypeNone && (nOrigC == 0) )
                        meFieldType = EXC_PCFIELD_DATECHILD;
                }
                OSL_ENSURE( meFieldType != EXC_PCFIELD_UNKNOWN, "XclImpPCField::ReadSxfield - invalid standard or grouped field" );
            }

            
            else
            {
                if( !bChild && !bNum && (nGroupC == 0) && (nBaseC == 0) && (nOrigC == 0) )
                    meFieldType = EXC_PCFIELD_CALCED;
                OSL_ENSURE( meFieldType == EXC_PCFIELD_CALCED, "XclImpPCField::ReadSxfield - invalid calculated field" );
            }
        }

        else if( !bItems && bPostp )
        {
            
            if( !bCalced && !bChild && !bNum && bType && (nGroupC == 0) && (nBaseC == 0) && (nOrigC == 0) )
                meFieldType = EXC_PCFIELD_STANDARD;
            OSL_ENSURE( meFieldType == EXC_PCFIELD_STANDARD, "XclImpPCField::ReadSxfield - invalid postponed field" );
        }
    }
}

void XclImpPCField::ReadItem( XclImpStream& rStrm )
{
    OSL_ENSURE( HasInlineItems() || HasPostponedItems(), "XclImpPCField::ReadItem - field does not expect items" );

    
    XclImpPCItemRef xItem( new XclImpPCItem( rStrm ) );

    
    if( mbNumGroupInfoRead )
    {
        
        if( maNumGroupItems.size() < 3 )
            maNumGroupItems.push_back( xItem );
        else
            maOrigItems.push_back( xItem );
    }
    else if( HasInlineItems() || HasPostponedItems() )
    {
        maItems.push_back( xItem );
        
        if( IsStandardField() )
            maOrigItems.push_back( xItem );
    }
}

void XclImpPCField::ReadSxnumgroup( XclImpStream& rStrm )
{
    OSL_ENSURE( IsNumGroupField() || IsDateGroupField(), "XclImpPCField::ReadSxnumgroup - SXNUMGROUP outside numeric grouping field" );
    OSL_ENSURE( !mbNumGroupInfoRead, "XclImpPCField::ReadSxnumgroup - multiple SXNUMGROUP records" );
    OSL_ENSURE( maItems.size() == maFieldInfo.mnGroupItems, "XclImpPCField::ReadSxnumgroup - SXNUMGROUP out of record order" );
    rStrm >> maNumGroupInfo;
    mbNumGroupInfoRead = IsNumGroupField() || IsDateGroupField();
}

void XclImpPCField::ReadSxgroupinfo( XclImpStream& rStrm )
{
    OSL_ENSURE( IsStdGroupField(), "XclImpPCField::ReadSxgroupinfo - SXGROUPINFO outside grouping field" );
    OSL_ENSURE( maGroupOrder.empty(), "XclImpPCField::ReadSxgroupinfo - multiple SXGROUPINFO records" );
    OSL_ENSURE( maItems.size() == maFieldInfo.mnGroupItems, "XclImpPCField::ReadSxgroupinfo - SXGROUPINFO out of record order" );
    OSL_ENSURE( (rStrm.GetRecLeft() / 2) == maFieldInfo.mnBaseItems, "XclImpPCField::ReadSxgroupinfo - wrong SXGROUPINFO size" );
    maGroupOrder.clear();
    size_t nSize = rStrm.GetRecLeft() / 2;
    maGroupOrder.resize( nSize, 0 );
    for( size_t nIdx = 0; nIdx < nSize; ++nIdx )
        rStrm >> maGroupOrder[ nIdx ];
}



void XclImpPCField::ConvertGroupField( ScDPSaveData& rSaveData, const ScfStringVec& rVisNames ) const
{
    if (!GetFieldName(rVisNames).isEmpty())
    {
        if( IsStdGroupField() )
            ConvertStdGroupField( rSaveData, rVisNames );
        else if( IsNumGroupField() )
            ConvertNumGroupField( rSaveData, rVisNames );
        else if( IsDateGroupField() )
            ConvertDateGroupField( rSaveData, rVisNames );
    }
}



void XclImpPCField::ConvertStdGroupField( ScDPSaveData& rSaveData, const ScfStringVec& rVisNames ) const
{
    if( const XclImpPCField* pBaseField = GetGroupBaseField() )
    {
        const OUString& rBaseFieldName = pBaseField->GetFieldName( rVisNames );
        if( !rBaseFieldName.isEmpty() )
        {
            
            ScDPSaveGroupItemVec aGroupItems;
            aGroupItems.reserve( maItems.size() );
            
            for( XclImpPCItemVec::const_iterator aIt = maItems.begin(), aEnd = maItems.end(); aIt != aEnd; ++aIt )
                aGroupItems.push_back( ScDPSaveGroupItem( (*aIt)->ConvertToText() ) );

            
            for( sal_uInt16 nItemIdx = 0, nItemCount = static_cast< sal_uInt16 >( maGroupOrder.size() ); nItemIdx < nItemCount; ++nItemIdx )
                if( maGroupOrder[ nItemIdx ] < aGroupItems.size() )
                    if( const XclImpPCItem* pBaseItem = pBaseField->GetItem( nItemIdx ) )
                        if( const XclImpPCItem* pGroupItem = GetItem( maGroupOrder[ nItemIdx ] ) )
                            if( *pBaseItem != *pGroupItem )
                                aGroupItems[ maGroupOrder[ nItemIdx ] ].AddElement( pBaseItem->ConvertToText() );

            
            ScDPSaveGroupDimension aGroupDim( rBaseFieldName, GetFieldName( rVisNames ) );
            for( ScDPSaveGroupItemVec::const_iterator aIt = aGroupItems.begin(), aEnd = aGroupItems.end(); aIt != aEnd; ++aIt )
                if( !aIt->IsEmpty() )
                    aGroupDim.AddGroupItem( *aIt );
            rSaveData.GetDimensionData()->AddGroupDimension( aGroupDim );
        }
    }
}

void XclImpPCField::ConvertNumGroupField( ScDPSaveData& rSaveData, const ScfStringVec& rVisNames ) const
{
    ScDPNumGroupInfo aNumInfo( GetScNumGroupInfo() );
    ScDPSaveNumGroupDimension aNumGroupDim( GetFieldName( rVisNames ), aNumInfo );
    rSaveData.GetDimensionData()->AddNumGroupDimension( aNumGroupDim );
}

void XclImpPCField::ConvertDateGroupField( ScDPSaveData& rSaveData, const ScfStringVec& rVisNames ) const
{
    ScDPNumGroupInfo aDateInfo( GetScDateGroupInfo() );
    sal_Int32 nScDateType = maNumGroupInfo.GetScDateType();

    switch( meFieldType )
    {
        case EXC_PCFIELD_DATEGROUP:
        {
            if( aDateInfo.mbDateValues )
            {
                
                ScDPSaveNumGroupDimension aNumGroupDim( GetFieldName( rVisNames ), aDateInfo );
                rSaveData.GetDimensionData()->AddNumGroupDimension( aNumGroupDim );
            }
            else
            {
                ScDPSaveNumGroupDimension aNumGroupDim( GetFieldName( rVisNames ), ScDPNumGroupInfo() );
                aNumGroupDim.SetDateInfo( aDateInfo, nScDateType );
                rSaveData.GetDimensionData()->AddNumGroupDimension( aNumGroupDim );
            }
        }
        break;

        case EXC_PCFIELD_DATECHILD:
        {
            if( const XclImpPCField* pBaseField = GetGroupBaseField() )
            {
                const OUString& rBaseFieldName = pBaseField->GetFieldName( rVisNames );
                if( !rBaseFieldName.isEmpty() )
                {
                    ScDPSaveGroupDimension aGroupDim( rBaseFieldName, GetFieldName( rVisNames ) );
                    aGroupDim.SetDateInfo( aDateInfo, nScDateType );
                    rSaveData.GetDimensionData()->AddGroupDimension( aGroupDim );
                }
            }
        }
        break;

        default:
            OSL_FAIL( "XclImpPCField::ConvertDateGroupField - unknown date field type" );
    }
}

ScDPNumGroupInfo XclImpPCField::GetScNumGroupInfo() const
{
    ScDPNumGroupInfo aNumInfo;
    aNumInfo.mbEnable = true;
    aNumInfo.mbDateValues = false;
    aNumInfo.mbAutoStart = true;
    aNumInfo.mbAutoEnd = true;

    if( const double* pfMinValue = GetNumGroupLimit( EXC_SXFIELD_INDEX_MIN ) )
    {
        aNumInfo.mfStart = *pfMinValue;
        aNumInfo.mbAutoStart = ::get_flag( maNumGroupInfo.mnFlags, EXC_SXNUMGROUP_AUTOMIN );
    }
    if( const double* pfMaxValue = GetNumGroupLimit( EXC_SXFIELD_INDEX_MAX ) )
    {
        aNumInfo.mfEnd = *pfMaxValue;
        aNumInfo.mbAutoEnd = ::get_flag( maNumGroupInfo.mnFlags, EXC_SXNUMGROUP_AUTOMAX );
    }
    if( const double* pfStepValue = GetNumGroupLimit( EXC_SXFIELD_INDEX_STEP ) )
        aNumInfo.mfStep = *pfStepValue;

    return aNumInfo;
}

ScDPNumGroupInfo XclImpPCField::GetScDateGroupInfo() const
{
    ScDPNumGroupInfo aDateInfo;
    aDateInfo.mbEnable = true;
    aDateInfo.mbDateValues = false;
    aDateInfo.mbAutoStart = true;
    aDateInfo.mbAutoEnd = true;

    if( const DateTime* pMinDate = GetDateGroupLimit( EXC_SXFIELD_INDEX_MIN ) )
    {
        aDateInfo.mfStart = GetDoubleFromDateTime( *pMinDate );
        aDateInfo.mbAutoStart = ::get_flag( maNumGroupInfo.mnFlags, EXC_SXNUMGROUP_AUTOMIN );
    }
    if( const DateTime* pMaxDate = GetDateGroupLimit( EXC_SXFIELD_INDEX_MAX ) )
    {
        aDateInfo.mfEnd = GetDoubleFromDateTime( *pMaxDate );
        aDateInfo.mbAutoEnd = ::get_flag( maNumGroupInfo.mnFlags, EXC_SXNUMGROUP_AUTOMAX );
    }
    
    if( const sal_Int16* pnStepValue = GetDateGroupStep() )
    {
        aDateInfo.mfStep = *pnStepValue;
        aDateInfo.mbDateValues = true;
    }

    return aDateInfo;
}

const double* XclImpPCField::GetNumGroupLimit( sal_uInt16 nLimitIdx ) const
{
    OSL_ENSURE( IsNumGroupField(), "XclImpPCField::GetNumGroupLimit - only for numeric grouping fields" );
    if( const XclImpPCItem* pItem = GetLimitItem( nLimitIdx ) )
    {
        OSL_ENSURE( pItem->GetDouble(), "XclImpPCField::GetNumGroupLimit - SXDOUBLE item expected" );
        return pItem->GetDouble();
    }
    return 0;
}

const DateTime* XclImpPCField::GetDateGroupLimit( sal_uInt16 nLimitIdx ) const
{
    OSL_ENSURE( IsDateGroupField(), "XclImpPCField::GetDateGroupLimit - only for date grouping fields" );
    if( const XclImpPCItem* pItem = GetLimitItem( nLimitIdx ) )
    {
        OSL_ENSURE( pItem->GetDateTime(), "XclImpPCField::GetDateGroupLimit - SXDATETIME item expected" );
        return pItem->GetDateTime();
    }
    return 0;
}

const sal_Int16* XclImpPCField::GetDateGroupStep() const
{
    
    if( !IsGroupBaseField() && !IsGroupChildField() )
    {
        
        if( maNumGroupInfo.GetXclDataType() == EXC_SXNUMGROUP_TYPE_DAY )
        {
            if( const XclImpPCItem* pItem = GetLimitItem( EXC_SXFIELD_INDEX_STEP ) )
            {
                OSL_ENSURE( pItem->GetInteger(), "XclImpPCField::GetDateGroupStep - SXINTEGER item expected" );
                if( const sal_Int16* pnStep = pItem->GetInteger() )
                {
                    OSL_ENSURE( *pnStep > 0, "XclImpPCField::GetDateGroupStep - invalid step count" );
                    
                    return (*pnStep > 1) ? pnStep : 0;
                }
            }
        }
    }
    return 0;
}



XclImpPivotCache::XclImpPivotCache( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot ),
    maSrcRange( ScAddress::INITIALIZE_INVALID ),
    mnStrmId( 0 ),
    mnSrcType( EXC_SXVS_UNKNOWN ),
    mbSelfRef( false )
{
}

XclImpPivotCache::~XclImpPivotCache()
{
}



sal_uInt16 XclImpPivotCache::GetFieldCount() const
{
    return static_cast< sal_uInt16 >( maFields.size() );
}

const XclImpPCField* XclImpPivotCache::GetField( sal_uInt16 nFieldIdx ) const
{
    return (nFieldIdx < maFields.size()) ? maFields[ nFieldIdx ].get() : 0;
}



void XclImpPivotCache::ReadSxidstm( XclImpStream& rStrm )
{
    rStrm >> mnStrmId;
}

void XclImpPivotCache::ReadSxvs( XclImpStream& rStrm )
{
    rStrm >> mnSrcType;
    GetTracer().TracePivotDataSource( mnSrcType != EXC_SXVS_SHEET );
}

void XclImpPivotCache::ReadDconref( XclImpStream& rStrm )
{
    /*  Read DCONREF only once (by checking maTabName), there may be other
        DCONREF records in another context. Read reference only if a leading
        SXVS record is present (by checking mnSrcType). */
    if( !maTabName.isEmpty() || (mnSrcType != EXC_SXVS_SHEET) )
        return;

    XclRange aXclRange( ScAddress::UNINITIALIZED );
    aXclRange.Read( rStrm, false );
    OUString aEncUrl = rStrm.ReadUniString();

    XclImpUrlHelper::DecodeUrl( maUrl, maTabName, mbSelfRef, GetRoot(), aEncUrl );

    /*  Do not convert maTabName to Calc sheet name -> original name is used to
        find the sheet in the document. Sheet index of source range will be
        found later in XclImpPivotCache::ReadPivotCacheStream(), because sheet
        may not exist yet. */
    if( mbSelfRef )
        GetAddressConverter().ConvertRange( maSrcRange, aXclRange, 0, 0, true );
}

void XclImpPivotCache::ReadDConName( XclImpStream& rStrm )
{
    maSrcRangeName = rStrm.ReadUniString();

    
    
    
    sal_uInt16 nFlag;
    rStrm >> nFlag;
    mbSelfRef = (nFlag == 0);

    if (!mbSelfRef)
        
        maSrcRangeName = OUString();
}

void XclImpPivotCache::ReadPivotCacheStream( XclImpStream& rStrm )
{
    if( (mnSrcType != EXC_SXVS_SHEET) && (mnSrcType != EXC_SXVS_EXTERN) )
        return;

    ScDocument& rDoc = GetDoc();
    SCCOL nFieldScCol = 0;              
    SCROW nItemScRow = 0;               
    SCTAB nScTab = 0;                   
    bool bGenerateSource = false;       

    if( mbSelfRef )
    {
        if (maSrcRangeName.isEmpty())
        {
            
            nScTab = GetTabInfo().GetScTabFromXclName( maTabName );
            if( rDoc.HasTable( nScTab ) )
            {
                
                maSrcRange.aStart.SetTab( nScTab );
                maSrcRange.aEnd.SetTab( nScTab );
            }
            else
            {
                
                bGenerateSource = true;
            }
        }
    }
    else
    {
        
        bGenerateSource = true;
    }

    
    if( bGenerateSource )
    {
        if( rDoc.GetTableCount() >= MAXTABCOUNT )
            
            return;

        nScTab = rDoc.GetTableCount();
        rDoc.MakeTable( nScTab );
        OUStringBuffer aDummyName("DPCache");
        if( maTabName.getLength() > 0 )
            aDummyName.append( '_' ).append( maTabName );
        OUString aName = aDummyName.makeStringAndClear();
        rDoc.CreateValidTabName( aName );
        rDoc.RenameTab( nScTab, aName );
        
        maSrcRange.aStart.SetTab( nScTab );
        maSrcRange.aEnd.SetTab( nScTab );
    }

    
    SotStorageRef xSvStrg = OpenStorage( EXC_STORAGE_PTCACHE );
    SotStorageStreamRef xSvStrm = OpenStream( xSvStrg, ScfTools::GetHexStr( mnStrmId ) );
    if( !xSvStrm.Is() )
        return;

    
    XclImpStream aPCStrm( *xSvStrm, GetRoot() );
    aPCStrm.CopyDecrypterFrom( rStrm );     

    XclImpPCFieldRef xCurrField;    
    XclImpPCFieldVec aOrigFields;   
    XclImpPCFieldVec aPostpFields;  
    size_t nPostpIdx = 0;           
    bool bLoop = true;              

    while( bLoop && aPCStrm.StartNextRecord() )
    {
        switch( aPCStrm.GetRecId() )
        {
            case EXC_ID_EOF:
                bLoop = false;
            break;

            case EXC_ID_SXDB:
                aPCStrm >> maPCInfo;
            break;

            case EXC_ID_SXFIELD:
            {
                xCurrField.reset();
                sal_uInt16 nNewFieldIdx = GetFieldCount();
                if( nNewFieldIdx < EXC_PC_MAXFIELDCOUNT )
                {
                    xCurrField.reset( new XclImpPCField( GetRoot(), *this, nNewFieldIdx ) );
                    maFields.push_back( xCurrField );
                    xCurrField->ReadSxfield( aPCStrm );
                    if( xCurrField->HasOrigItems() )
                    {
                        if( xCurrField->HasPostponedItems() )
                            aPostpFields.push_back( xCurrField );
                        else
                            aOrigFields.push_back( xCurrField );
                        
                        if( bGenerateSource && (nFieldScCol <= MAXCOL) )
                            xCurrField->WriteFieldNameToSource( nFieldScCol++, nScTab );
                    }
                    
                    if( !xCurrField->HasInlineItems() )
                        xCurrField.reset();
                }
            }
            break;

            case EXC_ID_SXINDEXLIST:
                
                if( bGenerateSource && (nItemScRow <= MAXROW) && (++nItemScRow <= MAXROW) )
                {
                    for( XclImpPCFieldVec::const_iterator aIt = aOrigFields.begin(), aEnd = aOrigFields.end(); aIt != aEnd; ++aIt )
                    {
                        sal_uInt16 nItemIdx = (*aIt)->Has16BitIndexes() ? aPCStrm.ReaduInt16() : aPCStrm.ReaduInt8();
                        (*aIt)->WriteOrigItemToSource( nItemScRow, nScTab, nItemIdx );
                    }
                }
                xCurrField.reset();
            break;

            case EXC_ID_SXDOUBLE:
            case EXC_ID_SXBOOLEAN:
            case EXC_ID_SXERROR:
            case EXC_ID_SXINTEGER:
            case EXC_ID_SXSTRING:
            case EXC_ID_SXDATETIME:
            case EXC_ID_SXEMPTY:
                if( xCurrField )                   
                {
                    xCurrField->ReadItem( aPCStrm );
                }
                else if( !aPostpFields.empty() )        
                {
                    
                    aPostpFields[ nPostpIdx ]->ReadItem( aPCStrm );
                    
                    if( bGenerateSource && (nItemScRow <= MAXROW) )
                    {
                        
                        if( aOrigFields.empty() && (nPostpIdx == 0) )
                            ++nItemScRow;
                        if( nItemScRow <= MAXROW )
                            aPostpFields[ nPostpIdx ]->WriteLastOrigItemToSource( nItemScRow, nScTab );
                    }
                    
                    ++nPostpIdx;
                    if( nPostpIdx >= aPostpFields.size() )
                        nPostpIdx = 0;
                }
            break;

            case EXC_ID_SXNUMGROUP:
                if( xCurrField )
                    xCurrField->ReadSxnumgroup( aPCStrm );
            break;

            case EXC_ID_SXGROUPINFO:
                if( xCurrField )
                    xCurrField->ReadSxgroupinfo( aPCStrm );
            break;

            
            case EXC_ID_SXRULE:
            case EXC_ID_SXFILT:
            case EXC_ID_00F5:
            case EXC_ID_SXNAME:
            case EXC_ID_SXPAIR:
            case EXC_ID_SXFMLA:
            case EXC_ID_SXFORMULA:
            case EXC_ID_SXDBEX:
            case EXC_ID_SXFDBTYPE:
            break;

            default:
                OSL_TRACE( "XclImpPivotCache::ReadPivotCacheStream - unknown record 0x%04hX", aPCStrm.GetRecId() );
        }
    }

    OSL_ENSURE( maPCInfo.mnTotalFields == maFields.size(),
        "XclImpPivotCache::ReadPivotCacheStream - field count mismatch" );

    if (HasCacheRecords())
    {
        SCROW nNewEnd = maSrcRange.aStart.Row() + maPCInfo.mnSrcRecs;
        maSrcRange.aEnd.SetRow(nNewEnd);
    }

    
    if( bGenerateSource && (nFieldScCol > 0) )
    {
        maSrcRange.aStart.SetCol( 0 );
        maSrcRange.aStart.SetRow( 0 );
        
        maSrcRange.aEnd.SetCol( nFieldScCol - 1 );
        
        maSrcRange.aEnd.SetRow( nItemScRow );
    }
}

bool XclImpPivotCache::HasCacheRecords() const
{
    return static_cast<bool>(maPCInfo.mnFlags & EXC_SXDB_SAVEDATA);
}

bool XclImpPivotCache::IsRefreshOnLoad() const
{
    return static_cast<bool>(maPCInfo.mnFlags & EXC_SXDB_REFRESH_LOAD);
}

bool XclImpPivotCache::IsValid() const
{
    if (!maSrcRangeName.isEmpty())
        return true;

    return maSrcRange.IsValid();
}





XclImpPTItem::XclImpPTItem( const XclImpPCField* pCacheField ) :
    mpCacheField( pCacheField )
{
}

const OUString* XclImpPTItem::GetItemName() const
{
    if( mpCacheField )
        if( const XclImpPCItem* pCacheItem = mpCacheField->GetItem( maItemInfo.mnCacheIdx ) )
            
            return pCacheItem->IsEmpty() ? NULL : pCacheItem->GetText();
    return 0;
}

void XclImpPTItem::ReadSxvi( XclImpStream& rStrm )
{
    rStrm >> maItemInfo;
}

void XclImpPTItem::ConvertItem( ScDPSaveDimension& rSaveDim ) const
{
    if (const OUString* pItemName = GetItemName())
    {
        ScDPSaveMember& rMember = *rSaveDim.GetMemberByName( *pItemName );
        rMember.SetIsVisible( !::get_flag( maItemInfo.mnFlags, EXC_SXVI_HIDDEN ) );
        rMember.SetShowDetails( !::get_flag( maItemInfo.mnFlags, EXC_SXVI_HIDEDETAIL ) );
        if (maItemInfo.HasVisName())
            rMember.SetLayoutName(*maItemInfo.GetVisName());
    }
}



XclImpPTField::XclImpPTField( const XclImpPivotTable& rPTable, sal_uInt16 nCacheIdx ) :
    mrPTable( rPTable )
{
    maFieldInfo.mnCacheIdx = nCacheIdx;
}



const XclImpPCField* XclImpPTField::GetCacheField() const
{
    XclImpPivotCacheRef xPCache = mrPTable.GetPivotCache();
    return xPCache ? xPCache->GetField( maFieldInfo.mnCacheIdx ) : 0;
}

OUString XclImpPTField::GetFieldName() const
{
    const XclImpPCField* pField = GetCacheField();
    return pField ? pField->GetFieldName( mrPTable.GetVisFieldNames() ) : OUString();
}

OUString XclImpPTField::GetVisFieldName() const
{
    const OUString* pVisName = maFieldInfo.GetVisName();
    return pVisName ? *pVisName : OUString();
}

const XclImpPTItem* XclImpPTField::GetItem( sal_uInt16 nItemIdx ) const
{
    return (nItemIdx < maItems.size()) ? maItems[ nItemIdx ].get() : 0;
}

const OUString* XclImpPTField::GetItemName( sal_uInt16 nItemIdx ) const
{
    const XclImpPTItem* pItem = GetItem( nItemIdx );
    return pItem ? pItem->GetItemName() : 0;
}



void XclImpPTField::ReadSxvd( XclImpStream& rStrm )
{
    rStrm >> maFieldInfo;
}

void XclImpPTField::ReadSxvdex( XclImpStream& rStrm )
{
    rStrm >> maFieldExtInfo;
}

void XclImpPTField::ReadSxvi( XclImpStream& rStrm )
{
    XclImpPTItemRef xItem( new XclImpPTItem( GetCacheField() ) );
    maItems.push_back( xItem );
    xItem->ReadSxvi( rStrm );
}



void XclImpPTField::ConvertRowColField( ScDPSaveData& rSaveData ) const
{
    OSL_ENSURE( maFieldInfo.mnAxes & EXC_SXVD_AXIS_ROWCOL, "XclImpPTField::ConvertRowColField - no row/column field" );
    
    if( maFieldInfo.mnCacheIdx == EXC_SXIVD_DATA )
        rSaveData.GetDataLayoutDimension()->SetOrientation( static_cast< sal_uInt16 >( maFieldInfo.GetApiOrient( EXC_SXVD_AXIS_ROWCOL ) ) );
    else
        ConvertRCPField( rSaveData );
}



void XclImpPTField::SetPageFieldInfo( const XclPTPageFieldInfo& rPageInfo )
{
    maPageInfo = rPageInfo;
}

void XclImpPTField::ConvertPageField( ScDPSaveData& rSaveData ) const
{
    OSL_ENSURE( maFieldInfo.mnAxes & EXC_SXVD_AXIS_PAGE, "XclImpPTField::ConvertPageField - no page field" );
    if( ScDPSaveDimension* pSaveDim = ConvertRCPField( rSaveData ) )
    {
        const OUString* pName = GetItemName( maPageInfo.mnSelItem );
        if (pName)
            pSaveDim->SetCurrentPage(pName);
    }
}



void XclImpPTField::ConvertHiddenField( ScDPSaveData& rSaveData ) const
{
    OSL_ENSURE( (maFieldInfo.mnAxes & EXC_SXVD_AXIS_ROWCOLPAGE) == 0, "XclImpPTField::ConvertHiddenField - field not hidden" );
    ConvertRCPField( rSaveData );
}



bool XclImpPTField::HasDataFieldInfo() const
{
    return !maDataInfoList.empty();
}

void XclImpPTField::AddDataFieldInfo( const XclPTDataFieldInfo& rDataInfo )
{
    OSL_ENSURE( maFieldInfo.mnAxes & EXC_SXVD_AXIS_DATA, "XclImpPTField::AddDataFieldInfo - no data field" );
    maDataInfoList.push_back( rDataInfo );
}

void XclImpPTField::ConvertDataField( ScDPSaveData& rSaveData ) const
{
    OSL_ENSURE( maFieldInfo.mnAxes & EXC_SXVD_AXIS_DATA, "XclImpPTField::ConvertDataField - no data field" );
    OSL_ENSURE( !maDataInfoList.empty(), "XclImpPTField::ConvertDataField - no data field info" );
    if (maDataInfoList.empty())
        return;

    OUString aFieldName = GetFieldName();
    if (aFieldName.isEmpty())
        return;

    XclPTDataFieldInfoList::const_iterator aIt = maDataInfoList.begin(), aEnd = maDataInfoList.end();

    ScDPSaveDimension& rSaveDim = *rSaveData.GetNewDimensionByName(aFieldName);
    ConvertDataField( rSaveDim, *aIt );

    
    for( ++aIt; aIt != aEnd; ++aIt )
    {
        ScDPSaveDimension& rDupDim = rSaveData.DuplicateDimension( rSaveDim );
        ConvertDataFieldInfo( rDupDim, *aIt );
    }
}



/**
 * Convert Excel-encoded subtotal name to a Calc-encoded one.
 */
static OUString lcl_convertExcelSubtotalName(const OUString& rName)
{
    OUStringBuffer aBuf;
    const sal_Unicode* p = rName.getStr();
    sal_Int32 n = rName.getLength();
    for (sal_Int32 i = 0; i < n; ++i)
    {
        const sal_Unicode c = p[i];
        if (c == '\\')
        {
            aBuf.append(c);
            aBuf.append(c);
        }
        else
            aBuf.append(c);
    }
    return aBuf.makeStringAndClear();
}

ScDPSaveDimension* XclImpPTField::ConvertRCPField( ScDPSaveData& rSaveData ) const
{
    const OUString& rFieldName = GetFieldName();
    if( rFieldName.isEmpty() )
        return 0;

    const XclImpPCField* pCacheField = GetCacheField();
    if( !pCacheField || !pCacheField->IsSupportedField() )
        return 0;

    ScDPSaveDimension* pTest = rSaveData.GetNewDimensionByName(rFieldName);
    if (!pTest)
        return NULL;

    ScDPSaveDimension& rSaveDim = *pTest;

    
    rSaveDim.SetOrientation( static_cast< sal_uInt16 >( maFieldInfo.GetApiOrient( EXC_SXVD_AXIS_ROWCOLPAGE ) ) );

    
    ConvertFieldInfo( rSaveDim );

    
    if (const OUString* pVisName = maFieldInfo.GetVisName())
        if (!pVisName->isEmpty())
            rSaveDim.SetLayoutName( *pVisName );

    
    XclPTSubtotalVec aSubtotalVec;
    maFieldInfo.GetSubtotals( aSubtotalVec );
    if( !aSubtotalVec.empty() )
        rSaveDim.SetSubTotals( static_cast< long >( aSubtotalVec.size() ), &aSubtotalVec[ 0 ] );

    
    DataPilotFieldSortInfo aSortInfo;
    aSortInfo.Field = mrPTable.GetDataFieldName( maFieldExtInfo.mnSortField );
    aSortInfo.IsAscending = ::get_flag( maFieldExtInfo.mnFlags, EXC_SXVDEX_SORT_ASC );
    aSortInfo.Mode = maFieldExtInfo.GetApiSortMode();
    rSaveDim.SetSortInfo( &aSortInfo );

    
    DataPilotFieldAutoShowInfo aShowInfo;
    aShowInfo.IsEnabled = ::get_flag( maFieldExtInfo.mnFlags, EXC_SXVDEX_AUTOSHOW );
    aShowInfo.ShowItemsMode = maFieldExtInfo.GetApiAutoShowMode();
    aShowInfo.ItemCount = maFieldExtInfo.GetApiAutoShowCount();
    aShowInfo.DataField = mrPTable.GetDataFieldName( maFieldExtInfo.mnShowField );
    rSaveDim.SetAutoShowInfo( &aShowInfo );

    
    DataPilotFieldLayoutInfo aLayoutInfo;
    aLayoutInfo.LayoutMode = maFieldExtInfo.GetApiLayoutMode();
    aLayoutInfo.AddEmptyLines = ::get_flag( maFieldExtInfo.mnFlags, EXC_SXVDEX_LAYOUT_BLANK );
    rSaveDim.SetLayoutInfo( &aLayoutInfo );

    
    pCacheField->ConvertGroupField( rSaveData, mrPTable.GetVisFieldNames() );

    
    if (maFieldExtInfo.mpFieldTotalName.get())
    {
        OUString aSubName = lcl_convertExcelSubtotalName(*maFieldExtInfo.mpFieldTotalName);
        rSaveDim.SetSubtotalName(aSubName);
    }

    return &rSaveDim;
}

void XclImpPTField::ConvertFieldInfo( ScDPSaveDimension& rSaveDim ) const
{
    rSaveDim.SetShowEmpty( ::get_flag( maFieldExtInfo.mnFlags, EXC_SXVDEX_SHOWALL ) );
    ConvertItems( rSaveDim );
}

void XclImpPTField::ConvertDataField( ScDPSaveDimension& rSaveDim, const XclPTDataFieldInfo& rDataInfo ) const
{
    
    rSaveDim.SetOrientation( DataPilotFieldOrientation_DATA );
    
    ConvertFieldInfo( rSaveDim );
    
    ConvertDataFieldInfo( rSaveDim, rDataInfo );
}

void XclImpPTField::ConvertDataFieldInfo( ScDPSaveDimension& rSaveDim, const XclPTDataFieldInfo& rDataInfo ) const
{
    
    const OUString* pVisName = rDataInfo.GetVisName();
    if (pVisName && !pVisName->isEmpty())
        rSaveDim.SetLayoutName(*pVisName);

    
    rSaveDim.SetFunction( static_cast< sal_uInt16 >( rDataInfo.GetApiAggFunc() ) );

    
    sal_Int32 nRefType = rDataInfo.GetApiRefType();
    DataPilotFieldReference aFieldRef;
    aFieldRef.ReferenceType = nRefType;
    const XclImpPTField* pRefField = mrPTable.GetField(rDataInfo.mnRefField);
    if (pRefField)
    {
        aFieldRef.ReferenceField = pRefField->GetFieldName();
        aFieldRef.ReferenceItemType = rDataInfo.GetApiRefItemType();
        if (aFieldRef.ReferenceItemType == sheet::DataPilotFieldReferenceItemType::NAMED)
        {
            const OUString* pRefItemName = pRefField->GetItemName(rDataInfo.mnRefItem);
            if (pRefItemName)
                aFieldRef.ReferenceItemName = *pRefItemName;
        }
    }

    rSaveDim.SetReferenceValue(&aFieldRef);
}

void XclImpPTField::ConvertItems( ScDPSaveDimension& rSaveDim ) const
{
    for( XclImpPTItemVec::const_iterator aIt = maItems.begin(), aEnd = maItems.end(); aIt != aEnd; ++aIt )
        (*aIt)->ConvertItem( rSaveDim );
}



XclImpPivotTable::XclImpPivotTable( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot ),
    maDataOrientField( *this, EXC_SXIVD_DATA ),
    mpDPObj(NULL)
{
}

XclImpPivotTable::~XclImpPivotTable()
{
}



sal_uInt16 XclImpPivotTable::GetFieldCount() const
{
    return static_cast< sal_uInt16 >( maFields.size() );
}

const XclImpPTField* XclImpPivotTable::GetField( sal_uInt16 nFieldIdx ) const
{
    return (nFieldIdx == EXC_SXIVD_DATA) ? &maDataOrientField :
        ((nFieldIdx < maFields.size()) ? maFields[ nFieldIdx ].get() : 0);
}

XclImpPTField* XclImpPivotTable::GetFieldAcc( sal_uInt16 nFieldIdx )
{
    
    return (nFieldIdx < maFields.size()) ? maFields[ nFieldIdx ].get() : 0;
}

const XclImpPTField* XclImpPivotTable::GetDataField( sal_uInt16 nDataFieldIdx ) const
{
    if( nDataFieldIdx < maOrigDataFields.size() )
        return GetField( maOrigDataFields[ nDataFieldIdx ] );
    return 0;
}

OUString XclImpPivotTable::GetDataFieldName( sal_uInt16 nDataFieldIdx ) const
{
    if( const XclImpPTField* pField = GetDataField( nDataFieldIdx ) )
        return pField->GetFieldName();
    return OUString();
}



void XclImpPivotTable::ReadSxview( XclImpStream& rStrm )
{
    rStrm >> maPTInfo;

    GetAddressConverter().ConvertRange(
        maOutScRange, maPTInfo.maOutXclRange, GetCurrScTab(), GetCurrScTab(), true );

    mxPCache = GetPivotTableManager().GetPivotCache( maPTInfo.mnCacheIdx );
    mxCurrField.reset();
}

void XclImpPivotTable::ReadSxvd( XclImpStream& rStrm )
{
    sal_uInt16 nFieldCount = GetFieldCount();
    if( nFieldCount < EXC_PT_MAXFIELDCOUNT )
    {
        
        mxCurrField.reset( new XclImpPTField( *this, nFieldCount ) );
        maFields.push_back( mxCurrField );
        mxCurrField->ReadSxvd( rStrm );
        
        maVisFieldNames.push_back( mxCurrField->GetVisFieldName() );
        OSL_ENSURE( maFields.size() == maVisFieldNames.size(),
            "XclImpPivotTable::ReadSxvd - wrong size of visible name array" );
    }
    else
        mxCurrField.reset();
}

void XclImpPivotTable::ReadSxvi( XclImpStream& rStrm )
{
    if( mxCurrField )
        mxCurrField->ReadSxvi( rStrm );
}

void XclImpPivotTable::ReadSxvdex( XclImpStream& rStrm )
{
    if( mxCurrField )
        mxCurrField->ReadSxvdex( rStrm );
}

void XclImpPivotTable::ReadSxivd( XclImpStream& rStrm )
{
    mxCurrField.reset();

    
    ScfUInt16Vec* pFieldVec = 0;
    if( maRowFields.empty() && (maPTInfo.mnRowFields > 0) )
        pFieldVec = &maRowFields;
    else if( maColFields.empty() && (maPTInfo.mnColFields > 0) )
        pFieldVec = &maColFields;

    
    if( pFieldVec )
    {
        sal_uInt16 nSize = ulimit_cast< sal_uInt16 >( rStrm.GetRecSize() / 2, EXC_PT_MAXROWCOLCOUNT );
        pFieldVec->reserve( nSize );
        for( sal_uInt16 nIdx = 0; nIdx < nSize; ++nIdx )
        {
            sal_uInt16 nFieldIdx;
            rStrm >> nFieldIdx;
            pFieldVec->push_back( nFieldIdx );

            
            if( nFieldIdx == EXC_SXIVD_DATA )
            {
                sal_uInt16 nAxis = (pFieldVec == &maRowFields) ? EXC_SXVD_AXIS_ROW : EXC_SXVD_AXIS_COL;
                maDataOrientField.SetAxes( nAxis );
            }
        }
    }
}

void XclImpPivotTable::ReadSxpi( XclImpStream& rStrm )
{
    mxCurrField.reset();

    sal_uInt16 nSize = ulimit_cast< sal_uInt16 >( rStrm.GetRecSize() / 6 );
    for( sal_uInt16 nEntry = 0; nEntry < nSize; ++nEntry )
    {
        XclPTPageFieldInfo aPageInfo;
        rStrm >> aPageInfo;
        if( XclImpPTField* pField = GetFieldAcc( aPageInfo.mnField ) )
        {
            maPageFields.push_back( aPageInfo.mnField );
            pField->SetPageFieldInfo( aPageInfo );
        }
        GetCurrSheetDrawing().SetSkipObj( aPageInfo.mnObjId );
    }
}

void XclImpPivotTable::ReadSxdi( XclImpStream& rStrm )
{
    mxCurrField.reset();

    XclPTDataFieldInfo aDataInfo;
    rStrm >> aDataInfo;
    if( XclImpPTField* pField = GetFieldAcc( aDataInfo.mnField ) )
    {
        maOrigDataFields.push_back( aDataInfo.mnField );
        
        if( !pField->HasDataFieldInfo() )
            maFiltDataFields.push_back( aDataInfo.mnField );
        pField->AddDataFieldInfo( aDataInfo );
    }
}

void XclImpPivotTable::ReadSxex( XclImpStream& rStrm )
{
    rStrm >> maPTExtInfo;
}

void XclImpPivotTable::ReadSxViewEx9( XclImpStream& rStrm )
{
    rStrm >> maPTViewEx9Info;
}



void XclImpPivotTable::Convert()
{
    if( !mxPCache || !mxPCache->IsValid() )
        return;

    ScDPSaveData aSaveData;

    

    aSaveData.SetRowGrand( ::get_flag( maPTInfo.mnFlags, EXC_SXVIEW_ROWGRAND ) );
    aSaveData.SetColumnGrand( ::get_flag( maPTInfo.mnFlags, EXC_SXVIEW_COLGRAND ) );
    aSaveData.SetFilterButton( false );
    aSaveData.SetDrillDown( ::get_flag( maPTExtInfo.mnFlags, EXC_SXEX_DRILLDOWN ) );

    

    ScfUInt16Vec::const_iterator aIt, aEnd;

    
    for( aIt = maRowFields.begin(), aEnd = maRowFields.end(); aIt != aEnd; ++aIt )
        if( const XclImpPTField* pField = GetField( *aIt ) )
            pField->ConvertRowColField( aSaveData );

    
    for( aIt = maColFields.begin(), aEnd = maColFields.end(); aIt != aEnd; ++aIt )
        if( const XclImpPTField* pField = GetField( *aIt ) )
            pField->ConvertRowColField( aSaveData );

    
    for( aIt = maPageFields.begin(), aEnd = maPageFields.end(); aIt != aEnd; ++aIt )
        if( const XclImpPTField* pField = GetField( *aIt ) )
            pField->ConvertPageField( aSaveData );

    
    
    

    
    for( sal_uInt16 nField = 0, nCount = GetFieldCount(); nField < nCount; ++nField )
        if( const XclImpPTField* pField = GetField( nField ) )
            if (!pField->GetAxes())
                pField->ConvertHiddenField( aSaveData );

    
    for( aIt = maFiltDataFields.begin(), aEnd = maFiltDataFields.end(); aIt != aEnd; ++aIt )
        if( const XclImpPTField* pField = GetField( *aIt ) )
            pField->ConvertDataField( aSaveData );

    

    
    ScSheetSourceDesc aDesc(GetDocPtr());
    const OUString& rSrcName = mxPCache->GetSourceRangeName();
    if (!rSrcName.isEmpty())
        
        aDesc.SetRangeName(rSrcName);
    else
        
        aDesc.SetSourceRange(mxPCache->GetSourceRange());

    
    ScRange aOutRange( maOutScRange );
    if( !maPageFields.empty() )
    {
        SCsROW nDecRows = ::std::min< SCsROW >( aOutRange.aStart.Row(), maPageFields.size() + 1 );
        aOutRange.aStart.IncRow( -nDecRows );
    }

    
    ScDPObject* pDPObj = new ScDPObject( GetDocPtr() );
    pDPObj->SetName( maPTInfo.maTableName );
    if (!maPTInfo.maDataName.isEmpty())
        aSaveData.GetDataLayoutDimension()->SetLayoutName(maPTInfo.maDataName);

    if (!maPTViewEx9Info.maGrandTotalName.isEmpty())
        aSaveData.SetGrandTotalName(maPTViewEx9Info.maGrandTotalName);

    pDPObj->SetSaveData( aSaveData );
    pDPObj->SetSheetDesc( aDesc );
    pDPObj->SetOutRange( aOutRange );
    pDPObj->SetHeaderLayout( maPTViewEx9Info.mnGridLayout == 0 );

    GetDoc().GetDPCollection()->InsertNewTable(pDPObj);
    mpDPObj = pDPObj;

    ApplyMergeFlags(aOutRange, aSaveData);
}

void XclImpPivotTable::MaybeRefresh()
{
    if (mpDPObj && mxPCache->IsRefreshOnLoad())
    {
        
        
        ScRange aOutRange = mpDPObj->GetOutRange();
        mpDPObj->Output(aOutRange.aStart);
    }
}

void XclImpPivotTable::ApplyMergeFlags(const ScRange& rOutRange, const ScDPSaveData& rSaveData)
{
    

    ScDPOutputGeometry aGeometry(rOutRange, false);
    aGeometry.setColumnFieldCount(maPTInfo.mnColFields);
    aGeometry.setPageFieldCount(maPTInfo.mnPageFields);
    aGeometry.setDataFieldCount(maPTInfo.mnDataFields);
    aGeometry.setRowFieldCount(maPTInfo.mnRowFields);

    ScDocument& rDoc = GetDoc();

    vector<const ScDPSaveDimension*> aFieldDims;
    vector<ScAddress> aFieldBtns;

    aGeometry.getPageFieldPositions(aFieldBtns);
    vector<ScAddress>::const_iterator itr = aFieldBtns.begin(), itrEnd = aFieldBtns.end();
    for (; itr != itrEnd; ++itr)
    {
        rDoc.ApplyFlagsTab(itr->Col(), itr->Row(), itr->Col(), itr->Row(), itr->Tab(), SC_MF_BUTTON);

        sal_uInt16 nMFlag = SC_MF_BUTTON_POPUP;
        OUString aName = rDoc.GetString(itr->Col(), itr->Row(), itr->Tab());
        if (rSaveData.HasInvisibleMember(aName))
            nMFlag |= SC_MF_HIDDEN_MEMBER;

        rDoc.ApplyFlagsTab(itr->Col()+1, itr->Row(), itr->Col()+1, itr->Row(), itr->Tab(), nMFlag);
    }

    aGeometry.getColumnFieldPositions(aFieldBtns);
    rSaveData.GetAllDimensionsByOrientation(sheet::DataPilotFieldOrientation_COLUMN, aFieldDims);
    if (aFieldBtns.size() == aFieldDims.size())
    {
        itr    = aFieldBtns.begin();
        itrEnd = aFieldBtns.end();
        vector<const ScDPSaveDimension*>::const_iterator itDim = aFieldDims.begin();
        for (; itr != itrEnd; ++itr, ++itDim)
        {
            sal_Int16 nMFlag = SC_MF_BUTTON;
            const ScDPSaveDimension* pDim = *itDim;
            if (pDim->HasInvisibleMember())
                nMFlag |= SC_MF_HIDDEN_MEMBER;
            if (!pDim->IsDataLayout())
                nMFlag |= SC_MF_BUTTON_POPUP;
            rDoc.ApplyFlagsTab(itr->Col(), itr->Row(), itr->Col(), itr->Row(), itr->Tab(), nMFlag);
        }
    }

    aGeometry.getRowFieldPositions(aFieldBtns);
    rSaveData.GetAllDimensionsByOrientation(sheet::DataPilotFieldOrientation_ROW, aFieldDims);
    if (aFieldBtns.size() == aFieldDims.size())
    {
        itr    = aFieldBtns.begin();
        itrEnd = aFieldBtns.end();
        vector<const ScDPSaveDimension*>::const_iterator itDim = aFieldDims.begin();
        for (; itr != itrEnd; ++itr, ++itDim)
        {
            sal_Int16 nMFlag = SC_MF_BUTTON;
            const ScDPSaveDimension* pDim = *itDim;
            if (pDim->HasInvisibleMember())
                nMFlag |= SC_MF_HIDDEN_MEMBER;
            if (!pDim->IsDataLayout())
                nMFlag |= SC_MF_BUTTON_POPUP;
            rDoc.ApplyFlagsTab(itr->Col(), itr->Row(), itr->Col(), itr->Row(), itr->Tab(), nMFlag);
        }
    }
}



XclImpPivotTableManager::XclImpPivotTableManager( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot )
{
}

XclImpPivotTableManager::~XclImpPivotTableManager()
{
}



XclImpPivotCacheRef XclImpPivotTableManager::GetPivotCache( sal_uInt16 nCacheIdx )
{
    XclImpPivotCacheRef xPCache;
    if( nCacheIdx < maPCaches.size() )
        xPCache = maPCaches[ nCacheIdx ];
    return xPCache;
}

void XclImpPivotTableManager::ReadSxidstm( XclImpStream& rStrm )
{
    XclImpPivotCacheRef xPCache( new XclImpPivotCache( GetRoot() ) );
    maPCaches.push_back( xPCache );
    xPCache->ReadSxidstm( rStrm );
}

void XclImpPivotTableManager::ReadSxvs( XclImpStream& rStrm )
{
    if( !maPCaches.empty() )
        maPCaches.back()->ReadSxvs( rStrm );
}

void XclImpPivotTableManager::ReadDconref( XclImpStream& rStrm )
{
    if( !maPCaches.empty() )
        maPCaches.back()->ReadDconref( rStrm );
}

void XclImpPivotTableManager::ReadDConName( XclImpStream& rStrm )
{
    if( !maPCaches.empty() )
        maPCaches.back()->ReadDConName( rStrm );
}



void XclImpPivotTableManager::ReadSxview( XclImpStream& rStrm )
{
    XclImpPivotTableRef xPTable( new XclImpPivotTable( GetRoot() ) );
    maPTables.push_back( xPTable );
    xPTable->ReadSxview( rStrm );
}

void XclImpPivotTableManager::ReadSxvd( XclImpStream& rStrm )
{
    if( !maPTables.empty() )
        maPTables.back()->ReadSxvd( rStrm );
}

void XclImpPivotTableManager::ReadSxvdex( XclImpStream& rStrm )
{
    if( !maPTables.empty() )
        maPTables.back()->ReadSxvdex( rStrm );
}

void XclImpPivotTableManager::ReadSxivd( XclImpStream& rStrm )
{
    if( !maPTables.empty() )
        maPTables.back()->ReadSxivd( rStrm );
}

void XclImpPivotTableManager::ReadSxpi( XclImpStream& rStrm )
{
    if( !maPTables.empty() )
        maPTables.back()->ReadSxpi( rStrm );
}

void XclImpPivotTableManager::ReadSxdi( XclImpStream& rStrm )
{
    if( !maPTables.empty() )
        maPTables.back()->ReadSxdi( rStrm );
}

void XclImpPivotTableManager::ReadSxvi( XclImpStream& rStrm )
{
    if( !maPTables.empty() )
        maPTables.back()->ReadSxvi( rStrm );
}

void XclImpPivotTableManager::ReadSxex( XclImpStream& rStrm )
{
    if( !maPTables.empty() )
        maPTables.back()->ReadSxex( rStrm );
}

void XclImpPivotTableManager::ReadSxViewEx9( XclImpStream& rStrm )
{
    if( !maPTables.empty() )
        maPTables.back()->ReadSxViewEx9( rStrm );
}



void XclImpPivotTableManager::ReadPivotCaches( XclImpStream& rStrm )
{
    for( XclImpPivotCacheVec::iterator aIt = maPCaches.begin(), aEnd = maPCaches.end(); aIt != aEnd; ++aIt )
        (*aIt)->ReadPivotCacheStream( rStrm );
}

void XclImpPivotTableManager::ConvertPivotTables()
{
    for( XclImpPivotTableVec::iterator aIt = maPTables.begin(), aEnd = maPTables.end(); aIt != aEnd; ++aIt )
        (*aIt)->Convert();
}

void XclImpPivotTableManager::MaybeRefreshPivotTables()
{
    for( XclImpPivotTableVec::iterator aIt = maPTables.begin(), aEnd = maPTables.end(); aIt != aEnd; ++aIt )
        (*aIt)->MaybeRefresh();
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
