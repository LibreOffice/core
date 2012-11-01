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
using ::rtl::OUString;

using ::rtl::OString;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

// ============================================================================
// Pivot cache
// ============================================================================

namespace {

// constants to track occurrence of specific data types
const sal_uInt16 EXC_PCITEM_DATA_STRING     = 0x0001;   /// String, empty, boolean, error.
const sal_uInt16 EXC_PCITEM_DATA_DOUBLE     = 0x0002;   /// Double with fraction.
const sal_uInt16 EXC_PCITEM_DATA_INTEGER    = 0x0004;   /// Integer, double without fraction.
const sal_uInt16 EXC_PCITEM_DATA_DATE       = 0x0008;   /// Date, time, date/time.

/** Maps a bitfield consisting of EXC_PCITEM_DATA_* flags above to SXFIELD data type bitfield. */
static const sal_uInt16 spnPCItemFlags[] =
{                               // STR DBL INT DAT
    EXC_SXFIELD_DATA_NONE,      //
    EXC_SXFIELD_DATA_STR,       //  x
    EXC_SXFIELD_DATA_INT,       //      x
    EXC_SXFIELD_DATA_STR_INT,   //  x   x
    EXC_SXFIELD_DATA_DBL,       //          x
    EXC_SXFIELD_DATA_STR_DBL,   //  x       x
    EXC_SXFIELD_DATA_INT,       //      x   x
    EXC_SXFIELD_DATA_STR_INT,   //  x   x   x
    EXC_SXFIELD_DATA_DATE,      //              x
    EXC_SXFIELD_DATA_DATE_STR,  //  x           x
    EXC_SXFIELD_DATA_DATE_NUM,  //      x       x
    EXC_SXFIELD_DATA_DATE_STR,  //  x   x       x
    EXC_SXFIELD_DATA_DATE_NUM,  //          x   x
    EXC_SXFIELD_DATA_DATE_STR,  //  x       x   x
    EXC_SXFIELD_DATA_DATE_NUM,  //      x   x   x
    EXC_SXFIELD_DATA_DATE_STR   //  x   x   x   x
};

} // namespace

// ----------------------------------------------------------------------------

XclExpPCItem::XclExpPCItem( const String& rText ) :
    XclExpRecord( (rText.Len() > 0) ? EXC_ID_SXSTRING : EXC_ID_SXEMPTY, 0 ),
    mnTypeFlag( EXC_PCITEM_DATA_STRING )
{
    if( rText.Len() )
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

// ----------------------------------------------------------------------------

bool XclExpPCItem::EqualsText( const rtl::OUString& rText ) const
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

// ----------------------------------------------------------------------------

void XclExpPCItem::WriteBody( XclExpStream& rStrm )
{
    if( const rtl::OUString* pText = GetText() )
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
        // nothing to do for SXEMPTY
        OSL_ENSURE( IsEmpty(), "XclExpPCItem::WriteBody - no data found" );
    }
}

// ============================================================================

XclExpPCField::XclExpPCField(
        const XclExpRoot& rRoot, const XclExpPivotCache& rPCache, sal_uInt16 nFieldIdx,
        const ScDPObject& rDPObj, const ScRange& rRange ) :
    XclExpRecord( EXC_ID_SXFIELD ),
    XclPCField( EXC_PCFIELD_STANDARD, nFieldIdx ),
    XclExpRoot( rRoot ),
    mrPCache( rPCache ),
    mnTypeFlags( 0 )
{
    // general settings for the standard field, insert all items from source range
    InitStandardField( rRange );

    // add special settings for inplace numeric grouping
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

    // final settings (flags, item numbers)
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
    // add base field info (always using first base field, not predecessor of this field) ***
    OSL_ENSURE( rBaseField.GetFieldName() == rGroupDim.GetSourceDimName(),
        "XclExpPCField::FillFromGroup - wrong base cache field" );
    maFieldInfo.maName = rGroupDim.GetGroupDimName();
    maFieldInfo.mnGroupBase = rBaseField.GetFieldIndex();

    // add standard group info or date group info
    const ScDPNumGroupInfo& rDateInfo = rGroupDim.GetDateInfo();
    if( rDateInfo.mbEnable && (rGroupDim.GetDatePart() != 0) )
        InitDateGroupField( rDPObj, rDateInfo, rGroupDim.GetDatePart() );
    else
        InitStdGroupField( rBaseField, rGroupDim );

    // final settings (flags, item numbers)
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

sal_uInt16 XclExpPCField::GetItemIndex( const rtl::OUString& rItemName ) const
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
    // only standard fields write item indexes
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
    // SXFIELD
    XclExpRecord::Save( rStrm );
    // SXFDBTYPE
    XclExpUInt16Record( EXC_ID_SXFDBTYPE, EXC_SXFDBTYPE_DEFAULT ).Save( rStrm );
    // list of grouping items
    maGroupItemList.Save( rStrm );
    // SXGROUPINFO
    WriteSxgroupinfo( rStrm );
    // SXNUMGROUP and additional grouping items (grouping limit settings)
    WriteSxnumgroup( rStrm );
    // list of original items
    maOrigItemList.Save( rStrm );
}

// private --------------------------------------------------------------------

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

    // field name is in top cell of the range
    ScAddress aPos( rRange.aStart );
    rDoc.GetString( aPos.Col(), aPos.Row(), aPos.Tab(), maFieldInfo.maName );
    // #i76047# maximum field name length in pivot cache is 255
    if (maFieldInfo.maName.getLength() > EXC_PC_MAXSTRLEN)
        maFieldInfo.maName = maFieldInfo.maName.copy(0, EXC_PC_MAXSTRLEN);

    // loop over all cells, create pivot cache items
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
            String aText;
            rDoc.GetString( aPos.Col(), aPos.Row(), aPos.Tab(), aText );
            InsertOrigTextItem( aText );
        }
    }
}

