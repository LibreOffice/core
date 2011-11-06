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
