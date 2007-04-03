/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Configuration.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-03 15:44:40 $
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

#include "precompiled_sd.hxx"

#include "framework/Configuration.hxx"

#include "framework/FrameworkHelper.hxx"
#include <comphelper/stl_types.hxx>



using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::sd::framework::FrameworkHelper;
using ::rtl::OUString;

#undef VERBOSE

namespace {
class XResourceIdLess
    :   public ::std::binary_function <Reference<XResourceId>, Reference<XResourceId>, bool>
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
    ResourceContainer (void) {}
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




Configuration::~Configuration (void)
{
}




void SAL_CALL Configuration::disposing (void)
{
    ::osl::MutexGuard aGuard (maMutex);
    mpResourceContainer->clear();
    mxBroadcaster = NULL;
}




//----- XConfiguration --------------------------------------------------------

void SAL_CALL Configuration::addResource (const Reference<XResourceId>& rxResourceId)
    throw (RuntimeException)
{
    ThrowIfDisposed();

    if ( ! rxResourceId.is() || rxResourceId->getResourceURL().getLength()==0)
        throw ::com::sun::star::lang::IllegalArgumentException();

    if (mpResourceContainer->find(rxResourceId) == mpResourceContainer->end())
    {
#ifdef VERBOSE
    OSL_TRACE("Configuration::addResource() %s",
        OUStringToOString(
            FrameworkHelper::ResourceIdToString(rxResourceId), RTL_TEXTENCODING_UTF8).getStr());
#endif
        mpResourceContainer->insert(rxResourceId);
        PostEvent(rxResourceId, true);
    }
}




void SAL_CALL Configuration::removeResource (const Reference<XResourceId>& rxResourceId)
    throw (RuntimeException)
{
    ThrowIfDisposed();

    if ( ! rxResourceId.is() || rxResourceId->getResourceURL().getLength()==0)
        throw ::com::sun::star::lang::IllegalArgumentException();

    ResourceContainer::iterator iResource (mpResourceContainer->find(rxResourceId));
    if (iResource != mpResourceContainer->end())
    {
#ifdef VERBOSE
    OSL_TRACE("Configuration::removeResource() %s",
        OUStringToOString(
            FrameworkHelper::ResourceIdToString(rxResourceId), RTL_TEXTENCODING_UTF8).getStr());
#endif
        PostEvent(rxResourceId,false);
        mpResourceContainer->erase(iResource);
    }
}




Sequence<Reference<XResourceId> > SAL_CALL Configuration::getResources (
    const Reference<XResourceId>& rxAnchorId,
    const ::rtl::OUString& rsResourceURLPrefix,
    AnchorBindingMode eMode)
    throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    ThrowIfDisposed();

    bool bFilterResources (rsResourceURLPrefix.getLength() > 0);

    // Collect the matching resources in a vector.
    ::std::vector<Reference<XResourceId> > aResources;
    ResourceContainer::const_iterator iResource;
    for (iResource=mpResourceContainer->begin();
         iResource!=mpResourceContainer->end();
         ++iResource)
    {
        if ( ! (*iResource)->isBoundTo(rxAnchorId,eMode))
            continue;


        if (bFilterResources)
        {
            // Apply the given resource prefix as filter.

            // Make sure that the resource is bound directly to the anchor.
            if (eMode != AnchorBindingMode_DIRECT
                && ! (*iResource)->isBoundTo(rxAnchorId, AnchorBindingMode_DIRECT))
            {
                continue;
            }

            // Make sure that the resource URL matches the given prefix.
            if ( ! (*iResource)->getResourceURL().match(rsResourceURLPrefix))
            {
                continue;
            }
        }

        aResources.push_back(*iResource);
    }

    // Copy the resources from the vector into a new sequence.
    Sequence<Reference<XResourceId> > aResult (aResources.size());
    for (sal_uInt32 nIndex=0; nIndex<aResources.size(); ++nIndex)
        aResult[nIndex] = aResources[nIndex];

    return aResult;
}




sal_Bool SAL_CALL Configuration::hasResource (const Reference<XResourceId>& rxResourceId)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    ThrowIfDisposed();

    return rxResourceId.is()
        && mpResourceContainer->find(rxResourceId) != mpResourceContainer->end();
}




