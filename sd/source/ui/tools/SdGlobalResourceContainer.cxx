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

#include "tools/SdGlobalResourceContainer.hxx"

#include <algorithm>
#include <vector>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sd {

//===== SdGlobalResourceContainer::Implementation =============================

class SdGlobalResourceContainer::Implementation
{
private:
    friend class SdGlobalResourceContainer;
    static SdGlobalResourceContainer* mpInstance;

    ::osl::Mutex maMutex;

    /** All instances of SdGlobalResource in this vector are owned by the
        container and will be destroyed when the container is destroyed.
    */
    typedef ::std::vector<SdGlobalResource*> ResourceList;
    ResourceList maResources;

    typedef ::std::vector<std::shared_ptr<SdGlobalResource> > SharedResourceList;
    SharedResourceList maSharedResources;

    typedef ::std::vector<Reference<XInterface> > XInterfaceResourceList;
    XInterfaceResourceList maXInterfaceResources;
};

// static
SdGlobalResourceContainer& SdGlobalResourceContainer::Instance()
{
    DBG_ASSERT(Implementation::mpInstance!=nullptr,
        "SdGlobalResourceContainer::Instance(): instance has been deleted");
    // Maybe we should throw an exception when the instance has been deleted.
    return *Implementation::mpInstance;
}

SdGlobalResourceContainer*
    SdGlobalResourceContainer::Implementation::mpInstance = nullptr;

//===== SdGlobalResourceContainer =============================================

void SdGlobalResourceContainer::AddResource (
    ::std::unique_ptr<SdGlobalResource> && pResource)
{
    ::osl::MutexGuard aGuard (mpImpl->maMutex);

    Implementation::ResourceList::iterator iResource;
    iResource = ::std::find (
        mpImpl->maResources.begin(),
        mpImpl->maResources.end(),
        pResource.get());
    if (iResource == mpImpl->maResources.end())
        mpImpl->maResources.push_back(pResource.get());
    else
    {
        // Because the given resource is a unique_ptr it is highly unlikely
        // that we come here.  But who knows?
        SAL_WARN ( "sd",
            "SdGlobalResourceContainer:AddResource(): Resource added twice.");
    }
    // We can not put the unique_ptr into the vector so we release the
    // unique_ptr and document that we take ownership explicitly.
    pResource.release();
}

void SdGlobalResourceContainer::AddResource (
    std::shared_ptr<SdGlobalResource> pResource)
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
        SAL_WARN ("sd",
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
        SAL_WARN ("sd",
            "SdGlobalResourceContainer:AddResource(): Resource added twice.");
    }
}

SdGlobalResourceContainer::SdGlobalResourceContainer()
    : mpImpl (new SdGlobalResourceContainer::Implementation())
{
    Implementation::mpInstance = this;
}

SdGlobalResourceContainer::~SdGlobalResourceContainer()
{
    ::osl::MutexGuard aGuard (mpImpl->maMutex);

    // Release the resources in reversed order of their addition to the
    // container.  This is because a resource A added before resource B
    // may have been created due to a request of B.  Thus B depends on A and
    // should be destroyed first.
    Implementation::ResourceList::reverse_iterator iResource;
    for (iResource = mpImpl->maResources.rbegin();
         iResource != mpImpl->maResources.rend();
         ++iResource)
    {
        delete *iResource;
    }

    // The SharedResourceList has not to be released manually.  We just
    // assert resources that are still held by someone other than us.
    Implementation::SharedResourceList::reverse_iterator iSharedResource;
    for (iSharedResource = mpImpl->maSharedResources.rbegin();
         iSharedResource != mpImpl->maSharedResources.rend();
         ++iSharedResource)
    {
        if ( ! iSharedResource->unique())
        {
            SdGlobalResource* pResource = iSharedResource->get();
            SAL_INFO(
                "sd.tools", pResource << " " << iSharedResource->use_count());
            DBG_ASSERT(iSharedResource->unique(),
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

    DBG_ASSERT(Implementation::mpInstance == this,
        "~SdGlobalResourceContainer(): more than one instance of singleton");
    Implementation::mpInstance = nullptr;
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
