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
#ifndef INCLUDED_SW_INC_ITABENUM_HXX
#define INCLUDED_SW_INC_ITABENUM_HXX

#include <sal/types.h>
#include <o3tl/typed_flags_set.hxx>

enum class SwInsertTableFlags
{
    NONE               = 0x00,
    DefaultBorder      = 0x01,
    Headline           = 0x02,
    SplitLayout        = 0x08,
    HeadlineNoBorder   = Headline | SplitLayout,
    All   = DefaultBorder | Headline | SplitLayout
};
namespace o3tl {
    template<> struct typed_flags<SwInsertTableFlags> : is_typed_flags<SwInsertTableFlags, 0x0b> {};
}

struct SwInsertTableOptions
{
    SwInsertTableFlags mnInsMode;
    sal_uInt16 mnRowsToRepeat;

    SwInsertTableOptions( SwInsertTableFlags nInsMode, sal_uInt16 nRowsToRepeat ) :
        mnInsMode( nInsMode ), mnRowsToRepeat( nRowsToRepeat ) {};
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
