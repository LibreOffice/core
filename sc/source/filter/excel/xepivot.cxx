/*************************************************************************
 *
 *  $RCSfile: xepivot.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-11 09:52:39 $
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

#ifndef SC_XEPIVOT_HXX
#include "xepivot.hxx"
#endif

#ifndef _COM_SUN_STAR_SHEET_DATAPILOTFIELDSORTINFO_HPP_
#include <com/sun/star/sheet/DataPilotFieldSortInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_DATAPILOTFIELDAUTOSHOWINFO_HPP_
#include <com/sun/star/sheet/DataPilotFieldAutoShowInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_DATAPILOTFIELDLAYOUTINFO_HPP_
#include <com/sun/star/sheet/DataPilotFieldLayoutInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_DATAPILOTFIELDREFERENCE_HPP_
#include <com/sun/star/sheet/DataPilotFieldReference.hpp>
#endif

#include <math.h>

#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif
#ifndef _DATE_HXX
#include <tools/date.hxx>
#endif
#ifndef _ZFORMAT_HXX
#include <svtools/zformat.hxx>
#endif
#ifndef _SVSTOR_HXX
#include <so3/svstor.hxx>
#endif

#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif
#ifndef SC_DPOBJECT_HXX
#include "dpobject.hxx"
#endif
#ifndef SC_DPSAVE_HXX
#include "dpsave.hxx"
#endif
#ifndef SC_DPDIMSAVE_HXX
#include "dpdimsave.hxx"
#endif
#ifndef SC_DPSHTTAB_HXX
#include "dpshttab.hxx"
#endif
#ifndef __GLOBSTR_HRC_
#include "globstr.hrc"
#endif

#ifndef SC_FAPIHELPER_HXX
#include "fapihelper.hxx"
#endif
#ifndef SC_XESTRING_HXX
#include "xestring.hxx"
#endif
#ifndef SC_XELINK_HXX
#include "xelink.hxx"
#endif

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

// ============================================================================
// Pivot cache
// ============================================================================

namespace {

// constants to track occurence of specific data types
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
    XclExpRecord( rText.Len() ? EXC_ID_SXSTRING : EXC_ID_SXEMPTY, 0 ),
    mnTypeFlag( EXC_PCITEM_DATA_STRING )
{
    if( rText.Len() )
        SetText( rText );
    else
        SetEmpty();
}

XclExpPCItem::XclExpPCItem( double fValue, bool bDate ) :
    XclExpRecord( bDate ? EXC_ID_SXDATETIME : EXC_ID_SXDOUBLE, 8 )
{
    if( bDate )
    {
        SetDate( fValue );
        mnTypeFlag = EXC_PCITEM_DATA_DATE;
    }
    else
    {
        SetDouble( fValue );
        mnTypeFlag = (fValue - floor( fValue ) == 0.0) ?
            EXC_PCITEM_DATA_INTEGER : EXC_PCITEM_DATA_DOUBLE;
    }
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

bool XclExpPCItem::EqualsText( const String& rText ) const
{
    return (rText.Len() && GetText() && (*GetText() == rText)) || (!rText.Len() && IsEmpty());
}

bool XclExpPCItem::EqualsDouble( double fValue ) const
{
    return GetDouble() && (*GetDouble() == fValue);
}

bool XclExpPCItem::EqualsDate( double fDate ) const
{
    return GetDate() && (*GetDate() == fDate);
}

bool XclExpPCItem::EqualsBool( bool bValue ) const
{
    return GetBool() && (*GetBool() == bValue);
}

// ----------------------------------------------------------------------------

void XclExpPCItem::WriteBody( XclExpStream& rStrm )
{
    if( const String* pText = GetText() )
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
    else if( const double* pfDate = GetDate() )
    {
        double fDays = ::rtl::math::approxFloor( *pfDate );
        double fTime = *pfDate - fDays;

        sal_uInt16 nYear = 0;
        sal_uInt16 nMonth = 0;
        sal_uInt8 nDay = 0;
        sal_uInt8 nHour = 0;
        sal_uInt8 nMin = 0;
        sal_uInt8 nSec = 0;

        if( fDays > 0.0 )
        {
            Date aDate( *rStrm.GetRoot().GetFormatter().GetNullDate() );
            aDate += static_cast< long >( fDays );

            nYear = static_cast< sal_uInt16 >( aDate.GetYear() );
            nMonth = static_cast< sal_uInt16 >( aDate.GetMonth() );
            nDay = static_cast< sal_uInt8 >( aDate.GetDay() );
        }

        if( fTime > 0.0 )
        {
            fTime *= 24;
            nHour = static_cast< sal_uInt8 >( fTime );
            fTime -= nHour;
            fTime *= 60;
            nMin = static_cast< sal_uInt8 >( fTime );
            fTime -= nMin;
            fTime *= 60;
            nSec = ::ulimit< sal_uInt8 >( fTime + 0.001, 59 );
        }

        rStrm << nYear << nMonth << nDay << nHour << nMin << nSec;
    }
    else if( const bool* pbValue = GetBool() )
    {
        rStrm << static_cast< sal_uInt16 >( *pbValue ? 1 : 0 );
    }
    else
    {
        // nothing to do for SXEMPTY
        DBG_ASSERT( IsEmpty(), "XclExpPCItem::WriteBody - no data found" );
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
                DBG_ASSERT( !rNumInfo.Enable || !rDateInfo.Enable,
                    "XclExpPCField::XclExpPCField - numeric and date grouping enabled" );

                if( rNumInfo.Enable )
                    InitNumGroupField( rDPObj, rNumInfo );
                else if( rDateInfo.Enable )
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
    DBG_ASSERT( rBaseField.GetFieldName() == rGroupDim.GetSourceDimName(),
        "XclExpPCField::FillFromGroup - wrong base cache field" );
    maFieldInfo.maName = rGroupDim.GetGroupDimName();
    maFieldInfo.mnGroupBase = rBaseField.GetFieldIndex();

    // add standard group info or date group info
    const ScDPNumGroupInfo& rDateInfo = rGroupDim.GetDateInfo();
    if( rDateInfo.Enable && (rGroupDim.GetDatePart() != 0) )
        InitDateGroupField( rDPObj, rDateInfo, rGroupDim.GetDatePart() );
    else
        InitStdGroupField( rBaseField, rGroupDim );

    // final settings (flags, item numbers)
    Finalize();
}

void XclExpPCField::SetGroupChildField( const XclExpPCField& rChildField )
{
    DBG_ASSERT( !::get_flag( maFieldInfo.mnFlags, EXC_SXFIELD_HASCHILD ),
        "XclExpPCField::SetGroupChildIndex - field already has a grouping child field" );
    ::set_flag( maFieldInfo.mnFlags, EXC_SXFIELD_HASCHILD );
    maFieldInfo.mnGroupChild = rChildField.GetFieldIndex();
}

sal_uInt16 XclExpPCField::GetItemCount() const
{
    return static_cast< sal_uInt16 >( GetVisItemList().Count() );
}

const XclExpPCItem* XclExpPCField::GetItem( sal_uInt16 nItemIdx ) const
{
    return GetVisItemList().GetObject( nItemIdx );
}

sal_uInt16 XclExpPCField::GetItemIndex( const String& rItemName ) const
{
    const XclExpPCItemList& rItemList = GetVisItemList();
    for( const XclExpPCItem* pItem = rItemList.First(); pItem; pItem = rItemList.Next() )
        if( pItem->ConvertToText() == rItemName )
            return static_cast< sal_uInt16 >( rItemList.GetCurPos() );
    return EXC_PC_NOITEM;
}

sal_uInt32 XclExpPCField::GetIndexSize() const
{
    return ::get_flag( maFieldInfo.mnFlags, EXC_SXFIELD_16BIT ) ? 2 : 1;
}

void XclExpPCField::WriteIndex( XclExpStream& rStrm, sal_uInt32 nSrcRow ) const
{
    // only standard fields write item indexes
    if( nSrcRow < maIndexVec.size() )
    {
        sal_uInt16 nIndex = maIndexVec[ nSrcRow ];
        if( ::get_flag( maFieldInfo.mnFlags, EXC_SXFIELD_16BIT ) )
            rStrm << nIndex;
        else
            rStrm << static_cast< sal_uInt8 >( nIndex );
    }
}

void XclExpPCField::Save( XclExpStream& rStrm )
{
    DBG_ASSERT( IsSupportedField(), "XclExpPCField::Save - unknown field type" );
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
    DBG_ASSERT( IsStandardField() == maGroupItemList.Empty(),
        "XclExpPCField::GetVisItemList - unexpected additional items in standard field" );
    return IsStandardField() ? maOrigItemList : maGroupItemList;
}

void XclExpPCField::InitStandardField( const ScRange& rRange )
{
    DBG_ASSERT( IsStandardField(), "XclExpPCField::InitStandardField - only for standard fields" );
    DBG_ASSERT( rRange.aStart.Col() == rRange.aEnd.Col(), "XclExpPCField::InitStandardField - cell range with multiple columns" );

    ScDocument& rDoc = GetDoc();
    SvNumberFormatter& rFormatter = GetFormatter();

    // field name is in top cell of the range
    ScAddress aPos( rRange.aStart );
    rDoc.GetString( aPos.Col(), aPos.Row(), aPos.Tab(), maFieldInfo.maName );

    // loop over all cells, create pivot cache items
    for( aPos.IncRow(); (aPos.Row() <= rRange.aEnd.Row()) && (maOrigItemList.Count() < EXC_PC_MAXITEMCOUNT); aPos.IncRow() )
    {
        if( rDoc.HasValueData( aPos.Col(), aPos.Row(), aPos.Tab() ) )
        {
            double fValue = rDoc.GetValue( aPos );
            short nFmtType = rFormatter.GetType( rDoc.GetNumberFormat( aPos ) );
            if( nFmtType == NUMBERFORMAT_LOGICAL )
                InsertOrigBoolItem( fValue != 0 );
            else if( nFmtType & NUMBERFORMAT_DATETIME )
                InsertOrigDateItem( fValue );
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
    DBG_ASSERT( IsGroupField(), "XclExpPCField::InitStdGroupField - only for standard grouping fields" );

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
                if( const String* pElemName = pGroupItem->GetElementByIndex( nElemIdx ) )
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
    DBG_ASSERT( IsStandardField(), "XclExpPCField::InitNumGroupField - only for standard fields" );
    DBG_ASSERT( rNumInfo.Enable, "XclExpPCField::InitNumGroupField - numeric grouping not enabled" );

    // new field type, date type, limit settings (min/max/step/auto)
    if( rNumInfo.DateValues )
    {
        // special case: group by days with step count
        meFieldType = EXC_PCFIELD_DATEGROUP;
        maNumGroupInfo.SetScDateType( SC_DP_DATE_DAYS );
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
    DBG_ASSERT( IsStandardField() || IsStdGroupField(), "XclExpPCField::InitDateGroupField - only for standard fields" );
    DBG_ASSERT( rDateInfo.Enable, "XclExpPCField::InitDateGroupField - date grouping not enabled" );

    // new field type
    meFieldType = IsStandardField() ? EXC_PCFIELD_DATEGROUP : EXC_PCFIELD_DATECHILD;

    // date type, limit settings (min/max/step/auto)
    maNumGroupInfo.SetScDateType( nDatePart );
    SetDateGroupLimit( rDateInfo, false );

    // generate visible items
    InsertNumDateGroupItems( rDPObj, rDateInfo, nDatePart );
}

void XclExpPCField::InsertItemArrayIndex( ULONG nListPos )
{
    DBG_ASSERT( IsStandardField(), "XclExpPCField::InsertItemArrayIndex - only for standard fields" );
    maIndexVec.push_back( static_cast< sal_uInt16 >( nListPos ) );
}

void XclExpPCField::InsertOrigItem( XclExpPCItem* pNewItem )
{
    sal_uInt16 nItemIdx = static_cast< sal_uInt16 >( maOrigItemList.Count() );
    maOrigItemList.Append( pNewItem );
    InsertItemArrayIndex( nItemIdx );
    mnTypeFlags |= pNewItem->GetTypeFlag();
}

void XclExpPCField::InsertOrigTextItem( const String& rText )
{
    bool bFound = false;
    for( const XclExpPCItem* pItem = maOrigItemList.First(); !bFound && pItem; pItem = maOrigItemList.Next() )
        bFound = pItem->EqualsText( rText );
    if( bFound )
        InsertItemArrayIndex( maOrigItemList.GetCurPos() );
    else
        InsertOrigItem( new XclExpPCItem( rText ) );
}

void XclExpPCField::InsertOrigDoubleItem( double fValue )
{
    bool bFound = false;
    for( const XclExpPCItem* pItem = maOrigItemList.First(); !bFound && pItem; pItem = maOrigItemList.Next() )
        bFound = pItem->EqualsDouble( fValue );
    if( bFound )
        InsertItemArrayIndex( maOrigItemList.GetCurPos() );
    else
        InsertOrigItem( new XclExpPCItem( fValue, false ) );
}

void XclExpPCField::InsertOrigDateItem( double fDate )
{
    bool bFound = false;
    for( const XclExpPCItem* pItem = maOrigItemList.First(); !bFound && pItem; pItem = maOrigItemList.Next() )
        bFound = pItem->EqualsDate( fDate );
    if( bFound )
        InsertItemArrayIndex( maOrigItemList.GetCurPos() );
    else
        InsertOrigItem( new XclExpPCItem( fDate, true ) );
}

void XclExpPCField::InsertOrigBoolItem( bool bValue )
{
    bool bFound = false;
    for( const XclExpPCItem* pItem = maOrigItemList.First(); !bFound && pItem; pItem = maOrigItemList.Next() )
        bFound = pItem->EqualsBool( bValue );
    if( bFound )
        InsertItemArrayIndex( maOrigItemList.GetCurPos() );
    else
        InsertOrigItem( new XclExpPCItem( bValue ) );
}

sal_uInt16 XclExpPCField::InsertGroupItem( XclExpPCItem* pNewItem )
{
    maGroupItemList.Append( pNewItem );
    return static_cast< sal_uInt16 >( maGroupItemList.Count() - 1 );
}

void XclExpPCField::InsertNumDateGroupItems( const ScDPObject& rDPObj, const ScDPNumGroupInfo& rNumInfo, sal_Int32 nDatePart )
{
    DBG_ASSERT( rDPObj.GetSheetDesc(), "XclExpPCField::InsertNumDateGroupItems - cannot generate element list" );
    if( const ScSheetSourceDesc* pSrcDesc = rDPObj.GetSheetDesc() )
    {
        // get the string collection with original source elements
        ScSheetDPData aDPData( GetDocPtr(), *pSrcDesc );
        const TypedStrCollection& rOrigColl = aDPData.GetColumnEntries( static_cast< long >( GetBaseFieldIndex() ) );

        // get the string collection with generated grouping elements
        ScDPNumGroupDimension aTmpDim( rNumInfo );
        if( nDatePart != 0 )
            aTmpDim.MakeDateHelper( rNumInfo, nDatePart );
        const TypedStrCollection& rGroupColl = aTmpDim.GetNumEntries( rOrigColl, GetDocPtr() );
        for( USHORT nIdx = 0, nCount = rGroupColl.GetCount(); nIdx < nCount; ++nIdx )
            if( const TypedStrData* pStrData = rGroupColl[ nIdx ] )
                maGroupItemList.Append( new XclExpPCItem( pStrData->GetString() ) );
    }
}

void XclExpPCField::SetNumGroupLimit( const ScDPNumGroupInfo& rNumInfo )
{
    ::set_flag( maNumGroupInfo.mnFlags, EXC_SXNUMGROUP_AUTOMIN, rNumInfo.AutoStart );
    ::set_flag( maNumGroupInfo.mnFlags, EXC_SXNUMGROUP_AUTOMAX, rNumInfo.AutoEnd );
    maNumGroupLimits.Append( new XclExpPCItem( rNumInfo.Start, false ) );
    maNumGroupLimits.Append( new XclExpPCItem( rNumInfo.End, false ) );
    maNumGroupLimits.Append( new XclExpPCItem( rNumInfo.Step, false ) );
}

void XclExpPCField::SetDateGroupLimit( const ScDPNumGroupInfo& rDateInfo, bool bUseStep )
{
    ::set_flag( maNumGroupInfo.mnFlags, EXC_SXNUMGROUP_AUTOMIN, rDateInfo.AutoStart );
    ::set_flag( maNumGroupInfo.mnFlags, EXC_SXNUMGROUP_AUTOMAX, rDateInfo.AutoEnd );
    maNumGroupLimits.Append( new XclExpPCItem( rDateInfo.Start, true ) );
    maNumGroupLimits.Append( new XclExpPCItem( rDateInfo.End, true ) );
    sal_Int16 nStep = bUseStep ? ::lulimit< sal_Int16 >( rDateInfo.Step, 1, SAL_MAX_INT16 ) : 1;
    maNumGroupLimits.Append( new XclExpPCItem( nStep ) );
}

void XclExpPCField::Finalize()
{
    // flags
    ::set_flag( maFieldInfo.mnFlags, EXC_SXFIELD_HASITEMS, !GetVisItemList().Empty() );
    // Excel writes long indexes even for 0x0100 items (indexes from 0x00 to 0xFF)
    ::set_flag( maFieldInfo.mnFlags, EXC_SXFIELD_16BIT, maOrigItemList.Count() >= 0x0100 );
    ::set_flag( maFieldInfo.mnFlags, EXC_SXFIELD_NUMGROUP, IsNumGroupField() || IsDateGroupField() );
    /*  mnTypeFlags is updated in all Insert***Item() functions. Now the flags
        for the current combination of item types is added to the flags. */
    ::set_flag( maFieldInfo.mnFlags, spnPCItemFlags[ mnTypeFlags ] );


    // item count fields
    maFieldInfo.mnVisItems = static_cast< sal_uInt16 >( GetVisItemList().Count() );
    maFieldInfo.mnGroupItems = static_cast< sal_uInt16 >( maGroupItemList.Count() );
    // maFieldInfo.mnBaseItems set in InitStdGroupField()
    maFieldInfo.mnOrigItems = static_cast< sal_uInt16 >( maOrigItemList.Count() );
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
        DBG_ASSERT( maNumGroupLimits.Count() == 3,
            "XclExpPCField::WriteSxnumgroup - missing numeric grouping limits" );
        maNumGroupLimits.Save( rStrm );
    }
}