//----- XCloneable ------------------------------------------------------------

Reference<util::XCloneable> SAL_CALL Configuration::createClone (void)
    throw (RuntimeException)
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

OUString SAL_CALL Configuration::getName (void)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    OUString aString;

    if (rBHelper.bDisposed || rBHelper.bInDispose)
        aString += OUString::createFromAscii("DISPOSED ");
    aString += OUString::createFromAscii("Configuration[");

    ResourceContainer::const_iterator iResource;
    for (iResource=mpResourceContainer->begin();
         iResource!=mpResourceContainer->end();
         ++iResource)
    {
        if (iResource != mpResourceContainer->begin())
            aString += OUString::createFromAscii(", ");
        aString += FrameworkHelper::ResourceIdToString(*iResource);
    }
    aString += OUString::createFromAscii("]");

    return aString;
}




void SAL_CALL Configuration::setName (const OUString& rsName)
    throw (RuntimeException)
{
    (void)rsName; // rsName is ignored.
}





// ----------------------------------------------------------------------------

void Configuration::PostEvent (
    const Reference<XResourceId>& rxResourceId,
    const bool bActivation)
{
    OSL_ASSERT(rxResourceId.is());

    if (mxBroadcaster.is())
    {
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
}




void Configuration::ThrowIfDisposed (void) const
    throw (::com::sun::star::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            OUString(RTL_CONSTASCII_USTRINGPARAM(
                "Configuration object has already been disposed")),
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}




//=============================================================================

bool AreConfigurationsEquivalent (
    const Reference<XConfiguration>& rxConfiguration1,
    const Reference<XConfiguration>& rxConfiguration2)
{
    // Get the lists of resources from the two given configurations.
    Sequence<Reference<XResourceId> > aResources1;
    if (rxConfiguration1.is())
        aResources1 = rxConfiguration1->getResources(
            NULL, OUString(), AnchorBindingMode_INDIRECT);
    Sequence<Reference<XResourceId> > aResources2;
    if (rxConfiguration2.is())
        aResources2 = rxConfiguration2->getResources(
            NULL, OUString(), AnchorBindingMode_INDIRECT);

    // When the number of resources differ then the configurations can not
    // be equivalent.
    sal_Int32 nCount1 (aResources1.getLength());
    sal_Int32 nCount2 (aResources2.getLength());
    if (nCount1 != nCount2)
        return false;

    // Compare the lists.
    // This is done by nested iteration over both lists.  The result is an
    // O(n^2) algorithm.  Sorting the lists first and comparing then would
    // be something like O(n*log(n)).  But, at the moment, the lists are
    // expected to be comparatively small (around 10 elements) and the
    // overhead in sorting the lists and the increased complexity of the
    // implementation do not support that approach.

    // Make a local copy of the second resource list so that elements can be
    // removed from that list.  This prevents elements from it to be
    // compared with elements from the first list even when their match has
    // already been found.
    ::std::list<Reference<XResourceId> > aResourceList2;
    for (sal_Int32 nIndex2=0; nIndex2<nCount2; ++nIndex2)
        aResourceList2.push_back(aResources2[nIndex2]);


    for (sal_Int32 nIndex1=0; nIndex1<nCount1; ++nIndex1)
    {
        Reference<XResourceId> xResource1 (aResources1[nIndex1]);
        ::std::list<Reference<XResourceId> >::iterator iResource2;
        bool bFound (false);
        for (iResource2=aResourceList2.begin(); iResource2!=aResourceList2.end(); ++iResource2)
        {
            if (xResource1.is())
            {
                if (iResource2->is() && xResource1->compareTo(*iResource2)==0)
                    bFound = true;
            }
            else if ( ! iResource2->is())
            {
                bFound = true;
            }

            // We have found a match between xResource and iResource2.
            // Remove the later one from aResourceList2 so that it will not
            // be compared to elements from aResource1 anymore.
            if (bFound)
            {
                aResourceList2.erase(iResource2);
                break;
            }
        }
        if ( ! bFound)
            return false;
    }
    return aResourceList2.empty();
}

} } // end of namespace sd::framework
