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

#include "oox/xls/pivottablebuffer.hxx"
#include <set>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sheet/CellFlags.hpp>
#include <com/sun/star/sheet/DataPilotFieldAutoShowInfo.hpp>
#include <com/sun/star/sheet/DataPilotFieldLayoutInfo.hpp>
#include <com/sun/star/sheet/DataPilotFieldLayoutMode.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/DataPilotFieldReference.hpp>
#include <com/sun/star/sheet/DataPilotFieldReferenceItemType.hpp>
#include <com/sun/star/sheet/DataPilotFieldReferenceType.hpp>
#include <com/sun/star/sheet/DataPilotFieldShowItemsMode.hpp>
#include <com/sun/star/sheet/DataPilotFieldSortInfo.hpp>
#include <com/sun/star/sheet/DataPilotFieldSortMode.hpp>
#include <com/sun/star/sheet/GeneralFunction.hpp>
#include <com/sun/star/sheet/XDataPilotDataLayoutFieldSupplier.hpp>
#include <com/sun/star/sheet/XDataPilotField.hpp>
#include <com/sun/star/sheet/XDataPilotTablesSupplier.hpp>
#include <com/sun/star/sheet/XSheetOperation.hpp>
#include "properties.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/helper/recordinputstream.hxx"
#include "oox/xls/addressconverter.hxx"
#include "oox/xls/biffinputstream.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::UNO_SET_THROW;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::container::XIndexAccess;
using ::com::sun::star::container::XNameAccess;
using ::com::sun::star::container::XNamed;
using ::com::sun::star::table::CellAddress;
using ::com::sun::star::sheet::DataPilotFieldOrientation;
using ::com::sun::star::sheet::XDataPilotDataLayoutFieldSupplier;
using ::com::sun::star::sheet::XDataPilotDescriptor;
using ::com::sun::star::sheet::XDataPilotField;
using ::com::sun::star::sheet::XDataPilotTables;
using ::com::sun::star::sheet::XDataPilotTablesSupplier;
using ::com::sun::star::sheet::XSheetOperation;

