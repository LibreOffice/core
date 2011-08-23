/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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


#include <bf_sfx2/sfxuno.hxx>

#include "schdll.hxx"
namespace binfilter {

using namespace ::com::sun::star;

//StarChart document
extern uno::Sequence< ::rtl::OUString > SAL_CALL SchDocument_getSupportedServiceNames() throw();
extern ::rtl::OUString SAL_CALL SchDocument_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SchDocument_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception );

extern "C" {

void SAL_CALL component_getImplementationEnvironment(	const	sal_Char**			ppEnvironmentTypeName	,
                                                                uno_Environment**	ppEnvironment			)
{
    *ppEnvironmentTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME ;
}

sal_Bool SAL_CALL component_writeInfo(	void*	pServiceManager	,
                                        void*	pRegistryKey	)
{
    uno::Reference< registry::XRegistryKey >
            xKey( reinterpret_cast< registry::XRegistryKey* >( pRegistryKey ) ) ;

    sal_Int32 i;
    uno::Reference< registry::XRegistryKey >  xNewKey;

    xNewKey = xKey->createKey(::rtl::OUString(
    RTL_CONSTASCII_USTRINGPARAM("/") ) + SchDocument_getImplementationName() +
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "/UNO/SERVICES") )  );

    uno::Sequence< ::rtl::OUString > rServices = SchDocument_getSupportedServiceNames();
    for(i = 0; i < rServices.getLength(); i++ )
        xNewKey->createKey( rServices.getConstArray()[i]);

    return sal_True;
}

void* SAL_CALL component_getFactory(	const	sal_Char*	pImplementationName	,
                                                void*		pServiceManager		,
                                                void*		pRegistryKey		)
{
    void* pReturn = NULL ;

    if	( ( pImplementationName	!=	NULL ) && ( pServiceManager		!=	NULL ) )
    {
        uno::Reference< lang::XSingleServiceFactory > xFactory;
        uno::Reference< lang::XMultiServiceFactory > xServiceManager( 
                    reinterpret_cast< lang::XMultiServiceFactory* >( pServiceManager ) );

        if( SchDocument_getImplementationName().equalsAsciiL(
            pImplementationName, strlen(pImplementationName)) )
        {
            xFactory = ::cppu::createSingleFactory( xServiceManager,
            SchDocument_getImplementationName(),
            SchDocument_createInstance,
            SchDocument_getSupportedServiceNames() );
        }


        if ( xFactory.is() )
        {
            xFactory->acquire();
            pReturn = xFactory.get();
        }
    }

    return pReturn ;
}
} // extern "C"



}
