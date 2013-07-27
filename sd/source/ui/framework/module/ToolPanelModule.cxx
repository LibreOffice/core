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

#include "ToolPanelModule.hxx"
#include "ReadOnlyModeObserver.hxx"
#include "framework/FrameworkHelper.hxx"

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/drawing/framework/XControllerManager.hpp>

#include <comphelper/processfactory.hxx>
#include <cppuhelper/compbase1.hxx>
#include <boost/enable_shared_from_this.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::rtl::OUString;
using ::sd::framework::FrameworkHelper;


namespace sd { namespace framework {

namespace {

typedef ::cppu::WeakComponentImplHelper1 <
      ::com::sun::star::frame::XStatusListener
    > LocalReadOnlyModeObserverInterfaceBase;

/** This local class enables or disables the ResourceManager of a
    ToolPanelModule.  It connects to a ReadOnlyModeObserver and is called
    when the state of the .uno:EditDoc command changes.  When either the
    ResourceManager or the ReadOnlyModeObserver are disposed then the
    LocalReadOnlyModeObserver disposes itself.  The link
    between the ResourceManager and the ReadOnlyModeObserver is removed and
    the ReadOnlyModeObserver typically looses its last reference and is
    destroyed.
*/
class LocalReadOnlyModeObserver
    : private MutexOwner,
      public LocalReadOnlyModeObserverInterfaceBase
{
public:
    LocalReadOnlyModeObserver (
        const Reference<frame::XController>& rxController,
        const ::rtl::Reference<ResourceManager>& rpResourceManager)
        : MutexOwner(),
          LocalReadOnlyModeObserverInterfaceBase(maMutex),
          mpResourceManager(rpResourceManager),
          mpObserver(new ReadOnlyModeObserver(rxController))
    {
        mpObserver->AddStatusListener(this);

        Reference<lang::XComponent> xComponent (
            static_cast<XWeak*>(mpResourceManager.get()), UNO_QUERY);
        if (xComponent.is())
            xComponent->addEventListener(this);
    }

    ~LocalReadOnlyModeObserver (void)
    {
    }

    virtual void SAL_CALL disposing (void)
    {
        Reference<lang::XComponent> xComponent (static_cast<XWeak*>(mpObserver.get()), UNO_QUERY);
        if (xComponent.is())
            xComponent->dispose();

        xComponent = Reference<lang::XComponent>(
            static_cast<XWeak*>(mpResourceManager.get()), UNO_QUERY);
        if (xComponent.is())
            xComponent->removeEventListener(this);

    }

    virtual void SAL_CALL disposing (const com::sun::star::lang::EventObject& rEvent)
        throw(RuntimeException)
    {
        if (rEvent.Source == Reference<XInterface>(static_cast<XWeak*>(mpObserver.get())))
        {
            mpObserver = NULL;
        }
        else if (rEvent.Source == Reference<XInterface>(
            static_cast<XWeak*>(mpResourceManager.get())))
        {
            mpResourceManager = NULL;
        }
        dispose();
    }

    virtual void SAL_CALL statusChanged (const com::sun::star::frame::FeatureStateEvent& rEvent)
        throw(RuntimeException)
    {
        bool bReadWrite (true);
        if (rEvent.IsEnabled)
            rEvent.State >>= bReadWrite;

        if (bReadWrite)
            mpResourceManager->Enable();
        else
            mpResourceManager->Disable();
    }

private:
    ::rtl::Reference<ResourceManager> mpResourceManager;
    ::rtl::Reference<ReadOnlyModeObserver> mpObserver;

};
}




//===== ToolPanelModule ====================================================

void ToolPanelModule::Initialize (const Reference<frame::XController>& rxController)
{
    ::rtl::Reference<ResourceManager> pResourceManager (
        new ResourceManager(
            rxController,
            FrameworkHelper::CreateResourceId(
                FrameworkHelper::msSidebarViewURL,
                FrameworkHelper::msSidebarPaneURL)));
    pResourceManager->AddActiveMainView(FrameworkHelper::msImpressViewURL);
    pResourceManager->AddActiveMainView(FrameworkHelper::msNotesViewURL);
    pResourceManager->AddActiveMainView(FrameworkHelper::msHandoutViewURL);
    pResourceManager->AddActiveMainView(FrameworkHelper::msSlideSorterURL);

    new LocalReadOnlyModeObserver(rxController, pResourceManager);
}




} } // end of namespace sd::framework
