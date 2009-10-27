/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: worksheetbuffer.cxx,v $
 * $Revision: 1.6.2.1 $
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

#include "oox/xls/worksheetbuffer.hxx"
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XExternalSheetName.hpp>
#include <com/sun/star/sheet/XSheetLinkable.hpp>
#include "properties.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/helper/recordinputstream.hxx"
#include "oox/core/filterbase.hxx"
#include "oox/xls/biffinputstream.hxx"
#include "oox/xls/excelhandlers.hxx"

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::container::XIndexAccess;
using ::com::sun::star::container::XNameAccess;
using ::com::sun::star::container::XNamed;
using ::com::sun::star::sheet::XSpreadsheetDocument;
using ::com::sun::star::sheet::XSpreadsheets;

namespace oox {
namespace xls {

// ============================================================================

SheetInfoModel::SheetInfoModel() :
    mnBiffHandle( -1 ),
    mnSheetId( -1 ),
    mnState( XML_visible )
{
}

// ============================================================================

WorksheetBuffer::WorksheetBuffer( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper )
{
}

/*static*/ OUString WorksheetBuffer::getBaseFileName( const OUString& rUrl )
{
    sal_Int32 nFileNamePos = ::std::max< sal_Int32 >( rUrl.lastIndexOf( '/' ) + 1, 0 );
    sal_Int32 nExtPos = rUrl.lastIndexOf( '.' );
    if( nExtPos <= nFileNamePos ) nExtPos = rUrl.getLength();
    return rUrl.copy( nFileNamePos, nExtPos - nFileNamePos );
}

void WorksheetBuffer::initializeSingleSheet()
{
    OSL_ENSURE( maSheetInfos.empty(), "WorksheetBuffer::initializeSingleSheet - invalid call" );
    SheetInfoModel aModel;
    aModel.maName = getBaseFileName( getBaseFilter().getFileUrl() );
    insertSheet( aModel );
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

void WorksheetBuffer::importSheet( RecordInputStream& rStrm )
{
    sal_Int32 nState;
    SheetInfoModel aModel;
    rStrm >> nState >> aModel.mnSheetId >> aModel.maRelId >> aModel.maName;
    static const sal_Int32 spnStates[] = { XML_visible, XML_hidden, XML_veryHidden };
    aModel.mnState = STATIC_ARRAY_SELECT( spnStates, nState, XML_visible );
    insertSheet( aModel );
}

void WorksheetBuffer::importSheet( BiffInputStream& rStrm )
{
    SheetInfoModel aModel;
    if( getBiff() >= BIFF5 )
    {
        rStrm.enableDecoder( false );
        aModel.mnBiffHandle = rStrm.readuInt32();
        rStrm.enableDecoder( true );
        sal_uInt16 nState = rStrm.readuInt16();
        static const sal_Int32 spnStates[] = { XML_visible, XML_hidden, XML_veryHidden };
        aModel.mnState = STATIC_ARRAY_SELECT( spnStates, nState, XML_visible );
    }
    aModel.maName = (getBiff() == BIFF8) ?
        rStrm.readUniStringBody( rStrm.readuInt8() ) :
        rStrm.readByteStringUC( false, getTextEncoding() );
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

sal_Int64 WorksheetBuffer::getBiffRecordHandle( sal_Int32 nWorksheet ) const
{
    const SheetInfo* pSheetInfo = maSheetInfos.get( nWorksheet ).get();
    return pSheetInfo ? pSheetInfo->mnBiffHandle : -1;
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
        if( aBuffer.charAt( nPos ) == '\'' )
            aBuffer.insert( nPos, sal_Unicode( '\'' ) );
    // add outer quotes and return
    return aBuffer.insert( 0, sal_Unicode( '\'' ) ).append( sal_Unicode( '\'' ) ).makeStringAndClear();
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
        Reference< XNameAccess > xSheetsNA( xSheets, UNO_QUERY_THROW );
        sal_Int16 nCalcSheet = -1;
        OUString aSheetName = (rPreferredName.getLength() == 0) ? CREATE_OUSTRING( "Sheet" ) : rPreferredName;
        PropertySet aPropSet;
        if( nSheetPos < xSheetsIA->getCount() )
        {
            nCalcSheet = static_cast< sal_Int16 >( nSheetPos );
            // existing sheet - try to rename
            Reference< XNamed > xSheetName( xSheetsIA->getByIndex( nSheetPos ), UNO_QUERY_THROW );
            if( xSheetName->getName() != aSheetName )
            {
                aSheetName = ContainerHelper::getUnusedName( xSheetsNA, aSheetName, ' ' );
                xSheetName->setName( aSheetName );
            }
            aPropSet.set( xSheetName );
        }
        else
        {
            nCalcSheet = static_cast< sal_Int16 >( xSheetsIA->getCount() );
            // new sheet - insert with unused name
            aSheetName = ContainerHelper::getUnusedName( xSheetsNA, aSheetName, ' ' );
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
        OSL_ENSURE( false, "WorksheetBuffer::createSheet - cannot insert or rename worksheet" );
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

// ============================================================================

} // namespace xls
} // namespace oox

