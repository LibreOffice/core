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

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>

namespace dbtools
{
    class SQLExceptionInfo;
}

namespace weld { class Window; }
namespace dbaui
{

    // ODatasourceConnector
    class ODatasourceConnector final
    {
        weld::Window*  m_pErrorMessageParent;
        css::uno::Reference< css::uno::XComponentContext >
                             m_xContext;
        OUString             m_sContextInformation;

    public:
        ODatasourceConnector(
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext,
            weld::Window* _pMessageParent
        );
        ODatasourceConnector(
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext,
            weld::Window* _pMessageParent,
            const OUString& _rContextInformation
        );

        /// returns <TRUE/> if the object is able to create data source connections
        bool    isValid() const { return m_xContext.is(); }

        /** creates a connection to the data source, displays the possible error to the user, or returns it
        */
        css::uno::Reference< css::sdbc::XConnection >
                    connect(
                        const OUString& _rDataSourceName,
                        ::dbtools::SQLExceptionInfo* _pErrorInfo
                    ) const;

        /** creates a connection to the data source, displays the possible error to the user, or returns it
        */
        css::uno::Reference< css::sdbc::XConnection >
                    connect(
                        const css::uno::Reference< css::sdbc::XDataSource>& _xDataSource,
                        ::dbtools::SQLExceptionInfo* _pErrorInfo
                    ) const;
    };

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
