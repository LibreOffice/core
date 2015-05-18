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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_DATASOURCECONNECTOR_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_DATASOURCECONNECTOR_HXX

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <vcl/vclptr.hxx>

namespace dbtools
{
    class SQLExceptionInfo;
}

namespace vcl { class Window; }
namespace dbaui
{

    // ODatasourceConnector
    class ODatasourceConnector
    {
    protected:
        VclPtr<vcl::Window>  m_pErrorMessageParent;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
                             m_xContext;
        OUString             m_sContextInformation;

    public:
        ODatasourceConnector(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext,
            vcl::Window* _pMessageParent
        );
        ODatasourceConnector(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext,
            vcl::Window* _pMessageParent,
            const OUString& _rContextInformation
        );

        /// returns <TRUE/> if the object is able to create data source connections
        bool    isValid() const { return m_xContext.is(); }

        /** creates a connection to the data source, displays the possible error to the user, or returns it
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >
                    connect(
                        const OUString& _rDataSourceName,
                        ::dbtools::SQLExceptionInfo* _pErrorInfo
                    ) const;

        /** creates a connection to the data source, displays the possible error to the user, or returns it
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >
                    connect(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource>& _xDataSource,
                        ::dbtools::SQLExceptionInfo* _pErrorInfo
                    ) const;
    };

}

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_DATASOURCECONNECTOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
