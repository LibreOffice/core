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

#include <svtools/genericunodialog.hxx>
#include <apitools.hxx>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <comphelper/proparrhlp.hxx>
#include <connectivity/CommonTools.hxx>

namespace dbaui
{

    // ODirectSQLDialog
    class ODirectSQLDialog;
    typedef ::svt::OGenericUnoDialog                                        ODirectSQLDialog_BASE;
    typedef ::comphelper::OPropertyArrayUsageHelper< ODirectSQLDialog >     ODirectSQLDialog_PBASE;

    class ODirectSQLDialog
            :public ODirectSQLDialog_BASE
            ,public ODirectSQLDialog_PBASE
    {
        OUString      m_sInitialSelection;
        css::uno::Reference< css::sdbc::XConnection > m_xActiveConnection;
    public:
        explicit ODirectSQLDialog(const css::uno::Reference< css::uno::XComponentContext >& _rxORB);
        virtual ~ODirectSQLDialog() override;

        virtual css::uno::Sequence<sal_Int8> SAL_CALL getImplementationId() override;

        DECLARE_SERVICE_INFO();

        DECLARE_PROPERTYCONTAINER_DEFAULTS( );

    protected:
        // OGenericUnoDialog overridables
        virtual std::unique_ptr<weld::DialogController> createDialog(const css::uno::Reference<css::awt::XWindow>& rParent) override;
        virtual void implInitialize(const css::uno::Any& _rValue) override;
    };

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
