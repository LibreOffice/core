/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERCURRENTSLIDEOBSERVER_HXX
#define INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERCURRENTSLIDEOBSERVER_HXX

#include "PresenterController.hxx"
#include <com/sun/star/presentation/XSlideShow.hpp>
#include <com/sun/star/presentation/XSlideShowController.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <rtl/ref.hxx>
#include <salhelper/timer.hxx>

namespace sdext { namespace presenter {

namespace {
    typedef ::cppu::WeakComponentImplHelper <
        css::presentation::XSlideShowListener
    > PresenterCurrentSlideObserverInterfaceBase;
}

/** Check periodically the slide show controller and the
    frame::XController whether the current slide has changed.  If so,
    then inform the presenter controller about it.

    Objects of this class have their own lifetime control and destroy
    themselves when the presenter controller is disposed.
*/
class PresenterCurrentSlideObserver
    : protected ::cppu::BaseMutex,
      public PresenterCurrentSlideObserverInterfaceBase
{
public:
    PresenterCurrentSlideObserver (
        const ::rtl::Reference<PresenterController>& rxPresenterController,
        const css::uno::Reference<css::presentation::XSlideShowController>& rxSlideShowController);
    virtual ~PresenterCurrentSlideObserver();

    virtual void SAL_CALL disposing() override;

    // XSlideShowListener
    virtual void SAL_CALL paused(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL resumed(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL slideTransitionStarted(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL slideTransitionEnded(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL slideAnimationsEnded(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL slideEnded(sal_Bool bReverse) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL hyperLinkClicked( const OUString& hyperLink ) throw (css::uno::RuntimeException, std::exception) override;

    // XAnimationListener
    virtual void SAL_CALL beginEvent( const css::uno::Reference< css::animations::XAnimationNode >& Node ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL endEvent( const css::uno::Reference< css::animations::XAnimationNode >& Node ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL repeat( const css::uno::Reference< css::animations::XAnimationNode >& Node, ::sal_Int32 Repeat ) throw (css::uno::RuntimeException, std::exception) override;

    // XEventListener
    virtual void SAL_CALL disposing (
        const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException, std::exception) override;

private:
    ::rtl::Reference<PresenterController> mpPresenterController;
    css::uno::Reference<css::presentation::XSlideShowController> mxSlideShowController;
};

} } // end of namespace ::sdext::presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
