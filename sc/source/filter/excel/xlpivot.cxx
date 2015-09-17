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

#include "dpgroup.hxx"
#include "dpsave.hxx"
#include "xestream.hxx"
#include "xistream.hxx"
#include "xestring.hxx"
#include "xlpivot.hxx"
#include <osl/diagnose.h>
#include <com/sun/star/sheet/DataPilotFieldGroupBy.hpp>

using ::com::sun::star::sheet::GeneralFunction;
using ::com::sun::star::sheet::DataPilotFieldOrientation;

namespace ScDPSortMode = ::com::sun::star::sheet::DataPilotFieldSortMode;
namespace ScDPShowItemsMode = ::com::sun::star::sheet::DataPilotFieldShowItemsMode;
namespace ScDPLayoutMode = ::com::sun::star::sheet::DataPilotFieldLayoutMode;
namespace ScDPRefItemType = ::com::sun::star::sheet::DataPilotFieldReferenceItemType;
namespace ScDPGroupBy = ::com::sun::star::sheet::DataPilotFieldGroupBy;

// Pivot cache

XclPCItem::XclPCItem() :
    meType( EXC_PCITEM_INVALID ),
    maDateTime( DateTime::EMPTY )
{
}

XclPCItem::~XclPCItem()
{
}

void XclPCItem::SetEmpty()
{
    meType = EXC_PCITEM_EMPTY;
    maText.clear();
}

void XclPCItem::SetText( const OUString& rText )
{
    meType = EXC_PCITEM_TEXT;
    maText = rText;
}

void XclPCItem::SetDouble( double fValue )
{
    meType = EXC_PCITEM_DOUBLE;
    //TODO convert double to string
    maText.clear();
    mfValue = fValue;
}

void XclPCItem::SetDateTime( const DateTime& rDateTime )
{
    meType = EXC_PCITEM_DATETIME;
    //TODO convert date to string
    maText.clear();
    maDateTime = rDateTime;
}

void XclPCItem::SetInteger( sal_Int16 nValue )
{
    meType = EXC_PCITEM_INTEGER;
    maText = OUString::number(nValue);
    mnValue = nValue;
}

void XclPCItem::SetError( sal_uInt16 nError )
{
    meType = EXC_PCITEM_ERROR;
    maText.clear();
    mnError = nError;
    switch( nError )
    {
    case 0x00: maText = "#NULL!"; break;
    case 0x07: maText = "#DIV/0!"; break;
    case 0x0F: maText = "#VALUE!"; break;
    case 0x17: maText = "#REF!"; break;
    case 0x1D: maText = "#NAME?"; break;
    case 0x24: maText = "#NUM!"; break;
    case 0x2A: maText = "#N/A"; break;
    default: break;
    }
}

void XclPCItem::SetBool( bool bValue )
{
    meType = EXC_PCITEM_BOOL;
    //TODO convert boolean to string
    maText.clear();
    mbValue = bValue;
}

bool XclPCItem::IsEqual( const XclPCItem& rItem ) const
{
    if( meType == rItem.meType ) switch( meType )
    {
        case EXC_PCITEM_INVALID:    return true;
        case EXC_PCITEM_EMPTY:      return true;
        case EXC_PCITEM_TEXT:       return maText     == rItem.maText;
        case EXC_PCITEM_DOUBLE:     return mfValue    == rItem.mfValue;
        case EXC_PCITEM_DATETIME:   return maDateTime == rItem.maDateTime;
        case EXC_PCITEM_INTEGER:    return mnValue    == rItem.mnValue;
        case EXC_PCITEM_BOOL:       return mbValue    == rItem.mbValue;
        case EXC_PCITEM_ERROR:      return mnError    == rItem.mnError;
        default:    OSL_FAIL( "XclPCItem::IsEqual - unknown pivot cache item type" );
    }
    return false;
}

bool XclPCItem::IsEmpty() const
{
    return meType == EXC_PCITEM_EMPTY;
}

const OUString* XclPCItem::GetText() const
{
    return (meType == EXC_PCITEM_TEXT || meType == EXC_PCITEM_ERROR) ? &maText : NULL;
}

const double* XclPCItem::GetDouble() const
{
    return (meType == EXC_PCITEM_DOUBLE) ? &mfValue : 0;
}

const DateTime* XclPCItem::GetDateTime() const
{
    return (meType == EXC_PCITEM_DATETIME) ? &maDateTime : 0;
}

const sal_Int16* XclPCItem::GetInteger() const
{
    return (meType == EXC_PCITEM_INTEGER) ? &mnValue : 0;
}

const sal_uInt16* XclPCItem::GetError() const
{
    return (meType == EXC_PCITEM_ERROR) ? &mnError : 0;
}

const bool* XclPCItem::GetBool() const
{
    return (meType == EXC_PCITEM_BOOL) ? &mbValue : 0;
}

// Field settings =============================================================

XclPCFieldInfo::XclPCFieldInfo() :
    mnFlags( 0 ),
    mnGroupChild( 0 ),
    mnGroupBase( 0 ),
    mnVisItems( 0 ),
    mnGroupItems( 0 ),
    mnBaseItems( 0 ),
    mnOrigItems( 0 )
{
}

XclImpStream& operator>>( XclImpStream& rStrm, XclPCFieldInfo& rInfo )
{
    rInfo.mnFlags = rStrm.ReaduInt16();
    rInfo.mnGroupChild = rStrm.ReaduInt16();
    rInfo.mnGroupBase = rStrm.ReaduInt16();
    rInfo.mnVisItems = rStrm.ReaduInt16();
    rInfo.mnGroupItems = rStrm.ReaduInt16();
    rInfo.mnBaseItems = rStrm.ReaduInt16();
    rInfo.mnOrigItems = rStrm.ReaduInt16();
    if( rStrm.GetRecLeft() >= 3 )
        rInfo.maName = rStrm.ReadUniString();
    else
        rInfo.maName.clear();
    return rStrm;
}

