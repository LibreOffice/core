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

#include "PresenterConfigurationAccess.hxx"

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <comphelper/propertysequence.hxx>
#include <osl/diagnose.h>
#include <tools/diagnose_ex.h>
#include <sal/log.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sdext { namespace presenter {

const OUString PresenterConfigurationAccess::msPresenterScreenRootName =
    "/org.openoffice.Office.PresenterScreen/";

PresenterConfigurationAccess::PresenterConfigurationAccess (
    const Reference<XComponentContext>& rxContext,
    const OUString& rsRootName,
    WriteMode eMode)
    : mxRoot(),
      maNode()
{
    try
    {
        if (rxContext.is())
        {
            uno::Sequence<uno::Any> aCreationArguments(comphelper::InitAnyPropertySequence(
            {
                {"nodepath", uno::Any(rsRootName)},
                {"depth", uno::Any(sal_Int32(-1))}
            }));

            OUString sAccessService;
            if (eMode == READ_ONLY)
                sAccessService = "com.sun.star.configuration.ConfigurationAccess";
            else
                sAccessService = "com.sun.star.configuration.ConfigurationUpdateAccess";

            Reference<lang::XMultiServiceFactory> xProvider =
                configuration::theDefaultProvider::get( rxContext );
            mxRoot = xProvider->createInstanceWithArguments(
                sAccessService, aCreationArguments);
            maNode <<= mxRoot;
        }
    }
    catch (const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("sdext.presenter", "caught exception while opening configuration");
    }
}

PresenterConfigurationAccess::~PresenterConfigurationAccess()
{
}

bool PresenterConfigurationAccess::IsValid() const
{
    return mxRoot.is();
}

Any PresenterConfigurationAccess::GetConfigurationNode (const OUString& sPathToNode)
{
    return GetConfigurationNode(
        Reference<container::XHierarchicalNameAccess>(mxRoot, UNO_QUERY),
        sPathToNode);
}

bool PresenterConfigurationAccess::GoToChild (const OUString& rsPathToNode)
{
    if ( ! IsValid())
        return false;

    Reference<container::XHierarchicalNameAccess> xNode (maNode, UNO_QUERY);
    if (xNode.is())
    {
        maNode = GetConfigurationNode(
            Reference<container::XHierarchicalNameAccess>(maNode, UNO_QUERY),
            rsPathToNode);
        if (Reference<XInterface>(maNode, UNO_QUERY).is())
            return true;
    }

    mxRoot = nullptr;
    return false;
}

bool PresenterConfigurationAccess::GoToChild (const Predicate& rPredicate)
{
    if ( ! IsValid())
        return false;

    maNode = Find(Reference<container::XNameAccess>(maNode,UNO_QUERY), rPredicate);
    if (Reference<XInterface>(maNode, UNO_QUERY).is())
        return true;

    mxRoot = nullptr;
    return false;
}

bool PresenterConfigurationAccess::SetProperty (
    const OUString& rsPropertyName,
    const Any& rValue)
{
    Reference<beans::XPropertySet> xProperties (maNode, UNO_QUERY);
    if (xProperties.is())
    {
        xProperties->setPropertyValue(rsPropertyName, rValue);
        return true;
    }
    else
        return false;
}

Any PresenterConfigurationAccess::GetConfigurationNode (
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
        SAL_WARN("sdext.presenter", "caught exception while getting configuration node " << sPathToNode << " : " << exceptionToString(ex));
    }

    return Any();
}

Reference<beans::XPropertySet> PresenterConfigurationAccess::GetNodeProperties (
    const css::uno::Reference<css::container::XHierarchicalNameAccess>& rxNode,
    const OUString& rsPathToNode)
{
    return Reference<beans::XPropertySet>(GetConfigurationNode(rxNode, rsPathToNode), UNO_QUERY);
}

