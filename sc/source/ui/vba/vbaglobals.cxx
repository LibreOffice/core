/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbaglobals.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 10:53:12 $
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
#include "helperdecl.hxx"
#include "vbaglobals.hxx"

#include <comphelper/unwrapargs.hxx>

#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include "vbaapplication.hxx"
#include "vbaworksheet.hxx"
#include "vbarange.hxx"
#include <cppuhelper/bootstrap.hxx>
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::org::openoffice;



// =============================================================================
// ScVbaGlobals
// =============================================================================

ScVbaGlobals::ScVbaGlobals(  css::uno::Reference< css::uno::XComponentContext >const& rxContext )
        :m_xContext( rxContext )
{
    OSL_TRACE("ScVbaGlobals::ScVbaGlobals()");
    mxApplication = uno::Reference< excel::XApplication > ( new ScVbaApplication( m_xContext) );
}

ScVbaGlobals::~ScVbaGlobals()
{
    OSL_TRACE("ScVbaGlobals::~ScVbaGlobals");
}

// Will throw if singleton can't be accessed
uno::Reference< vba::XGlobals >
ScVbaGlobals::getGlobalsImpl( const uno::Reference< uno::XComponentContext >& xContext ) throw ( uno::RuntimeException )
{
    uno::Reference< vba::XGlobals > xGlobals(
        xContext->getValueByName( ::rtl::OUString::createFromAscii(
            "/singletons/org.openoffice.vba.theGlobals") ), uno::UNO_QUERY);

    if ( !xGlobals.is() )
    {
        throw uno::RuntimeException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ": Couldn't access Globals" ) ),
                    uno::Reference< XInterface >() );
    }
    return xGlobals;
}

// =============================================================================
// XGlobals
// =============================================================================
uno::Reference<excel::XApplication >
ScVbaGlobals::getApplication() throw (uno::RuntimeException)
{
//  OSL_TRACE("In ScVbaGlobals::getApplication");
    return mxApplication;
}

uno::Reference< excel::XWorkbook > SAL_CALL
ScVbaGlobals::getActiveWorkbook() throw (uno::RuntimeException)
{
//  OSL_TRACE("In ScVbaGlobals::getActiveWorkbook");
    uno::Reference< excel::XWorkbook > xWorkbook( mxApplication->getActiveWorkbook(), uno::UNO_QUERY);
    if ( xWorkbook.is() )
    {
        return xWorkbook;
    }
// FIXME check if this is correct/desired behavior
    throw uno::RuntimeException( rtl::OUString::createFromAscii(
        "No activeWorkbook available" ), Reference< uno::XInterface >() );
}


uno::Reference< excel::XWorksheet > SAL_CALL
ScVbaGlobals::getActiveSheet() throw (uno::RuntimeException)
{
    return mxApplication->getActiveSheet();
}

uno::Any SAL_CALL
ScVbaGlobals::WorkBooks( const uno::Any& aIndex ) throw (uno::RuntimeException)
{
    return uno::Any( mxApplication->Workbooks(aIndex) );
}

uno::Any SAL_CALL
ScVbaGlobals::WorkSheets(const uno::Any& aIndex) throw (uno::RuntimeException)
{
    return mxApplication->Worksheets( aIndex );
}
uno::Any SAL_CALL
ScVbaGlobals::Sheets( const uno::Any& aIndex ) throw (uno::RuntimeException)
{
    return WorkSheets( aIndex );
}

::uno::Sequence< ::uno::Any > SAL_CALL
ScVbaGlobals::getGlobals(  ) throw (::uno::RuntimeException)
{
    sal_uInt32 nMax = 0;
    uno::Sequence< uno::Any > maGlobals(4);
    maGlobals[ nMax++ ] <<= ScVbaGlobals::getGlobalsImpl(m_xContext);
    maGlobals[ nMax++ ] <<= mxApplication;

    uno::Reference< excel::XWorkbook > xWorkbook = mxApplication->getActiveWorkbook();
    if( xWorkbook.is() )
    {
        maGlobals[ nMax++ ] <<= xWorkbook;
        uno::Reference< excel::XWorksheet > xWorksheet = xWorkbook->getActiveSheet();
        if( xWorksheet.is() )
            maGlobals[ nMax++ ] <<= xWorksheet;
    }
    maGlobals.realloc( nMax );
    return maGlobals;
}

uno::Any SAL_CALL
ScVbaGlobals::Range( const uno::Any& Cell1, const uno::Any& Cell2 ) throw (uno::RuntimeException)
{
    return getApplication()->Range( Cell1, Cell2 );
}

uno::Any SAL_CALL
ScVbaGlobals::Names( ) throw ( uno::RuntimeException )
{
    return getApplication()->Names();
}

namespace globals
{
namespace sdecl = comphelper::service_decl;
sdecl::class_<ScVbaGlobals, sdecl::with_args<false> > serviceImpl;
extern sdecl::ServiceDecl const serviceDecl(
    serviceImpl,
    "ScVbaGlobals",
    "org.openoffice.vba.Globals" );
}

