/*************************************************************************
 *
 *  $RCSfile: SdGlobalResourceContainer.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-08-04 09:00:20 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "tools/SdGlobalResourceContainer.hxx"

#include <algorithm>
#include <vector>

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

    DBG_ASSERT(Implementation::mpInstance == this,
        "~SdGlobalResourceContainer(): more than one instance of singleton");
    Implementation::mpInstance = NULL;
}




} // end of namespace sd
