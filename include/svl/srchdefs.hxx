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
 *   Licensed to the Apache Software Foundation (ASF, under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License",; you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SVL_SRCHDEFS_HXX
#define INCLUDED_SVL_SRCHDEFS_HXX

#include <o3tl/typed_flags_set.hxx>

enum class SearchOptionFlags
{
    NONE          = 0x0000,
    SEARCH        = 0x0001,
    SEARCHALL     = 0x0002, // named to this way to avoid conflict with macro on Windows.
    REPLACE       = 0x0004,
    REPLACE_ALL   = 0x0008,
    WHOLE_WORDS   = 0x0010,
    BACKWARDS     = 0x0020,
    REG_EXP       = 0x0040,
    EXACT         = 0x0080,
    SELECTION     = 0x0100,
    FAMILIES      = 0x0200,
    FORMAT        = 0x0400,
    SIMILARITY    = 0x0800,
    WILDCARD      = 0x1000,
    ALL           = 0x1fff
};
namespace o3tl
{
    template<> struct typed_flags<SearchOptionFlags> : is_typed_flags<SearchOptionFlags, 0x1fff> {};
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
