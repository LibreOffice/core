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

#ifndef INCLUDED_TOOLKIT_SOURCE_CONTROLS_GRID_GRIDEVENTFORWARDER_HXX
#define INCLUDED_TOOLKIT_SOURCE_CONTROLS_GRID_GRIDEVENTFORWARDER_HXX

#include <com/sun/star/awt/grid/XGridDataListener.hpp>
#include <com/sun/star/awt/grid/XGridColumnListener.hpp>
#include <com/sun/star/container/XContainerListener.hpp>

#include <cppuhelper/implbase.hxx>


namespace toolkit
{


    class UnoGridControl;


    //= GridEventForwarder

    typedef ::cppu::ImplHelper  <   css::awt::grid::XGridDataListener
                                ,   css::container::XContainerListener
                                >   GridEventForwarder_Base;

    class GridEventForwarder : public GridEventForwarder_Base
    {
    public:
        explicit GridEventForwarder( UnoGridControl& i_parent );
        virtual ~GridEventForwarder();

    public:
        // XInterface
        virtual void SAL_CALL acquire() throw() override;
        virtual void SAL_CALL release() throw() override;

        // XGridDataListener
        virtual void SAL_CALL rowsInserted( const css::awt::grid::GridDataEvent& Event ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL rowsRemoved( const css::awt::grid::GridDataEvent& Event ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL dataChanged( const css::awt::grid::GridDataEvent& Event ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL rowHeadingChanged( const css::awt::grid::GridDataEvent& Event ) throw (css::uno::RuntimeException, std::exception) override;

        // XContainerListener
        virtual void SAL_CALL elementInserted( const css::container::ContainerEvent& Event ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL elementRemoved( const css::container::ContainerEvent& Event ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL elementReplaced( const css::container::ContainerEvent& Event ) throw (css::uno::RuntimeException, std::exception) override;

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& i_event ) throw (css::uno::RuntimeException, std::exception) override;

    private:
        UnoGridControl& m_parent;
    };


} // namespace toolkit


#endif // INCLUDED_TOOLKIT_SOURCE_CONTROLS_GRID_GRIDEVENTFORWARDER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
