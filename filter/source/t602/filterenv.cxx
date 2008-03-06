 /*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: filterenv.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 11:39:09 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2004-2007 Milan Broz <mbroz@redhat.com>
 *    Copyright 2007 by Sun Microsystems, Inc.
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

#include <osl/mutex.hxx>
#include <osl/thread.h>
#include <cppuhelper/factory.hxx>

#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include "t602filter.hxx"

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

using namespace T602ImportFilter;

extern "C"
{
//==================================================================================================
void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** /* ppEnv */ )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
sal_Bool SAL_CALL component_writeInfo(
    void * /* pServiceManager */, void * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            sal_Int32 nPos = 0;
            Reference< XRegistryKey > xNewKey(
                reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey( T602ImportFilter_getImplementationName() ) );
            xNewKey = xNewKey->createKey( OUString::createFromAscii( "/UNO/SERVICES" ) );

            const Sequence< OUString > & rSNL = T602ImportFilter_getSupportedServiceNames();
            const OUString * pArray = rSNL.getConstArray();
            for ( nPos = rSNL.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );

            xNewKey = reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey( T602ImportFilterDialog_getImplementationName() );
            xNewKey = xNewKey->createKey( OUString::createFromAscii( "/UNO/SERVICES" ) );

            const Sequence< OUString > & rSNL2 = T602ImportFilterDialog_getSupportedServiceNames();
            pArray = rSNL2.getConstArray();
            for ( nPos = rSNL2.getLength(); nPos--; )
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
    const sal_Char * pImplName, void * pServiceManager, void * /* pRegistryKey */ )
{
    void * pRet = 0;

    OUString implName = OUString::createFromAscii( pImplName );
    if ( pServiceManager && implName.equals(T602ImportFilter_getImplementationName()) )
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString::createFromAscii( pImplName ),
            T602ImportFilter_createInstance, T602ImportFilter_getSupportedServiceNames() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }
    else if ( pServiceManager && implName.equals(T602ImportFilterDialog_getImplementationName()) )
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString::createFromAscii( pImplName ),
            T602ImportFilterDialog_createInstance, T602ImportFilterDialog_getSupportedServiceNames() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }


    return pRet;
}
}