XclExpStream& operator<<( XclExpStream& rStrm, const XclPCFieldInfo& rInfo )
{
    return rStrm
        << rInfo.mnFlags
        << rInfo.mnGroupChild
        << rInfo.mnGroupBase
        << rInfo.mnVisItems
        << rInfo.mnGroupItems
        << rInfo.mnBaseItems
        << rInfo.mnOrigItems
        << XclExpString( rInfo.maName );
}

// Numeric grouping field settings ============================================

XclPCNumGroupInfo::XclPCNumGroupInfo() :
    mnFlags( EXC_SXNUMGROUP_AUTOMIN | EXC_SXNUMGROUP_AUTOMAX )
{
    SetNumType();
}

void XclPCNumGroupInfo::SetNumType()
{
    SetXclDataType( EXC_SXNUMGROUP_TYPE_NUM );
}

sal_Int32 XclPCNumGroupInfo::GetScDateType() const
{
    sal_Int32 nScType = 0;
    switch( GetXclDataType() )
    {
        case EXC_SXNUMGROUP_TYPE_SEC:   nScType = ScDPGroupBy::SECONDS;   break;
        case EXC_SXNUMGROUP_TYPE_MIN:   nScType = ScDPGroupBy::MINUTES;   break;
        case EXC_SXNUMGROUP_TYPE_HOUR:  nScType = ScDPGroupBy::HOURS;     break;
        case EXC_SXNUMGROUP_TYPE_DAY:   nScType = ScDPGroupBy::DAYS;      break;
        case EXC_SXNUMGROUP_TYPE_MONTH: nScType = ScDPGroupBy::MONTHS;    break;
        case EXC_SXNUMGROUP_TYPE_QUART: nScType = ScDPGroupBy::QUARTERS;  break;
        case EXC_SXNUMGROUP_TYPE_YEAR:  nScType = ScDPGroupBy::YEARS;     break;
        default:    OSL_TRACE( "XclPCNumGroupInfo::GetScDateType - unexpected date type %d", GetXclDataType() );
    }
    return nScType;
}

void XclPCNumGroupInfo::SetScDateType( sal_Int32 nScType )
{
    sal_uInt16 nXclType = EXC_SXNUMGROUP_TYPE_NUM;
    switch( nScType )
    {
        case ScDPGroupBy::SECONDS:    nXclType = EXC_SXNUMGROUP_TYPE_SEC;     break;
        case ScDPGroupBy::MINUTES:    nXclType = EXC_SXNUMGROUP_TYPE_MIN;     break;
        case ScDPGroupBy::HOURS:      nXclType = EXC_SXNUMGROUP_TYPE_HOUR;    break;
        case ScDPGroupBy::DAYS:       nXclType = EXC_SXNUMGROUP_TYPE_DAY;     break;
        case ScDPGroupBy::MONTHS:     nXclType = EXC_SXNUMGROUP_TYPE_MONTH;   break;
        case ScDPGroupBy::QUARTERS:   nXclType = EXC_SXNUMGROUP_TYPE_QUART;   break;
        case ScDPGroupBy::YEARS:      nXclType = EXC_SXNUMGROUP_TYPE_YEAR;    break;
        default:
            SAL_INFO("sc.filter", "unexpected date type " << nScType);
    }
    SetXclDataType( nXclType );
}

sal_uInt16 XclPCNumGroupInfo::GetXclDataType() const
{
    return ::extract_value< sal_uInt16 >( mnFlags, 2, 4 );
}

void XclPCNumGroupInfo::SetXclDataType( sal_uInt16 nXclType )
{
    ::insert_value( mnFlags, nXclType, 2, 4 );
}

XclImpStream& operator>>( XclImpStream& rStrm, XclPCNumGroupInfo& rInfo )
{
    rInfo.mnFlags = rStrm.ReaduInt16();
    return rStrm;;
}

XclExpStream& operator<<( XclExpStream& rStrm, const XclPCNumGroupInfo& rInfo )
{
    return rStrm << rInfo.mnFlags;
}

// Base class for pivot cache fields ==========================================

XclPCField::XclPCField( XclPCFieldType eFieldType, sal_uInt16 nFieldIdx ) :
    meFieldType( eFieldType ),
    mnFieldIdx( nFieldIdx )
{
}

XclPCField::~XclPCField()
{
}

bool XclPCField::IsSupportedField() const
{
    return (meFieldType != EXC_PCFIELD_CALCED) && (meFieldType != EXC_PCFIELD_UNKNOWN);
}

bool XclPCField::IsStandardField() const
{
    return meFieldType == EXC_PCFIELD_STANDARD;
}

bool XclPCField::IsStdGroupField() const
{
    return meFieldType == EXC_PCFIELD_STDGROUP;
}

bool XclPCField::IsNumGroupField() const
{
    return meFieldType == EXC_PCFIELD_NUMGROUP;
}

bool XclPCField::IsDateGroupField() const
{
    return (meFieldType == EXC_PCFIELD_DATEGROUP) || (meFieldType == EXC_PCFIELD_DATECHILD);
}

bool XclPCField::IsGroupField() const
{
    return IsStdGroupField() || IsNumGroupField() || IsDateGroupField();
}

bool XclPCField::IsGroupBaseField() const
{
    return ::get_flag( maFieldInfo.mnFlags, EXC_SXFIELD_HASCHILD );
}

bool XclPCField::IsGroupChildField() const
{
    return (meFieldType == EXC_PCFIELD_STDGROUP) || (meFieldType == EXC_PCFIELD_DATECHILD);
}

bool XclPCField::HasOrigItems() const
{
    return IsSupportedField() && ((maFieldInfo.mnOrigItems > 0) || HasPostponedItems());
}

