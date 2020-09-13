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

#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/basemutex.hxx>

#include <vector>

namespace dbaccess
{

    // OPropertyForward
    typedef ::cppu::WeakImplHelper<   css::beans::XPropertyChangeListener
                                  >   OPropertyForward_Base;
    class OPropertyForward  :public ::cppu::BaseMutex
                            ,public OPropertyForward_Base
    {
        css::uno::Reference< css::beans::XPropertySet >       m_xSource;
        css::uno::Reference< css::beans::XPropertySet >       m_xDest;
        css::uno::Reference< css::beans::XPropertySetInfo >   m_xDestInfo;
        css::uno::Reference< css::container::XNameAccess >    m_xDestContainer;
        OUString        m_sName;
        bool            m_bInInsert;

    protected:
        virtual ~OPropertyForward() override;

    public:
        OPropertyForward( const css::uno::Reference< css::beans::XPropertySet>& _xSource,
                          const css::uno::Reference< css::container::XNameAccess>& _xDestContainer,
                          const OUString& _sName,
                          const std::vector< OUString >& _aPropertyList
                         );

        // css::beans::XPropertyChangeListener
        virtual void SAL_CALL propertyChange( const css::beans::PropertyChangeEvent& evt ) override;

        // css::lang::XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& _rSource ) override;

        void setName( const OUString& _sName ) { m_sName = _sName; }
        void setDefinition( const css::uno::Reference< css::beans::XPropertySet >& _xDest);
        const css::uno::Reference< css::beans::XPropertySet >& getDefinition() const { return m_xDest; }
    };

}   // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
