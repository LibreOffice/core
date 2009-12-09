/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PresenterCurrentSlideObserver.cxx,v $
 *
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sdext.hxx"

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

void SAL_CALL PresenterCurrentSlideObserver::slideEnded (sal_Bool bReverse)
    throw (css::uno::RuntimeException)
{
    // Determine whether the new current slide (the one after the one that
    // just ended) is the slide past the last slide in the presentation,
    // i.e. the one that says something like "click to end presentation...".
    if (mxSlideShowController.is() && !bReverse)
        if (mxSlideShowController->getNextSlideIndex() < 0)
            if( mpPresenterController.is() )
                mpPresenterController->UpdateCurrentSlide(+1);
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
