/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: scenariobuffer.cxx,v $
 * $Revision: 1.1 $
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

#include "oox/xls/scenariobuffer.hxx"
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sheet/XScenario.hpp>
#include <com/sun/star/sheet/XScenarios.hpp>
#include <com/sun/star/sheet/XScenariosSupplier.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include "properties.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/helper/recordinputstream.hxx"
#include "oox/xls/addressconverter.hxx"
#include "oox/xls/biffinputstream.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::UNO_SET_THROW;
using ::com::sun::star::container::XIndexAccess;
using ::com::sun::star::container::XNameAccess;
using ::com::sun::star::table::CellAddress;
using ::com::sun::star::table::CellRangeAddress;
using ::com::sun::star::table::XCell;
using ::com::sun::star::sheet::XScenario;
using ::com::sun::star::sheet::XScenarios;
using ::com::sun::star::sheet::XScenariosSupplier;
using ::com::sun::star::sheet::XSpreadsheet;

namespace oox {
namespace xls {

// ============================================================================

namespace {

const sal_Int32 BIFF_SCENARIO_DELETED       = 0x4000;

} // namespace

// ============================================================================

ScenarioCellModel::ScenarioCellModel() :
    mnNumFmtId( 0 ),
    mbDeleted( false )
{
}

// ----------------------------------------------------------------------------

ScenarioModel::ScenarioModel() :
    mbLocked( false ),
    mbHidden( false )
{
}

// ----------------------------------------------------------------------------

Scenario::Scenario( const WorkbookHelper& rHelper, sal_Int16 nSheet ) :
    WorkbookHelper( rHelper ),
    mnSheet( nSheet )
{
}

void Scenario::importScenario( const AttributeList& rAttribs )
{
    maModel.maName    = rAttribs.getXString( XML_name, OUString() );
    maModel.maComment = rAttribs.getXString( XML_comment, OUString() );
    maModel.maUser    = rAttribs.getXString( XML_user, OUString() );
    maModel.mbLocked  = rAttribs.getBool( XML_locked, false );
    maModel.mbHidden  = rAttribs.getBool( XML_hidden, false );
}

void Scenario::importInputCells( const AttributeList& rAttribs )
{
    ScenarioCellModel aModel;
    getAddressConverter().convertToCellAddressUnchecked( aModel.maPos, rAttribs.getString( XML_r, OUString() ), mnSheet );
    aModel.maValue    = rAttribs.getXString( XML_val, OUString() );
    aModel.mnNumFmtId = rAttribs.getInteger( XML_numFmtId, 0 );
    aModel.mbDeleted  = rAttribs.getBool( XML_deleted, false );
    maCells.push_back( aModel );
}

void Scenario::importScenario( RecordInputStream& rStrm )
{
    rStrm.skip( 2 );    // cell count
    // two longs instead of flag field
    maModel.mbLocked = rStrm.readInt32() != 0;
    maModel.mbHidden = rStrm.readInt32() != 0;
    rStrm >> maModel.maName >> maModel.maComment >> maModel.maUser;
}

void Scenario::importInputCells( RecordInputStream& rStrm )
{
    // TODO: where is the deleted flag?
    ScenarioCellModel aModel;
    BinAddress aPos;
    rStrm >> aPos;
    rStrm.skip( 8 );
    aModel.mnNumFmtId = rStrm.readuInt16();
    rStrm >> aModel.maValue;
    getAddressConverter().convertToCellAddressUnchecked( aModel.maPos, aPos, mnSheet );
    maCells.push_back( aModel );
}

void Scenario::importScenario( BiffInputStream& rStrm )
{
    sal_uInt16 nCellCount;
    sal_uInt8 nNameLen, nCommentLen, nUserLen;
    rStrm >> nCellCount;
    // two bytes instead of flag field
    maModel.mbLocked = rStrm.readuInt8() != 0;
    maModel.mbHidden = rStrm.readuInt8() != 0;
    rStrm >> nNameLen >> nCommentLen >> nUserLen;
    maModel.maName = rStrm.readUniStringBody( nNameLen );
    // user name: before comment (in difference to leading length field), repeated length
    if( nUserLen > 0 )
        maModel.maUser = rStrm.readUniString();
    // comment: repeated length
    if( nCommentLen > 0 )
        maModel.maComment = rStrm.readUniString();

    // list of cell addresses
    for( sal_uInt16 nCell = 0; !rStrm.isEof() && (nCell < nCellCount); ++nCell )
    {
        ScenarioCellModel aModel;
        BinAddress aPos;
        rStrm >> aPos;
        // deleted flag is encoded in column index
        aModel.mbDeleted = getFlag( aPos.mnCol, BIFF_SCENARIO_DELETED );
        setFlag( aPos.mnCol, BIFF_SCENARIO_DELETED, false );
        getAddressConverter().convertToCellAddressUnchecked( aModel.maPos, aPos, mnSheet );
        maCells.push_back( aModel );
    }

    // list of cell values
    for( ScenarioCellVector::iterator aIt = maCells.begin(), aEnd = maCells.end(); !rStrm.isEof() && (aIt != aEnd); ++aIt )
        aIt->maValue = rStrm.readUniString();
}

void Scenario::finalizeImport()
{
    AddressConverter& rAddrConv = getAddressConverter();
    ::std::vector< CellRangeAddress > aRanges;
    for( ScenarioCellVector::iterator aIt = maCells.begin(), aEnd = maCells.end(); aIt != aEnd; ++aIt )
        if( !aIt->mbDeleted && rAddrConv.checkCellAddress( aIt->maPos, true ) )
            aRanges.push_back( CellRangeAddress( aIt->maPos.Sheet, aIt->maPos.Column, aIt->maPos.Row, aIt->maPos.Column, aIt->maPos.Row ) );

    if( !aRanges.empty() && (maModel.maName.getLength() > 0) ) try
    {
        /*  Find an unused name for the scenario (Calc stores scenario data in
            hidden sheets named after the scenario following the base sheet). */
        Reference< XNameAccess > xSheetsNA( getDocument()->getSheets(), UNO_QUERY_THROW );
        OUString aScenName = ContainerHelper::getUnusedName( xSheetsNA, maModel.maName, '_' );

        // create the new scenario sheet
        Reference< XScenariosSupplier > xScenariosSupp( getSheetFromDoc( mnSheet ), UNO_QUERY_THROW );
        Reference< XScenarios > xScenarios( xScenariosSupp->getScenarios(), UNO_SET_THROW );
        xScenarios->addNewByName( aScenName, ContainerHelper::vectorToSequence( aRanges ), maModel.maComment );

        // write scenario cell values
        Reference< XSpreadsheet > xSheet( getSheetFromDoc( aScenName ), UNO_SET_THROW );
        for( ScenarioCellVector::iterator aIt = maCells.begin(), aEnd = maCells.end(); aIt != aEnd; ++aIt )
        {
            if( !aIt->mbDeleted ) try
            {
                // use XCell::setFormula to auto-detect values and strings
                Reference< XCell > xCell( xSheet->getCellByPosition( aIt->maPos.Column, aIt->maPos.Row ), UNO_SET_THROW );
                xCell->setFormula( aIt->maValue );
            }
            catch( Exception& )
            {
            }
        }

        // scenario properties
        PropertySet aPropSet( xScenarios->getByName( aScenName ) );
        aPropSet.setProperty( PROP_IsActive, false );
        aPropSet.setProperty( PROP_CopyBack, false );
        aPropSet.setProperty( PROP_CopyStyles, false );
        aPropSet.setProperty( PROP_CopyFormulas, false );
        aPropSet.setProperty( PROP_Protected, maModel.mbLocked );
        // #112621# do not show/print scenario border
        aPropSet.setProperty( PROP_ShowBorder, false );
        aPropSet.setProperty( PROP_PrintBorder, false );
    }
    catch( Exception& )
    {
    }
}

// ============================================================================

SheetScenariosModel::SheetScenariosModel() :
    mnCurrent( 0 ),
    mnShown( 0 )
{
}

// ----------------------------------------------------------------------------

SheetScenarios::SheetScenarios( const WorkbookHelper& rHelper, sal_Int16 nSheet ) :
    WorkbookHelper( rHelper ),
    mnSheet( nSheet )
{
}

void SheetScenarios::importScenarios( const AttributeList& rAttribs )
{
    maModel.mnCurrent = rAttribs.getInteger( XML_current, 0 );
    maModel.mnShown   = rAttribs.getInteger( XML_show, 0 );
}

void SheetScenarios::importScenarios( RecordInputStream& rStrm )
{
    maModel.mnCurrent = rStrm.readuInt16();
    maModel.mnShown   = rStrm.readuInt16();
}

void SheetScenarios::importScenarios( BiffInputStream& rStrm )
{
    rStrm.skip( 2 );    // scenario count
    maModel.mnCurrent = rStrm.readuInt16();
    maModel.mnShown   = rStrm.readuInt16();

    // read following SCENARIO records
    while( (rStrm.getNextRecId() == BIFF_ID_SCENARIO) && rStrm.startNextRecord() )
        createScenario().importScenario( rStrm );
}

Scenario& SheetScenarios::createScenario()
{
    ScenarioVector::value_type xScenario( new Scenario( *this, mnSheet ) );
    maScenarios.push_back( xScenario );
    return *xScenario;
}

void SheetScenarios::finalizeImport()
{
    maScenarios.forEachMem( &Scenario::finalizeImport );

    // activate a scenario
    try
    {
        Reference< XScenariosSupplier > xScenariosSupp( getSheetFromDoc( mnSheet ), UNO_QUERY_THROW );
        Reference< XIndexAccess > xScenariosIA( xScenariosSupp->getScenarios(), UNO_QUERY_THROW );
        Reference< XScenario > xScenario( xScenariosIA->getByIndex( maModel.mnShown ), UNO_QUERY_THROW );
        xScenario->apply();
    }
    catch( Exception& )
    {
    }
}

// ============================================================================

ScenarioBuffer::ScenarioBuffer( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper )
{
}

SheetScenarios& ScenarioBuffer::createSheetScenarios( sal_Int16 nSheet )
{
    SheetScenariosMap::mapped_type& rxSheetScens = maSheetScenarios[ nSheet ];
    if( !rxSheetScens )
        rxSheetScens.reset( new SheetScenarios( *this, nSheet ) );
    return *rxSheetScens;
}

void ScenarioBuffer::finalizeImport()
{
    maSheetScenarios.forEachMem( &SheetScenarios::finalizeImport );
}

// ============================================================================

} // namespace xls
} // namespace oox

