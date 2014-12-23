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
#ifndef INCLUDED_CUI_SOURCE_TABPAGES_PAGE_H
#define INCLUDED_CUI_SOURCE_TABPAGES_PAGE_H

// define ----------------------------------------------------------------

//!! the values of the following defines must correspond to the array position
//!! of the respective paper size in the file i18nutil/source/utility/paper.cxx
//!! There are enums for them in i18nutil/inc/i18nutil/paper.hxx but unfortunately
//!! the resource compiler does not understand enums, thus the enum values need
//!! to be duplicated here for use in the src file.
#define PAPERSIZE_A0            0
#define PAPERSIZE_A1            1
#define PAPERSIZE_A2            2
#define PAPERSIZE_A3            3
#define PAPERSIZE_A4            4
#define PAPERSIZE_A5            5
#define PAPERSIZE_B4_ISO        6
#define PAPERSIZE_B5_ISO        7
#define PAPERSIZE_LETTER        8
#define PAPERSIZE_LEGAL         9
#define PAPERSIZE_TABLOID       10
#define PAPERSIZE_USER          11
#define PAPERSIZE_B6_ISO        12
#define PAPERSIZE_C4            13
#define PAPERSIZE_C5            14
#define PAPERSIZE_C6            15
#define PAPERSIZE_C65           16
#define PAPERSIZE_DL            17
#define PAPERSIZE_DIA           18
#define PAPERSIZE_SCREEN_4_3    19
#define PAPERSIZE_LEGAL2        24
#define PAPERSIZE_MONARCH       25
#define PAPERSIZE_COM675        26
#define PAPERSIZE_COM9          27
#define PAPERSIZE_COM10         28
#define PAPERSIZE_COM11         29
#define PAPERSIZE_COM12         30
#define PAPERSIZE_KAI16         31
#define PAPERSIZE_KAI32         32
#define PAPERSIZE_KAI32BIG      33
#define PAPERSIZE_B4_JIS        34
#define PAPERSIZE_B5_JIS        35
#define PAPERSIZE_B6_JIS        36
#define PAPERSIZE_POSTCARD_JP   46
#define PAPERSIZE_A6            56
#define PAPERSIZE_SCREEN_16_9   78
#define PAPERSIZE_SCREEN_16_10  79

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
