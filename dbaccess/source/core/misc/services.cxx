/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: services.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:06:41 $
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
#include "precompiled_dbaccess.hxx"

#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>
#include "module_dba.hxx"
#include <osl/diagnose.h>
#include "DatabaseDataProvider.hxx"

/********************************************************************************************/

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

//***************************************************************************************
//
// registry functions
extern "C" void SAL_CALL createRegistryInfo_ODatabaseContext();
extern "C" void SAL_CALL createRegistryInfo_OCommandDefinition();
extern "C" void SAL_CALL createRegistryInfo_OComponentDefinition();
extern "C" void SAL_CALL createRegistryInfo_ORowSet();
extern "C" void SAL_CALL createRegistryInfo_ODatabaseDocument();
extern "C" void SAL_CALL createRegistryInfo_ODatabaseSource();
extern "C" void SAL_CALL createRegistryInfo_DataAccessDescriptorFactory();

namespace {
//--------------------------------------------------------------------------
    ::cppu::ImplementationEntry entries[] = {
        { &::dbaccess::DatabaseDataProvider::Create, &::dbaccess::DatabaseDataProvider::getImplementationName_Static, &::dbaccess::DatabaseDataProvider::getSupportedServiceNames_Static,
            &cppu::createSingleComponentFactory, 0, 0 },
        { 0, 0, 0, 0, 0, 0 }
    };
}

//***************************************************************************************
//
// Die vorgeschriebene C-Api muss erfuellt werden!
// Sie besteht aus drei Funktionen, die von dem Modul exportiert werden muessen.
//
extern "C" void SAL_CALL createRegistryInfo_DBA()
{
    static sal_Bool bInit = sal_False;
    if (!bInit)
    {
        createRegistryInfo_ODatabaseContext();
        createRegistryInfo_OCommandDefinition();
        createRegistryInfo_OComponentDefinition();
        createRegistryInfo_ORowSet();
        createRegistryInfo_ODatabaseDocument();
        createRegistryInfo_ODatabaseSource();
        createRegistryInfo_DataAccessDescriptorFactory();
        bInit = sal_True;
    }
}

//---------------------------------------------------------------------------------------

extern "C" void SAL_CALL component_getImplementationEnvironment(
                const sal_Char  **ppEnvTypeName,
                uno_Environment **
            )
{
    createRegistryInfo_DBA();
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
        return ::dba::DbaModule::getInstance().writeComponentInfos(
            static_cast< XMultiServiceFactory* >( pServiceManager ),
            static_cast< XRegistryKey* >( pRegistryKey ) )
            && cppu::component_writeInfoHelper(pServiceManager, pRegistryKey, entries);
    }
    catch (InvalidRegistryException& )
    {
        OSL_ENSURE( false, "DBA::component_writeInfo : could not create a registry key ! ## InvalidRegistryException !" );
    }

    return sal_False;
}

//---------------------------------------------------------------------------------------
extern "C" void* SAL_CALL component_getFactory(
                    const sal_Char* pImplementationName,
                    void* pServiceManager,
                    void* pRegistryKey)
{
    Reference< XInterface > xRet;
    if (pServiceManager && pImplementationName)
    {
        xRet = ::dba::DbaModule::getInstance().getComponentFactory(
            ::rtl::OUString::createFromAscii( pImplementationName ),
            static_cast< XMultiServiceFactory* >( pServiceManager ) );
    }

    if (xRet.is())
        xRet->acquire();
    else
        return cppu::component_getFactoryHelper(
            pImplementationName, pServiceManager, pRegistryKey, entries);
    return xRet.get();
};
