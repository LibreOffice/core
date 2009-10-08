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
#include "oox/helper/containerhelper.hxx"
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

namespace {

/** Returns the base file name without path and extension. */
OUString lclGetBaseFileName( const OUString& rUrl )
{
    sal_Int32 nFileNamePos = ::std::max< sal_Int32 >( rUrl.lastIndexOf( '/' ) + 1, 0 );
    sal_Int32 nExtPos = rUrl.lastIndexOf( '.' );
    if( nExtPos <= nFileNamePos ) nExtPos = rUrl.getLength();
    return rUrl.copy( nFileNamePos, nExtPos - nFileNamePos );
}

} // namespace

// ============================================================================

SheetInfoModel::SheetInfoModel() :
    mnSheetId( -1 ),
    mnState( XML_visible )
{
}

// ============================================================================

WorksheetBuffer::WorksheetBuffer( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper )
{
}

void WorksheetBuffer::initializeSingleSheet()
{
    OSL_ENSURE( maSheetInfos.empty(), "WorksheetBuffer::initializeSingleSheet - invalid call" );
    SheetInfoModel aModel;
    aModel.maName = lclGetBaseFileName( getBaseFilter().getFileUrl() );
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
    sal_uInt16 nState = 0;
    if( getBiff() >= BIFF5 )
    {
        rStrm.skip( 4 );
        rStrm >> nState;
    }

    SheetInfoModel aModel;
    aModel.maName = (getBiff() == BIFF8) ?
        rStrm.readUniStringBody( rStrm.readuInt8() ) :
        rStrm.readByteStringUC( false, getTextEncoding() );
    static const sal_Int32 spnStates[] = { XML_visible, XML_hidden, XML_veryHidden };
    aModel.mnState = STATIC_ARRAY_SELECT( spnStates, nState, XML_visible );
    insertSheet( aModel );
}

sal_Int16 WorksheetBuffer::insertEmptySheet( const OUString& rPreferredName, bool bVisible )
{
    IndexNamePair aIndexName = insertSheet( rPreferredName, SAL_MAX_INT16, bVisible );
    return aIndexName.first;
}

sal_Int32 WorksheetBuffer::getSheetCount() const
{
    return static_cast< sal_Int32 >( maSheetInfos.size() );
}

OUString WorksheetBuffer::getSheetRelId( sal_Int32 nSheet ) const
{
    OUString aRelId;
    if( const SheetInfoModel* pModel = getSheetInfo( nSheet ) )
        aRelId = pModel->maRelId;
    return aRelId;
}

OUString WorksheetBuffer::getCalcSheetName( sal_Int32 nSheet ) const
{
    OUString aName;
    if( const SheetInfoModel* pModel = getSheetInfo( nSheet ) )
        aName = pModel->maFinalName;
    return aName;
}

OUString WorksheetBuffer::getCalcSheetName( const OUString& rModelName ) const
{
    for( SheetInfoModelVec::const_iterator aIt = maSheetInfos.begin(), aEnd = maSheetInfos.end(); aIt != aEnd; ++aIt )
        // TODO: handle encoded characters
        if( aIt->maName.equalsIgnoreAsciiCase( rModelName ) )
            return aIt->maFinalName;
    return OUString();
}

sal_Int32 WorksheetBuffer::getCalcSheetIndex( const OUString& rModelName ) const
{
    for( SheetInfoModelVec::const_iterator aIt = maSheetInfos.begin(), aEnd = maSheetInfos.end(); aIt != aEnd; ++aIt )
        // TODO: handle encoded characters
        if( aIt->maName.equalsIgnoreAsciiCase( rModelName ) )
            return static_cast< sal_Int32 >( aIt - maSheetInfos.begin() );
    return -1;
}

// private --------------------------------------------------------------------

const SheetInfoModel* WorksheetBuffer::getSheetInfo( sal_Int32 nSheet ) const
{
    return ContainerHelper::getVectorElement( maSheetInfos, nSheet );
}

WorksheetBuffer::IndexNamePair WorksheetBuffer::insertSheet( const OUString& rPreferredName, sal_Int16 nSheet, bool bVisible )
{
    IndexNamePair aIndexName;
    aIndexName.first = -1;
    aIndexName.second = (rPreferredName.getLength() == 0) ? CREATE_OUSTRING( "Sheet" ) : rPreferredName;
    try
    {
        Reference< XSpreadsheets > xSheets( getDocument()->getSheets(), UNO_QUERY_THROW );
        Reference< XIndexAccess > xSheetsIA( xSheets, UNO_QUERY_THROW );
        Reference< XNameAccess > xSheetsNA( xSheets, UNO_QUERY_THROW );
        PropertySet aPropSet;
        if( nSheet < xSheetsIA->getCount() )
        {
            // existing sheet - try to rename
            Reference< XNamed > xSheetName( xSheetsIA->getByIndex( nSheet ), UNO_QUERY_THROW );
            if( xSheetName->getName() != aIndexName.second )
            {
                aIndexName.second = ContainerHelper::getUnusedName( xSheetsNA, aIndexName.second, ' ' );
                xSheetName->setName( aIndexName.second );
            }
            aPropSet.set( xSheetName );
        }
        else
        {
            // new sheet - insert with unused name
            aIndexName.second = ContainerHelper::getUnusedName( xSheetsNA, aIndexName.second, ' ' );
            nSheet = static_cast< sal_Int16 >( xSheetsIA->getCount() );
            xSheets->insertNewByName( aIndexName.second, nSheet );
            aPropSet.set( xSheetsIA->getByIndex( nSheet ) );
        }

        // sheet properties
        aPropSet.setProperty( PROP_IsVisible, bVisible );

        // return final sheet index if sheet exists
        aIndexName.first = nSheet;
    }
    catch( Exception& )
    {
        OSL_ENSURE( false, "WorksheetBuffer::insertSheet - cannot insert or rename worksheet" );
    }
    return aIndexName;
}

void WorksheetBuffer::insertSheet( const SheetInfoModel& rModel )
{
    sal_Int16 nSheet = static_cast< sal_Int16 >( maSheetInfos.size() );
    maSheetInfos.push_back( rModel );
    IndexNamePair aIndexName = insertSheet( rModel.maName, nSheet, rModel.mnState == XML_visible );
    if( aIndexName.first >= 0 )
        maSheetInfos.back().maFinalName = aIndexName.second;
}

// ============================================================================

} // namespace xls
} // namespace oox

