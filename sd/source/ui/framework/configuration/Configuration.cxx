/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
/** Use the XResourceId::compareTo() method to implement a compare operator
    for STL containers.
*/
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




//----- Service ---------------------------------------------------------------

Reference<XInterface> SAL_CALL Configuration_createInstance (
    const Reference<XComponentContext>& rxContext)
{
    (void)rxContext;
    return Reference<XInterface>(static_cast<XWeak*>(new Configuration(NULL,false)));
}




OUString Configuration_getImplementationName (void) throw(RuntimeException)
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.comp.Draw.framework.configuration.Configuration"));
}




Sequence<rtl::OUString> SAL_CALL Configuration_getSupportedServiceNames (void)
    throw (RuntimeException)
{
    static const OUString sServiceName(OUString::createFromAscii(
        "com.sun.star.drawing.framework.Configuration"));
    return Sequence<rtl::OUString>(&sServiceName, 1);
}




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
    if (rxConfiguration1.is() != rxConfiguration2.is())
        return false;
    if ( ! rxConfiguration1.is() && ! rxConfiguration2.is())
        return true;

    // Get the lists of resources from the two given configurations.
    const Sequence<Reference<XResourceId> > aResources1(
        rxConfiguration1->getResources(
            NULL, OUString(), AnchorBindingMode_INDIRECT));
    const Sequence<Reference<XResourceId> > aResources2(
        rxConfiguration2->getResources(
            NULL, OUString(), AnchorBindingMode_INDIRECT));

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