void PresenterConfigurationAccess::CommitChanges()
{
    Reference<util::XChangesBatch> xConfiguration (mxRoot, UNO_QUERY);
    if (xConfiguration.is())
        xConfiguration->commitChanges();
}

void PresenterConfigurationAccess::ForAll (
    const Reference<container::XNameAccess>& rxContainer,
    const ::std::vector<OUString>& rArguments,
    const ItemProcessor& rProcessor)
{
    if (!rxContainer.is())
        return;

    ::std::vector<Any> aValues(rArguments.size());
    Sequence<OUString> aKeys (rxContainer->getElementNames());
    for (sal_Int32 nItemIndex=0; nItemIndex<aKeys.getLength(); ++nItemIndex)
    {
        bool bHasAllValues (true);
        const OUString& rsKey (aKeys[nItemIndex]);
        Reference<container::XNameAccess> xSetItem (rxContainer->getByName(rsKey), UNO_QUERY);
        Reference<beans::XPropertySet> xSet (xSetItem, UNO_QUERY);
        OSL_ASSERT(xSet.is());
        if (xSetItem.is())
        {
            // Get from the current item of the container the children
            // that match the names in the rArguments list.
            for (size_t nValueIndex=0; nValueIndex<aValues.size(); ++nValueIndex)
            {
                if ( ! xSetItem->hasByName(rArguments[nValueIndex]))
                    bHasAllValues = false;
                else
                    aValues[nValueIndex] = xSetItem->getByName(rArguments[nValueIndex]);
            }
        }
        else
            bHasAllValues = false;
        if (bHasAllValues)
            rProcessor(aValues);
    }
}

void PresenterConfigurationAccess::ForAll (
    const Reference<container::XNameAccess>& rxContainer,
    const PropertySetProcessor& rProcessor)
{
    if (rxContainer.is())
    {
        Sequence<OUString> aKeys (rxContainer->getElementNames());
        for (sal_Int32 nItemIndex=0; nItemIndex<aKeys.getLength(); ++nItemIndex)
        {
            const OUString& rsKey (aKeys[nItemIndex]);
            Reference<beans::XPropertySet> xSet (rxContainer->getByName(rsKey), UNO_QUERY);
            if (xSet.is())
                rProcessor(rsKey, xSet);
        }
    }
}

Any PresenterConfigurationAccess::Find (
    const Reference<container::XNameAccess>& rxContainer,
    const Predicate& rPredicate)
{
    if (rxContainer.is())
    {
        Sequence<OUString> aKeys (rxContainer->getElementNames());
        for (sal_Int32 nItemIndex=0; nItemIndex<aKeys.getLength(); ++nItemIndex)
        {
            Reference<beans::XPropertySet> xProperties (
                rxContainer->getByName(aKeys[nItemIndex]),
                UNO_QUERY);
            if (xProperties.is())
                if (rPredicate(aKeys[nItemIndex], xProperties))
                    return Any(xProperties);
        }
    }
    return Any();
}

bool PresenterConfigurationAccess::IsStringPropertyEqual (
    const OUString& rsValue,
    const OUString& rsPropertyName,
    const css::uno::Reference<css::beans::XPropertySet>& rxNode)
{
    OUString sValue;
    if (GetProperty(rxNode, rsPropertyName) >>= sValue)
        return sValue == rsValue;
    else
        return false;
}

Any PresenterConfigurationAccess::GetProperty (
    const Reference<beans::XPropertySet>& rxProperties,
    const OUString& rsKey)
{
    OSL_ASSERT(rxProperties.is());
    if ( ! rxProperties.is())
        return Any();
    try
    {
        Reference<beans::XPropertySetInfo> xInfo (rxProperties->getPropertySetInfo());
        if (xInfo.is())
            if ( ! xInfo->hasPropertyByName(rsKey))
                return Any();
        return rxProperties->getPropertyValue(rsKey);
    }
    catch (beans::UnknownPropertyException&)
    {
    }
    return Any();
}

} } // end of namespace sdext::tools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
