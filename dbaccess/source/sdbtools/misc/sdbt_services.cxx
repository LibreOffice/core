/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdbt_services.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 15:21:16 $
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

#ifndef DBACCESS_MODULE_SDBT_HXX
#include "module_sdbt.hxx"
#endif

/** === begin UNO using === **/
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::registry::XRegistryKey;
using ::com::sun::star::registry::InvalidRegistryException;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::XInterface;
/** === end UNO using === **/

extern "C" void SAL_CALL createRegistryInfo_ConnectionTools();

//---------------------------------------------------------------------------------------

extern "C" void SAL_CALL sdbt_initializeModule()
{
    static sal_Bool s_bInit = sal_False;
    if (!s_bInit)
    {
        createRegistryInfo_ConnectionTools();
        s_bInit = sal_True;
    }
}

//---------------------------------------------------------------------------------------

extern "C" void SAL_CALL component_getImplementationEnvironment(
                const sal_Char  **ppEnvTypeName,
                uno_Environment **/*ppEnv*/
            )
{
    sdbt_initializeModule();
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

//---------------------------------------------------------------------------------------
extern "C" sal_Bool SAL_CALL component_writeInfo(
                void* pServiceManager,
                void* pRegistryKey
            )
{
    if (pRegistryKey)
    try
    {
        return ::sdbtools::SdbtModule::getInstance().writeComponentInfos(
            static_cast<XMultiServiceFactory*>(pServiceManager),
            static_cast<XRegistryKey*>(pRegistryKey));
    }
    catch (const InvalidRegistryException& )
    {
        OSL_ASSERT("sdbt::component_writeInfo: could not create a registry key (InvalidRegistryException) !");
    }

    return sal_False;
}

//---------------------------------------------------------------------------------------
extern "C" void* SAL_CALL component_getFactory(
                    const sal_Char* pImplementationName,
                    void* pServiceManager,
                    void* /*pRegistryKey*/)
{
    Reference< XInterface > xRet;
    if (pServiceManager && pImplementationName)
    {
        xRet = ::sdbtools::SdbtModule::getInstance().getComponentFactory(
            ::rtl::OUString::createFromAscii(pImplementationName),
            static_cast< XMultiServiceFactory* >(pServiceManager));
    }

    if (xRet.is())
        xRet->acquire();
    return xRet.get();
};

