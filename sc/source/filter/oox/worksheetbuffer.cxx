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

#include "worksheetbuffer.hxx"

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/sheet/XExternalSheetName.hpp>
#include <com/sun/star/sheet/XSheetLinkable.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <rtl/ustrbuf.hxx>
#include "oox/core/filterbase.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/helper/containerhelper.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/token/properties.hxx"
#include "biffinputstream.hxx"
#include "excelhandlers.hxx"

namespace oox {
namespace xls {



using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::uno;




SheetInfoModel::SheetInfoModel() :
    mnBiffHandle( -1 ),
    mnSheetId( -1 ),
    mnState( XML_visible )
{
}



WorksheetBuffer::WorksheetBuffer( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper )
{
}

void WorksheetBuffer::importSheet( const AttributeList& rAttribs )
{
    SheetInfoModel aModel;
    aModel.maRelId = rAttribs.getString( R_TOKEN( id ), OUString() );
    aModel.maName = rAttribs.getXString( XML_name, OUString() );
    aModel.mnSheetId = rAttribs.getInteger( XML_sheetId, -1 );
    aModel.mnState = rAttribs.getToken( XML_state, XML_visible );
    insertSheet( aModel );
}

void WorksheetBuffer::importSheet( SequenceInputStream& rStrm )
{
    sal_Int32 nState;
    SheetInfoModel aModel;
    rStrm >> nState >> aModel.mnSheetId >> aModel.maRelId >> aModel.maName;
    static const sal_Int32 spnStates[] = { XML_visible, XML_hidden, XML_veryHidden };
    aModel.mnState = STATIC_ARRAY_SELECT( spnStates, nState, XML_visible );
    insertSheet( aModel );
}

sal_Int16 WorksheetBuffer::insertEmptySheet( const OUString& rPreferredName, bool bVisible )
{
    return createSheet( rPreferredName, SAL_MAX_INT32, bVisible ).first;
}

sal_Int32 WorksheetBuffer::getWorksheetCount() const
{
    return static_cast< sal_Int32 >( maSheetInfos.size() );
}

OUString WorksheetBuffer::getWorksheetRelId( sal_Int32 nWorksheet ) const
{
    const SheetInfo* pSheetInfo = maSheetInfos.get( nWorksheet ).get();
    return pSheetInfo ? pSheetInfo->maRelId : OUString();
}

sal_Int16 WorksheetBuffer::getCalcSheetIndex( sal_Int32 nWorksheet ) const
{
    const SheetInfo* pSheetInfo = maSheetInfos.get( nWorksheet ).get();
    return pSheetInfo ? pSheetInfo->mnCalcSheet : -1;
}

OUString WorksheetBuffer::getCalcSheetName( sal_Int32 nWorksheet ) const
{
    const SheetInfo* pSheetInfo = maSheetInfos.get( nWorksheet ).get();
    return pSheetInfo ? pSheetInfo->maCalcName : OUString();
}

void WorksheetBuffer::convertSheetNameRef( OUString& sSheetNameRef ) const
{
    // convert '#SheetName!A1' to '#SheetName.A1'
    if( sSheetNameRef.startsWith("#") )
    {
        sal_Int32 nSepPos = sSheetNameRef.lastIndexOf( '!' );
        if( nSepPos > 0 )
        {
            // replace the exclamation mark with a period
            sSheetNameRef = sSheetNameRef.replaceAt( nSepPos, 1, OUString( '.' ) );
            // #i66592# convert sheet names that have been renamed on import
            OUString aSheetName = sSheetNameRef.copy( 1, nSepPos - 1 );
            OUString aCalcName = getCalcSheetName( aSheetName );
            if( !aCalcName.isEmpty() )
                sSheetNameRef = sSheetNameRef.replaceAt( 1, nSepPos - 1, aCalcName );
        }
    }
}

sal_Int16 WorksheetBuffer::getCalcSheetIndex( const OUString& rWorksheetName ) const
{
    const SheetInfo* pSheetInfo = maSheetInfosByName.get( rWorksheetName ).get();
    return pSheetInfo ? pSheetInfo->mnCalcSheet : -1;
}

OUString WorksheetBuffer::getCalcSheetName( const OUString& rWorksheetName ) const
{
    if( const SheetInfo* pSheetInfo = maSheetInfosByName.get( rWorksheetName ).get() )
    {
        bool bIsQuoted = pSheetInfo->maName != rWorksheetName;
        return bIsQuoted ? pSheetInfo->maCalcQuotedName : pSheetInfo->maCalcName;
    }
    return OUString();
}

// private --------------------------------------------------------------------

namespace {

OUString lclQuoteName( const OUString& rName )
{
    OUStringBuffer aBuffer( rName );
    // duplicate all quote characters
    for( sal_Int32 nPos = aBuffer.getLength() - 1; nPos >= 0; --nPos )
        if( aBuffer[nPos] == '\'' )
            aBuffer.insert( nPos, '\'' );
    // add outer quotes and return
    return aBuffer.insert( 0, '\'' ).append( '\'' ).makeStringAndClear();
}

} // namespace

WorksheetBuffer::SheetInfo::SheetInfo( const SheetInfoModel& rModel, sal_Int16 nCalcSheet, const OUString& rCalcName ) :
    SheetInfoModel( rModel ),
    maCalcName( rCalcName ),
    maCalcQuotedName( lclQuoteName( rCalcName ) ),
    mnCalcSheet( nCalcSheet )
{
}

WorksheetBuffer::IndexNamePair WorksheetBuffer::createSheet( const OUString& rPreferredName, sal_Int32 nSheetPos, bool bVisible )
{
    try
    {
        Reference< XSpreadsheets > xSheets( getDocument()->getSheets(), UNO_QUERY_THROW );
        Reference< XIndexAccess > xSheetsIA( xSheets, UNO_QUERY_THROW );
        sal_Int16 nCalcSheet = -1;
        OUString aSheetName = rPreferredName.isEmpty() ? "Sheet" : rPreferredName;
        PropertySet aPropSet;
        if( nSheetPos < xSheetsIA->getCount() )
        {
            nCalcSheet = static_cast< sal_Int16 >( nSheetPos );
            // existing sheet - try to rename
            Reference< XNamed > xSheetName( xSheetsIA->getByIndex( nSheetPos ), UNO_QUERY_THROW );
            if( xSheetName->getName() != aSheetName )
            {
                aSheetName = ContainerHelper::getUnusedName( xSheets, aSheetName, ' ' );
                xSheetName->setName( aSheetName );
            }
            aPropSet.set( xSheetName );
        }
        else
        {
            nCalcSheet = static_cast< sal_Int16 >( xSheetsIA->getCount() );
            // new sheet - insert with unused name
            aSheetName = ContainerHelper::getUnusedName( xSheets, aSheetName, ' ' );
            xSheets->insertNewByName( aSheetName, nCalcSheet );
            aPropSet.set( xSheetsIA->getByIndex( nCalcSheet ) );
        }

        // sheet properties
        aPropSet.setProperty( PROP_IsVisible, bVisible );

        // return final sheet index if sheet exists
        return IndexNamePair( nCalcSheet, aSheetName );
    }
    catch( Exception& )
    {
        OSL_FAIL( "WorksheetBuffer::createSheet - cannot insert or rename worksheet" );
    }
    return IndexNamePair( -1, OUString() );
}

void WorksheetBuffer::insertSheet( const SheetInfoModel& rModel )
{
    sal_Int32 nWorksheet = static_cast< sal_Int32 >( maSheetInfos.size() );
    IndexNamePair aIndexName = createSheet( rModel.maName, nWorksheet, rModel.mnState == XML_visible );
    ::boost::shared_ptr< SheetInfo > xSheetInfo( new SheetInfo( rModel, aIndexName.first, aIndexName.second ) );
    maSheetInfos.push_back( xSheetInfo );
    maSheetInfosByName[ rModel.maName ] = xSheetInfo;
    maSheetInfosByName[ lclQuoteName( rModel.maName ) ] = xSheetInfo;
}



} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