void XclExpPCField::WriteSxgroupinfo( XclExpStream& rStrm )
{
    DBG_ASSERT( IsStdGroupField() != maGroupOrder.empty(),
        "XclExpPCField::WriteSxgroupinfo - missing grouping info" );
    if( IsStdGroupField() && !maGroupOrder.empty() )
    {
        rStrm.StartRecord( EXC_ID_SXGROUPINFO, static_cast< sal_uInt32 >( 2 * maGroupOrder.size() ) );
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
        maOrigSrcRange = maExpSrcRange = maDocSrcRange = pSrcDesc->aSourceRange;

        // internal sheet data only
        SCTAB nScTab = maExpSrcRange.aStart.Tab();
        if( (nScTab == maExpSrcRange.aEnd.Tab()) && GetTabInfo().IsExportTab( nScTab ) )
        {
            // CheckCellRange() restricts source range to valid Excel limits
            if( CheckCellRange( maExpSrcRange ) )
            {
                // #i22541# skip empty cell areas (performance)
                SCCOL nFirstDocCol, nLastDocCol;
                SCROW nFirstDocRow, nLastDocRow;
                GetDoc().GetDataStart( nScTab, nFirstDocCol, nFirstDocRow );
                GetDoc().GetPrintArea( nScTab, nLastDocCol, nLastDocRow, false );
                SCCOL nFirstSrcCol = maExpSrcRange.aStart.Col(), nLastSrcCol = maExpSrcRange.aEnd.Col();
                SCROW nFirstSrcRow = maExpSrcRange.aStart.Row(), nLastSrcRow = maExpSrcRange.aEnd.Row();

                // #i22541# do not store index list for too big ranges
                if( 2 * (nLastDocRow - nFirstDocRow) < (maExpSrcRange.aEnd.Row() - maExpSrcRange.aStart.Row()) )
                    ::set_flag( maPCInfo.mnFlags, EXC_SXDB_SAVEDATA, false );

                // adjust row indexes, keep one row of empty area to surely have the empty cache item
                if( nFirstSrcRow < nFirstDocRow )
                    nFirstSrcRow = nFirstDocRow - 1;
                if( nLastSrcRow > nLastDocRow )
                    nLastSrcRow = nLastDocRow + 1;

                maDocSrcRange.aStart.SetCol( ::std::max( nFirstDocCol, nFirstSrcCol ) );
                maDocSrcRange.aStart.SetRow( nFirstSrcRow );
                maDocSrcRange.aEnd.SetCol( ::std::min( nLastDocCol, nLastSrcCol ) );
                maDocSrcRange.aEnd.SetRow( nLastSrcRow );

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
    return static_cast< sal_uInt16 >( maFieldList.Count() );
}

const XclExpPCField* XclExpPivotCache::GetField( sal_uInt16 nFieldIdx ) const
{
    return maFieldList.GetObject( nFieldIdx );
}

const XclExpPCField* XclExpPivotCache::GetField( const String& rFieldName ) const
{
    return const_cast< XclExpPivotCache* >( this )->GetFieldAcc( rFieldName );
}

bool XclExpPivotCache::HasAddFields() const
{
    // pivot cache can be shared, if there are no additional cache fields
    return maPCInfo.mnStdFields == maPCInfo.mnTotalFields;
}

bool XclExpPivotCache::HasEqualDataSource( const ScDPObject& rDPObj ) const
{
    /*  For now, only sheet sources are supported, therefore it is enough to
        compare the ScSheetSourceDesc. Later, there should be done more complicated
        comparisons regarding the source type of rDPObj and this cache. */
    if( const ScSheetSourceDesc* pSrcDesc = rDPObj.GetSheetDesc() )
        return pSrcDesc->aSourceRange == maOrigSrcRange;
    return false;
}

void XclExpPivotCache::Save( XclExpStream& rStrm )
{
    DBG_ASSERT( mbValid, "XclExpPivotCache::Save - invalid pivot cache" );
    // SXIDSTM
    XclExpUInt16Record( EXC_ID_SXIDSTM, maPCInfo.mnStrmId ).Save( rStrm );
    // SXVS
    XclExpUInt16Record( EXC_ID_SXVS, EXC_SXVS_SHEET ).Save( rStrm );
    // DCONREF
    WriteDconref( rStrm );
    // create the pivot cache storage stream
    WriteCacheStream();
}

// private --------------------------------------------------------------------

XclExpPCField* XclExpPivotCache::GetFieldAcc( sal_uInt16 nFieldIdx )
{
    return maFieldList.GetObject( nFieldIdx );
}

XclExpPCField* XclExpPivotCache::GetFieldAcc( const String& rFieldName )
{
    for( XclExpPCField* pField = maFieldList.First(); pField; pField = maFieldList.Next() )
        if( pField->GetFieldName() == rFieldName )
            return pField;
    return 0;
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
        maFieldList.Append( new XclExpPCField( GetRoot(), *this, GetFieldCount(), rDPObj, aColRange ) );
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
                        XclExpPCField* pNewGroupField = new XclExpPCField( GetRoot(), *this, GetFieldCount(), rDPObj, *pGroupDim, *pCurrStdField );
                        maFieldList.Append( pNewGroupField );

                        // register new grouping field at current grouping field, building a chain
                        pLastGroupField->SetGroupChildField( *pNewGroupField );

                        // next grouping dimension
                        pGroupDim = pSaveDimData->GetGroupDimForBase( pGroupDim->GetGroupDimName() );
                        pLastGroupField = pNewGroupField;
                    }
                }
            }
        }
    }
}

