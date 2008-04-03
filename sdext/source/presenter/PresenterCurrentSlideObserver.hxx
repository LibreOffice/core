/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterCurrentSlideObserver.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 15:58:00 $
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

#ifndef SDEXT_PRESENTER_CURRENT_SLIDE_OBSERVER
#define SDEXT_PRESENTER_CURRENT_SLIDE_OBSERVER

#include "PresenterController.hxx"
#include <com/sun/star/presentation/XSlideShow.hpp>
#include <com/sun/star/presentation/XSlideShowController.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/basemutex.hxx>
#include <rtl/ref.hxx>
#include <vos/timer.hxx>

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
    virtual void SAL_CALL slideEnded(  ) throw (::com::sun::star::uno::RuntimeException);
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
