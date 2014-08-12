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

namespace tabopts
{
    const sal_uInt16 DEFAULT_BORDER     = 0x01;
    const sal_uInt16 HEADLINE           = 0x02;
    const sal_uInt16 SPLIT_LAYOUT       = 0x08;
    const sal_uInt16 HEADLINE_NO_BORDER = HEADLINE | SPLIT_LAYOUT;
    const sal_uInt16 ALL_TBL_INS_ATTR   = DEFAULT_BORDER | HEADLINE | SPLIT_LAYOUT;
}

struct SwInsertTableOptions
{
    sal_uInt16 mnInsMode;
    sal_uInt16 mnRowsToRepeat;

    SwInsertTableOptions( sal_uInt16 nInsMode, sal_uInt16 nRowsToRepeat ) :
        mnInsMode( nInsMode ), mnRowsToRepeat( nRowsToRepeat ) {};
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
