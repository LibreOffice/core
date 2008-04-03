/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ResourceManager.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 13:39:35 $
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

#include "ResourceManager.hxx"

#include "framework/FrameworkHelper.hxx"
#include "framework/ConfigurationController.hxx"

#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_XCONTROLLERMANAGER_HPP_
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#endif

#include <comphelper/stl_types.hxx>
#include <set>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

using ::rtl::OUString;
using ::sd::framework::FrameworkHelper;

namespace {
    static const sal_Int32 ResourceActivationRequestEvent = 0;
    static const sal_Int32 ResourceDeactivationRequestEvent = 1;
}




namespace sd { namespace framework {

class ResourceManager::MainViewContainer
    : public ::std::set<OUString, ::comphelper::UStringLess>
{
public:
    MainViewContainer (void) {}
};




//===== ResourceManager =======================================================

ResourceManager::ResourceManager (
    const Reference<frame::XController>& rxController,
    const Reference<XResourceId>& rxResourceId)
    : ResourceManagerInterfaceBase(MutexOwner::maMutex),
      mxConfigurationController(),
      mpActiveMainViewContainer(new MainViewContainer()),
      mxResourceId(rxResourceId),
      mxMainViewAnchorId(FrameworkHelper::Instance(rxController)->CreateResourceId(
          FrameworkHelper::msCenterPaneURL)),
      msCurrentMainViewURL(),
      mbIsEnabled(true)
{
    Reference<XControllerManager> xControllerManager (rxController, UNO_QUERY);
    if (xControllerManager.is())
    {
        mxConfigurationController = xControllerManager->getConfigurationController();

        if (mxConfigurationController.is())
        {
            mxConfigurationController->addConfigurationChangeListener(
                this,
                FrameworkHelper::msResourceActivationRequestEvent,
                makeAny(ResourceActivationRequestEvent));
            mxConfigurationController->addConfigurationChangeListener(
                this,
                FrameworkHelper::msResourceDeactivationRequestEvent,
                makeAny(ResourceDeactivationRequestEvent));
        }
    }
}




ResourceManager::~ResourceManager (void)
{
}




void ResourceManager::AddActiveMainView (
    const OUString& rsMainViewURL)
{
    mpActiveMainViewContainer->insert(rsMainViewURL);
}




void SAL_CALL ResourceManager::disposing (void)
{
    if (mxConfigurationController.is())
    {
        mxConfigurationController->removeConfigurationChangeListener(this);
        mxConfigurationController = NULL;
    }
}




void ResourceManager::Enable (void)
{
    mbIsEnabled = true;
    UpdateForMainViewShell();
}




void ResourceManager::Disable (void)
{
    mbIsEnabled = false;
    UpdateForMainViewShell();
}




void SAL_CALL ResourceManager::notifyConfigurationChange (
    const ConfigurationChangeEvent& rEvent)
    throw (RuntimeException)
{
    OSL_ASSERT(rEvent.ResourceId.is());

    sal_Int32 nEventType = 0;
    rEvent.UserData >>= nEventType;
    switch (nEventType)
    {
        case ResourceActivationRequestEvent:
            if (rEvent.ResourceId->isBoundToURL(
                FrameworkHelper::msCenterPaneURL,
                AnchorBindingMode_DIRECT))
            {
                // A resource directly bound to the center pane has been
                // requested.
                if (rEvent.ResourceId->getResourceTypePrefix().equals(
                    FrameworkHelper::msViewURLPrefix))
                {
                    // The requested resource is a view.  Show or hide the
                    // resource managed by this ResourceManager accordingly.
                    HandleMainViewSwitch(
                        rEvent.ResourceId->getResourceURL(),
                        rEvent.Configuration,
                        true);
                }
            }
            else if (rEvent.ResourceId->compareTo(mxResourceId) == 0)
            {
                // The resource managed by this ResourceManager has been
                // explicitly been requested (maybe by us).  Remember this
                // setting.
                HandleResourceRequest(true, rEvent.Configuration);
            }
            break;

        case ResourceDeactivationRequestEvent:
            if (rEvent.ResourceId->compareTo(mxMainViewAnchorId) == 0)
            {
                HandleMainViewSwitch(
                    OUString(),
                    rEvent.Configuration,
                    false);
            }
            else if (rEvent.ResourceId->compareTo(mxResourceId) == 0)
            {
                // The resource managed by this ResourceManager has been
                // explicitly been requested to be hidden (maybe by us).
                // Remember this setting.
                HandleResourceRequest(false, rEvent.Configuration);
            }
            break;
    }
}




void ResourceManager::UpdateForMainViewShell (void)
{
    if (mxConfigurationController.is())
    {
        ConfigurationController::Lock aLock (mxConfigurationController);

        if (mbIsEnabled
            && mpActiveMainViewContainer->find(msCurrentMainViewURL)
               != mpActiveMainViewContainer->end())
        {
            // Activate resource.
            mxConfigurationController->requestResourceActivation(
                mxResourceId->getAnchor(),
                ResourceActivationMode_ADD);
            mxConfigurationController->requestResourceActivation(
                mxResourceId,
                ResourceActivationMode_REPLACE);
        }
        else
        {
            mxConfigurationController->requestResourceDeactivation(mxResourceId);
        }
    }
}




void ResourceManager::HandleMainViewSwitch (
    const OUString& rsViewURL,
    const Reference<XConfiguration>& rxConfiguration,
    const bool bIsActivated)
{
    (void)rxConfiguration;
    if (bIsActivated)
        msCurrentMainViewURL = rsViewURL;
    else
        msCurrentMainViewURL = OUString();
    UpdateForMainViewShell();
}




void ResourceManager::HandleResourceRequest(
    bool bActivation,
    const Reference<XConfiguration>& rxConfiguration)
{
    if (mbIsEnabled)
    {
        Sequence<Reference<XResourceId> > aCenterViews = rxConfiguration->getResources(
            FrameworkHelper::CreateResourceId(FrameworkHelper::msCenterPaneURL),
            FrameworkHelper::msViewURLPrefix,
            AnchorBindingMode_DIRECT);
        if (aCenterViews.getLength() == 1)
        {
            if (bActivation)
            {
                mpActiveMainViewContainer->insert(aCenterViews[0]->getResourceURL());
            }
            else
            {
                MainViewContainer::iterator iElement (
                    mpActiveMainViewContainer->find(aCenterViews[0]->getResourceURL()));
                if (iElement != mpActiveMainViewContainer->end())
                    mpActiveMainViewContainer->erase(iElement);
            }
        }
    }
}




void SAL_CALL ResourceManager::disposing (
    const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    if (mxConfigurationController.is()
        && rEvent.Source == mxConfigurationController)
    {
        // Without the configuration controller this class can do nothing.
        mxConfigurationController = NULL;
        dispose();
    }
}




void ResourceManager::Trace (void) const
{
    OSL_TRACE("main views with resource %s:",
        ::rtl::OUStringToOString(
            FrameworkHelper::ResourceIdToString(mxResourceId), RTL_TEXTENCODING_UTF8).getStr());

    MainViewContainer::const_iterator iDescriptor;
    for (iDescriptor=mpActiveMainViewContainer->begin();
         iDescriptor!=mpActiveMainViewContainer->end();
         ++iDescriptor)
    {
        OSL_TRACE("    %s",
            ::rtl::OUStringToOString(*iDescriptor, RTL_TEXTENCODING_UTF8).getStr());
    }
}



} } // end of namespace sd::framework