namespace oox {
namespace xls {

// ============================================================================

namespace {

const sal_Int32 OOX_PT_DATALAYOUTFIELD              = -2;           /// Placeholder index of data layout field.

const sal_Int32 OOX_PT_PREVIOUS_ITEM                = 0x001000FC;   /// Calculation of data item result is based on previous item.
const sal_Int32 OOX_PT_NEXT_ITEM                    = 0x001000FD;   /// Calculation of data item result is based on next item.

// ----------------------------------------------------------------------------

const sal_uInt32 OOBIN_PTFIELD_DATAFIELD            = 0x00000008;
const sal_uInt32 OOBIN_PTFIELD_DEFAULT              = 0x00000100;
const sal_uInt32 OOBIN_PTFIELD_SUM                  = 0x00000200;
const sal_uInt32 OOBIN_PTFIELD_COUNTA               = 0x00000400;
const sal_uInt32 OOBIN_PTFIELD_AVERAGE              = 0x00000800;
const sal_uInt32 OOBIN_PTFIELD_MAX                  = 0x00001000;
const sal_uInt32 OOBIN_PTFIELD_MIN                  = 0x00002000;
const sal_uInt32 OOBIN_PTFIELD_PRODUCT              = 0x00004000;
const sal_uInt32 OOBIN_PTFIELD_COUNT                = 0x00008000;
const sal_uInt32 OOBIN_PTFIELD_STDDEV               = 0x00010000;
const sal_uInt32 OOBIN_PTFIELD_STDDEVP              = 0x00020000;
const sal_uInt32 OOBIN_PTFIELD_VAR                  = 0x00040000;
const sal_uInt32 OOBIN_PTFIELD_VARP                 = 0x00080000;

const sal_uInt32 OOBIN_PTFIELD_SHOWALL              = 0x00000020;
const sal_uInt32 OOBIN_PTFIELD_OUTLINE              = 0x00000040;
const sal_uInt32 OOBIN_PTFIELD_INSERTBLANKROW       = 0x00000080;
const sal_uInt32 OOBIN_PTFIELD_SUBTOTALTOP          = 0x00000100;
const sal_uInt32 OOBIN_PTFIELD_INSERTPAGEBREAK      = 0x00000800;
const sal_uInt32 OOBIN_PTFIELD_AUTOSORT             = 0x00001000;
const sal_uInt32 OOBIN_PTFIELD_SORTASCENDING        = 0x00002000;
const sal_uInt32 OOBIN_PTFIELD_AUTOSHOW             = 0x00004000;
const sal_uInt32 OOBIN_PTFIELD_AUTOSHOWTOP          = 0x00008000;
const sal_uInt32 OOBIN_PTFIELD_MULTIPAGEITEMS       = 0x00080000;

const sal_uInt16 OOBIN_PTFITEM_HIDDEN               = 0x0001;
const sal_uInt16 OOBIN_PTFITEM_HIDEDETAILS          = 0x0002;

const sal_uInt8 OOBIN_PTPAGEFIELD_HASNAME           = 0x01;
const sal_uInt8 OOBIN_PTPAGEFIELD_HASOLAPCAPTION    = 0x02;
const sal_Int32 OOBIN_PTPAGEFIELD_MULTIITEMS        = 0x001000FE;

const sal_uInt16 OOBIN_PTFILTER_HASNAME             = 0x0001;
const sal_uInt16 OOBIN_PTFILTER_HASDESCRIPTION      = 0x0002;
const sal_uInt16 OOBIN_PTFILTER_HASSTRVALUE1        = 0x0004;
const sal_uInt16 OOBIN_PTFILTER_HASSTRVALUE2        = 0x0008;

const sal_uInt8 OOBIN_TOP10FILTER_TOP               = 0x01;
const sal_uInt8 OOBIN_TOP10FILTER_PERCENT           = 0x02;

const sal_uInt32 OOBIN_PTDEF_SHOWITEMS              = 0x00000100;
const sal_uInt32 OOBIN_PTDEF_DISABLEFIELDLIST       = 0x00000400;
const sal_uInt32 OOBIN_PTDEF_HIDECALCMEMBERS        = 0x00001000;
const sal_uInt32 OOBIN_PTDEF_WITHHIDDENTOTALS       = 0x00002000;
const sal_uInt32 OOBIN_PTDEF_HIDEDRILL              = 0x00100000;
const sal_uInt32 OOBIN_PTDEF_PRINTDRILL             = 0x00200000;
const sal_uInt32 OOBIN_PTDEF_HIDEHEADERS            = 0x80000000;

const sal_uInt32 OOBIN_PTDEF_SHOWEMPTYROW           = 0x00000004;
const sal_uInt32 OOBIN_PTDEF_SHOWEMPTYCOL           = 0x00000008;
const sal_uInt32 OOBIN_PTDEF_ENABLEDRILL            = 0x00000020;
const sal_uInt32 OOBIN_PTDEF_PRESERVEFORMATTING     = 0x00000080;
const sal_uInt32 OOBIN_PTDEF_SHOWERROR              = 0x00000200;
const sal_uInt32 OOBIN_PTDEF_SHOWMISSING            = 0x00000400;
const sal_uInt32 OOBIN_PTDEF_PAGEOVERTHENDOWN       = 0x00000800;
const sal_uInt32 OOBIN_PTDEF_SUBTOTALHIDDENITEMS    = 0x00001000;
const sal_uInt32 OOBIN_PTDEF_ROWGRANDTOTALS         = 0x00002000;
const sal_uInt32 OOBIN_PTDEF_COLGRANDTOTALS         = 0x00004000;
const sal_uInt32 OOBIN_PTDEF_FIELDPRINTTITLES       = 0x00008000;
const sal_uInt32 OOBIN_PTDEF_ITEMPRINTTITLES        = 0x00020000;
const sal_uInt32 OOBIN_PTDEF_MERGEITEM              = 0x00040000;
const sal_uInt32 OOBIN_PTDEF_HASDATACAPTION         = 0x00080000;
const sal_uInt32 OOBIN_PTDEF_HASGRANDTOTALCAPTION   = 0x00100000;
const sal_uInt32 OOBIN_PTDEF_HASPAGESTYLE           = 0x00200000;
const sal_uInt32 OOBIN_PTDEF_HASPIVOTTABLESTYLE     = 0x00400000;
const sal_uInt32 OOBIN_PTDEF_HASVACATEDSTYLE        = 0x00800000;
const sal_uInt32 OOBIN_PTDEF_HASTAG                 = 0x40000000;

const sal_uInt32 OOBIN_PTDEF_NOERRORCAPTION         = 0x00000040;
const sal_uInt32 OOBIN_PTDEF_NOMISSINGCAPTION       = 0x00000080;
const sal_uInt32 OOBIN_PTDEF_HASROWHEADERCAPTION    = 0x00000400;
const sal_uInt32 OOBIN_PTDEF_HASCOLHEADERCAPTION    = 0x00000800;
const sal_uInt32 OOBIN_PTDEF_FIELDLISTSORTASC       = 0x00001000;
const sal_uInt32 OOBIN_PTDEF_NOCUSTOMLISTSORT       = 0x00004000;

const sal_uInt8 OOBIN_PTDEF_ROWAXIS                 = 1;
const sal_uInt8 OOBIN_PTDEF_COLAXIS                 = 2;

// ----------------------------------------------------------------------------

const sal_uInt16 BIFF_PT_NOSTRING                   = 0xFFFF;

const sal_uInt16 BIFF_PTFIELD_DATAFIELD             = 0x0008;
const sal_uInt16 BIFF_PTFIELD_DEFAULT               = 0x0001;
const sal_uInt16 BIFF_PTFIELD_SUM                   = 0x0002;
const sal_uInt16 BIFF_PTFIELD_COUNTA                = 0x0004;
const sal_uInt16 BIFF_PTFIELD_AVERAGE               = 0x0008;
const sal_uInt16 BIFF_PTFIELD_MAX                   = 0x0010;
const sal_uInt16 BIFF_PTFIELD_MIN                   = 0x0020;
const sal_uInt16 BIFF_PTFIELD_PRODUCT               = 0x0040;
const sal_uInt16 BIFF_PTFIELD_COUNT                 = 0x0080;
const sal_uInt16 BIFF_PTFIELD_STDDEV                = 0x0100;
const sal_uInt16 BIFF_PTFIELD_STDDEVP               = 0x0200;
const sal_uInt16 BIFF_PTFIELD_VAR                   = 0x0400;
const sal_uInt16 BIFF_PTFIELD_VARP                  = 0x0800;

const sal_uInt32 BIFF_PTFIELD2_SHOWALL              = 0x00000001;
const sal_uInt32 BIFF_PTFIELD2_AUTOSORT             = 0x00000200;
const sal_uInt32 BIFF_PTFIELD2_SORTASCENDING        = 0x00000400;
const sal_uInt32 BIFF_PTFIELD2_AUTOSHOW             = 0x00000800;
const sal_uInt32 BIFF_PTFIELD2_AUTOSHOWTOP          = 0x00001000;
const sal_uInt32 BIFF_PTFIELD2_OUTLINE              = 0x00200000;
const sal_uInt32 BIFF_PTFIELD2_INSERTBLANKROW       = 0x00400000;
const sal_uInt32 BIFF_PTFIELD2_SUBTOTALTOP          = 0x00800000;

const sal_uInt16 BIFF_PTFITEM_HIDDEN                = 0x0001;
const sal_uInt16 BIFF_PTFITEM_HIDEDETAILS           = 0x0002;

const sal_uInt16 BIFF_PTDEF_ROWGRANDTOTALS          = 0x0001;
const sal_uInt16 BIFF_PTDEF_COLGRANDTOTALS          = 0x0002;

const sal_uInt8 BIFF_PTDEF_ROWAXIS                  = 1;
const sal_uInt8 BIFF_PTDEF_COLAXIS                  = 2;

const sal_uInt32 BIFF_PTDEF2_PAGEOVERTHENDOWN       = 0x00000001;
const sal_uInt32 BIFF_PTDE2F_ENABLEDRILL            = 0x00020000;
const sal_uInt32 BIFF_PTDEF2_PRESERVEFORMATTING     = 0x00080000;
const sal_uInt32 BIFF_PTDEF2_MERGEITEM              = 0x00100000;
const sal_uInt32 BIFF_PTDEF2_SHOWERROR              = 0x00200000;
const sal_uInt32 BIFF_PTDEF2_SHOWMISSING            = 0x00400000;
const sal_uInt32 BIFF_PTDEF2_SUBTOTALHIDDENITEMS    = 0x00800000;

const sal_Int16 BIFF_PTPAGEFIELDS_ALLITEMS          = 0x7FFD;

const sal_Int16 BIFF_PTDATAFIELD_PREVIOUS           = 0x7FFB;
const sal_Int16 BIFF_PTDATAFIELD_NEXT               = 0x7FFC;

// ----------------------------------------------------------------------------

OUString lclReadPivotString( const WorkbookHelper& rHelper, BiffInputStream& rStrm, sal_uInt16 nLen )
{
    if( nLen == BIFF_PT_NOSTRING )
        return OUString();
    return (rHelper.getBiff() == BIFF8) ? rStrm.readUniStringBody( nLen ) : rStrm.readCharArrayUC( nLen, rHelper.getTextEncoding() );
}

} // namespace

// ============================================================================

PTFieldItemModel::PTFieldItemModel() :
    mnCacheItem( -1 ),
    mnType( XML_data ),
    mbShowDetails( true ),
    mbHidden( false )
{
}

void PTFieldItemModel::setBinType( sal_uInt16 nType )
{
    static const sal_Int32 spnTypes[] = { XML_data, XML_default,
        XML_sum, XML_countA, XML_avg, XML_max, XML_min, XML_product, XML_count,
        XML_stdDev, XML_stdDevP, XML_var, XML_varP, XML_grand, XML_blank };
    mnType = STATIC_ARRAY_SELECT( spnTypes, nType, XML_data );
}

// ----------------------------------------------------------------------------

PTFieldModel::PTFieldModel() :
    mnAxis( XML_TOKEN_INVALID ),
    mnNumFmtId( 0 ),
    mnAutoShowItems( 10 ),
    mnAutoShowRankBy( -1 ),
    mnSortType( XML_manual ),
    mnSortRefField( -1 ),
    mnSortRefItem( -1 ),
    mbDataField( false ),
    mbDefaultSubtotal( true ),
    mbSumSubtotal( false ),
    mbCountASubtotal( false ),
    mbAverageSubtotal( false ),
    mbMaxSubtotal( false ),
    mbMinSubtotal( false ),
    mbProductSubtotal( false ),
    mbCountSubtotal( false ),
    mbStdDevSubtotal( false ),
    mbStdDevPSubtotal( false ),
    mbVarSubtotal( false ),
    mbVarPSubtotal( false ),
    mbShowAll( true ),
    mbOutline( true ),
    mbSubtotalTop( true ),
    mbInsertBlankRow( false ),
    mbInsertPageBreak( false ),
    mbAutoShow( false ),
    mbTopAutoShow( true ),
    mbMultiPageItems( false )
{
}

void PTFieldModel::setBinAxis( sal_uInt8 nAxis )
{
    /*  Weird. The axis field is organized as bit field, but only one of the
        row/col/page flags are allowed at the same time and refer to the values
        'axisRow', 'axisCol', and 'axisPage' of the XML attribute
        'pivotField@axis'. Additionally, the fourth bit determines if the field
        is a data field, which may appear combined with the row/col/page flags.
        Therefore, this bit is unrelated to the 'axisValues' value of the
        'pivotField@axis' attribute, but refers to the 'pivotField@dataField'
        boolean attribute. */
    static const sal_Int32 spnAxisIds[] = { XML_TOKEN_INVALID, XML_axisRow, XML_axisCol, XML_TOKEN_INVALID, XML_axisPage };
    mnAxis = STATIC_ARRAY_SELECT( spnAxisIds, nAxis, XML_TOKEN_INVALID );
}

// ----------------------------------------------------------------------------

PTPageFieldModel::PTPageFieldModel() :
    mnField( -1 ),
    mnItem( OOBIN_PTPAGEFIELD_MULTIITEMS )
{
}

// ----------------------------------------------------------------------------

PTDataFieldModel::PTDataFieldModel() :
    mnField( -1 ),
    mnSubtotal( XML_sum ),
    mnShowDataAs( XML_normal ),
    mnBaseField( -1 ),
    mnBaseItem( -1 ),
    mnNumFmtId( 0 )
{
}

void PTDataFieldModel::setBinSubtotal( sal_Int32 nSubtotal )
{
    static sal_Int32 spnSubtotals[] = { XML_sum, XML_count, XML_average, XML_max, XML_min, XML_product, XML_countNums, XML_stdDev, XML_stdDevp, XML_var, XML_varp };
    mnSubtotal = STATIC_ARRAY_SELECT( spnSubtotals, nSubtotal, XML_TOKEN_INVALID );
}

void PTDataFieldModel::setBinShowDataAs( sal_Int32 nShowDataAs )
{
    static sal_Int32 spnShowDataAs[] = { XML_normal, XML_difference, XML_percent, XML_percentDiff, XML_runTotal, XML_percentOfRow, XML_percentOfCol, XML_percentOfTotal, XML_index };
    mnShowDataAs = STATIC_ARRAY_SELECT( spnShowDataAs, nShowDataAs, XML_TOKEN_INVALID );
}

// ----------------------------------------------------------------------------

PivotTableField::PivotTableField( PivotTable& rPivotTable, sal_Int32 nFieldIndex ) :
    WorkbookHelper( rPivotTable ),
    mrPivotTable( rPivotTable ),
    mnFieldIndex( nFieldIndex )
{
}

void PivotTableField::importPivotField( const AttributeList& rAttribs )
{
    /*  The documentation mentions a value 'axisValues' for the attribute
        'pivotField@axis'. But this value is not used to mark a data field, as
        data fields may be inserted in one of the row/column/page dimensions at
        the same time. Therefore, the boolean attribute 'pivotField@dataField'
        is really used to mark data fields. */
    maModel.mnAxis            = rAttribs.getToken( XML_axis, XML_TOKEN_INVALID );
    maModel.mnNumFmtId        = rAttribs.getInteger( XML_numFmtId, 0 );
    maModel.mnAutoShowItems   = rAttribs.getInteger( XML_itemPageCount, 10 );
    maModel.mnAutoShowRankBy  = rAttribs.getInteger( XML_rankBy, -1 );
    maModel.mnSortType        = rAttribs.getToken( XML_sortType, XML_manual );
    maModel.mbDataField       = rAttribs.getBool( XML_dataField, false );
    maModel.mbDefaultSubtotal = rAttribs.getBool( XML_defaultSubtotal, true );
    maModel.mbSumSubtotal     = rAttribs.getBool( XML_sumSubtotal, false );
    maModel.mbCountASubtotal  = rAttribs.getBool( XML_countASubtotal, false );
    maModel.mbAverageSubtotal = rAttribs.getBool( XML_avgSubtotal, false );
    maModel.mbMaxSubtotal     = rAttribs.getBool( XML_maxSubtotal, false );
    maModel.mbMinSubtotal     = rAttribs.getBool( XML_minSubtotal, false );
    maModel.mbProductSubtotal = rAttribs.getBool( XML_productSubtotal, false );
    maModel.mbCountSubtotal   = rAttribs.getBool( XML_countSubtotal, false );
    maModel.mbStdDevSubtotal  = rAttribs.getBool( XML_stdDevSubtotal, false );
    maModel.mbStdDevPSubtotal = rAttribs.getBool( XML_stdDevPSubtotal, false );
    maModel.mbVarSubtotal     = rAttribs.getBool( XML_varSubtotal, false );
    maModel.mbVarPSubtotal    = rAttribs.getBool( XML_varPSubtotal, false );
    maModel.mbShowAll         = rAttribs.getBool( XML_showAll, true );
    maModel.mbOutline         = rAttribs.getBool( XML_outline, true );
    maModel.mbSubtotalTop     = rAttribs.getBool( XML_subtotalTop, true );
    maModel.mbInsertBlankRow  = rAttribs.getBool( XML_insertBlankRow, false );
    maModel.mbInsertPageBreak = rAttribs.getBool( XML_insertPageBreak, false );
    maModel.mbAutoShow        = rAttribs.getBool( XML_autoShow, false );
    maModel.mbTopAutoShow     = rAttribs.getBool( XML_topAutoShow, true );
    maModel.mbMultiPageItems  = rAttribs.getBool( XML_multipleItemSelectionAllowed, false );
}

void PivotTableField::importItem( const AttributeList& rAttribs )
{
    PTFieldItemModel aModel;
    aModel.mnCacheItem   = rAttribs.getInteger( XML_x, -1 );
    aModel.mnType        = rAttribs.getToken( XML_t, XML_data );
    aModel.mbShowDetails = rAttribs.getBool( XML_sd, true );
    aModel.mbHidden      = rAttribs.getBool( XML_h, false );
    maItems.push_back( aModel );
}

void PivotTableField::importReference( const AttributeList& rAttribs )
{
    // field index is stored as unsigned integer
    maModel.mnSortRefField = static_cast< sal_Int32 >( rAttribs.getUnsigned( XML_field, SAL_MAX_UINT32 ) );
}

void PivotTableField::importReferenceItem( const AttributeList& rAttribs )
{
    maModel.mnSortRefItem = rAttribs.getInteger( XML_v, -1 );
}

void PivotTableField::importPTField( RecordInputStream& rStrm )
{
    sal_uInt32 nFlags1, nFlags2;
    rStrm >> nFlags1 >> maModel.mnNumFmtId >> nFlags2 >> maModel.mnAutoShowItems >> maModel.mnAutoShowRankBy;

    maModel.setBinAxis( extractValue< sal_uInt8 >( nFlags1, 0, 3 ) );
    maModel.mbDataField       = getFlag( nFlags1, OOBIN_PTFIELD_DATAFIELD );
    maModel.mbDefaultSubtotal = getFlag( nFlags1, OOBIN_PTFIELD_DEFAULT );
    maModel.mbSumSubtotal     = getFlag( nFlags1, OOBIN_PTFIELD_SUM );
    maModel.mbCountASubtotal  = getFlag( nFlags1, OOBIN_PTFIELD_COUNTA );
    maModel.mbAverageSubtotal = getFlag( nFlags1, OOBIN_PTFIELD_AVERAGE );
    maModel.mbMaxSubtotal     = getFlag( nFlags1, OOBIN_PTFIELD_MAX );
    maModel.mbMinSubtotal     = getFlag( nFlags1, OOBIN_PTFIELD_MIN );
    maModel.mbProductSubtotal = getFlag( nFlags1, OOBIN_PTFIELD_PRODUCT );
    maModel.mbCountSubtotal   = getFlag( nFlags1, OOBIN_PTFIELD_COUNT );
    maModel.mbStdDevSubtotal  = getFlag( nFlags1, OOBIN_PTFIELD_STDDEV );
    maModel.mbStdDevPSubtotal = getFlag( nFlags1, OOBIN_PTFIELD_STDDEVP );
    maModel.mbVarSubtotal     = getFlag( nFlags1, OOBIN_PTFIELD_VAR );
    maModel.mbVarPSubtotal    = getFlag( nFlags1, OOBIN_PTFIELD_VARP );

    maModel.mbShowAll         = getFlag( nFlags2, OOBIN_PTFIELD_SHOWALL );
    maModel.mbOutline         = getFlag( nFlags2, OOBIN_PTFIELD_OUTLINE );
    maModel.mbSubtotalTop     = getFlag( nFlags2, OOBIN_PTFIELD_SUBTOTALTOP );
    maModel.mbInsertBlankRow  = getFlag( nFlags2, OOBIN_PTFIELD_INSERTBLANKROW );
    maModel.mbInsertPageBreak = getFlag( nFlags2, OOBIN_PTFIELD_INSERTPAGEBREAK );
    maModel.mbAutoShow        = getFlag( nFlags2, OOBIN_PTFIELD_AUTOSHOW );
    maModel.mbTopAutoShow     = getFlag( nFlags2, OOBIN_PTFIELD_AUTOSHOWTOP );
    maModel.mbMultiPageItems  = getFlag( nFlags2, OOBIN_PTFIELD_MULTIPAGEITEMS );

    bool bAutoSort = getFlag( nFlags2, OOBIN_PTFIELD_AUTOSORT );
    bool bAscending = getFlag( nFlags2, OOBIN_PTFIELD_SORTASCENDING );
    maModel.mnSortType = bAutoSort ? (bAscending ? XML_ascending : XML_descending) : XML_manual;
}

void PivotTableField::importPTFItem( RecordInputStream& rStrm )
{
    PTFieldItemModel aModel;
    sal_uInt8 nType;
    sal_uInt16 nFlags;
    rStrm >> nType >> nFlags >> aModel.mnCacheItem;

    aModel.setBinType( nType );
    aModel.mbShowDetails = !getFlag( nFlags, OOBIN_PTFITEM_HIDEDETAILS );
    aModel.mbHidden      = getFlag( nFlags, OOBIN_PTFITEM_HIDDEN );

    maItems.push_back( aModel );
}

void PivotTableField::importPTReference( RecordInputStream& rStrm )
{
    rStrm >> maModel.mnSortRefField;
}

void PivotTableField::importPTReferenceItem( RecordInputStream& rStrm )
{
    rStrm >> maModel.mnSortRefItem;
}

void PivotTableField::importPTField( BiffInputStream& rStrm )
{
    sal_uInt16 nAxis, nSubtCount, nSubtotals;
    rStrm >> nAxis >> nSubtCount >> nSubtotals;
    rStrm.skip( 2 );    // item count

    maModel.setBinAxis( extractValue< sal_uInt8 >( nAxis, 0, 3 ) );
    maModel.mbDataField       = getFlag( nAxis, BIFF_PTFIELD_DATAFIELD );

    maModel.mbDefaultSubtotal = getFlag( nSubtotals, BIFF_PTFIELD_DEFAULT );
    maModel.mbSumSubtotal     = getFlag( nSubtotals, BIFF_PTFIELD_SUM );
    maModel.mbCountASubtotal  = getFlag( nSubtotals, BIFF_PTFIELD_COUNTA );
    maModel.mbAverageSubtotal = getFlag( nSubtotals, BIFF_PTFIELD_AVERAGE );
    maModel.mbMaxSubtotal     = getFlag( nSubtotals, BIFF_PTFIELD_MAX );
    maModel.mbMinSubtotal     = getFlag( nSubtotals, BIFF_PTFIELD_MIN );
    maModel.mbProductSubtotal = getFlag( nSubtotals, BIFF_PTFIELD_PRODUCT );
    maModel.mbCountSubtotal   = getFlag( nSubtotals, BIFF_PTFIELD_COUNT );
    maModel.mbStdDevSubtotal  = getFlag( nSubtotals, BIFF_PTFIELD_STDDEV );
    maModel.mbStdDevPSubtotal = getFlag( nSubtotals, BIFF_PTFIELD_STDDEVP );
    maModel.mbVarSubtotal     = getFlag( nSubtotals, BIFF_PTFIELD_VAR );
    maModel.mbVarPSubtotal    = getFlag( nSubtotals, BIFF_PTFIELD_VARP );

    // set different defaults for BIFF
    maModel.mbShowAll = maModel.mbOutline = maModel.mbSubtotalTop = false;

    // read following items
    while( (rStrm.getNextRecId() == BIFF_ID_PTFITEM) && rStrm.startNextRecord() )
        importPTFItem( rStrm );

    // read following PTFIELD2 record with additional field settings
    if( (getBiff() == BIFF8) && (rStrm.getNextRecId() == BIFF_ID_PTFIELD2) && rStrm.startNextRecord() )
        importPTField2( rStrm );
}

void PivotTableField::importPTField2( BiffInputStream& rStrm )
{
    sal_uInt32 nFlags;
    rStrm >> nFlags;
    maModel.mnSortRefItem    = rStrm.readInt16();
    maModel.mnAutoShowRankBy = rStrm.readInt16();
    maModel.mnNumFmtId       = rStrm.readuInt16();

    maModel.mnAutoShowItems   = extractValue< sal_Int32 >( nFlags, 24, 8 );
    maModel.mbShowAll         = getFlag( nFlags, BIFF_PTFIELD2_SHOWALL );
    maModel.mbOutline         = getFlag( nFlags, BIFF_PTFIELD2_OUTLINE );
    maModel.mbSubtotalTop     = getFlag( nFlags, BIFF_PTFIELD2_SUBTOTALTOP );
    maModel.mbInsertBlankRow  = getFlag( nFlags, BIFF_PTFIELD2_INSERTBLANKROW );
    maModel.mbAutoShow        = getFlag( nFlags, BIFF_PTFIELD2_AUTOSHOW );
    maModel.mbTopAutoShow     = getFlag( nFlags, BIFF_PTFIELD2_AUTOSHOWTOP );

    bool bAutoSort = getFlag( nFlags, BIFF_PTFIELD2_AUTOSORT );
    bool bAscending = getFlag( nFlags, BIFF_PTFIELD2_SORTASCENDING );
    maModel.mnSortType = bAutoSort ? (bAscending ? XML_ascending : XML_descending) : XML_manual;
    // mnSortRefField == OOX_PT_DATALAYOUTFIELD will indicate sorting by data field
    if( maModel.mnSortRefItem >= 0 )
        maModel.mnSortRefField = OOX_PT_DATALAYOUTFIELD;
}

void PivotTableField::importPTFItem( BiffInputStream& rStrm )
{
    PTFieldItemModel aModel;
    sal_uInt16 nType, nFlags;
    sal_Int16 nCacheItem;
    rStrm >> nType >> nFlags >> nCacheItem;

    aModel.setBinType( nType );
    aModel.mnCacheItem = nCacheItem;
    aModel.mbShowDetails = !getFlag( nFlags, BIFF_PTFITEM_HIDEDETAILS );
    aModel.mbHidden      = getFlag( nFlags, BIFF_PTFITEM_HIDDEN );

    maItems.push_back( aModel );
}

void PivotTableField::finalizeImport( const Reference< XDataPilotDescriptor >& rxDPDesc )
{
    /*  Process all fields based on source data, other fields (e.g. group
        fields) are processed from here. PivotCacahe::getDatabaseIndex()
        returns -1 for all fields not based on source data. */
    Reference< XDataPilotField > xDPField;
    sal_Int32 nDatabaseIdx = mrPivotTable.getCacheDatabaseIndex( mnFieldIndex );
    if( (nDatabaseIdx >= 0) && rxDPDesc.is() ) try
    {
        // try to get the source field and its name from passed DataPilot descriptor
        Reference< XIndexAccess > xDPFieldsIA( rxDPDesc->getDataPilotFields(), UNO_SET_THROW );
        xDPField.set( xDPFieldsIA->getByIndex( nDatabaseIdx ), UNO_QUERY_THROW );
        Reference< XNamed > xDPFieldName( xDPField, UNO_QUERY_THROW );
        maDPFieldName = xDPFieldName->getName();
        OSL_ENSURE( maDPFieldName.getLength() > 0, "PivotTableField::finalizeImport - no field name in source data found" );

        // try to convert grouping settings
        if( const PivotCacheField* pCacheField = mrPivotTable.getCacheField( mnFieldIndex ) )
        {
            // numeric grouping is done inplace, no nested group fields will appear
            if( pCacheField->hasNumericGrouping() )
            {
                pCacheField->convertNumericGrouping( xDPField );
            }
            else if( pCacheField->hasDateGrouping() )
            {
                // first date group settings are inplace
                pCacheField->createDateGroupField( xDPField );
                // create all nested group fields (if any)
                mrPivotTable.finalizeDateGroupingImport( xDPField, mnFieldIndex );
            }
            else if( pCacheField->hasParentGrouping() )
            {
                // create a list of all item names, needed to map between original and group items
                ::std::vector< OUString > aItems;
                pCacheField->getCacheItemNames( aItems );
                PivotCacheGroupItemVector aItemNames;
                for( ::std::vector< OUString >::iterator aIt = aItems.begin(), aEnd = aItems.end(); aIt != aEnd; ++aIt )
                    aItemNames.push_back( PivotCacheGroupItem( *aIt ) );
                // create all nested group fields (if any)
                mrPivotTable.finalizeParentGroupingImport( xDPField, *pCacheField, aItemNames );
            }
        }
    }
    catch( Exception& )
    {
    }
}

void PivotTableField::finalizeDateGroupingImport( const Reference< XDataPilotField >& rxBaseDPField, sal_Int32 nBaseFieldIdx )
{
    if( maDPFieldName.getLength() == 0 )    // prevent endless loops if file format is broken
    {
        if( const PivotCacheField* pCacheField = mrPivotTable.getCacheField( mnFieldIndex ) )
        {
            if( !pCacheField->isDatabaseField() && pCacheField->hasDateGrouping() && (pCacheField->getGroupBaseField() == nBaseFieldIdx) )
            {
                maDPFieldName = pCacheField->createDateGroupField( rxBaseDPField );
                OSL_ENSURE( maDPFieldName.getLength() > 0, "PivotTableField::finalizeDateGroupingImport - cannot create date group field" );
            }
        }
    }
}

void PivotTableField::finalizeParentGroupingImport( const Reference< XDataPilotField >& rxBaseDPField, PivotCacheGroupItemVector& orItemNames )
{
    if( maDPFieldName.getLength() == 0 )    // prevent endless loops if file format is broken
    {
        if( const PivotCacheField* pCacheField = mrPivotTable.getCacheField( mnFieldIndex ) )
        {
            maDPFieldName = pCacheField->createParentGroupField( rxBaseDPField, orItemNames );
            // on success, try to create nested group fields
            Reference< XDataPilotField > xDPField = mrPivotTable.getDataPilotField( maDPFieldName );
            if( xDPField.is() )
                mrPivotTable.finalizeParentGroupingImport( xDPField, *pCacheField, orItemNames );
        }
    }
}

void PivotTableField::convertRowField()
{
    convertRowColPageField( XML_axisRow );
}

void PivotTableField::convertColField()
{
    convertRowColPageField( XML_axisCol );
}

void PivotTableField::convertHiddenField()
{
    convertRowColPageField( XML_TOKEN_INVALID );
}

void PivotTableField::convertPageField( const PTPageFieldModel& rPageField )
{
    OSL_ENSURE( rPageField.mnField == mnFieldIndex, "PivotTableField::convertPageField - wrong field index" );
    // convert all settings common for row/column/page fields
    Reference< XDataPilotField > xDPField = convertRowColPageField( XML_axisPage );

    if( xDPField.is() )
    {
        PropertySet aPropSet( xDPField );
        using namespace ::com::sun::star::sheet;

        // find cache item used as 'selected page'
        sal_Int32 nCacheItem = -1;
        if( maModel.mbMultiPageItems )
        {
            // multiple items may be selected
            OSL_ENSURE( rPageField.mnItem == OOBIN_PTPAGEFIELD_MULTIITEMS, "PivotTableField::convertPageField - unexpected cache item index" );
            // try to find a single visible item
            bool bHasMultiItems = false;
            for( ItemModelVector::iterator aIt = maItems.begin(), aEnd = maItems.end(); (aIt != aEnd) && !bHasMultiItems; ++aIt )
            {
                if( (aIt->mnType == XML_data) && !aIt->mbHidden )
                {
                    bHasMultiItems = nCacheItem >= 0;
                    nCacheItem = bHasMultiItems ? -1 : aIt->mnCacheItem;
                }
            }
        }
        else
        {
            // single item may be selected
            if( (0 <= rPageField.mnItem) && (rPageField.mnItem < static_cast< sal_Int32 >( maItems.size() )) )
                nCacheItem = maItems[ rPageField.mnItem ].mnCacheItem;
        }

        if( nCacheItem >= 0 )
        {
            if( const PivotCacheField* pCacheField = mrPivotTable.getCacheField( mnFieldIndex ) )
            {
                if( const PivotCacheItem* pSharedItem = pCacheField->getCacheItem( nCacheItem ) )
                {
                    OUString aSelectedPage = pSharedItem->getName();
                    if( aSelectedPage.getLength() > 0 )
                        aPropSet.setProperty( PROP_SelectedPage, aSelectedPage );
                }
            }
        }
    }
}

void PivotTableField::convertDataField( const PTDataFieldModel& rDataField )
{
    OSL_ENSURE( rDataField.mnField == mnFieldIndex, "PivotTableField::convertDataField - wrong field index" );
    OSL_ENSURE( maModel.mbDataField, "PivotTableField::convertDataField - not a data field" );
    Reference< XDataPilotField > xDPField = mrPivotTable.getDataPilotField( maDPFieldName );
    if( xDPField.is() )
    {
        PropertySet aPropSet( xDPField );
        using namespace ::com::sun::star::sheet;

        // field orientation
        aPropSet.setProperty( PROP_Orientation, DataPilotFieldOrientation_DATA );

        /*  Field aggregation function. Documentation is a little bit confused
            about which names to use for the count functions. The name 'count'
            means 'count all', and 'countNum' means 'count numbers'. On the
            other hand, for subtotals, 'countA' means 'count all', and 'count'
            means 'count numbers' (see above). */
        GeneralFunction eAggFunc = GeneralFunction_SUM;
        switch( rDataField.mnSubtotal )
        {
            case XML_sum:       eAggFunc = GeneralFunction_SUM;         break;
            case XML_count:     eAggFunc = GeneralFunction_COUNT;       break;
            case XML_average:   eAggFunc = GeneralFunction_AVERAGE;     break;
            case XML_max:       eAggFunc = GeneralFunction_MAX;         break;
            case XML_min:       eAggFunc = GeneralFunction_MIN;         break;
            case XML_product:   eAggFunc = GeneralFunction_PRODUCT;     break;
            case XML_countNums: eAggFunc = GeneralFunction_COUNTNUMS;   break;
            case XML_stdDev:    eAggFunc = GeneralFunction_STDEV;       break;
            case XML_stdDevp:   eAggFunc = GeneralFunction_STDEVP;      break;
            case XML_var:       eAggFunc = GeneralFunction_VAR;         break;
            case XML_varp:      eAggFunc = GeneralFunction_VARP;        break;
            default:            OSL_ENSURE( false, "PivotTableField::convertDataField - unknown aggregation function" );
        }
        aPropSet.setProperty( PROP_Function, eAggFunc );

        // field reference ('show data as')
        DataPilotFieldReference aReference;
        aReference.ReferenceType = DataPilotFieldReferenceType::NONE;
        switch( rDataField.mnShowDataAs )
        {
            case XML_difference:        aReference.ReferenceType = DataPilotFieldReferenceType::ITEM_DIFFERENCE;            break;
            case XML_percent:           aReference.ReferenceType = DataPilotFieldReferenceType::ITEM_PERCENTAGE;            break;
            case XML_percentDiff:       aReference.ReferenceType = DataPilotFieldReferenceType::ITEM_PERCENTAGE_DIFFERENCE; break;
            case XML_runTotal:          aReference.ReferenceType = DataPilotFieldReferenceType::RUNNING_TOTAL;              break;
            case XML_percentOfRow:      aReference.ReferenceType = DataPilotFieldReferenceType::ROW_PERCENTAGE;             break;
            case XML_percentOfCol:      aReference.ReferenceType = DataPilotFieldReferenceType::COLUMN_PERCENTAGE;          break;
            case XML_percentOfTotal:    aReference.ReferenceType = DataPilotFieldReferenceType::TOTAL_PERCENTAGE;           break;
            case XML_index:             aReference.ReferenceType = DataPilotFieldReferenceType::INDEX;                      break;
        }
        if( aReference.ReferenceType != DataPilotFieldReferenceType::NONE )
        {
            if( const PivotCacheField* pCacheField = mrPivotTable.getCacheField( rDataField.mnBaseField ) )
            {
                aReference.ReferenceField = pCacheField->getName();
                switch( rDataField.mnBaseItem )
                {
                    case OOX_PT_PREVIOUS_ITEM:
                        aReference.ReferenceItemType = DataPilotFieldReferenceItemType::PREVIOUS;
                    break;
                    case OOX_PT_NEXT_ITEM:
                        aReference.ReferenceItemType = DataPilotFieldReferenceItemType::NEXT;
                    break;
                    default:
                        aReference.ReferenceItemType = DataPilotFieldReferenceItemType::NAMED;
                        if( const PivotCacheItem* pCacheItem = pCacheField->getCacheItem( rDataField.mnBaseItem ) )
                            aReference.ReferenceItemName = pCacheItem->getName();
                }
                aPropSet.setProperty( PROP_Reference, aReference );
            }
        }
    }
}

// private --------------------------------------------------------------------

Reference< XDataPilotField > PivotTableField::convertRowColPageField( sal_Int32 nAxis )
{
    bool bDataLayout = mnFieldIndex == OOX_PT_DATALAYOUTFIELD;
    Reference< XDataPilotField > xDPField = bDataLayout ? mrPivotTable.getDataLayoutField() : mrPivotTable.getDataPilotField( maDPFieldName );
    OSL_ENSURE( bDataLayout || (nAxis == maModel.mnAxis), "PivotTableField::convertRowColPageField - field axis mismatch" );

    if( xDPField.is() )
    {
        PropertySet aPropSet( xDPField );
        using namespace ::com::sun::star::sheet;

        // field orientation
        DataPilotFieldOrientation eFieldOrient = DataPilotFieldOrientation_HIDDEN;
        switch( nAxis )
        {
            case XML_axisRow:   eFieldOrient = DataPilotFieldOrientation_ROW;       break;
            case XML_axisCol:   eFieldOrient = DataPilotFieldOrientation_COLUMN;    break;
            case XML_axisPage:  eFieldOrient = DataPilotFieldOrientation_PAGE;      break;
        }
        if( eFieldOrient != DataPilotFieldOrientation_HIDDEN )
            aPropSet.setProperty( PROP_Orientation, eFieldOrient );

        // all other settings not for the data layout field
        if( !bDataLayout )
        {
            /*  Field subtotal functions. Ignore the 'defaultSubtotal' flag, if
                explicit functions are set. This is different behaviour between
                XML (where 'defaultSubtotal' is set regardless of other
                functions) and binary formats (where 'defaultSubtotal' is not
                set if other functions are set). */
            ::std::vector< GeneralFunction > aSubtotals;
            /*  Order of subtotals is fixed in Excel. Documentation is a little
                bit confused about which names to use for the count functions.
                For subtotals, 'countA' means 'count all', and 'count' means
                'count numbers'. On the other hand, for the data field
                aggregation function, 'count' means 'count all', and 'countNum'
                means 'count numbers' (see below). */
            if( maModel.mbSumSubtotal )     aSubtotals.push_back( GeneralFunction_SUM );
            if( maModel.mbCountASubtotal )  aSubtotals.push_back( GeneralFunction_COUNT );
            if( maModel.mbAverageSubtotal ) aSubtotals.push_back( GeneralFunction_AVERAGE );
            if( maModel.mbMaxSubtotal )     aSubtotals.push_back( GeneralFunction_MAX );
            if( maModel.mbMinSubtotal )     aSubtotals.push_back( GeneralFunction_MIN );
            if( maModel.mbProductSubtotal ) aSubtotals.push_back( GeneralFunction_PRODUCT );
            if( maModel.mbCountSubtotal )   aSubtotals.push_back( GeneralFunction_COUNTNUMS );
            if( maModel.mbStdDevSubtotal )  aSubtotals.push_back( GeneralFunction_STDEV );
            if( maModel.mbStdDevPSubtotal ) aSubtotals.push_back( GeneralFunction_STDEVP );
            if( maModel.mbVarSubtotal )     aSubtotals.push_back( GeneralFunction_VAR );
            if( maModel.mbVarPSubtotal )    aSubtotals.push_back( GeneralFunction_VARP );
            // if no function is set manually, check the 'defaultSubtotal' flag
            if( aSubtotals.empty() && maModel.mbDefaultSubtotal )
                aSubtotals.push_back( GeneralFunction_AUTO );
            aPropSet.setProperty( PROP_Subtotals, ContainerHelper::vectorToSequence( aSubtotals ) );

            // layout settings
            DataPilotFieldLayoutInfo aLayoutInfo;
            aLayoutInfo.LayoutMode = maModel.mbOutline ?
                (maModel.mbSubtotalTop ? DataPilotFieldLayoutMode::OUTLINE_SUBTOTALS_TOP : DataPilotFieldLayoutMode::OUTLINE_SUBTOTALS_BOTTOM) :
                DataPilotFieldLayoutMode::TABULAR_LAYOUT;
            aLayoutInfo.AddEmptyLines = maModel.mbInsertBlankRow;
            aPropSet.setProperty( PROP_LayoutInfo, aLayoutInfo );
            aPropSet.setProperty( PROP_ShowEmpty, maModel.mbShowAll );

            // auto show (OOXML3/OOBIN3 only)
            if( maModel.mbAutoShow )
            {
                DataPilotFieldAutoShowInfo aAutoShowInfo;
                aAutoShowInfo.IsEnabled = sal_True;
                aAutoShowInfo.ShowItemsMode = maModel.mbTopAutoShow ? DataPilotFieldShowItemsMode::FROM_TOP : DataPilotFieldShowItemsMode::FROM_BOTTOM;
                aAutoShowInfo.ItemCount = maModel.mnAutoShowItems;
                if( const PivotCacheField* pCacheField = mrPivotTable.getCacheFieldOfDataField( maModel.mnAutoShowRankBy ) )
                    aAutoShowInfo.DataField = pCacheField->getName();
                aPropSet.setProperty( PROP_AutoShowInfo, aAutoShowInfo );
            }

            // auto sort
            DataPilotFieldSortInfo aSortInfo;
            aSortInfo.IsAscending = maModel.mnSortType == XML_ascending;
            if( (maModel.mnSortType != XML_ascending) && (maModel.mnSortType != XML_descending) )
            {
                aSortInfo.Mode = DataPilotFieldSortMode::MANUAL;
            }
            else
            {
                const PivotCacheField* pCacheField = (maModel.mnSortRefField == OOX_PT_DATALAYOUTFIELD) ?
                    mrPivotTable.getCacheFieldOfDataField( maModel.mnSortRefItem ) : 0;
                if( pCacheField )
                {
                    aSortInfo.Mode = DataPilotFieldSortMode::DATA;
                    aSortInfo.Field = pCacheField->getName();
                }
                else
                {
                    aSortInfo.Mode = DataPilotFieldSortMode::NAME;
                }
            }
            aPropSet.setProperty( PROP_SortInfo, aSortInfo );

            // item settings
            if( const PivotCacheField* pCacheField = mrPivotTable.getCacheField( mnFieldIndex ) ) try
            {
                Reference< XNameAccess > xDPItemsNA( xDPField->getItems(), UNO_QUERY_THROW );
                for( ItemModelVector::iterator aIt = maItems.begin(), aEnd = maItems.end(); aIt != aEnd; ++aIt )
                {
                    if( aIt->mnType == XML_data )
                    {
                        if( const PivotCacheItem* pSharedItem = pCacheField->getCacheItem( aIt->mnCacheItem ) ) try
                        {
                            PropertySet aItemProp( xDPItemsNA->getByName( pSharedItem->getName() ) );
                            aItemProp.setProperty( PROP_ShowDetail, aIt->mbShowDetails );
                            aItemProp.setProperty( PROP_IsHidden, aIt->mbHidden );
                        }
                        catch( Exception& )
                        {
                            // catch every failed container access to be able to process following items
                        }
                    }
                }
            }
            catch( Exception& )
            {
            }
        }
    }
    return xDPField;
}

// ============================================================================

PTFilterModel::PTFilterModel() :
    mfValue( 0.0 ),
    mnField( -1 ),
    mnMemPropField( -1 ),
    mnType( XML_TOKEN_INVALID ),
    mnEvalOrder( 0 ),
    mnId( -1 ),
    mnMeasureField( -1 ),
    mnMeasureHier( -1 ),
    mbTopFilter( true )
{
}

// ----------------------------------------------------------------------------

PivotTableFilter::PivotTableFilter( const PivotTable& rPivotTable ) :
    WorkbookHelper( rPivotTable ),
    mrPivotTable( rPivotTable )
{
}

void PivotTableFilter::importFilter( const AttributeList& rAttribs )
{
    maModel.maName         = rAttribs.getXString( XML_name, OUString() );
    maModel.maDescription  = rAttribs.getXString( XML_description, OUString() );
    maModel.maStrValue1    = rAttribs.getXString( XML_stringValue1, OUString() );
    maModel.maStrValue2    = rAttribs.getXString( XML_stringValue2, OUString() );
    maModel.mnField        = rAttribs.getInteger( XML_fld, -1 );
    maModel.mnMemPropField = rAttribs.getInteger( XML_mpFld, -1 );
    maModel.mnType         = rAttribs.getToken( XML_type, XML_TOKEN_INVALID );
    maModel.mnEvalOrder    = rAttribs.getInteger( XML_evalOrder, 0 );
    maModel.mnId           = rAttribs.getInteger( XML_id, -1 );
    maModel.mnMeasureField = rAttribs.getInteger( XML_iMeasureFld, -1 );
    maModel.mnMeasureHier  = rAttribs.getInteger( XML_iMeasureHier, -1 );
}

void PivotTableFilter::importTop10( const AttributeList& rAttribs )
{
    OSL_ENSURE( rAttribs.getBool( XML_percent, false ) == (maModel.mnType == XML_percent),
        "PivotTableFilter::importTop10 - unexpected value of percent attribute" );
    maModel.mfValue     = rAttribs.getDouble( XML_val, 0.0 );
    maModel.mbTopFilter = rAttribs.getBool( XML_top, true );
}

void PivotTableFilter::importPTFilter( RecordInputStream& rStrm )
{
    sal_Int32 nType;
    sal_uInt16 nFlags;
    rStrm >> maModel.mnField >> maModel.mnMemPropField >> nType;
    rStrm.skip( 4 );    // unused
    rStrm >> maModel.mnId >> maModel.mnMeasureField >> maModel.mnMeasureHier >> nFlags;
    if( getFlag( nFlags, OOBIN_PTFILTER_HASNAME ) )
        rStrm >> maModel.maName;
    if( getFlag( nFlags, OOBIN_PTFILTER_HASDESCRIPTION ) )
        rStrm >> maModel.maDescription;
    if( getFlag( nFlags, OOBIN_PTFILTER_HASSTRVALUE1 ) )
        rStrm >> maModel.maStrValue1;
    if( getFlag( nFlags, OOBIN_PTFILTER_HASSTRVALUE2 ) )
        rStrm >> maModel.maStrValue2;

    static sal_Int32 spnTypes[] =
    {
        XML_unknown,
        // data field top10 filter (1-3)
        XML_count, XML_percent, XML_sum,
        // caption filter (4-17)
        XML_captionEqual, XML_captionNotEqual,
        XML_captionBeginsWith, XML_captionNotBeginsWith, XML_captionEndsWith, XML_captionNotEndsWith,
        XML_captionContains, XML_captionNotContains, XML_captionGreaterThan, XML_captionGreaterThanOrEqual,
        XML_captionLessThan, XML_captionLessThanOrEqual, XML_captionBetween, XML_captionNotBetween,
        // value filter (18-25)
        XML_valueEqual, XML_valueNotEqual, XML_valueGreaterThan, XML_valueGreaterThanOrEqual,
        XML_valueLessThan, XML_valueLessThanOrEqual, XML_valueBetween, XML_valueNotBetween,
        // date filter (26-65)
        XML_dateEqual, XML_dateOlderThan, XML_dateNewerThan, XML_dateBetween,
        XML_tomorrow, XML_today, XML_yesterday, XML_nextWeek, XML_thisWeek, XML_lastWeek,
        XML_nextMonth, XML_thisMonth, XML_lastMonth, XML_nextQuarter, XML_thisQuarter, XML_lastQuarter,
        XML_nextYear, XML_thisYear, XML_lastYear, XML_yearToDate, XML_Q1, XML_Q2, XML_Q3, XML_Q4,
        XML_M1, XML_M2, XML_M3, XML_M4, XML_M5, XML_M6, XML_M7, XML_M8, XML_M9, XML_M10, XML_M11, XML_M12,
        XML_dateNotEqual, XML_dateOlderThanOrEqual, XML_dateNewerThanOrEqual, XML_dateNotBetween
    };
    maModel.mnType = STATIC_ARRAY_SELECT( spnTypes, nType, XML_TOKEN_INVALID );
}

void PivotTableFilter::importTop10Filter( RecordInputStream& rStrm )
{
    sal_uInt8 nFlags;
    rStrm >> nFlags >> maModel.mfValue;

    OSL_ENSURE( getFlag( nFlags, OOBIN_TOP10FILTER_PERCENT ) == (maModel.mnType == XML_percent),
        "PivotTableFilter::importTop10 - unexpected value of percent attribute" );
    maModel.mbTopFilter = getFlag( nFlags, OOBIN_TOP10FILTER_TOP );
}

void PivotTableFilter::finalizeImport()
{
    // only simple top10 filter supported
    if( maModel.mnType == XML_count )
    {
        PropertySet aPropSet( mrPivotTable.getDataPilotField( maModel.mnField ) );
        if( aPropSet.is() )
        {
            using namespace ::com::sun::star::sheet;
            DataPilotFieldAutoShowInfo aAutoShowInfo;
            aAutoShowInfo.IsEnabled = sal_True;
            aAutoShowInfo.ShowItemsMode = maModel.mbTopFilter ? DataPilotFieldShowItemsMode::FROM_TOP : DataPilotFieldShowItemsMode::FROM_BOTTOM;
            aAutoShowInfo.ItemCount = getLimitedValue< sal_Int32, double >( maModel.mfValue, 0, SAL_MAX_INT32 );
            if( const PivotCacheField* pCacheField = mrPivotTable.getCacheFieldOfDataField( maModel.mnMeasureField ) )
                aAutoShowInfo.DataField = pCacheField->getName();
            aPropSet.setProperty( PROP_AutoShowInfo, aAutoShowInfo );
        }
    }
}

// ============================================================================

PTDefinitionModel::PTDefinitionModel() :
    mnCacheId( -1 ),
    mnDataPosition( 0 ),
    mnPageWrap( 0 ),
    mnIndent( 1 ),
    mnChartFormat( 0 ),
    mnRowFields( 0 ),
    mnColFields( 0 ),
    mbDataOnRows( false ),
    mbShowError( false ),
    mbShowMissing( true ),
    mbShowItems( true ),
    mbDisableFieldList( false ),
    mbShowCalcMembers( true ),
    mbVisualTotals( true ),
    mbShowDrill( true ),
    mbPrintDrill( false ),
    mbEnableDrill( true ),
    mbPreserveFormatting( true ),
    mbPageOverThenDown( false ),
    mbSubtotalHiddenItems( false ),
    mbRowGrandTotals( true ),
    mbColGrandTotals( true ),
    mbFieldPrintTitles( false ),
    mbItemPrintTitles( false ),
    mbMergeItem( false ),
    mbShowEmptyRow( false ),
    mbShowEmptyCol( false ),
    mbShowHeaders( true ),
    mbFieldListSortAsc( false ),
    mbCustomListSort( true )
{
}

// ----------------------------------------------------------------------------

PTLocationModel::PTLocationModel() :
    mnFirstHeaderRow( 0 ),
    mnFirstDataRow( 0 ),
    mnFirstDataCol( 0 ),
    mnRowPageCount( 0 ),
    mnColPageCount( 0 )
{
}

// ----------------------------------------------------------------------------

PivotTable::PivotTable( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    maDataField( *this, OOX_PT_DATALAYOUTFIELD ),
    mpPivotCache( 0 )
{
}

void PivotTable::importPivotTableDefinition( const AttributeList& rAttribs )
{
    maDefModel.maName                = rAttribs.getXString( XML_name, OUString() );
    maDefModel.maDataCaption         = rAttribs.getXString( XML_dataCaption , OUString() );
    maDefModel.maGrandTotalCaption   = rAttribs.getXString( XML_grandTotalCaption, OUString() );
    maDefModel.maRowHeaderCaption    = rAttribs.getXString( XML_rowHeaderCaption, OUString() );
    maDefModel.maColHeaderCaption    = rAttribs.getXString( XML_colHeaderCaption, OUString() );
    maDefModel.maErrorCaption        = rAttribs.getXString( XML_errorCaption, OUString() );
    maDefModel.maMissingCaption      = rAttribs.getXString( XML_missingCaption, OUString() );
    maDefModel.maPageStyle           = rAttribs.getXString( XML_pageStyle, OUString() );
    maDefModel.maPivotTableStyle     = rAttribs.getXString( XML_pivotTableStyle, OUString() );
    maDefModel.maVacatedStyle        = rAttribs.getXString( XML_vacatedStyle, OUString() );
    maDefModel.maTag                 = rAttribs.getXString( XML_tag, OUString() );
    maDefModel.mnCacheId             = rAttribs.getInteger( XML_cacheId, -1 );
    maDefModel.mnDataPosition        = rAttribs.getInteger( XML_dataPosition, 0 );
    maDefModel.mnPageWrap            = rAttribs.getInteger( XML_pageWrap, 0 );
    maDefModel.mnIndent              = rAttribs.getInteger( XML_indent, 1 );
    maDefModel.mnChartFormat         = rAttribs.getInteger( XML_chartFormat, 0 );
    maDefModel.mbDataOnRows          = rAttribs.getBool( XML_dataOnRows, false );
    maDefModel.mbShowError           = rAttribs.getBool( XML_showError, false );
    maDefModel.mbShowMissing         = rAttribs.getBool( XML_showMissing, true );
    maDefModel.mbShowItems           = rAttribs.getBool( XML_showItems, true );
    maDefModel.mbDisableFieldList    = rAttribs.getBool( XML_disableFieldList, false );
    maDefModel.mbShowCalcMembers     = rAttribs.getBool( XML_showCalcMbrs, true );
    maDefModel.mbVisualTotals        = rAttribs.getBool( XML_visualTotals, true );
    maDefModel.mbShowDrill           = rAttribs.getBool( XML_showDrill, true );
    maDefModel.mbPrintDrill          = rAttribs.getBool( XML_printDrill, false );
    maDefModel.mbEnableDrill         = rAttribs.getBool( XML_enableDrill, true );
    maDefModel.mbPreserveFormatting  = rAttribs.getBool( XML_preserveFormatting, true );
    maDefModel.mbPageOverThenDown    = rAttribs.getBool( XML_pageOverThenDown, false );
    maDefModel.mbSubtotalHiddenItems = rAttribs.getBool( XML_subtotalHiddenItems, false );
    maDefModel.mbRowGrandTotals      = rAttribs.getBool( XML_rowGrandTotals, true );
    maDefModel.mbColGrandTotals      = rAttribs.getBool( XML_colGrandTotals, true );
    maDefModel.mbFieldPrintTitles    = rAttribs.getBool( XML_fieldPrintTitles, false );
    maDefModel.mbItemPrintTitles     = rAttribs.getBool( XML_itemPrintTitles, false );
    maDefModel.mbMergeItem           = rAttribs.getBool( XML_mergeItem, false );
    maDefModel.mbShowEmptyRow        = rAttribs.getBool( XML_showEmptyRow, false );
    maDefModel.mbShowEmptyCol        = rAttribs.getBool( XML_showEmptyCol, false );
    maDefModel.mbShowHeaders         = rAttribs.getBool( XML_showHeaders, true );
    maDefModel.mbFieldListSortAsc    = rAttribs.getBool( XML_fieldListSortAscending, false );
    maDefModel.mbCustomListSort      = rAttribs.getBool( XML_customListSort, true );
}

void PivotTable::importLocation( const AttributeList& rAttribs, sal_Int16 nSheet )
{
    getAddressConverter().convertToCellRangeUnchecked( maLocationModel.maRange, rAttribs.getString( XML_ref, OUString() ), nSheet );
    maLocationModel.mnFirstHeaderRow = rAttribs.getInteger( XML_firstHeaderRow, 0 );
    maLocationModel.mnFirstDataRow   = rAttribs.getInteger( XML_firstDataRow, 0 );
    maLocationModel.mnFirstDataCol   = rAttribs.getInteger( XML_firstDataCol, 0 );
    maLocationModel.mnRowPageCount   = rAttribs.getInteger( XML_rowPageCount, 0 );
    maLocationModel.mnColPageCount   = rAttribs.getInteger( XML_colPageCount, 0 );
}

void PivotTable::importRowField( const AttributeList& rAttribs )
{
    importField( maRowFields, rAttribs );
}

void PivotTable::importColField( const AttributeList& rAttribs )
{
    importField( maColFields, rAttribs );
}

void PivotTable::importPageField( const AttributeList& rAttribs )
{
    PTPageFieldModel aModel;
    aModel.maName      = rAttribs.getXString( XML_name, OUString() );
    aModel.mnField     = rAttribs.getInteger( XML_fld, -1 );
    // specification is wrong, XML_item is not the cache item, but the field item
    aModel.mnItem      = rAttribs.getInteger( XML_item, OOBIN_PTPAGEFIELD_MULTIITEMS );
    maPageFields.push_back( aModel );
}

void PivotTable::importDataField( const AttributeList& rAttribs )
{
    PTDataFieldModel aModel;
    aModel.maName       = rAttribs.getXString( XML_name, OUString() );
    aModel.mnField      = rAttribs.getInteger( XML_fld, -1 );
    aModel.mnSubtotal   = rAttribs.getToken( XML_subtotal, XML_sum );
    aModel.mnShowDataAs = rAttribs.getToken( XML_showDataAs, XML_normal );
    aModel.mnBaseField  = rAttribs.getInteger( XML_baseField, -1 );
    aModel.mnBaseItem   = rAttribs.getInteger( XML_baseItem, -1 );
    aModel.mnNumFmtId   = rAttribs.getInteger( XML_numFmtId, 0 );
    maDataFields.push_back( aModel );
}

void PivotTable::importPTDefinition( RecordInputStream& rStrm )
{
    sal_uInt32 nFlags1, nFlags2, nFlags3;
    sal_uInt8 nDataAxis;
    rStrm >> nFlags1 >> nFlags2 >> nFlags3 >> nDataAxis;
    maDefModel.mnPageWrap = rStrm.readuInt8();
    rStrm.skip( 2 );    // refresh versions
    rStrm >> maDefModel.mnDataPosition;
    rStrm.skip( 4 );    // 2 bytes autoformat id, 2 bytes unused
    rStrm >> maDefModel.mnChartFormat >> maDefModel.mnCacheId >> maDefModel.maName;
    if( getFlag( nFlags2, OOBIN_PTDEF_HASDATACAPTION ) )
        rStrm >> maDefModel.maDataCaption;
    if( getFlag( nFlags2, OOBIN_PTDEF_HASGRANDTOTALCAPTION ) )
        rStrm >> maDefModel.maGrandTotalCaption;
    if( !getFlag( nFlags3, OOBIN_PTDEF_NOERRORCAPTION ) )   // missing flag indicates existing string
        rStrm >> maDefModel.maErrorCaption;
    if( !getFlag( nFlags3, OOBIN_PTDEF_NOMISSINGCAPTION ) ) // missing flag indicates existing string
        rStrm >> maDefModel.maMissingCaption;
    if( getFlag( nFlags2, OOBIN_PTDEF_HASPAGESTYLE ) )
        rStrm >> maDefModel.maPageStyle;
    if( getFlag( nFlags2, OOBIN_PTDEF_HASPIVOTTABLESTYLE ) )
        rStrm >> maDefModel.maPivotTableStyle;
    if( getFlag( nFlags2, OOBIN_PTDEF_HASVACATEDSTYLE ) )
        rStrm >> maDefModel.maVacatedStyle;
    if( getFlag( nFlags2, OOBIN_PTDEF_HASTAG ) )
        rStrm >> maDefModel.maTag;
    if( getFlag( nFlags3, OOBIN_PTDEF_HASCOLHEADERCAPTION ) )   // TODO: right order (col/row)? spec is unclear
        rStrm >> maDefModel.maColHeaderCaption;
    if( getFlag( nFlags3, OOBIN_PTDEF_HASROWHEADERCAPTION ) )
        rStrm >> maDefModel.maRowHeaderCaption;

    OSL_ENSURE( (nDataAxis == OOBIN_PTDEF_ROWAXIS) || (nDataAxis == OOBIN_PTDEF_COLAXIS),
        "PivotTable::importPTDefinition - unexpected axis position for data field" );

    maDefModel.mnIndent              = extractValue< sal_uInt8 >( nFlags1, 24, 7 );
    maDefModel.mbDataOnRows          = nDataAxis == OOBIN_PTDEF_ROWAXIS;
    maDefModel.mbShowError           = getFlag( nFlags2, OOBIN_PTDEF_SHOWERROR );
    maDefModel.mbShowMissing         = getFlag( nFlags2, OOBIN_PTDEF_SHOWMISSING );
    maDefModel.mbShowItems           = getFlag( nFlags1, OOBIN_PTDEF_SHOWITEMS );
    maDefModel.mbDisableFieldList    = getFlag( nFlags1, OOBIN_PTDEF_DISABLEFIELDLIST );
    maDefModel.mbShowCalcMembers     = !getFlag( nFlags1, OOBIN_PTDEF_HIDECALCMEMBERS );
    maDefModel.mbVisualTotals        = !getFlag( nFlags1, OOBIN_PTDEF_WITHHIDDENTOTALS );
    maDefModel.mbShowDrill           = !getFlag( nFlags1, OOBIN_PTDEF_HIDEDRILL );
    maDefModel.mbPrintDrill          = getFlag( nFlags1, OOBIN_PTDEF_PRINTDRILL );
    maDefModel.mbEnableDrill         = getFlag( nFlags2, OOBIN_PTDEF_ENABLEDRILL );
    maDefModel.mbPreserveFormatting  = getFlag( nFlags2, OOBIN_PTDEF_PRESERVEFORMATTING );
    maDefModel.mbPageOverThenDown    = getFlag( nFlags2, OOBIN_PTDEF_PAGEOVERTHENDOWN );
    maDefModel.mbSubtotalHiddenItems = getFlag( nFlags2, OOBIN_PTDEF_SUBTOTALHIDDENITEMS );
    maDefModel.mbRowGrandTotals      = getFlag( nFlags2, OOBIN_PTDEF_ROWGRANDTOTALS );
    maDefModel.mbColGrandTotals      = getFlag( nFlags2, OOBIN_PTDEF_COLGRANDTOTALS );
    maDefModel.mbFieldPrintTitles    = getFlag( nFlags2, OOBIN_PTDEF_FIELDPRINTTITLES );
    maDefModel.mbItemPrintTitles     = getFlag( nFlags2, OOBIN_PTDEF_ITEMPRINTTITLES );
    maDefModel.mbMergeItem           = getFlag( nFlags2, OOBIN_PTDEF_MERGEITEM );
    maDefModel.mbShowEmptyRow        = getFlag( nFlags2, OOBIN_PTDEF_SHOWEMPTYROW );
    maDefModel.mbShowEmptyCol        = getFlag( nFlags2, OOBIN_PTDEF_SHOWEMPTYCOL );
    maDefModel.mbShowHeaders         = !getFlag( nFlags1, OOBIN_PTDEF_HIDEHEADERS );
    maDefModel.mbFieldListSortAsc    = getFlag( nFlags3, OOBIN_PTDEF_FIELDLISTSORTASC );
    maDefModel.mbCustomListSort      = !getFlag( nFlags3, OOBIN_PTDEF_NOCUSTOMLISTSORT );
}

void PivotTable::importPTLocation( RecordInputStream& rStrm, sal_Int16 nSheet )
{
    BinRange aBinRange;
    rStrm   >> aBinRange >> maLocationModel.mnFirstHeaderRow
            >> maLocationModel.mnFirstDataRow >> maLocationModel.mnFirstDataCol
            >> maLocationModel.mnRowPageCount >> maLocationModel.mnColPageCount;
    getAddressConverter().convertToCellRangeUnchecked( maLocationModel.maRange, aBinRange, nSheet );
}

void PivotTable::importPTRowFields( RecordInputStream& rStrm )
{
    importFields( maRowFields, rStrm );
}

void PivotTable::importPTColFields( RecordInputStream& rStrm )
{
    importFields( maColFields, rStrm );
}

void PivotTable::importPTPageField( RecordInputStream& rStrm )
{
    PTPageFieldModel aModel;
    sal_uInt8 nFlags;
    rStrm >> aModel.mnField >> aModel.mnItem;
    rStrm.skip( 4 );    // hierarchy
    rStrm >> nFlags;
    if( getFlag( nFlags, OOBIN_PTPAGEFIELD_HASNAME ) )
        rStrm >> aModel.maName;
    maPageFields.push_back( aModel );
}

void PivotTable::importPTDataField( RecordInputStream& rStrm )
{
    PTDataFieldModel aModel;
    sal_Int32 nSubtotal, nShowDataAs;
    sal_uInt8 nHasName;
    rStrm >> aModel.mnField >> nSubtotal >> nShowDataAs >> aModel.mnBaseField >> aModel.mnBaseItem >> aModel.mnNumFmtId >> nHasName;
    if( nHasName == 1 )
        rStrm >> aModel.maName;
    aModel.setBinSubtotal( nSubtotal );
    aModel.setBinShowDataAs( nShowDataAs );
    maDataFields.push_back( aModel );
}

void PivotTable::importPTDefinition( BiffInputStream& rStrm, sal_Int16 nSheet )
{
    BinRange aBinRange;
    sal_uInt16 nFlags, nTabNameLen, nDataNameLen;
    rStrm >> aBinRange;
    maLocationModel.mnFirstHeaderRow = rStrm.readuInt16();
    maLocationModel.mnFirstDataRow   = rStrm.readuInt16();
    maLocationModel.mnFirstDataCol   = rStrm.readuInt16();
    maDefModel.mnCacheId             = rStrm.readuInt16();
    rStrm.skip( 2 );                 // unused
    maDefModel.mbDataOnRows          = rStrm.readuInt16() == BIFF_PTDEF_ROWAXIS;
    maDefModel.mnDataPosition        = rStrm.readInt16();
    rStrm.skip( 2 );                 // number of fields
    rStrm >> maDefModel.mnRowFields >> maDefModel.mnColFields;
    rStrm.skip( 8 );                 // number of page fields, data fields, data rows, data columns
    rStrm >> nFlags;
    maDefModel.mnChartFormat         = rStrm.readuInt16();
    rStrm >> nTabNameLen >> nDataNameLen;
    maDefModel.maName                = lclReadPivotString( *this, rStrm, nTabNameLen );
    maDefModel.maDataCaption         = lclReadPivotString( *this, rStrm, nDataNameLen );

    maDefModel.mbRowGrandTotals  = getFlag( nFlags, BIFF_PTDEF_ROWGRANDTOTALS );
    maDefModel.mbColGrandTotals  = getFlag( nFlags, BIFF_PTDEF_COLGRANDTOTALS );

    getAddressConverter().convertToCellRangeUnchecked( maLocationModel.maRange, aBinRange, nSheet );
}

void PivotTable::importPTDefinition2( BiffInputStream& rStrm )
{
    if( getBiff() == BIFF8 )
    {
        sal_uInt16 nErrCaptLen, nMissCaptLen, nTagLen, nPageStyleLen, nTabStyleLen, nVacStyleLen;
        sal_uInt32 nFlags;
        rStrm.skip( 2 );    // number of formatting records
        rStrm >> nErrCaptLen >> nMissCaptLen >> nTagLen;
        rStrm.skip( 6 );    // number of selection records, page rows, page columns
        rStrm >> nFlags >> nPageStyleLen >> nTabStyleLen >> nVacStyleLen;
        maDefModel.maErrorCaption    = lclReadPivotString( *this, rStrm, nErrCaptLen );
        maDefModel.maMissingCaption  = lclReadPivotString( *this, rStrm, nMissCaptLen );
        maDefModel.maTag             = lclReadPivotString( *this, rStrm, nTagLen );
        maDefModel.maPageStyle       = lclReadPivotString( *this, rStrm, nPageStyleLen );
        maDefModel.maPivotTableStyle = lclReadPivotString( *this, rStrm, nTabStyleLen );
        maDefModel.maVacatedStyle    = lclReadPivotString( *this, rStrm, nVacStyleLen );

        maDefModel.mbShowError           = getFlag( nFlags, BIFF_PTDEF2_SHOWERROR );
        maDefModel.mbShowMissing         = getFlag( nFlags, BIFF_PTDEF2_SHOWMISSING );
        maDefModel.mbEnableDrill         = getFlag( nFlags, BIFF_PTDE2F_ENABLEDRILL );
        maDefModel.mbPreserveFormatting  = getFlag( nFlags, BIFF_PTDEF2_PRESERVEFORMATTING );
        maDefModel.mbPageOverThenDown    = getFlag( nFlags, BIFF_PTDEF2_PAGEOVERTHENDOWN );
        maDefModel.mbSubtotalHiddenItems = getFlag( nFlags, BIFF_PTDEF2_SUBTOTALHIDDENITEMS );
        maDefModel.mbMergeItem           = getFlag( nFlags, BIFF_PTDEF2_MERGEITEM );
    }
}

void PivotTable::importPTRowColFields( BiffInputStream& rStrm )
{
    // first PTROWCOLFIELDS record contains row fields unless there are no row fields
    if( (maDefModel.mnRowFields > 0) && maRowFields.empty() )
        importFields( maRowFields, rStrm, maDefModel.mnRowFields );
    else if( (maDefModel.mnColFields > 0) && maColFields.empty() )
        importFields( maColFields, rStrm, maDefModel.mnColFields );
}

void PivotTable::importPTPageFields( BiffInputStream& rStrm )
{
    while( rStrm.getRemaining() >= 6 )
    {
        PTPageFieldModel aModel;
        sal_Int16 nField, nItem;
        rStrm >> nField >> nItem;
        rStrm.skip( 2 );    // dropdown object ID
        aModel.mnField = nField;
        aModel.mnItem = (nItem == BIFF_PTPAGEFIELDS_ALLITEMS) ? OOBIN_PTPAGEFIELD_MULTIITEMS : nItem;
        maPageFields.push_back( aModel );
    }
}

void PivotTable::importPTDataField( BiffInputStream& rStrm )
{
    PTDataFieldModel aModel;
    sal_Int16 nField, nBaseField, nBaseItem;
    sal_uInt16 nSubtotal, nShowDataAs, nNumFmt, nNameLen;
    rStrm >> nField >> nSubtotal >> nShowDataAs >> nBaseField >> nBaseItem >> nNumFmt >> nNameLen;
    aModel.maName = lclReadPivotString( *this, rStrm, nNameLen );

    aModel.mnField = nField;
    aModel.setBinSubtotal( nSubtotal );
    aModel.setBinShowDataAs( nShowDataAs );
    aModel.mnBaseField = nBaseField;
    switch( nBaseItem )
    {
        case BIFF_PTDATAFIELD_PREVIOUS: aModel.mnBaseItem = OOX_PT_PREVIOUS_ITEM;   break;
        case BIFF_PTDATAFIELD_NEXT:     aModel.mnBaseItem = OOX_PT_NEXT_ITEM;       break;
        default:                        aModel.mnBaseItem = nBaseItem;
    }
    aModel.mnNumFmtId = nNumFmt;

    maDataFields.push_back( aModel );
}

PivotTableField& PivotTable::createTableField()
{
    sal_Int32 nFieldIndex = static_cast< sal_Int32 >( maFields.size() );
    PivotTableFieldVector::value_type xTableField( new PivotTableField( *this, nFieldIndex ) );
    maFields.push_back( xTableField );
    return *xTableField;
}

PivotTableFilter& PivotTable::createTableFilter()
{
    PivotTableFilterVector::value_type xTableFilter( new PivotTableFilter( *this ) );
    maFilters.push_back( xTableFilter );
    return *xTableFilter;
}

void PivotTable::finalizeImport()
{
    if( getAddressConverter().validateCellRange( maLocationModel.maRange, true, true ) )
    {
        mpPivotCache = getPivotCaches().importPivotCacheFragment( maDefModel.mnCacheId );
        if( mpPivotCache && mpPivotCache->isValidDataSource() && (maDefModel.maName.getLength() > 0) )
        {
            // clear destination area of the original pivot table
            try
            {
                Reference< XSheetOperation > xSheetOp( getCellRangeFromDoc( maLocationModel.maRange ), UNO_QUERY_THROW );
                using namespace ::com::sun::star::sheet::CellFlags;
                xSheetOp->clearContents( VALUE | DATETIME | STRING | FORMULA | HARDATTR | STYLES | EDITATTR | FORMATTED );
            }
            catch( Exception& )
            {
            }

            try
            {
                // create a new data pilot descriptor based on the source data
                Reference< XDataPilotTablesSupplier > xDPTablesSupp( getSheetFromDoc( maLocationModel.maRange.Sheet ), UNO_QUERY_THROW );
                Reference< XDataPilotTables > xDPTables( xDPTablesSupp->getDataPilotTables(), UNO_SET_THROW );
                mxDPDescriptor.set( xDPTables->createDataPilotDescriptor(), UNO_SET_THROW );
                mxDPDescriptor->setSourceRange( mpPivotCache->getSourceRange() );
                mxDPDescriptor->setTag( maDefModel.maTag );

                // global data pilot properties
                PropertySet aDescProp( mxDPDescriptor );
                aDescProp.setProperty( PROP_ColumnGrand, maDefModel.mbColGrandTotals );
                aDescProp.setProperty( PROP_RowGrand, maDefModel.mbRowGrandTotals );
                aDescProp.setProperty( PROP_ShowFilterButton, false );
                aDescProp.setProperty( PROP_DrillDownOnDoubleClick, maDefModel.mbEnableDrill );

                // finalize all fields, this finds field names and creates grouping fields
                maFields.forEachMem( &PivotTableField::finalizeImport, ::boost::cref( mxDPDescriptor ) );

                // all row fields
                for( IndexVector::iterator aIt = maRowFields.begin(), aEnd = maRowFields.end(); aIt != aEnd; ++aIt )
                    if( PivotTableField* pField = getTableField( *aIt ) )
                        pField->convertRowField();

                // all column fields
                for( IndexVector::iterator aIt = maColFields.begin(), aEnd = maColFields.end(); aIt != aEnd; ++aIt )
                    if( PivotTableField* pField = getTableField( *aIt ) )
                        pField->convertColField();

                // all page fields
                for( PageFieldVector::iterator aIt = maPageFields.begin(), aEnd = maPageFields.end(); aIt != aEnd; ++aIt )
                    if( PivotTableField* pField = getTableField( aIt->mnField ) )
                        pField->convertPageField( *aIt );

                // all hidden fields
                ::std::set< sal_Int32 > aVisFields;
                aVisFields.insert( maRowFields.begin(), maRowFields.end() );
                aVisFields.insert( maColFields.begin(), maColFields.end() );
                for( PageFieldVector::iterator aIt = maPageFields.begin(), aEnd = maPageFields.end(); aIt != aEnd; ++aIt )
                    aVisFields.insert( aIt->mnField );
                for( PivotTableFieldVector::iterator aBeg = maFields.begin(), aIt = aBeg, aEnd = maFields.end(); aIt != aEnd; ++aIt )
                    if( aVisFields.count( static_cast< sal_Int32 >( aIt - aBeg ) ) == 0 )
                        (*aIt)->convertHiddenField();

                // all data fields
                for( DataFieldVector::iterator aIt = maDataFields.begin(), aEnd = maDataFields.end(); aIt != aEnd; ++aIt )
                    if( PivotTableField* pField = getTableField( aIt->mnField ) )
                        pField->convertDataField( *aIt );

                // filters
                maFilters.forEachMem( &PivotTableFilter::finalizeImport );

                // calculate base position of table
                CellAddress aPos( maLocationModel.maRange.Sheet, maLocationModel.maRange.StartColumn, maLocationModel.maRange.StartRow );
                /*  If page fields exist, include them into the destination
                    area (they are excluded in Excel). Add an extra blank row. */
                if( !maPageFields.empty() )
                    aPos.Row = ::std::max< sal_Int32 >( static_cast< sal_Int32 >( aPos.Row - maPageFields.size() - 1 ), 0 );

                // insert the DataPilot table into the sheet
                xDPTables->insertNewByName( maDefModel.maName, aPos, mxDPDescriptor );
            }
            catch( Exception& )
            {
                OSL_ENSURE( false, "PivotTable::finalizeImport - exception while creating the DataPilot table" );
            }
        }
    }
}

void PivotTable::finalizeDateGroupingImport( const Reference< XDataPilotField >& rxBaseDPField, sal_Int32 nBaseFieldIdx )
{
    // process all fields, there is no chaining information in the cache fields
    maFields.forEachMem( &PivotTableField::finalizeDateGroupingImport, ::boost::cref( rxBaseDPField ), nBaseFieldIdx );
}

void PivotTable::finalizeParentGroupingImport( const Reference< XDataPilotField >& rxBaseDPField,
        const PivotCacheField& rBaseCacheField, PivotCacheGroupItemVector& orItemNames )
{
    // try to create parent group fields that group the items of the passed base field
    if( PivotTableField* pParentTableField = maFields.get( rBaseCacheField.getParentGroupField() ).get() )
        pParentTableField->finalizeParentGroupingImport( rxBaseDPField, orItemNames );
}

Reference< XDataPilotField > PivotTable::getDataPilotField( const OUString& rFieldName ) const
{
    Reference< XDataPilotField > xDPField;
    if( (rFieldName.getLength() > 0) && mxDPDescriptor.is() ) try
    {
        Reference< XNameAccess > xDPFieldsNA( mxDPDescriptor->getDataPilotFields(), UNO_QUERY_THROW );
        xDPField.set( xDPFieldsNA->getByName( rFieldName ), UNO_QUERY );
    }
    catch( Exception& )
    {
    }
    return xDPField;
}

Reference< XDataPilotField > PivotTable::getDataPilotField( sal_Int32 nFieldIdx ) const
{
    Reference< XDataPilotField > xDPField;
    if( const PivotTableField* pTableField = maFields.get( nFieldIdx ).get() )
        xDPField = getDataPilotField( pTableField->getDPFieldName() );
    return xDPField;
}

Reference< XDataPilotField > PivotTable::getDataLayoutField() const
{
    Reference< XDataPilotField > xDPField;
    try
    {
        Reference< XDataPilotDataLayoutFieldSupplier > xDPDataFieldSupp( mxDPDescriptor, UNO_QUERY_THROW );
        xDPField = xDPDataFieldSupp->getDataLayoutField();
    }
    catch( Exception& )
    {
    }
    return xDPField;
}

const PivotCacheField* PivotTable::getCacheField( sal_Int32 nFieldIdx ) const
{
    return mpPivotCache ? mpPivotCache->getCacheField( nFieldIdx ) : 0;
}

const PivotCacheField* PivotTable::getCacheFieldOfDataField( sal_Int32 nDataItemIdx ) const
{
    const PTDataFieldModel* pDataField = ContainerHelper::getVectorElement( maDataFields, nDataItemIdx );
    return pDataField ? getCacheField( pDataField->mnField ) : 0;
}

sal_Int32 PivotTable::getCacheDatabaseIndex( sal_Int32 nFieldIdx ) const
{
    return mpPivotCache ? mpPivotCache->getCacheDatabaseIndex( nFieldIdx ) : -1;
}

// private --------------------------------------------------------------------

PivotTableField* PivotTable::getTableField( sal_Int32 nFieldIdx )
{
    return (nFieldIdx == OOX_PT_DATALAYOUTFIELD) ? &maDataField : maFields.get( nFieldIdx ).get();
}

void PivotTable::importField( IndexVector& orFields, const AttributeList& rAttribs )
{
    orFields.push_back( rAttribs.getInteger( XML_x, -1 ) );
}

void PivotTable::importFields( IndexVector& orFields, RecordInputStream& rStrm )
{
    OSL_ENSURE( orFields.empty(), "PivotTable::importFields - multiple record instances" );
    orFields.clear();
    sal_Int32 nCount = rStrm.readInt32();
    OSL_ENSURE( 4 * nCount == rStrm.getRemaining(), "PivotTable::importFields - invalid field count" );
    nCount = static_cast< sal_Int32 >( rStrm.getRemaining() / 4 );
    for( sal_Int32 nIdx = 0; nIdx < nCount; ++nIdx )
        orFields.push_back( rStrm.readInt32() );
}

void PivotTable::importFields( IndexVector& orFields, BiffInputStream& rStrm, sal_Int32 nCount )
{
    OSL_ENSURE( orFields.empty(), "PivotTable::importFields - multiple record instances" );
    orFields.clear();
    OSL_ENSURE( 2 * nCount == rStrm.getRemaining(), "PivotTable::importFields - invalid field count" );
    nCount = static_cast< sal_Int32 >( rStrm.getRemaining() / 2 );
    for( sal_Int32 nIdx = 0; nIdx < nCount; ++nIdx )
        orFields.push_back( rStrm.readInt16() );
}

// ============================================================================

PivotTableBuffer::PivotTableBuffer( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper )
{
}

PivotTable& PivotTableBuffer::createPivotTable()
{
    PivotTableVector::value_type xTable( new PivotTable( *this ) );
    maTables.push_back( xTable );
    return *xTable;
}

void PivotTableBuffer::finalizeImport()
{
    maTables.forEachMem( &PivotTable::finalizeImport );
}

// ============================================================================

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
