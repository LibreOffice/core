/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterCurrentSlideObserver.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 15:57:50 $
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

#include "PresenterCurrentSlideObserver.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;

namespace sdext { namespace presenter {

//===== PresenterCurrentSlideObserver =========================================

PresenterCurrentSlideObserver::PresenterCurrentSlideObserver (
    const ::rtl::Reference<PresenterController>& rxPresenterController,
    const Reference<presentation::XSlideShowController>& rxSlideShowController)
    : PresenterCurrentSlideObserverInterfaceBase(m_aMutex),
      mpPresenterController(rxPresenterController),
      mxSlideShowController(rxSlideShowController)
{
    if( mpPresenterController.is() )
    {
        mpPresenterController->addEventListener(this);
    }

    if( mxSlideShowController.is() )
    {
        // Listen for events from the slide show controller.
        mxSlideShowController->addSlideShowListener(static_cast<XSlideShowListener*>(this));
    }
}

PresenterCurrentSlideObserver::~PresenterCurrentSlideObserver (void)
{
}

void SAL_CALL PresenterCurrentSlideObserver::disposing (void)
{
    // Disconnect form the slide show controller.
    if(mxSlideShowController.is())
    {
        mxSlideShowController->removeSlideShowListener(static_cast<XSlideShowListener*>(this));
        mxSlideShowController = NULL;
    }
}

//----- XSlideShowListener ----------------------------------------------------

void SAL_CALL PresenterCurrentSlideObserver::beginEvent (
    const Reference<animations::XAnimationNode>& rNode)
    throw (css::uno::RuntimeException)
{
    (void)rNode;
}

void SAL_CALL PresenterCurrentSlideObserver::endEvent (
    const Reference<animations::XAnimationNode>& rNode)
    throw(css::uno::RuntimeException)
{
    (void)rNode;
}

void SAL_CALL PresenterCurrentSlideObserver::repeat (
    const css::uno::Reference<css::animations::XAnimationNode>& rNode,
    sal_Int32)
    throw (com::sun::star::uno::RuntimeException)
{
    (void)rNode;
}

void SAL_CALL PresenterCurrentSlideObserver::paused (void)
    throw (com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL PresenterCurrentSlideObserver::resumed (void)
    throw (css::uno::RuntimeException)
{
}

void SAL_CALL PresenterCurrentSlideObserver::slideEnded (void)
    throw (css::uno::RuntimeException)
{
}

void SAL_CALL PresenterCurrentSlideObserver::hyperLinkClicked (const rtl::OUString &)
    throw (css::uno::RuntimeException)
{
}

void SAL_CALL PresenterCurrentSlideObserver::slideTransitionStarted (void)
    throw (css::uno::RuntimeException)
{
    if( mpPresenterController.is() )
        mpPresenterController->UpdateCurrentSlide(0);
}

void SAL_CALL PresenterCurrentSlideObserver::slideTransitionEnded (void)
    throw (css::uno::RuntimeException)
{
}

void SAL_CALL PresenterCurrentSlideObserver::slideAnimationsEnded (void)
    throw (css::uno::RuntimeException)
{
}

//----- XEventListener --------------------------------------------------------

void SAL_CALL PresenterCurrentSlideObserver::disposing (
    const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    if (rEvent.Source == Reference<XInterface>(static_cast<XWeak*>(mpPresenterController.get())))
        dispose();
    else if (rEvent.Source == mxSlideShowController)
        mxSlideShowController = NULL;
}

} } // end of namespace ::sdext::presenter
