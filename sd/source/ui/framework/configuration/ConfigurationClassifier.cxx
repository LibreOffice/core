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

#include "framework/FrameworkHelper.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

namespace sd { namespace framework {

ConfigurationClassifier::ConfigurationClassifier (
    const Reference<XConfiguration>& rxConfiguration1,
    const Reference<XConfiguration>& rxConfiguration2)
    : mxConfiguration1(rxConfiguration1),
      mxConfiguration2(rxConfiguration2),
      maC1minusC2(),
      maC2minusC1(),
      maC1andC2()
{
}

bool ConfigurationClassifier::Partition()
{
    maC1minusC2.clear();
    maC2minusC1.clear();
    maC1andC2.clear();

    PartitionResources(
        mxConfiguration1->getResources(NULL, OUString(), AnchorBindingMode_DIRECT),
        mxConfiguration2->getResources(NULL, OUString(), AnchorBindingMode_DIRECT));

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

    SAL_INFO("sd.fwk", OSL_THIS_FUNC << ": copying resource ids to C1-C2");
    CopyResources(aC1minusC2, mxConfiguration1, maC1minusC2);
    SAL_INFO("sd.fwk", OSL_THIS_FUNC << ": copying resource ids to C2-C1");
    CopyResources(aC2minusC1, mxConfiguration2, maC2minusC1);

    // Process the unique resources that belong to both configurations.
    ResourceIdVector::const_iterator iResource;
    for (iResource=aC1andC2.begin(); iResource!=aC1andC2.end(); ++iResource)
    {
        maC1andC2.push_back(*iResource);
        PartitionResources(
            mxConfiguration1->getResources(*iResource, OUString(), AnchorBindingMode_DIRECT),
            mxConfiguration2->getResources(*iResource, OUString(), AnchorBindingMode_DIRECT));
    }
}

void ConfigurationClassifier::ClassifyResources (
    const css::uno::Sequence<Reference<XResourceId> >& rS1,
    const css::uno::Sequence<Reference<XResourceId> >& rS2,
    ResourceIdVector& rS1minusS2,
    ResourceIdVector& rS2minusS1,
    ResourceIdVector& rS1andS2)
{
    // Get arrays from the sequences for faster iteration.
    const Reference<XResourceId>* aA1 = rS1.getConstArray();
    const Reference<XResourceId>* aA2 = rS2.getConstArray();
    sal_Int32 nL1 (rS1.getLength());
    sal_Int32 nL2 (rS2.getLength());

    // Find all elements in rS1 and place them in rS1minusS2 or rS1andS2
    // depending on whether they are in rS2 or not.
    for (sal_Int32 i=0; i<nL1; ++i)
    {
        bool bFound (false);
        for (sal_Int32 j=0; j<nL2 && !bFound; ++j)
            if (aA1[i]->getResourceURL().equals(aA2[j]->getResourceURL()))
                bFound = true;

        if (bFound)
            rS1andS2.push_back(aA1[i]);
        else
            rS1minusS2.push_back(aA1[i]);
    }

    // Find all elements in rS2 that are not in rS1.  The elements that are
    // in both rS1 and rS2 have been handled above and are therefore ignored
    // here.
    for (sal_Int32 j=0; j<nL2; ++j)
    {
        bool bFound (false);
        for (sal_Int32 i=0; i<nL1 && !bFound; ++i)
            if (aA2[j]->getResourceURL().equals(aA1[i]->getResourceURL()))
                bFound = true;

        if ( ! bFound)
            rS2minusS1.push_back(aA2[j]);
    }
}

void ConfigurationClassifier::CopyResources (
    const ResourceIdVector& rSource,
    const Reference<XConfiguration>& rxConfiguration,
    ResourceIdVector& rTarget)
{
    // Copy all resources bound to the ones in aC1minusC2Unique to rC1minusC2.
    ResourceIdVector::const_iterator iResource (rSource.begin());
    ResourceIdVector::const_iterator iEnd(rSource.end());
    for ( ; iResource!=iEnd; ++iResource)
    {
        const Sequence<Reference<XResourceId> > aBoundResources (
            rxConfiguration->getResources(
                *iResource,
                OUString(),
                AnchorBindingMode_INDIRECT));
        const sal_Int32 nL (aBoundResources.getLength());

        rTarget.reserve(rTarget.size() + 1 + nL);
        rTarget.push_back(*iResource);

        SAL_INFO("sd.fwk", OSL_THIS_FUNC << ":    copying " <<
            OUStringToOString(FrameworkHelper::ResourceIdToString(*iResource),
                RTL_TEXTENCODING_UTF8).getStr());

        const Reference<XResourceId>* aA = aBoundResources.getConstArray();
        for (sal_Int32 i=0; i<nL; ++i)
        {
            rTarget.push_back(aA[i]);
            SAL_INFO("sd.fwk", OSL_THIS_FUNC << ":    copying " <<
                OUStringToOString(FrameworkHelper::ResourceIdToString(aA[i]),
                    RTL_TEXTENCODING_UTF8).getStr());
        }
    }
}



} } // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
