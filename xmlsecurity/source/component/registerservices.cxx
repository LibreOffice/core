/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: registerservices.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2007-11-07 10:06:31 $
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
#include "precompiled_xmlsecurity.hxx"

#include <tools/debug.hxx>

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif

#include <cppuhelper/factory.hxx>


#include <documentdigitalsignatures.hxx>
#include <certificatecontainer.hxx>

using namespace ::com::sun::star;

extern "C"
{
void SAL_CALL component_getImplementationEnvironment( const sal_Char ** ppEnvTypeName, uno_Environment ** )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

sal_Bool SAL_CALL component_writeInfo( void* /*pServiceManager*/, void* pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
                    sal_Int32 nPos = 0;
                    // SERVICE DocumentDigitalSignatures
                    nPos = 0;
                    uno::Reference< registry::XRegistryKey > xNewKey(
                    reinterpret_cast< registry::XRegistryKey* >( pRegistryKey )->createKey( DocumentDigitalSignatures::GetImplementationName() ) );

                    xNewKey = xNewKey->createKey( rtl::OUString::createFromAscii( "/UNO/SERVICES" ) );

                    const uno::Sequence< rtl::OUString >& rSNL = DocumentDigitalSignatures::GetSupportedServiceNames();
                    const rtl::OUString* pArray = rSNL.getConstArray();
                    for ( nPos = rSNL.getLength(); nPos--; )
                        xNewKey->createKey( pArray[nPos] );

                    // SERVICE CertificateContainer
                    nPos = 0;
                    uno::Reference< registry::XRegistryKey > xNewKeyCertificateContainer(
                    reinterpret_cast< registry::XRegistryKey* >( pRegistryKey )->createKey( CertificateContainer::impl_getStaticImplementationName() ) );
                    xNewKeyCertificateContainer = xNewKeyCertificateContainer->createKey( rtl::OUString::createFromAscii( "/UNO/SERVICES" ) );

                    const uno::Sequence< rtl::OUString >& rSNLCertificateContainer = CertificateContainer::impl_getStaticSupportedServiceNames();
                    const rtl::OUString* pArrayCertificateContainer = rSNLCertificateContainer.getConstArray();
                    for ( nPos = rSNLCertificateContainer.getLength(); nPos--; )
                        xNewKeyCertificateContainer->createKey( pArrayCertificateContainer[nPos] );

                    //-----------------------------

                    return sal_True;
        }
        catch (registry::InvalidRegistryException &)
        {
            DBG_ERROR( "InvalidRegistryException!" );
        }
    }
    return sal_False;
}

void* SAL_CALL component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * /*pRegistryKey*/ )
{
    void* pRet = NULL;

    //Decryptor
        rtl::OUString implName = rtl::OUString::createFromAscii( pImplName );

        // DocumentDigitalSignatures
    if ( pServiceManager && implName.equals( DocumentDigitalSignatures::GetImplementationName() ) )
    {
            uno::Reference< lang::XSingleServiceFactory > xFactory( cppu::createSingleFactory(
            reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ),
            rtl::OUString::createFromAscii( pImplName ),
            DocumentDigitalSignatures_CreateInstance, DocumentDigitalSignatures::GetSupportedServiceNames() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

        // CertificateContainer
        if ( pServiceManager && implName.equals( CertificateContainer::impl_getStaticImplementationName() ))
    {
            uno::Reference< lang::XSingleServiceFactory > xFactoryCertificateContainer( cppu::createOneInstanceFactory(
                reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ),
                rtl::OUString::createFromAscii( pImplName ),
                CertificateContainer::impl_createInstance,
                CertificateContainer::impl_getStaticSupportedServiceNames() ) );

            if (xFactoryCertificateContainer.is())
            {
        xFactoryCertificateContainer->acquire();
        pRet = xFactoryCertificateContainer.get();
            }
    }

    return pRet;
}

}   // extern "C"