bool XclPCField::HasInlineItems() const
{
    return (IsStandardField() || IsGroupField()) && ((maFieldInfo.mnGroupItems > 0) || (maFieldInfo.mnOrigItems > 0));
}

bool XclPCField::HasPostponedItems() const
{
    return IsStandardField() && ::get_flag( maFieldInfo.mnFlags, EXC_SXFIELD_POSTPONE );
}

bool XclPCField::Has16BitIndexes() const
{
    return IsStandardField() && ::get_flag( maFieldInfo.mnFlags, EXC_SXFIELD_16BIT );
}

// Pivot cache settings =======================================================

/** Contains data for a pivot cache (SXDB record). */
XclPCInfo::XclPCInfo() :
    mnSrcRecs( 0 ),
    mnStrmId( 0xFFFF ),
    mnFlags( EXC_SXDB_DEFAULTFLAGS ),
    mnBlockRecs( EXC_SXDB_BLOCKRECS ),
    mnStdFields( 0 ),
    mnTotalFields( 0 ),
    mnSrcType( EXC_SXDB_SRC_SHEET )
{
}

XclImpStream& operator>>( XclImpStream& rStrm, XclPCInfo& rInfo )
{
    rInfo.mnSrcRecs = rStrm.ReaduInt32();
    rInfo.mnStrmId = rStrm.ReaduInt16();
    rInfo.mnFlags = rStrm.ReaduInt16();
    rInfo.mnBlockRecs = rStrm.ReaduInt16();
    rInfo.mnStdFields = rStrm.ReaduInt16();
    rInfo.mnTotalFields = rStrm.ReaduInt16();
    rStrm.Ignore( 2 );
    rInfo.mnSrcType = rStrm.ReaduInt16();
    rInfo.maUserName = rStrm.ReadUniString();
    return rStrm;
}

XclExpStream& operator<<( XclExpStream& rStrm, const XclPCInfo& rInfo )
{
    return rStrm
        << rInfo.mnSrcRecs
        << rInfo.mnStrmId
        << rInfo.mnFlags
        << rInfo.mnBlockRecs
        << rInfo.mnStdFields
        << rInfo.mnTotalFields
        << sal_uInt16( 0 )
        << rInfo.mnSrcType
        << XclExpString( rInfo.maUserName );
}

// Pivot table

// cached name ================================================================

XclImpStream& operator>>( XclImpStream& rStrm, XclPTCachedName& rCachedName )
{
    sal_uInt16 nStrLen;
    nStrLen = rStrm.ReaduInt16();
    rCachedName.mbUseCache = nStrLen == EXC_PT_NOSTRING;
    if( rCachedName.mbUseCache )
        rCachedName.maName.clear();
    else
        rCachedName.maName = rStrm.ReadUniString( nStrLen );
    return rStrm;
}

XclExpStream& operator<<( XclExpStream& rStrm, const XclPTCachedName& rCachedName )
{
    if( rCachedName.mbUseCache )
        rStrm << EXC_PT_NOSTRING;
    else
        rStrm << XclExpString( rCachedName.maName, EXC_STR_DEFAULT, EXC_PT_MAXSTRLEN );
    return rStrm;
}

const OUString* XclPTVisNameInfo::GetVisName() const
{
    return HasVisName() ? &maVisName.maName : 0;
}

void XclPTVisNameInfo::SetVisName( const OUString& rName )
{
    maVisName.maName = rName;
    maVisName.mbUseCache = rName.isEmpty();
}

// Field item settings ========================================================

XclPTItemInfo::XclPTItemInfo() :
    mnType( EXC_SXVI_TYPE_DATA ),
    mnFlags( EXC_SXVI_DEFAULTFLAGS ),
    mnCacheIdx( EXC_SXVI_DEFAULT_CACHE )
{
}

XclImpStream& operator>>( XclImpStream& rStrm, XclPTItemInfo& rInfo )
{
    rInfo.mnType = rStrm.ReaduInt16();
    rInfo.mnFlags = rStrm.ReaduInt16();
    rInfo.mnCacheIdx = rStrm.ReaduInt16();
    rStrm >> rInfo.maVisName;
    return rStrm;
}

XclExpStream& operator<<( XclExpStream& rStrm, const XclPTItemInfo& rInfo )
{
    return rStrm
        << rInfo.mnType
        << rInfo.mnFlags
        << rInfo.mnCacheIdx
        << rInfo.maVisName;
}

// General field settings =====================================================

XclPTFieldInfo::XclPTFieldInfo() :
    mnAxes( EXC_SXVD_AXIS_NONE ),
    mnSubtCount( 1 ),
    mnSubtotals( EXC_SXVD_SUBT_DEFAULT ),
    mnItemCount( 0 ),
    mnCacheIdx( EXC_SXVD_DEFAULT_CACHE )
{
}

DataPilotFieldOrientation XclPTFieldInfo::GetApiOrient( sal_uInt16 nMask ) const
{
    using namespace ::com::sun::star::sheet;
    DataPilotFieldOrientation eOrient = DataPilotFieldOrientation_HIDDEN;
    sal_uInt16 nUsedAxes = mnAxes & nMask;
    if( nUsedAxes & EXC_SXVD_AXIS_ROW )
        eOrient = DataPilotFieldOrientation_ROW;
    else if( nUsedAxes & EXC_SXVD_AXIS_COL )
        eOrient = DataPilotFieldOrientation_COLUMN;
    else if( nUsedAxes & EXC_SXVD_AXIS_PAGE )
        eOrient = DataPilotFieldOrientation_PAGE;
    else if( nUsedAxes & EXC_SXVD_AXIS_DATA )
        eOrient = DataPilotFieldOrientation_DATA;
    return eOrient;
}

