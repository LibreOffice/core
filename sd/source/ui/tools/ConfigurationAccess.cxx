/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <tools/ConfigurationAccess.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <tools/diagnose_ex.h>
#include <sal/log.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

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
        Sequence<Any> aCreationArguments(comphelper::InitAnyPropertySequence(
        {
            {"nodepath", makeAny(rsRootName)},
            {"depth", makeAny(sal_Int32(-1))}
        }));

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
        DBG_UNHANDLED_EXCEPTION("sd.tools");
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
    catch (const Exception&)
    {
        css::uno::Any ex( cppu::getCaughtException() );
        SAL_WARN("sd", "caught exception while getting configuration node" << sPathToNode << ": " << exceptionToString(ex));
    }

    return Any();
}

void ConfigurationAccess::CommitChanges()
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
    if (!rxContainer.is())
        return;

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
            for (size_t nValueIndex=0; nValueIndex<aValues.size(); ++nValueIndex)
                aValues[nValueIndex] = xSetItem->getByName(rArguments[nValueIndex]);
        }
        rFunctor(rsKey, aValues);
    }
}

void ConfigurationAccess::FillList(
    const Reference<container::XNameAccess>& rxContainer,
    const OUString& rsArgument,
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
