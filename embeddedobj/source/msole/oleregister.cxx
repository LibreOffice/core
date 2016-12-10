/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_embeddedobj.hxx"


#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/registry/InvalidRegistryException.hpp>
#include <cppuhelper/factory.hxx>

#include "xolefactory.hxx"
#include "xdialogcreator.hxx"

using namespace ::com::sun::star;


extern "C" {

SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment( const sal_Char ** ppEnvTypeName, uno_Environment ** /*ppEnv*/ )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * /*pRegistryKey*/ )
{
    void * pRet = 0;

    ::rtl::OUString aImplName( ::rtl::OUString::createFromAscii( pImplName ) );
    uno::Reference< lang::XSingleServiceFactory > xFactory;

    if ( pServiceManager )
    {
        if ( aImplName.equals( OleEmbeddedObjectFactory::impl_staticGetImplementationName() ) )
        {
            xFactory= ::cppu::createOneInstanceFactory( reinterpret_cast< lang::XMultiServiceFactory*>( pServiceManager ),
                                                OleEmbeddedObjectFactory::impl_staticGetImplementationName(),
                                                OleEmbeddedObjectFactory::impl_staticCreateSelfInstance,
                                                OleEmbeddedObjectFactory::impl_staticGetSupportedServiceNames() );
        }
#ifdef WNT
        // the following service makes sense only on windows
        else if ( aImplName.equals( MSOLEDialogObjectCreator::impl_staticGetImplementationName() ) )
        {
            xFactory= ::cppu::createOneInstanceFactory( reinterpret_cast< lang::XMultiServiceFactory*>( pServiceManager ),
                                                MSOLEDialogObjectCreator::impl_staticGetImplementationName(),
                                                MSOLEDialogObjectCreator::impl_staticCreateSelfInstance,
                                                MSOLEDialogObjectCreator::impl_staticGetSupportedServiceNames() );
        }
#endif
    }

    if ( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }

    return pRet;
}

sal_Bool SAL_CALL component_writeInfo( void * /*pServiceManager*/, void * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            uno::Reference< registry::XRegistryKey > xKey( reinterpret_cast< registry::XRegistryKey* >( pRegistryKey ) );
            uno::Reference< registry::XRegistryKey >  xNewKey;

            xNewKey = xKey->createKey( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/") ) +
                                        OleEmbeddedObjectFactory::impl_staticGetImplementationName() +
                                        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "/UNO/SERVICES") )  );
            uno::Sequence< ::rtl::OUString > rServices = OleEmbeddedObjectFactory::impl_staticGetSupportedServiceNames();
            for( sal_Int32 ind = 0; ind < rServices.getLength(); ind++ )
                xNewKey->createKey( rServices.getConstArray()[ind] );

#ifdef WNT
        // the following service makes sense only on windows
            xNewKey = xKey->createKey( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/") ) +
                                        MSOLEDialogObjectCreator::impl_staticGetImplementationName() +
                                        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "/UNO/SERVICES") )  );
            rServices = MSOLEDialogObjectCreator::impl_staticGetSupportedServiceNames();
            for( sal_Int32 ind = 0; ind < rServices.getLength(); ind++ )
                xNewKey->createKey( rServices.getConstArray()[ind] );
#endif

            return sal_True;
        }
        catch (registry::InvalidRegistryException &)
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}

} // extern "C"