void XclPTFieldInfo::AddApiOrient( DataPilotFieldOrientation eOrient )
{
    using namespace ::com::sun::star::sheet;
    switch( eOrient )
    {
        case DataPilotFieldOrientation_ROW:     mnAxes |= EXC_SXVD_AXIS_ROW;    break;
        case DataPilotFieldOrientation_COLUMN:  mnAxes |= EXC_SXVD_AXIS_COL;    break;
        case DataPilotFieldOrientation_PAGE:    mnAxes |= EXC_SXVD_AXIS_PAGE;   break;
        case DataPilotFieldOrientation_DATA:    mnAxes |= EXC_SXVD_AXIS_DATA;   break;
        default:;
    }
}

//TODO: should be a Sequence<GeneralFunction> in ScDPSaveData
void XclPTFieldInfo::GetSubtotals( XclPTSubtotalVec& rSubtotals ) const
{
    rSubtotals.clear();
    rSubtotals.reserve( 16 );

    using namespace ::com::sun::star::sheet;
    if( mnSubtotals & EXC_SXVD_SUBT_DEFAULT )   rSubtotals.push_back( GeneralFunction_AUTO );
    if( mnSubtotals & EXC_SXVD_SUBT_SUM )       rSubtotals.push_back( GeneralFunction_SUM );
    if( mnSubtotals & EXC_SXVD_SUBT_COUNT )     rSubtotals.push_back( GeneralFunction_COUNT );
    if( mnSubtotals & EXC_SXVD_SUBT_AVERAGE )   rSubtotals.push_back( GeneralFunction_AVERAGE );
    if( mnSubtotals & EXC_SXVD_SUBT_MAX )       rSubtotals.push_back( GeneralFunction_MAX );
    if( mnSubtotals & EXC_SXVD_SUBT_MIN )       rSubtotals.push_back( GeneralFunction_MIN );
    if( mnSubtotals & EXC_SXVD_SUBT_PROD )      rSubtotals.push_back( GeneralFunction_PRODUCT );
    if( mnSubtotals & EXC_SXVD_SUBT_COUNTNUM )  rSubtotals.push_back( GeneralFunction_COUNTNUMS );
    if( mnSubtotals & EXC_SXVD_SUBT_STDDEV )    rSubtotals.push_back( GeneralFunction_STDEV );
    if( mnSubtotals & EXC_SXVD_SUBT_STDDEVP )   rSubtotals.push_back( GeneralFunction_STDEVP );
    if( mnSubtotals & EXC_SXVD_SUBT_VAR )       rSubtotals.push_back( GeneralFunction_VAR );
    if( mnSubtotals & EXC_SXVD_SUBT_VARP )      rSubtotals.push_back( GeneralFunction_VARP );
}

void XclPTFieldInfo::SetSubtotals( const XclPTSubtotalVec& rSubtotals )
{
    mnSubtotals = EXC_SXVD_SUBT_NONE;
    using namespace ::com::sun::star::sheet;
    for( XclPTSubtotalVec::const_iterator aIt = rSubtotals.begin(), aEnd = rSubtotals.end(); aIt != aEnd; ++aIt )
    {
        switch( *aIt )
        {
            case GeneralFunction_AUTO:      mnSubtotals |= EXC_SXVD_SUBT_DEFAULT;   break;
            case GeneralFunction_SUM:       mnSubtotals |= EXC_SXVD_SUBT_SUM;       break;
            case GeneralFunction_COUNT:     mnSubtotals |= EXC_SXVD_SUBT_COUNT;     break;
            case GeneralFunction_AVERAGE:   mnSubtotals |= EXC_SXVD_SUBT_AVERAGE;   break;
            case GeneralFunction_MAX:       mnSubtotals |= EXC_SXVD_SUBT_MAX;       break;
            case GeneralFunction_MIN:       mnSubtotals |= EXC_SXVD_SUBT_MIN;       break;
            case GeneralFunction_PRODUCT:   mnSubtotals |= EXC_SXVD_SUBT_PROD;      break;
            case GeneralFunction_COUNTNUMS: mnSubtotals |= EXC_SXVD_SUBT_COUNTNUM;  break;
            case GeneralFunction_STDEV:     mnSubtotals |= EXC_SXVD_SUBT_STDDEV;    break;
            case GeneralFunction_STDEVP:    mnSubtotals |= EXC_SXVD_SUBT_STDDEVP;   break;
            case GeneralFunction_VAR:       mnSubtotals |= EXC_SXVD_SUBT_VAR;       break;
            case GeneralFunction_VARP:      mnSubtotals |= EXC_SXVD_SUBT_VARP;      break;
        }
    }

    mnSubtCount = 0;
    for( sal_uInt16 nMask = 0x8000; nMask; nMask >>= 1 )
        if( mnSubtotals & nMask )
            ++mnSubtCount;
}

XclImpStream& operator>>( XclImpStream& rStrm, XclPTFieldInfo& rInfo )
{
    // rInfo.mnCacheIdx is not part of the SXVD record
    rInfo.mnAxes = rStrm.ReaduInt16();
    rInfo.mnSubtCount = rStrm.ReaduInt16();
    rInfo.mnSubtotals = rStrm.ReaduInt16();
    rInfo.mnItemCount = rStrm.ReaduInt16();
    rStrm >> rInfo.maVisName;
    return rStrm;
}

XclExpStream& operator<<( XclExpStream& rStrm, const XclPTFieldInfo& rInfo )
{
    // rInfo.mnCacheIdx is not part of the SXVD record
    return rStrm
        << rInfo.mnAxes
        << rInfo.mnSubtCount
        << rInfo.mnSubtotals
        << rInfo.mnItemCount
        << rInfo.maVisName;
}

// Extended field settings ====================================================

