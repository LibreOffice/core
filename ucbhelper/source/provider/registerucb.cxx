/*************************************************************************
 *
 *  $RCSfile: registerucb.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:37 $
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
#include <ucbhelper/registerucb.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_CONTENTPROVIDERSERVICEINFO2_HPP_
#include <com/sun/star/ucb/ContentProviderServiceInfo2.hpp>
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

using namespace com::sun;
using namespace com::sun::star;

//============================================================================
//
//  registerAtUcb
//
//============================================================================

namespace ucb {

void
registerAtUcb(
    uno::Reference< star::ucb::XContentProviderManager > const & rUcb,
    uno::Reference< lang::XMultiServiceFactory > const & rFactory,
    uno::Sequence< star::ucb::ContentProviderServiceInfo2 > const &
        rProviders,
    std::vector< ContentProviderRegistrationInfo > * pResults)
    throw (uno::RuntimeException)
{
    uno::Reference< star::ucb::XContentProviderFactory > xFac;
    try
    {
        xFac = uno::Reference< star::ucb::XContentProviderFactory >(
                    rFactory->createInstance(
                        rtl::OUString::createFromAscii(
                            "com.sun.star.ucb.ContentProviderProxyFactory")),
                    uno::UNO_QUERY);
    }
    catch (uno::RuntimeException const &) { throw; }
    catch (uno::Exception const &) {}

    star::ucb::ContentProviderServiceInfo2 const * pInfo
        = rProviders.getConstArray();
    for (sal_Int32 n = rProviders.getLength(); n > 0; --n)
    {
        uno::Reference< star::ucb::XContentProvider > xProvider;
        if (xFac.is())
        {
            // Try to instantiate proxy for provider.
            xProvider = xFac->createContentProvider(pInfo->Service);
        }

        if (!xProvider.is())
        {
            // Try to instantiate provider directly.
            try
            {
                xProvider = uno::Reference< star::ucb::XContentProvider >(
                                rFactory->createInstance(pInfo->Service),
                                uno::UNO_QUERY);
            }
            catch (uno::RuntimeException const &) { throw; }
            catch (uno::Exception const &) {}
        }

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
                    = xParameterized->registerInstance(pInfo->Scheme,
                                                       pInfo->Arguments,
                                                       pInfo->
                                                           ReplaceExisting);
                    //@@@ if this call replaces an old instance, the commit-
                    // or-rollback code below will not work
            }
            catch (lang::IllegalArgumentException const &) {}

            if (xInstance.is())
                xProvider = xInstance;
        }

        if (rUcb.is() && xProvider.is())
            try
            {
                rUcb->registerContentProvider(xProvider, pInfo->Scheme,
                                              pInfo->ReplaceExisting);
            }
            catch (star::ucb::DuplicateProviderException const &)
            {
                if (xParameterized.is())
                    try
                    {
                        xParameterized->deregisterInstance(pInfo->Scheme,
                                                           pInfo->Arguments);
                    }
                    catch (lang::IllegalArgumentException const &) {}
                xOriginalProvider = 0;
            }
            catch (...)
            {
                if (xParameterized.is())
                    try
                    {
                        xParameterized->deregisterInstance(pInfo->Scheme,
                                                           pInfo->Arguments);
                    }
                    catch (lang::IllegalArgumentException const &) {}
                throw;
            }

        if (pResults)
            pResults->
                push_back(ContentProviderRegistrationInfo(xOriginalProvider,
                                                          pInfo->Arguments,
                                                          pInfo->Scheme));

        ++pInfo;
    }
}

//============================================================================
//
//  deregisterFromUcb
//
//============================================================================

void
deregisterFromUcb(
    uno::Reference< star::ucb::XContentProviderManager > const & rUcb,
    std::vector< ContentProviderRegistrationInfo > const & rProviders)
    throw (uno::RuntimeException)
{
    std::vector< ContentProviderRegistrationInfo >::const_iterator
        aEnd(rProviders.end());
    for (std::vector< ContentProviderRegistrationInfo >::const_iterator
             aIt(rProviders.begin());
         aIt != aEnd; ++aIt)
    {
        uno::Reference< star::ucb::XContentProvider >
            xProvider(aIt->m_xProvider);
        if (xProvider.is())
        {
            uno::Reference< star::ucb::XParameterizedContentProvider >
                xParameterized(xProvider, uno::UNO_QUERY);
            if (xParameterized.is())
            {
                uno::Reference< star::ucb::XContentProvider > xInstance;
                try
                {
                    xInstance
                        = xParameterized->
                              deregisterInstance(aIt->m_aTemplate,
                                                 aIt->m_aArguments);
                }
                catch (lang::IllegalArgumentException const &) {}

                if (xInstance.is())
                    xProvider = xInstance;
            }

            if (rUcb.is() && xProvider.is())
                rUcb->deregisterContentProvider(xProvider, aIt->m_aTemplate);
                    //@@@ if this fails, a roll-back of deregisterInstance()
                    // is missing
        }
    }
}

}

