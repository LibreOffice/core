 /*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svgfilter.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 14:02:59 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"

#include <cstdio>

#include "svgfilter.hxx"

#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGE_HPP_
#include <com/sun/star/drawing/XDrawPage.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWVIEW_HPP_
#include <com/sun/star/drawing/XDrawView.hpp>
#endif
#include <com/sun/star/frame/XDesktop.hdl>
#include <com/sun/star/frame/XController.hdl>

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;

// -------------
// - SVGFilter -
// -------------

SVGFilter::SVGFilter( const Reference< XMultiServiceFactory > &rxMSF ) :
    mxMSF( rxMSF ),
    mpSVGDoc( NULL ),
    mpSVGExport( NULL ),
    mpSVGFontExport( NULL ),
    mpSVGWriter( NULL ),
    mpDefaultSdrPage( NULL ),
    mpSdrModel( NULL ),
    mbPresentation( sal_False )
{
}

// -----------------------------------------------------------------------------

SVGFilter::~SVGFilter()
{
    DBG_ASSERT( mpSVGDoc == NULL, "mpSVGDoc not destroyed" );
    DBG_ASSERT( mpSVGExport == NULL, "mpSVGExport not destroyed" );
    DBG_ASSERT( mpSVGFontExport == NULL, "mpSVGFontExport not destroyed" );
    DBG_ASSERT( mpSVGWriter == NULL, "mpSVGWriter not destroyed" );
    DBG_ASSERT( mpObjects == NULL, "mpObjects not destroyed" );
}

// -----------------------------------------------------------------------------

sal_Bool SAL_CALL SVGFilter::filter( const Sequence< PropertyValue >& rDescriptor )
    throw (RuntimeException)
{
    vos::OGuard aGuard( Application::GetSolarMutex() );
    Window*     pFocusWindow = Application::GetFocusWindow();
    sal_Int16   nCurrentPageNumber = -1;
    sal_Bool    bRet;

    if( pFocusWindow )
        pFocusWindow->EnterWait();

#ifdef SOLAR_JAVA
    if( mxDstDoc.is() )
        bRet = implImport( rDescriptor );
    else
#endif
    if( mxSrcDoc.is() )
    {
        uno::Reference< frame::XDesktop > xDesktop( mxMSF->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.frame.Desktop" ) ),
                                                    uno::UNO_QUERY);
        if( xDesktop.is() )
        {
            uno::Reference< frame::XFrame > xFrame( xDesktop->getCurrentFrame() );

            if( xFrame.is() )
            {
                uno::Reference< frame::XController > xController( xFrame->getController() );

                if( xController.is() )
                {
                    uno::Reference< drawing::XDrawView > xDrawView( xController, uno::UNO_QUERY );

                    if( xDrawView.is() )
                    {
                        uno::Reference< drawing::XDrawPage > xDrawPage( xDrawView->getCurrentPage() );

                        if( xDrawPage.is() )
                        {
                            uno::Reference< beans::XPropertySet >( xDrawPage, uno::UNO_QUERY )->
                                getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Number" ) ) ) >>= nCurrentPageNumber;
                        }
                    }
                }
            }
        }

        Sequence< PropertyValue > aNewDescritor( rDescriptor );

        if( nCurrentPageNumber > 0 )
        {
            const sal_uInt32    nOldLength = rDescriptor.getLength();

            aNewDescritor.realloc( nOldLength + 1 );
            aNewDescritor[ nOldLength ].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PagePos" ) );
            aNewDescritor[ nOldLength ].Value <<= static_cast< sal_Int16 >( nCurrentPageNumber - 1 );
        }

        bRet = implExport( aNewDescritor );
    }
    else
        bRet = sal_False;

    if( pFocusWindow )
        pFocusWindow->LeaveWait();

    return bRet;
}

// -----------------------------------------------------------------------------

void SAL_CALL SVGFilter::cancel( ) throw (RuntimeException)
{
}

// -----------------------------------------------------------------------------

void SAL_CALL SVGFilter::setSourceDocument( const Reference< XComponent >& xDoc )
    throw (IllegalArgumentException, RuntimeException)
{
    mxSrcDoc = xDoc;
}

// -----------------------------------------------------------------------------

#ifdef SOLAR_JAVA
void SAL_CALL SVGFilter::setTargetDocument( const Reference< XComponent >& xDoc )
    throw (::com::sun::star::lang::IllegalArgumentException, RuntimeException)
{
    mxDstDoc = xDoc;
}
#endif

// -----------------------------------------------------------------------------

void SAL_CALL SVGFilter::initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
    throw (Exception, RuntimeException)
{
}

// -----------------------------------------------------------------------------

OUString SVGFilter_getImplementationName ()
    throw (RuntimeException)
{
    return OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.comp.Draw.SVGFilter" ) );
}

// -----------------------------------------------------------------------------

#define SERVICE_NAME "com.sun.star.document.SVGFilter"

sal_Bool SAL_CALL SVGFilter_supportsService( const OUString& ServiceName )
    throw (RuntimeException)
{
    return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME ) );
}

// -----------------------------------------------------------------------------

Sequence< OUString > SAL_CALL SVGFilter_getSupportedServiceNames(  ) throw (RuntimeException)
{
    Sequence < OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] =  OUString ( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );
    return aRet;
}

#undef SERVICE_NAME

// -----------------------------------------------------------------------------

Reference< XInterface > SAL_CALL SVGFilter_createInstance( const Reference< XMultiServiceFactory > & rSMgr) throw( Exception )
{
    return (cppu::OWeakObject*) new SVGFilter( rSMgr );
}

// -----------------------------------------------------------------------------

OUString SAL_CALL SVGFilter::getImplementationName(  )
    throw (RuntimeException)
{
    return SVGFilter_getImplementationName();
}

// -----------------------------------------------------------------------------

sal_Bool SAL_CALL SVGFilter::supportsService( const OUString& rServiceName )
    throw (RuntimeException)
{
    return SVGFilter_supportsService( rServiceName );
}

// -----------------------------------------------------------------------------

::com::sun::star::uno::Sequence< OUString > SAL_CALL SVGFilter::getSupportedServiceNames(  ) throw (RuntimeException)
{
    return SVGFilter_getSupportedServiceNames();
}
