/*************************************************************************
 *
 *  $RCSfile: xepivot.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 12:54:01 $
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

#include "root.hxx"
extern const sal_Char*  pPivotCacheStorageName;

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

const sal_uInt16 EXC_PCITEM_TYPE_STRING     = 0x0001;
const sal_uInt16 EXC_PCITEM_TYPE_DOUBLE     = 0x0002;
const sal_uInt16 EXC_PCITEM_TYPE_INTEGER    = 0x0004;
const sal_uInt16 EXC_PCITEM_TYPE_DATE       = 0x0008;

static const sal_uInt16 spnPCItemFlags[] =
{               // STR DBL INT DAT
    0x0000,     //
    0x0480,     //  x
    0x0520,     //      x
    0x05A0,     //  x   x
    0x0560,     //          x
    0x05E0,     //  x       x
    0x0520,     //      x   x
    0x05A0,     //  x   x   x
    0x0900,     //              x
    0x0D80,     //  x           x
    0x0D00,     //      x       x
    0x0D80,     //  x   x       x
    0x0D00,     //          x   x
    0x0D80,     //  x       x   x
    0x0D00,     //      x   x   x
    0x0D80      //  x   x   x   x
};

} // namespace

// ----------------------------------------------------------------------------

XclExpPCItem::XclExpPCItem( const String& rText ) :
    XclExpRecord( rText.Len() ? EXC_ID_SXSTRING : EXC_ID_SXEMPTY, 0 ),
    mnTypeFlag( EXC_PCITEM_TYPE_STRING )
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
        mnTypeFlag = EXC_PCITEM_TYPE_DATE;
    }
    else
    {
        SetValue( fValue );
        mnTypeFlag = (fValue - floor( fValue ) == 0.0) ?
            EXC_PCITEM_TYPE_INTEGER : EXC_PCITEM_TYPE_DOUBLE;
    }
}

XclExpPCItem::XclExpPCItem( bool bValue ) :
    XclExpRecord( EXC_ID_SXBOOLEAN, 2 ),
    mnTypeFlag( EXC_PCITEM_TYPE_STRING )
{
    SetBool( bValue );
}

// ----------------------------------------------------------------------------

bool XclExpPCItem::EqualsText( const String& rText ) const
{
    return (rText.Len() && GetText() && (*GetText() == rText)) || (!rText.Len() && IsEmpty());
}

bool XclExpPCItem::EqualsValue( double fValue ) const
{
    return GetValue() && (*GetValue() == fValue);
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
    else if( const double* pfValue = GetValue() )
    {
        rStrm << *pfValue;
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
    // nothing to do for SXEMPTY
}

// ============================================================================

XclExpPCField::XclExpPCField( const XclExpRoot& rRoot, const ScRange& rRange ) :
    XclExpRecord( EXC_ID_SXFIELD ),
    mnFlags( EXC_SXFIELD_DEFAULTFLAGS ),
    mnTypeFlags( 0 )
{
    ScDocument& rDoc = rRoot.GetDoc();
    SvNumberFormatter& rFormatter = rRoot.GetFormatter();

    ScAddress aPos( rRange.aStart );
    rDoc.GetString( aPos.Col(), aPos.Row(), aPos.Tab(), maName );

    for( aPos.IncRow(); (aPos.Row() <= rRange.aEnd.Row()) && (maItemList.Count() < EXC_PC_MAXITEMCOUNT); aPos.IncRow() )
    {
        if( rDoc.HasValueData( aPos.Col(), aPos.Row(), aPos.Tab() ) )
        {
            double fValue = rDoc.GetValue( aPos );
            short nFmtType = rFormatter.GetType( rDoc.GetNumberFormat( aPos ) );
            if( nFmtType == NUMBERFORMAT_LOGICAL )
                InsertBoolItem( fValue != 0 );
            else if( nFmtType & NUMBERFORMAT_DATETIME )
                InsertDateItem( fValue );
            else
                InsertValueItem( fValue );
        }
        else
        {
            String aText;
            rDoc.GetString( aPos.Col(), aPos.Row(), aPos.Tab(), aText );
            InsertTextItem( aText );
        }
    }

    /*  mnTypeFlags is updated in all Insert***Item() functions. Now the flags
        for the current combination of item types is added to the flags. */
    mnFlags |= spnPCItemFlags[ mnTypeFlags ];
}

