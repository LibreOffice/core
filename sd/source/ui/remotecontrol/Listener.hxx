/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <sal/config.h>
#include <com/sun/star/presentation/XSlideShowListener.hpp>

#include <rtl/ref.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/uno/Reference.hxx>

namespace com::sun::star::presentation { class XSlideShowController; }
namespace sd { class Communicator; }
namespace sd { class Transmitter; }

namespace sd {
/**
 * Slide show listener. This class can also be used for anything else that is
 * specific to the lifetime of one slideshow while a client is connected.
 */
class Listener
    : protected ::cppu::BaseMutex,
      public ::cppu::WeakComponentImplHelper< css::presentation::XSlideShowListener >
{
public:
    Listener( const ::rtl::Reference<Communicator>& rServer, sd::Transmitter *aTransmitter );
    virtual ~Listener() override;
    void init( const css::uno::Reference< css::presentation::XSlideShowController >& aController );

    // XAnimationListener
    virtual void SAL_CALL beginEvent(const css::uno::Reference<
        css::animations::XAnimationNode >&  rNode ) override;
    virtual void SAL_CALL endEvent( const css::uno::Reference<
        css::animations::XAnimationNode >& rNode ) override;
    virtual void SAL_CALL repeat( const css::uno::Reference<
        css::animations::XAnimationNode >& rNode, ::sal_Int32 Repeat ) override;

    // XSlideShowListener
    virtual void SAL_CALL paused(  ) override;
    virtual void SAL_CALL resumed(  ) override;
    virtual void SAL_CALL slideTransitionStarted(  ) override;
    virtual void SAL_CALL slideTransitionEnded(  ) override;
    virtual void SAL_CALL slideAnimationsEnded(  ) override;
    virtual void SAL_CALL slideEnded(sal_Bool bReverse) override;
    virtual void SAL_CALL hyperLinkClicked( const OUString& hyperLink ) override;

    // XEventListener
    virtual void SAL_CALL disposing() override;
    virtual void SAL_CALL disposing (const css::lang::EventObject& rEvent) override;

private:
    rtl::Reference<Communicator> mCommunicator;
    sd::Transmitter *pTransmitter;
    css::uno::Reference< css::presentation::XSlideShowController > mController;
};
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
