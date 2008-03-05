/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: excelfilter.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:58:54 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "oox/xls/excelfilter.hxx"
#include "oox/helper/binaryinputstream.hxx"
#include "oox/xls/biffdetector.hxx"
#include "oox/xls/biffinputstream.hxx"
#include "oox/xls/themebuffer.hxx"
#include "oox/xls/workbookfragment.hxx"
#include "oox/dump/biffdumper.hxx"
#include "oox/dump/xlsbdumper.hxx"

using ::rtl::OUString;
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
        file:///<path-to-oox-module>/source/dump/xlsbdumperconfig.dat. */
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

sal_Int32 ExcelFilter::getSchemeClr( sal_Int32 nColorSchemeToken ) const
{
    OSL_ENSURE( mpHelper, "ExcelFilter::getSchemeClr - no workbook helper" );
    return mpHelper ? mpHelper->getTheme().getColorByToken( nColorSchemeToken ) : -1;
}

const DrawingPtr ExcelFilter::getDrawings()
{
    return DrawingPtr();
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
        file:///<path-to-oox-module>/source/dump/biffdumperconfig.dat. */
    OOX_DUMP_FILE( ::oox::dump::biff::Dumper );

    bool bRet = false;

    // detect BIFF version and workbook stream name
    OUString aWorkbookName;
    BiffType eBiff = BiffDetector::detectStorageBiffVersion( aWorkbookName, getStorage() );
    BinaryInputStream aInStrm( getStorage()->openInputStream( aWorkbookName ), aWorkbookName.getLength() > 0 );
    OSL_ENSURE( (eBiff != BIFF_UNKNOWN) && aInStrm.is(), "ExcelBiffFilter::ExcelBiffFilter - invalid file format" );

    if( (eBiff != BIFF_UNKNOWN) && aInStrm.is() )
    {
        WorkbookHelperRoot aHelper( *this, eBiff );
        if( aHelper.isValid() )
        {
            BiffWorkbookFragment aFragment( aHelper );
            BiffInputStream aBiffStream( aInStrm );
            bRet = aFragment.importFragment( aBiffStream );
        }
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

