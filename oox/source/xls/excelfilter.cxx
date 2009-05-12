/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: excelfilter.cxx,v $
 * $Revision: 1.6.6.7 $
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
#include "oox/xls/themebuffer.hxx"
#include "oox/xls/workbookfragment.hxx"
#include "oox/dump/biffdumper.hxx"
#include "oox/dump/xlsbdumper.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::xml::sax::XFastDocumentHandler;
using ::oox::core::BinaryFilterBase;
using ::oox::core::FragmentHandlerRef;
using ::oox::core::Relation;
using ::oox::core::Relations;
using ::oox::core::XmlFilterBase;
using ::oox::vml::DrawingPtr;
using ::oox::drawingml::table::TableStyleListPtr;

namespace oox {
namespace xls {

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
        const Reference< XMultiServiceFactory >& rxFactory ) throw( Exception )
{
    return static_cast< ::cppu::OWeakObject* >( new ExcelFilter( rxFactory ) );
}

// ----------------------------------------------------------------------------

ExcelFilter::ExcelFilter( const Reference< XMultiServiceFactory >& rxFactory ) :
    XmlFilterBase( rxFactory ),
    mpHelper( 0 )
{
}

ExcelFilter::~ExcelFilter()
{
}

bool ExcelFilter::importDocument() throw()
{
    /*  to activate the XLSX/XLSB dumper, define the environment variable
        OOO_XLSBDUMPER and insert the full path to the file
        file:///<path-to-oox-module>/source/dump/xlsbdumper.ini. */
    OOX_DUMP_FILE( ::oox::dump::xlsb::Dumper );

    bool bRet = false;
    OUString aWorkbookPath = getFragmentPathFromType( CREATE_OFFICEDOC_RELATIONSTYPE( "officeDocument" ) );
    if( aWorkbookPath.getLength() > 0 )
    {
        WorkbookHelperRoot aHelper( *this );
        if( aHelper.isValid() )
        {
            mpHelper = &aHelper;    // needed for callbacks
            bRet = importFragment( new OoxWorkbookFragment( aHelper, aWorkbookPath ) );
            mpHelper = 0;
        }
    }
    return bRet;
}

bool ExcelFilter::exportDocument() throw()
{
    return false;
}

const ::oox::drawingml::Theme* ExcelFilter::getCurrentTheme() const
{
    return &mpHelper->getTheme();
}

sal_Int32 ExcelFilter::getSchemeClr( sal_Int32 nColorSchemeToken ) const
{
    OSL_ENSURE( mpHelper, "ExcelFilter::getSchemeClr - no workbook helper" );
    return mpHelper->getTheme().getColorByToken( nColorSchemeToken );
}

const DrawingPtr ExcelFilter::getDrawings()
{
    return DrawingPtr();
}

const TableStyleListPtr ExcelFilter::getTableStyles()
{
    return TableStyleListPtr();
}

::oox::drawingml::chart::ChartConverter& ExcelFilter::getChartConverter()
{
    OSL_ENSURE( mpHelper, "ExcelFilter::getChartConverter - no workbook helper" );
    return mpHelper->getChartConverter();
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
        const Reference< XMultiServiceFactory >& rxFactory ) throw( Exception )
{
    return static_cast< ::cppu::OWeakObject* >( new ExcelBiffFilter( rxFactory ) );
}

// ----------------------------------------------------------------------------

ExcelBiffFilter::ExcelBiffFilter( const Reference< XMultiServiceFactory >& rxFactory ) :
    BinaryFilterBase( rxFactory )
{
}

ExcelBiffFilter::~ExcelBiffFilter()
{
}

bool ExcelBiffFilter::importDocument() throw()
{
    /*  to activate the BIFF dumper, define the environment variable
        OOO_BIFFDUMPER and insert the full path to the file
        file:///<path-to-oox-module>/source/dump/biffdumper.ini. */
    OOX_DUMP_FILE( ::oox::dump::biff::Dumper );

    /*  A boolean argument passed through XInitialisation decides whether to
        use the BIFF file dumper implemented in this filter only, or to really
        import/export the document. */
    const Sequence< Any >& rArgs = getArguments();
    bool bDumperOnly = false;
    if( (rArgs.getLength() >= 2) && (rArgs[ 1 ] >>= bDumperOnly) && bDumperOnly )
        return true;

    bool bRet = false;

    // detect BIFF version and workbook stream name
    OUString aWorkbookName;
    BiffType eBiff = BiffDetector::detectStorageBiffVersion( aWorkbookName, getStorage() );
    OSL_ENSURE( eBiff != BIFF_UNKNOWN, "ExcelBiffFilter::ExcelBiffFilter - invalid file format" );
    if( eBiff != BIFF_UNKNOWN )
    {
        WorkbookHelperRoot aHelper( *this, eBiff );
        bRet = aHelper.isValid() && BiffWorkbookFragment( aHelper, aWorkbookName ).importFragment();
    }
    return bRet;
}

bool ExcelBiffFilter::exportDocument() throw()
{
    return false;
}

OUString ExcelBiffFilter::implGetImplementationName() const
{
    return ExcelBiffFilter_getImplementationName();
}

// ============================================================================

} // namespace xls
} // namespace oox

