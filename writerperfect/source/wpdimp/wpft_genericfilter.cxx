/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* genericfilter: mostly generic code for registering the filter
 *
 * Portions of this code Copyright 2000 by Sun Microsystems, Inc.
 * Rest is Copyright (C) 2002 William Lachance (wlach@interlog.com)
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

/* "This product is not manufactured, approved, or supported by 
 * Corel Corporation or Corel Corporation Limited."
 */
#include <stdio.h>

#include <osl/mutex.hxx>
#include <osl/thread.h>
#include <cppuhelper/factory.hxx>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include "WordPerfectImportFilter.hxx"

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

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
                reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey( WordPerfectImportFilter_getImplementationName() ) ); 
            xNewKey = xNewKey->createKey( OUString::createFromAscii( "/UNO/SERVICES" ) );
            
            const Sequence< OUString > & rSNL = WordPerfectImportFilter_getSupportedServiceNames();
            const OUString * pArray = rSNL.getConstArray();
            for ( nPos = rSNL.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );
#if 0
            xNewKey = reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey( WordPerfectImportFilterDialog_getImplementationName() );
            xNewKey = xNewKey->createKey( OUString::createFromAscii( "/UNO/SERVICES" ) );

            const Sequence< OUString > & rSNL2 = WordPerfectImportFilterDialog_getSupportedServiceNames();
            pArray = rSNL2.getConstArray();
            for ( nPos = rSNL2.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );
#endif
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
    if ( pServiceManager && implName.equals(WordPerfectImportFilter_getImplementationName()) )
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString::createFromAscii( pImplName ),
            WordPerfectImportFilter_createInstance, WordPerfectImportFilter_getSupportedServiceNames() ) );
        
        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }
#if 0
    else if ( pServiceManager && implName.equals(WordPerfectImportFilterDialog_getImplementationName()) )
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString::createFromAscii( pImplName ),
            WordPerfectImportFilterDialog_createInstance, WordPerfectImportFilterDialog_getSupportedServiceNames() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }
#endif
    
    return pRet;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
