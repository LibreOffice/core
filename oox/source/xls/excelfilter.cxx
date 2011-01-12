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

#include "oox/xls/excelfilter.hxx"

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include "oox/dump/biffdumper.hxx"
#include "oox/dump/xlsbdumper.hxx"
#include "oox/helper/binaryinputstream.hxx"
#include "oox/xls/biffdetector.hxx"
#include "oox/xls/biffinputstream.hxx"
#include "oox/xls/excelchartconverter.hxx"
#include "oox/xls/excelvbaproject.hxx"
#include "oox/xls/stylesbuffer.hxx"
#include "oox/xls/themebuffer.hxx"
#include "oox/xls/workbookfragment.hxx"

namespace oox {
namespace xls {

// ============================================================================

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::oox::core;

using ::rtl::OUString;
using ::oox::drawingml::table::TableStyleListPtr;

// ============================================================================

ExcelFilterBase::ExcelFilterBase() :
    mpData( 0 )
{
}

ExcelFilterBase::~ExcelFilterBase()
{
    OSL_ENSURE( !mpData, "ExcelFilterBase::~ExcelFilterBase - workbook data not cleared" );
}

void ExcelFilterBase::registerWorkbookData( WorkbookData& rData )
{
    mpData = &rData;
}

WorkbookData& ExcelFilterBase::getWorkbookData() const
{
    OSL_ENSURE( mpData, "ExcelFilterBase::getWorkbookData - missing workbook data" );
    return *mpData;
}

void ExcelFilterBase::unregisterWorkbookData()
{
    mpData = 0;
}

// ============================================================================

OUString SAL_CALL ExcelFilter_getImplementationName() throw()
{
    return CREATE_OUSTRING( "com.sun.star.comp.oox.xls.ExcelFilter" );
}

Sequence< OUString > SAL_CALL ExcelFilter_getSupportedServiceNames() throw()
{
    Sequence< OUString > aSeq( 2 );
    aSeq[ 0 ] = CREATE_OUSTRING( "com.sun.star.document.ImportFilter" );
    aSeq[ 1 ] = CREATE_OUSTRING( "com.sun.star.document.ExportFilter" );
    return aSeq;
}

Reference< XInterface > SAL_CALL ExcelFilter_createInstance(
        const Reference< XComponentContext >& rxContext ) throw( Exception )
{
    return static_cast< ::cppu::OWeakObject* >( new ExcelFilter( rxContext ) );
}

// ----------------------------------------------------------------------------

ExcelFilter::ExcelFilter( const Reference< XComponentContext >& rxContext ) throw( RuntimeException ) :
    XmlFilterBase( rxContext )
{
}

ExcelFilter::~ExcelFilter()
{
}

bool ExcelFilter::importDocument() throw()
{
    /*  To activate the XLSX/XLSB dumper, insert the full path to the file
        file:///<path-to-oox-module>/source/dump/xlsbdumper.ini
        into the environment variable OOO_XLSBDUMPER and start the office with
        this variable (nonpro only). */
    OOX_DUMP_FILE( ::oox::dump::xlsb::Dumper );

    OUString aWorkbookPath = getFragmentPathFromFirstType( CREATE_OFFICEDOC_RELATION_TYPE( "officeDocument" ) );
    if( aWorkbookPath.getLength() == 0 )
        return false;

    WorkbookHelperRoot aHelper( *this );
    return aHelper.isValid() && importFragment( new WorkbookFragment( aHelper, aWorkbookPath ) );
}

bool ExcelFilter::exportDocument() throw()
{
    return false;
}

const ::oox::drawingml::Theme* ExcelFilter::getCurrentTheme() const
{
    return &WorkbookHelper( getWorkbookData() ).getTheme();
}

::oox::vml::Drawing* ExcelFilter::getVmlDrawing()
{
    return 0;
}

const TableStyleListPtr ExcelFilter::getTableStyles()
{
    return TableStyleListPtr();
}

::oox::drawingml::chart::ChartConverter& ExcelFilter::getChartConverter()
{
    return WorkbookHelper( getWorkbookData() ).getChartConverter();
}

GraphicHelper* ExcelFilter::implCreateGraphicHelper() const
{
    return new ExcelGraphicHelper( getWorkbookData() );
}

::oox::ole::VbaProject* ExcelFilter::implCreateVbaProject() const
{
    return new ExcelVbaProject( getComponentContext(), Reference< XSpreadsheetDocument >( getModel(), UNO_QUERY ) );
}

OUString ExcelFilter::implGetImplementationName() const
{
    return ExcelFilter_getImplementationName();
}

// ============================================================================

OUString SAL_CALL ExcelBiffFilter_getImplementationName() throw()
{
    return CREATE_OUSTRING( "com.sun.star.comp.oox.xls.ExcelBiffFilter" );
}

Sequence< OUString > SAL_CALL ExcelBiffFilter_getSupportedServiceNames() throw()
{
    Sequence< OUString > aSeq( 2 );
    aSeq[ 0 ] = CREATE_OUSTRING( "com.sun.star.document.ImportFilter" );
    aSeq[ 1 ] = CREATE_OUSTRING( "com.sun.star.document.ExportFilter" );
    return aSeq;
}

Reference< XInterface > SAL_CALL ExcelBiffFilter_createInstance(
        const Reference< XComponentContext >& rxContext ) throw( Exception )
{
    return static_cast< ::cppu::OWeakObject* >( new ExcelBiffFilter( rxContext ) );
}

// ----------------------------------------------------------------------------

ExcelBiffFilter::ExcelBiffFilter( const Reference< XComponentContext >& rxContext ) throw( RuntimeException ) :
    BinaryFilterBase( rxContext )
{
}

ExcelBiffFilter::~ExcelBiffFilter()
{
}

bool ExcelBiffFilter::importDocument() throw()
{
    /*  To activate the BIFF dumper, insert the full path to the file
        file:///<path-to-oox-module>/source/dump/biffdumper.ini
        into the environment variable OOO_BIFFDUMPER and start the office with
        this variable (nonpro only). */
    OOX_DUMP_FILE( ::oox::dump::biff::Dumper );

    /*  The boolean argument "UseBiffFilter" passed through XInitialisation
        decides whether to import/export the document with this filter (true),
        or to only use the BIFF file dumper implemented in this filter (false
        or missing) */
    Any aUseBiffFilter = getArgument( CREATE_OUSTRING( "UseBiffFilter" ) );
    bool bUseBiffFilter = false;
    if( !(aUseBiffFilter >>= bUseBiffFilter) || !bUseBiffFilter )
        return true;

    // detect BIFF version and workbook stream name
    OUString aWorkbookName;
    BiffType eBiff = BiffDetector::detectStorageBiffVersion( aWorkbookName, getStorage() );
    OSL_ENSURE( eBiff != BIFF_UNKNOWN, "ExcelBiffFilter::ExcelBiffFilter - invalid file format" );
    if( eBiff == BIFF_UNKNOWN )
        return false;

    WorkbookHelperRoot aHelper( *this, eBiff );
    return aHelper.isValid() && BiffWorkbookFragment( aHelper, aWorkbookName ).importFragment();
}

bool ExcelBiffFilter::exportDocument() throw()
{
    return false;
}

GraphicHelper* ExcelBiffFilter::implCreateGraphicHelper() const
{
    return new ExcelGraphicHelper( getWorkbookData() );
}

::oox::ole::VbaProject* ExcelBiffFilter::implCreateVbaProject() const
{
    return new ExcelVbaProject( getComponentContext(), Reference< XSpreadsheetDocument >( getModel(), UNO_QUERY ) );
}

OUString ExcelBiffFilter::implGetImplementationName() const
{
    return ExcelBiffFilter_getImplementationName();
}

// ============================================================================

OUString SAL_CALL ExcelVbaProjectFilter_getImplementationName() throw()
{
    return CREATE_OUSTRING( "com.sun.star.comp.oox.xls.ExcelVbaProjectFilter" );
}

Sequence< OUString > SAL_CALL ExcelVbaProjectFilter_getSupportedServiceNames() throw()
{
    Sequence< OUString > aSeq( 1 );
    aSeq[ 0 ] = CREATE_OUSTRING( "com.sun.star.document.ImportFilter" );
    return aSeq;
}

Reference< XInterface > SAL_CALL ExcelVbaProjectFilter_createInstance(
        const Reference< XComponentContext >& rxContext ) throw( Exception )
{
    return static_cast< ::cppu::OWeakObject* >( new ExcelVbaProjectFilter( rxContext ) );
}

// ----------------------------------------------------------------------------

ExcelVbaProjectFilter::ExcelVbaProjectFilter( const Reference< XComponentContext >& rxContext ) throw( RuntimeException ) :
    ExcelBiffFilter( rxContext )
{
}

bool ExcelVbaProjectFilter::importDocument() throw()
{
    // detect BIFF version and workbook stream name
    OUString aWorkbookName;
    BiffType eBiff = BiffDetector::detectStorageBiffVersion( aWorkbookName, getStorage() );
    OSL_ENSURE( eBiff == BIFF8, "ExcelVbaProjectFilter::ExcelVbaProjectFilter - invalid file format" );
    if( eBiff != BIFF8 )
        return false;

    StorageRef xVbaPrjStrg = openSubStorage( CREATE_OUSTRING( "_VBA_PROJECT_CUR" ), false );
    if( !xVbaPrjStrg || !xVbaPrjStrg->isStorage() )
        return false;

    WorkbookHelperRoot aHelper( *this, eBiff );
    // set palette colors passed in service constructor
    Any aPalette = getArgument( CREATE_OUSTRING( "ColorPalette" ) );
    aHelper.getStyles().importPalette( aPalette );
    // import the VBA project
    getVbaProject().importVbaProject( *xVbaPrjStrg, getGraphicHelper() );
    return true;
}

bool ExcelVbaProjectFilter::exportDocument() throw()
{
    return false;
}

OUString ExcelVbaProjectFilter::implGetImplementationName() const
{
    return ExcelVbaProjectFilter_getImplementationName();
}

// ============================================================================

} // namespace xls
} // namespace oox
