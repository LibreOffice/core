/*************************************************************************
 *
 *  $RCSfile: xwin.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-02-25 17:11:18 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _XWIN_HXX_
#define _XWIN_HXX_


#include "common.h"

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XSYSTEMDEPENDENTWINDOWPEER_HPP_
#include <com/sun/star/awt/XSystemDependentWindowPeer.hpp>
#endif


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
