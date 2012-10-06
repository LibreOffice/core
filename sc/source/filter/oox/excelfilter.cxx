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

#include "excelfilter.hxx"

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include "oox/helper/binaryinputstream.hxx"
#include "biffinputstream.hxx"
#include "excelchartconverter.hxx"
#include "excelvbaproject.hxx"
#include "stylesbuffer.hxx"
#include "themebuffer.hxx"
#include "workbookfragment.hxx"

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
    return OUString( "com.sun.star.comp.oox.xls.ExcelFilter" );
}

Sequence< OUString > SAL_CALL ExcelFilter_getSupportedServiceNames() throw()
{
    Sequence< OUString > aSeq( 2 );
    aSeq[ 0 ] = "com.sun.star.document.ImportFilter";
    aSeq[ 1 ] = "com.sun.star.document.ExportFilter";
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
    //OOX_DUMP_FILE( ::oox::dump::xlsb::Dumper );

    OUString aWorkbookPath = getFragmentPathFromFirstType( CREATE_OFFICEDOC_RELATION_TYPE( "officeDocument" ) );
    if( aWorkbookPath.isEmpty() )
        return false;

    /*  Construct the WorkbookGlobals object referred to by every instance of
        the class WorkbookHelper, and execute the import filter by constructing
        an instance of WorkbookFragment and loading the file. */
    WorkbookGlobalsRef xBookGlob = WorkbookHelper::constructGlobals( *this );
    if ( xBookGlob.get() && importFragment( new WorkbookFragment( *xBookGlob, aWorkbookPath ) ) )
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

::oox::drawingml::chart::ChartConverter* ExcelFilter::getChartConverter()
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


sal_Bool SAL_CALL ExcelFilter::filter( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rDescriptor ) throw( ::com::sun::star::uno::RuntimeException )
{
    if ( XmlFilterBase::filter( rDescriptor ) )
        return true;

    if ( isExportFilter() )
    {
        Reference< XExporter > xExporter( getServiceFactory()->createInstance( "com.sun.star.comp.oox.ExcelFilterExport" ), UNO_QUERY );

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

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
