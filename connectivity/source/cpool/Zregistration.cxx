/*************************************************************************
 *
 *  $RCSfile: Zregistration.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2001-07-24 06:03:21 $
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
    void SAL_CALL component_getImplementationEnvironment(const sal_Char** _ppEnvTypeName, uno_Environment** _ppEnv)
{
    *_ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

//---------------------------------------------------------------------------------------
sal_Bool SAL_CALL component_writeInfo(void* _pServiceManager, com::sun::star::registry::XRegistryKey* _pRegistryKey)
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


