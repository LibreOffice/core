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

#include <i18nutil/oneToOneMapping.hxx>
#define TRANSLITERATION_largeToSmall_ja_JP
#include <transliteration_OneToOne.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

namespace com { namespace sun { namespace star { namespace i18n {







OneToOneMappingTable_t large2small[] = {
    { 0x3041, 0x3042 },  
    { 0x3043, 0x3044 },  
    { 0x3045, 0x3046 },  
    { 0x3047, 0x3048 },  
    { 0x3049, 0x304A },  
    { 0x3063, 0x3064 },  
    { 0x3083, 0x3084 },  
    { 0x3085, 0x3086 },  
    { 0x3087, 0x3088 },  
    { 0x308E, 0x308F },  
    { 0x30A1, 0x30A2 },  
    { 0x30A3, 0x30A4 },  
    { 0x30A5, 0x30A6 },  
    { 0x30A7, 0x30A8 },  
    { 0x30A9, 0x30AA },  
    { 0x30C3, 0x30C4 },  
    { 0x30E3, 0x30E4 },  
    { 0x30E5, 0x30E6 },  
    { 0x30E7, 0x30E8 },  
    { 0x30EE, 0x30EF },  
    { 0x30F5, 0x30AB },  
    { 0x30F6, 0x30B1 },  
    { 0xFF67, 0xFF71 },  
    { 0xFF68, 0xFF72 },  
    { 0xFF69, 0xFF73 },  
    { 0xFF6A, 0xFF74 },  
    { 0xFF6B, 0xFF75 },  
    { 0xFF6C, 0xFF94 },  
    { 0xFF6D, 0xFF95 },  
    { 0xFF6E, 0xFF96 },  
    { 0xFF6F, 0xFF82 }   
};

largeToSmall_ja_JP::largeToSmall_ja_JP()
{
    static oneToOneMapping _table(large2small, sizeof(large2small));
    func = (TransFunc) 0;
    table = &_table;
    transliterationName = "largeToSmall_ja_JP";
    implementationName = "com.sun.star.i18n.Transliteration.largeToSmall_ja_JP";
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
