/*************************************************************************
 *
 *  $RCSfile: services.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kso $ $Date: 2001-04-05 09:52:00 $
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

#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif
#ifndef _CPPU_MACROS_HXX_
#include <cppu/macros.hxx>
#endif
#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _UNO_ENVIRONMENT_H_
#include <uno/environment.h>
#endif

#ifndef UUI_IAHNDL_HXX
#include <iahndl.hxx>
#endif

using namespace com::sun::star;

//============================================================================
//
//  component_getImplementationEnvironment
//
//============================================================================

extern "C" void SAL_CALL
component_getImplementationEnvironment(sal_Char const ** pEnvTypeName,
                                       uno_Environment **)
{
    *pEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

//============================================================================
//
//  component_getDescription
//
//============================================================================

extern "C" sal_Char const * SAL_CALL component_getDescription()
{
    return "UCB User Interface Interaction Handler";
}

//============================================================================
//
//  component_writeInfo
//
//============================================================================

extern "C" sal_Bool SAL_CALL component_writeInfo(void *, void * pRegistryKey)
{
    bool bSuccess = pRegistryKey != 0;
    uno::Reference< registry::XRegistryKey > xKey;
    if (bSuccess)
    {
        rtl::OUString aKeyName(rtl::OUString::createFromAscii("/"));
        aKeyName += rtl::OUString::createFromAscii(
                        UUIInteractionHandler::m_aImplementationName);
        aKeyName += rtl::OUString::createFromAscii("/UNO/SERVICES");
        try
        {
            xKey = static_cast< registry::XRegistryKey * >(pRegistryKey)->
                       createKey(aKeyName);
        }
        catch (registry::InvalidRegistryException &) {}
        bSuccess = xKey.is() != false;
    }
    if (bSuccess)
    {
        uno::Sequence< rtl::OUString >
            aServiceNames(
                UUIInteractionHandler::getSupportedServiceNames_static());
        for (sal_Int32 i = 0; i < aServiceNames.getLength(); ++i)
            try
            {
                xKey->createKey(aServiceNames[i]);
            }
            catch (registry::InvalidRegistryException &)
            {
                bSuccess = false;
                break;
            }
    }
    return bSuccess;
}

//============================================================================
//
//  component_getFactory
//
//============================================================================

extern "C" void * SAL_CALL component_getFactory(sal_Char const * pImplName,
                                                void * pServiceManager,
                                                void *)
{
    void * pFactory = 0;
    if (pServiceManager
        && (rtl_str_compare(pImplName,
                           UUIInteractionHandler::m_aImplementationName)
               == 0)
            ||
           // Backward compatibility... :-/
           (rtl_str_compare(pImplName, "UUIInteractionHandler") == 0))
    {
        uno::Reference< lang::XSingleServiceFactory >
            xTheFactory(
                cppu::createOneInstanceFactory(
                    static_cast< lang::XMultiServiceFactory * >(
                        pServiceManager),
                    rtl::OUString::createFromAscii(
                        UUIInteractionHandler::m_aImplementationName),
                    &UUIInteractionHandler::createInstance,
                   UUIInteractionHandler::getSupportedServiceNames_static()));
        if (xTheFactory.is())
        {
            xTheFactory->acquire();
            pFactory = xTheFactory.get();
        }
    }
    return pFactory;
}

