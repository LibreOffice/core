 /*************************************************************************
 *
 *  $RCSfile: svgfilter.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 17:57:52 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "svgfilter.hxx"

// -------------
// - SVGFilter -
// -------------

SVGFilter::SVGFilter( const Reference< XMultiServiceFactory > &rxMSF ) :
    mxMSF( rxMSF ),
    mpSVGDoc( NULL ),
    mpSVGExport( NULL ),
    mpSVGFontExport( NULL ),
    mpSVGWriter( NULL ),
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
    Window*     pFocusWindow = Application::GetFocusWindow();
    sal_Bool    bRet;

    if( pFocusWindow )
        pFocusWindow->EnterWait();

    if( mxDstDoc.is() )
        bRet = implImport( rDescriptor );
    else if( mxSrcDoc.is() )
        bRet = implExport( rDescriptor );
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

void SAL_CALL SVGFilter::setTargetDocument( const Reference< XComponent >& xDoc )
    throw (::com::sun::star::lang::IllegalArgumentException, RuntimeException)
{
    mxDstDoc = xDoc;
}

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
