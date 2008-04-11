/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: confproviderimpl2.cxx,v $
 * $Revision: 1.33 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

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
#include "treeprovider.hxx"
#include "tracer.hxx"
#include <com/sun/star/beans/PropertyValue.hpp>

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
    OConfigurationProviderImpl::OConfigurationProviderImpl(OConfigurationProvider* _pProvider, CreationContext const & xContext)
                               :OProviderImpl(_pProvider, xContext)
    {
    }
    //--------------------------------------------------------------------------
    uno::Reference<uno::XInterface>  OConfigurationProviderImpl::createReadAccess( uno::Sequence<uno::Any> const& aArgs)
        CFG_UNO_THROW_ALL(  )
    {
        CFG_TRACE_INFO("config provider: going to create a read access instance");

        // extract the args
        OUString sPath;
        sal_Int32 nLevels;

        RequestOptions aOptions = getDefaultOptions();

        OProviderImpl::FactoryArguments::extractArgs(aArgs, sPath, nLevels, aOptions);

        CFG_TRACE_INFO_NI("config provider: node accessor extracted from the args is %s", OUSTRING2ASCII(sPath));
        CFG_TRACE_INFO_NI("config provider: level depth extracted from the args is %i", nLevels);

        // create the access object
        uno::Reference< uno::XInterface > xReturn;

        NodeElement* pElement = buildReadAccess(sPath, aOptions, nLevels);
        if (pElement != 0)
        {
            xReturn = pElement->getUnoInstance();
            if (xReturn.is())
                // acquired once by buildReadAccess
                xReturn->release();
        }

        return xReturn;
    }

    //-----------------------------------------------------------------------------------
    uno::Reference<uno::XInterface> OConfigurationProviderImpl::createUpdateAccess( uno::Sequence<uno::Any> const& aArgs)
        CFG_UNO_THROW_ALL(  )
    {
        CFG_TRACE_INFO("config provider: going to create an update access instance");

        // extract the args
        OUString sPath;
        sal_Int32 nLevels;

        RequestOptions aOptions = getDefaultOptions();

        OProviderImpl::FactoryArguments::extractArgs(aArgs, sPath, nLevels, aOptions);

        CFG_TRACE_INFO_NI("config provider: node accessor extracted from the args is %s", OUSTRING2ASCII(sPath));
        CFG_TRACE_INFO_NI("config provider: level depth extracted from the args is %i", nLevels);

        // create the access object
        uno::Reference< uno::XInterface > xReturn;

        NodeElement* pElement = buildUpdateAccess(sPath, aOptions, nLevels);
        if (pElement != 0)
        {
            xReturn = pElement->getUnoInstance();
            if (xReturn.is())
                // acquired once by buildReadAccess
                xReturn->release();
        }

        return xReturn;
    }
} // namespace configmgr