sal_uInt16 XclExpPCField::GetItemCount() const
{
    return static_cast< sal_uInt16 >( maItemList.Count() );
}

sal_uInt32 XclExpPCField::GetIndexSize() const
{
    return ::get_flag( mnFlags, EXC_SXFIELD_16BIT ) ? 2 : 1;
}

const XclExpPCItem* XclExpPCField::GetItem( sal_uInt16 nItemIdx ) const
{
    return maItemList.GetObject( nItemIdx );
}

void XclExpPCField::WriteIndex( XclExpStream& rStrm, sal_uInt16 nPos ) const
{
    sal_uInt16 nIndex = (nPos < maIndexVec.size()) ? maIndexVec[ nPos ] : 0;
    if( ::get_flag( mnFlags, EXC_SXFIELD_16BIT ) )
        rStrm << nIndex;
    else
        rStrm << static_cast< sal_uInt8 >( nIndex );
}

void XclExpPCField::Save( XclExpStream& rStrm )
{
    // SXFIELD
    XclExpRecord::Save( rStrm );
    // SXFDBTYPE
    XclExpUInt16Record( EXC_ID_SXFDBTYPE, EXC_SXFDBTYPE_DEFAULT ).Save( rStrm );
    // list of items
    maItemList.Save( rStrm );
}

// private --------------------------------------------------------------------

void XclExpPCField::InsertIndex( ULONG nListPos )
{
    maIndexVec.push_back( static_cast< sal_uInt16 >( nListPos ) );
}

void XclExpPCField::InsertNewItem( XclExpPCItem* pNewItem )
{
    InsertIndex( maItemList.Count() );
    maItemList.Append( pNewItem );
    if( maItemList.Count() == 0x0100 )   // 16-bit column in index list
        ::set_flag( mnFlags, EXC_SXFIELD_16BIT );
    mnTypeFlags |= pNewItem->GetTypeFlag();
}

void XclExpPCField::InsertTextItem( const String& rText )
{
    bool bFound = false;
    for( const XclExpPCItem* pItem = maItemList.First(); !bFound && pItem; pItem = maItemList.Next() )
        bFound = pItem->EqualsText( rText );
    if( bFound )
        InsertIndex( maItemList.GetCurPos() );
    else
        InsertNewItem( new XclExpPCItem( rText ) );
}

void XclExpPCField::InsertValueItem( double fValue )
{
    bool bFound = false;
    for( const XclExpPCItem* pItem = maItemList.First(); !bFound && pItem; pItem = maItemList.Next() )
        bFound = pItem->EqualsValue( fValue );
    if( bFound )
        InsertIndex( maItemList.GetCurPos() );
    else
        InsertNewItem( new XclExpPCItem( fValue, false ) );
}

void XclExpPCField::InsertDateItem( double fDate )
{
    bool bFound = false;
    for( const XclExpPCItem* pItem = maItemList.First(); !bFound && pItem; pItem = maItemList.Next() )
        bFound = pItem->EqualsDate( fDate );
    if( bFound )
        InsertIndex( maItemList.GetCurPos() );
    else
        InsertNewItem( new XclExpPCItem( fDate, true ) );
}

void XclExpPCField::InsertBoolItem( bool bValue )
{
    bool bFound = false;
    for( const XclExpPCItem* pItem = maItemList.First(); !bFound && pItem; pItem = maItemList.Next() )
        bFound = pItem->EqualsBool( bValue );
    if( bFound )
        InsertIndex( maItemList.GetCurPos() );
    else
        InsertNewItem( new XclExpPCItem( bValue ) );
}

