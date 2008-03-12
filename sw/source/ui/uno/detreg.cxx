/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: detreg.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 12:58:07 $
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
#include "precompiled_sw.hxx"
#include "swdetect.hxx"

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

extern "C" {

SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment(
        const  sal_Char**   ppEnvironmentTypeName,
        uno_Environment**   /*ppEnvironment*/ )
{
    *ppEnvironmentTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME ;
}

SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL component_writeInfo(
    void*   /*pServiceManager*/,
    void*   pRegistryKey    )
{
    Reference< ::registry::XRegistryKey >
            xKey( reinterpret_cast< ::registry::XRegistryKey* >( pRegistryKey ) ) ;

    OUString aDelimiter( RTL_CONSTASCII_USTRINGPARAM("/") );
    OUString aUnoServices( RTL_CONSTASCII_USTRINGPARAM( "/UNO/SERVICES") );

    // Eigentliche Implementierung und ihre Services registrieren
    sal_Int32 i;
    Reference< ::registry::XRegistryKey >  xNewKey;

    xNewKey = xKey->createKey( aDelimiter + SwFilterDetect::impl_getStaticImplementationName() +
                               aUnoServices );

    Sequence< OUString > aServices = SwFilterDetect::impl_getStaticSupportedServiceNames();
    for(i = 0; i < aServices.getLength(); i++ )
        xNewKey->createKey( aServices.getConstArray()[i] );

    return sal_True;
}

SAL_DLLPUBLIC_EXPORT void* SAL_CALL component_getFactory(
    const sal_Char* pImplementationName,
    void* pServiceManager,
    void* /*pRegistryKey*/ )
{
    // Set default return value for this operation - if it failed.
    void* pReturn = NULL ;

    if  (
            ( pImplementationName   !=  NULL ) &&
            ( pServiceManager       !=  NULL )
        )
    {
        // Define variables which are used in following macros.
        Reference< XSingleServiceFactory >   xFactory                                                                                                ;
        Reference< XMultiServiceFactory >    xServiceManager( reinterpret_cast< XMultiServiceFactory* >( pServiceManager ) ) ;

        if( SwFilterDetect::impl_getStaticImplementationName().equalsAscii( pImplementationName ) )
        {
            xFactory = ::cppu::createSingleFactory( xServiceManager,
            SwFilterDetect::impl_getStaticImplementationName(),
            SwFilterDetect::impl_createInstance,
            SwFilterDetect::impl_getStaticSupportedServiceNames() );
        }

        // Factory is valid - service was found.
        if ( xFactory.is() )
        {
            xFactory->acquire();
            pReturn = xFactory.get();
        }
    }

    // Return with result of this operation.
    return pReturn ;
}
} // extern "C"



