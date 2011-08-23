/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "oox/helper/binaryinputstream.hxx"
#include "oox/xls/biffdetector.hxx"
#include "oox/xls/biffinputstream.hxx"
#include "oox/xls/excelchartconverter.hxx"
#include "oox/xls/stylesbuffer.hxx"
#include "oox/xls/themebuffer.hxx"
#include "oox/xls/workbookfragment.hxx"
#include "oox/dump/biffdumper.hxx"
#include "oox/dump/xlsbdumper.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::lang::XComponent;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::xml::sax::XFastDocumentHandler;
using ::oox::core::BinaryFilterBase;
using ::oox::core::FragmentHandlerRef;
using ::oox::core::Relation;
using ::oox::core::Relations;
using ::oox::core::XmlFilterBase;
using ::oox::drawingml::table::TableStyleListPtr;

namespace oox {
namespace xls {

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
    return CREATE_OUSTRING( "com.sun.star.comp.oox.ExcelFilter" );
}

Sequence< OUString > SAL_CALL ExcelFilter_getSupportedServiceNames() throw()
{
    OUString aServiceName = CREATE_OUSTRING( "com.sun.star.comp.oox.ExcelFilter" );
    Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

Reference< XInterface > SAL_CALL ExcelFilter_createInstance(
        const Reference< XMultiServiceFactory >& rxGlobalFactory ) throw( Exception )
{
    return static_cast< ::cppu::OWeakObject* >( new ExcelFilter( rxGlobalFactory ) );
}

// ----------------------------------------------------------------------------

ExcelFilter::ExcelFilter( const Reference< XMultiServiceFactory >& rxGlobalFactory ) :
    XmlFilterBase( rxGlobalFactory )
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

    OUString aWorkbookPath = getFragmentPathFromFirstType( CREATE_OFFICEDOC_RELATIONSTYPE( "officeDocument" ) );
    if( aWorkbookPath.getLength() == 0 )
        return false;
        
    WorkbookHelperRoot aHelper( *this );
    if( aHelper.isValid() && importFragment( new OoxWorkbookFragment( aHelper, aWorkbookPath ) ) )
    {
        importDocumentProperties();
        return true;
    }
    return false;
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

sal_Bool SAL_CALL ExcelFilter::filter( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rDescriptor ) throw( ::com::sun::star::uno::RuntimeException )
{
    if ( XmlFilterBase::filter( rDescriptor ) )
        return true;

    if ( isExportFilter() )
    {
        Reference< XExporter > xExporter( getGlobalFactory()->createInstance( CREATE_OUSTRING( "com.sun.star.comp.oox.ExcelFilterExport" ) ), UNO_QUERY );

        if ( xExporter.is() )
        {
            Reference< XComponent > xDocument( getModel(), UNO_QUERY );
            Reference< XFilter > xFilter( xExporter, UNO_QUERY );

            if ( xFilter.is() )
            {
                xExporter->setSourceDocument( xDocument );
                if ( xFilter->filter( rDescriptor ) )
                    return true;
            }
        }
    }

    return false;
}

OUString ExcelFilter::implGetImplementationName() const
{
    return ExcelFilter_getImplementationName();
}

// ============================================================================

OUString SAL_CALL ExcelBiffFilter_getImplementationName() throw()
{
    return CREATE_OUSTRING( "com.sun.star.comp.oox.ExcelBiffFilter" );
}

Sequence< OUString > SAL_CALL ExcelBiffFilter_getSupportedServiceNames() throw()
{
    OUString aServiceName = CREATE_OUSTRING( "com.sun.star.comp.oox.ExcelBiffFilter" );
    Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

Reference< XInterface > SAL_CALL ExcelBiffFilter_createInstance(
        const Reference< XMultiServiceFactory >& rxGlobalFactory ) throw( Exception )
{
    return static_cast< ::cppu::OWeakObject* >( new ExcelBiffFilter( rxGlobalFactory ) );
}

// ----------------------------------------------------------------------------

ExcelBiffFilter::ExcelBiffFilter( const Reference< XMultiServiceFactory >& rxGlobalFactory ) :
    BinaryFilterBase( rxGlobalFactory )
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

OUString ExcelBiffFilter::implGetImplementationName() const
{
    return ExcelBiffFilter_getImplementationName();
}

// ============================================================================

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
