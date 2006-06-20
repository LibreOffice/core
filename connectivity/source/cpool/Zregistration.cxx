/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Zregistration.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:07:38 $
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

#include <stdio.h>

#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
#ifndef CONNECTIVITY_POOLCOLLECTION_HXX
#include "ZPoolCollection.hxx"
#endif


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace connectivity;
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
    ::rtl::OUString sMainKeyName = ::rtl::OUString::createFromAscii("/");
    sMainKeyName += OPoolCollection::getImplementationName_Static();
    sMainKeyName += ::rtl::OUString::createFromAscii("/UNO/SERVICES");

    try
    {
        Reference< XRegistryKey > xMainKey = _pRegistryKey->createKey(sMainKeyName);
        if (!xMainKey.is())
            return sal_False;

        Sequence< ::rtl::OUString > sServices = OPoolCollection::getSupportedServiceNames_Static();
        const ::rtl::OUString* pServices = sServices.getConstArray();
        for (sal_Int32 i=0; i<sServices.getLength(); ++i, ++pServices)
            xMainKey->createKey(*pServices);
    }
    catch(InvalidRegistryException&)
    {
        return sal_False;
    }
    catch(InvalidValueException&)
    {
        return sal_False;
    }
    return sal_True;
}

//---------------------------------------------------------------------------------------
void* SAL_CALL component_getFactory(const sal_Char* _pImplName, ::com::sun::star::lang::XMultiServiceFactory* _pServiceManager, void* /*_pRegistryKey*/)
{
    void* pRet = NULL;

    if (OPoolCollection::getImplementationName_Static().compareToAscii(_pImplName) == 0)
    {
        Reference< XSingleServiceFactory > xFactory(
            ::cppu::createOneInstanceFactory(
                _pServiceManager,
                OPoolCollection::getImplementationName_Static(),
                OPoolCollection::CreateInstance,
                OPoolCollection::getSupportedServiceNames_Static()
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