void XclExpPCField::WriteBody( XclExpStream& rStrm )
{
    rStrm   << mnFlags
            << sal_uInt32( 0 )                  // unknown
            << GetItemCount()
            << sal_uInt32( 0 )                  // unknown
            << GetItemCount()
            << XclExpString( maName );
}

// ============================================================================

XclExpPivotCache::XclExpPivotCache( const XclExpRoot& rRoot,
        const ScRange& rSrcRange, sal_uInt16 nListIdx ) :
    XclExpRoot( rRoot ),
    maOrigSrcRange( rSrcRange ),
    maSrcRange( rSrcRange ),
    mnStrmId( nListIdx + 1 ),
    mnListIdx( nListIdx ),
    mnSrcRecs( 0 ),
    mbValid( false )
{
    // internal sheet data only
    SCTAB nScTab = maSrcRange.aStart.Tab();
    if( (nScTab == maSrcRange.aEnd.Tab()) && GetTabInfo().IsExportTab( nScTab ) )
    {
        // #i22541# skip empty cell areas (performance)
        SCCOL nFirstDocCol, nLastDocCol;
        SCROW nFirstDocRow, nLastDocRow;
        GetDoc().GetDataStart( nScTab, nFirstDocCol, nFirstDocRow );
        GetDoc().GetPrintArea( nScTab, nLastDocCol, nLastDocRow, false );
        SCCOL nFirstSrcCol = maSrcRange.aStart.Col(), nLastSrcCol = maSrcRange.aEnd.Col();
        SCCOL nFirstSrcRow = maSrcRange.aStart.Row(), nLastSrcRow = maSrcRange.aEnd.Row();

        // adjust row indexes, keep one row of empty area to add an empty cache item
        if( nFirstSrcRow < nFirstDocRow )
            nFirstSrcRow = nFirstDocRow - 1;
        if( nLastSrcRow > nLastDocRow )
            nLastSrcRow = nLastDocRow + 1;

        maSrcRange.aStart.SetCol( ::std::max( nFirstDocCol, nFirstSrcCol ) );
        maSrcRange.aStart.SetRow( nFirstSrcRow );
        maSrcRange.aEnd.SetCol( ::std::min( nLastDocCol, nLastSrcCol ) );
        maSrcRange.aEnd.SetRow( nLastSrcRow );

        if( (maSrcRange.aStart.Col() <= maSrcRange.aEnd.Col()) &&
            (maSrcRange.aStart.Row() <= maSrcRange.aEnd.Row()) &&
            CheckCellRange( maSrcRange ) )
        {
            GetDoc().GetName( nScTab, maTabName );
            mnSrcRecs = static_cast< sal_uInt16 >( maSrcRange.aEnd.Row() - maSrcRange.aStart.Row() );
            mbValid = true;

            for( SCCOL nCol = maSrcRange.aStart.Col(), nEndCol = maSrcRange.aEnd.Col(); nCol <= nEndCol; ++nCol )
            {
                ScRange aColRange(
                    nCol, maSrcRange.aStart.Row(), maSrcRange.aStart.Tab(),
                    nCol, maSrcRange.aEnd.Row(), maSrcRange.aEnd.Tab() );
                maFieldList.Append( new XclExpPCField( GetRoot(), aColRange ) );
            }
        }
    }
}

sal_uInt16 XclExpPivotCache::GetFieldCount() const
{
    return static_cast< sal_uInt16 >( maFieldList.Count() );
}

const XclExpPCField* XclExpPivotCache::GetField( sal_uInt16 nFieldIdx ) const
{
    return maFieldList.GetObject( nFieldIdx );
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
    XclExpUInt16Record( EXC_ID_SXIDSTM, mnStrmId ).Save( rStrm );
    // SXVS
    XclExpUInt16Record( EXC_ID_SXVS, EXC_SXVS_SHEET ).Save( rStrm );
    // DCONREF
    WriteDconref( rStrm );
    // create the pivot cache storage stream
    WriteCacheStream();
}

