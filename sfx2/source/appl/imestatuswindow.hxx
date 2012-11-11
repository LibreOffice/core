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

#ifndef INCLUDED_SFX2_APPL_IMESTATUSWINDOW_HXX
#define INCLUDED_SFX2_APPL_IMESTATUSWINDOW_HXX

#include "com/sun/star/beans/XPropertyChangeListener.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "cppuhelper/implbase1.hxx"
#include "osl/mutex.hxx"

namespace com { namespace sun { namespace star {
    namespace beans { class XPropertySet; }
    namespace lang { class XMultiServiceFactory; }
} } }

namespace sfx2 { namespace appl {

// The MS compiler needs this typedef work-around to accept the using
// declarations within ImeStatusWindow:
typedef cppu::WeakImplHelper1< com::sun::star::beans::XPropertyChangeListener >
ImeStatusWindow_Impl;

/** Control the behavior of any (platform-dependent) IME status windows.

    The decision of whether a status window shall be displayed or not can be
    stored permanently in the configuration (under key
    org.openoffice.office.Common/I18N/InputMethod/ShowStatusWindow; if that
    entry is nil, VCL is asked for a default).
 */
class ImeStatusWindow: private ImeStatusWindow_Impl
{
public:
    ImeStatusWindow( com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > const& rxContext );

    /** Set up VCL according to the configuration.

        Is it not strictly required that this method is called exactly once
        (though that will be the typical use).

        Must only be called with the Solar mutex locked.
     */
    void init();

    /** Return true if the status window is toggled on.

        This is only meaningful when canToggle returns true.

        Can be called without the Solar mutex locked.
     */
    bool isShowing();

    /** Toggle the status window on or off.

        This only works if canToggle returns true (otherwise, any calls of this
        method are ignored).

        Must only be called with the Solar mutex locked.
     */
    void show(bool bShow);

    /** Return true if the status window can be toggled on and off externally.

        Must only be called with the Solar mutex locked.
     */
    bool canToggle() const;

    using ImeStatusWindow_Impl::acquire;
    using ImeStatusWindow_Impl::release;
    using ImeStatusWindow_Impl::operator new;
    using ImeStatusWindow_Impl::operator delete;

private:
    ImeStatusWindow(ImeStatusWindow &); // not implemented
    void operator =(ImeStatusWindow); // not implemented

    virtual ~ImeStatusWindow();

    virtual void SAL_CALL
    disposing(com::sun::star::lang::EventObject const & rSource)
        throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
    propertyChange(com::sun::star::beans::PropertyChangeEvent const & rEvent)
        throw (com::sun::star::uno::RuntimeException);

    com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet >
    getConfig();

    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >
        m_xContext;

    osl::Mutex m_aMutex;
    com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet >
        m_xConfig;
    com::sun::star::uno::Reference< com::sun::star::beans::XPropertyChangeListener >
        m_xConfigListener;
    bool m_bDisposed;
};

} }

#endif // INCLUDED_SFX2_APPL_IMESTATUSWINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
