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

#include <framework/Configuration.hxx>

#include <framework/FrameworkHelper.hxx>

#include <com/sun/star/drawing/framework/ConfigurationChangeEvent.hpp>
#include <com/sun/star/drawing/framework/XConfigurationControllerBroadcaster.hpp>
#include <comphelper/sequence.hxx>
#include <facreg.hxx>
#include <sal/log.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::sd::framework::FrameworkHelper;

namespace {
/** Use the XResourceId::compareTo() method to implement a compare operator
    for STL containers.
*/
class XResourceIdLess
{
public:
    bool operator () (const Reference<XResourceId>& rId1, const Reference<XResourceId>& rId2) const
    {
        return rId1->compareTo(rId2) == -1;
    }
};

} // end of anonymous namespace

namespace sd { namespace framework {

class Configuration::ResourceContainer
    : public ::std::set<Reference<XResourceId>, XResourceIdLess>
{
public:
    ResourceContainer() {}
};

//===== Configuration =========================================================

Configuration::Configuration (
    const Reference<XConfigurationControllerBroadcaster>& rxBroadcaster,
    bool bBroadcastRequestEvents)
    : ConfigurationInterfaceBase(MutexOwner::maMutex),
      mpResourceContainer(new ResourceContainer()),
      mxBroadcaster(rxBroadcaster),
      mbBroadcastRequestEvents(bBroadcastRequestEvents)
{
}

Configuration::Configuration (
    const Reference<XConfigurationControllerBroadcaster>& rxBroadcaster,
    bool bBroadcastRequestEvents,
    const ResourceContainer& rResourceContainer)
    : ConfigurationInterfaceBase(MutexOwner::maMutex),
      mpResourceContainer(new ResourceContainer(rResourceContainer)),
      mxBroadcaster(rxBroadcaster),
      mbBroadcastRequestEvents(bBroadcastRequestEvents)
{
}

Configuration::~Configuration()
{
}

void SAL_CALL Configuration::disposing()
{
    ::osl::MutexGuard aGuard (maMutex);
    mpResourceContainer->clear();
    mxBroadcaster = nullptr;
}

//----- XConfiguration --------------------------------------------------------

void SAL_CALL Configuration::addResource (const Reference<XResourceId>& rxResourceId)
{
    ThrowIfDisposed();

    if ( ! rxResourceId.is() || rxResourceId->getResourceURL().isEmpty())
        throw css::lang::IllegalArgumentException();

    if (mpResourceContainer->find(rxResourceId) == mpResourceContainer->end())
    {
        SAL_INFO("sd.fwk", OSL_THIS_FUNC << ": Configuration::addResource() " <<
                FrameworkHelper::ResourceIdToString(rxResourceId));
        mpResourceContainer->insert(rxResourceId);
        PostEvent(rxResourceId, true);
    }
}

void SAL_CALL Configuration::removeResource (const Reference<XResourceId>& rxResourceId)
{
    ThrowIfDisposed();

    if ( ! rxResourceId.is() || rxResourceId->getResourceURL().isEmpty())
        throw css::lang::IllegalArgumentException();

    ResourceContainer::iterator iResource (mpResourceContainer->find(rxResourceId));
    if (iResource != mpResourceContainer->end())
    {
        SAL_INFO("sd.fwk", OSL_THIS_FUNC << ": Configuration::removeResource() " <<
                FrameworkHelper::ResourceIdToString(rxResourceId));
        PostEvent(rxResourceId,false);
        mpResourceContainer->erase(iResource);
    }
}

Sequence<Reference<XResourceId> > SAL_CALL Configuration::getResources (
    const Reference<XResourceId>& rxAnchorId,
    const OUString& rsResourceURLPrefix,
    AnchorBindingMode eMode)
{
    ::osl::MutexGuard aGuard (maMutex);
    ThrowIfDisposed();

    bool bFilterResources (!rsResourceURLPrefix.isEmpty());

    // Collect the matching resources in a vector.
    ::std::vector<Reference<XResourceId> > aResources;
    for (const auto& rxResource : *mpResourceContainer)
    {
        if ( ! rxResource->isBoundTo(rxAnchorId,eMode))
            continue;

        if (bFilterResources)
        {
            // Apply the given resource prefix as filter.

            // Make sure that the resource is bound directly to the anchor.
            if (eMode != AnchorBindingMode_DIRECT
                && ! rxResource->isBoundTo(rxAnchorId, AnchorBindingMode_DIRECT))
            {
                continue;
            }

            // Make sure that the resource URL matches the given prefix.
            if ( ! rxResource->getResourceURL().match(rsResourceURLPrefix))
            {
                continue;
            }
        }

        aResources.push_back(rxResource);
    }

    return comphelper::containerToSequence(aResources);
}

sal_Bool SAL_CALL Configuration::hasResource (const Reference<XResourceId>& rxResourceId)
{
    ::osl::MutexGuard aGuard (maMutex);
    ThrowIfDisposed();

    return rxResourceId.is()
        && mpResourceContainer->find(rxResourceId) != mpResourceContainer->end();
}

//----- XCloneable ------------------------------------------------------------

Reference<util::XCloneable> SAL_CALL Configuration::createClone()
{
    ::osl::MutexGuard aGuard (maMutex);
    ThrowIfDisposed();

    Configuration* pConfiguration = new Configuration(
        mxBroadcaster,
        mbBroadcastRequestEvents,
        *mpResourceContainer);

    return Reference<util::XCloneable>(pConfiguration);
}

//----- XNamed ----------------------------------------------------------------

OUString SAL_CALL Configuration::getName()
{
    ::osl::MutexGuard aGuard (maMutex);
    OUStringBuffer aString;

    if (rBHelper.bDisposed || rBHelper.bInDispose)
        aString.append("DISPOSED ");
    aString.append("Configuration[");

    ResourceContainer::const_iterator iResource;
    for (iResource=mpResourceContainer->begin();
         iResource!=mpResourceContainer->end();
         ++iResource)
    {
        if (iResource != mpResourceContainer->begin())
            aString.append(", ");
        aString.append(FrameworkHelper::ResourceIdToString(*iResource));
    }
    aString.append("]");

    return aString.makeStringAndClear();
}

void SAL_CALL Configuration::setName (const OUString&)
{
    // ignored.
}

OUString Configuration::getImplementationName()
{
    return OUString(
        "com.sun.star.comp.Draw.framework.configuration.Configuration");
}

sal_Bool Configuration::supportsService(OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> Configuration::getSupportedServiceNames()
{
    return css::uno::Sequence<OUString>{
        "com.sun.star.drawing.framework.Configuration"};
}

void Configuration::PostEvent (
    const Reference<XResourceId>& rxResourceId,
    const bool bActivation)
{
    OSL_ASSERT(rxResourceId.is());

    if (!mxBroadcaster.is())
        return;

    ConfigurationChangeEvent aEvent;
    aEvent.ResourceId = rxResourceId;
    if (bActivation)
        if (mbBroadcastRequestEvents)
            aEvent.Type = FrameworkHelper::msResourceActivationRequestEvent;
        else
            aEvent.Type = FrameworkHelper::msResourceActivationEvent;
    else
        if (mbBroadcastRequestEvents)
            aEvent.Type = FrameworkHelper::msResourceDeactivationRequestEvent;
        else
            aEvent.Type = FrameworkHelper::msResourceDeactivationEvent;
    aEvent.Configuration = this;

    mxBroadcaster->notifyEvent(aEvent);
}

void Configuration::ThrowIfDisposed() const
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException ("Configuration object has already been disposed",
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}

bool AreConfigurationsEquivalent (
    const Reference<XConfiguration>& rxConfiguration1,
    const Reference<XConfiguration>& rxConfiguration2)
{
    if (rxConfiguration1.is() != rxConfiguration2.is())
        return false;
    if ( ! rxConfiguration1.is() && ! rxConfiguration2.is())
        return true;

    // Get the lists of resources from the two given configurations.
    const Sequence<Reference<XResourceId> > aResources1(
        rxConfiguration1->getResources(
            nullptr, OUString(), AnchorBindingMode_INDIRECT));
    const Sequence<Reference<XResourceId> > aResources2(
        rxConfiguration2->getResources(
            nullptr, OUString(), AnchorBindingMode_INDIRECT));

    // When the number of resources differ then the configurations can not
    // be equivalent.
    const sal_Int32 nCount (aResources1.getLength());
    const sal_Int32 nCount2 (aResources2.getLength());
    if (nCount != nCount2)
        return false;

    // Comparison of the two lists of resource ids relies on their
    // ordering.
    for (sal_Int32 nIndex=0; nIndex<nCount; ++nIndex)
    {
        const Reference<XResourceId> xResource1 (aResources1[nIndex]);
        const Reference<XResourceId> xResource2 (aResources2[nIndex]);
        if (xResource1.is() && xResource2.is())
        {
            if (xResource1->compareTo(xResource2) != 0)
                return false;
        }
        else if (xResource1.is() != xResource2.is())
        {
            return false;
        }
    }

    return true;
}

} } // end of namespace sd::framework


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Draw_framework_configuration_Configuration_get_implementation(
        css::uno::XComponentContext*,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new sd::framework::Configuration(nullptr, false));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