void XclExpPivotCache::AddCalcFields( const ScDPObject& rDPObj )
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

void XclExpPivotCache::WriteCacheStream()
{
    SvStorageRef xSvStrg = OpenStorage( EXC_STORAGE_PTCACHE );
    SvStorageStreamRef xSvStrm = OpenStream( xSvStrg, ScfTools::GetHexStr( maPCInfo.mnStrmId ) );
    if( xSvStrm.Is() )
    {
        XclExpStream aStrm( *xSvStrm, GetRoot() );
        // SXDB
        WriteSxdb( aStrm );
        // SXDBEX
        WriteSxdbex( aStrm );
        // field list (SXFIELD and items)
        maFieldList.Save( aStrm );
        // index table (list of SXIDARRAY)
        WriteSxidarrayList( aStrm );
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

void XclExpPivotCache::WriteSxidarrayList( XclExpStream& rStrm ) const
{
    if( HasItemIndexList() )
    {
        sal_uInt32 nRecSize = 0;
        for( const XclExpPCField* pField = maFieldList.First(); pField; pField = maFieldList.Next() )
            nRecSize += pField->GetIndexSize();

        for( sal_uInt32 nSrcRow = 0; nSrcRow < maPCInfo.mnSrcRecs; ++nSrcRow )
        {
            rStrm.StartRecord( EXC_ID_SXIDARRAY, nRecSize );
            for( const XclExpPCField* pField = maFieldList.First(); pField; pField = maFieldList.Next() )
                pField->WriteIndex( rStrm, nSrcRow );
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

    USHORT nResIdx = 0;
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

const String& XclExpPTItem::GetItemName() const
{
    return mpCacheItem ? mpCacheItem->ConvertToText() : EMPTY_STRING;
}

void XclExpPTItem::SetPropertiesFromMember( const ScDPSaveMember& rSaveMem )
{
    ::set_flag( maItemInfo.mnFlags, EXC_SXVI_HIDDEN, !rSaveMem.GetIsVisible() );
    ::set_flag( maItemInfo.mnFlags, EXC_SXVI_HIDEDETAIL, !rSaveMem.GetShowDetails() );
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
            maItemList.Append( new XclExpPTItem( *mpCacheField, nItemIdx ) );
    maFieldInfo.mnItemCount = static_cast< sal_uInt16 >( maItemList.Count() );
}

// data access ----------------------------------------------------------------

const String& XclExpPTField::GetFieldName() const
{
    return mpCacheField ? mpCacheField->GetFieldName() : EMPTY_STRING;
}

sal_uInt16 XclExpPTField::GetFieldIndex() const
{
    // field index always equal to cache index
    return maFieldInfo.mnCacheIdx;
}

sal_uInt16 XclExpPTField::GetLastDataInfoIndex() const
{
    DBG_ASSERT( !maDataInfoVec.empty(), "XclExpPTField::GetLastDataInfoIndex - no data info found" );
    // will return 0xFFFF for empty vector -> ok
    return static_cast< sal_uInt16 >( maDataInfoVec.size() - 1 );
}

const XclExpPTItem* XclExpPTField::GetItem( const String& rName ) const
{
    return const_cast< XclExpPTField* >( this )->GetItemAcc( rName );
}

sal_uInt16 XclExpPTField::GetItemIndex( const String& rName, sal_uInt16 nDefaultIdx ) const
{
    for( const XclExpPTItem* pItem = maItemList.First(); pItem; pItem = maItemList.Next() )
        if( pItem->GetItemName() == rName )
            return static_cast< sal_uInt16 >( maItemList.GetCurPos() );
    return nDefaultIdx;
}

// fill data --------------------------------------------------------------

void XclExpPTField::SetPropertiesFromDim( const ScDPSaveDimension& rSaveDim )
{
    // orientation
    DataPilotFieldOrientation eOrient = static_cast< DataPilotFieldOrientation >( rSaveDim.GetOrientation() );
    DBG_ASSERT( eOrient != DataPilotFieldOrientation_DATA, "XclExpPTField::SetPropertiesFromDim - called for data field" );
    maFieldInfo.AddApiOrient( eOrient );

    // show empty items
    ::set_flag( maFieldExtInfo.mnFlags, EXC_SXVDEX_SHOWALL, rSaveDim.GetShowEmpty() );

    // visible name
    if( rSaveDim.HasLayoutName() && (rSaveDim.GetLayoutName() != GetFieldName()) )
        maFieldInfo.SetVisName( rSaveDim.GetLayoutName() );

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
    const List& rMemList = rSaveDim.GetMembers();
    for( ULONG nMemIdx = 0, nMemCount = rMemList.Count(); nMemIdx < nMemCount; ++nMemIdx )
        if( const ScDPSaveMember* pSaveMem = static_cast< const ScDPSaveMember* >( rMemList.GetObject( nMemIdx ) ) )
            if( XclExpPTItem* pItem = GetItemAcc( pSaveMem->GetName() ) )
                pItem->SetPropertiesFromMember( *pSaveMem );
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
    DBG_ASSERT( nDataInfoIdx < maDataInfoVec.size(), "XclExpPTField::WriteSxdi - data field not found" );
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

XclExpPTItem* XclExpPTField::GetItemAcc( const String& rName )
{
    for( XclExpPTItem* pItem = maItemList.First(); pItem; pItem = maItemList.Next() )
        if( pItem->GetItemName() == rName )
            return pItem;
    return 0;
}

void XclExpPTField::AppendSubtotalItem( sal_uInt16 nItemType )
{
    maItemList.Append( new XclExpPTItem( nItemType, EXC_SXVI_DEFAULT_CACHE, true ) );
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
    maOutputRange( rDPObj.GetOutRange() ),
    mbValid( false ),
    mbFilterBtn( false )
{
    if( CheckCellRange( maOutputRange ) )
    {
        // DataPilot properties -----------------------------------------------

        // pivot table properties from DP object
        maPTInfo.maTableName = rDPObj.GetName();
        maPTInfo.maDataName = ScGlobal::GetRscString( STR_PIVOT_DATA );
        maPTInfo.mnCacheIdx = mrPCache.GetCacheIndex();

        if( const ScDPSaveData* pSaveData = rDPObj.GetSaveData() )
        {
            // additional properties from ScDPSaveData
            SetPropertiesFromDP( *pSaveData );

            // loop over all dimensions ---------------------------------------

            /*  1)  Default-construct all pivot table fields for all pivot cache fields. */
            for( sal_uInt16 nFieldIdx = 0, nFieldCount = mrPCache.GetFieldCount(); nFieldIdx < nFieldCount; ++nFieldIdx )
                maFieldList.Append( new XclExpPTField( *this, nFieldIdx ) );

            const List& rDimList = pSaveData->GetDimensions();
            ULONG nDimIdx, nDimCount = rDimList.Count();

            /*  2)  First process all data dimensions, they are needed for extended
                    settings of row/column/page fields (sorting/auto show). */
            for( nDimIdx = 0; nDimIdx < nDimCount; ++nDimIdx )
                if( const ScDPSaveDimension* pSaveDim = static_cast< const ScDPSaveDimension* >( rDimList.GetObject( nDimIdx ) ) )
                    if( pSaveDim->GetOrientation() == DataPilotFieldOrientation_DATA )
                        SetDataFieldPropertiesFromDim( *pSaveDim );

            /*  3)  Row/column/page/hidden fields. */
            for( nDimIdx = 0; nDimIdx < nDimCount; ++nDimIdx )
                if( const ScDPSaveDimension* pSaveDim = static_cast< const ScDPSaveDimension* >( rDimList.GetObject( nDimIdx ) ) )
                    if( pSaveDim->GetOrientation() != DataPilotFieldOrientation_DATA )
                        SetFieldPropertiesFromDim( *pSaveDim );

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

SCTAB XclExpPivotTable::GetScTab() const
{
    return maOutputRange.aStart.Tab();
}

const XclExpPTField* XclExpPivotTable::GetField( sal_uInt16 nFieldIdx ) const
{
    return (nFieldIdx == EXC_SXIVD_DATA) ? &maDataOrientField : maFieldList.GetObject( nFieldIdx );
}

const XclExpPTField* XclExpPivotTable::GetField( const String& rName ) const
{
    return const_cast< XclExpPivotTable* >( this )->GetFieldAcc( rName );
}

sal_uInt16 XclExpPivotTable::GetDataFieldIndex( const String& rName, sal_uInt16 nDefaultIdx ) const
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
    }
}

// private --------------------------------------------------------------------

XclExpPTField* XclExpPivotTable::GetFieldAcc( const String& rName )
{
    for( XclExpPTField* pField = maFieldList.First(); pField; pField = maFieldList.Next() )
        if( pField->GetFieldName() == rName )
            return pField;
    return 0;
}

XclExpPTField* XclExpPivotTable::GetFieldAcc( const ScDPSaveDimension& rSaveDim )
{
    // data field orientation field?
    if( rSaveDim.IsDataLayout() )
        return &maDataOrientField;

    // a real dimension
    String aFieldName( rSaveDim.GetName() );
    return aFieldName.Len() ? GetFieldAcc( aFieldName ) : 0;
}

// fill data --------------------------------------------------------------

void XclExpPivotTable::SetPropertiesFromDP( const ScDPSaveData& rSaveData )
{
    ::set_flag( maPTInfo.mnFlags, EXC_SXVIEW_ROWGRAND, rSaveData.GetRowGrand() );
    ::set_flag( maPTInfo.mnFlags, EXC_SXVIEW_COLGRAND, rSaveData.GetColumnGrand() );
    ::set_flag( maPTExtInfo.mnFlags, EXC_SXEX_DRILLDOWN, rSaveData.GetDrillDown() );
    mbFilterBtn = rSaveData.GetFilterButton();
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
                DBG_ASSERT( !bDataLayout, "XclExpPivotTable::SetFieldPropertiesFromDim - wrong orientation for data fields" );
            break;
            case DataPilotFieldOrientation_DATA:
                DBG_ERRORFILE( "XclExpPivotTable::SetFieldPropertiesFromDim - called for data field" );
            break;
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
    maPTInfo.mnFields = static_cast< sal_uInt16 >( maFieldList.Count() );
    maPTInfo.mnRowFields = static_cast< sal_uInt16 >( maRowFields.size() );
    maPTInfo.mnColFields = static_cast< sal_uInt16 >( maColFields.size() );
    maPTInfo.mnPageFields = static_cast< sal_uInt16 >( maPageFields.size() );
    maPTInfo.mnDataFields = static_cast< sal_uInt16 >( maDataFields.size() );

    maPTExtInfo.mnPagePerRow = maPTInfo.mnPageFields ? 1 : 0;
    maPTExtInfo.mnPagePerCol = maPTInfo.mnPageFields;

    // subtotal items
    for( XclExpPTField* pField = maFieldList.First(); pField; pField = maFieldList.Next() )
        pField->AppendSubtotalItems();

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

    // output range
    maPTInfo.mnFirstRow = static_cast< sal_uInt16 >( maOutputRange.aStart.Row() );
    maPTInfo.mnLastRow = static_cast< sal_uInt16 >( maOutputRange.aEnd.Row() );
    maPTInfo.mnFirstCol = static_cast< sal_uInt16 >( maOutputRange.aStart.Col() );
    maPTInfo.mnLastCol = static_cast< sal_uInt16 >( maOutputRange.aEnd.Col() );
    // exclude page fields from output range
    maPTInfo.mnFirstRow += maPTInfo.mnPageFields;
    // exclude filter button from output range
    if( mbFilterBtn )
        ++maPTInfo.mnFirstRow;
    // exclude empty row between (filter button and/or page fields) and table
    if( mbFilterBtn || maPTInfo.mnPageFields )
        ++maPTInfo.mnFirstRow;

    // data area
    maPTInfo.mnFirstDataRow = maPTInfo.mnFirstRow + maPTInfo.mnColFields + 1;
    maPTInfo.mnFirstDataCol = maPTInfo.mnFirstCol + maPTInfo.mnRowFields;
    if( maDataFields.empty() )
        ++maPTInfo.mnFirstDataRow;
    maPTInfo.mnLastRow = ::std::max( maPTInfo.mnLastRow, maPTInfo.mnFirstDataRow );
    maPTInfo.mnLastCol = ::std::max( maPTInfo.mnLastCol, maPTInfo.mnFirstDataCol );
    maPTInfo.mnDataRows = maPTInfo.mnLastRow - maPTInfo.mnFirstDataRow + 1;
    maPTInfo.mnDataCols = maPTInfo.mnLastCol - maPTInfo.mnFirstDataCol + 1;

    // first heading
    maPTInfo.mnFirstHeadRow = maPTInfo.mnFirstRow + 1;
}

// records ----------------------------------------------------------------

void XclExpPivotTable::WriteSxview( XclExpStream& rStrm ) const
{
    rStrm.StartRecord( EXC_ID_SXVIEW, 46 + maPTInfo.maTableName.Len() + maPTInfo.maDataName.Len() );
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
            if( const XclExpPTField* pField = maFieldList.GetObject( *aIt ) )
                pField->WriteSxpiEntry( rStrm );
        rStrm.EndRecord();
    }
}

void XclExpPivotTable::WriteSxdiList( XclExpStream& rStrm ) const
{
    for( XclPTDataFieldPosVec::const_iterator aIt = maDataFields.begin(), aEnd = maDataFields.end(); aIt != aEnd; ++aIt )
        if( const XclExpPTField* pField = maFieldList.GetObject( aIt->first ) )
            pField->WriteSxdi( rStrm, aIt->second );
}

void XclExpPivotTable::WriteSxli( XclExpStream& rStrm, sal_uInt16 nLineCount, sal_uInt16 nIndexCount ) const
{
    if( nLineCount )
    {
        sal_uInt32 nLineSize = 8 + 2 * nIndexCount;
        rStrm.StartRecord( EXC_ID_SXLI, nLineSize * nLineCount );
        rStrm.SetSliceSize( nLineSize );
        for( sal_uInt16 nLine = 0; nLine < nLineCount; ++nLine )
        {
            // #106598# Excel XP needs a partly initialized SXLI record
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

// ============================================================================

XclExpPivotTableManager::XclExpPivotTableManager( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot ),
    mbShareCaches( true )
{
}

void XclExpPivotTableManager::CreatePivotTables()
{
    if( ScDPCollection* pDPColl = GetDoc().GetDPCollection() )
        for( USHORT nDPObj = 0, nCount = pDPColl->GetCount(); nDPObj < nCount; ++nDPObj )
            if( ScDPObject* pDPObj = (*pDPColl)[ nDPObj ] )
                if( const XclExpPivotCache* pPCache = CreatePivotCache( *pDPObj ) )
                    maPTableList.Append( new XclExpPivotTable( GetRoot(), *pDPObj, *pPCache ) );
}

void XclExpPivotTableManager::WritePivotCaches( XclExpStream& rStrm )
{
    maPCacheList.Save( rStrm );
}

void XclExpPivotTableManager::WritePivotTables( XclExpStream& rStrm, SCTAB nScTab )
{
    for( XclExpPivotTable* pPTable = maPTableList.First(); pPTable; pPTable = maPTableList.Next() )
        if( pPTable->GetScTab() == nScTab )
            pPTable->Save( rStrm );
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
                for( XclExpPivotCache* pPCache = maPCacheList.First(); pPCache; pPCache = maPCacheList.Next() )
                    // pivot cache does not have grouping info and source data is equal
                    if( !pPCache->HasAddFields() && pPCache->HasEqualDataSource( rDPObj ) )
                        return pPCache;
            }
        }
    }

    // create a new pivot cache
    sal_uInt16 nNewCacheIdx = static_cast< sal_uInt16 >( maPCacheList.Count() );
    ::std::auto_ptr< XclExpPivotCache > xNewPCache(
        new XclExpPivotCache( GetRoot(), rDPObj, nNewCacheIdx ) );
    if( xNewPCache->IsValid() )
    {
        maPCacheList.Append( xNewPCache.release() );
        return maPCacheList.Last();
    }

    return 0;
}

// ============================================================================
// Reference record classes
// ============================================================================

XclExpPivotCacheRefRecord::XclExpPivotCacheRefRecord( const XclExpRoot& rRoot ) :
    mrPTManager( rRoot.GetPivotTableManager() )
{
}

void XclExpPivotCacheRefRecord::Save( XclExpStream& rStrm )
{
    mrPTManager.WritePivotCaches( rStrm );
}

// ----------------------------------------------------------------------------

XclExpPivotTablesRefRecord::XclExpPivotTablesRefRecord( const XclExpRoot& rRoot ) :
    mrPTManager( rRoot.GetPivotTableManager() ),
    mnScTab( rRoot.GetCurrScTab() )
{
}

void XclExpPivotTablesRefRecord::Save( XclExpStream& rStrm )
{
    mrPTManager.WritePivotTables( rStrm, mnScTab );
}

// ============================================================================

