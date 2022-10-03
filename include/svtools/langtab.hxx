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
#pragma once

#include <i18nlangtag/lang.h>
#include <rtl/ustring.hxx>
#include <svtools/svtdllapi.h>

class LanguageTag;

class SVT_DLLPUBLIC SvtLanguageTable
{
public:

    static bool         HasLanguageType( const LanguageType eType );
    /** Obtain the UI name for a LanguageType of a language/locale (string
        resource STR_ARR_SVT_LANGUAGE_TABLE).
     */
    static OUString     GetLanguageString( const LanguageType eType );
    /** Obtain the LanguageType for a UI name of a language/locale (string
        resource STR_ARR_SVT_LANGUAGE_TABLE).
     */
    static LanguageType GetLanguageType( std::u16string_view rStr );
    static sal_uInt32   GetLanguageEntryCount();
    static LanguageType GetLanguageTypeAtIndex( sal_uInt32 nIndex );

    /** Add a language tag to the table.
     */
    static void         AddLanguageTag( const LanguageTag& rLanguageTag );
};

// Add LRE or RLE embedding characters to the string based on the
// String content (see #i78466#, #i32179#)
SVT_DLLPUBLIC OUString ApplyLreOrRleEmbedding( const OUString &rText );

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
