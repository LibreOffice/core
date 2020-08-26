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

#include "ConfigurationClassifier.hxx"

#include <framework/FrameworkHelper.hxx>
#include <com/sun/star/drawing/framework/XConfiguration.hpp>
#include <sal/log.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

namespace sd::framework {

ConfigurationClassifier::ConfigurationClassifier (
    const Reference<XConfiguration>& rxConfiguration1,
    const Reference<XConfiguration>& rxConfiguration2)
    : mxConfiguration1(rxConfiguration1),
      mxConfiguration2(rxConfiguration2),
      maC1minusC2(),
      maC2minusC1()
{
}

bool ConfigurationClassifier::Partition()
{
    maC1minusC2.clear();
    maC2minusC1.clear();

    PartitionResources(
        mxConfiguration1->getResources(nullptr, OUString(), AnchorBindingMode_DIRECT),
        mxConfiguration2->getResources(nullptr, OUString(), AnchorBindingMode_DIRECT));

    return !maC1minusC2.empty() || !maC2minusC1.empty();
}

void ConfigurationClassifier::PartitionResources (
    const css::uno::Sequence<Reference<XResourceId> >& rS1,
    const css::uno::Sequence<Reference<XResourceId> >& rS2)
{
    ResourceIdVector aC1minusC2;
    ResourceIdVector aC2minusC1;
    ResourceIdVector aC1andC2;

    // Classify the resources in the configurations that are not bound to
    // other resources.
    ClassifyResources(
        rS1,
        rS2,
        aC1minusC2,
        aC2minusC1,
        aC1andC2);

    SAL_INFO("sd.fwk", __func__ << ": copying resource ids to C1-C2");
    CopyResources(aC1minusC2, mxConfiguration1, maC1minusC2);
    SAL_INFO("sd.fwk", __func__ << ": copying resource ids to C2-C1");
    CopyResources(aC2minusC1, mxConfiguration2, maC2minusC1);

    // Process the unique resources that belong to both configurations.
    for (const auto& rxResource : aC1andC2)
    {
        PartitionResources(
            mxConfiguration1->getResources(rxResource, OUString(), AnchorBindingMode_DIRECT),
            mxConfiguration2->getResources(rxResource, OUString(), AnchorBindingMode_DIRECT));
    }
}

void ConfigurationClassifier::ClassifyResources (
    const css::uno::Sequence<Reference<XResourceId> >& rS1,
    const css::uno::Sequence<Reference<XResourceId> >& rS2,
    ResourceIdVector& rS1minusS2,
    ResourceIdVector& rS2minusS1,
    ResourceIdVector& rS1andS2)
{
    // Find all elements in rS1 and place them in rS1minusS2 or rS1andS2
    // depending on whether they are in rS2 or not.
    for (const Reference<XResourceId>& rA1 : rS1)
    {
        bool bFound = std::any_of(rS2.begin(), rS2.end(),
            [&rA1](const Reference<XResourceId>& rA2) {
                return rA1->getResourceURL() == rA2->getResourceURL(); });

        if (bFound)
            rS1andS2.push_back(rA1);
        else
            rS1minusS2.push_back(rA1);
    }

    // Find all elements in rS2 that are not in rS1.  The elements that are
    // in both rS1 and rS2 have been handled above and are therefore ignored
    // here.
    for (const Reference<XResourceId>& rA2 : rS2)
    {
        bool bFound = std::any_of(rS1.begin(), rS1.end(),
            [&rA2](const Reference<XResourceId>& rA1) {
                return rA2->getResourceURL() == rA1->getResourceURL(); });

        if ( ! bFound)
            rS2minusS1.push_back(rA2);
    }
}

void ConfigurationClassifier::CopyResources (
    const ResourceIdVector& rSource,
    const Reference<XConfiguration>& rxConfiguration,
    ResourceIdVector& rTarget)
{
    // Copy all resources bound to the ones in aC1minusC2Unique to rC1minusC2.
    for (const auto& rxResource : rSource)
    {
        const Sequence<Reference<XResourceId> > aBoundResources (
            rxConfiguration->getResources(
                rxResource,
                OUString(),
                AnchorBindingMode_INDIRECT));
        const sal_Int32 nL (aBoundResources.getLength());

        rTarget.reserve(rTarget.size() + 1 + nL);
        rTarget.push_back(rxResource);

        SAL_INFO("sd.fwk", __func__ << ":    copying " <<
            FrameworkHelper::ResourceIdToString(rxResource));

        for (const Reference<XResourceId>& rBoundResource : aBoundResources)
        {
            rTarget.push_back(rBoundResource);
            SAL_INFO("sd.fwk", __func__ << ":    copying " <<
                FrameworkHelper::ResourceIdToString(rBoundResource));
        }
    }
}

#if DEBUG_SD_CONFIGURATION_TRACE

void ConfigurationClassifier::TraceResourceIdVector (
    const char* pMessage,
    const ResourceIdVector& rResources)
{

    SAL_INFO("sd.fwk", __func__ << ": " << pMessage);
    for (const auto& rxResource : rResources)
    {
        OUString sResource (FrameworkHelper::ResourceIdToString(rxResource));
        SAL_INFO("sd.fwk", __func__ << ": " << sResource);
    }
}

#endif

} // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
