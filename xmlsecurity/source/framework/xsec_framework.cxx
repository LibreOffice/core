/*************************************************************************
 *
 *  $RCSfile: xsec_framework.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: mt $ $Date: 2004-07-12 13:15:23 $
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

#include <stdio.h>

#include <osl/mutex.hxx>
#include <osl/thread.h>
#include <cppuhelper/factory.hxx>

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif

#include <decryptorimpl.hxx>
#include <encryptorimpl.hxx>
#include <signaturecreatorimpl.hxx>
#include <signatureverifierimpl.hxx>
#include <saxeventkeeperimpl.hxx>
#include <xmlencryptiontemplateimpl.hxx>
#include <xmlsignaturetemplateimpl.hxx>

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

extern "C"
{
//==================================================================================================
void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

//==================================================================================================
sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            //Decryptor
            sal_Int32 nPos = 0;
            Reference< XRegistryKey > xNewKey(
                reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey( DecryptorImpl_getImplementationName() ) );
            xNewKey = xNewKey->createKey( OUString::createFromAscii( "/UNO/SERVICES" ) );

            const Sequence< OUString > & rSNL = DecryptorImpl_getSupportedServiceNames();
            const OUString * pArray = rSNL.getConstArray();
            for ( nPos = rSNL.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );

            //Encryptor
            xNewKey = reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey( EncryptorImpl_getImplementationName() );
            xNewKey = xNewKey->createKey( OUString::createFromAscii( "/UNO/SERVICES" ) );
            const Sequence< OUString > & rSNL2 = EncryptorImpl_getSupportedServiceNames();
            pArray = rSNL2.getConstArray();
            for ( nPos = rSNL2.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );

            //SignatureCreator
            xNewKey = reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey( SignatureCreatorImpl_getImplementationName() );
            xNewKey = xNewKey->createKey( OUString::createFromAscii( "/UNO/SERVICES" ) );
            const Sequence< OUString > & rSNL3 = SignatureCreatorImpl_getSupportedServiceNames();
            pArray = rSNL3.getConstArray();
            for ( nPos = rSNL3.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );

            //SignatureVerifier
            xNewKey = reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey( SignatureVerifierImpl_getImplementationName() );
            xNewKey = xNewKey->createKey( OUString::createFromAscii( "/UNO/SERVICES" ) );
            const Sequence< OUString > & rSNL4 = SignatureVerifierImpl_getSupportedServiceNames();
            pArray = rSNL4.getConstArray();
            for ( nPos = rSNL4.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );

            //SAXEventKeeper
            xNewKey = reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey( SAXEventKeeperImpl_getImplementationName() );
            xNewKey = xNewKey->createKey( OUString::createFromAscii( "/UNO/SERVICES" ) );
            const Sequence< OUString > & rSNL5 = SAXEventKeeperImpl_getSupportedServiceNames();
            pArray = rSNL5.getConstArray();
            for ( nPos = rSNL5.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );

            //XMLSignatureTemplateImpl
            xNewKey = reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey( XMLSignatureTemplateImpl::impl_getImplementationName() );
            xNewKey = xNewKey->createKey( OUString::createFromAscii( "/UNO/SERVICES" ) );
            const Sequence< OUString > & rSNL6 = XMLSignatureTemplateImpl::impl_getSupportedServiceNames();
            pArray = rSNL6.getConstArray();
            for ( nPos = rSNL6.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );

            // XMLEncryptionTemplateImpl
            xNewKey = reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey( XMLEncryptionTemplateImpl::impl_getImplementationName() );
            xNewKey = xNewKey->createKey( OUString::createFromAscii( "/UNO/SERVICES" ) );
            const Sequence< OUString > & rSNL7 = XMLEncryptionTemplateImpl::impl_getSupportedServiceNames();
            pArray = rSNL7.getConstArray();
            for ( nPos = rSNL7.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );

            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}

//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pRet = 0;

    //Decryptor
    OUString implName = OUString::createFromAscii( pImplName );
    if ( pServiceManager && implName.equals(DecryptorImpl_getImplementationName()) )
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString::createFromAscii( pImplName ),
            DecryptorImpl_createInstance, DecryptorImpl_getSupportedServiceNames() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    //Encryptor
    if ( pServiceManager && implName.equals(EncryptorImpl_getImplementationName()) )
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString::createFromAscii( pImplName ),
            EncryptorImpl_createInstance, EncryptorImpl_getSupportedServiceNames() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    //SignatureCreator
    if ( pServiceManager && implName.equals(SignatureCreatorImpl_getImplementationName()) )
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString::createFromAscii( pImplName ),
            SignatureCreatorImpl_createInstance, SignatureCreatorImpl_getSupportedServiceNames() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    //SignatureVerifier
    if ( pServiceManager && implName.equals(SignatureVerifierImpl_getImplementationName()) )
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString::createFromAscii( pImplName ),
            SignatureVerifierImpl_createInstance, SignatureVerifierImpl_getSupportedServiceNames() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    //SAXEventKeeper
    if ( pServiceManager && implName.equals(SAXEventKeeperImpl_getImplementationName()) )
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString::createFromAscii( pImplName ),
            SAXEventKeeperImpl_createInstance, SAXEventKeeperImpl_getSupportedServiceNames() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    //XMLSignatureTemplate
    if ( pServiceManager && implName.equals( XMLSignatureTemplateImpl::impl_getImplementationName()) )
    {
        Reference< XSingleServiceFactory > xFactory = XMLSignatureTemplateImpl::impl_createFactory(
                reinterpret_cast< XMultiServiceFactory* >( pServiceManager ) ) ;

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    //XMLEncryptionTemplate
    if ( pServiceManager && implName.equals( XMLEncryptionTemplateImpl::impl_getImplementationName()) )
    {
        Reference< XSingleServiceFactory > xFactory = XMLEncryptionTemplateImpl::impl_createFactory(
                reinterpret_cast< XMultiServiceFactory* >( pServiceManager ) ) ;

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}
}