XclPTFieldExtInfo::XclPTFieldExtInfo() :
    mnFlags( EXC_SXVDEX_DEFAULTFLAGS ),
    mnSortField( EXC_SXVDEX_SORT_OWN ),
    mnShowField( EXC_SXVDEX_SHOW_NONE ),
    mnNumFmt(0),
    mpFieldTotalName(NULL)
{
}

sal_Int32 XclPTFieldExtInfo::GetApiSortMode() const
{
    sal_Int32 nSortMode = ScDPSortMode::MANUAL;
    if( ::get_flag( mnFlags, EXC_SXVDEX_SORT ) )
        nSortMode = (mnSortField == EXC_SXVDEX_SORT_OWN) ? ScDPSortMode::NAME : ScDPSortMode::DATA;
    return nSortMode;
}

void XclPTFieldExtInfo::SetApiSortMode( sal_Int32 nSortMode )
{
    bool bSort = (nSortMode == ScDPSortMode::NAME) || (nSortMode == ScDPSortMode::DATA);
    ::set_flag( mnFlags, EXC_SXVDEX_SORT, bSort );
    if( nSortMode == ScDPSortMode::NAME )
        mnSortField = EXC_SXVDEX_SORT_OWN;  // otherwise sort field has to be set by caller
}

sal_Int32 XclPTFieldExtInfo::GetApiAutoShowMode() const
{
    return ::get_flagvalue( mnFlags, EXC_SXVDEX_AUTOSHOW_ASC,
        ScDPShowItemsMode::FROM_TOP, ScDPShowItemsMode::FROM_BOTTOM );
}

void XclPTFieldExtInfo::SetApiAutoShowMode( sal_Int32 nShowMode )
{
    ::set_flag( mnFlags, EXC_SXVDEX_AUTOSHOW_ASC, nShowMode == ScDPShowItemsMode::FROM_TOP );
}

sal_Int32 XclPTFieldExtInfo::GetApiAutoShowCount() const
{
    return ::extract_value< sal_Int32 >( mnFlags, 24, 8 );
}

void XclPTFieldExtInfo::SetApiAutoShowCount( sal_Int32 nShowCount )
{
    ::insert_value( mnFlags, limit_cast< sal_uInt8 >( nShowCount ), 24, 8 );
}

sal_Int32 XclPTFieldExtInfo::GetApiLayoutMode() const
{
    sal_Int32 nLayoutMode = ScDPLayoutMode::TABULAR_LAYOUT;
    if( ::get_flag( mnFlags, EXC_SXVDEX_LAYOUT_REPORT ) )
        nLayoutMode = ::get_flag( mnFlags, EXC_SXVDEX_LAYOUT_TOP ) ?
            ScDPLayoutMode::OUTLINE_SUBTOTALS_TOP : ScDPLayoutMode::OUTLINE_SUBTOTALS_BOTTOM;
    return nLayoutMode;
}

void XclPTFieldExtInfo::SetApiLayoutMode( sal_Int32 nLayoutMode )
{
    ::set_flag( mnFlags, EXC_SXVDEX_LAYOUT_REPORT, nLayoutMode != ScDPLayoutMode::TABULAR_LAYOUT );
    ::set_flag( mnFlags, EXC_SXVDEX_LAYOUT_TOP, nLayoutMode == ScDPLayoutMode::OUTLINE_SUBTOTALS_TOP );
}

XclImpStream& operator>>( XclImpStream& rStrm, XclPTFieldExtInfo& rInfo )
{
    sal_uInt8 nNameLen = 0;
    rInfo.mnFlags = rStrm.ReaduInt32();
    rInfo.mnSortField = rStrm.ReaduInt16();
    rInfo.mnShowField = rStrm.ReaduInt16();
    rInfo.mnNumFmt = rStrm.ReaduInt16();
    nNameLen = rStrm.ReaduInt8();

    rStrm.Ignore(10);
    if (nNameLen != 0xFF)
        // Custom field total name is used.  Pick it up.
        rInfo.mpFieldTotalName.reset(new OUString(rStrm.ReadUniString(nNameLen, 0)));

    return rStrm;
}

XclExpStream& operator<<( XclExpStream& rStrm, const XclPTFieldExtInfo& rInfo )
{
    rStrm   << rInfo.mnFlags
            << rInfo.mnSortField
            << rInfo.mnShowField
            << EXC_SXVDEX_FORMAT_NONE;

    if (rInfo.mpFieldTotalName.get() && !rInfo.mpFieldTotalName->isEmpty())
    {
        OUString aFinalName = *rInfo.mpFieldTotalName;
        if (aFinalName.getLength() >= 254)
            aFinalName = aFinalName.copy(0, 254);
        sal_uInt8 nNameLen = static_cast<sal_uInt8>(aFinalName.getLength());
        rStrm << nNameLen;
        rStrm.WriteZeroBytes(10);
        rStrm << XclExpString(aFinalName, EXC_STR_NOHEADER);
    }
    else
    {
        rStrm << sal_uInt16(0xFFFF);
        rStrm.WriteZeroBytes(8);
    }
    return rStrm;
}

// Page field settings ========================================================

XclPTPageFieldInfo::XclPTPageFieldInfo() :
    mnField( 0 ),
    mnSelItem( EXC_SXPI_ALLITEMS ),
    mnObjId( 0xFFFF )
{
}

XclImpStream& operator>>( XclImpStream& rStrm, XclPTPageFieldInfo& rInfo )
{
    rInfo.mnField = rStrm.ReaduInt16();
    rInfo.mnSelItem = rStrm.ReaduInt16();
    rInfo.mnObjId = rStrm.ReaduInt16();
    return rStrm;
}

XclExpStream& operator<<( XclExpStream& rStrm, const XclPTPageFieldInfo& rInfo )
{
    return rStrm
        << rInfo.mnField
        << rInfo.mnSelItem
        << rInfo.mnObjId;
}

