/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _SD_IMPRESSREMOTE_LISTENER_HXX
#define _SD_IMPRESSREMOTE_LISTENER_HXX

#include <sal/config.h>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/presentation/XSlideShowListener.hpp>
#include <com/sun/star/presentation/XSlideShowController.hpp>

#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/basemutex.hxx>
#include <osl/socket.hxx>
#include <rtl/ref.hxx>

#include "Server.hxx"
#include "Transmitter.hxx"
#include "ImagePreparer.hxx"

namespace css = ::com::sun::star;

namespace sd {
class Listener
    : protected ::cppu::BaseMutex,
      public ::cppu::WeakComponentImplHelper1< css::presentation::XSlideShowListener >
{
public:
    Listener( const ::rtl::Reference<Server>& rServer, sd::Transmitter *aTransmitter );
    ~Listener();
    void init( const css::uno::Reference< css::presentation::XSlideShowController >& aController );

    // XAnimationListener
    virtual void SAL_CALL beginEvent(const css::uno::Reference<
        css::animations::XAnimationNode >&  rNode ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL endEvent( const css::uno::Reference<
        css::animations::XAnimationNode >& rNode ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL repeat( const css::uno::Reference<
        css::animations::XAnimationNode >& rNode, ::sal_Int32 Repeat )
        throw (css::uno::RuntimeException);

    // XSlideShowListener
    virtual void SAL_CALL paused(  ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL resumed(  ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL slideTransitionStarted(  ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL slideTransitionEnded(  ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL slideAnimationsEnded(  ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL slideEnded(sal_Bool bReverse) throw (css::uno::RuntimeException);
    virtual void SAL_CALL hyperLinkClicked( const ::rtl::OUString& hyperLink )
        throw (css::uno::RuntimeException);

    // XEventListener
    virtual void SAL_CALL disposing (void);
    virtual void SAL_CALL disposing (
        const com::sun::star::lang::EventObject& rEvent)
    throw (com::sun::star::uno::RuntimeException);

private:
    rtl::Reference<Server> mServer;
    sd::Transmitter *pTransmitter;
    css::uno::Reference< css::presentation::XSlideShowController > mController;
    rtl::Reference<sd::ImagePreparer> mPreparer;
};
}
#endif // _SD_IMPRESSREMOTE_LISTENER_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
