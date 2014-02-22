/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */



#include <utility>

#define TRANSLITERATION_Separator_ja_JP
#include <transliteration_Ignore.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

OneToOneMappingTable_t ignoreSeparatorTable[] = {
    { 0x0021, 0xFFFF },  
    { 0x0023, 0xFFFF },  
    { 0x0024, 0xFFFF },  
    { 0x0025, 0xFFFF },  
    { 0x0026, 0xFFFF },  
    { 0x002A, 0xFFFF },  
    { 0x002B, 0xFFFF },  
    { 0x002C, 0xFFFF },  
    { 0x002D, 0xFFFF },  
    { 0x002E, 0xFFFF },  
    { 0x002F, 0xFFFF },  
    { 0x003A, 0xFFFF },  
    { 0x003B, 0xFFFF },  
    { 0x003C, 0xFFFF },  
    { 0x003D, 0xFFFF },  
    { 0x003E, 0xFFFF },  
    { 0x005C, 0xFFFF },  
    { 0x005F, 0xFFFF },  
    { 0x007B, 0xFFFF },  
    { 0x007C, 0xFFFF },  
    { 0x007D, 0xFFFF },  
    { 0x007E, 0xFFFF },  
    { 0x00A5, 0xFFFF },  
    { 0x3001, 0xFFFF },  
    { 0x3002, 0xFFFF },  
    { 0x3008, 0xFFFF },  
    { 0x3009, 0xFFFF },  
    { 0x300A, 0xFFFF },  
    { 0x300B, 0xFFFF },  
    { 0x300C, 0xFFFF },  
    { 0x300D, 0xFFFF },  
    { 0x300E, 0xFFFF },  
    { 0x300F, 0xFFFF },  
    { 0x3010, 0xFFFF },  
    { 0x3011, 0xFFFF },  
    { 0x3014, 0xFFFF },  
    { 0x3015, 0xFFFF },  
    { 0x3016, 0xFFFF },  
    { 0x3017, 0xFFFF },  
    { 0x3018, 0xFFFF },  
    { 0x3019, 0xFFFF },  
    { 0x301A, 0xFFFF },  
    { 0x301B, 0xFFFF },  
    { 0x301C, 0xFFFF },  
    { 0x301D, 0xFFFF },  
    { 0x301E, 0xFFFF },  
    { 0x301F, 0xFFFF },  
    { 0x3030, 0xFFFF },  
    { 0x30FB, 0xFFFF },  
    { 0x30FC, 0xFFFF },  
    { 0xFF01, 0xFFFF },  
    { 0xFF03, 0xFFFF },  
    { 0xFF04, 0xFFFF },  
    { 0xFF05, 0xFFFF },  
    { 0xFF06, 0xFFFF },  
    { 0xFF0A, 0xFFFF },  
    { 0xFF0B, 0xFFFF },  
    { 0xFF0C, 0xFFFF },  
    { 0xFF0D, 0xFFFF },  
    { 0xFF0E, 0xFFFF },  
    { 0xFF0F, 0xFFFF },  
    { 0xFF1A, 0xFFFF },  
    { 0xFF1B, 0xFFFF },  
    { 0xFF1C, 0xFFFF },  
    { 0xFF1D, 0xFFFF },  
    { 0xFF1E, 0xFFFF },  
    { 0xFF3C, 0xFFFF },  
    { 0xFF3F, 0xFFFF },  
    { 0xFF5B, 0xFFFF },  
    { 0xFF5C, 0xFFFF },  
    { 0xFF5D, 0xFFFF },  
    { 0xFF5E, 0xFFFF },  
    { 0xFFE5, 0xFFFF },  
};

ignoreSeparator_ja_JP::ignoreSeparator_ja_JP()
{
    static oneToOneMapping _table(ignoreSeparatorTable, sizeof(ignoreSeparatorTable));
    func = (TransFunc) 0;
    table = &_table;
    map = 0;
    transliterationName = "ignoreSeparator_ja_JP";
    implementationName = "com.sun.star.i18n.Transliteration.ignoreSeparator_ja_JP";
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
