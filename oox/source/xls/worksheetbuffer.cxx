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

OoxSheetInfo::OoxSheetInfo() :
    mnSheetId( -1 ),
    mnState( XML_visible )
{
}

// ============================================================================

WorksheetBuffer::WorksheetBuffer( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    maIsVisibleProp( CREATE_OUSTRING( "IsVisible" ) )
{
}

void WorksheetBuffer::initializeSingleSheet()
{
    OSL_ENSURE( maSheetInfos.empty(), "WorksheetBuffer::initializeSingleSheet - invalid call" );
    OoxSheetInfo aSheetInfo;
    aSheetInfo.maName = lclGetBaseFileName( getBaseFilter().getFileUrl() );
    insertSheet( aSheetInfo );
}

void WorksheetBuffer::importSheet( const AttributeList& rAttribs )
{
    OoxSheetInfo aSheetInfo;
    aSheetInfo.maRelId = rAttribs.getString( R_TOKEN( id ), OUString() );
    aSheetInfo.maName = rAttribs.getString( XML_name, OUString() );
    aSheetInfo.mnSheetId = rAttribs.getInteger( XML_sheetId, -1 );
    aSheetInfo.mnState = rAttribs.getToken( XML_state, XML_visible );
    insertSheet( aSheetInfo );
}

void WorksheetBuffer::importSheet( RecordInputStream& rStrm )
{
    sal_Int32 nState;
    OoxSheetInfo aSheetInfo;
    rStrm >> nState >> aSheetInfo.mnSheetId >> aSheetInfo.maRelId >> aSheetInfo.maName;
    static const sal_Int32 spnStates[] = { XML_visible, XML_hidden, XML_veryHidden };
    aSheetInfo.mnState = STATIC_ARRAY_SELECT( spnStates, nState, XML_visible );
    insertSheet( aSheetInfo );
}

void WorksheetBuffer::importSheet( BiffInputStream& rStrm )
{
    sal_uInt16 nState = 0;
    if( getBiff() >= BIFF5 )
    {
        rStrm.skip( 4 );
        rStrm >> nState;
    }

    OoxSheetInfo aSheetInfo;
    aSheetInfo.maName = (getBiff() == BIFF8) ?
        rStrm.readUniString( rStrm.readuInt8() ) :
        rStrm.readByteString( false, getTextEncoding() );
    static const sal_Int32 spnStates[] = { XML_visible, XML_hidden, XML_veryHidden };
    aSheetInfo.mnState = STATIC_ARRAY_SELECT( spnStates, nState, XML_visible );
    insertSheet( aSheetInfo );
}

sal_Int32 WorksheetBuffer::getSheetCount() const
{
    return static_cast< sal_Int32 >( maSheetInfos.size() );
}

OUString WorksheetBuffer::getSheetRelId( sal_Int32 nSheet ) const
{
    OUString aRelId;
    if( const OoxSheetInfo* pInfo = getSheetInfo( nSheet ) )
        aRelId = pInfo->maRelId;
    return aRelId;
}

OUString WorksheetBuffer::getFinalSheetName( sal_Int32 nSheet ) const
{
    OUString aName;
    if( const OoxSheetInfo* pInfo = getSheetInfo( nSheet ) )
        aName = pInfo->maFinalName;
    return aName;
}

OUString WorksheetBuffer::getFinalSheetName( const OUString& rName ) const
{
    for( SheetInfoVec::const_iterator aIt = maSheetInfos.begin(), aEnd = maSheetInfos.end(); aIt != aEnd; ++aIt )
        // TODO: handle encoded characters
        if( aIt->maName.equalsIgnoreAsciiCase( rName ) )
            return aIt->maFinalName;
    return OUString();
}

sal_Int32 WorksheetBuffer::getFinalSheetIndex( const OUString& rName ) const
{
    for( SheetInfoVec::const_iterator aIt = maSheetInfos.begin(), aEnd = maSheetInfos.end(); aIt != aEnd; ++aIt )
        // TODO: handle encoded characters
        if( aIt->maName.equalsIgnoreAsciiCase( rName ) )
            return static_cast< sal_Int32 >( aIt - maSheetInfos.begin() );
    return -1;
}

// private --------------------------------------------------------------------

const OoxSheetInfo* WorksheetBuffer::getSheetInfo( sal_Int32 nSheet ) const
{
    return ((0 <= nSheet) && (static_cast< size_t >( nSheet ) < maSheetInfos.size())) ?
        &maSheetInfos[ static_cast< size_t >( nSheet ) ] : 0;
}

OUString WorksheetBuffer::insertSheet( const OUString& rName, sal_Int16 nSheet, bool bVisible )
{
    OUString aFinalName = (rName.getLength() == 0) ? CREATE_OUSTRING( "Sheet" ) : rName;
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
            if( xSheetName->getName() != aFinalName )
            {
                aFinalName = ContainerHelper::getUnusedName( xSheetsNA, aFinalName, ' ' );
                xSheetName->setName( aFinalName );
            }
            aPropSet.set( xSheetName );
        }
        else
        {
            // new sheet - insert with unused name
            aFinalName = ContainerHelper::getUnusedName( xSheetsNA, aFinalName, ' ' );
            xSheets->insertNewByName( aFinalName, nSheet );
            aPropSet.set( xSheetsIA->getByIndex( nSheet ) );
        }

        // sheet properties
        aPropSet.setProperty( maIsVisibleProp, bVisible );
    }
    catch( Exception& )
    {
        OSL_ENSURE( false, "WorksheetBuffer::insertSheet - cannot insert or rename worksheet" );
    }
    return aFinalName;
}

void WorksheetBuffer::insertSheet( const OoxSheetInfo& rSheetInfo )
{
    sal_Int16 nSheet = static_cast< sal_Int16 >( maSheetInfos.size() );
    maSheetInfos.push_back( rSheetInfo );
    maSheetInfos.back().maFinalName = insertSheet( rSheetInfo.maName, nSheet, rSheetInfo.mnState == XML_visible );
}

// ============================================================================

} // namespace xls
} // namespace oox

