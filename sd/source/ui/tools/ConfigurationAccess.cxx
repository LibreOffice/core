/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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


#include "tools/ConfigurationAccess.hxx"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <comphelper/processfactory.hxx>
#include <tools/diagnose_ex.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;

namespace sd { namespace tools {

ConfigurationAccess::ConfigurationAccess (
    const Reference<XComponentContext>& rxContext,
    const OUString& rsRootName,
    const WriteMode eMode)
    : mxRoot()
{
    Reference<lang::XMultiServiceFactory> xProvider =
           configuration::theDefaultProvider::get( rxContext );
    Initialize(xProvider, rsRootName, eMode);
}




ConfigurationAccess::ConfigurationAccess (
    const OUString& rsRootName,
    const WriteMode eMode)
    : mxRoot()
{
    Reference<lang::XMultiServiceFactory> xProvider =
        configuration::theDefaultProvider::get( ::comphelper::getProcessComponentContext() );
    Initialize(xProvider, rsRootName, eMode);
}




void ConfigurationAccess::Initialize (
    const Reference<lang::XMultiServiceFactory>& rxProvider,
    const OUString& rsRootName,
    const WriteMode eMode)
{
    try
    {
        Sequence<Any> aCreationArguments(3);
        aCreationArguments[0] = makeAny(beans::PropertyValue(
            "nodepath",
            0,
            makeAny(rsRootName),
            beans::PropertyState_DIRECT_VALUE));
        aCreationArguments[1] = makeAny(beans::PropertyValue(
            "depth",
            0,
            makeAny((sal_Int32)-1),
            beans::PropertyState_DIRECT_VALUE));
        aCreationArguments[2] = makeAny(beans::PropertyValue(
            "lazywrite",
            0,
            makeAny(true),
            beans::PropertyState_DIRECT_VALUE));
        OUString sAccessService;
        if (eMode == READ_ONLY)
            sAccessService = "com.sun.star.configuration.ConfigurationAccess";
        else
            sAccessService = "com.sun.star.configuration.ConfigurationUpdateAccess";

        mxRoot = rxProvider->createInstanceWithArguments(
            sAccessService,
            aCreationArguments);
    }
    catch (Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}




Any ConfigurationAccess::GetConfigurationNode (
    const OUString& sPathToNode)
{
    return GetConfigurationNode(
        Reference<container::XHierarchicalNameAccess>(mxRoot, UNO_QUERY),
        sPathToNode);
}




Any ConfigurationAccess::GetConfigurationNode (
    const css::uno::Reference<css::container::XHierarchicalNameAccess>& rxNode,
    const OUString& sPathToNode)
{
    if (sPathToNode.isEmpty())
        return Any(rxNode);

    try
    {
        if (rxNode.is())
        {
            return rxNode->getByHierarchicalName(sPathToNode);
        }
    }
    catch (const Exception& rException)
    {
        OSL_TRACE ("caught exception while getting configuration node %s: %s",
            ::rtl::OUStringToOString(sPathToNode, RTL_TEXTENCODING_UTF8).getStr(),
            ::rtl::OUStringToOString(rException.Message, RTL_TEXTENCODING_UTF8).getStr());
    }

    return Any();
}




void ConfigurationAccess::CommitChanges (void)
{
    Reference<util::XChangesBatch> xConfiguration (mxRoot, UNO_QUERY);
    if (xConfiguration.is())
        xConfiguration->commitChanges();
}




void ConfigurationAccess::ForAll (
    const Reference<container::XNameAccess>& rxContainer,
    const ::std::vector<OUString>& rArguments,
    const Functor& rFunctor)
{
    if (rxContainer.is())
    {
        ::std::vector<Any> aValues(rArguments.size());
        Sequence<OUString> aKeys (rxContainer->getElementNames());
        for (sal_Int32 nItemIndex=0; nItemIndex < aKeys.getLength(); ++nItemIndex)
        {
            const OUString& rsKey (aKeys[nItemIndex]);
            Reference<container::XNameAccess> xSetItem (rxContainer->getByName(rsKey), UNO_QUERY);
            if (xSetItem.is())
            {
                // Get from the current item of the container the children
                // that match the names in the rArguments list.
                for (sal_uInt32 nValueIndex=0; nValueIndex<aValues.size(); ++nValueIndex)
                    aValues[nValueIndex] = xSetItem->getByName(rArguments[nValueIndex]);
            }
            rFunctor(rsKey, aValues);
        }
    }
}




void ConfigurationAccess::FillList(
    const Reference<container::XNameAccess>& rxContainer,
    const ::rtl::OUString& rsArgument,
    ::std::vector<OUString>& rList)
{
    try
    {
        if (rxContainer.is())
        {
            Sequence<OUString> aKeys (rxContainer->getElementNames());
            rList.resize(aKeys.getLength());
            for (sal_Int32 nItemIndex=0; nItemIndex<aKeys.getLength(); ++nItemIndex)
            {
                Reference<container::XNameAccess> xSetItem (
                    rxContainer->getByName(aKeys[nItemIndex]), UNO_QUERY);
                if (xSetItem.is())
                {
                    xSetItem->getByName(rsArgument) >>= rList[nItemIndex];
                }
            }
        }
    }
    catch (RuntimeException&)
    {}
}


} } // end of namespace sd::tools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
