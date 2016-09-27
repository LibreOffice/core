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

#include "pivottablebuffer.hxx"

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
#include <oox/helper/attributelist.hxx>
#include <oox/helper/containerhelper.hxx>
#include <oox/helper/propertyset.hxx>
#include <oox/token/properties.hxx>
#include "addressconverter.hxx"
#include "biffinputstream.hxx"

#include "dapiuno.hxx"
#include "dpobject.hxx"
#include "dpsave.hxx"
#include "dpdimsave.hxx"
#include "document.hxx"
#include "documentimport.hxx"

namespace oox {
namespace xls {

using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::uno;
using namespace com::sun::star;

namespace {

const sal_Int32 OOX_PT_DATALAYOUTFIELD              = -2;           /// Placeholder index of data layout field.

const sal_Int32 OOX_PT_PREVIOUS_ITEM                = 0x001000FC;   /// Calculation of data item result is based on previous item.
const sal_Int32 OOX_PT_NEXT_ITEM                    = 0x001000FD;   /// Calculation of data item result is based on next item.

const sal_uInt32 BIFF12_PTFIELD_DATAFIELD           = 0x00000008;
const sal_uInt32 BIFF12_PTFIELD_DEFAULT             = 0x00000100;
const sal_uInt32 BIFF12_PTFIELD_SUM                 = 0x00000200;
const sal_uInt32 BIFF12_PTFIELD_COUNTA              = 0x00000400;
const sal_uInt32 BIFF12_PTFIELD_AVERAGE             = 0x00000800;
const sal_uInt32 BIFF12_PTFIELD_MAX                 = 0x00001000;
const sal_uInt32 BIFF12_PTFIELD_MIN                 = 0x00002000;
const sal_uInt32 BIFF12_PTFIELD_PRODUCT             = 0x00004000;
const sal_uInt32 BIFF12_PTFIELD_COUNT               = 0x00008000;
const sal_uInt32 BIFF12_PTFIELD_STDDEV              = 0x00010000;
const sal_uInt32 BIFF12_PTFIELD_STDDEVP             = 0x00020000;
const sal_uInt32 BIFF12_PTFIELD_VAR                 = 0x00040000;
const sal_uInt32 BIFF12_PTFIELD_VARP                = 0x00080000;

const sal_uInt32 BIFF12_PTFIELD_SHOWALL             = 0x00000020;
const sal_uInt32 BIFF12_PTFIELD_OUTLINE             = 0x00000040;
const sal_uInt32 BIFF12_PTFIELD_INSERTBLANKROW      = 0x00000080;
const sal_uInt32 BIFF12_PTFIELD_SUBTOTALTOP         = 0x00000100;
const sal_uInt32 BIFF12_PTFIELD_INSERTPAGEBREAK     = 0x00000800;
const sal_uInt32 BIFF12_PTFIELD_AUTOSORT            = 0x00001000;
const sal_uInt32 BIFF12_PTFIELD_SORTASCENDING       = 0x00002000;
const sal_uInt32 BIFF12_PTFIELD_AUTOSHOW            = 0x00004000;
const sal_uInt32 BIFF12_PTFIELD_AUTOSHOWTOP         = 0x00008000;
const sal_uInt32 BIFF12_PTFIELD_MULTIPAGEITEMS      = 0x00080000;

const sal_uInt16 BIFF12_PTFITEM_HIDDEN              = 0x0001;
const sal_uInt16 BIFF12_PTFITEM_HIDEDETAILS         = 0x0002;

const sal_uInt8 BIFF12_PTPAGEFIELD_HASNAME          = 0x01;
const sal_Int32 BIFF12_PTPAGEFIELD_MULTIITEMS       = 0x001000FE;

const sal_uInt16 BIFF12_PTFILTER_HASNAME            = 0x0001;
const sal_uInt16 BIFF12_PTFILTER_HASDESCRIPTION     = 0x0002;
const sal_uInt16 BIFF12_PTFILTER_HASSTRVALUE1       = 0x0004;
const sal_uInt16 BIFF12_PTFILTER_HASSTRVALUE2       = 0x0008;

const sal_uInt8 BIFF12_TOP10FILTER_TOP              = 0x01;
const sal_uInt8 BIFF12_TOP10FILTER_PERCENT          = 0x02;

const sal_uInt32 BIFF12_PTDEF_SHOWITEMS             = 0x00000100;
const sal_uInt32 BIFF12_PTDEF_DISABLEFIELDLIST      = 0x00000400;
const sal_uInt32 BIFF12_PTDEF_HIDECALCMEMBERS       = 0x00001000;
const sal_uInt32 BIFF12_PTDEF_WITHHIDDENTOTALS      = 0x00002000;
const sal_uInt32 BIFF12_PTDEF_HIDEDRILL             = 0x00100000;
const sal_uInt32 BIFF12_PTDEF_PRINTDRILL            = 0x00200000;
const sal_uInt32 BIFF12_PTDEF_HIDEHEADERS           = 0x80000000;

const sal_uInt32 BIFF12_PTDEF_SHOWEMPTYROW          = 0x00000004;
const sal_uInt32 BIFF12_PTDEF_SHOWEMPTYCOL          = 0x00000008;
const sal_uInt32 BIFF12_PTDEF_ENABLEDRILL           = 0x00000020;
const sal_uInt32 BIFF12_PTDEF_PRESERVEFORMATTING    = 0x00000080;
const sal_uInt32 BIFF12_PTDEF_USEAUTOFORMAT         = 0x00000100;
const sal_uInt32 BIFF12_PTDEF_SHOWERROR             = 0x00000200;
const sal_uInt32 BIFF12_PTDEF_SHOWMISSING           = 0x00000400;
const sal_uInt32 BIFF12_PTDEF_PAGEOVERTHENDOWN      = 0x00000800;
const sal_uInt32 BIFF12_PTDEF_SUBTOTALHIDDENITEMS   = 0x00001000;
const sal_uInt32 BIFF12_PTDEF_ROWGRANDTOTALS        = 0x00002000;
const sal_uInt32 BIFF12_PTDEF_COLGRANDTOTALS        = 0x00004000;
const sal_uInt32 BIFF12_PTDEF_FIELDPRINTTITLES      = 0x00008000;
const sal_uInt32 BIFF12_PTDEF_ITEMPRINTTITLES       = 0x00020000;
const sal_uInt32 BIFF12_PTDEF_MERGEITEM             = 0x00040000;
const sal_uInt32 BIFF12_PTDEF_HASDATACAPTION        = 0x00080000;
const sal_uInt32 BIFF12_PTDEF_HASGRANDTOTALCAPTION  = 0x00100000;
const sal_uInt32 BIFF12_PTDEF_HASPAGESTYLE          = 0x00200000;
const sal_uInt32 BIFF12_PTDEF_HASPIVOTTABLESTYLE    = 0x00400000;
const sal_uInt32 BIFF12_PTDEF_HASVACATEDSTYLE       = 0x00800000;
const sal_uInt32 BIFF12_PTDEF_APPLYNUMFMT           = 0x01000000;
const sal_uInt32 BIFF12_PTDEF_APPLYFONT             = 0x02000000;
const sal_uInt32 BIFF12_PTDEF_APPLYALIGNMENT        = 0x04000000;
const sal_uInt32 BIFF12_PTDEF_APPLYBORDER           = 0x08000000;
const sal_uInt32 BIFF12_PTDEF_APPLYFILL             = 0x10000000;
const sal_uInt32 BIFF12_PTDEF_APPLYPROTECTION       = 0x20000000;
const sal_uInt32 BIFF12_PTDEF_HASTAG                = 0x40000000;

const sal_uInt32 BIFF12_PTDEF_NOERRORCAPTION        = 0x00000040;
const sal_uInt32 BIFF12_PTDEF_NOMISSINGCAPTION      = 0x00000080;
const sal_uInt32 BIFF12_PTDEF_HASROWHEADERCAPTION   = 0x00000400;
const sal_uInt32 BIFF12_PTDEF_HASCOLHEADERCAPTION   = 0x00000800;
const sal_uInt32 BIFF12_PTDEF_FIELDLISTSORTASC      = 0x00001000;
const sal_uInt32 BIFF12_PTDEF_NOCUSTOMLISTSORT      = 0x00004000;

const sal_uInt8 BIFF12_PTDEF_ROWAXIS                = 1;
const sal_uInt8 BIFF12_PTDEF_COLAXIS                = 2;

} // namespace

PTFieldItemModel::PTFieldItemModel() :
    mnCacheItem( -1 ),
    mnType( XML_data ),
    mbShowDetails( true ),
    mbHidden( false )
{
}

void PTFieldItemModel::setBiffType( sal_uInt16 nType )
{
    static const sal_Int32 spnTypes[] = { XML_data, XML_default,
        XML_sum, XML_countA, XML_avg, XML_max, XML_min, XML_product, XML_count,
        XML_stdDev, XML_stdDevP, XML_var, XML_varP, XML_grand, XML_blank };
    mnType = STATIC_ARRAY_SELECT( spnTypes, nType, XML_data );
}

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

void PTFieldModel::setBiffAxis( sal_uInt8 nAxis )
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

PTPageFieldModel::PTPageFieldModel() :
    mnField( -1 ),
    mnItem( BIFF12_PTPAGEFIELD_MULTIITEMS )
{
}

PTDataFieldModel::PTDataFieldModel() :
    mnField( -1 ),
    mnSubtotal( XML_sum ),
    mnShowDataAs( XML_normal ),
    mnBaseField( -1 ),
    mnBaseItem( -1 ),
    mnNumFmtId( 0 )
{
}

void PTDataFieldModel::setBiffSubtotal( sal_Int32 nSubtotal )
{
    static const sal_Int32 spnSubtotals[] = { XML_sum, XML_count, XML_average, XML_max, XML_min, XML_product, XML_countNums, XML_stdDev, XML_stdDevp, XML_var, XML_varp };
    mnSubtotal = STATIC_ARRAY_SELECT( spnSubtotals, nSubtotal, XML_TOKEN_INVALID );
}

void PTDataFieldModel::setBiffShowDataAs( sal_Int32 nShowDataAs )
{
    static const sal_Int32 spnShowDataAs[] = { XML_normal, XML_difference, XML_percent, XML_percentDiff, XML_runTotal, XML_percentOfRow, XML_percentOfCol, XML_percentOfTotal, XML_index };
    mnShowDataAs = STATIC_ARRAY_SELECT( spnShowDataAs, nShowDataAs, XML_TOKEN_INVALID );
}

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
    aModel.msCaption     = rAttribs.getXString( XML_n, OUString() );
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

void PivotTableField::importPTField( SequenceInputStream& rStrm )
{
    sal_uInt32 nFlags1, nFlags2;
    nFlags1 = rStrm.readuInt32();
    maModel.mnNumFmtId = rStrm.readInt32();
    nFlags2 = rStrm.readuInt32();
    maModel.mnAutoShowItems = rStrm.readInt32();
    maModel.mnAutoShowRankBy = rStrm.readInt32();

    maModel.setBiffAxis( extractValue< sal_uInt8 >( nFlags1, 0, 3 ) );
    maModel.mbDataField       = getFlag( nFlags1, BIFF12_PTFIELD_DATAFIELD );
    maModel.mbDefaultSubtotal = getFlag( nFlags1, BIFF12_PTFIELD_DEFAULT );
    maModel.mbSumSubtotal     = getFlag( nFlags1, BIFF12_PTFIELD_SUM );
    maModel.mbCountASubtotal  = getFlag( nFlags1, BIFF12_PTFIELD_COUNTA );
    maModel.mbAverageSubtotal = getFlag( nFlags1, BIFF12_PTFIELD_AVERAGE );
    maModel.mbMaxSubtotal     = getFlag( nFlags1, BIFF12_PTFIELD_MAX );
    maModel.mbMinSubtotal     = getFlag( nFlags1, BIFF12_PTFIELD_MIN );
    maModel.mbProductSubtotal = getFlag( nFlags1, BIFF12_PTFIELD_PRODUCT );
    maModel.mbCountSubtotal   = getFlag( nFlags1, BIFF12_PTFIELD_COUNT );
    maModel.mbStdDevSubtotal  = getFlag( nFlags1, BIFF12_PTFIELD_STDDEV );
    maModel.mbStdDevPSubtotal = getFlag( nFlags1, BIFF12_PTFIELD_STDDEVP );
    maModel.mbVarSubtotal     = getFlag( nFlags1, BIFF12_PTFIELD_VAR );
    maModel.mbVarPSubtotal    = getFlag( nFlags1, BIFF12_PTFIELD_VARP );

    maModel.mbShowAll         = getFlag( nFlags2, BIFF12_PTFIELD_SHOWALL );
    maModel.mbOutline         = getFlag( nFlags2, BIFF12_PTFIELD_OUTLINE );
    maModel.mbSubtotalTop     = getFlag( nFlags2, BIFF12_PTFIELD_SUBTOTALTOP );
    maModel.mbInsertBlankRow  = getFlag( nFlags2, BIFF12_PTFIELD_INSERTBLANKROW );
    maModel.mbInsertPageBreak = getFlag( nFlags2, BIFF12_PTFIELD_INSERTPAGEBREAK );
    maModel.mbAutoShow        = getFlag( nFlags2, BIFF12_PTFIELD_AUTOSHOW );
    maModel.mbTopAutoShow     = getFlag( nFlags2, BIFF12_PTFIELD_AUTOSHOWTOP );
    maModel.mbMultiPageItems  = getFlag( nFlags2, BIFF12_PTFIELD_MULTIPAGEITEMS );

    bool bAutoSort = getFlag( nFlags2, BIFF12_PTFIELD_AUTOSORT );
    bool bAscending = getFlag( nFlags2, BIFF12_PTFIELD_SORTASCENDING );
    maModel.mnSortType = bAutoSort ? (bAscending ? XML_ascending : XML_descending) : XML_manual;
}

void PivotTableField::importPTFItem( SequenceInputStream& rStrm )
{
    PTFieldItemModel aModel;
    sal_uInt8 nType;
    sal_uInt16 nFlags;
    nType = rStrm.readuChar();
    nFlags = rStrm.readuInt16();
    aModel.mnCacheItem = rStrm.readInt32();

    aModel.setBiffType( nType );
    aModel.mbShowDetails = !getFlag( nFlags, BIFF12_PTFITEM_HIDEDETAILS );
    aModel.mbHidden      = getFlag( nFlags, BIFF12_PTFITEM_HIDDEN );

    maItems.push_back( aModel );
}

void PivotTableField::importPTReference( SequenceInputStream& rStrm )
{
    maModel.mnSortRefField = rStrm.readInt32();
}

void PivotTableField::importPTReferenceItem( SequenceInputStream& rStrm )
{
    maModel.mnSortRefItem = rStrm.readInt32();
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
        OSL_ENSURE( !maDPFieldName.isEmpty(), "PivotTableField::finalizeImport - no field name in source data found" );

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
    if( maDPFieldName.isEmpty() )    // prevent endless loops if file format is broken
    {
        if( PivotCacheField* pCacheField = mrPivotTable.getCacheField( mnFieldIndex ) )
        {
            if( !pCacheField->isDatabaseField() && pCacheField->hasDateGrouping() && (pCacheField->getGroupBaseField() == nBaseFieldIdx) )
            {
                maDPFieldName = pCacheField->createDateGroupField( rxBaseDPField );
                pCacheField->setFinalGroupName(maDPFieldName);
                OSL_ENSURE( !maDPFieldName.isEmpty(), "PivotTableField::finalizeDateGroupingImport - cannot create date group field" );
            }
        }
    }
}

void PivotTableField::finalizeParentGroupingImport( const Reference< XDataPilotField >& rxBaseDPField,  const PivotCacheField& rBaseCacheField, PivotCacheGroupItemVector& orItemNames )
{
    if( maDPFieldName.isEmpty() )    // prevent endless loops if file format is broken
    {
        if( PivotCacheField* pCacheField = mrPivotTable.getCacheField( mnFieldIndex ) )
        {
            // data field can have user defined groupname captions, apply them
            // if they do
            IdCaptionPairList captionList;
            for( ItemModelVector::iterator aIt = maItems.begin(), aEnd = maItems.end(); aIt != aEnd; ++aIt )
            {
                if ( aIt->mnType == XML_data  && aIt->msCaption.getLength() )
                    captionList.push_back( IdCaptionPair( aIt->mnCacheItem, aIt->msCaption ) );
            }
            if ( !captionList.empty() )
                pCacheField->applyItemCaptions( captionList );

            maDPFieldName = pCacheField->createParentGroupField( rxBaseDPField, rBaseCacheField, orItemNames );
            pCacheField->setFinalGroupName(maDPFieldName);
            // on success, try to create nested group fields
            Reference< XDataPilotField > xDPField = mrPivotTable.getDataPilotField( maDPFieldName );
            if( xDPField.is() )
                mrPivotTable.finalizeParentGroupingImport( xDPField, *pCacheField, orItemNames );
        }
    }
}

void PivotTableField::finalizeImportBasedOnCache( const Reference< XDataPilotDescriptor >& rxDPDesc)
{
    /*  Process all fields based on source data, other fields (e.g. group
        fields) are processed based on cache fields.*/
    Reference< XDataPilotField > xDPField;
    sal_Int32 nDatabaseIdx = mrPivotTable.getCacheDatabaseIndex( mnFieldIndex );
    if( (nDatabaseIdx >= 0) && rxDPDesc.is() ) try
    {
        // Try to get the source field and its name from passed DataPilot descriptor
        Reference< XIndexAccess > xDPFieldsIA( rxDPDesc->getDataPilotFields(), UNO_SET_THROW );
        xDPField.set( xDPFieldsIA->getByIndex( nDatabaseIdx ), UNO_QUERY_THROW );
        Reference< XNamed > xDPFieldName( xDPField, UNO_QUERY_THROW );
        maDPFieldName = xDPFieldName->getName();
        SAL_WARN_IF( maDPFieldName.isEmpty(), "sc.filter", "PivotTableField::finalizeImportBasedOnCache - no field name in source data found" );
    }
    catch( Exception& )
    {
    }

    // Use group names already generated for another table using the same group field.
    if( const PivotCacheField* pCacheField = mrPivotTable.getCacheField( mnFieldIndex ) )
    {
        if(!pCacheField->getFinalGroupName().isEmpty())
            maDPFieldName = pCacheField->getFinalGroupName();
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
            OSL_ENSURE( rPageField.mnItem == BIFF12_PTPAGEFIELD_MULTIITEMS, "PivotTableField::convertPageField - unexpected cache item index" );
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
            default:            OSL_FAIL( "PivotTableField::convertDataField - unknown aggregation function" );
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
        // TODO: Use this to set properties directly, bypassing the slow uno layer.
        ScDPObject* pDPObj = mrPivotTable.getDPObject();

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

            // auto show (OOXML/BIFF12 only)
            if( maModel.mbAutoShow )
            {
                DataPilotFieldAutoShowInfo aAutoShowInfo;
                aAutoShowInfo.IsEnabled = true;
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
                    mrPivotTable.getCacheFieldOfDataField( maModel.mnSortRefItem ) : nullptr;
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
            if (const PivotCacheField* pCacheField = mrPivotTable.getCacheField(mnFieldIndex))
            {
                ScDPSaveData* pSaveData = pDPObj->GetSaveData();
                ScDPSaveDimension* pDim = pSaveData->GetDimensionByName(pCacheField->getName());

                try
                {
                    for( ItemModelVector::iterator aIt = maItems.begin(), aEnd = maItems.end(); aIt != aEnd; ++aIt )
                    {
                        if (aIt->mnType != XML_data)
                            continue;

                        const PivotCacheItem* pSharedItem = pCacheField->getCacheItem(aIt->mnCacheItem);
                        if (!pSharedItem)
                            continue;

                        try
                        {
                            ScDPSaveMember* pMem = pDim->GetMemberByName(pSharedItem->getName());
                            pMem->SetShowDetails(aIt->mbShowDetails);
                            pMem->SetIsVisible(!aIt->mbHidden);
                        }
                        catch( Exception& )
                        {
                            // catch every failed container access to be able to process following items
                        }
                    }
                }
                catch (const Exception&) {}
            }
        }
    }
    return xDPField;
}

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

void PivotTableFilter::importPTFilter( SequenceInputStream& rStrm )
{
    sal_Int32 nType;
    sal_uInt16 nFlags;
    maModel.mnField = rStrm.readInt32();
    maModel.mnMemPropField = rStrm.readInt32();
    nType = rStrm.readInt32();
    rStrm.skip( 4 );    // unused
    maModel.mnId = rStrm.readInt32();
    maModel.mnMeasureField = rStrm.readInt32();
    maModel.mnMeasureHier = rStrm.readInt32();
    nFlags = rStrm.readuInt16();
    if( getFlag( nFlags, BIFF12_PTFILTER_HASNAME ) )
        rStrm >> maModel.maName;
    if( getFlag( nFlags, BIFF12_PTFILTER_HASDESCRIPTION ) )
        rStrm >> maModel.maDescription;
    if( getFlag( nFlags, BIFF12_PTFILTER_HASSTRVALUE1 ) )
        rStrm >> maModel.maStrValue1;
    if( getFlag( nFlags, BIFF12_PTFILTER_HASSTRVALUE2 ) )
        rStrm >> maModel.maStrValue2;

    static const sal_Int32 spnTypes[] =
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

void PivotTableFilter::importTop10Filter( SequenceInputStream& rStrm )
{
    sal_uInt8 nFlags;
    nFlags = rStrm.readuChar();
    maModel.mfValue = rStrm.readDouble();

    SAL_WARN_IF(
        getFlag(nFlags, BIFF12_TOP10FILTER_PERCENT) != (maModel.mnType == XML_percent),
        "sc.filter",
        "PivotTableFilter::importTop10 - unexpected value of percent attribute");
    maModel.mbTopFilter = getFlag( nFlags, BIFF12_TOP10FILTER_TOP );
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
            aAutoShowInfo.IsEnabled = true;
            aAutoShowInfo.ShowItemsMode = maModel.mbTopFilter ? DataPilotFieldShowItemsMode::FROM_TOP : DataPilotFieldShowItemsMode::FROM_BOTTOM;
            aAutoShowInfo.ItemCount = getLimitedValue< sal_Int32, double >( maModel.mfValue, 0, SAL_MAX_INT32 );
            if( const PivotCacheField* pCacheField = mrPivotTable.getCacheFieldOfDataField( maModel.mnMeasureField ) )
                aAutoShowInfo.DataField = pCacheField->getName();
            aPropSet.setProperty( PROP_AutoShowInfo, aAutoShowInfo );
        }
    }
}

PTDefinitionModel::PTDefinitionModel() :
    mnCacheId( -1 ),
    mnDataPosition( 0 ),
    mnPageWrap( 0 ),
    mnIndent( 1 ),
    mnChartFormat( 0 ),
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
    mbUseAutoFormat( false ),
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

PTLocationModel::PTLocationModel() :
    mnFirstHeaderRow( 0 ),
    mnFirstDataRow( 0 ),
    mnFirstDataCol( 0 ),
    mnRowPageCount( 0 ),
    mnColPageCount( 0 )
{
}

PivotTable::PivotTable( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    mpDPObject(nullptr),
    maDataField( *this, OOX_PT_DATALAYOUTFIELD ),
    mpPivotCache( nullptr )
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
    maDefModel.mnAutoFormatId        = rAttribs.getInteger( XML_autoFormatId, 0 );
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
    maDefModel.mbUseAutoFormat       = rAttribs.getBool( XML_useAutoFormatting, false );
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
    maDefModel.mbApplyNumFmt         = rAttribs.getBool( XML_applyNumberFormats, false );
    maDefModel.mbApplyFont           = rAttribs.getBool( XML_applyFontFormats, false );
    maDefModel.mbApplyAlignment      = rAttribs.getBool( XML_applyAlignmentFormats, false );
    maDefModel.mbApplyBorder         = rAttribs.getBool( XML_applyBorderFormats, false );
    maDefModel.mbApplyFill           = rAttribs.getBool( XML_applyPatternFormats, false );
    // OOXML and BIFF12 documentation differ: OOXML mentions width/height, BIFF12 mentions protection
    maDefModel.mbApplyProtection     = rAttribs.getBool( XML_applyWidthHeightFormats, false );
}

void PivotTable::importLocation( const AttributeList& rAttribs, sal_Int16 nSheet )
{
    AddressConverter::convertToCellRangeUnchecked( maLocationModel.maRange, rAttribs.getString( XML_ref, OUString() ), nSheet );
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
    aModel.mnItem      = rAttribs.getInteger( XML_item, BIFF12_PTPAGEFIELD_MULTIITEMS );
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

void PivotTable::importPTDefinition( SequenceInputStream& rStrm )
{
    sal_uInt32 nFlags1, nFlags2, nFlags3;
    sal_uInt8 nDataAxis;
    nFlags1 = rStrm.readuInt32();
    nFlags2 = rStrm.readuInt32();
    nFlags3 = rStrm.readuInt32();
    nDataAxis = rStrm.readuChar();
    maDefModel.mnPageWrap = rStrm.readuInt8();
    rStrm.skip( 2 );    // refresh versions
    maDefModel.mnDataPosition = rStrm.readInt32();
    maDefModel.mnAutoFormatId = rStrm.readuInt16();
    rStrm.skip( 2 );    // unused
    maDefModel.mnChartFormat = rStrm.readInt32();
    maDefModel.mnCacheId = rStrm.readInt32();
    rStrm >> maDefModel.maName;
    if( getFlag( nFlags2, BIFF12_PTDEF_HASDATACAPTION ) )
        rStrm >> maDefModel.maDataCaption;
    if( getFlag( nFlags2, BIFF12_PTDEF_HASGRANDTOTALCAPTION ) )
        rStrm >> maDefModel.maGrandTotalCaption;
    if( !getFlag( nFlags3, BIFF12_PTDEF_NOERRORCAPTION ) )   // missing flag indicates existing string
        rStrm >> maDefModel.maErrorCaption;
    if( !getFlag( nFlags3, BIFF12_PTDEF_NOMISSINGCAPTION ) ) // missing flag indicates existing string
        rStrm >> maDefModel.maMissingCaption;
    if( getFlag( nFlags2, BIFF12_PTDEF_HASPAGESTYLE ) )
        rStrm >> maDefModel.maPageStyle;
    if( getFlag( nFlags2, BIFF12_PTDEF_HASPIVOTTABLESTYLE ) )
        rStrm >> maDefModel.maPivotTableStyle;
    if( getFlag( nFlags2, BIFF12_PTDEF_HASVACATEDSTYLE ) )
        rStrm >> maDefModel.maVacatedStyle;
    if( getFlag( nFlags2, BIFF12_PTDEF_HASTAG ) )
        rStrm >> maDefModel.maTag;
    if( getFlag( nFlags3, BIFF12_PTDEF_HASCOLHEADERCAPTION ) )   // TODO: right order (col/row)? spec is unclear
        rStrm >> maDefModel.maColHeaderCaption;
    if( getFlag( nFlags3, BIFF12_PTDEF_HASROWHEADERCAPTION ) )
        rStrm >> maDefModel.maRowHeaderCaption;

    SAL_WARN_IF(
        (nDataAxis != BIFF12_PTDEF_ROWAXIS) && (nDataAxis != BIFF12_PTDEF_COLAXIS),
        "sc.filter",
        "PivotTable::importPTDefinition - unexpected axis position for data field");

    maDefModel.mnIndent              = extractValue< sal_uInt8 >( nFlags1, 24, 7 );
    maDefModel.mbDataOnRows          = nDataAxis == BIFF12_PTDEF_ROWAXIS;
    maDefModel.mbShowError           = getFlag( nFlags2, BIFF12_PTDEF_SHOWERROR );
    maDefModel.mbShowMissing         = getFlag( nFlags2, BIFF12_PTDEF_SHOWMISSING );
    maDefModel.mbShowItems           = getFlag( nFlags1, BIFF12_PTDEF_SHOWITEMS );
    maDefModel.mbDisableFieldList    = getFlag( nFlags1, BIFF12_PTDEF_DISABLEFIELDLIST );
    maDefModel.mbShowCalcMembers     = !getFlag( nFlags1, BIFF12_PTDEF_HIDECALCMEMBERS );
    maDefModel.mbVisualTotals        = !getFlag( nFlags1, BIFF12_PTDEF_WITHHIDDENTOTALS );
    maDefModel.mbShowDrill           = !getFlag( nFlags1, BIFF12_PTDEF_HIDEDRILL );
    maDefModel.mbPrintDrill          = getFlag( nFlags1, BIFF12_PTDEF_PRINTDRILL );
    maDefModel.mbEnableDrill         = getFlag( nFlags2, BIFF12_PTDEF_ENABLEDRILL );
    maDefModel.mbPreserveFormatting  = getFlag( nFlags2, BIFF12_PTDEF_PRESERVEFORMATTING );
    maDefModel.mbUseAutoFormat       = getFlag( nFlags2, BIFF12_PTDEF_USEAUTOFORMAT );
    maDefModel.mbPageOverThenDown    = getFlag( nFlags2, BIFF12_PTDEF_PAGEOVERTHENDOWN );
    maDefModel.mbSubtotalHiddenItems = getFlag( nFlags2, BIFF12_PTDEF_SUBTOTALHIDDENITEMS );
    maDefModel.mbRowGrandTotals      = getFlag( nFlags2, BIFF12_PTDEF_ROWGRANDTOTALS );
    maDefModel.mbColGrandTotals      = getFlag( nFlags2, BIFF12_PTDEF_COLGRANDTOTALS );
    maDefModel.mbFieldPrintTitles    = getFlag( nFlags2, BIFF12_PTDEF_FIELDPRINTTITLES );
    maDefModel.mbItemPrintTitles     = getFlag( nFlags2, BIFF12_PTDEF_ITEMPRINTTITLES );
    maDefModel.mbMergeItem           = getFlag( nFlags2, BIFF12_PTDEF_MERGEITEM );
    maDefModel.mbApplyNumFmt         = getFlag( nFlags2, BIFF12_PTDEF_APPLYNUMFMT );
    maDefModel.mbApplyFont           = getFlag( nFlags2, BIFF12_PTDEF_APPLYFONT );
    maDefModel.mbApplyAlignment      = getFlag( nFlags2, BIFF12_PTDEF_APPLYALIGNMENT );
    maDefModel.mbApplyBorder         = getFlag( nFlags2, BIFF12_PTDEF_APPLYBORDER );
    maDefModel.mbApplyFill           = getFlag( nFlags2, BIFF12_PTDEF_APPLYFILL );
    maDefModel.mbApplyProtection     = getFlag( nFlags2, BIFF12_PTDEF_APPLYPROTECTION );
    maDefModel.mbShowEmptyRow        = getFlag( nFlags2, BIFF12_PTDEF_SHOWEMPTYROW );
    maDefModel.mbShowEmptyCol        = getFlag( nFlags2, BIFF12_PTDEF_SHOWEMPTYCOL );
    maDefModel.mbShowHeaders         = !getFlag( nFlags1, BIFF12_PTDEF_HIDEHEADERS );
    maDefModel.mbFieldListSortAsc    = getFlag( nFlags3, BIFF12_PTDEF_FIELDLISTSORTASC );
    maDefModel.mbCustomListSort      = !getFlag( nFlags3, BIFF12_PTDEF_NOCUSTOMLISTSORT );
}

void PivotTable::importPTLocation( SequenceInputStream& rStrm, sal_Int16 nSheet )
{
    BinRange aBinRange;
    rStrm >> aBinRange;
    maLocationModel.mnFirstHeaderRow = rStrm.readInt32();
    maLocationModel.mnFirstDataRow = rStrm.readInt32();
    maLocationModel.mnFirstDataCol = rStrm.readInt32();
    maLocationModel.mnRowPageCount = rStrm.readInt32();
    maLocationModel.mnColPageCount = rStrm.readInt32();
    AddressConverter::convertToCellRangeUnchecked( maLocationModel.maRange, aBinRange, nSheet );
}

void PivotTable::importPTRowFields( SequenceInputStream& rStrm )
{
    importFields( maRowFields, rStrm );
}

void PivotTable::importPTColFields( SequenceInputStream& rStrm )
{
    importFields( maColFields, rStrm );
}

void PivotTable::importPTPageField( SequenceInputStream& rStrm )
{
    PTPageFieldModel aModel;
    sal_uInt8 nFlags;
    aModel.mnField = rStrm.readInt32();
    aModel.mnItem = rStrm.readInt32();
    rStrm.skip( 4 );    // hierarchy
    nFlags = rStrm.readuChar();
    if( getFlag( nFlags, BIFF12_PTPAGEFIELD_HASNAME ) )
        rStrm >> aModel.maName;
    maPageFields.push_back( aModel );
}

void PivotTable::importPTDataField( SequenceInputStream& rStrm )
{
    PTDataFieldModel aModel;
    sal_Int32 nSubtotal, nShowDataAs;
    sal_uInt8 nHasName;
    aModel.mnField = rStrm.readInt32(  );
    nSubtotal = rStrm.readInt32();
    nShowDataAs = rStrm.readInt32();
    aModel.mnBaseField = rStrm.readInt32();
    aModel.mnBaseItem = rStrm.readInt32();
    aModel.mnNumFmtId = rStrm.readInt32();
    nHasName = rStrm.readuChar();
    if( nHasName == 1 )
        rStrm >> aModel.maName;
    aModel.setBiffSubtotal( nSubtotal );
    aModel.setBiffShowDataAs( nShowDataAs );
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
        if( mpPivotCache && mpPivotCache->isValidDataSource() && !maDefModel.maName.isEmpty() )
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

                // TODO: This is a hack. Eventually we need to convert the whole thing to the internal API.
                ScDataPilotDescriptorBase* pImpl = ScDataPilotDescriptorBase::getImplementation(mxDPDescriptor);
                if (!pImpl)
                    return;

                mpDPObject = pImpl->GetDPObject();
                if (!mpDPObject)
                    return;

                // global data pilot properties
                PropertySet aDescProp( mxDPDescriptor );
                aDescProp.setProperty( PROP_ColumnGrand, maDefModel.mbColGrandTotals );
                aDescProp.setProperty( PROP_RowGrand, maDefModel.mbRowGrandTotals );
                aDescProp.setProperty( PROP_ShowFilterButton, false );
                aDescProp.setProperty( PROP_DrillDownOnDoubleClick, maDefModel.mbEnableDrill );

                // finalize all fields, this finds field names and creates grouping fields
                finalizeFieldsImport();

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
                {
                    if( const PivotCacheField* pCacheField = getCacheField( aIt->mnField  ) )
                    {
                        if ( pCacheField-> getGroupBaseField() != -1 )
                            aIt->mnField = pCacheField-> getGroupBaseField();
                    }
                    if( PivotTableField* pField = getTableField( aIt->mnField ) )
                        pField->convertDataField( *aIt );
                }

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
                OSL_FAIL( "PivotTable::finalizeImport - exception while creating the DataPilot table" );
            }
        }
    }
}

