/*************************************************************************
 *
 *  $RCSfile: ZipEnumeration.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mtg $ $Date: 2000-11-13 13:37:57 $
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
#ifndef _ZIP_ENUMERATION_HXX
#define _ZIP_ENUMERATION_HXX

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx> // helper for implementations
#endif

#ifndef COM_SUN_STAR_CONTAINER_XENUMERATION_HPP
#include <com/sun/star/container/XEnumeration.hpp>
#endif

#ifndef _ZIP_ENTRY_IMPL_HXX_
#include "ZipEntryImpl.hxx"
#endif

class ZipEnumeration : public cppu::WeakImplHelper1 < com::sun::star::container::XEnumeration>
{
private:
    com::sun::star::uno::Sequence< com::sun::star::package::ZipEntry > xZipList;
    sal_uInt16      nCurrent;
public:
    virtual sal_Bool SAL_CALL hasMoreElements() throw (::com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Any SAL_CALL nextElement() throw (::com::sun::star::uno::RuntimeException);
    virtual ~ZipEnumeration(void);
             ZipEnumeration(void);
             ZipEnumeration( com::sun::star::uno::Sequence< com::sun::star::package::ZipEntry > &xList);
             //ZipEnumeration( com::sun::star::uno::Reference < com::sun::star::uno::Sequence< com::sun::star::package::ZipEntry > >&xList);
/*
             ZipEnumeration(const com::sun::star::uno::Sequence<
                                com::sun::star::uno::Reference<
                                com::sun::star::package::ZipEntry> > & seqEntries);
    // XInterface
    virtual com::sun::star::uno::Any SAL_CALL queryInterface( const com::sun::star::uno::Type & aType )
            throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL acquire() throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL release() throw( ::com::sun::star::uno::RuntimeException );
*/

};

#if 0
/**
 * Function to create a new component instance; is needed by factory helper implementation.
  * @param xMgr service manager to if the components needs other component instances
   */
com::sun::star::uno::Reference< com::sun::star::uno::XInterface > ZipEnum_create
 ( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > & xMgr )
{
    return com::sun::star::uno::Reference< com::sun::star::uno::XInterface >( *new ZipEnum( xMgr ) );
}

/**
 * This function returns the name of the used environment.
 * @param ppEnvTypeName name of the environment
 * @param ppEnv could be point to a special environment, this parameter is normally set to null
 */
extern "C" void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

/**
 * This function creates an implementation section in the registry and another subkey
 * for each supported service.
 * @param pServiceManager generic uno interface providing a service manager
 * @param pRegistryKey generic uno interface providing registry key to write
 */
extern "C" sal_Bool SAL_CALL component_writeInfo( void* pServiceManager, void* pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
           com::sun::star::uno::Reference< com::sun::star::registry::XRegistryKey > xNewKey(
              reinterpret_cast< com::sun::star::registry::XRegistryKey * >( pRegistryKey )->createKey(
                 ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/package.ZipEnum/UNO/SERVICES") ) ) );
           xNewKey->createKey( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("package.ZipEnum") ) );
           return sal_True;
        }
        catch ( com::sun::star::registry::InvalidRegistryException& )
        {
            DBG_ERROR( "InvalidRegistryException detected\n");
            return sal_False;
        }
    }
    return sal_False;
}

/**
 * This function is called to get service factories for an implementation.
 * @param pImplName name of implementation
 * @param pServiceManager generic uno interface providing a service manager to instantiate components
 * @param pRegistryKey registry data key to read and write component persistent data
 * @return a component factory (generic uno interface)
 */
extern "C" void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pRet = 0;
    // which implementation is demanded?
    if (pServiceManager && rtl_str_compare( pImplName, "package.ZipEnumeration" ))
    {
        rtl::OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM("package.ZipEnumeration") );
        com::sun::star::uno::Reference< com::sun::star::lang::XSingleServiceFactory > xFactory(
           cppu::createSingleFactory( // helper function from cppuhelper lib
           reinterpret_cast< com::sun::star::lang::XMultiServiceFactory * >( pServiceManager ),
           ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("package.ZipEnumeration") ),
           ZipEnum_create,
           com::sun::star::uno::Sequence< rtl::OUString >( &aServiceName, 1 ) ) );
        if (xFactory.is())
        {
           xFactory->acquire();
           pRet = xFactory.get();
        }
    }
    return pRet;
}

#endif
#endif
