/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterFrameworkObserver.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 15:58:11 $
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
        mxConfigurationController->addConfigurationChangeListener(
            this,
            rsEventName,
            Any());
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




bool PresenterFrameworkObserver::HasResource (
    const css::uno::Reference<css::drawing::framework::XConfigurationController>&rxController,
    const css::uno::Reference<css::drawing::framework::XResourceId>& rxResourceId)
{
    return rxController.is() && rxController->getResource(rxResourceId).is();
}




void SAL_CALL PresenterFrameworkObserver::disposing (void)
{
    if ( ! maAction.empty())
        maAction(false);
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

    if (rEvent.Type.equals(A2S("ConfigurationUpdateEnd")))
    {
        maAction(maPredicate);
        bDispose = true;
    }
    else if (maPredicate())
    {
        maAction(true);
        bDispose = true;
    }

    if (bDispose)
    {
        maAction.clear();
        dispose();
    }
}

} }  // end of namespace ::sdext::presenter
