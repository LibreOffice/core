/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifndef _SVX_DATALISTENER_HXX
#define _SVX_DATALISTENER_HXX

#include <cppuhelper/implbase3.hxx>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/frame/XFrameActionListener.hpp>
#include <com/sun/star/xml/dom/events/XEventListener.hpp>

//............................................................................
namespace svxform
{
//............................................................................

    class DataNavigatorWindow;

    typedef cppu::WeakImplHelper3<
        com::sun::star::container::XContainerListener,
        com::sun::star::frame::XFrameActionListener,
        com::sun::star::xml::dom::events::XEventListener > DataListener_t;

    class DataListener : public DataListener_t
    {
    private:
        DataNavigatorWindow*        m_pNaviWin;

    public:
        DataListener( DataNavigatorWindow* pNaviWin );

    protected:
        ~DataListener();

    public:
        // XContainerListener
        virtual void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException);

        // XFrameActionListener
        virtual void SAL_CALL frameAction( const ::com::sun::star::frame::FrameActionEvent& Action ) throw (::com::sun::star::uno::RuntimeException);

        // xml::dom::events::XEventListener
        virtual void SAL_CALL handleEvent( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::dom::events::XEvent >& evt ) throw (::com::sun::star::uno::RuntimeException);

        // lang::XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);
    };

//............................................................................
}   // namespace svxform
//............................................................................

#endif // #ifndef _SVX_DATALISTENER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
