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

#include <cppuhelper/basemutex.hxx>
#include "loadlisteneradapter.hxx"
#include <com/sun/star/awt/XControlContainer.hpp>
#include <rtl/ref.hxx>


namespace bib
{

    class FormControlContainer
            :public ::cppu::BaseMutex
            ,public ::bib::OLoadListener
    {
    private:
        rtl::Reference<OLoadListenerAdapter>         m_xFormAdapter;
        css::uno::Reference< css::form::XLoadable >  m_xForm;
    private:
        void    implSetDesignMode( bool _bDesign );

    protected:
        FormControlContainer( );
        virtual ~FormControlContainer( ) override;

        bool        isFormConnected() const { return m_xFormAdapter.is(); }
        void        connectForm( const css::uno::Reference< css::form::XLoadable >& _rxForm );
        void        disconnectForm();

        virtual css::uno::Reference< css::awt::XControlContainer >
                    getControlContainer() = 0;

    protected:
    // XLoadListener equivalents
        virtual void _loaded( const css::lang::EventObject& _rEvent ) override;
        virtual void _unloading( const css::lang::EventObject& _rEvent ) override;
        virtual void _reloading( const css::lang::EventObject& _rEvent ) override;
        virtual void _reloaded( const css::lang::EventObject& _rEvent ) override;

    };


}   // namespace bib


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
