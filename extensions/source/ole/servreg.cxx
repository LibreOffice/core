/*************************************************************************
 *
 *  $RCSfile: servreg.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:16:40 $
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

#include "ole2uno.hxx"

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif



using namespace rtl;
using namespace ole_adapter;


extern "C" void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pRet = 0;

    OUString aImplName( OUString::createFromAscii( pImplName ) );
    Reference< XSingleServiceFactory > xFactory;
    if (pServiceManager && aImplName.equals(  L"com.sun.star.comp.ole.OleConverter2"  ))
    {
        xFactory= o2u_getConverterProvider2(
            reinterpret_cast< XMultiServiceFactory*>(pServiceManager),
            reinterpret_cast< XRegistryKey*>(pRegistryKey));
    }
    else if (pServiceManager && aImplName.equals(  L"com.sun.star.comp.ole.OleConverterVar1"  ))
    {
        xFactory= o2u_getConverterProviderVar1(
            reinterpret_cast< XMultiServiceFactory*>(pServiceManager),
            reinterpret_cast< XRegistryKey*>(pRegistryKey));
    }
    else if(pServiceManager && aImplName.equals(L"com.sun.star.comp.ole.OleClient"))
    {
        xFactory= o2u_getClientProvider(
            reinterpret_cast< XMultiServiceFactory*>(pServiceManager),
            reinterpret_cast< XRegistryKey*>(pRegistryKey));
    }
    else if(pServiceManager && aImplName.equals(L"com.sun.star.comp.ole.OleServer"))
    {
        xFactory= o2u_getServerProvider(
            reinterpret_cast< XMultiServiceFactory*>(pServiceManager),
            reinterpret_cast< XRegistryKey*>(pRegistryKey));
    }

    if (xFactory.is())
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }

    return pRet;
}


extern "C" sal_Bool SAL_CALL component_writeInfo(   void * pServiceManager, void * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            Reference<XRegistryKey> xNewKey =
                reinterpret_cast<XRegistryKey*>( pRegistryKey)->createKey(L"/com.sun.star.comp.ole.OleConverter2/UNO/SERVICES");
            xNewKey->createKey(L"com.sun.star.bridge.OleBridgeSupplier2");

            xNewKey =
                reinterpret_cast<XRegistryKey*>( pRegistryKey)->createKey(L"/com.sun.star.comp.ole.OleConverterVar1/UNO/SERVICES");
            xNewKey->createKey(L"com.sun.star.bridge.OleBridgeSupplierVar1");

            xNewKey =
                reinterpret_cast<XRegistryKey*>( pRegistryKey)->createKey(L"/com.sun.star.comp.ole.OleClient/UNO/SERVICES");
            xNewKey->createKey(L"com.sun.star.bridge.OleObjectFactory");

            xNewKey =
                reinterpret_cast<XRegistryKey*>( pRegistryKey)->createKey(L"/com.sun.star.comp.ole.OleServer/UNO/SERVICES");
            xNewKey->createKey(L"com.sun.star.bridge.OleApplicationRegistration");

            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}

extern "C" void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}


