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

#include <worksheetbuffer.hxx>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/helper/binaryinputstream.hxx>
#include <oox/helper/containerhelper.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include <document.hxx>
#include <documentimport.hxx>
#include <biffhelper.hxx>
#include <globstr.hrc>
#include <scresid.hxx>

namespace oox::xls {

using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::uno;

SheetInfoModel::SheetInfoModel() :
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
    nState = rStrm.readInt32();
    aModel.mnSheetId = rStrm.readInt32();
    rStrm >> aModel.maRelId >> aModel.maName;
    static const sal_Int32 spnStates[] = { XML_visible, XML_hidden, XML_veryHidden };
    aModel.mnState = STATIC_ARRAY_SELECT( spnStates, nState, XML_visible );
    insertSheet( aModel );
}

sal_Int16 WorksheetBuffer::insertEmptySheet( const OUString& rPreferredName )
{
    IndexNamePair aIndexName = createSheet( rPreferredName, SAL_MAX_INT32 );
    ScDocument& rDoc = getScDocument();

    rDoc.SetVisible( aIndexName.first, false );
    return aIndexName.first;
}

sal_Int32 WorksheetBuffer::getWorksheetCount() const
{
    return static_cast< sal_Int32 >( maSheetInfos.size() );
}

sal_Int32 WorksheetBuffer::getAllSheetCount() const
{
    const ScDocumentImport& rDoc = getDocImport();
    return rDoc.getSheetCount();
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
    if( !sSheetNameRef.startsWith("#") )
        return;

    sal_Int32 nSepPos = sSheetNameRef.lastIndexOf( '!' );
    if( nSepPos <= 0 )
        return;

    // Do not attempt to blindly convert '#SheetName!A1' to
    // '#SheetName.A1', it can be #SheetName!R1C1 as well. Hyperlink
    // handler has to handle all, but prefer '#SheetName.A1' if
    // possible.
    if (nSepPos < sSheetNameRef.getLength() - 1)
    {
        ScRange aRange;
        if ((aRange.ParseAny( sSheetNameRef.copy( nSepPos + 1 ), getScDocument(),
                        formula::FormulaGrammar::CONV_XL_R1C1) & ScRefFlags::VALID) == ScRefFlags::ZERO)
            sSheetNameRef = sSheetNameRef.replaceAt( nSepPos, 1, OUString( '.' ) );
    }
    // #i66592# convert sheet names that have been renamed on import
    OUString aSheetName = sSheetNameRef.copy( 1, nSepPos - 1 );
    OUString aCalcName = getCalcSheetName( aSheetName );
    if( !aCalcName.isEmpty() )
        sSheetNameRef = sSheetNameRef.replaceAt( 1, nSepPos - 1, aCalcName );
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

WorksheetBuffer::IndexNamePair WorksheetBuffer::createSheet( const OUString& rPreferredName, sal_Int32 nSheetPos )
{
    //FIXME: Rewrite this block using ScDocument[Import] instead of UNO
    try
    {
        Reference< XSpreadsheets > xSheets( getDocument()->getSheets(), UNO_SET_THROW );
        Reference< XIndexAccess > xSheetsIA( xSheets, UNO_QUERY_THROW );
        sal_Int16 nCalcSheet = -1;
        OUString aSheetName = rPreferredName.isEmpty() ? ScResId(STR_TABLE_DEF) : rPreferredName;
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
        }
        else
        {
            nCalcSheet = static_cast< sal_Int16 >( xSheetsIA->getCount() );
            // new sheet - insert with unused name
            aSheetName = ContainerHelper::getUnusedName( xSheets, aSheetName, ' ' );
            xSheets->insertNewByName( aSheetName, nCalcSheet );
        }

        // return final sheet index if sheet exists
        return IndexNamePair( nCalcSheet, aSheetName );
    }
    catch (const Exception&)
    {
        OSL_FAIL( "WorksheetBuffer::createSheet - cannot insert or rename worksheet" );
    }
    return IndexNamePair( -1, OUString() );
}

void WorksheetBuffer::insertSheet( const SheetInfoModel& rModel )
{
    sal_Int32 nWorksheet = static_cast< sal_Int32 >( maSheetInfos.size() );
    IndexNamePair aIndexName = createSheet( rModel.maName, nWorksheet );
    auto xSheetInfo = std::make_shared<SheetInfo>( rModel, aIndexName.first, aIndexName.second );
    maSheetInfos.push_back( xSheetInfo );
    maSheetInfosByName[ rModel.maName ] = xSheetInfo;
    maSheetInfosByName[ lclQuoteName( rModel.maName ) ] = xSheetInfo;
}

void WorksheetBuffer::finalizeImport( sal_Int16 nActiveSheet )
{
    ScDocument& rDoc = getScDocument();

    for ( const auto& aSheetInfo: maSheetInfos )
    {
        // make sure at least 1 sheet (the active one) is visible
        if ( aSheetInfo->mnCalcSheet == nActiveSheet)
            rDoc.SetVisible( aSheetInfo->mnCalcSheet, true );
        else
            rDoc.SetVisible( aSheetInfo->mnCalcSheet, aSheetInfo->mnState == XML_visible );
    }
}

} // namespace oox::xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
