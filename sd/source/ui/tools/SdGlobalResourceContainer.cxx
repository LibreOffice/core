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

#include <tools/SdGlobalResourceContainer.hxx>

#include <../cache/SlsCacheConfiguration.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/unique_disposing_ptr.hxx>

#include <com/sun/star/frame/Desktop.hpp>

#include <o3tl/deleter.hxx>
#include <rtl/instance.hxx>
#include <sal/log.hxx>
#include <tools/debug.hxx>

#include <algorithm>
#include <memory>
#include <vector>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sd {

class SdGlobalResourceContainerInstance
    : public comphelper::unique_disposing_solar_mutex_reset_ptr<SdGlobalResourceContainer>
{
public:
    SdGlobalResourceContainerInstance()
        : comphelper::unique_disposing_solar_mutex_reset_ptr<SdGlobalResourceContainer>(
            uno::Reference<lang::XComponent>(frame::Desktop::create(comphelper::getProcessComponentContext()), uno::UNO_QUERY_THROW),
            new SdGlobalResourceContainer, true)
    {
    }
};

namespace {

struct theSdGlobalResourceContainerInstance : public rtl::Static<SdGlobalResourceContainerInstance, theSdGlobalResourceContainerInstance> {};

} // namespace

//===== SdGlobalResourceContainer::Implementation =============================

class SdGlobalResourceContainer::Implementation
{
private:
    friend class SdGlobalResourceContainer;

    ::osl::Mutex maMutex;

    /** All instances of SdGlobalResource in this vector are owned by the
        container and will be destroyed when the container is destroyed.
    */
    std::vector<std::unique_ptr<SdGlobalResource>> maResources;

    typedef ::std::vector<std::shared_ptr<SdGlobalResource> > SharedResourceList;
    SharedResourceList maSharedResources;

    typedef ::std::vector<Reference<XInterface> > XInterfaceResourceList;
    XInterfaceResourceList maXInterfaceResources;
};

// static
SdGlobalResourceContainer& SdGlobalResourceContainer::Instance()
{
    SdGlobalResourceContainer *const pRet(theSdGlobalResourceContainerInstance::get().get());
    assert(pRet); // error if it has been deleted and is null
    return *pRet;
}

//===== SdGlobalResourceContainer =============================================

void SdGlobalResourceContainer::AddResource (
    ::std::unique_ptr<SdGlobalResource> pResource)
{
    ::osl::MutexGuard aGuard (mpImpl->maMutex);

    assert( std::none_of(
                mpImpl->maResources.begin(),
                mpImpl->maResources.end(),
                [&](const std::unique_ptr<SdGlobalResource>& p) { return p == pResource; })
            && "duplicate resource?");

    mpImpl->maResources.push_back(std::move(pResource));
}

void SdGlobalResourceContainer::AddResource (
    const std::shared_ptr<SdGlobalResource>& pResource)
{
    ::osl::MutexGuard aGuard (mpImpl->maMutex);

    Implementation::SharedResourceList::iterator iResource;
    iResource = ::std::find (
        mpImpl->maSharedResources.begin(),
        mpImpl->maSharedResources.end(),
        pResource);
    if (iResource == mpImpl->maSharedResources.end())
        mpImpl->maSharedResources.push_back(pResource);
    else
    {
        SAL_WARN ("sd.tools",
            "SdGlobalResourceContainer:AddResource(): Resource added twice.");
    }
}

void SdGlobalResourceContainer::AddResource (const Reference<XInterface>& rxResource)
{
    ::osl::MutexGuard aGuard (mpImpl->maMutex);

    Implementation::XInterfaceResourceList::iterator iResource;
    iResource = ::std::find (
        mpImpl->maXInterfaceResources.begin(),
        mpImpl->maXInterfaceResources.end(),
        rxResource);
    if (iResource == mpImpl->maXInterfaceResources.end())
        mpImpl->maXInterfaceResources.push_back(rxResource);
    else
    {
        SAL_WARN ("sd.tools",
            "SdGlobalResourceContainer:AddResource(): Resource added twice.");
    }
}

SdGlobalResourceContainer::SdGlobalResourceContainer()
    : mpImpl (new SdGlobalResourceContainer::Implementation)
{
}

SdGlobalResourceContainer::~SdGlobalResourceContainer()
{
    ::osl::MutexGuard aGuard (mpImpl->maMutex);

    // Release the resources in reversed order of their addition to the
    // container.  This is because a resource A added before resource B
    // may have been created due to a request of B.  Thus B depends on A and
    // should be destroyed first.
    for (auto iResource = mpImpl->maResources.rbegin();
         iResource != mpImpl->maResources.rend();
         ++iResource)
    {
        iResource->reset();
    }


    // The SharedResourceList has not to be released manually.  We just
    // assert resources that are still held by someone other than us.
    Implementation::SharedResourceList::reverse_iterator iSharedResource;
    for (iSharedResource = mpImpl->maSharedResources.rbegin();
         iSharedResource != mpImpl->maSharedResources.rend();
         ++iSharedResource)
    {
        if (iSharedResource->use_count() > 1)
        {
            SdGlobalResource* pResource = iSharedResource->get();
            SAL_INFO(
                "sd.tools", pResource << " " << iSharedResource->use_count());
            DBG_ASSERT(iSharedResource->use_count() == 1,
                "SdGlobalResource still held in ~SdGlobalResourceContainer");
        }
    }

    Implementation::XInterfaceResourceList::reverse_iterator iXInterfaceResource;
    for (iXInterfaceResource = mpImpl->maXInterfaceResources.rbegin();
         iXInterfaceResource != mpImpl->maXInterfaceResources.rend();
         ++iXInterfaceResource)
    {
        Reference<lang::XComponent> xComponent (*iXInterfaceResource, UNO_QUERY);
        *iXInterfaceResource = nullptr;
        if (xComponent.is())
            xComponent->dispose();
    }

    sd::slidesorter::cache::CacheConfiguration::Shutdown();
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
