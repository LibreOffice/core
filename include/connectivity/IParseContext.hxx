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
#ifndef INCLUDED_CONNECTIVITY_IPARSECONTEXT_HXX
#define INCLUDED_CONNECTIVITY_IPARSECONTEXT_HXX

namespace connectivity { class OSQLParseNode; }

#include <rtl/ustring.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <connectivity/dbtoolsdllapi.hxx>

namespace connectivity
{

    //= IParseContext

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
            ERROR_INVALID_TABLE_NOSUCH,     // "The database does not contain a table named \"#\"."
                                            // Named like this to avoid conflict with a #define in the Windows system ODBC headers.
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
        virtual OUString getErrorMessage(ErrorCode _eCodes) const = 0;

        // retrieves language specific keyword strings (only ASCII allowed)
        virtual OString getIntlKeywordAscii(InternationalKeyCode _eKey) const = 0;

        // finds out, if we have an international keyword (only ASCII allowed)
        virtual InternationalKeyCode getIntlKeyCode(const OString& rToken) const = 0;

        /** get's a locale instance which should be used when parsing in the context specified by this instance
            <p>if this is not overridden by derived classes, it returns the static default locale.</p>
        */
        virtual css::lang::Locale getPreferredLocale( ) const = 0;

    protected:
        ~IParseContext() {}
    };
}

#endif // INCLUDED_CONNECTIVITY_IPARSECONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
