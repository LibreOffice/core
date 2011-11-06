/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    mpBookGlob( 0 )
{
}

ExcelFilterBase::~ExcelFilterBase()
{
    OSL_ENSURE( !mpBookGlob, "ExcelFilterBase::~ExcelFilterBase - workbook data not cleared" );
}

void ExcelFilterBase::registerWorkbookGlobals( WorkbookGlobals& rBookGlob )
{
    mpBookGlob = &rBookGlob;
}

WorkbookGlobals& ExcelFilterBase::getWorkbookGlobals() const
{
    OSL_ENSURE( mpBookGlob, "ExcelFilterBase::getWorkbookGlobals - missing workbook data" );
    return *mpBookGlob;
}

void ExcelFilterBase::unregisterWorkbookGlobals()
{
    mpBookGlob = 0;
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

    /*  Construct the WorkbookGlobals object referred to by every instance of
        the class WorkbookHelper, and execute the import filter by constructing
        an instance of WorkbookFragment and loading the file. */
    WorkbookGlobalsRef xBookGlob = WorkbookHelper::constructGlobals( *this );
    return xBookGlob.get() && importFragment( new WorkbookFragment( *xBookGlob, aWorkbookPath ) );
}

bool ExcelFilter::exportDocument() throw()
{
    return false;
}

const ::oox::drawingml::Theme* ExcelFilter::getCurrentTheme() const
{
    return &WorkbookHelper( getWorkbookGlobals() ).getTheme();
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
    return WorkbookHelper( getWorkbookGlobals() ).getChartConverter();
}

GraphicHelper* ExcelFilter::implCreateGraphicHelper() const
{
    return new ExcelGraphicHelper( getWorkbookGlobals() );
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

    /*  Construct the WorkbookGlobals object referred to by every instance of
        the class WorkbookHelper, and execute the import filter by constructing
        an instance of BiffWorkbookFragment and loading the file. */
    WorkbookGlobalsRef xBookGlob = WorkbookHelper::constructGlobals( *this, eBiff );
    return xBookGlob.get() && BiffWorkbookFragment( *xBookGlob, aWorkbookName ).importFragment();
}

bool ExcelBiffFilter::exportDocument() throw()
{
    return false;
}

GraphicHelper* ExcelBiffFilter::implCreateGraphicHelper() const
{
    return new ExcelGraphicHelper( getWorkbookGlobals() );
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

    /*  Construct the WorkbookGlobals object referred to by every instance of
        the class WorkbookHelper. */
    WorkbookGlobalsRef xBookGlob = WorkbookHelper::constructGlobals( *this, eBiff );
    if( !xBookGlob.get() )
        return false;

    // set palette colors passed in service constructor
    Any aPalette = getArgument( CREATE_OUSTRING( "ColorPalette" ) );
    WorkbookHelper( *xBookGlob ).getStyles().importPalette( aPalette );
    // import the VBA project (getVbaProject() implemented in base class)
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
