/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SD_SOURCE_UI_REMOTECONTROL_LISTENER_HXX
#define INCLUDED_SD_SOURCE_UI_REMOTECONTROL_LISTENER_HXX

#include <sal/config.h>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/presentation/XSlideShowListener.hpp>
#include <com/sun/star/presentation/XSlideShowController.hpp>

#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/basemutex.hxx>
#include <osl/socket.hxx>

#include "Communicator.hxx"
#include "Transmitter.hxx"

namespace sd {
/**
 * Slide show listener. This class can also be used for anything else that is
 * specific to the lifetime of one slideshow while a client is connected.
 */
class Listener
    : protected ::cppu::BaseMutex,
      public ::cppu::WeakComponentImplHelper1< css::presentation::XSlideShowListener >
{
public:
    Listener( const ::rtl::Reference<Communicator>& rServer, sd::Transmitter *aTransmitter );
    virtual ~Listener();
    void init( const css::uno::Reference< css::presentation::XSlideShowController >& aController );

    // XAnimationListener
    virtual void SAL_CALL beginEvent(const css::uno::Reference<
        css::animations::XAnimationNode >&  rNode ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL endEvent( const css::uno::Reference<
        css::animations::XAnimationNode >& rNode ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL repeat( const css::uno::Reference<
        css::animations::XAnimationNode >& rNode, ::sal_Int32 Repeat )
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XSlideShowListener
    virtual void SAL_CALL paused(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL resumed(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL slideTransitionStarted(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL slideTransitionEnded(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL slideAnimationsEnded(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL slideEnded(sal_Bool bReverse) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL hyperLinkClicked( const OUString& hyperLink )
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XEventListener
    virtual void SAL_CALL disposing() SAL_OVERRIDE;
    virtual void SAL_CALL disposing (
        const com::sun::star::lang::EventObject& rEvent)
    throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

private:
    rtl::Reference<Communicator> mCommunicator;
    sd::Transmitter *pTransmitter;
    css::uno::Reference< css::presentation::XSlideShowController > mController;
};
}
#endif // INCLUDED_SD_SOURCE_UI_REMOTECONTROL_LISTENER_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