void XclExpPCField::InitStdGroupField( const XclExpPCField& rBaseField, const ScDPSaveGroupDimension& rGroupDim )
{
    OSL_ENSURE( IsGroupField(), "XclExpPCField::InitStdGroupField - only for standard grouping fields" );

    maFieldInfo.mnBaseItems = rBaseField.GetItemCount();
    maGroupOrder.resize( maFieldInfo.mnBaseItems, EXC_PC_NOITEM );

    // loop over all groups of this field
    for( long nGroupIdx = 0, nGroupCount = rGroupDim.GetGroupCount(); nGroupIdx < nGroupCount; ++nGroupIdx )
    {
        if( const ScDPSaveGroupItem* pGroupItem = rGroupDim.GetGroupByIndex( nGroupIdx ) )
        {
            // the index of the new item containing the grouping name
            sal_uInt16 nGroupItemIdx = EXC_PC_NOITEM;
            // loop over all elements of one group
            for( size_t nElemIdx = 0, nElemCount = pGroupItem->GetElementCount(); nElemIdx < nElemCount; ++nElemIdx )
            {
                if (const rtl::OUString* pElemName = pGroupItem->GetElementByIndex(nElemIdx))
                {
                    // try to find the item that is part of the group in the base field
                    sal_uInt16 nBaseItemIdx = rBaseField.GetItemIndex( *pElemName );
                    if( nBaseItemIdx < maFieldInfo.mnBaseItems )
                    {
                        // add group name item only if there are any valid base items
                        if( nGroupItemIdx == EXC_PC_NOITEM )
                            nGroupItemIdx = InsertGroupItem( new XclExpPCItem( pGroupItem->GetGroupName() ) );
                        maGroupOrder[ nBaseItemIdx ] = nGroupItemIdx;
                    }
                }
            }
        }
    }

    // add items and base item indexes of all ungrouped elements
    for( sal_uInt16 nBaseItemIdx = 0; nBaseItemIdx < maFieldInfo.mnBaseItems; ++nBaseItemIdx )
        // items that are not part of a group still have the EXC_PC_NOITEM entry
        if( maGroupOrder[ nBaseItemIdx ] == EXC_PC_NOITEM )
            // try to find the base item
            if( const XclExpPCItem* pBaseItem = rBaseField.GetItem( nBaseItemIdx ) )
                // create a clone of the base item, insert its index into item order list
                maGroupOrder[ nBaseItemIdx ] = InsertGroupItem( new XclExpPCItem( *pBaseItem ) );
}

