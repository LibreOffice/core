/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
