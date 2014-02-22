/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include "tools/SdGlobalResourceContainer.hxx"

#include <algorithm>
#include <vector>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


namespace sd {




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

    typedef ::std::vector<boost::shared_ptr<SdGlobalResource> > SharedResourceList;
    SharedResourceList maSharedResources;

    typedef ::std::vector<Reference<XInterface> > XInterfaceResourceList;
    XInterfaceResourceList maXInterfaceResources;
};





SdGlobalResourceContainer& SdGlobalResourceContainer::Instance (void)
{
    DBG_ASSERT(Implementation::mpInstance!=NULL,
        "SdGlobalResourceContainer::Instance(): instance has been deleted");
    
    return *Implementation::mpInstance;
}

SdGlobalResourceContainer*
    SdGlobalResourceContainer::Implementation::mpInstance = NULL;






void SdGlobalResourceContainer::AddResource (
    ::std::auto_ptr<SdGlobalResource> pResource)
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
        
        
        DBG_ASSERT (false,
            "SdGlobalResourceContainer:AddResource(): Resource added twice.");
    }
    
    
    pResource.release();
}




void SdGlobalResourceContainer::AddResource (
    ::boost::shared_ptr<SdGlobalResource> pResource)
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
        DBG_ASSERT (false,
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
        DBG_ASSERT (false,
            "SdGlobalResourceContainer:AddResource(): Resource added twice.");
    }
}



SdGlobalResourceContainer::SdGlobalResourceContainer (void)
    : mpImpl (new SdGlobalResourceContainer::Implementation())
{
    Implementation::mpInstance = this;
}




SdGlobalResourceContainer::~SdGlobalResourceContainer (void)
{
    ::osl::MutexGuard aGuard (mpImpl->maMutex);

    
    
    
    
    Implementation::ResourceList::reverse_iterator iResource;
    for (iResource = mpImpl->maResources.rbegin();
         iResource != mpImpl->maResources.rend();
         ++iResource)
    {
        delete *iResource;
    }

    
    
    Implementation::SharedResourceList::reverse_iterator iSharedResource;
    for (iSharedResource = mpImpl->maSharedResources.rbegin();
         iSharedResource != mpImpl->maSharedResources.rend();
         ++iSharedResource)
    {
        if ( ! iSharedResource->unique())
        {
            SdGlobalResource* pResource = iSharedResource->get();
            OSL_TRACE(" %p %d", pResource, iSharedResource->use_count());
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
        *iXInterfaceResource = NULL;
        if (xComponent.is())
            xComponent->dispose();
    }

    DBG_ASSERT(Implementation::mpInstance == this,
        "~SdGlobalResourceContainer(): more than one instance of singleton");
    Implementation::mpInstance = NULL;
}




} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
