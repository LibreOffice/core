/*************************************************************************
 *
 *  $RCSfile: confproviderimpl2.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: dg $ $Date: 2001-06-14 10:02:27 $
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
#include "confproviderimpl2.hxx"
#include "roottree.hxx"
#include "noderef.hxx"
#include "apitreeaccess.hxx"
#include "objectregistry.hxx"
#include "configexcept.hxx"

#ifndef CONFIGMGR_API_PROVIDER2_HXX_
#include "confprovider2.hxx"
#endif
#ifndef _CONFIGMGR_TREECACHE_HXX_
#include "treecache.hxx"
#endif
#ifndef _CONFIGMGR_TRACER_HXX_
#include "tracer.hxx"
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

namespace configmgr
{
    namespace css   = ::com::sun::star;
    namespace uno   = css::uno;
    namespace beans = css::beans;

    using ::rtl::OUString;

    using configapi::NodeElement;
    using configuration::RootTree;

    //=============================================================================
    //= OConfigurationProviderImpl
    //=============================================================================
    //-----------------------------------------------------------------------------------
    OConfigurationProviderImpl::OConfigurationProviderImpl(OConfigurationProvider* _pProvider, const uno::Reference< lang::XMultiServiceFactory >& _xServiceFactory)
                               :OProviderImpl(_pProvider, _xServiceFactory)
    {
    }
    //--------------------------------------------------------------------------
    uno::Reference<uno::XInterface>  OConfigurationProviderImpl::createReadAccess( uno::Sequence<uno::Any> const& aArgs)
        throw (uno::Exception, uno::RuntimeException)
    {
        CFG_TRACE_INFO("config provider: going to create a read access instance");

        // extract the args
        OUString sPath;
        sal_Int32 nLevels;
        vos::ORef<OOptions> xOptions = new OOptions(getDefaultOptions());

        OProviderImpl::FactoryArguments::extractArgs(aArgs, sPath, nLevels, xOptions);

        CFG_TRACE_INFO_NI("config provider: node accessor extracted from the args is %s", OUSTRING2ASCII(sPath));
        CFG_TRACE_INFO_NI("config provider: level depth extracted from the args is %i", nLevels);

        if (!xOptions->canUseCache()) CFG_TRACE_INFO_NI("config provider: Ignoring cache for request");

        OUString sUser = xOptions->getUser();
        if (sUser.getLength())
        {
            if (xOptions->getDefaultUser() == sUser)
            {
                OSL_ASSERT(xOptions->hasDefaultUser());
                OSL_ASSERT(xOptions->getUser() == sUser);
                // respecified the actual user
                CFG_TRACE_WARNING_NI("config provider: User should not be specified again when creating an Access");
                OSL_ENSURE(false,"config provider: User should not be specified again when creating an Access");
            }
            else if (!xOptions->getDefaultUser().getLength() )
            {
                // have no default user - cannot tell whether the argument is the wrong one
                CFG_TRACE_WARNING_NI("config provider: No user should be specified, when creating an Access - user parameter is ignored");
                OSL_ENSURE(false,"config provider: No user should be specified, when creating an Access - user parameter is ignored");
            }
            else
            {
                // specified a different user
                CFG_TRACE_ERROR_NI("config provider: Cannot access foreign user data");
                throw lang::IllegalArgumentException(OUString::createFromAscii("config provider: Cannot access foreign user data"),this->getProviderInstance(), -1);
            }
            // user should always be the default user!
            xOptions->setUser(xOptions->getDefaultUser());
        }

        // create the access object
        uno::Reference< uno::XInterface > xReturn;
        try
        {
            if (aArgs.getLength() != 0)
            {
                NodeElement* pElement = buildReadAccess(sPath, xOptions, nLevels);
                if (pElement != 0)
                {
                    xReturn = pElement->getUnoInstance();
                    if (xReturn.is())
                        // acquired once by buildReadAccess
                        xReturn->release();
                }
            }
        }
        catch (configuration::Exception& e)
        {
            configapi::ExceptionMapper ec(e);
            ec.setContext(this->getProviderInstance());
            //ec.unhandled();
            throw lang::WrappedTargetException(ec.message(), ec.context(), uno::Any());
        }
        return xReturn;
    }

    //-----------------------------------------------------------------------------------
    uno::Reference<uno::XInterface> OConfigurationProviderImpl::createUpdateAccess( uno::Sequence<uno::Any> const& aArgs)
        throw (uno::Exception, uno::RuntimeException)
    {
        CFG_TRACE_INFO("config provider: going to create an update access instance");

        // extract the args
        OUString sPath;
        sal_Int32 nLevels;
        vos::ORef<OOptions> xOptions = new OOptions(getDefaultOptions());

        OProviderImpl::FactoryArguments::extractArgs(aArgs, sPath, nLevels, xOptions);

        CFG_TRACE_INFO_NI("config provider: node accessor extracted from the args is %s", OUSTRING2ASCII(sPath));
        CFG_TRACE_INFO_NI("config provider: level depth extracted from the args is %i", nLevels);

        if (!xOptions->canUseCache()) CFG_TRACE_INFO_NI("config provider: Ignoring cache for request");

        OUString sUser = xOptions->getUser();
        if (sUser.getLength())
        {
            if (xOptions->getDefaultUser() == sUser)
            {
                OSL_ASSERT(xOptions->hasDefaultUser());
                OSL_ASSERT(xOptions->getUser() == sUser);
                // respecified the actual user
                CFG_TRACE_WARNING_NI("config provider: User should not be specified again when creating an Access");
                OSL_ENSURE(false,"config provider: User should not be specified again when creating an Access");
            }
            else if (!xOptions->getDefaultUser().getLength() )
            {
                // have no default user - cannot tell whether the argument is the wrong one
                CFG_TRACE_WARNING_NI("config provider: No user should be specified, when creating an Access - user parameter is ignored");
                OSL_ENSURE(false,"config provider: No user should be specified, when creating an Access - user parameter is ignored");
            }
            else
            {
                // specified a different user
                CFG_TRACE_ERROR_NI("config provider: Cannot access foreign user data");
                throw lang::IllegalArgumentException(OUString::createFromAscii("config provider: Cannot access foreign user data"),this->getProviderInstance(), -1);
            }
            // user should always be the default user!
            xOptions->setUser(xOptions->getDefaultUser());
        }

        // create the access object
        uno::Reference< uno::XInterface > xReturn;
        try
        {
            if (aArgs.getLength() != 0)
            {
                NodeElement* pElement = buildUpdateAccess(sPath, xOptions, nLevels);
                if (pElement != 0)
                {
                    xReturn = pElement->getUnoInstance();
                    if (xReturn.is())
                        // acquired once by buildReadAccess
                        xReturn->release();
                }
            }
        }
        catch (configuration::Exception& e)
        {
            configapi::ExceptionMapper ec(e);
            ec.setContext(this->getProviderInstance());
            //ec.unhandled();
            throw lang::WrappedTargetException(ec.message(), ec.context(), uno::Any());
        }
        return xReturn;
    }
} // namespace configmgr


