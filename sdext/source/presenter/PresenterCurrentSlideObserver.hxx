/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef SDEXT_PRESENTER_CURRENT_SLIDE_OBSERVER
#define SDEXT_PRESENTER_CURRENT_SLIDE_OBSERVER

#include "PresenterController.hxx"
#include <com/sun/star/presentation/XSlideShow.hpp>
#include <com/sun/star/presentation/XSlideShowController.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/basemutex.hxx>
#include <rtl/ref.hxx>
#include <salhelper/timer.hxx>

namespace css = ::com::sun::star;

namespace sdext { namespace presenter {

namespace {
    typedef ::cppu::WeakComponentImplHelper1 <
        css::presentation::XSlideShowListener
    > PresenterCurrentSlideObserverInterfaceBase;
}

/** Check periodically the slide show controller and the
    frame::XController whether the current slide has changed.  If so,
    then inform the presenter controller about it.

    Objects of this class have their own lifetime control and destroy
    themselves when the presenter controller is diposed.
*/
class PresenterCurrentSlideObserver
    : protected ::cppu::BaseMutex,
      public PresenterCurrentSlideObserverInterfaceBase
{
public:
    PresenterCurrentSlideObserver (
        const ::rtl::Reference<PresenterController>& rxPresenterController,
        const css::uno::Reference<css::presentation::XSlideShowController>& rxSlideShowController);
    virtual ~PresenterCurrentSlideObserver (void);

    virtual void SAL_CALL disposing (void);

    // XSlideShowListener
    virtual void SAL_CALL paused(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL resumed(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL slideTransitionStarted(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL slideTransitionEnded(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL slideAnimationsEnded(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL slideEnded(sal_Bool bReverse) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL hyperLinkClicked( const ::rtl::OUString& hyperLink ) throw (::com::sun::star::uno::RuntimeException);

    // XAnimationListener
    virtual void SAL_CALL beginEvent( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& Node ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL endEvent( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& Node ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL repeat( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& Node, ::sal_Int32 Repeat ) throw (::com::sun::star::uno::RuntimeException);

    // XEventListener
    virtual void SAL_CALL disposing (
        const com::sun::star::lang::EventObject& rEvent)
        throw (com::sun::star::uno::RuntimeException);

private:
    ::rtl::Reference<PresenterController> mpPresenterController;
    css::uno::Reference<css::presentation::XSlideShowController> mxSlideShowController;
};

} } // end of namespace ::sdext::presenter

#endif
