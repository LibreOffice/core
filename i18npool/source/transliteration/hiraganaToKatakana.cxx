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

#define TRANSLITERATION_hiraganaToKatakana
#include <transliteration_OneToOne.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

namespace com { namespace sun { namespace star { namespace i18n {



static sal_Unicode toKatakana (const sal_Unicode c) {
    if ( (0x3041 <= c && c <= 0x3096) || (0x309d <= c && c <= 0x309f) ) { 
        
        return c + (0x30a0 - 0x3040);
    }
    return c;
}

hiraganaToKatakana::hiraganaToKatakana()
{
    func = toKatakana;
    table = 0;
    transliterationName = "hiraganaToKatakana";
    implementationName = "com.sun.star.i18n.Transliteration.HIRAGANA_KATAKANA";
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
