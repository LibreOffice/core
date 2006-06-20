/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: services.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 02:48:22 $
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

#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _DBA_REGHELPER_HXX_
#include "dba_reghelper.hxx"
#endif

/********************************************************************************************/

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

//***************************************************************************************
//
// registry functions
extern "C" void SAL_CALL createRegistryInfo_ODatabaseContext();
//  extern "C" void SAL_CALL createRegistryInfo_ODocumentDefinition();
extern "C" void SAL_CALL createRegistryInfo_OCommandDefinition();
extern "C" void SAL_CALL createRegistryInfo_OComponentDefinition();
extern "C" void SAL_CALL createRegistryInfo_ORowSet();
extern "C" void SAL_CALL createRegistryInfo_ODatabaseDocument();
extern "C" void SAL_CALL createRegistryInfo_ODatabaseSource();

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
        //  createRegistryInfo_ODocumentDefinition();
        createRegistryInfo_OCommandDefinition();
        createRegistryInfo_OComponentDefinition();
        createRegistryInfo_ORowSet();
        createRegistryInfo_ODatabaseDocument();
        createRegistryInfo_ODatabaseSource();
        bInit = sal_True;
    }
}

//---------------------------------------------------------------------------------------

extern "C" void SAL_CALL component_getImplementationEnvironment(
                const sal_Char  **ppEnvTypeName,
                uno_Environment **/*ppEnv*/
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
        return ::dbaccess::OModuleRegistration::writeComponentInfos(
            static_cast<XMultiServiceFactory*>(pServiceManager),
            static_cast<XRegistryKey*>(pRegistryKey));
    }
    catch (InvalidRegistryException& )
    {
        OSL_ENSURE(sal_False, "DBA::component_writeInfo : could not create a registry key ! ## InvalidRegistryException !");
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
        xRet = ::dbaccess::OModuleRegistration::getComponentFactory(
            ::rtl::OUString::createFromAscii(pImplementationName),
            static_cast< XMultiServiceFactory* >(pServiceManager));
    }

    if (xRet.is())
        xRet->acquire();
    return xRet.get();
};