// Data field settings ========================================================

XclPTDataFieldInfo::XclPTDataFieldInfo() :
    mnField( 0 ),
    mnAggFunc( EXC_SXDI_FUNC_SUM ),
    mnRefType( EXC_SXDI_REF_NORMAL ),
    mnRefField( 0 ),
    mnRefItem( 0 ),
    mnNumFmt( 0 )
{
}

GeneralFunction XclPTDataFieldInfo::GetApiAggFunc() const
{
    using namespace ::com::sun::star::sheet;
    GeneralFunction eAggFunc;
    switch( mnAggFunc )
    {
        case EXC_SXDI_FUNC_SUM:         eAggFunc = GeneralFunction_SUM;         break;
        case EXC_SXDI_FUNC_COUNT:       eAggFunc = GeneralFunction_COUNT;       break;
        case EXC_SXDI_FUNC_AVERAGE:     eAggFunc = GeneralFunction_AVERAGE;     break;
        case EXC_SXDI_FUNC_MAX:         eAggFunc = GeneralFunction_MAX;         break;
        case EXC_SXDI_FUNC_MIN:         eAggFunc = GeneralFunction_MIN;         break;
        case EXC_SXDI_FUNC_PRODUCT:     eAggFunc = GeneralFunction_PRODUCT;     break;
        case EXC_SXDI_FUNC_COUNTNUM:    eAggFunc = GeneralFunction_COUNTNUMS;   break;
        case EXC_SXDI_FUNC_STDDEV:      eAggFunc = GeneralFunction_STDEV;       break;
        case EXC_SXDI_FUNC_STDDEVP:     eAggFunc = GeneralFunction_STDEVP;      break;
        case EXC_SXDI_FUNC_VAR:         eAggFunc = GeneralFunction_VAR;         break;
        case EXC_SXDI_FUNC_VARP:        eAggFunc = GeneralFunction_VARP;        break;
        default:                        eAggFunc = GeneralFunction_SUM;
    }
    return eAggFunc;
}

void XclPTDataFieldInfo::SetApiAggFunc( GeneralFunction eAggFunc )
{
    using namespace ::com::sun::star::sheet;
    switch( eAggFunc )
    {
        case GeneralFunction_SUM:       mnAggFunc = EXC_SXDI_FUNC_SUM;      break;
        case GeneralFunction_COUNT:     mnAggFunc = EXC_SXDI_FUNC_COUNT;    break;
        case GeneralFunction_AVERAGE:   mnAggFunc = EXC_SXDI_FUNC_AVERAGE;  break;
        case GeneralFunction_MAX:       mnAggFunc = EXC_SXDI_FUNC_MAX;      break;
        case GeneralFunction_MIN:       mnAggFunc = EXC_SXDI_FUNC_MIN;      break;
        case GeneralFunction_PRODUCT:   mnAggFunc = EXC_SXDI_FUNC_PRODUCT;  break;
        case GeneralFunction_COUNTNUMS: mnAggFunc = EXC_SXDI_FUNC_COUNTNUM; break;
        case GeneralFunction_STDEV:     mnAggFunc = EXC_SXDI_FUNC_STDDEV;   break;
        case GeneralFunction_STDEVP:    mnAggFunc = EXC_SXDI_FUNC_STDDEVP;  break;
        case GeneralFunction_VAR:       mnAggFunc = EXC_SXDI_FUNC_VAR;      break;
        case GeneralFunction_VARP:      mnAggFunc = EXC_SXDI_FUNC_VARP;     break;
        default:                        mnAggFunc = EXC_SXDI_FUNC_SUM;
    }
}

sal_Int32 XclPTDataFieldInfo::GetApiRefType() const
{
    namespace ScDPRefType = ::com::sun::star::sheet::DataPilotFieldReferenceType;
    sal_Int32 nRefType;
    switch( mnRefType )
    {
        case EXC_SXDI_REF_DIFF:         nRefType = ScDPRefType::ITEM_DIFFERENCE;            break;
        case EXC_SXDI_REF_PERC:         nRefType = ScDPRefType::ITEM_PERCENTAGE;            break;
        case EXC_SXDI_REF_PERC_DIFF:    nRefType = ScDPRefType::ITEM_PERCENTAGE_DIFFERENCE; break;
        case EXC_SXDI_REF_RUN_TOTAL:    nRefType = ScDPRefType::RUNNING_TOTAL;              break;
        case EXC_SXDI_REF_PERC_ROW:     nRefType = ScDPRefType::ROW_PERCENTAGE;             break;
        case EXC_SXDI_REF_PERC_COL:     nRefType = ScDPRefType::COLUMN_PERCENTAGE;          break;
        case EXC_SXDI_REF_PERC_TOTAL:   nRefType = ScDPRefType::TOTAL_PERCENTAGE;           break;
        case EXC_SXDI_REF_INDEX:        nRefType = ScDPRefType::INDEX;                      break;
        default:                        nRefType = ScDPRefType::NONE;
    }
    return nRefType;
}

void XclPTDataFieldInfo::SetApiRefType( sal_Int32 nRefType )
{
    namespace ScDPRefType = ::com::sun::star::sheet::DataPilotFieldReferenceType;
    switch( nRefType )
    {
        case ScDPRefType::ITEM_DIFFERENCE:              mnRefType = EXC_SXDI_REF_DIFF;      break;
        case ScDPRefType::ITEM_PERCENTAGE:              mnRefType = EXC_SXDI_REF_PERC;      break;
        case ScDPRefType::ITEM_PERCENTAGE_DIFFERENCE:   mnRefType = EXC_SXDI_REF_PERC_DIFF; break;
        case ScDPRefType::RUNNING_TOTAL:                mnRefType = EXC_SXDI_REF_RUN_TOTAL; break;
        case ScDPRefType::ROW_PERCENTAGE:               mnRefType = EXC_SXDI_REF_PERC_ROW;  break;
        case ScDPRefType::COLUMN_PERCENTAGE:            mnRefType = EXC_SXDI_REF_PERC_COL;  break;
        case ScDPRefType::TOTAL_PERCENTAGE:             mnRefType = EXC_SXDI_REF_PERC_TOTAL;break;
        case ScDPRefType::INDEX:                        mnRefType = EXC_SXDI_REF_INDEX;     break;
        default:                                        mnRefType = EXC_SXDI_REF_NORMAL;
    }
}

