 /*************************************************************************
 *
 *  $RCSfile: pdffilter.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ka $ $Date: 2002-08-16 15:57:27 $
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

#include "pdffilter.hxx"
#include "pdfexport.hxx"
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <svtools/outstrm.hxx>

// -------------
// - PDFFilter -
// -------------

PDFFilter::PDFFilter( const Reference< XMultiServiceFactory > &rxMSF ) :
    mxMSF( rxMSF )
{
}

// -----------------------------------------------------------------------------

PDFFilter::~PDFFilter()
{
}

// -----------------------------------------------------------------------------

sal_Bool PDFFilter::implExport( const Sequence< PropertyValue >& rDescriptor )
{
    Reference< XOutputStream >  xOStm;
    sal_Int32                   nLength = rDescriptor.getLength();
    const PropertyValue*        pValue = rDescriptor.getConstArray();
    sal_Bool                    bRet = sal_False;

    for ( sal_Int32 i = 0 ; ( i < nLength ) && !xOStm.is(); ++i)
        if( pValue[ i ].Name.equalsAscii( "OutputStream" ) )
            pValue[ i ].Value >>= xOStm;

    if( mxSrcDoc.is() && xOStm.is() )
    {
        PDFExport       aExport( mxSrcDoc );
        ::utl::TempFile aTempFile;

        aTempFile.EnableKillingFile();
        bRet = aExport.Export( aTempFile.GetURL() );

        if( bRet )
        {
            SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( aTempFile.GetURL(), STREAM_READ );

            if( pIStm )
            {
                SvOutputStream aOStm( xOStm );

                aOStm << *pIStm;
                bRet = ( aOStm.Tell() && ( aOStm.GetError() == ERRCODE_NONE ) );

                delete pIStm;
            }
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool SAL_CALL PDFFilter::filter( const Sequence< PropertyValue >& rDescriptor )
    throw (RuntimeException)
{
    Window* pFocusWindow = Application::GetFocusWindow();

    if( pFocusWindow )
        pFocusWindow->EnterWait();

    const sal_Bool bRet = implExport( rDescriptor );

    if( pFocusWindow )
        pFocusWindow->LeaveWait();

    return bRet;
}

// -----------------------------------------------------------------------------

void SAL_CALL PDFFilter::cancel( ) throw (RuntimeException)
{
}

// -----------------------------------------------------------------------------

void SAL_CALL PDFFilter::setSourceDocument( const Reference< XComponent >& xDoc )
    throw (IllegalArgumentException, RuntimeException)
{
    mxSrcDoc = xDoc;
}

// -----------------------------------------------------------------------------

void SAL_CALL PDFFilter::initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
    throw (Exception, RuntimeException)
{
}

// -----------------------------------------------------------------------------

OUString PDFFilter_getImplementationName ()
    throw (RuntimeException)
{
    return OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.comp.PDF.PDFFilter" ) );
}

// -----------------------------------------------------------------------------

#define SERVICE_NAME "com.sun.star.document.PDFFilter"

sal_Bool SAL_CALL PDFFilter_supportsService( const OUString& ServiceName )
    throw (RuntimeException)
{
    return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME ) );
}

// -----------------------------------------------------------------------------

Sequence< OUString > SAL_CALL PDFFilter_getSupportedServiceNames(  ) throw (RuntimeException)
{
    Sequence < OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] =  OUString ( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );
    return aRet;
}

#undef SERVICE_NAME

// -----------------------------------------------------------------------------

Reference< XInterface > SAL_CALL PDFFilter_createInstance( const Reference< XMultiServiceFactory > & rSMgr) throw( Exception )
{
    return (cppu::OWeakObject*) new PDFFilter( rSMgr );
}

// -----------------------------------------------------------------------------

OUString SAL_CALL PDFFilter::getImplementationName()
    throw (RuntimeException)
{
    return PDFFilter_getImplementationName();
}

// -----------------------------------------------------------------------------

sal_Bool SAL_CALL PDFFilter::supportsService( const OUString& rServiceName )
    throw (RuntimeException)
{
    return PDFFilter_supportsService( rServiceName );
}

// -----------------------------------------------------------------------------

::com::sun::star::uno::Sequence< OUString > SAL_CALL PDFFilter::getSupportedServiceNames(  ) throw (RuntimeException)
{
    return PDFFilter_getSupportedServiceNames();
}
