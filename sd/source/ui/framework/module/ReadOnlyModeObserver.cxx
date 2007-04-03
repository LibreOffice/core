/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ReadOnlyModeObserver.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-03 15:53:35 $
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

#include "ReadOnlyModeObserver.hxx"

#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/interfacecontainer.hxx>

#include "tools/SlotStateListener.hxx"
#include "framework/FrameworkHelper.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::com::sun::star::drawing::framework;
using ::rtl::OUString;

namespace sd { namespace framework {

class ReadOnlyModeObserver::ModifyBroadcaster
    : public ::cppu::OBroadcastHelper
{
public:
    explicit ModifyBroadcaster (::osl::Mutex& rOslMutex) : ::cppu::OBroadcastHelper(rOslMutex) {}
};




ReadOnlyModeObserver::ReadOnlyModeObserver (
    const Reference<frame::XController>& rxController)
    : ReadOnlyModeObserverInterfaceBase(maMutex),
      maSlotNameURL(),
      mxController(rxController),
      mxConfigurationController(NULL),
      mxDispatch(NULL),
      mpBroadcaster(new ModifyBroadcaster(maMutex))
{
    // Create a URL object for the slot name.
    maSlotNameURL.Complete = OUString::createFromAscii(".uno:EditDoc");
    uno::Reference<lang::XMultiServiceFactory> xServiceManager (
        ::comphelper::getProcessServiceFactory());
    if (xServiceManager.is())
    {
        Reference<util::XURLTransformer> xTransformer(xServiceManager->createInstance(
            OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.util.URLTransformer"))),
            UNO_QUERY);
        if (xTransformer.is())
            xTransformer->parseStrict(maSlotNameURL);
    }

    if ( ! ConnectToDispatch())
    {
        // The controller is not yet connected to a frame.  This means that
        // the dispatcher is not yet set up.  We wait for this to happen by
        // waiting for configuration updates and try again.
        Reference<XControllerManager> xControllerManager (rxController, UNO_QUERY);
        if (xControllerManager.is())
        {
            mxConfigurationController = xControllerManager->getConfigurationController();
            if (mxConfigurationController.is())
            {
                mxConfigurationController->addConfigurationChangeListener(
                    this,
                    FrameworkHelper::msConfigurationUpdateStartEvent,
                    Any());
            }
        }
    }
}




ReadOnlyModeObserver::~ReadOnlyModeObserver (void)
{
}




void SAL_CALL ReadOnlyModeObserver::disposing (void)
{
    if (mxController.is())
    {
        mxController = NULL;
    }
    if (mxConfigurationController.is())
    {
        mxConfigurationController->removeConfigurationChangeListener(this);
        mxConfigurationController = NULL;
    }
    if (mxDispatch.is())
    {
        mxDispatch->removeStatusListener(this, maSlotNameURL);
        mxDispatch = NULL;
    }

    lang::EventObject aEvent;
    aEvent.Source = static_cast<XWeak*>(this);
    ::cppu::OInterfaceContainerHelper* pIterator
        = mpBroadcaster->getContainer(getCppuType((Reference<frame::XStatusListener>*)NULL));
    pIterator->disposeAndClear(aEvent);
}




void ReadOnlyModeObserver::AddStatusListener (
    const Reference<frame::XStatusListener>& rxListener)
{
    mpBroadcaster->addListener(
        getCppuType((Reference<frame::XStatusListener>*)NULL),
        rxListener);
}




void ReadOnlyModeObserver::RemoveStatusListener (
    const Reference<frame::XStatusListener>& rxListener)
{
    mpBroadcaster->removeListener(
        getCppuType((Reference<frame::XStatusListener>*)NULL),
        rxListener);
}




bool ReadOnlyModeObserver::ConnectToDispatch (void)
{
    if ( ! mxDispatch.is())
    {
        // Get the dispatch object.
        Reference<frame::XDispatchProvider> xProvider (mxController->getFrame(), UNO_QUERY);
        if (xProvider.is())
        {
            mxDispatch = xProvider->queryDispatch(maSlotNameURL, OUString(), 0);
            if (mxDispatch.is())
            {
                mxDispatch->addStatusListener(this, maSlotNameURL);
            }
        }
    }

    return mxDispatch.is();
}




void ReadOnlyModeObserver::statusChanged (const frame::FeatureStateEvent& rEvent)
    throw (RuntimeException)
{
    ::cppu::OInterfaceContainerHelper* pIterator
          = mpBroadcaster->getContainer(getCppuType((Reference<frame::XStatusListener>*)NULL));
    if (pIterator != NULL)
    {
        pIterator->notifyEach(&frame::XStatusListener::statusChanged, rEvent);
    }
}




//----- XEventListener --------------------------------------------------------

void SAL_CALL ReadOnlyModeObserver::disposing (
    const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    if (rEvent.Source == mxConfigurationController)
        mxConfigurationController = NULL;
    else if (rEvent.Source == mxDispatch)
        mxDispatch = NULL;

    dispose();
}




void SAL_CALL ReadOnlyModeObserver::notifyConfigurationChange (
    const ConfigurationChangeEvent& rEvent)
    throw (RuntimeException)
{
    if (rEvent.Type.equals(FrameworkHelper::msConfigurationUpdateStartEvent))
    {
        if (mxController.is() && mxController->getFrame().is())
        {
            if (ConnectToDispatch())
            {
                // We have connected successfully to the dispatcher and
                // therefore can disconnect from the configuration controller.
                mxConfigurationController->removeConfigurationChangeListener(this);
                mxConfigurationController = NULL;
            }
        }
    }
}

} } // end of namespace sd::framework