sal_Int32 XclPTDataFieldInfo::GetApiRefItemType() const
{
    sal_Int32 nRefItemType;
    switch( mnRefItem )
    {
        case EXC_SXDI_PREVITEM: nRefItemType = ScDPRefItemType::PREVIOUS;   break;
        case EXC_SXDI_NEXTITEM: nRefItemType = ScDPRefItemType::NEXT;       break;
        default:                nRefItemType = ScDPRefItemType::NAMED;
    }
    return nRefItemType;
}

void XclPTDataFieldInfo::SetApiRefItemType( sal_Int32 nRefItemType )
{
    switch( nRefItemType )
    {
        case ScDPRefItemType::PREVIOUS: mnRefItem = EXC_SXDI_PREVITEM;  break;
        case ScDPRefItemType::NEXT:     mnRefItem = EXC_SXDI_NEXTITEM;  break;
        // nothing for named item reference
    }
}

XclImpStream& operator>>( XclImpStream& rStrm, XclPTDataFieldInfo& rInfo )
{
    rInfo.mnField = rStrm.ReaduInt16();
    rInfo.mnAggFunc = rStrm.ReaduInt16();
    rInfo.mnRefType = rStrm.ReaduInt16();
    rInfo.mnRefField = rStrm.ReaduInt16();
    rInfo.mnRefItem = rStrm.ReaduInt16();
    rInfo.mnNumFmt = rStrm.ReaduInt16();
    rStrm >> rInfo.maVisName;
    return rStrm;
}

XclExpStream& operator<<( XclExpStream& rStrm, const XclPTDataFieldInfo& rInfo )
{
    return rStrm
        << rInfo.mnField
        << rInfo.mnAggFunc
        << rInfo.mnRefType
        << rInfo.mnRefField
        << rInfo.mnRefItem
        << rInfo.mnNumFmt
        << rInfo.maVisName;
}

// Pivot table settings =======================================================

XclPTInfo::XclPTInfo() :
    mnFirstHeadRow( 0 ),
    mnCacheIdx( 0xFFFF ),
    mnDataAxis( EXC_SXVD_AXIS_NONE ),
    mnDataPos( EXC_SXVIEW_DATALAST ),
    mnFields( 0 ),
    mnRowFields( 0 ),
    mnColFields( 0 ),
    mnPageFields( 0 ),
    mnDataFields( 0 ),
    mnDataRows( 0 ),
    mnDataCols( 0 ),
    mnFlags( EXC_SXVIEW_DEFAULTFLAGS ),
    mnAutoFmtIdx( EXC_SXVIEW_AUTOFMT )
{
}

XclImpStream& operator>>( XclImpStream& rStrm, XclPTInfo& rInfo )
{
    sal_uInt16 nTabLen, nDataLen;

    rStrm   >> rInfo.maOutXclRange;
    rInfo.mnFirstHeadRow = rStrm.ReaduInt16();
    rStrm   >> rInfo.maDataXclPos;
    rInfo.mnCacheIdx = rStrm.ReaduInt16();
    rStrm.Ignore( 2 );
    rInfo.mnDataAxis = rStrm.ReaduInt16();
    rInfo.mnDataPos = rStrm.ReaduInt16();
    rInfo.mnFields = rStrm.ReaduInt16();
    rInfo.mnRowFields = rStrm.ReaduInt16();
    rInfo.mnColFields = rStrm.ReaduInt16();
    rInfo.mnPageFields = rStrm.ReaduInt16();
    rInfo.mnDataFields = rStrm.ReaduInt16();
    rInfo.mnDataRows = rStrm.ReaduInt16();
    rInfo.mnDataCols = rStrm.ReaduInt16();
    rInfo.mnFlags = rStrm.ReaduInt16();
    rInfo.mnAutoFmtIdx = rStrm.ReaduInt16();
    nTabLen = rStrm.ReaduInt16();
    nDataLen = rStrm.ReaduInt16();
    rInfo.maTableName = rStrm.ReadUniString( nTabLen );
    rInfo.maDataName = rStrm.ReadUniString( nDataLen );
    return rStrm;
}

XclExpStream& operator<<( XclExpStream& rStrm, const XclPTInfo& rInfo )
{
    XclExpString aXclTableName( rInfo.maTableName );
    XclExpString aXclDataName( rInfo.maDataName );

    rStrm   << rInfo.maOutXclRange
            << rInfo.mnFirstHeadRow
            << rInfo.maDataXclPos
            << rInfo.mnCacheIdx
            << sal_uInt16( 0 )
            << rInfo.mnDataAxis << rInfo.mnDataPos
            << rInfo.mnFields
            << rInfo.mnRowFields << rInfo.mnColFields
            << rInfo.mnPageFields << rInfo.mnDataFields
            << rInfo.mnDataRows << rInfo.mnDataCols
            << rInfo.mnFlags
            << rInfo.mnAutoFmtIdx
            << aXclTableName.Len() << aXclDataName.Len();
    aXclTableName.WriteFlagField( rStrm );
    aXclTableName.WriteBuffer( rStrm );
    aXclDataName.WriteFlagField( rStrm );
    aXclDataName.WriteBuffer( rStrm );
    return rStrm;
}

// Extended pivot table settings ==============================================

