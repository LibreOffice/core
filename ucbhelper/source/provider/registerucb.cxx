/*************************************************************************
 *
 *  $RCSfile: registerucb.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: sb $ $Date: 2000-11-09 13:23:55 $
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

#ifndef _UCBHELPER_REGISTERUCB_HXX_
#include <registerucb.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTPROVIDERMANAGER_HPP_
#include <com/sun/star/ucb/XContentProviderManager.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XPARAMETERIZEDCONTENTPROVIDER_HPP_
#include <com/sun/star/ucb/XParameterizedContentProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTPROVIDERFACTORY_HPP_
#include <com/sun/star/ucb/XContentProviderFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_
#include <com/sun/star/uno/RuntimeException.hpp>
#endif
#ifndef _VOS_DIAGNOSE_H_
#include <vos/diagnose.hxx>
#endif

#ifndef _UCBHELPER_CONFIGUREUCB_HXX_
#include <ucbhelper/configureucb.hxx>
#endif

using namespace com::sun;
using namespace com::sun::star;

namespace ucb {

//============================================================================
//
//  registerAtUcb
//
//============================================================================

void
registerAtUcb(
    uno::Reference< star::ucb::XContentProviderManager > const & rManager,
    uno::Reference< lang::XMultiServiceFactory > const & rServiceFactory,
    rtl::OUString const & rName,
    rtl::OUString const & rArguments,
    rtl::OUString const & rTemplate,
    ContentProviderRegistrationInfo * pInfo)
    throw (uno::RuntimeException)
{
    VOS_ENSURE(rServiceFactory.is(),
               "ucb::registerAtUcb(): No service factory");

    uno::Reference< star::ucb::XContentProviderFactory > xProxyFactory;
    try
    {
        xProxyFactory
            = uno::Reference< star::ucb::XContentProviderFactory >(
                  rServiceFactory->
                      createInstance(
                          rtl::OUString::createFromAscii(
                              "com.sun.star.ucb."
                                  "ContentProviderProxyFactory")),
                  uno::UNO_QUERY);
    }
    catch (uno::RuntimeException const &) { throw; }
    catch (uno::Exception const &) {}

    // First, try to instantiate proxy for provider:
    uno::Reference< star::ucb::XContentProvider > xProvider;
    if (xProxyFactory.is())
        xProvider = xProxyFactory->createContentProvider(rName);

    // Then, try to instantiate provider directly:
    if (!xProvider.is())
        try
        {
            xProvider = uno::Reference< star::ucb::XContentProvider >(
                            rServiceFactory->createInstance(rName),
                            uno::UNO_QUERY);
        }
        catch (uno::RuntimeException const &) { throw; }
        catch (uno::Exception const &) {}

    uno::Reference< star::ucb::XContentProvider >
        xOriginalProvider(xProvider);
    uno::Reference< star::ucb::XParameterizedContentProvider >
        xParameterized(xProvider, uno::UNO_QUERY);
    if (xParameterized.is())
    {
        uno::Reference< star::ucb::XContentProvider > xInstance;
        try
        {
            xInstance
                = xParameterized->registerInstance(rTemplate,
                                                   rArguments,
                                                   true);
                //@@@ if this call replaces an old instance, the commit-or-
                // rollback code below will not work
        }
        catch (lang::IllegalArgumentException const &) {}

        if (xInstance.is())
            xProvider = xInstance;
    }

    if (rManager.is() && xProvider.is())
        try
        {
            rManager->registerContentProvider(xProvider, rTemplate, true);
        }
        catch (star::ucb::DuplicateProviderException const &)
        {
            if (xParameterized.is())
                try
                {
                    xParameterized->deregisterInstance(rTemplate, rArguments);
                }
                catch (lang::IllegalArgumentException const &) {}
            xOriginalProvider = 0;
        }
        catch (...)
        {
            if (xParameterized.is())
                try
                {
                    xParameterized->deregisterInstance(rTemplate, rArguments);
                }
                catch (lang::IllegalArgumentException const &) {}
            throw;
        }

    if (pInfo)
    {
        pInfo->m_xProvider = xOriginalProvider;
        pInfo->m_aArguments = rArguments;
        pInfo->m_aTemplate = rTemplate;
    }
}

//============================================================================
//
//  deregisterFromUcb
//
//============================================================================

void
deregisterFromUcb(
    uno::Reference< star::ucb::XContentProviderManager > const & rManager,
    ContentProviderRegistrationInfo const & rInfo)
    throw (uno::RuntimeException)
{
    uno::Reference< star::ucb::XContentProvider >
        xProvider(rInfo.m_xProvider);
    uno::Reference< star::ucb::XParameterizedContentProvider >
        xParameterized(xProvider, uno::UNO_QUERY);
    if (xParameterized.is())
    {
        uno::Reference< star::ucb::XContentProvider > xInstance;
        try
        {
            xInstance
                = xParameterized->deregisterInstance(rInfo.m_aTemplate,
                                                     rInfo.m_aArguments);
        }
        catch (lang::IllegalArgumentException const &) {}

        if (xInstance.is())
            xProvider = xInstance;
    }

    if (rManager.is() && xProvider.is())
        rManager->deregisterContentProvider(xProvider, rInfo.m_aTemplate);
            //@@@ if this fails, a roll-back of deregisterInstance() is
            // missing
}

}
