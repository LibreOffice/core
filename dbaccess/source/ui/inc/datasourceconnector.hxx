/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _DBAUI_DATASOURCECONNECTOR_HXX_
#define _DBAUI_DATASOURCECONNECTOR_HXX_

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>

namespace dbtools
{
    class SQLExceptionInfo;
}

class Window;
//.........................................................................
namespace dbaui
{
//.........................................................................

    //=====================================================================
    //= ODatasourceConnector
    //=====================================================================
    class ODatasourceConnector
    {
    protected:
        Window*         m_pErrorMessageParent;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                        m_xORB;
        ::rtl::OUString m_sContextInformation;

    public:
        ODatasourceConnector(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            Window* _pMessageParent
        );
        ODatasourceConnector(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            Window* _pMessageParent,
            const ::rtl::OUString& _rContextInformation
        );

        /// returns <TRUE/> if the object is able to create data source connections
        sal_Bool    isValid() const { return m_xORB.is(); }

        /** creates a connection to the data source, displays the possible error to the user, or returns it
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >
                    connect(
                        const ::rtl::OUString& _rDataSourceName,
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

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_DATASOURCECONNECTOR_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
