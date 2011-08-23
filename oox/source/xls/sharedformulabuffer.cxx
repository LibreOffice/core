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

#include "oox/xls/sharedformulabuffer.hxx"
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/sheet/XFormulaTokens.hpp>
#include "properties.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/helper/recordinputstream.hxx"
#include "oox/xls/addressconverter.hxx"
#include "oox/xls/biffinputstream.hxx"
#include "oox/xls/formulaparser.hxx"

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::table::CellAddress;
using ::com::sun::star::table::CellRangeAddress;
using ::com::sun::star::sheet::XFormulaTokens;
using ::com::sun::star::sheet::XNamedRange;

namespace oox {
namespace xls {

// ============================================================================

namespace {

bool operator==( const CellAddress& rAddr1, const CellAddress& rAddr2 )
{
    return
        (rAddr1.Sheet == rAddr2.Sheet) &&
        (rAddr1.Column == rAddr2.Column) &&
        (rAddr1.Row == rAddr2.Row);
}

bool lclContains( const CellRangeAddress& rRange, const CellAddress& rAddr )
{
    return
        (rRange.Sheet == rAddr.Sheet) &&
        (rRange.StartColumn <= rAddr.Column) && (rAddr.Column <= rRange.EndColumn) &&
        (rRange.StartRow <= rAddr.Row) && (rAddr.Row <= rRange.EndRow);
}

} // namespace

// ============================================================================

ExtCellFormulaContext::ExtCellFormulaContext( const WorksheetHelper& rHelper,
        const Reference< XFormulaTokens >& rxTokens, const CellAddress& rCellPos ) :
    SimpleFormulaContext( rxTokens, false, false ),
    WorksheetHelper( rHelper )
{
    setBaseAddress( rCellPos );
}

void ExtCellFormulaContext::setSharedFormula( const CellAddress& rBaseAddr )
{
    getSharedFormulas().setSharedFormulaCell( *this, rBaseAddr );
}

// ============================================================================

SharedFormulaBuffer::SharedFormulaBuffer( const WorksheetHelper& rHelper ) :
    WorksheetHelper( rHelper )
{
}

void SharedFormulaBuffer::importSharedFmla( const OUString& rFormula, const OUString& rSharedRange, sal_Int32 nSharedId, const CellAddress& rBaseAddr )
{
    CellRangeAddress aFmlaRange;
    if( getAddressConverter().convertToCellRange( aFmlaRange, rSharedRange, getSheetIndex(), true, true ) )
    {
        // create the defined name representing the shared formula
        OSL_ENSURE( lclContains( aFmlaRange, rBaseAddr ), "SharedFormulaBuffer::importSharedFmla - invalid range for shared formula" );
        BinAddress aMapKey( nSharedId, 0 );
        Reference< XNamedRange > xNamedRange = createDefinedName( aMapKey );
        // convert the formula definition
        Reference< XFormulaTokens > xTokens( xNamedRange, UNO_QUERY );
        if( xTokens.is() )
        {
            SimpleFormulaContext aContext( xTokens, true, false );
            aContext.setBaseAddress( rBaseAddr );
            getFormulaParser().importFormula( aContext, rFormula );
            updateCachedCell( rBaseAddr, aMapKey );
        }
    }
}

void SharedFormulaBuffer::importSharedFmla( RecordInputStream& rStrm, const CellAddress& rBaseAddr )
{
    BinRange aRange;
    rStrm >> aRange;
    CellRangeAddress aFmlaRange;
    if( getAddressConverter().convertToCellRange( aFmlaRange, aRange, getSheetIndex(), true, true ) )
    {
        // create the defined name representing the shared formula
        OSL_ENSURE( lclContains( aFmlaRange, rBaseAddr ), "SharedFormulaBuffer::importSharedFmla - invalid range for shared formula" );
        BinAddress aMapKey( rBaseAddr );
        Reference< XNamedRange > xNamedRange = createDefinedName( aMapKey );
        // load the formula definition
        Reference< XFormulaTokens > xTokens( xNamedRange, UNO_QUERY );
        if( xTokens.is() )
        {
            SimpleFormulaContext aContext( xTokens, true, false );
            aContext.setBaseAddress( rBaseAddr );
            getFormulaParser().importFormula( aContext, rStrm );
            updateCachedCell( rBaseAddr, aMapKey );
        }
    }
}

void SharedFormulaBuffer::importSharedFmla( BiffInputStream& rStrm, const CellAddress& rBaseAddr )
{
    BinRange aRange;
    aRange.read( rStrm, false );        // always 8bit column indexes
    CellRangeAddress aFmlaRange;
    if( getAddressConverter().convertToCellRange( aFmlaRange, aRange, getSheetIndex(), true, true ) )
    {
        // create the defined name representing the shared formula
        OSL_ENSURE( lclContains( aFmlaRange, rBaseAddr ), "SharedFormulaBuffer::importSharedFmla - invalid range for shared formula" );
        BinAddress aMapKey( rBaseAddr );
        Reference< XNamedRange > xNamedRange = createDefinedName( aMapKey );
        // load the formula definition
        Reference< XFormulaTokens > xTokens( xNamedRange, UNO_QUERY );
        if( xTokens.is() )
        {
            rStrm.skip( 2 );    // flags
            SimpleFormulaContext aContext( xTokens, true, false );
            aContext.setBaseAddress( rBaseAddr );
            getFormulaParser().importFormula( aContext, rStrm );
            updateCachedCell( rBaseAddr, aMapKey );
        }
    }
}

void SharedFormulaBuffer::setSharedFormulaCell( ExtCellFormulaContext& rContext, const CellAddress& rBaseAddr )
{
    if( !implSetSharedFormulaCell( rContext, BinAddress( rBaseAddr ) ) )
        if( rContext.getBaseAddress() == rBaseAddr )
            mxLastContext.reset( new ExtCellFormulaContext( rContext ) );
}

void SharedFormulaBuffer::setSharedFormulaCell( ExtCellFormulaContext& rContext, sal_Int32 nSharedId )
{
    implSetSharedFormulaCell( rContext, BinAddress( nSharedId, 0 ) );
}

Reference< XNamedRange > SharedFormulaBuffer::createDefinedName( const BinAddress& rMapKey )
{
    OSL_ENSURE( maIndexMap.count( rMapKey ) == 0, "SharedFormulaBuffer::createDefinedName - shared formula exists already" );
    // create the defined name representing the shared formula
    OUString aName = OUStringBuffer().appendAscii( "__shared_" ).
        append( static_cast< sal_Int32 >( getSheetIndex() + 1 ) ).
        append( sal_Unicode( '_' ) ).append( rMapKey.mnRow ).
        append( sal_Unicode( '_' ) ).append( rMapKey.mnCol ).makeStringAndClear();
    Reference< XNamedRange > xNamedRange = createNamedRangeObject( aName );
    PropertySet aNameProps( xNamedRange );
    aNameProps.setProperty( PROP_IsSharedFormula, true );
    sal_Int32 nTokenIndex = -1;
    if( aNameProps.getProperty( nTokenIndex, PROP_TokenIndex ) && (nTokenIndex >= 0) )
        maIndexMap[ rMapKey ] = nTokenIndex;
    return xNamedRange;
}

bool SharedFormulaBuffer::implSetSharedFormulaCell( ExtCellFormulaContext& rContext, const BinAddress& rMapKey )
{
    TokenIndexMap::const_iterator aIt = maIndexMap.find( rMapKey );
    sal_Int32 nTokenIndex = (aIt == maIndexMap.end()) ? -1 : aIt->second;
    if( nTokenIndex >= 0 )
    {
        getFormulaParser().convertNameToFormula( rContext, nTokenIndex );
        return true;
    }
    return false;
}

void SharedFormulaBuffer::updateCachedCell( const CellAddress& rBaseAddr, const BinAddress& rMapKey )
{
    if( mxLastContext.get() && (mxLastContext->getBaseAddress() == rBaseAddr) )
        implSetSharedFormulaCell( *mxLastContext, rMapKey );
    mxLastContext.reset();
}

// ============================================================================

} // namespace xls
} // namespace oox

