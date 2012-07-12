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
#include <com/sun/star/presentation/XSlideShowListener.hpp>
#include <com/sun/star/presentation/XSlideShowController.hpp>

#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/basemutex.hxx>
#include <osl/socket.hxx>

namespace css = ::com::sun::star;

namespace sd {
class Listener
    : protected ::cppu::BaseMutex,
      public ::cppu::WeakComponentImplHelper1< css::presentation::XSlideShowListener >
{
public:
    Listener( const css::uno::Reference<css::presentation::XSlideShowController>& rxSlideShowController,
        osl::StreamSocket aSocket );
    ~Listener();

    // XSlideShowListener
    virtual void SAL_CALL paused(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL resumed(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL slideTransitionStarted(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL slideTransitionEnded(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL slideAnimationsEnded(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL slideEnded(sal_Bool bReverse) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL hyperLinkClicked( const ::rtl::OUString& hyperLink ) throw (::com::sun::star::uno::RuntimeException);

private:
    css::uno::Reference<css::presentation::XSlideShowController> mxSlideShowController;
    osl::StreamSocket mStreamSocket;
};
}
#endif // _SD_IMPRESSREMOTE_LISTENER_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
