/*************************************************************************
 *
 *  $RCSfile: xlpivot.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-03 11:33:22 $
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

// ============================================================================

#ifndef SC_XLPIVOT_HXX
#include "xlpivot.hxx"
#endif

#ifndef SC_DPGROUP_HXX
#include "dpgroup.hxx"
#endif

#ifndef SC_XISTREAM_HXX
#include "xistream.hxx"
#endif
#ifndef SC_XESTREAM_HXX
#include "xestream.hxx"
#endif
#ifndef SC_XESTRING_HXX
#include "xestring.hxx"
#endif

using ::com::sun::star::sheet::GeneralFunction;
using ::com::sun::star::sheet::DataPilotFieldOrientation;

namespace ScDPSortMode = ::com::sun::star::sheet::DataPilotFieldSortMode;
namespace ScDPShowItemsMode = ::com::sun::star::sheet::DataPilotFieldShowItemsMode;
namespace ScDPLayoutMode = ::com::sun::star::sheet::DataPilotFieldLayoutMode;
namespace ScDPRefItemType = ::com::sun::star::sheet::DataPilotFieldReferenceItemType;

// ============================================================================
// Pivot cache
// ============================================================================

XclPCItem::XclPCItem() :
    meType( EXC_PCITEM_INVALID ),
    mfValue( 0.0 ),
    mnValue( 0 ),
    mnError( EXC_ERR_NULL ),
    mbValue( false )
{
}

void XclPCItem::SetEmpty()
{
    meType = EXC_PCITEM_EMPTY;
    maText.Erase();
    mfValue = 0.0;
    mnValue = 0;
    mnError = EXC_ERR_NULL;
    mbValue = false;
}

void XclPCItem::SetText( const String& rText )
{
    meType = EXC_PCITEM_TEXT;
    maText = rText;
    //! TODO convert string to double
    mfValue = 0.0;
    mnValue = 0;
    mnError = EXC_ERR_NULL;
    mbValue = rText.Len() != 0;
}

void XclPCItem::SetDouble( double fValue )
{
    meType = EXC_PCITEM_VALUE;
    //! TODO convert double to string
    maText.Erase();
    mfValue = fValue;
    mnValue = ::lulimit< sal_Int16 >( fValue );
    mnError = EXC_ERR_NULL;
    mbValue = fValue != 0.0;
}

void XclPCItem::SetInteger( sal_Int16 nValue )
{
    meType = EXC_PCITEM_INTEGER;
    maText = String::CreateFromInt32( nValue );
    mfValue = nValue;
    mnValue = nValue;
    mnError = EXC_ERR_NULL;
    mbValue = nValue != 0;
}

void XclPCItem::SetDate( double fDate )
{
    meType = EXC_PCITEM_DATE;
    //! TODO convert date to string
    maText.Erase();
    mfValue = fDate;
    mnValue = ::lulimit< sal_Int16 >( fDate );
    mnError = EXC_ERR_NULL;
    mbValue = fDate != 0.0;
}

void XclPCItem::SetError( sal_uInt16 nError )
{
    meType = EXC_PCITEM_ERROR;
    //! TODO convert error to string
    maText.Erase();
    mfValue = nError;
    mnValue = ::lulimit< sal_Int16 >( nError );
    mnError = nError;
    mbValue = false;
}

void XclPCItem::SetBool( bool bValue )
{
    meType = EXC_PCITEM_BOOL;
    //! TODO convert boolean to string
    maText.Erase();
    mfValue = bValue ? 1.0 : 0.0;
    mnValue = bValue ? 1 : 0;
    mnError = EXC_ERR_NULL;
    mbValue = bValue;
}

// ----------------------------------------------------------------------------

bool XclPCItem::IsEmpty() const
{
    return meType == EXC_PCITEM_EMPTY;
}

const String* XclPCItem::GetText() const
{
    return (meType == EXC_PCITEM_TEXT) ? &maText : 0;
}

const double* XclPCItem::GetDouble() const
{
    return (meType == EXC_PCITEM_VALUE) ? &mfValue : 0;
}

const sal_Int16* XclPCItem::GetInteger() const
{
    return (meType == EXC_PCITEM_INTEGER) ? &mnValue : 0;
}

const double* XclPCItem::GetDate() const
{
    return (meType == EXC_PCITEM_DATE) ? &mfValue : 0;
}

const sal_uInt16* XclPCItem::GetError() const
{
    return (meType == EXC_PCITEM_ERROR) ? &mnError : 0;
}

const bool* XclPCItem::GetBool() const
{
    return (meType == EXC_PCITEM_BOOL) ? &mbValue : 0;
}

// ----------------------------------------------------------------------------

bool operator==( const XclPCItem& rLeft, const XclPCItem& rRight )
{
    if( rLeft.GetType() != rRight.GetType() )
        return false;
    switch( rLeft.GetType() )
    {
        case EXC_PCITEM_INVALID:    return true;
        case EXC_PCITEM_EMPTY:      return true;
        case EXC_PCITEM_TEXT:       return rLeft.ConvertToText()    == rRight.ConvertToText();
        case EXC_PCITEM_VALUE:      return rLeft.ConvertToDouble()  == rRight.ConvertToDouble();
        case EXC_PCITEM_INTEGER:    return rLeft.ConvertToInteger() == rRight.ConvertToInteger();
        case EXC_PCITEM_DATE:       return rLeft.ConvertToDate()    == rRight.ConvertToDate();
        case EXC_PCITEM_BOOL:       return rLeft.ConvertToBool()    == rRight.ConvertToBool();
        case EXC_PCITEM_ERROR:      return rLeft.ConvertToError()   == rRight.ConvertToError();
        default:    DBG_ERRORFILE( "operator== - unknown pivot cache item type" );
    }
    return false;
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
    rStrm   >> rInfo.mnFlags
            >> rInfo.mnGroupChild
            >> rInfo.mnGroupBase
            >> rInfo.mnVisItems
            >> rInfo.mnGroupItems
            >> rInfo.mnBaseItems
            >> rInfo.mnOrigItems;
    if( rStrm.GetRecLeft() >= 3 )
        rInfo.maName = rStrm.ReadUniString();
    else
        rInfo.maName.Erase();
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

bool XclPCNumGroupInfo::IsNumType() const
{
    return GetXclDataType() == EXC_SXNUMGROUP_TYPE_NUM;
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
        case EXC_SXNUMGROUP_TYPE_SEC:   nScType = SC_DP_DATE_SECONDS;   break;
        case EXC_SXNUMGROUP_TYPE_MIN:   nScType = SC_DP_DATE_MINUTES;   break;
        case EXC_SXNUMGROUP_TYPE_HOUR:  nScType = SC_DP_DATE_HOURS;     break;
        case EXC_SXNUMGROUP_TYPE_DAY:   nScType = SC_DP_DATE_DAYS;      break;
        case EXC_SXNUMGROUP_TYPE_MONTH: nScType = SC_DP_DATE_MONTHS;    break;
        case EXC_SXNUMGROUP_TYPE_QUART: nScType = SC_DP_DATE_QUARTERS;  break;
        case EXC_SXNUMGROUP_TYPE_YEAR:  nScType = SC_DP_DATE_YEARS;     break;
        default:    DBG_ERROR1( "XclPCNumGroupInfo::GetScDateType - unexpected date type %d", GetXclDataType() );
    }
    return nScType;
}

void XclPCNumGroupInfo::SetScDateType( sal_Int32 nScType )
{
    sal_uInt16 nXclType = EXC_SXNUMGROUP_TYPE_NUM;
    switch( nScType )
    {
        case SC_DP_DATE_SECONDS:    nXclType = EXC_SXNUMGROUP_TYPE_SEC;     break;
        case SC_DP_DATE_MINUTES:    nXclType = EXC_SXNUMGROUP_TYPE_MIN;     break;
        case SC_DP_DATE_HOURS:      nXclType = EXC_SXNUMGROUP_TYPE_HOUR;    break;
        case SC_DP_DATE_DAYS:       nXclType = EXC_SXNUMGROUP_TYPE_DAY;     break;
        case SC_DP_DATE_MONTHS:     nXclType = EXC_SXNUMGROUP_TYPE_MONTH;   break;
        case SC_DP_DATE_QUARTERS:   nXclType = EXC_SXNUMGROUP_TYPE_QUART;   break;
        case SC_DP_DATE_YEARS:      nXclType = EXC_SXNUMGROUP_TYPE_YEAR;    break;
        default:    DBG_ERROR1( "XclPCNumGroupInfo::SetScDateType - unexpected date type %d", nScType );
    }
    SetXclDataType( nXclType );
}

sal_uInt16 XclPCNumGroupInfo::GetXclDataType() const
{
    sal_uInt16 nXclType = 0;
    ::extract_value( nXclType, mnFlags, 2, 4 );
    return nXclType;
}

void XclPCNumGroupInfo::SetXclDataType( sal_uInt16 nXclType )
{
    ::insert_value( mnFlags, nXclType, 2, 4 );
}

XclImpStream& operator>>( XclImpStream& rStrm, XclPCNumGroupInfo& rInfo )
{
    return rStrm >> rInfo.mnFlags;
}

XclExpStream& operator<<( XclExpStream& rStrm, const XclPCNumGroupInfo& rInfo )
{
    return rStrm << rInfo.mnFlags;
}

// Base class for pivot cache fields ==========================================

XclPCField::XclPCField( XclPCFieldType eFieldType, sal_uInt16 nFieldIdx ) :
    mnFieldIdx( nFieldIdx ),
    meFieldType( eFieldType )
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

bool XclPCField::IsCalculatedField() const
{
    return meFieldType == EXC_PCFIELD_CALCED;
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

sal_uInt16 XclPCField::GetBaseFieldIndex() const
{
    return IsGroupChildField() ? maFieldInfo.mnGroupBase : mnFieldIdx;
}

bool XclPCField::HasPostponedItems() const
{
    return ::get_flag( maFieldInfo.mnFlags, EXC_SXFIELD_POSTPONE );
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
    rStrm   >> rInfo.mnSrcRecs
            >> rInfo.mnStrmId
            >> rInfo.mnFlags
            >> rInfo.mnBlockRecs
            >> rInfo.mnStdFields
            >> rInfo.mnTotalFields;
    rStrm.Ignore( 2 );
    rStrm   >> rInfo.mnSrcType;
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

// ============================================================================
// Pivot table
// ============================================================================

// cached name ================================================================

XclImpStream& operator>>( XclImpStream& rStrm, XclPTCachedName& rCachedName )
{
    sal_uInt16 nStrLen;
    rStrm >> nStrLen;
    rCachedName.mbUseCache = nStrLen == EXC_PT_NOSTRING;
    if( rCachedName.mbUseCache )
        rCachedName.maName.Erase();
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

// ----------------------------------------------------------------------------

const String* XclPTVisNameInfo::GetVisName() const
{
    return HasVisName() ? &maVisName.maName : 0;
}

void XclPTVisNameInfo::SetVisName( const String& rName )
{
    maVisName.maName = rName;
    maVisName.mbUseCache = rName.Len() == 0;
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
    return rStrm
        >> rInfo.mnType
        >> rInfo.mnFlags
        >> rInfo.mnCacheIdx
        >> rInfo.maVisName;
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
    }
}

//! TODO: should be a Sequence<GeneralFunction> in ScDPSaveData
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
    return rStrm
        >> rInfo.mnAxes
        >> rInfo.mnSubtCount
        >> rInfo.mnSubtotals
        >> rInfo.mnItemCount
        >> rInfo.maVisName;
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
    mnShowField( EXC_SXVDEX_SHOW_NONE )
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
    sal_uInt8 nShowCount;
    ::extract_value( nShowCount, mnFlags, 24, 8 );
    return nShowCount;
}

void XclPTFieldExtInfo::SetApiAutoShowCount( sal_Int32 nShowCount )
{
    ::insert_value( mnFlags, ::lulimit< sal_uInt8 >( nShowCount ), 24, 8 );
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
    return rStrm
        >> rInfo.mnFlags
        >> rInfo.mnSortField
        >> rInfo.mnShowField;
}

XclExpStream& operator<<( XclExpStream& rStrm, const XclPTFieldExtInfo& rInfo )
{
    rStrm   << rInfo.mnFlags
            << rInfo.mnSortField
            << rInfo.mnShowField
            << EXC_SXVDEX_FORMAT_NONE;
    rStrm.WriteZeroBytes( 10 );                 // unknown
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
    return rStrm
        >> rInfo.mnField
        >> rInfo.mnSelItem
        >> rInfo.mnObjId;
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
    return rStrm
        >> rInfo.mnField
        >> rInfo.mnAggFunc
        >> rInfo.mnRefType
        >> rInfo.mnRefField
        >> rInfo.mnRefItem
        >> rInfo.mnNumFmt
        >> rInfo.maVisName;
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
    mnFirstRow( 0 ),
    mnLastRow( 0 ),
    mnFirstCol( 0 ),
    mnLastCol( 0 ),
    mnFirstHeadRow( 0 ),
    mnFirstDataRow( 0 ),
    mnFirstDataCol( 0 ),
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

    rStrm   >> rInfo.mnFirstRow >> rInfo.mnLastRow
            >> rInfo.mnFirstCol >> rInfo.mnLastCol
            >> rInfo.mnFirstHeadRow
            >> rInfo.mnFirstDataRow >> rInfo.mnFirstDataCol
            >> rInfo.mnCacheIdx;
    rStrm.Ignore( 2 );
    rStrm   >> rInfo.mnDataAxis >> rInfo.mnDataPos
            >> rInfo.mnFields
            >> rInfo.mnRowFields >> rInfo.mnColFields
            >> rInfo.mnPageFields >> rInfo.mnDataFields
            >> rInfo.mnDataRows >> rInfo.mnDataCols
            >> rInfo.mnFlags
            >> rInfo.mnAutoFmtIdx
            >> nTabLen >> nDataLen;
    rInfo.maTableName = rStrm.ReadUniString( nTabLen );
    rInfo.maDataName = rStrm.ReadUniString( nDataLen );
    return rStrm;
}

XclExpStream& operator<<( XclExpStream& rStrm, const XclPTInfo& rInfo )
{
    XclExpString aXclTableName( rInfo.maTableName );
    XclExpString aXclDataName( rInfo.maDataName );

    rStrm   << rInfo.mnFirstRow << rInfo.mnLastRow
            << rInfo.mnFirstCol << rInfo.mnLastCol
            << rInfo.mnFirstHeadRow
            << rInfo.mnFirstDataRow << rInfo.mnFirstDataCol
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
    rStrm >> rInfo.mnSxformulaRecs;
    rStrm.Ignore( 6 );
    return rStrm
        >> rInfo.mnSxselectRecs
        >> rInfo.mnPagePerRow
        >> rInfo.mnPagePerCol
        >> rInfo.mnFlags;
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

// ============================================================================

