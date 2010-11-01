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
#ifndef DBAUI_IENVIRONMENT_HXX
#define DBAUI_IENVIRONMENT_HXX

#include <com/sun/star/sdbc/SQLException.hpp>

#include <connectivity/standardsqlstate.hxx>

namespace dbtools
{
    class SQLExceptionInfo;
}

namespace dbaui
{
    // interface class for a generell environment
    class IEnvironment
    {
    public:
        /** appends an error in the current environment.
        */
        virtual void appendError(
                        const ::rtl::OUString& _rErrorMessage,
                        const ::dbtools::StandardSQLState _eSQLState = ::dbtools::SQL_GENERAL_ERROR,
                        const sal_Int32 _nErrorCode = 1000
                     ) = 0;

        /** clears the error state.
        */
        virtual void clearError() = 0;

        /** @retrun
            returns <TRUE/> when an error was set otherwise <FALSE/>
        */
        virtual sal_Bool hasError() const = 0;

        /** returns the current error
        */
        virtual const ::dbtools::SQLExceptionInfo& getError() const = 0;

        /** displays the current error, or does nothing if there is no current error
        */
        virtual void displayError() = 0;

        /** gives access to the currently used connection
            @return
                the currently used connection.
        */
        virtual const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& getConnection() const = 0;
    };
}
#endif // DBAUI_IENVIRONMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
