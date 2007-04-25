/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbaglobals.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-25 16:06:41 $
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
#ifndef SC_VBA_GLOBALS
#include "vbaglobals.hxx"
#endif

#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include "vbaapplication.hxx"
#include "vbaworksheet.hxx"
#include "vbarange.hxx"
#include <cppuhelper/bootstrap.hxx>
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::org::openoffice;



namespace vbaobj
{
    ::rtl::OUString SAL_CALL getImplementationName()
    {
        static ::rtl::OUString* pImplName = 0;
        if ( !pImplName )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if ( !pImplName )
            {
                static ::rtl::OUString aImplName( RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.vba.Globals" ) );
                pImplName = &aImplName;
            }
        }
        return *pImplName;
    }

    uno::Reference< XComponentContext > getComponentContextFromMSF( uno::Reference< lang::XMultiServiceFactory > const& xFactory )
    {
        uno::Reference< XComponentContext > xContext;

            uno::Reference< beans::XPropertySet > xProps( xFactory, UNO_QUERY );
        if (xProps.is())
        {
            xProps->getPropertyValue(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("DefaultContext") ) ) >>= xContext;
        }
        return xContext;
    }


    uno::Reference< XInterface > SAL_CALL create(
        Reference< XComponentContext > const & xContext )
        SAL_THROW( () )
    {
    if ( !xContext.is() )
    {
        OSL_TRACE("Failed to obtain context" );
        return uno::Reference< uno::XInterface >(NULL);
    }
    OSL_TRACE("In create component for vbaglobals");
        return static_cast< lang::XTypeProvider * >( new ScVbaGlobals( xContext ) );
    }

    Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
    {
        const ::rtl::OUString strName( ::vbaobj::getImplementationName() );
        return Sequence< ::rtl::OUString >( &strName, 1 );
    }
}

// =============================================================================
// ScVbaGlobals
// =============================================================================

ScVbaGlobals::ScVbaGlobals( const Reference< XComponentContext >& rxContext )
        :m_xContext( rxContext )
{
//  OSL_TRACE("ScVbaGlobals::ScVbaGlobals()");
    mxApplication = uno::Reference< excel::XApplication > ( new ScVbaApplication( m_xContext) );
}

ScVbaGlobals::~ScVbaGlobals()
{
//  OSL_TRACE("ScVbaGlobals::~ScVbaGlobals");
}

// Will throw if singleton can't be accessed
uno::Reference< vba::XGlobals >
ScVbaGlobals::getGlobalsImpl( uno::Reference< uno::XComponentContext >& xContext ) throw ( uno::RuntimeException )
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

// -----------------------------------------------------------------------------
// XServiceInfo
// -----------------------------------------------------------------------------

::rtl::OUString ScVbaGlobals::getImplementationName(  ) throw (RuntimeException)
{
    return ::vbaobj::getImplementationName();
}

sal_Bool ScVbaGlobals::supportsService( const ::rtl::OUString& rServiceName ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aNames( ::vbaobj::getSupportedServiceNames() );
    const ::rtl::OUString* pNames = aNames.getConstArray();
    const ::rtl::OUString* pEnd = pNames + aNames.getLength();
    for ( ; pNames != pEnd && !pNames->equals( rServiceName ); ++pNames )
        ;

    return pNames != pEnd;
}

Sequence< ::rtl::OUString > ScVbaGlobals::getSupportedServiceNames(  ) throw (RuntimeException)
{
    return vbaobj::getSupportedServiceNames();
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


