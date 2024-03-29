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

#include <transliteration_Ignore.hxx>

namespace i18npool
{
const transliteration_Ignore::Mapping TiJi[] = {
    // TU + I --> TI
    { 0x30C4, 0x30A3, 0x30C1, true },
    // TE + I --> TI
    { 0x30C6, 0x30A3, 0x30C1, true },
    // TU + I --> TI
    { 0x3064, 0x3043, 0x3061, true },
    // TE + I --> TI
    { 0x3066, 0x3043, 0x3061, true },
    // DE + I --> ZI
    { 0x30C7, 0x30A3, 0x30B8, true },
    // DE + I --> ZI
    { 0x3067, 0x3043, 0x3058, true },

    { 0, 0, 0, true }
};

ignoreTiJi_ja_JP::ignoreTiJi_ja_JP()
{
    func = nullptr;
    table = nullptr;
    map = TiJi;
    transliterationName = "ignoreTiJi_ja_JP";
    implementationName = "com.sun.star.i18n.Transliteration.ignoreTiJi_ja_JP";
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
