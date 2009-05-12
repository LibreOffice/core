/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: xwin.hxx,v $
 * $Revision: 1.4 $
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

#ifndef _XWIN_HXX_
#define _XWIN_HXX_


#include "common.h"
#include <osl/mutex.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XSystemDependentWindowPeer.hpp>


class ContainerWindowWrapper:
    public ::cppu::WeakImplHelper2<
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
