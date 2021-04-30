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

/// @HTML
#ifndef INCLUDED_SW_SOURCE_FILTER_WW8_FIELDS_HXX
#define INCLUDED_SW_SOURCE_FILTER_WW8_FIELDS_HXX

#include <filter/msfilter/ww8fields.hxx>

namespace ww
{
/** Find the English Field Name from a winword index

    See OpenOffice.org issue 12831
    (https://bz.apache.org/ooo/show_bug.cgi?id=12831) and MS
    Knowledge Base article 268717
    (http://support.microsoft.com/default.aspx?scid=kb;en-us;268717) for
    details of why to use english field names and not localized ones since
    Word 2000.

    @param
    nIndex the index to search for

    @return 0 if not found, otherwise the fieldname as a C style ASCII
    string
*/
const char* GetEnglishFieldName(eField eIndex) noexcept;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
