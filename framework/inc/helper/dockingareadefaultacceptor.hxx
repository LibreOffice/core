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

#pragma once

#include <com/sun/star/ui/XDockingAreaAcceptor.hpp>
#include <com/sun/star/frame/XFrame.hpp>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/weakref.hxx>

namespace framework{

class DockingAreaDefaultAcceptor final : public ::cppu::WeakImplHelper< css::ui::XDockingAreaAcceptor >
{
    public:

        //  constructor / destructor

        /*-****************************************************************************************************
            @short      constructor to initialize this instance
            @descr      A docking area acceptor
                        But we need an instance to create more than one enumerations to the same tasklist!

            @seealso    class Desktop
            @seealso    class OTasksEnumeration

            @param      "xOwner" is a reference to our owner and must be the desktop!
            @param      "pTasks" is a pointer to the taskcontainer of the desktop. We need it to create a new enumeration.
            @onerror    Do nothing and reset this object to default with an empty list.
        *//*-*****************************************************************************************************/

         DockingAreaDefaultAcceptor(    const css::uno::Reference< css::frame::XFrame >& xOwner );
        virtual ~DockingAreaDefaultAcceptor() override;

        //  XDockingAreaAcceptor

        virtual css::uno::Reference< css::awt::XWindow > SAL_CALL getContainerWindow() override;
        virtual sal_Bool SAL_CALL requestDockingAreaSpace( const css::awt::Rectangle& RequestedSpace ) override;
        virtual void SAL_CALL setDockingAreaSpace( const css::awt::Rectangle& BorderSpace ) override;

    //  variables
    //  (should be private everyway!)

    private:

        css::uno::WeakReference< css::frame::XFrame >       m_xOwner;   /// weak reference to our frame object!
};

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
