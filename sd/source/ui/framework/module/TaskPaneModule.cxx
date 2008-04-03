/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TaskPaneModule.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 13:40:47 $
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

#include "TaskPaneModule.hxx"
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
    TaskPaneModule.  It connects to a ReadOnlyModeObserver and is called
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




//===== TaskPaneModule ====================================================

void TaskPaneModule::Initialize (const Reference<frame::XController>& rxController)
{
    ::rtl::Reference<ResourceManager> pResourceManager (
        new ResourceManager(
            rxController,
            FrameworkHelper::CreateResourceId(
                FrameworkHelper::msTaskPaneURL,
                FrameworkHelper::msRightPaneURL)));
    pResourceManager->AddActiveMainView(FrameworkHelper::msImpressViewURL);
    pResourceManager->AddActiveMainView(FrameworkHelper::msNotesViewURL);
    pResourceManager->AddActiveMainView(FrameworkHelper::msHandoutViewURL);
    pResourceManager->AddActiveMainView(FrameworkHelper::msSlideSorterURL);

    new LocalReadOnlyModeObserver(rxController, pResourceManager);
}




} } // end of namespace sd::framework
