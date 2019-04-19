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
#include <cppuhelper/implbase.hxx>


namespace pcr
{


    //= PropertyEventTranslation

    typedef ::cppu::WeakImplHelper <   css::beans::XPropertyChangeListener
                                    >   PropertyEventTranslation_Base;

    class PropertyEventTranslation : public PropertyEventTranslation_Base
    {
        css::uno::Reference< css::beans::XPropertyChangeListener >
                m_xDelegator;
        css::uno::Reference< css::uno::XInterface >
                m_xTranslatedEventSource;

    public:
        /** constructs the object
            @throws NullPointerException
                if <arg>_rxDelegator</arg> is <NULL/>
        */
        PropertyEventTranslation(
            const css::uno::Reference< css::beans::XPropertyChangeListener >& _rxDelegator,
            const css::uno::Reference< css::uno::XInterface >& _rxTranslatedEventSource
        );

        const css::uno::Reference< css::beans::XPropertyChangeListener >&
            getDelegator() const { return m_xDelegator; }

    protected:
        // XPropertyChangeListener
        virtual void SAL_CALL propertyChange( const css::beans::PropertyChangeEvent& evt ) override;
        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

    private:
        PropertyEventTranslation( const PropertyEventTranslation& ) = delete;
        PropertyEventTranslation& operator=( const PropertyEventTranslation& ) = delete;
    };


} // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
