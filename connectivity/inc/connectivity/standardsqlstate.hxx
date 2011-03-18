/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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

#ifndef CONNECTIVITY_STANDARD_SQL_STATE_HXX
#define CONNECTIVITY_STANDARD_SQL_STATE_HXX

//.........................................................................
namespace dbtools
{
//.........................................................................

    //----------------------------------------------------------------------------------
    /** standard SQLStates to be used with an SQLException

        Extend this list whenever you need a new state ...

        @see http://msdn.microsoft.com/library/default.asp?url=/library/en-us/odbc/htm/odbcodbc_error_codes.asp
    */
    enum StandardSQLState
    {
        SQL_WRONG_PARAMETER_NUMBER,     // 07001
        SQL_INVALID_DESCRIPTOR_INDEX,   // 07009
        SQL_UNABLE_TO_CONNECT,          // 08001
        SQL_NUMERIC_OUT_OF_RANGE,       // 22003
        SQL_INVALID_DATE_TIME,          // 22007
        SQL_INVALID_CURSOR_STATE,       // 24000
        SQL_TABLE_OR_VIEW_EXISTS,       // 42S01
        SQL_TABLE_OR_VIEW_NOT_FOUND,    // 42S02
        SQL_INDEX_ESISTS,               // 42S11
        SQL_INDEX_NOT_FOUND,            // 42S12
        SQL_COLUMN_EXISTS,              // 42S21
        SQL_COLUMN_NOT_FOUND,           // 42S22
        SQL_GENERAL_ERROR,              // HY000
        SQL_INVALID_SQL_DATA_TYPE,      // HY004
        SQL_OPERATION_CANCELED,         // HY008
        SQL_FUNCTION_SEQUENCE_ERROR,    // HY010
        SQL_INVALID_CURSOR_POSITION,    // HY109
        SQL_INVALID_BOOKMARK_VALUE,     // HY111
        SQL_FEATURE_NOT_IMPLEMENTED,    // HYC00
        SQL_FUNCTION_NOT_SUPPORTED,     // IM001
        SQL_CONNECTION_DOES_NOT_EXIST,  // 08003

        SQL_ERROR_UNSPECIFIED = SAL_MAX_ENUM    // special value indicating that an SQLState is not to be specified
    };

//.........................................................................
}   // namespace dbtools
//.........................................................................

#endif // CONNECTIVITY_STANDARD_SQL_STATE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
