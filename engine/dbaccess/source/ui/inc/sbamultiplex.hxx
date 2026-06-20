/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <com/sun/star/beans/XVetoableChangeListener.hpp>
#include <com/sun/star/form/XDatabaseParameterListener.hpp>
#include <com/sun/star/form/XLoadListener.hpp>
#include <com/sun/star/beans/XPropertiesChangeListener.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/form/XSubmitListener.hpp>
#include <com/sun/star/form/XResetListener.hpp>
#include <com/sun/star/sdb/XSQLErrorListener.hpp>
#include <com/sun/star/sdb/XRowSetApproveListener.hpp>
#include <com/sun/star/sdbc/XRowSetListener.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <comphelper/uno3.hxx>
#include <comphelper/interfacecontainer3.hxx>
#include <comphelper/multiinterfacecontainer3.hxx>
#include <cppuhelper/weak.hxx>

namespace dbaui
{
    // TODO : replace this class if MM provides a WeakSubObject in cppu
    class OSbaWeakSubObject : public ::cppu::OWeakObject
    {
    protected:
        ::cppu::OWeakObject&    m_rParent;

    public:
        OSbaWeakSubObject(::cppu::OWeakObject& rParent) : m_rParent(rParent) { }

        virtual void SAL_CALL acquire() noexcept override { m_rParent.acquire(); }
        virtual void SAL_CALL release() noexcept override { m_rParent.release(); }
    };

    // some listener multiplexers
    // css::frame::XStatusListener
    class SbaXStatusMultiplexer
            :public OSbaWeakSubObject
            ,public css::frame::XStatusListener
            ,public ::comphelper::OInterfaceContainerHelper3<css::frame::XStatusListener>
    {
    public:
        SbaXStatusMultiplexer(::cppu::OWeakObject& rSource, ::osl::Mutex& rMutex);
        DECLARE_UNO3_DEFAULTS(SbaXStatusMultiplexer, OSbaWeakSubObject)
        virtual cpo::uno::Any  SAL_CALL queryInterface(const css::uno::Type& _rType) override;

        /* css::lang::XEventListener */
        virtual void SAL_CALL disposing(const css::lang::EventObject& Source) override;

        virtual void SAL_CALL statusChanged(const css::frame::FeatureStateEvent& e) override;

    private:
        css::frame::FeatureStateEvent  m_aLastKnownStatus;
    public:
        const css::frame::FeatureStateEvent& getLastEvent( ) const { return m_aLastKnownStatus; }
    };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
