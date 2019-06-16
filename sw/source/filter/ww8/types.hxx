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

#ifndef INCLUDED_SW_SOURCE_FILTER_WW8_TYPES_HXX
#define INCLUDED_SW_SOURCE_FILTER_WW8_TYPES_HXX

#include <sal/types.h>

#include <vector>

namespace ww
{
    typedef std::vector<sal_uInt8> bytes;

    enum WordVersion {eWW1 = 1, eWW2 = 2, eWW6 = 6, eWW7 = 7, eWW8 = 8};
    inline bool IsSevenMinus(WordVersion eVer) { return eVer <= eWW7; }
    inline bool IsEightPlus(WordVersion eVer) { return eVer >= eWW8; }

    /** For custom wrapping

        When you edit the wrap points of a contour in word, word uses a relative
        scale of 0 to 21600 where 21600 is apparently 100% of the graphic width
    */
    const int nWrap100Percent = 21600;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
