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

#ifndef INCLUDED_CONNECTIVITY_STANDARDSQLSTATE_HXX
#define INCLUDED_CONNECTIVITY_STANDARDSQLSTATE_HXX


namespace dbtools
{


    /** standard SQLStates to be used with an SQLException

        Extend this list whenever you need a new state ...

        @see http://msdn.microsoft.com/library/default.asp?url=/library/en-us/odbc/htm/odbcodbc_error_codes.asp
    */
    enum class StandardSQLState
    {
        WRONG_PARAMETER_NUMBER,     // 07001
        INVALID_DESCRIPTOR_INDEX,   // 07009
        UNABLE_TO_CONNECT,          // 08001
        NUMERIC_OUT_OF_RANGE,       // 22003
        INVALID_DATE_TIME,          // 22007
        INVALID_CURSOR_STATE,       // 24000
        TABLE_OR_VIEW_EXISTS,       // 42S01
        TABLE_OR_VIEW_NOT_FOUND,    // 42S02
        INDEX_ESISTS,               // 42S11
        INDEX_NOT_FOUND,            // 42S12
        COLUMN_EXISTS,              // 42S21
        COLUMN_NOT_FOUND,           // 42S22
        GENERAL_ERROR,              // HY000
        INVALID_SQL_DATA_TYPE,      // HY004
        OPERATION_CANCELED,         // HY008
        FUNCTION_SEQUENCE_ERROR,    // HY010
        INVALID_CURSOR_POSITION,    // HY109
        INVALID_BOOKMARK_VALUE,     // HY111
        FEATURE_NOT_IMPLEMENTED,    // HYC00
        FUNCTION_NOT_SUPPORTED,     // IM001
        CONNECTION_DOES_NOT_EXIST,  // 08003

        ERROR_UNSPECIFIED = SAL_MAX_ENUM    // special value indicating that an SQLState is not to be specified
    };


}   // namespace dbtools


#endif // INCLUDED_CONNECTIVITY_STANDARDSQLSTATE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