// private --------------------------------------------------------------------

void XclExpPivotCache::WriteDconref( XclExpStream& rStrm ) const
{
    XclExpString aRef( XclExpUrlHelper::EncodeUrl( GetRoot(), EMPTY_STRING, &maTabName ) );
    rStrm.StartRecord( EXC_ID_DCONREF, 7 + aRef.GetSize() );
    rStrm   << static_cast< sal_uInt16 >( maSrcRange.aStart.Row() )
            << static_cast< sal_uInt16 >( maSrcRange.aEnd.Row() )
            << static_cast< sal_uInt8 >( maSrcRange.aStart.Col() )
            << static_cast< sal_uInt8 >( maSrcRange.aEnd.Col() )
            << aRef
            << sal_uInt8( 0 );
    rStrm.EndRecord();
}

void XclExpPivotCache::WriteCacheStream()
{
    if( !mpRD->pPivotCacheStorage )
        mpRD->pPivotCacheStorage =
            GetRootStorage()->OpenStorage(
                String::CreateFromAscii(pPivotCacheStorageName), STREAM_STD_WRITE );
    SvStorageStreamRef xSvStrm = mpRD->pPivotCacheStorage->OpenStream(
        ScfTools::GetHexStr( mnStrmId ), STREAM_READWRITE | STREAM_TRUNC );
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
    rStrm   << sal_uInt32( mnSrcRecs )
            << mnStrmId
            << EXC_SXDB_DEFAULTFLAGS
            << EXC_SXDB_BLOCKRECS
            << GetFieldCount()              // base fields only
            << GetFieldCount()              // base, grouped, calc'ed fields
            << sal_uInt16( 0 )              // not used
            << EXC_SXDB_SRC_SHEET
            << XclExpString();              // user name
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
    sal_uInt32 nRecSize = 0;
    for( const XclExpPCField* pField = maFieldList.First(); pField; pField = maFieldList.Next() )
        nRecSize += pField->GetIndexSize();

    for( sal_uInt16 nRow = 0; nRow < mnSrcRecs; ++nRow )
    {
        rStrm.StartRecord( EXC_ID_SXIDARRAY, nRecSize );
        for( const XclExpPCField* pField = maFieldList.First(); pField; pField = maFieldList.Next() )
            pField->WriteIndex( rStrm, nRow );
        rStrm.EndRecord();
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
    maItemInfo.mbUseCache = mpCacheItem != 0;
}

XclExpPTItem::XclExpPTItem( sal_uInt16 nItemType, sal_uInt16 nCacheIdx, bool bUseCache ) :
    XclExpRecord( EXC_ID_SXVI, 8 ),
    mpCacheItem( 0 )
{
    maItemInfo.mnType = nItemType;
    maItemInfo.mnCacheIdx = nCacheIdx;
    maItemInfo.mbUseCache = bUseCache;
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
    return mpCacheField ? mpCacheField->GetName() : EMPTY_STRING;
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
    rDataInfo.maVisName = lclGetDataFieldCaption( GetFieldName(), eFunc );

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

XclExpPivotTable::XclExpPivotTable( const XclExpRoot& rRoot,
        const XclExpPivotCache& rPCache, const ScDPObject& rDPObj ) :
    XclExpRoot( rRoot ),
    mrPCache( rPCache ),
    maDataOrientField( *this, EXC_SXIVD_DATA ),
    maOutputRange( rDPObj.GetOutRange() ),
    mbFilterBtn( false )
{
    mbValid = CheckCellRange( maOutputRange );
    if( mbValid )
    {
        Initialize( rDPObj );

        if( const ScDPSaveData* pSaveData = rDPObj.GetSaveData() )
        {
            // DataPilot properties -------------------------------------------

            SetPropertiesFromDP( *pSaveData );

            // loop over all dimensions ---------------------------------------

            const List& rDimList = pSaveData->GetDimensions();
            ULONG nDimIdx, nDimCount = rDimList.Count();

            /*  First process all data dimensions, they are needed for extended
                settings of row/column/page fields (sorting/auto show). */
            for( nDimIdx = 0; nDimIdx < nDimCount; ++nDimIdx )
                if( const ScDPSaveDimension* pSaveDim = static_cast< const ScDPSaveDimension* >( rDimList.GetObject( nDimIdx ) ) )
                    if( pSaveDim->GetOrientation() == DataPilotFieldOrientation_DATA )
                        SetDataFieldPropertiesFromDim( *pSaveDim );

            /*  Row/column/page/hidden fields. */
            for( nDimIdx = 0; nDimIdx < nDimCount; ++nDimIdx )
                if( const ScDPSaveDimension* pSaveDim = static_cast< const ScDPSaveDimension* >( rDimList.GetObject( nDimIdx ) ) )
                    if( pSaveDim->GetOrientation() != DataPilotFieldOrientation_DATA )
                        SetFieldPropertiesFromDim( *pSaveDim );
        }

        Finalize();
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

void XclExpPivotTable::Initialize( const ScDPObject& rDPObj )
{
    // default-construct all pivot table fields according to cache
    for( sal_uInt16 nFieldIdx = 0, nFieldCount = mrPCache.GetFieldCount(); nFieldIdx < nFieldCount; ++nFieldIdx )
        maFieldList.Append( new XclExpPTField( *this, nFieldIdx ) );

    // pivot table properties from DP object
    maPTInfo.maTableName = rDPObj.GetName();
    maPTInfo.maDataName = ScGlobal::GetRscString( STR_PIVOT_DATA );
    maPTInfo.mnCacheIdx = mrPCache.GetCacheIndex();
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
    XclExpRoot( rRoot )
{
}

void XclExpPivotTableManager::CreatePivotTables()
{
    if( ScDPCollection* pDPColl = GetDoc().GetDPCollection() )
        for( USHORT nDPObj = 0, nCount = pDPColl->GetCount(); nDPObj < nCount; ++nDPObj )
            if( ScDPObject* pDPObj = (*pDPColl)[ nDPObj ] )
                if( const XclExpPivotCache* pCache = CreatePivotCache( *pDPObj ) )
                    maPTableList.Append( new XclExpPivotTable( GetRoot(), *pCache, *pDPObj ) );
}

void XclExpPivotTableManager::WritePivotCaches( XclExpStream& rStrm )
{
    maPCacheList.Save( rStrm );
}

void XclExpPivotTableManager::WritePivotTables( XclExpStream& rStrm, SCTAB nScTab )
{
    for( XclExpPivotTable* pPTable = maPTableList.First(); pPTable; pPTable = maPTableList.Next() )
        if( pPTable->IsValid() && (pPTable->GetScTab() == nScTab) )
            pPTable->Save( rStrm );
}

// private --------------------------------------------------------------------

const XclExpPivotCache* XclExpPivotTableManager::CreatePivotCache( const ScDPObject& rDPObj, bool bUseExisting )
{
    // try to find a pivot cache with the same data source
    if( bUseExisting )
        for( const XclExpPivotCache* pCache = maPCacheList.First(); pCache; pCache = maPCacheList.Next() )
            if( pCache->HasEqualDataSource( rDPObj ) )
                return pCache;

    // create a new pivot cache
    if( const ScSheetSourceDesc* pSrcDesc = rDPObj.GetSheetDesc() )
    {
        sal_uInt16 nNewCacheIdx = static_cast< sal_uInt16 >( maPCacheList.Count() );
        ::std::auto_ptr< XclExpPivotCache > xNewCache(
            new XclExpPivotCache( GetRoot(), pSrcDesc->aSourceRange, nNewCacheIdx ) );
        if( xNewCache->IsValid() )
        {
            maPCacheList.Append( xNewCache.release() );
            return maPCacheList.Last();
        }
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

