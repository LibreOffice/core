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

#include <rtl/ustrbuf.hxx>


namespace connectivity::hsqldb
{

    class HTools
    {
    public:
        /** appends a proper WHERE clause to the given buffer, which filters
            for a given table name

            @param _bShortForm
                <TRUE/> if the column names of the system table which is being asked
                have the short form (TABLE_CAT instead of TABLE_CATALOG, and so on)
        */
        static void appendTableFilterCrit(
            OUStringBuffer& _inout_rBuffer, std::u16string_view _rCatalog,
            std::u16string_view _rSchema, std::u16string_view _rName,
            bool _bShortForm
        );
    };


} // namespace connectivity::hsqldb


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
