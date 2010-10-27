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

#ifndef DBACCESS_SOURCE_UI_INC_OBJECTNAMECHECK_HXX
#define DBACCESS_SOURCE_UI_INC_OBJECTNAMECHECK_HXX

/** === begin UNO includes === **/
/** === end UNO includes === **/

namespace rtl { class OUString; }
namespace dbtools { class SQLExceptionInfo; }

//........................................................................
namespace dbaui
{
//........................................................................

    //====================================================================
    //= IObjectNameCheck
    //====================================================================
    /** interface encapsulating the check for the validity of an object name
    */
    class IObjectNameCheck
    {
    public:
        /** determines whether a given object name is valid

            @param  _rObjectName
                the name to check
            @param  _out_rErrorToDisplay
                output parameter taking an error message describing why the name is not
                valid, if applicable.

            @return
                <TRUE/> if and only if the given name is valid.
        */
        virtual bool    isNameValid(
            const ::rtl::OUString& _rObjectName,
            ::dbtools::SQLExceptionInfo& _out_rErrorToDisplay
        ) const = 0;

    public:
        virtual ~IObjectNameCheck() { }
    };

//........................................................................
} // namespace dbaui
//........................................................................

#endif // DBACCESS_SOURCE_UI_INC_OBJECTNAMECHECK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
