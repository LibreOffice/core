/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mregistration.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 14:41:48 $
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
#include "precompiled_connectivity.hxx"

#include "mdrivermanager.hxx"

#include <cppuhelper/factory.hxx>

#include <stdio.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

//==========================================================================
//= registration
//==========================================================================
extern "C"
{

//---------------------------------------------------------------------------------------
    void SAL_CALL component_getImplementationEnvironment(const sal_Char** _ppEnvTypeName, uno_Environment** /*_ppEnv*/)
{
    *_ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

//---------------------------------------------------------------------------------------
sal_Bool SAL_CALL component_writeInfo(void* /*_pServiceManager*/, com::sun::star::registry::XRegistryKey* _pRegistryKey)
{


    sal_Bool bReturn = sal_False;

    try
    {
        ::rtl::OUString sMainKeyName = ::rtl::OUString::createFromAscii("/");
        sMainKeyName += ::drivermanager::OSDBCDriverManager::getImplementationName_static();
        sMainKeyName += ::rtl::OUString::createFromAscii("/UNO/SERVICES");
        Reference< XRegistryKey > xMainKey = _pRegistryKey->createKey(sMainKeyName);
        if (xMainKey.is())
        {
            Sequence< ::rtl::OUString > sServices(::drivermanager::OSDBCDriverManager::getSupportedServiceNames_static());
            const ::rtl::OUString* pBegin = sServices.getConstArray();
            const ::rtl::OUString* pEnd = pBegin + sServices.getLength();
            for (;pBegin != pEnd ; ++pBegin)
                xMainKey->createKey(*pBegin);
            bReturn = sal_True;
        }
    }
    catch(InvalidRegistryException&)
    {
    }
    catch(InvalidValueException&)
    {
    }
    return bReturn;
}

//---------------------------------------------------------------------------------------
void* SAL_CALL component_getFactory(const sal_Char* _pImplName, ::com::sun::star::lang::XMultiServiceFactory* _pServiceManager, void* /*_pRegistryKey*/)
{
    void* pRet = NULL;

    if (::drivermanager::OSDBCDriverManager::getImplementationName_static().compareToAscii(_pImplName) == 0)
    {
        Reference< XSingleServiceFactory > xFactory(
            ::cppu::createOneInstanceFactory(
                _pServiceManager,
                ::drivermanager::OSDBCDriverManager::getImplementationName_static(),
                ::drivermanager::OSDBCDriverManager::Create,
                ::drivermanager::OSDBCDriverManager::getSupportedServiceNames_static()
            )
        );
        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}

}   // extern "C"


