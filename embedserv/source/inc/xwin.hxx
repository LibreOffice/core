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

#ifndef INCLUDED_EMBEDSERV_SOURCE_INC_XWIN_HXX
#define INCLUDED_EMBEDSERV_SOURCE_INC_XWIN_HXX


#include "common.h"
#include <osl/mutex.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XSystemDependentWindowPeer.hpp>


class ContainerWindowWrapper:
    public ::cppu::WeakImplHelper<
                 ::com::sun::star::awt::XWindow,
                 ::com::sun::star::awt::XSystemDependentWindowPeer>
{
public:

    ContainerWindowWrapper(HWND aHwnd);

    ~ ContainerWindowWrapper();


    // XComponent

    virtual void SAL_CALL
    dispose(
    )
        throw (
            ::com::sun::star::uno::RuntimeException
        );

    virtual void SAL_CALL
    addEventListener(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::lang::XEventListener >& xListener
    )
        throw (
            ::com::sun::star::uno::RuntimeException
        );

    virtual void SAL_CALL
    removeEventListener(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::lang::XEventListener >& aListener
    )
        throw (
            ::com::sun::star::uno::RuntimeException
        );


    // XSystemDependentWindowPeer

    virtual ::com::sun::star::uno::Any SAL_CALL
    getWindowHandle(
        const ::com::sun::star::uno::Sequence< sal_Int8 >& ProcessId,
        sal_Int16 SystemType
    )
        throw (
            ::com::sun::star::uno::RuntimeException
        );

    // XWindow

    virtual void SAL_CALL
    setPosSize(
        sal_Int32 X,
        sal_Int32 Y,
        sal_Int32 Width,
        sal_Int32 Height,
        sal_Int16 Flags
    )
        throw (
            ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::awt::Rectangle SAL_CALL
    getPosSize(
    )
        throw (
            ::com::sun::star::uno::RuntimeException
        );

    virtual void SAL_CALL
    setVisible(
        sal_Bool Visible
    )
        throw (
            ::com::sun::star::uno::RuntimeException
        );

    virtual void SAL_CALL
    setEnable(
        sal_Bool Enable
    )
        throw (
            ::com::sun::star::uno::RuntimeException
        );

    virtual void SAL_CALL
    setFocus(
    )
        throw (
            ::com::sun::star::uno::RuntimeException
        );

    virtual void SAL_CALL
    addWindowListener(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::awt::XWindowListener >& xListener
    )
        throw (
            ::com::sun::star::uno::RuntimeException
        );

    virtual void SAL_CALL
    removeWindowListener(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::awt::XWindowListener >& xListener
    )
        throw (
            ::com::sun::star::uno::RuntimeException
        );

    virtual void SAL_CALL
    addFocusListener(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::awt::XFocusListener >& xListener
    )
        throw (
            ::com::sun::star::uno::RuntimeException
        );

    virtual void SAL_CALL
    removeFocusListener(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::awt::XFocusListener >& xListener
    )
        throw (
            ::com::sun::star::uno::RuntimeException
        );

    virtual void SAL_CALL
    addKeyListener(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::awt::XKeyListener >& xListener
    )
        throw (
            ::com::sun::star::uno::RuntimeException
        );

    virtual void SAL_CALL
    removeKeyListener(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::awt::XKeyListener >& xListener
    )
        throw (
            ::com::sun::star::uno::RuntimeException
        );

    virtual void SAL_CALL
    addMouseListener(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::awt::XMouseListener >& xListener
    )
        throw (
            ::com::sun::star::uno::RuntimeException
        );

    virtual void SAL_CALL
    removeMouseListener(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::awt::XMouseListener >& xListener
    )
        throw (
            ::com::sun::star::uno::RuntimeException
        );

    virtual void SAL_CALL
    addMouseMotionListener(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::awt::XMouseMotionListener >& xListener
    )
        throw (
            ::com::sun::star::uno::RuntimeException
        );

    virtual void SAL_CALL
    removeMouseMotionListener(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::awt::XMouseMotionListener >& xListener
    )
        throw (
            ::com::sun::star::uno::RuntimeException
        );

    virtual void SAL_CALL
    addPaintListener(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::awt::XPaintListener >& xListener
    )
        throw (
            ::com::sun::star::uno::RuntimeException
        );

    virtual void SAL_CALL
    removePaintListener(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::awt::XPaintListener >& xListener
    )
        throw (
            ::com::sun::star::uno::RuntimeException
        );


private:

    osl::Mutex m_aMutex;
    cppu::OInterfaceContainerHelper *m_pDisposeEventListeners;

    HWND m_aHwnd;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