void XclExpPCField::InitNumGroupField( const ScDPObject& rDPObj, const ScDPNumGroupInfo& rNumInfo )
{
    OSL_ENSURE( IsStandardField(), "XclExpPCField::InitNumGroupField - only for standard fields" );
    OSL_ENSURE( rNumInfo.mbEnable, "XclExpPCField::InitNumGroupField - numeric grouping not enabled" );

    // new field type, date type, limit settings (min/max/step/auto)
    if( rNumInfo.mbDateValues )
    {
        // special case: group by days with step count
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

    // generate visible items
    InsertNumDateGroupItems( rDPObj, rNumInfo );
}

void XclExpPCField::InitDateGroupField( const ScDPObject& rDPObj, const ScDPNumGroupInfo& rDateInfo, sal_Int32 nDatePart )
{
    OSL_ENSURE( IsStandardField() || IsStdGroupField(), "XclExpPCField::InitDateGroupField - only for standard fields" );
    OSL_ENSURE( rDateInfo.mbEnable, "XclExpPCField::InitDateGroupField - date grouping not enabled" );

    // new field type
    meFieldType = IsStandardField() ? EXC_PCFIELD_DATEGROUP : EXC_PCFIELD_DATECHILD;

    // date type, limit settings (min/max/step/auto)
    maNumGroupInfo.SetScDateType( nDatePart );
    SetDateGroupLimit( rDateInfo, false );

    // generate visible items
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

void XclExpPCField::InsertOrigTextItem( const String& rText )
{
    size_t nPos = 0;
    bool bFound = false;
    // #i76047# maximum item text length in pivot cache is 255
    String aShortText( rText, 0, ::std::min( rText.Len(), EXC_PC_MAXSTRLEN ) );
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
        // get the string collection with original source elements
        const ScDPSaveData* pSaveData = rDPObj.GetSaveData();
        const ScDPDimensionSaveData* pDimData = NULL;
        if (pSaveData)
            pDimData = pSaveData->GetExistingDimensionData();

        const ScDPCache* pCache = pSrcDesc->CreateCache(pDimData);
        if (!pCache)
            return;

        ScSheetDPData aDPData(GetDocPtr(), *pSrcDesc, *pCache);
        long nDim = GetFieldIndex();
        const std::vector< SCROW > aOrignial = aDPData.GetColumnEntries(nDim);
        // get the string collection with generated grouping elements
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
                rtl::OUString aStr = pCache->GetFormattedString(nDim, *pData);
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
    // flags
    ::set_flag( maFieldInfo.mnFlags, EXC_SXFIELD_HASITEMS, !GetVisItemList().IsEmpty() );
    // Excel writes long indexes even for 0x0100 items (indexes from 0x00 to 0xFF)
    ::set_flag( maFieldInfo.mnFlags, EXC_SXFIELD_16BIT, maOrigItemList.GetSize() >= 0x0100 );
    ::set_flag( maFieldInfo.mnFlags, EXC_SXFIELD_NUMGROUP, IsNumGroupField() || IsDateGroupField() );
    /*  mnTypeFlags is updated in all Insert***Item() functions. Now the flags
        for the current combination of item types is added to the flags. */
    ::set_flag( maFieldInfo.mnFlags, spnPCItemFlags[ mnTypeFlags ] );

    // item count fields
    maFieldInfo.mnVisItems = static_cast< sal_uInt16 >( GetVisItemList().GetSize() );
    maFieldInfo.mnGroupItems = static_cast< sal_uInt16 >( maGroupItemList.GetSize() );
    // maFieldInfo.mnBaseItems set in InitStdGroupField()
    maFieldInfo.mnOrigItems = static_cast< sal_uInt16 >( maOrigItemList.GetSize() );
}

void XclExpPCField::WriteSxnumgroup( XclExpStream& rStrm )
{
    if( IsNumGroupField() || IsDateGroupField() )
    {
        // SXNUMGROUP record
        rStrm.StartRecord( EXC_ID_SXNUMGROUP, 2 );
        rStrm << maNumGroupInfo;
        rStrm.EndRecord();

        // limits (min/max/step) for numeric grouping
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

// ============================================================================

XclExpPivotCache::XclExpPivotCache( const XclExpRoot& rRoot, const ScDPObject& rDPObj, sal_uInt16 nListIdx ) :
    XclExpRoot( rRoot ),
    mnListIdx( nListIdx ),
    mbValid( false )
{
    // source from sheet only
    if( const ScSheetSourceDesc* pSrcDesc = rDPObj.GetSheetDesc() )
    {
        /*  maOrigSrcRange: Range received from the DataPilot object.
            maExpSrcRange: Range written to the DCONREF record.
            maDocSrcRange: Range used to get source data from Calc document.
                This range may be shorter than maExpSrcRange to improve export
                performance (#i22541#). */
        maOrigSrcRange = maExpSrcRange = maDocSrcRange = pSrcDesc->GetSourceRange();
        maSrcRangeName = pSrcDesc->GetRangeName();

        // internal sheet data only
        SCTAB nScTab = maExpSrcRange.aStart.Tab();
        if( (nScTab == maExpSrcRange.aEnd.Tab()) && GetTabInfo().IsExportTab( nScTab ) )
        {
            // ValidateRange() restricts source range to valid Excel limits
            if( GetAddressConverter().ValidateRange( maExpSrcRange, true ) )
            {
                // #i22541# skip empty cell areas (performance)
                SCCOL nDocCol1, nDocCol2;
                SCROW nDocRow1, nDocRow2;
                GetDoc().GetDataStart( nScTab, nDocCol1, nDocRow1 );
                GetDoc().GetPrintArea( nScTab, nDocCol2, nDocRow2, false );
                SCCOL nSrcCol1 = maExpSrcRange.aStart.Col();
                SCROW nSrcRow1 = maExpSrcRange.aStart.Row();
                SCCOL nSrcCol2 = maExpSrcRange.aEnd.Col();
                SCROW nSrcRow2 = maExpSrcRange.aEnd.Row();

                // #i22541# do not store index list for too big ranges
                if( 2 * (nDocRow2 - nDocRow1) < (nSrcRow2 - nSrcRow1) )
                    ::set_flag( maPCInfo.mnFlags, EXC_SXDB_SAVEDATA, false );

                // adjust row indexes, keep one row of empty area to surely have the empty cache item
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
    // pivot cache can be shared, if there are no additional cache fields
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
    // SXIDSTM
    XclExpUInt16Record( EXC_ID_SXIDSTM, maPCInfo.mnStrmId ).Save( rStrm );
    // SXVS
    XclExpUInt16Record( EXC_ID_SXVS, EXC_SXVS_SHEET ).Save( rStrm );

    if (!maSrcRangeName.isEmpty())
        // DCONNAME
        WriteDConName(rStrm);
    else
        // DCONREF
        WriteDconref(rStrm);

    // create the pivot cache storage stream
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
            XML_cacheId, OString::valueOf( (sal_Int32)maPCInfo.mnStrmId ).getStr(),
            FSNS( XML_r, XML_id ), XclXmlUtils::ToOString( sId ).getStr(),
            FSEND );
    // SXIDSTM
    XclExpUInt16Record( EXC_ID_SXIDSTM, maPCInfo.mnStrmId ).SaveXml( rStrm );
    // SXVS
    XclExpUInt16Record( EXC_ID_SXVS, EXC_SXVS_SHEET ).SaveXml( rStrm );
    // DCONREF
    // OOXTODO: WriteDconref( rStrm );
    // create the pivot cache storage stream
    // OOXTODO: WriteCacheStream();
    rWorkbook->endElement( XML_pivotCache );
#endif /* XLSX_PIVOT_CACHE */
}

// private --------------------------------------------------------------------

XclExpPCField* XclExpPivotCache::GetFieldAcc( sal_uInt16 nFieldIdx )
{
    return maFieldList.GetRecord( nFieldIdx ).get();
}

XclExpPCField* XclExpPivotCache::GetFieldAcc( const rtl::OUString& rFieldName )
{
    XclExpPCField* pField = 0;
    for( size_t nPos = 0, nSize = maFieldList.GetSize(); !pField && (nPos < nSize); ++nPos )
        if( maFieldList.GetRecord( nPos )->GetFieldName() == rFieldName )
            pField = maFieldList.GetRecord( nPos ).get();
    return pField;
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
    // if item index list is not written, used shortened source range (maDocSrcRange) for performance
    const ScRange& rRange = HasItemIndexList() ? maExpSrcRange : maDocSrcRange;
    // create a standard pivot cache field for each source column
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
            // loop over all existing standard fields to find their group fields
            for( sal_uInt16 nFieldIdx = 0; nFieldIdx < maPCInfo.mnStdFields; ++nFieldIdx )
            {
                if( XclExpPCField* pCurrStdField = GetFieldAcc( nFieldIdx ) )
                {
                    const ScDPSaveGroupDimension* pGroupDim = pSaveDimData->GetGroupDimForBase( pCurrStdField->GetFieldName() );
                    XclExpPCField* pLastGroupField = pCurrStdField;
                    while( pGroupDim )
                    {
                        // insert the new grouping field
                        XclExpPCFieldRef xNewGroupField( new XclExpPCField(
                            GetRoot(), *this, GetFieldCount(), rDPObj, *pGroupDim, *pCurrStdField ) );
                        maFieldList.AppendRecord( xNewGroupField );

                        // register new grouping field at current grouping field, building a chain
                        pLastGroupField->SetGroupChildField( *xNewGroupField );

                        // next grouping dimension
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
    // not supported
}

void XclExpPivotCache::WriteDconref( XclExpStream& rStrm ) const
{
    XclExpString aRef( XclExpUrlHelper::EncodeUrl( GetRoot(), EMPTY_STRING, &maTabName ) );
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
        // SXDB
        WriteSxdb( aStrm );
        // SXDBEX
        WriteSxdbex( aStrm );
        // field list (SXFIELD and items)
        maFieldList.Save( aStrm );
        // index table (list of SXINDEXLIST)
        WriteSxindexlistList( aStrm );
        // EOF
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
            << sal_uInt32( 0 );             // number of SXFORMULA records
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

// ============================================================================
// Pivot table
// ============================================================================

namespace {

// ----------------------------------------------------------------------------

/** Returns a display string for a data field containing the field name and aggregation function. */
String lclGetDataFieldCaption( const String& rFieldName, GeneralFunction eFunc )
{
    String aCaption;

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
        aCaption.Assign( ScGlobal::GetRscString( nResIdx ) ).AppendAscii( RTL_CONSTASCII_STRINGPARAM( " - " ) );
    aCaption.Append( rFieldName );
    return aCaption;
}

// ----------------------------------------------------------------------------

} // namespace

// ============================================================================

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

rtl::OUString XclExpPTItem::GetItemName() const
{
    return mpCacheItem ? mpCacheItem->ConvertToText() : rtl::OUString();
}

void XclExpPTItem::SetPropertiesFromMember( const ScDPSaveMember& rSaveMem )
{
    // #i115659# GetIsVisible() is not valid if HasIsVisible() returns false, default is 'visible' then
    ::set_flag( maItemInfo.mnFlags, EXC_SXVI_HIDDEN, rSaveMem.HasIsVisible() && !rSaveMem.GetIsVisible() );
    // #i115659# GetShowDetails() is not valid if HasShowDetails() returns false, default is 'show detail' then
    ::set_flag( maItemInfo.mnFlags, EXC_SXVI_HIDEDETAIL, rSaveMem.HasShowDetails() && !rSaveMem.GetShowDetails() );

    // visible name
    const OUString* pVisName = rSaveMem.GetLayoutName();
    if (pVisName && !pVisName->equals(GetItemName()))
        maItemInfo.SetVisName(*pVisName);
}

void XclExpPTItem::WriteBody( XclExpStream& rStrm )
{
    rStrm << maItemInfo;
}

// ============================================================================

XclExpPTField::XclExpPTField( const XclExpPivotTable& rPTable, sal_uInt16 nCacheIdx ) :
    mrPTable( rPTable ),
    mpCacheField( rPTable.GetCacheField( nCacheIdx ) )
{
    maFieldInfo.mnCacheIdx = nCacheIdx;

    // create field items
    if( mpCacheField )
        for( sal_uInt16 nItemIdx = 0, nItemCount = mpCacheField->GetItemCount(); nItemIdx < nItemCount; ++nItemIdx )
            maItemList.AppendNewRecord( new XclExpPTItem( *mpCacheField, nItemIdx ) );
    maFieldInfo.mnItemCount = static_cast< sal_uInt16 >( maItemList.GetSize() );
}

// data access ----------------------------------------------------------------

rtl::OUString XclExpPTField::GetFieldName() const
{
    return mpCacheField ? mpCacheField->GetFieldName() : rtl::OUString();
}

sal_uInt16 XclExpPTField::GetFieldIndex() const
{
    // field index always equal to cache index
    return maFieldInfo.mnCacheIdx;
}

sal_uInt16 XclExpPTField::GetLastDataInfoIndex() const
{
    OSL_ENSURE( !maDataInfoVec.empty(), "XclExpPTField::GetLastDataInfoIndex - no data info found" );
    // will return 0xFFFF for empty vector -> ok
    return static_cast< sal_uInt16 >( maDataInfoVec.size() - 1 );
}

sal_uInt16 XclExpPTField::GetItemIndex( const rtl::OUString& rName, sal_uInt16 nDefaultIdx ) const
{
    for( size_t nPos = 0, nSize = maItemList.GetSize(); nPos < nSize; ++nPos )
        if( maItemList.GetRecord( nPos )->GetItemName() == rName )
            return static_cast< sal_uInt16 >( nPos );
    return nDefaultIdx;
}

// fill data --------------------------------------------------------------

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
        if (!bEscaped && c == sal_Unicode('\\'))
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
    // orientation
    DataPilotFieldOrientation eOrient = static_cast< DataPilotFieldOrientation >( rSaveDim.GetOrientation() );
    OSL_ENSURE( eOrient != DataPilotFieldOrientation_DATA, "XclExpPTField::SetPropertiesFromDim - called for data field" );
    maFieldInfo.AddApiOrient( eOrient );

    // show empty items (#i115659# GetShowEmpty() is not valid if HasShowEmpty() returns false, default is false then)
    ::set_flag( maFieldExtInfo.mnFlags, EXC_SXVDEX_SHOWALL, rSaveDim.HasShowEmpty() && rSaveDim.GetShowEmpty() );

    // visible name
    const OUString* pLayoutName = rSaveDim.GetLayoutName();
    if (pLayoutName && !pLayoutName->equals(GetFieldName()))
        maFieldInfo.SetVisName(*pLayoutName);

    const rtl::OUString* pSubtotalName = rSaveDim.GetSubtotalName();
    if (pSubtotalName)
    {
        OUString aSubName = lcl_convertCalcSubtotalName(*pSubtotalName);
        maFieldExtInfo.mpFieldTotalName.reset(new rtl::OUString(aSubName));
    }

    // subtotals
    XclPTSubtotalVec aSubtotals;
    aSubtotals.reserve( static_cast< size_t >( rSaveDim.GetSubTotalsCount() ) );
    for( long nSubtIdx = 0, nSubtCount = rSaveDim.GetSubTotalsCount(); nSubtIdx < nSubtCount; ++nSubtIdx )
        aSubtotals.push_back( rSaveDim.GetSubTotalFunc( nSubtIdx ) );
    maFieldInfo.SetSubtotals( aSubtotals );

    // sorting
    if( const DataPilotFieldSortInfo* pSortInfo = rSaveDim.GetSortInfo() )
    {
        maFieldExtInfo.SetApiSortMode( pSortInfo->Mode );
        if( pSortInfo->Mode == ::com::sun::star::sheet::DataPilotFieldSortMode::DATA )
            maFieldExtInfo.mnSortField = mrPTable.GetDataFieldIndex( pSortInfo->Field, EXC_SXVDEX_SORT_OWN );
        ::set_flag( maFieldExtInfo.mnFlags, EXC_SXVDEX_SORT_ASC, pSortInfo->IsAscending );
    }

    // auto show
    if( const DataPilotFieldAutoShowInfo* pShowInfo = rSaveDim.GetAutoShowInfo() )
    {
        ::set_flag( maFieldExtInfo.mnFlags, EXC_SXVDEX_AUTOSHOW, pShowInfo->IsEnabled );
        maFieldExtInfo.SetApiAutoShowMode( pShowInfo->ShowItemsMode );
        maFieldExtInfo.SetApiAutoShowCount( pShowInfo->ItemCount );
        maFieldExtInfo.mnShowField = mrPTable.GetDataFieldIndex( pShowInfo->DataField, EXC_SXVDEX_SHOW_NONE );
    }

    // layout
    if( const DataPilotFieldLayoutInfo* pLayoutInfo = rSaveDim.GetLayoutInfo() )
    {
        maFieldExtInfo.SetApiLayoutMode( pLayoutInfo->LayoutMode );
        ::set_flag( maFieldExtInfo.mnFlags, EXC_SXVDEX_LAYOUT_BLANK, pLayoutInfo->AddEmptyLines );
    }

    // special page field properties
    if( eOrient == DataPilotFieldOrientation_PAGE )
    {
        maPageInfo.mnField = GetFieldIndex();

        // selected item
        if( rSaveDim.HasCurrentPage() )
            maPageInfo.mnSelItem = GetItemIndex( rSaveDim.GetCurrentPage(), EXC_SXPI_ALLITEMS );
        else
            maPageInfo.mnSelItem = EXC_SXPI_ALLITEMS;
    }

    // item properties
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

    // orientation
    maFieldInfo.AddApiOrient( DataPilotFieldOrientation_DATA );

    // aggregation function
    GeneralFunction eFunc = static_cast< GeneralFunction >( rSaveDim.GetFunction() );
    rDataInfo.SetApiAggFunc( eFunc );

    // visible name
    const rtl::OUString* pVisName = rSaveDim.GetLayoutName();
    if (pVisName)
        rDataInfo.SetVisName(*pVisName);
    else
        rDataInfo.SetVisName( lclGetDataFieldCaption( GetFieldName(), eFunc ) );

    // result field reference
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

// records --------------------------------------------------------------------

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
    // SXVD
    WriteSxvd( rStrm );
    // list of SXVI records
    maItemList.Save( rStrm );
    // SXVDEX
    WriteSxvdex( rStrm );
}

// private --------------------------------------------------------------------

XclExpPTItem* XclExpPTField::GetItemAcc( const rtl::OUString& rName )
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

// ============================================================================

XclExpPivotTable::XclExpPivotTable( const XclExpRoot& rRoot, const ScDPObject& rDPObj, const XclExpPivotCache& rPCache ) :
    XclExpRoot( rRoot ),
    mrPCache( rPCache ),
    maDataOrientField( *this, EXC_SXIVD_DATA ),
    mnOutScTab( 0 ),
    mbValid( false ),
    mbFilterBtn( false )
{
    const ScRange& rOutScRange = rDPObj.GetOutRange();
    if( GetAddressConverter().ConvertRange( maPTInfo.maOutXclRange, rOutScRange, true ) )
    {
        // DataPilot properties -----------------------------------------------

        // pivot table properties from DP object
        mnOutScTab = rOutScRange.aStart.Tab();
        maPTInfo.maTableName = rDPObj.GetName();
        maPTInfo.mnCacheIdx = mrPCache.GetCacheIndex();

        maPTViewEx9Info.Init( rDPObj );

        if( const ScDPSaveData* pSaveData = rDPObj.GetSaveData() )
        {
            // additional properties from ScDPSaveData
            SetPropertiesFromDP( *pSaveData );

            // loop over all dimensions ---------------------------------------

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

            // Finalize -------------------------------------------------------

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

const XclExpPTField* XclExpPivotTable::GetField( const rtl::OUString& rName ) const
{
    return const_cast< XclExpPivotTable* >( this )->GetFieldAcc( rName );
}

sal_uInt16 XclExpPivotTable::GetDataFieldIndex( const rtl::OUString& rName, sal_uInt16 nDefaultIdx ) const
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
        // SXVIEW
        WriteSxview( rStrm );
        // pivot table fields (SXVD, SXVDEX, and item records)
        maFieldList.Save( rStrm );
        // SXIVD records for row and column fields
        WriteSxivd( rStrm, maRowFields );
        WriteSxivd( rStrm, maColFields );
        // SXPI
        WriteSxpi( rStrm );
        // list of SXDI records containing data field info
        WriteSxdiList( rStrm );
        // SXLI records
        WriteSxli( rStrm, maPTInfo.mnDataRows, maPTInfo.mnRowFields );
        WriteSxli( rStrm, maPTInfo.mnDataCols, maPTInfo.mnColFields );
        // SXEX
        WriteSxex( rStrm );
        // QSISXTAG
        WriteQsiSxTag( rStrm );
        // SXVIEWEX9
        WriteSxViewEx9( rStrm );
    }
}

void XclExpPivotTable::SaveXml( XclExpXmlStream& rStrm )
{
    if( !mbValid )
        return;
    sax_fastparser::FSHelperPtr aPivotTableDefinition = rStrm.CreateOutputStream(
            XclXmlUtils::GetStreamName( "xl/", "pivotTables/pivotTable", mnOutScTab+1),
            XclXmlUtils::GetStreamName( "../", "pivotTables/pivotTable", mnOutScTab+1),
            rStrm.GetCurrentStream()->getOutputStream(),
            "application/vnd.openxmlformats-officedocument.spreadsheetml.pivotTable+xml",
            "http://schemas.openxmlformats.org/officeDocument/2006/relationships/pivotTable");
    rStrm.PushStream( aPivotTableDefinition );

    aPivotTableDefinition->startElement( XML_pivotTableDefinition,
            XML_xmlns,                      "http://schemas.openxmlformats.org/spreadsheetml/2006/main",
            XML_name,                       XclXmlUtils::ToOString( maPTInfo.maTableName ).getStr(),
            XML_cacheId,                    OString::valueOf( (sal_Int32) maPTInfo.mnCacheIdx ).getStr(),
            XML_dataOnRows,                 XclXmlUtils::ToPsz( maPTInfo.mnDataAxis == EXC_SXVD_AXIS_COL ),
            XML_dataPosition,               OString::valueOf( (sal_Int32) maPTInfo.mnDataPos ).getStr(),
            XML_autoFormatId,               OString::valueOf( (sal_Int32) maPTInfo.mnAutoFmtIdx ).getStr(),
            // OOXTODO: XML_applyNumberFormats,         [ SXVIEW fAtrNum (maPTInfo.mnFlags) ]
            // OOXTODO: XML_applyBorderFormats,         [ SXVIEW fAtrBdr (maPTInfo.mnFlags) ]
            // OOXTODO: XML_applyFontFormats,           [ SXVIEW fAtrFnt (maPTInfo.mnFlags) ]
            // OOXTODO: XML_applyPatternFormats,        [ SXVIEW fAtrPat (maPTInfo.mnFlags) ]
            // OOXTODO: XML_applyAlignmentFormats,      [ SXVIEW fAtrAlc (maPTInfo.mnFlags) ]
            // OOXTODO: XML_applyWidthHeightFormats,    [ SXVIEW fAtrProc (maPTInfo.mnFlags) ]
            XML_dataCaption,                XclXmlUtils::ToOString( maPTInfo.maDataName ).getStr(),
            // OOXTODO: XML_grandTotalCaption,          [ SxViewEx9 chGrand ]
            // OOXTODO: XML_errorCaption,               [ SXEx stError ]
            // OOXTODO: XML_showError,                  [ SXEx fDisplayErrorString ]
            // OOXTODO: XML_missingCaption,             [ SXEx stDisplayNull ]
            // OOXTODO: XML_showMissing,                [ SXEx fDisplayNullString ]
            // OOXTODO: XML_pageStyle,                  [ SXEx stPageFieldStyle ]
            // OOXTODO: XML_pivotTableStyle,            [ SXEx stTableStyle ]
            // OOXTODO: XML_vacatedStyle,               [ SXEx stVacateStyle ]
            // OOXTODO: XML_tag,                        [ SXEx stTag ]
            // OOXTODO: XML_updatedVersion,             [ app-dependent ]
            // OOXTODO: XML_minRefreshableVersion,      [ app-dependent ]
            // OOXTODO: XML_asteriskTotals,             [ QsiSXTag/SXView9Save fHideTotAnnotation ]
            // OOXTODO: XML_showItems,                  [ ??? ]
            // OOXTODO: XML_editData,                   [ ??? ]
            // OOXTODO: XML_disableFieldList,           [ SXEx fEnableFieldDialog? ]
            // OOXTODO: XML_showCalcMbrs,               [ ??? ]
            // OOXTODO: XML_visualTotals,               [ ??? ]
            // OOXTODO: XML_showMultipleLabel,          [ SXEx fMergeLabels? ]
            // OOXTODO: XML_showDataDropDown,           [ SXEx fEnableDrillDown? ]
            // OOXTODO: XML_showDrill,                  [ ??? ]
            // OOXTODO: XML_printDrill,                 [ ??? ]
            // OOXTODO: XML_showMemberPropertyTips,
            // OOXTODO: XML_showDataTips,
            // OOXTODO: XML_enableWizard,
            XML_enableDrill,                XclXmlUtils::ToPsz( maPTExtInfo.mnFlags & EXC_SXEX_DRILLDOWN ), // ???
            // OOXTODO: XML_enableFieldProperties,      [ SXEx fEnableFieldDialog (maPTExtInfo.mnFlags) ]
            // OOXTODO: XML_preserveFormatting,         [ SXEx fPreserveFormatting (maPTExtInfo.mnFlags) ]
            // OOXTODO: XML_pageWrap,                   [ SXEx cWrapPage (maPTExtInfo.mnFlags) ]
            // OOXTODO: XML_pageOverThenDown,           [ SXEx fAcrossPageLay (maPTExtInfo.mnFlags) ]
            // OOXTODO: XML_subtotalHiddenItems,        [ SXEx fSubtotalHiddenPageItems (maPTExtInfo.mnFlags) ]
            XML_rowGrandTotals,             XclXmlUtils::ToPsz( maPTInfo.mnFlags & EXC_SXVIEW_ROWGRAND ),
            XML_colGrandTotals,             XclXmlUtils::ToPsz( maPTInfo.mnFlags & EXC_SXVIEW_COLGRAND ),
            // OOXTODO: XML_fieldPrintTitles,
            // OOXTODO: XML_itemPrintTitles,
            // OOXTODO: XML_mergeItem,
            // OOXTODO: XML_showDropZones,
            // OOXTODO: XML_createdVersion,
            // OOXTODO: XML_indent,
            // OOXTODO: XML_showEmptyRow,
            // OOXTODO: XML_showEmptyCol,
            // OOXTODO: XML_showHeaders,
            // OOXTODO: XML_compact,
            // OOXTODO: XML_outline,
            // OOXTODO: XML_outlineData,
            // OOXTODO: XML_compactData,
            // OOXTODO: XML_published,
            // OOXTODO: XML_gridDropZones,
            // OOXTODO: XML_immersive,
            // OOXTODO: XML_multipleFieldFilters,
            // OOXTODO: XML_chartFormat,
            // OOXTODO: XML_rowHeaderCaption,
            // OOXTODO: XML_colHeaderCaption,
            // OOXTODO: XML_fieldListSortAscending,
            // OOXTODO: XML_mdxSubqueries,
            // OOXTODO: XML_customListSort,
            FSEND );

    aPivotTableDefinition->singleElement( XML_location,
            XML_ref,            XclXmlUtils::ToOString( maPTInfo.maOutXclRange ).getStr(),
            XML_firstHeaderRow, OString::valueOf( (sal_Int32) maPTInfo.mnFirstHeadRow ).getStr(),
            XML_firstDataRow,   OString::valueOf( (sal_Int32) maPTInfo.maDataXclPos.mnRow ).getStr(),
            XML_firstDataCol,   OString::valueOf( (sal_Int32) maPTInfo.maDataXclPos.mnCol ).getStr(),
            XML_rowPageCount,   OString::valueOf( (sal_Int32) maPTInfo.mnDataRows ).getStr(),   // OOXTODO?
            XML_colPageCount,   OString::valueOf( (sal_Int32) maPTInfo.mnDataCols ).getStr(),   // OOXTODO?
            FSEND );

    // OOXTODO: XML_pivotFields

    // maPTInfo.mnFields?
    if( maPTInfo.mnRowFields )
    {
        aPivotTableDefinition->startElement( XML_rowFields,
                XML_count,  OString::valueOf( (sal_Int32) maPTInfo.mnRowFields ).getStr(),
                FSEND );
        aPivotTableDefinition->endElement( XML_rowFields );
    }

    // OOXTODO: XML_rowItems

    if( maPTInfo.mnColFields )
    {
        aPivotTableDefinition->startElement( XML_colFields,
                XML_count,  OString::valueOf( (sal_Int32) maPTInfo.mnColFields ).getStr(),
                FSEND );
        aPivotTableDefinition->endElement( XML_colFields );
    }

    // OOXTODO: XML_colItems

    if( maPTInfo.mnPageFields )
    {
        aPivotTableDefinition->startElement( XML_pageFields,
                XML_count,  OString::valueOf( (sal_Int32) maPTInfo.mnPageFields ).getStr(),
                FSEND );
        aPivotTableDefinition->endElement( XML_pageFields );
    }

    if( maPTInfo.mnDataFields )
    {
        aPivotTableDefinition->startElement( XML_dataFields,
                XML_count,  OString::valueOf( (sal_Int32) maPTInfo.mnDataFields ).getStr(),
                FSEND );
        aPivotTableDefinition->endElement( XML_dataFields );
    }

    // OOXTODO: XML_formats, XML_conditionalFormats, XML_chartFormats,
    //          XML_pivotHierarchies, XML_pivotTableStyleInfo, XML_filters,
    //          XML_rowHierarchiesUsage, XML_colHierarchiesUsage, XML_ext

    aPivotTableDefinition->endElement( XML_pivotTableDefinition );

    rStrm.PopStream();
}

// private --------------------------------------------------------------------

XclExpPTField* XclExpPivotTable::GetFieldAcc( const rtl::OUString& rName )
{
    XclExpPTField* pField = 0;
    for( size_t nPos = 0, nSize = maFieldList.GetSize(); !pField && (nPos < nSize); ++nPos )
        if( maFieldList.GetRecord( nPos )->GetFieldName() == rName )
            pField = maFieldList.GetRecord( nPos ).get();
    return pField;
}

XclExpPTField* XclExpPivotTable::GetFieldAcc( const ScDPSaveDimension& rSaveDim )
{
    // data field orientation field?
    if( rSaveDim.IsDataLayout() )
        return &maDataOrientField;

    // a real dimension
    rtl::OUString aFieldName = ScDPUtil::getSourceDimensionName(rSaveDim.GetName());
    return aFieldName.isEmpty() ? NULL : GetFieldAcc(aFieldName);
}

// fill data --------------------------------------------------------------

void XclExpPivotTable::SetPropertiesFromDP( const ScDPSaveData& rSaveData )
{
    ::set_flag( maPTInfo.mnFlags, EXC_SXVIEW_ROWGRAND, rSaveData.GetRowGrand() );
    ::set_flag( maPTInfo.mnFlags, EXC_SXVIEW_COLGRAND, rSaveData.GetColumnGrand() );
    ::set_flag( maPTExtInfo.mnFlags, EXC_SXEX_DRILLDOWN, rSaveData.GetDrillDown() );
    mbFilterBtn = rSaveData.GetFilterButton();
    const ScDPSaveDimension* pDim = rSaveData.GetExistingDataLayoutDimension();
    if (!pDim)
        return;

    const rtl::OUString* pLayoutName = pDim->GetLayoutName();
    if (pLayoutName)
        maPTInfo.maDataName = *pLayoutName;
    else
        maPTInfo.maDataName = ScGlobal::GetRscString(STR_PIVOT_DATA);
}

void XclExpPivotTable::SetFieldPropertiesFromDim( const ScDPSaveDimension& rSaveDim )
{
    if( XclExpPTField* pField = GetFieldAcc( rSaveDim ) )
    {
        // field properties
        pField->SetPropertiesFromDim( rSaveDim );

        // update the corresponding field position list
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
        // field properties
        pField->SetDataPropertiesFromDim( rSaveDim );
        // update the data field position list
        maDataFields.push_back( XclPTDataFieldPos( pField->GetFieldIndex(), pField->GetLastDataInfoIndex() ) );
    }
}

void XclExpPivotTable::Finalize()
{
    // field numbers
    maPTInfo.mnFields = static_cast< sal_uInt16 >( maFieldList.GetSize() );
    maPTInfo.mnRowFields = static_cast< sal_uInt16 >( maRowFields.size() );
    maPTInfo.mnColFields = static_cast< sal_uInt16 >( maColFields.size() );
    maPTInfo.mnPageFields = static_cast< sal_uInt16 >( maPageFields.size() );
    maPTInfo.mnDataFields = static_cast< sal_uInt16 >( maDataFields.size() );

    maPTExtInfo.mnPagePerRow = maPTInfo.mnPageFields;
    maPTExtInfo.mnPagePerCol = (maPTInfo.mnPageFields > 0) ? 1 : 0;

    // subtotal items
    for( size_t nPos = 0, nSize = maFieldList.GetSize(); nPos < nSize; ++nPos )
        maFieldList.GetRecord( nPos )->AppendSubtotalItems();

    // find data field orientation field
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

    // single data field is always row oriented
    if( maPTInfo.mnDataAxis == EXC_SXVD_AXIS_NONE )
        maPTInfo.mnDataAxis = EXC_SXVD_AXIS_ROW;

    // update output range (initialized in ctor)
    sal_uInt16& rnXclCol1 = maPTInfo.maOutXclRange.maFirst.mnCol;
    sal_uInt32& rnXclRow1 = maPTInfo.maOutXclRange.maFirst.mnRow;
    sal_uInt16& rnXclCol2 = maPTInfo.maOutXclRange.maLast.mnCol;
    sal_uInt32& rnXclRow2 = maPTInfo.maOutXclRange.maLast.mnRow;
    // exclude page fields from output range
    rnXclRow1 = rnXclRow1 + maPTInfo.mnPageFields;
    // exclude filter button from output range
    if( mbFilterBtn )
        ++rnXclRow1;
    // exclude empty row between (filter button and/or page fields) and table
    if( mbFilterBtn || maPTInfo.mnPageFields )
        ++rnXclRow1;

    // data area
    sal_uInt16& rnDataXclCol = maPTInfo.maDataXclPos.mnCol;
    sal_uInt32& rnDataXclRow = maPTInfo.maDataXclPos.mnRow;
    rnDataXclCol = rnXclCol1 + maPTInfo.mnRowFields;
    rnDataXclRow = rnXclRow1 + maPTInfo.mnColFields + 1;
    if( maDataFields.empty() )
        ++rnDataXclRow;

    bool bExtraHeaderRow = (0 == maPTViewEx9Info.mnGridLayout && maPTInfo.mnColFields == 0);
    if (bExtraHeaderRow)
        // Insert an extra row only when there is no column field.
        ++rnDataXclRow;

    rnXclCol2 = ::std::max( rnXclCol2, rnDataXclCol );
    rnXclRow2 = ::std::max( rnXclRow2, rnDataXclRow );
    maPTInfo.mnDataCols = rnXclCol2 - rnDataXclCol + 1;
    maPTInfo.mnDataRows = rnXclRow2 - rnDataXclRow + 1;

    // first heading
    maPTInfo.mnFirstHeadRow = rnXclRow1;
    if (bExtraHeaderRow)
        maPTInfo.mnFirstHeadRow += 2;
}

// records ----------------------------------------------------------------

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
//        rStrm.SetSliceSize( nLineSize );

        for( sal_uInt16 nLine = 0; nLine < nLineCount; ++nLine )
        {
            // Excel XP needs a partly initialized SXLI record
            rStrm   << sal_uInt16( 0 )      // number of equal index entries
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
    sal_uInt16 nTableType  = 1; // 0 = query table : 1 = pivot table

    rStrm << nRecordType << nDummyFlags << nTableType;

    // General flags
    bool bEnableRefresh = true;
    bool bPCacheInvalid = false;
    bool bOlapPTReport  = false;

    sal_uInt16 nFlags = 0x0000;
    if (bEnableRefresh) nFlags |= 0x0001;
    if (bPCacheInvalid) nFlags |= 0x0002;
    if (bOlapPTReport)  nFlags |= 0x0004;
    rStrm << nFlags;

    // Feature-specific options.  The value differs depending on the table
    // type, but we assume the table type is always pivot table.
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
    rStrm << static_cast<sal_uInt8>(eXclVer)  // version table last refreshed
          << static_cast<sal_uInt8>(eXclVer)  // minimum version to refresh
          << nOffsetBytes
          << static_cast<sal_uInt8>(eXclVer); // first version created

    rStrm << XclExpString(maPTInfo.maTableName);
    rStrm << static_cast<sal_uInt16>(0x0001); // no idea what this is for.

    rStrm.EndRecord();
}

void XclExpPivotTable::WriteSxViewEx9( XclExpStream& rStrm ) const
{
    // Until we sync the autoformat ids only export if using grid header layout
    // That could only have been set via xls import so far.
    if ( 0 == maPTViewEx9Info.mnGridLayout )
    {
        rStrm.StartRecord( EXC_ID_SXVIEWEX9, 17 );
        rStrm << maPTViewEx9Info;
        rStrm.EndRecord();
    }
}

// ============================================================================

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

} // namespace

// ----------------------------------------------------------------------------

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
                    maPTableList.AppendNewRecord( new XclExpPivotTable( GetRoot(), *pDPObj, *pPCache ) );
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

// private --------------------------------------------------------------------

const XclExpPivotCache* XclExpPivotTableManager::CreatePivotCache( const ScDPObject& rDPObj )
{
    // try to find a pivot cache with the same data source
    /*  #i25110# In Excel, the pivot cache contains additional fields
        (i.e. grouping info, calculated fields). If the passed DataPilot object
        or the found cache contains this data, do not share the cache with
        multiple pivot tables. */
    if( mbShareCaches )
    {
        if( const ScDPSaveData* pSaveData = rDPObj.GetSaveData() )
        {
            const ScDPDimensionSaveData* pDimSaveData = pSaveData->GetExistingDimensionData();
            // no dimension save data at all or save data does not contain grouping info
            if( !pDimSaveData || !pDimSaveData->HasGroupDimensions() )
            {
                // check all existing pivot caches
                for( size_t nPos = 0, nSize = maPCacheList.GetSize(); nPos < nSize; ++nPos )
                {
                    XclExpPivotCacheRef xPCache = maPCacheList.GetRecord( nPos );
                    // pivot cache does not have grouping info and source data is equal
                    if( !xPCache->HasAddFields() && xPCache->HasEqualDataSource( rDPObj ) )
                        return xPCache.get();
                }
            }
        }
    }

    // create a new pivot cache
    sal_uInt16 nNewCacheIdx = static_cast< sal_uInt16 >( maPCacheList.GetSize() );
    XclExpPivotCacheRef xNewPCache( new XclExpPivotCache( GetRoot(), rDPObj, nNewCacheIdx ) );
    if( xNewPCache->IsValid() )
    {
        maPCacheList.AppendRecord( xNewPCache );
        return xNewPCache.get();
    }

    return 0;
}

// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
