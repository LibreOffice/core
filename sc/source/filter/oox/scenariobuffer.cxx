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

#include <scenariobuffer.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XScenarios.hpp>
#include <com/sun/star/sheet/XScenariosSupplier.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <oox/helper/binaryinputstream.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/helper/containerhelper.hxx>
#include <oox/helper/propertyset.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/tokens.hxx>
#include <addressconverter.hxx>
#include <biffhelper.hxx>

namespace oox {
namespace xls {

using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::uno;

ScenarioCellModel::ScenarioCellModel() :
    mnNumFmtId( 0 ),
    mbDeleted( false )
{
}

ScenarioModel::ScenarioModel() :
    mbLocked( false ),
    mbHidden( false ),
    mbActive( false )
{
}

Scenario::Scenario( const WorkbookHelper& rHelper, sal_Int16 nSheet, bool bIsActive ) :
    WorkbookHelper( rHelper ),
    mnSheet( nSheet )
{
    maModel.mbActive = bIsActive;
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
    AddressConverter::convertToCellAddressUnchecked( aModel.maPos, rAttribs.getString( XML_r, OUString() ), mnSheet );
    aModel.maValue    = rAttribs.getXString( XML_val, OUString() );
    aModel.mnNumFmtId = rAttribs.getInteger( XML_numFmtId, 0 );
    aModel.mbDeleted  = rAttribs.getBool( XML_deleted, false );
    maCells.push_back( aModel );
}

void Scenario::importScenario( SequenceInputStream& rStrm )
{
    rStrm.skip( 2 );    // cell count
    // two longs instead of flag field
    maModel.mbLocked = rStrm.readInt32() != 0;
    maModel.mbHidden = rStrm.readInt32() != 0;
    rStrm >> maModel.maName >> maModel.maComment >> maModel.maUser;
}

void Scenario::importInputCells( SequenceInputStream& rStrm )
{
    // TODO: where is the deleted flag?
    ScenarioCellModel aModel;
    BinAddress aPos;
    rStrm >> aPos;
    rStrm.skip( 8 );
    aModel.mnNumFmtId = rStrm.readuInt16();
    rStrm >> aModel.maValue;
    AddressConverter::convertToCellAddressUnchecked( aModel.maPos, aPos, mnSheet );
    maCells.push_back( aModel );
}

void Scenario::finalizeImport()
{
    AddressConverter& rAddrConv = getAddressConverter();
    ScRangeList aRanges;
    for( const auto& rCell : maCells )
        if( !rCell.mbDeleted && rAddrConv.checkCellAddress( rCell.maPos, true ) )
            aRanges.push_back( ScRange(rCell.maPos, rCell.maPos) );

    if( !aRanges.empty() && !maModel.maName.isEmpty() ) try
    {
        /*  Find an unused name for the scenario (Calc stores scenario data in
            hidden sheets named after the scenario following the base sheet). */
        Reference< XNameAccess > xSheetsNA( getDocument()->getSheets(), UNO_QUERY_THROW );
        OUString aScenName = ContainerHelper::getUnusedName( xSheetsNA, maModel.maName, '_' );

        // create the new scenario sheet
        Reference< XScenariosSupplier > xScenariosSupp( getSheetFromDoc( mnSheet ), UNO_QUERY_THROW );
        Reference< XScenarios > xScenarios( xScenariosSupp->getScenarios(), UNO_SET_THROW );
        xScenarios->addNewByName( aScenName, AddressConverter::toApiSequence(aRanges), maModel.maComment );

        // write scenario cell values
        Reference< XSpreadsheet > xSheet( getSheetFromDoc( aScenName ), UNO_SET_THROW );
        for( const auto& rCell : maCells )
        {
            if( !rCell.mbDeleted ) try
            {
                // use XCell::setFormula to auto-detect values and strings
                Reference< XCell > xCell( xSheet->getCellByPosition( rCell.maPos.Col(), rCell.maPos.Row() ), UNO_SET_THROW );
                xCell->setFormula( rCell.maValue );
            }
            catch( Exception& )
            {
            }
        }

        // scenario properties
        PropertySet aPropSet( xScenarios->getByName( aScenName ) );
        aPropSet.setProperty( PROP_IsActive, maModel.mbActive );
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

SheetScenariosModel::SheetScenariosModel() :
    mnCurrent( 0 ),
    mnShown( 0 )
{
}

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

void SheetScenarios::importScenarios( SequenceInputStream& rStrm )
{
    maModel.mnCurrent = rStrm.readuInt16();
    maModel.mnShown   = rStrm.readuInt16();
}

Scenario& SheetScenarios::createScenario()
{
    bool bIsActive = maScenarios.size() == static_cast<sal_uInt32>(maModel.mnShown);
    ScenarioVector::value_type xScenario( new Scenario( *this, mnSheet, bIsActive ) );
    maScenarios.push_back( xScenario );
    return *xScenario;
}

void SheetScenarios::finalizeImport()
{
    maScenarios.forEachMem( &Scenario::finalizeImport );
}

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

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
