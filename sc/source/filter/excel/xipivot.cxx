/*************************************************************************
 *
 *  $RCSfile: xipivot.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 12:54:30 $
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

#ifndef SC_XIPIVOT_HXX
#include "xipivot.hxx"
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

#ifndef _DATETIME_HXX
#include <tools/datetime.hxx>
#endif
#ifndef _ZFORMAT_HXX
#include <svtools/zformat.hxx>
#endif

#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif
#ifndef SC_DPSAVE_HXX
#include "dpsave.hxx"
#endif
#ifndef SC_DPOBJECT_HXX
#include "dpobject.hxx"
#endif
#ifndef SC_DPSHTTAB_HXX
#include "dpshttab.hxx"
#endif

#ifndef SC_XLTRACER_HXX
#include "xltracer.hxx"
#endif
#ifndef SC_XISTREAM_HXX
#include "xistream.hxx"
#endif
#ifndef SC_XILINK_HXX
#include "xilink.hxx"
#endif
#ifndef SC_XIESCHER_HXX
#include "xiescher.hxx"
#endif

#include "root.hxx"

using ::rtl::OUString;
using ::com::sun::star::sheet::DataPilotFieldOrientation;
using ::com::sun::star::sheet::DataPilotFieldOrientation_DATA;
using ::com::sun::star::sheet::DataPilotFieldSortInfo;
using ::com::sun::star::sheet::DataPilotFieldAutoShowInfo;
using ::com::sun::star::sheet::DataPilotFieldLayoutInfo;
using ::com::sun::star::sheet::DataPilotFieldReference;

// ============================================================================
// Pivot cache
// ============================================================================

XclImpPCItem::XclImpPCItem( XclImpStream& rStrm, sal_uInt16 nRecId )
{
    switch( nRecId )
    {
        case EXC_ID_SXDOUBLE:   ReadSxdouble( rStrm );      break;
        case EXC_ID_SXBOOLEAN:  ReadSxboolean( rStrm );     break;
        case EXC_ID_SXERROR:    ReadSxerror( rStrm );       break;
        case EXC_ID_SXSTRING:   ReadSxstring( rStrm );      break;
        case EXC_ID_SXDATETIME: ReadSxdatetime( rStrm );    break;
        case EXC_ID_SXEMPTY:    ReadSxempty( rStrm );       break;
        default:    DBG_ERRORFILE( "XclImpPCItem::XclImpPCItem - unknown record id" );
    }
}

void XclImpPCItem::ReadSxdouble( XclImpStream& rStrm )
{
    DBG_ASSERT( rStrm.GetRecSize() == 8, "XclImpPCItem::ReadSxdouble - wrong record size" );
    SetValue( rStrm.ReadDouble() );
}

void XclImpPCItem::ReadSxboolean( XclImpStream& rStrm )
{
    DBG_ASSERT( rStrm.GetRecSize() == 2, "XclImpPCItem::ReadSxboolean - wrong record size" );
    SetBool( rStrm.ReaduInt16() != 0 );
}

void XclImpPCItem::ReadSxerror( XclImpStream& rStrm )
{
    DBG_ASSERT( rStrm.GetRecSize() == 2, "XclImpPCItem::ReadSxerror - wrong record size" );
    SetError( rStrm.ReaduInt16() );
}

void XclImpPCItem::ReadSxstring( XclImpStream& rStrm )
{
    DBG_ASSERT( rStrm.GetRecSize() >= 3, "XclImpPCItem::ReadSxstring - wrong record size" );
    SetText( rStrm.ReadUniString() );
}

void XclImpPCItem::ReadSxdatetime( XclImpStream& rStrm )
{
    DBG_ASSERT( rStrm.GetRecSize() == 8, "XclImpPCItem::ReadSxdatetime - wrong record size" );

    sal_uInt16 nYear, nMonth;
    sal_uInt8 nDay, nHour, nMin, nSec;
    rStrm >> nYear >> nMonth >> nDay >> nHour >> nMin >> nSec;

    DateTime aNullDate( *rStrm.GetRoot().GetFormatter().GetNullDate() );
    DateTime aDate( Date( nDay, nMonth, nYear ), Time( nHour, nMin, nSec ) );
    SetDate( aDate - aNullDate );
}

void XclImpPCItem::ReadSxempty( XclImpStream& rStrm )
{
    DBG_ASSERT( rStrm.GetRecSize() == 0, "XclImpPCItem::ReadSxempty - wrong record size" );
    SetEmpty();
}

// ============================================================================

XclImpPCField::XclImpPCField( const String& rName, bool bPostponeItems ) :
    maName( rName ),
    mbPostponeItems( bPostponeItems )
{
}

XclImpPCField::~XclImpPCField()
{
}

const XclImpPCItem* XclImpPCField::GetItem( sal_uInt16 nItemIdx ) const
{
    return maItemList.GetObject( nItemIdx );
}

void XclImpPCField::ReadItem( XclImpStream& rStrm, sal_uInt16 nRecId )
{
    maItemList.Append( new XclImpPCItem( rStrm, nRecId ) );
}

// ============================================================================

XclImpPivotCache::XclImpPivotCache( const XclImpRoot& rRoot, sal_uInt16 nStrmId ) :
    XclImpRoot( rRoot ),
    mnSrcType( EXC_SXVS_UNKNOWN )
{
    if( SvStorage* pSt = mpRD->pPivotCacheStorage )
    {
        SvStorageStreamRef pStIn = pSt->OpenStream( ScfTools::GetHexStr( nStrmId ), STREAM_STD_READ );
        if( pStIn.Is() )
        {
            XclImpStream aStrm( *pStIn, GetRoot() );
            ReadPivotCacheStream( aStrm );
        }
    }
}

XclImpPivotCache::~XclImpPivotCache()
{
}

// data access ----------------------------------------------------------------

const String& XclImpPivotCache::GetFieldName( sal_uInt16 nFieldIdx ) const
{
    if( const XclImpPCField* pField = maFieldList.GetObject( nFieldIdx ) )
        return pField->GetFieldName();
    return EMPTY_STRING;
}

const XclImpPCItem* XclImpPivotCache::GetItem( sal_uInt16 nFieldIdx, sal_uInt16 nItemIdx ) const
{
    if( const XclImpPCField* pField = maFieldList.GetObject( nFieldIdx ) )
        return pField->GetItem( nItemIdx );
    return 0;
}

// records --------------------------------------------------------------------

void XclImpPivotCache::ReadDconref( XclImpStream& rStrm )
{
    // read DCONREF only once, there may be others in another context
    if( maTabName.Len() )
        return;

    sal_uInt16 nStartRow, nEndRow;
    sal_uInt8 nStartCol, nEndCol;
    rStrm >> nStartRow >> nEndRow >> nStartCol >> nEndCol;
    String aEncUrl( rStrm.ReadUniString() );

    XclImpUrlHelper::DecodeUrl( maUrl, maTabName, mbSelf, GetRoot(), aEncUrl );
    if( !maTabName.Len() )
    {
        maTabName = maUrl;
        maUrl.Erase();
    }

    // Sheet index will be found later in XclImpPivotTable::Apply() (sheet may not exist yet).
    // Do not convert maTabName to Calc sheet name -> original name is used to find the sheet.
    maSrcRange.aStart.Set( static_cast< SCCOL >( nStartCol ), static_cast< SCROW >( nStartRow ), 0 );
    maSrcRange.aEnd.Set( static_cast< SCCOL >( nEndCol ), static_cast< SCROW >( nEndRow ), 0 );
    CheckCellRange( maSrcRange );
}

void XclImpPivotCache::ReadSxvs( XclImpStream& rStrm )
{
    rStrm >> mnSrcType;
    GetTracer().TracePivotDataSource( mnSrcType == EXC_SXVS_EXTERN );
}

// private --------------------------------------------------------------------

void XclImpPivotCache::ReadPivotCacheStream( XclImpStream& rStrm )
{
    XclImpPCField* pCurrField = 0;  // Current field for new items.
    bool bExistPostponed = false;   // true = Any field with postponed items.
    bool bReadPostponed = false;    // true = Read now into postponed fields.
    bool bLoop = true;              // true = Continue loop.

    while( bLoop && rStrm.StartNextRecord() )
    {
        bool bLastWasItem = false;  // true = Last record was item data.

        switch( rStrm.GetRecId() )
        {
            case EXC_ID_EOF:
                bLoop = false;
            break;

            case EXC_ID_SXFIELD:
            {
                sal_uInt16 nFlags;
                rStrm >> nFlags;
                rStrm.Ignore( 12 );
                if( rStrm.GetRecLeft() >= 3 )
                {
                    bool bPostponeItems = ::get_flag( nFlags, EXC_SXFIELD_POSTPONE );
                    pCurrField = new XclImpPCField( rStrm.ReadUniString(), bPostponeItems );
                    maFieldList.Append( pCurrField );
                    bExistPostponed |= bPostponeItems;
                }
            }
            break;

            case EXC_ID_SXIDARRAY:
                // Items of postponed fields start now.
                bReadPostponed = bExistPostponed;
                if( bReadPostponed )
                {
                    // Find the first postponed field.
                    pCurrField = maFieldList.First();
                    while( pCurrField && !pCurrField->HasPostponedItems() )
                        pCurrField = maFieldList.Next();
                }
            break;

            case EXC_ID_SXDOUBLE:
            case EXC_ID_SXBOOLEAN:
            case EXC_ID_SXERROR:
            case EXC_ID_SXSTRING:
            case EXC_ID_SXDATETIME:
            case EXC_ID_SXEMPTY:
                if( pCurrField )
                    pCurrField->ReadItem( rStrm, rStrm.GetRecId() );
                bLastWasItem = true;
            break;
        }

        if( bReadPostponed && bLastWasItem )
        {
            // Find the next postponed field.
            pCurrField = maFieldList.Next();
            while( pCurrField && !pCurrField->HasPostponedItems() )
                pCurrField = maFieldList.Next();
        }
    }
}

// ============================================================================
// Pivot table
// ============================================================================

XclImpPTItem::XclImpPTItem( sal_uInt16 nFieldCacheIdx ) :
    mnFieldCacheIdx( nFieldCacheIdx )
{
}

const String* XclImpPTItem::GetItemName( const XclImpPivotCache* pCache ) const
{
    if( !maItemInfo.mbUseCache )
        return &maItemInfo.maName;
    if( pCache )
        if( const XclImpPCItem* pCacheItem = pCache->GetItem( mnFieldCacheIdx, maItemInfo.mnCacheIdx ) )
            //! TODO: use XclImpPCItem::ConvertToString(), if all conversions are available
            return pCacheItem->IsEmpty() ? &EMPTY_STRING : pCacheItem->GetText();
    return 0;
}

void XclImpPTItem::ReadSxvi( XclImpStream& rStrm )
{
    rStrm >> maItemInfo;
}

void XclImpPTItem::ApplyItem( ScDPSaveDimension& rSaveDim, const XclImpPivotCache* pCache ) const
{
    if( const String* pItemName = GetItemName( pCache ) )
    {
        ScDPSaveMember& rMember = *rSaveDim.GetMemberByName( *pItemName );
        rMember.SetIsVisible( !::get_flag( maItemInfo.mnFlags, EXC_SXVI_HIDDEN ) );
        rMember.SetShowDetails( !::get_flag( maItemInfo.mnFlags, EXC_SXVI_HIDEDETAIL ) );
    }
}

// ============================================================================

XclImpPTField::XclImpPTField( const XclImpPivotTable& rPTable, sal_uInt16 nCacheIdx ) :
    mrPTable( rPTable )
{
    maFieldInfo.mnCacheIdx = nCacheIdx;
}

// general field/item access --------------------------------------------------

const String& XclImpPTField::GetFieldName() const
{
    if( const XclImpPivotCache* pCache = mrPTable.GetPivotCache() )
        return pCache->GetFieldName( maFieldInfo.mnCacheIdx );
    return EMPTY_STRING;
}

const XclImpPTItem* XclImpPTField::GetItem( sal_uInt16 nItemIdx ) const
{
    return maItemList.GetObject( nItemIdx );
}

const String* XclImpPTField::GetItemName( sal_uInt16 nItemIdx ) const
{
    if( const XclImpPTItem* pItem = GetItem( nItemIdx ) )
        return pItem->GetItemName( mrPTable.GetPivotCache() );
    return 0;
}

void XclImpPTField::SetAxes( sal_uInt16 nAxes )
{
    maFieldInfo.mnAxes = nAxes;
}

// records --------------------------------------------------------------------

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
    XclImpPTItem* pItem = new XclImpPTItem( maFieldInfo.mnCacheIdx );
    maItemList.Append( pItem );
    pItem->ReadSxvi( rStrm );
}

// row/column fields ----------------------------------------------------------

void XclImpPTField::ApplyRowColField( ScDPSaveData& rSaveData ) const
{
    DBG_ASSERT( maFieldInfo.mnAxes & EXC_SXVD_AXIS_RC_MASK, "XclImpPTField::ApplyRowColField - no row/column field" );
    if( maFieldInfo.mnCacheIdx == EXC_SXIVD_DATA )
    {
        // special data orientation field
        rSaveData.GetDataLayoutDimension()->SetOrientation( maFieldInfo.GetApiOrient() );
    }
    else
    {
        const String& rFieldName = GetFieldName();
        if( rFieldName.Len() )
        {
            ScDPSaveDimension& rSaveDim = *rSaveData.GetNewDimensionByName( rFieldName );
            ApplyRCPField( rSaveDim );
        }
    }
}

// page fields ----------------------------------------------------------------

void XclImpPTField::SetPageFieldInfo( const XclPTPageFieldInfo& rPageInfo )
{
    maPageInfo = rPageInfo;
}

void XclImpPTField::ApplyPageField( ScDPSaveData& rSaveData ) const
{
    DBG_ASSERT( maFieldInfo.mnAxes & EXC_SXVD_AXIS_PAGE, "XclImpPTField::ApplyPageField - no page field" );
    const String& rFieldName = GetFieldName();
    if( rFieldName.Len() )
    {
        ScDPSaveDimension& rSaveDim = *rSaveData.GetNewDimensionByName( rFieldName );
        ApplyRCPField( rSaveDim );
        rSaveDim.SetCurrentPage( GetItemName( maPageInfo.mnSelItem ) );
    }
}

// data fields ----------------------------------------------------------------

bool XclImpPTField::HasDataFieldInfo() const
{
    return !maDataInfoList.empty();
}

void XclImpPTField::AddDataFieldInfo( const XclPTDataFieldInfo& rDataInfo )
{
    DBG_ASSERT( maFieldInfo.mnAxes & EXC_SXVD_AXIS_DATA, "XclImpPTField::AddDataFieldInfo - no data field" );
    maDataInfoList.push_back( rDataInfo );
}

void XclImpPTField::ApplyDataField( ScDPSaveData& rSaveData ) const
{
    DBG_ASSERT( maFieldInfo.mnAxes & EXC_SXVD_AXIS_DATA, "XclImpPTField::ApplyDataField - no data field" );
    DBG_ASSERT( !maDataInfoList.empty(), "XclImpPTField::ApplyDataField - no data field info" );
    if( !maDataInfoList.empty() )
    {
        const String& rFieldName = GetFieldName();
        if( rFieldName.Len() )
        {
            XclPTDataFieldInfoList::const_iterator aIt = maDataInfoList.begin(), aEnd = maDataInfoList.end();

            ScDPSaveDimension& rSaveDim = *rSaveData.GetNewDimensionByName( rFieldName );
            ApplyDataField( rSaveDim, *aIt );

            // multiple data fields -> clone dimension
            for( ++aIt; aIt != aEnd; ++aIt )
            {
                ScDPSaveDimension& rDupDim = rSaveData.DuplicateDimension( rSaveDim );
                ApplyDataFieldInfo( rDupDim, *aIt );
            }
        }
    }
}

// private --------------------------------------------------------------------

void XclImpPTField::ApplyRCPField( ScDPSaveDimension& rSaveDim ) const
{
    // orientation
    rSaveDim.SetOrientation( maFieldInfo.GetApiOrient() );

    // general field info
    ApplyFieldInfo( rSaveDim );

    // visible name
    if( maFieldInfo.maVisName.Len() )
        rSaveDim.SetLayoutName( &maFieldInfo.maVisName );

    // subtotal function(s)
    XclPTSubtotalVec aSubtotalVec;
    maFieldInfo.GetSubtotals( aSubtotalVec );
    if( !aSubtotalVec.empty() )
        rSaveDim.SetSubTotals( static_cast< long >( aSubtotalVec.size() ), &aSubtotalVec[ 0 ] );

    // sorting
    DataPilotFieldSortInfo aSortInfo;
    aSortInfo.Field = mrPTable.GetDataFieldName( maFieldExtInfo.mnSortField );
    aSortInfo.IsAscending = ::get_flag( maFieldExtInfo.mnFlags, EXC_SXVDEX_SORT_ASC );
    aSortInfo.Mode = maFieldExtInfo.GetApiSortMode();
    rSaveDim.SetSortInfo( &aSortInfo );

    // auto show
    DataPilotFieldAutoShowInfo aShowInfo;
    aShowInfo.IsEnabled = ::get_flag( maFieldExtInfo.mnFlags, EXC_SXVDEX_AUTOSHOW );
    aShowInfo.ShowItemsMode = maFieldExtInfo.GetApiAutoShowMode();
    aShowInfo.ItemCount = maFieldExtInfo.GetApiAutoShowCount();
    aShowInfo.DataField = mrPTable.GetDataFieldName( maFieldExtInfo.mnShowField );
    rSaveDim.SetAutoShowInfo( &aShowInfo );

    // layout
    DataPilotFieldLayoutInfo aLayoutInfo;
    aLayoutInfo.LayoutMode = maFieldExtInfo.GetApiLayoutMode();
    aLayoutInfo.AddEmptyLines = ::get_flag( maFieldExtInfo.mnFlags, EXC_SXVDEX_LAYOUT_BLANK );
    rSaveDim.SetLayoutInfo( &aLayoutInfo );
}

void XclImpPTField::ApplyDataField( ScDPSaveDimension& rSaveDim, const XclPTDataFieldInfo& rDataInfo ) const
{
    // orientation
    rSaveDim.SetOrientation( DataPilotFieldOrientation_DATA );
    // general field info
    ApplyFieldInfo( rSaveDim );
    // extended data field info
    ApplyDataFieldInfo( rSaveDim, rDataInfo );
}

void XclImpPTField::ApplyFieldInfo( ScDPSaveDimension& rSaveDim ) const
{
    rSaveDim.SetShowEmpty( ::get_flag( maFieldExtInfo.mnFlags, EXC_SXVDEX_SHOWALL ) );
    ApplyItems( rSaveDim );
}

void XclImpPTField::ApplyDataFieldInfo( ScDPSaveDimension& rSaveDim, const XclPTDataFieldInfo& rDataInfo ) const
{
    // visible name
    if( rDataInfo.maVisName.Len() )
        rSaveDim.SetLayoutName( &rDataInfo.maVisName );

    // aggregation function
    rSaveDim.SetFunction( rDataInfo.GetApiAggFunc() );

    // result field reference
    sal_Int32 nRefType = rDataInfo.GetApiRefType();
    if( nRefType != ::com::sun::star::sheet::DataPilotFieldReferenceType::NONE )
    {
        DataPilotFieldReference aFieldRef;
        aFieldRef.ReferenceType = nRefType;

        if( const XclImpPTField* pRefField = mrPTable.GetField( rDataInfo.mnRefField ) )
        {
            aFieldRef.ReferenceField = pRefField->GetFieldName();
            aFieldRef.ReferenceItemType = rDataInfo.GetApiRefItemType();
            if( aFieldRef.ReferenceItemType == ::com::sun::star::sheet::DataPilotFieldReferenceItemType::NAMED )
                if( const String* pRefItemName = pRefField->GetItemName( rDataInfo.mnRefItem ) )
                    aFieldRef.ReferenceItemName = *pRefItemName;
        }

        rSaveDim.SetReferenceValue( &aFieldRef );
    }
}

void XclImpPTField::ApplyItems( ScDPSaveDimension& rSaveDim ) const
{
    const XclImpPivotCache* pCache = mrPTable.GetPivotCache();
    for( const XclImpPTItem* pItem = maItemList.First(); pItem; pItem = maItemList.Next() )
        pItem->ApplyItem( rSaveDim, pCache );
}

// ============================================================================

XclImpPivotTable::XclImpPivotTable( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot ),
    mpPCache( 0 ),

    maDataOrientField( *this, EXC_SXIVD_DATA ),
    mpCurrField( 0 )
{
}

XclImpPivotTable::~XclImpPivotTable()
{
}

// cache/field access, misc. --------------------------------------------------

const XclImpPTField* XclImpPivotTable::GetField( sal_uInt16 nFieldIdx ) const
{
    return (nFieldIdx == EXC_SXIVD_DATA) ? &maDataOrientField : maFieldList.GetObject( nFieldIdx );
}

XclImpPTField* XclImpPivotTable::GetFieldAcc( sal_uInt16 nFieldIdx )
{
    return maFieldList.GetObject( nFieldIdx );  // do not return maDataOrientField
}

const String& XclImpPivotTable::GetFieldName( sal_uInt16 nFieldIdx ) const
{
    if( const XclImpPTField* pField = GetField( nFieldIdx ) )
        return pField->GetFieldName();
    return EMPTY_STRING;
}

const XclImpPTField* XclImpPivotTable::GetDataField( sal_uInt16 nDataFieldIdx ) const
{
    if( nDataFieldIdx < maOrigDataFields.size() )
        return GetField( maOrigDataFields[ nDataFieldIdx ] );
    return 0;
}

const String& XclImpPivotTable::GetDataFieldName( sal_uInt16 nDataFieldIdx ) const
{
    if( const XclImpPTField* pField = GetDataField( nDataFieldIdx ) )
        return pField->GetFieldName();
    return EMPTY_STRING;
}

// records --------------------------------------------------------------------

void XclImpPivotTable::ReadSxview( XclImpStream& rStrm )
{
    rStrm >> maPTInfo;

    maOutputRange.aStart.Set( static_cast< SCCOL >( maPTInfo.mnFirstCol ), static_cast< SCROW >( maPTInfo.mnFirstRow ), GetCurrScTab() );
    maOutputRange.aEnd.Set( static_cast< SCCOL >( maPTInfo.mnLastCol ), static_cast< SCROW >( maPTInfo.mnLastRow ), GetCurrScTab() );
    CheckCellRange( maOutputRange );

    mpPCache = GetPivotTableManager().GetPivotCache( maPTInfo.mnCacheIdx );
    mpCurrField = 0;
}

void XclImpPivotTable::ReadSxvd( XclImpStream& rStrm )
{
    if( maFieldList.Count() < EXC_PT_MAXFIELDCOUNT )
    {
        // cache index for the field is equal to the SXVD record index
        sal_uInt16 nCacheIdx = static_cast< sal_uInt16 >( maFieldList.Count() );

        mpCurrField = new XclImpPTField( *this, nCacheIdx );
        maFieldList.Append( mpCurrField );
        mpCurrField->ReadSxvd( rStrm );
    }
    else
        mpCurrField = 0;
}

void XclImpPivotTable::ReadSxvi( XclImpStream& rStrm )
{
    if( mpCurrField )
        mpCurrField->ReadSxvi( rStrm );
}

void XclImpPivotTable::ReadSxvdex( XclImpStream& rStrm )
{
    if( mpCurrField )
        mpCurrField->ReadSxvdex( rStrm );
}

void XclImpPivotTable::ReadSxivd( XclImpStream& rStrm )
{
    mpCurrField = 0;

    // find the index vector to fill (row SXIVD doesn't exist without row fields)
    ScfUInt16Vec* pFieldVec = 0;
    if( maRowFields.empty() && (maPTInfo.mnRowFields > 0) )
        pFieldVec = &maRowFields;
    else if( maColFields.empty() && (maPTInfo.mnColFields > 0) )
        pFieldVec = &maColFields;

    // fill the vector from record data
    if( pFieldVec )
    {
        sal_uInt16 nSize = ::ulimit< sal_uInt16 >( rStrm.GetRecSize() / 2, EXC_PT_MAXROWCOLCOUNT );
        pFieldVec->reserve( nSize );
        for( sal_uInt16 nIdx = 0; nIdx < nSize; ++nIdx )
        {
            sal_uInt16 nFieldIdx;
            rStrm >> nFieldIdx;
            pFieldVec->push_back( nFieldIdx );

            // set orientation at special data orientation field
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
    mpCurrField = 0;

    sal_uInt16 nSize = ::ulimit< sal_uInt16 >( rStrm.GetRecSize() / 6 );
    for( sal_uInt16 nEntry = 0; nEntry < nSize; ++nEntry )
    {
        XclPTPageFieldInfo aPageInfo;
        rStrm >> aPageInfo;
        if( XclImpPTField* pField = GetFieldAcc( aPageInfo.mnField ) )
        {
            maPageFields.push_back( aPageInfo.mnField );
            pField->SetPageFieldInfo( aPageInfo );
        }
        GetObjectManager().SetSkipObj( GetCurrScTab(), aPageInfo.mnObjId );
    }
}

void XclImpPivotTable::ReadSxdi( XclImpStream& rStrm )
{
    mpCurrField = 0;

    XclPTDataFieldInfo aDataInfo;
    rStrm >> aDataInfo;
    if( XclImpPTField* pField = GetFieldAcc( aDataInfo.mnField ) )
    {
        maOrigDataFields.push_back( aDataInfo.mnField );
        // DataPilot does not support double data fields -> add first appearence to index list only
        if( !pField->HasDataFieldInfo() )
            maFiltDataFields.push_back( aDataInfo.mnField );
        pField->AddDataFieldInfo( aDataInfo );
    }
}

void XclImpPivotTable::ReadSxex( XclImpStream& rStrm )
{
    rStrm >> maPTExtInfo;
}

// ----------------------------------------------------------------------------

void XclImpPivotTable::Apply() const
{
    if( !mpPCache || !mpPCache->IsSelfRef() || (mpPCache->GetSourceType() != EXC_SXVS_SHEET) )
        return;

    ScRange aSrcRange( mpPCache->GetSourceRange() );
    SCTAB nScTab = GetTabInfo().GetScTabFromXclName( mpPCache->GetTabName() );
    if( nScTab == SCNOTAB )
        return;

    ScDPSaveData aSaveData;

    // *** global settings ***

    aSaveData.SetRowGrand( ::get_flag( maPTInfo.mnFlags, EXC_SXVIEW_ROWGRAND ) );
    aSaveData.SetColumnGrand( ::get_flag( maPTInfo.mnFlags, EXC_SXVIEW_COLGRAND ) );
    aSaveData.SetFilterButton( FALSE );
    aSaveData.SetDrillDown( ::get_flag( maPTExtInfo.mnFlags, EXC_SXEX_DRILLDOWN ) );

    // *** fields ***

    ScfUInt16Vec::const_iterator aIt, aEnd;

    // row fields
    for( aIt = maRowFields.begin(), aEnd = maRowFields.end(); aIt != aEnd; ++aIt )
        if( const XclImpPTField* pField = GetField( *aIt ) )
            pField->ApplyRowColField( aSaveData );

    // column fields
    for( aIt = maColFields.begin(), aEnd = maColFields.end(); aIt != aEnd; ++aIt )
        if( const XclImpPTField* pField = GetField( *aIt ) )
            pField->ApplyRowColField( aSaveData );

    // page fields
    for( aIt = maPageFields.begin(), aEnd = maPageFields.end(); aIt != aEnd; ++aIt )
        if( const XclImpPTField* pField = GetField( *aIt ) )
            pField->ApplyPageField( aSaveData );

    // data fields
    for( aIt = maFiltDataFields.begin(), aEnd = maFiltDataFields.end(); aIt != aEnd; ++aIt )
        if( const XclImpPTField* pField = GetField( *aIt ) )
            pField->ApplyDataField( aSaveData );

    // *** insert into Calc document ***

    // create source descriptor
    aSrcRange.aStart.SetTab( nScTab );
    aSrcRange.aEnd.SetTab( nScTab );
    ScSheetSourceDesc aDesc;
    aDesc.aSourceRange = aSrcRange;

    // adjust output range to include the page fields
    ScRange aOutRange( maOutputRange );
    if( !maPageFields.empty() )
    {
        SCsROW nDecRows = ::std::min< SCsROW >( maOutputRange.aStart.Row(), maPageFields.size() + 1 );
        aOutRange.aStart.IncRow( -nDecRows );
    }

    // create the DataPilot
    ScDPObject* pDPObj = new ScDPObject( GetDocPtr() );
    pDPObj->SetName( maPTInfo.maTableName );
    pDPObj->SetSaveData( aSaveData );
    pDPObj->SetSheetDesc( aDesc );
    pDPObj->SetOutRange( aOutRange );
    pDPObj->SetAlive( TRUE );
    GetDoc().GetDPCollection()->Insert( pDPObj );
}

// ============================================================================
// ============================================================================

XclImpPivotTableManager::XclImpPivotTableManager( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot )
{
}

XclImpPivotTableManager::~XclImpPivotTableManager()
{
}

// access functions -----------------------------------------------------------

const XclImpPivotCache* XclImpPivotTableManager::GetPivotCache( sal_uInt16 nCacheIdx ) const
{
    return maPCacheList.GetObject( nCacheIdx );
}

// pivot cache records --------------------------------------------------------

void XclImpPivotTableManager::ReadSxidstm( XclImpStream& rStrm )
{
    sal_uInt16 nStrmId;
    rStrm >> nStrmId;
    maPCacheList.Append( new XclImpPivotCache( GetRoot(), nStrmId ) );
}

void XclImpPivotTableManager::ReadDconref( XclImpStream& rStrm )
{
    if( !maPCacheList.Empty() )
        maPCacheList.Last()->ReadDconref( rStrm );
}

void XclImpPivotTableManager::ReadSxvs( XclImpStream& rStrm )
{
    if( !maPCacheList.Empty() )
        maPCacheList.Last()->ReadSxvs( rStrm );
}

// pivot table records --------------------------------------------------------

void XclImpPivotTableManager::ReadSxview( XclImpStream& rStrm )
{
    XclImpPivotTable* pPTable = new XclImpPivotTable( GetRoot() );
    maPTableList.Append( pPTable );
    pPTable->ReadSxview( rStrm );
}

void XclImpPivotTableManager::ReadSxvd( XclImpStream& rStrm )
{
    if( !maPTableList.Empty() )
        maPTableList.Last()->ReadSxvd( rStrm );
}

void XclImpPivotTableManager::ReadSxvdex( XclImpStream& rStrm )
{
    if( !maPTableList.Empty() )
        maPTableList.Last()->ReadSxvdex( rStrm );
}

void XclImpPivotTableManager::ReadSxivd( XclImpStream& rStrm )
{
    if( !maPTableList.Empty() )
        maPTableList.Last()->ReadSxivd( rStrm );
}

void XclImpPivotTableManager::ReadSxpi( XclImpStream& rStrm )
{
    if( !maPTableList.Empty() )
        maPTableList.Last()->ReadSxpi( rStrm );
}

void XclImpPivotTableManager::ReadSxdi( XclImpStream& rStrm )
{
    if( !maPTableList.Empty() )
        maPTableList.Last()->ReadSxdi( rStrm );
}

void XclImpPivotTableManager::ReadSxvi( XclImpStream& rStrm )
{
    if( !maPTableList.Empty() )
        maPTableList.Last()->ReadSxvi( rStrm );
}

void XclImpPivotTableManager::ReadSxex( XclImpStream& rStrm )
{
    if( !maPTableList.Empty() )
        maPTableList.Last()->ReadSxex( rStrm );
}

// ----------------------------------------------------------------------------

void XclImpPivotTableManager::Apply() const
{
    for( const XclImpPivotTable* pPTable = maPTableList.First(); pPTable; pPTable = maPTableList.Next() )
        pPTable->Apply();
}

// ============================================================================

