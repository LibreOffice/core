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

#include <sal/types.h>
#include <connectivity/dbtoolsdllapi.hxx>

namespace connectivity
{
    class OOO_DLLPUBLIC_DBTOOLS SAL_NO_VTABLE IResultSetHelper
    {
    public:
        enum Movement
        {
            NEXT        = 0,
            PRIOR,
            FIRST,
            LAST,
            // Named like this to avoid conflict with a #define in the Windows system ODBC headers.
            RELATIVE1,
            ABSOLUTE1,
            BOOKMARK,
        };
    public:
        virtual bool move(Movement _eCursorPosition, sal_Int32 _nOffset, bool _bRetrieveData) = 0;
        virtual sal_Int32 getDriverPos() const = 0;
        virtual bool isRowDeleted() const = 0;

    protected:
        ~IResultSetHelper() {}
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
