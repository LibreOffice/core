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
#ifndef CONNECTIVITY_IPARSECONTEXT_HXX
#define CONNECTIVITY_IPARSECONTEXT_HXX

namespace connectivity { class OSQLParseNode; }

#include <rtl/ustring.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include "connectivity/dbtoolsdllapi.hxx"

namespace connectivity
{
    //==========================================================================
    //= IParseContext
    //==========================================================================
    class IParseContext
    {
    public:
        enum    ErrorCode
        {
            ERROR_NONE      = 0,
            ERROR_GENERAL,                  // "Syntax error in SQL expression"
            ERROR_VALUE_NO_LIKE,            // "The value #1 can not be used with LIKE."
            ERROR_FIELD_NO_LIKE,            // "LIKE can not be used with this field."
            ERROR_INVALID_COMPARE,          // "The entered criterion can not be compared with this field."
            ERROR_INVALID_INT_COMPARE,      // "The field can not be compared with a number."
            ERROR_INVALID_DATE_COMPARE,     // "The field can not be compared with a date."
            ERROR_INVALID_REAL_COMPARE,     // "The field can not be compared with a floating point number."
            ERROR_INVALID_TABLE,            // "The database does not contain a table named \"#\"."
            ERROR_INVALID_TABLE_OR_QUERY,   // "The database does contain neither a table nor a query named \"#\"."
            ERROR_INVALID_COLUMN,           // "The column \"#1\" is unknown in the table \"#2\"."
            ERROR_INVALID_TABLE_EXIST,      // "The database already contains a table or view with name \"#\"."
            ERROR_INVALID_QUERY_EXIST       // "The database already contains a query with name \"#\".";
        };

        enum    InternationalKeyCode
        {
            KEY_NONE = 0,
            KEY_LIKE,
            KEY_NOT,
            KEY_NULL,
            KEY_TRUE,
            KEY_FALSE,
            KEY_IS,
            KEY_BETWEEN,
            KEY_OR,
            KEY_AND,
            KEY_AVG,
            KEY_COUNT,
            KEY_MAX,
            KEY_MIN,
            KEY_SUM,
            KEY_EVERY,
            KEY_ANY,
            KEY_SOME,
            KEY_STDDEV_POP,
            KEY_STDDEV_SAMP,
            KEY_VAR_SAMP,
            KEY_VAR_POP,
            KEY_COLLECT,
            KEY_FUSION,
            KEY_INTERSECTION
        };

    public:
        // retrieves language specific error messages
        virtual ::rtl::OUString getErrorMessage(ErrorCode _eCodes) const = 0;

        // retrieves language specific keyword strings (only ASCII allowed)
        virtual ::rtl::OString getIntlKeywordAscii(InternationalKeyCode _eKey) const = 0;

        // finds out, if we have an international keyword (only ASCII allowed)
        virtual InternationalKeyCode getIntlKeyCode(const ::rtl::OString& rToken) const = 0;

        /** get's a locale instance which should be used when parsing in the context specified by this instance
            <p>if this is not overridden by derived classes, it returns the static default locale.</p>
        */
        virtual ::com::sun::star::lang::Locale getPreferredLocale( ) const = 0;
    };
}

#endif // CONNECTIVITY_IPARSECONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