void PivotTable::finalizeFieldsImport()
{
    if (maFields.empty())
        return;

    /* Check whether group fields are already imported for an other table
       sharing the same groups. */
    ScDPObject* pDPObj = getDPObject();
    const ScDocument& rDoc = getDocImport().getDoc();
    if (rDoc.HasPivotTable())
    {
        const ScDPCollection* pDPCollection = rDoc.GetDPCollection();
        assert(pDPCollection != nullptr);
        const ScDPDimensionSaveData* pGroups = nullptr;
        bool bRefFound = pDPCollection->GetReferenceGroups(*pDPObj, &pGroups);
        // Apply reference groups on this table.
        if (bRefFound && pGroups && pGroups->HasGroupDimensions()) {
            ScDPSaveData* pSaveData = pDPObj->GetSaveData();
            if (pSaveData) {
                pSaveData->SetDimensionData(pGroups);
                pDPObj->ReloadGroupTableData();
                maFields.forEachMem(&PivotTableField::finalizeImportBasedOnCache, ::std::cref(mxDPDescriptor));
                return;
            }

        }
    }
    maFields.forEachMem(&PivotTableField::finalizeImport, ::std::cref(mxDPDescriptor));
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
        pParentTableField->finalizeParentGroupingImport( rxBaseDPField, rBaseCacheField, orItemNames );
}

