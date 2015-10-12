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
#ifndef INCLUDED_SVX_SOURCE_INC_DATALISTENER_HXX
#define INCLUDED_SVX_SOURCE_INC_DATALISTENER_HXX

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/frame/XFrameActionListener.hpp>
#include <com/sun/star/xml/dom/events/XEventListener.hpp>
#include <vcl/vclptr.hxx>


namespace svxform
{


    class DataNavigatorWindow;

    typedef cppu::WeakImplHelper<
        com::sun::star::container::XContainerListener,
        com::sun::star::frame::XFrameActionListener,
        com::sun::star::xml::dom::events::XEventListener > DataListener_t;

    class DataListener : public DataListener_t
    {
    private:
        VclPtr<DataNavigatorWindow>        m_pNaviWin;

    public:
        DataListener( DataNavigatorWindow* pNaviWin );

    protected:
        virtual ~DataListener();

    public:
        // XContainerListener
        virtual void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        // XFrameActionListener
        virtual void SAL_CALL frameAction( const ::com::sun::star::frame::FrameActionEvent& Action ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        // xml::dom::events::XEventListener
        virtual void SAL_CALL handleEvent( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::dom::events::XEvent >& evt ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        // lang::XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    };


}


#endif // INCLUDED_SVX_SOURCE_INC_DATALISTENER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