XclPTExtInfo::XclPTExtInfo() :
    mnSxformulaRecs( 0 ),
    mnSxselectRecs( 0 ),
    mnPagePerRow( 0 ),
    mnPagePerCol( 0 ),
    mnFlags( EXC_SXEX_DEFAULTFLAGS )
{
}

XclImpStream& operator>>( XclImpStream& rStrm, XclPTExtInfo& rInfo )
{
    rInfo.mnSxformulaRecs = rStrm.ReaduInt16();
    rStrm.Ignore( 6 );
    rInfo.mnSxselectRecs = rStrm.ReaduInt16();
    rInfo.mnPagePerRow = rStrm.ReaduInt16();
    rInfo.mnPagePerCol = rStrm.ReaduInt16();
    rInfo.mnFlags = rStrm.ReaduInt32();
    return rStrm;
}

XclExpStream& operator<<( XclExpStream& rStrm, const XclPTExtInfo& rInfo )
{
    return rStrm
        << rInfo.mnSxformulaRecs
        << EXC_PT_NOSTRING              // length of alt. error text
        << EXC_PT_NOSTRING              // length of alt. empty text
        << EXC_PT_NOSTRING              // length of tag
        << rInfo.mnSxselectRecs
        << rInfo.mnPagePerRow
        << rInfo.mnPagePerCol
        << rInfo.mnFlags
        << EXC_PT_NOSTRING              // length of page field style name
        << EXC_PT_NOSTRING              // length of table style name
        << EXC_PT_NOSTRING;             // length of vacate style name
}

// Pivot table autoformat settings ============================================

/**
classic     : 10 08 00 00 00 00 00 00 20 00 00 00 01 00 00 00 00
default     : 10 08 00 00 00 00 00 00 20 00 00 00 01 00 00 00 00
report01    : 10 08 02 00 00 00 00 00 20 00 00 00 00 10 00 00 00
report02    : 10 08 02 00 00 00 00 00 20 00 00 00 01 10 00 00 00
report03    : 10 08 02 00 00 00 00 00 20 00 00 00 02 10 00 00 00
report04    : 10 08 02 00 00 00 00 00 20 00 00 00 03 10 00 00 00
report05    : 10 08 02 00 00 00 00 00 20 00 00 00 04 10 00 00 00
report06    : 10 08 02 00 00 00 00 00 20 00 00 00 05 10 00 00 00
report07    : 10 08 02 00 00 00 00 00 20 00 00 00 06 10 00 00 00
report08    : 10 08 02 00 00 00 00 00 20 00 00 00 07 10 00 00 00
report09    : 10 08 02 00 00 00 00 00 20 00 00 00 08 10 00 00 00
report10    : 10 08 02 00 00 00 00 00 20 00 00 00 09 10 00 00 00
table01     : 10 08 00 00 00 00 00 00 20 00 00 00 0a 10 00 00 00
table02     : 10 08 00 00 00 00 00 00 20 00 00 00 0b 10 00 00 00
table03     : 10 08 00 00 00 00 00 00 20 00 00 00 0c 10 00 00 00
table04     : 10 08 00 00 00 00 00 00 20 00 00 00 0d 10 00 00 00
table05     : 10 08 00 00 00 00 00 00 20 00 00 00 0e 10 00 00 00
table06     : 10 08 00 00 00 00 00 00 20 00 00 00 0f 10 00 00 00
table07     : 10 08 00 00 00 00 00 00 20 00 00 00 10 10 00 00 00
table08     : 10 08 00 00 00 00 00 00 20 00 00 00 11 10 00 00 00
table09     : 10 08 00 00 00 00 00 00 20 00 00 00 12 10 00 00 00
table10     : 10 08 00 00 00 00 00 00 20 00 00 00 13 10 00 00 00
none        : 10 08 00 00 00 00 00 00 20 00 00 00 15 10 00 00 00
**/

XclPTViewEx9Info::XclPTViewEx9Info() :
    mbReport( 0 ),
    mnAutoFormat( 0 ),
    mnGridLayout( 0x10 )
{
}

void XclPTViewEx9Info::Init( const ScDPObject& rDPObj )
{
    if( rDPObj.GetHeaderLayout() )
    {
        mbReport     = 0;
        mnAutoFormat = 1;
        mnGridLayout = 0;
    }
    else
    {
        // Report1 for now
        // TODO : sync with autoformat indices
        mbReport     = 2;
        mnAutoFormat = 1;
        mnGridLayout = 0x10;
    }

    const ScDPSaveData* pData = rDPObj.GetSaveData();
    if (pData)
    {
        const OUString* pGrandTotal = pData->GetGrandTotalName();
        if (pGrandTotal)
            maGrandTotalName = *pGrandTotal;
    }
}

XclImpStream& operator>>( XclImpStream& rStrm, XclPTViewEx9Info& rInfo )
{
    rStrm.Ignore( 2 );
    rInfo.mbReport = rStrm.ReaduInt32();            /// 2 for report* fmts ?
    rStrm.Ignore( 6 );
    rInfo.mnAutoFormat = rStrm.ReaduInt8();
    rInfo.mnGridLayout = rStrm.ReaduInt8();
    rInfo.maGrandTotalName = rStrm.ReadUniString();
    return rStrm;
}

XclExpStream& operator<<( XclExpStream& rStrm, const XclPTViewEx9Info& rInfo )
{
    return rStrm
        << EXC_PT_AUTOFMT_HEADER
        << rInfo.mbReport
        << EXC_PT_AUTOFMT_ZERO
        << EXC_PT_AUTOFMT_FLAGS
        << rInfo.mnAutoFormat
        << rInfo.mnGridLayout
        << XclExpString(rInfo.maGrandTotalName, EXC_STR_DEFAULT, EXC_PT_MAXSTRLEN);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
