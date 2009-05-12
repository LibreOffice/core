/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: xwin.cxx,v $
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

#include "xwin.hxx"
#include <com/sun/star/lang/SystemDependent.hpp>


using namespace ::com::sun::star;


ContainerWindowWrapper::ContainerWindowWrapper(HWND aHwnd)
    : m_aHwnd(aHwnd),
      m_pDisposeEventListeners(0)
{
}

ContainerWindowWrapper::~ContainerWindowWrapper()
{
    delete m_pDisposeEventListeners;
}


void SAL_CALL
ContainerWindowWrapper::dispose(
)
    throw (
        ::com::sun::star::uno::RuntimeException
    )
{
    cppu::OInterfaceContainerHelper *pDisposeEventListeners(0);

    {
        osl::MutexGuard aGuard(m_aMutex);
        pDisposeEventListeners = m_pDisposeEventListeners;
    }

    if(pDisposeEventListeners) {
        lang::EventObject aEvt;
        aEvt.Source = static_cast< awt::XWindow* >(this);

        pDisposeEventListeners->disposeAndClear(aEvt);
    }
}


void SAL_CALL
ContainerWindowWrapper::addEventListener(
    const ::com::sun::star::uno::Reference<
    ::com::sun::star::lang::XEventListener >& Listener
)
    throw (
        ::com::sun::star::uno::RuntimeException
    )
{
    cppu::OInterfaceContainerHelper *pDisposeEventListeners(0);
    {
        osl::MutexGuard aGuard(m_aMutex);
        pDisposeEventListeners = m_pDisposeEventListeners;
    }

    if(! pDisposeEventListeners)
    {
        osl::MutexGuard aGuard(m_aMutex);
        pDisposeEventListeners = m_pDisposeEventListeners =
            new cppu::OInterfaceContainerHelper(m_aMutex);
    }

    pDisposeEventListeners->addInterface( Listener );
}


void SAL_CALL
ContainerWindowWrapper::removeEventListener(
    const ::com::sun::star::uno::Reference<
    ::com::sun::star::lang::XEventListener >& Listener
)
    throw (
        ::com::sun::star::uno::RuntimeException
    )
{
    cppu::OInterfaceContainerHelper *pDisposeEventListeners(0);
    {
        osl::MutexGuard aGuard(m_aMutex);
        pDisposeEventListeners = m_pDisposeEventListeners;
    }
    if( pDisposeEventListeners )
        pDisposeEventListeners->removeInterface( Listener );
}



// XSystemDependentWindowPeer

::com::sun::star::uno::Any SAL_CALL
ContainerWindowWrapper::getWindowHandle(
    const ::com::sun::star::uno::Sequence< sal_Int8 >& ProcessId,
    sal_Int16 SystemType
)
    throw (
        ::com::sun::star::uno::RuntimeException
    )
{
    if(SystemType == lang::SystemDependent::SYSTEM_WIN32 ||
       SystemType == lang::SystemDependent::SYSTEM_WIN16)
    {
        uno::Any aAny;
        sal_Int32 nHwnd = sal_Int32(m_aHwnd);
        aAny <<= nHwnd;
        return aAny;
    }
    else
        return uno::Any();
}



void SAL_CALL
ContainerWindowWrapper::setPosSize(
    sal_Int32 X,
    sal_Int32 Y,
    sal_Int32 Width,
    sal_Int32 Height,
    sal_Int16 Flags
)
    throw (
        ::com::sun::star::uno::RuntimeException)
{

}

::com::sun::star::awt::Rectangle SAL_CALL
ContainerWindowWrapper::getPosSize(
)
    throw (
        ::com::sun::star::uno::RuntimeException
    )
{
    return awt::Rectangle();
}


void SAL_CALL
ContainerWindowWrapper::setVisible(
    sal_Bool Visible
)
    throw (
        ::com::sun::star::uno::RuntimeException
    )
{

}


void SAL_CALL
ContainerWindowWrapper::setEnable(
    sal_Bool Enable
)
    throw (
        ::com::sun::star::uno::RuntimeException
    )
{

}

void SAL_CALL
ContainerWindowWrapper::setFocus(
)
    throw (
        ::com::sun::star::uno::RuntimeException
    )
{

}

void SAL_CALL
ContainerWindowWrapper::addWindowListener(
    const ::com::sun::star::uno::Reference<
    ::com::sun::star::awt::XWindowListener >& xListener
)
    throw (
        ::com::sun::star::uno::RuntimeException
    )
{

}

void SAL_CALL
ContainerWindowWrapper::removeWindowListener(
    const ::com::sun::star::uno::Reference<
    ::com::sun::star::awt::XWindowListener >& xListener
)
    throw (
        ::com::sun::star::uno::RuntimeException
    )
{

}


void SAL_CALL
ContainerWindowWrapper::addFocusListener(
    const ::com::sun::star::uno::Reference<
    ::com::sun::star::awt::XFocusListener >& xListener
)
    throw (
        ::com::sun::star::uno::RuntimeException
    )
{

}


void SAL_CALL
ContainerWindowWrapper::removeFocusListener(
    const ::com::sun::star::uno::Reference<
    ::com::sun::star::awt::XFocusListener >& xListener
)
    throw (
        ::com::sun::star::uno::RuntimeException
    )
{

}

void SAL_CALL
ContainerWindowWrapper::addKeyListener(
    const ::com::sun::star::uno::Reference<
    ::com::sun::star::awt::XKeyListener >& xListener
)
    throw (
        ::com::sun::star::uno::RuntimeException
    )
{

}

void SAL_CALL
ContainerWindowWrapper::removeKeyListener(
    const ::com::sun::star::uno::Reference<
    ::com::sun::star::awt::XKeyListener >& xListener
)
    throw (
        ::com::sun::star::uno::RuntimeException
    )
{

}


void SAL_CALL
ContainerWindowWrapper::addMouseListener(
    const ::com::sun::star::uno::Reference<
    ::com::sun::star::awt::XMouseListener >& xListener
)
    throw (
        ::com::sun::star::uno::RuntimeException
    )
{

}


void SAL_CALL
ContainerWindowWrapper::removeMouseListener(
    const ::com::sun::star::uno::Reference<
    ::com::sun::star::awt::XMouseListener >& xListener
)
    throw (
        ::com::sun::star::uno::RuntimeException
    )
{

}


void SAL_CALL
ContainerWindowWrapper::addMouseMotionListener(
    const ::com::sun::star::uno::Reference<
    ::com::sun::star::awt::XMouseMotionListener >& xListener
)
    throw (
        ::com::sun::star::uno::RuntimeException
    )
{

}

void SAL_CALL
ContainerWindowWrapper::removeMouseMotionListener(
    const ::com::sun::star::uno::Reference<
    ::com::sun::star::awt::XMouseMotionListener >& xListener
)
    throw (
        ::com::sun::star::uno::RuntimeException
    )
{

}

void SAL_CALL
ContainerWindowWrapper::addPaintListener(
    const ::com::sun::star::uno::Reference<
    ::com::sun::star::awt::XPaintListener >& xListener
)
    throw (
        ::com::sun::star::uno::RuntimeException
    )
{

}

void SAL_CALL
ContainerWindowWrapper::removePaintListener(
    const ::com::sun::star::uno::Reference<
    ::com::sun::star::awt::XPaintListener >& xListener
)
    throw (
        ::com::sun::star::uno::RuntimeException
    )
{

}
