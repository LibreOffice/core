/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SdGlobalResourceContainer.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:53:33 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

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

    typedef ::std::vector<boost::shared_ptr<SdGlobalResource> > SharedResourceList;
    SharedResourceList maSharedResources;

    typedef ::std::vector<Reference<XInterface> > XInterfaceResourceList;
    XInterfaceResourceList maXInterfaceResources;
};




// static
SdGlobalResourceContainer& SdGlobalResourceContainer::Instance (void)
{
    DBG_ASSERT(Implementation::mpInstance!=NULL,
        "SdGlobalResourceContainer::Instance(): instance has been deleted");
    // Maybe we should throw an exception when the instance has been deleted.
    return *Implementation::mpInstance;
}

SdGlobalResourceContainer*
    SdGlobalResourceContainer::Implementation::mpInstance = NULL;




//===== SdGlobalResourceContainer =============================================

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
        // Because the given resource is an auto_ptr it is highly unlikely
        // that we come here.  But who knows?
        DBG_ASSERT (false,
            "SdGlobalResourceContainer:AddResource(): Resource added twice.");
    }
    // We can not put the auto_ptr into the vector so we release the
    // auto_ptr and document that we take ownership explicitly.
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




::std::auto_ptr<SdGlobalResource> SdGlobalResourceContainer::ReleaseResource (
    SdGlobalResource* pResource)
{
    ::std::auto_ptr<SdGlobalResource> pResult (NULL);

    ::osl::MutexGuard aGuard (mpImpl->maMutex);

    Implementation::ResourceList::iterator iResource;
    iResource = ::std::find (
        mpImpl->maResources.begin(),
        mpImpl->maResources.end(),
        pResource);
    if (iResource != mpImpl->maResources.end())
    {
        pResult.reset (*iResource);
        mpImpl->maResources.erase(iResource);
    }

    return pResult;
}




SdGlobalResourceContainer::SdGlobalResourceContainer (void)
    : mpImpl (new SdGlobalResourceContainer::Implementation())
{
    Implementation::mpInstance = this;
}




SdGlobalResourceContainer::~SdGlobalResourceContainer (void)
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




} // end of namespace sd