Reference< XDataPilotField > PivotTable::getDataPilotField( const OUString& rFieldName ) const
{
    Reference< XDataPilotField > xDPField;
    if( !rFieldName.isEmpty() && mxDPDescriptor.is() ) try
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

PivotCacheField* PivotTable::getCacheField( sal_Int32 nFieldIdx )
{
    return mpPivotCache ? mpPivotCache->getCacheField( nFieldIdx ) : nullptr;
}

const PivotCacheField* PivotTable::getCacheField( sal_Int32 nFieldIdx ) const
{
    return mpPivotCache ? mpPivotCache->getCacheField( nFieldIdx ) : nullptr;
}

const PivotCacheField* PivotTable::getCacheFieldOfDataField( sal_Int32 nDataItemIdx ) const
{
    const PTDataFieldModel* pDataField = ContainerHelper::getVectorElement( maDataFields, nDataItemIdx );
    return pDataField ? getCacheField( pDataField->mnField ) : nullptr;
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

void PivotTable::importFields( IndexVector& orFields, SequenceInputStream& rStrm )
{
    OSL_ENSURE( orFields.empty(), "PivotTable::importFields - multiple record instances" );
    orFields.clear();
    sal_Int32 nCount = rStrm.readInt32();
    OSL_ENSURE( 4 * nCount == rStrm.getRemaining(), "PivotTable::importFields - invalid field count" );
    nCount = static_cast< sal_Int32 >( rStrm.getRemaining() / 4 );
    for( sal_Int32 nIdx = 0; nIdx < nCount; ++nIdx )
        orFields.push_back( rStrm.readInt32() );
}

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

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
