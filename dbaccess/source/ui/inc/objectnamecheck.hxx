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

#ifndef DBACCESS_SOURCE_UI_INC_OBJECTNAMECHECK_HXX
#define DBACCESS_SOURCE_UI_INC_OBJECTNAMECHECK_HXX

#include <rtl/ustring.hxx>

namespace dbtools { class SQLExceptionInfo; }

namespace dbaui
{

    // IObjectNameCheck
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
            const OUString& _rObjectName,
            ::dbtools::SQLExceptionInfo& _out_rErrorToDisplay
        ) const = 0;

    public:
        virtual ~IObjectNameCheck() { }
    };

} // namespace dbaui

#endif // DBACCESS_SOURCE_UI_INC_OBJECTNAMECHECK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
