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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sdext.hxx"

#include "PresenterFrameworkObserver.hxx"
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <boost/bind.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

using ::rtl::OUString;

#define A2S(pString) (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(pString)))

namespace sdext { namespace presenter {

PresenterFrameworkObserver::PresenterFrameworkObserver (
    const css::uno::Reference<css::drawing::framework::XConfigurationController>&rxController,
    const OUString& rsEventName,
    const Predicate& rPredicate,
    const Action& rAction)
    : PresenterFrameworkObserverInterfaceBase(m_aMutex),
      mxConfigurationController(rxController),
      maPredicate(rPredicate),
      maAction(rAction)
{
    if ( ! mxConfigurationController.is())
        throw lang::IllegalArgumentException();

    if (mxConfigurationController->hasPendingRequests())
    {
        if (rsEventName.getLength() > 0)
        {
            mxConfigurationController->addConfigurationChangeListener(
                this,
                rsEventName,
                Any());
        }
        mxConfigurationController->addConfigurationChangeListener(
            this,
            A2S("ConfigurationUpdateEnd"),
            Any());
    }
    else
    {
        rAction(maPredicate());
    }
}




PresenterFrameworkObserver::~PresenterFrameworkObserver (void)
{
}




void PresenterFrameworkObserver::RunOnResourceActivation (
    const css::uno::Reference<css::drawing::framework::XConfigurationController>&rxController,
    const css::uno::Reference<css::drawing::framework::XResourceId>& rxResourceId,
    const Action& rAction)
{
    new PresenterFrameworkObserver(
        rxController,
        A2S("ResourceActivation"),
        ::boost::bind(&PresenterFrameworkObserver::HasResource, rxController, rxResourceId),
        rAction);
}




void PresenterFrameworkObserver::RunOnUpdateEnd (
    const css::uno::Reference<css::drawing::framework::XConfigurationController>&rxController,
    const Action& rAction)
{
    new PresenterFrameworkObserver(
        rxController,
        OUString(),
        &PresenterFrameworkObserver::True,
        rAction);
}




bool PresenterFrameworkObserver::HasResource (
    const css::uno::Reference<css::drawing::framework::XConfigurationController>&rxController,
    const css::uno::Reference<css::drawing::framework::XResourceId>& rxResourceId)
{
    return rxController.is() && rxController->getResource(rxResourceId).is();
}




bool PresenterFrameworkObserver::True (void)
{
    return true;
}




bool PresenterFrameworkObserver::False (void)
{
    return false;
}




void SAL_CALL PresenterFrameworkObserver::disposing (void)
{
    if ( ! maAction.empty())
        maAction(false);
    Shutdown();
}




void PresenterFrameworkObserver::Shutdown (void)
{
    maAction = Action();
    maPredicate = Predicate();

    if (mxConfigurationController != NULL)
    {
        mxConfigurationController->removeConfigurationChangeListener(this);
        mxConfigurationController = NULL;
    }
}




void SAL_CALL PresenterFrameworkObserver::disposing (const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    if ( ! rEvent.Source.is())
        return;

    if (rEvent.Source == mxConfigurationController)
    {
        mxConfigurationController = NULL;
        if ( ! maAction.empty())
            maAction(false);
    }
}




void SAL_CALL PresenterFrameworkObserver::notifyConfigurationChange (
    const ConfigurationChangeEvent& rEvent)
    throw (RuntimeException)
{
    bool bDispose(false);

    Action aAction (maAction);
    Predicate aPredicate (maPredicate);
    if (rEvent.Type.equals(A2S("ConfigurationUpdateEnd")))
    {
        Shutdown();
        aAction(aPredicate);
        bDispose = true;
    }
    else if (aPredicate())
    {
        Shutdown();
        aAction(true);
        bDispose = true;
    }

    if (bDispose)
    {
        maAction.clear();
        dispose();
    }
}

} }  // end of namespace ::sdext::presenter
