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
#ifndef INCLUDED_I18NUTIL_UNICODE_HXX
#define INCLUDED_I18NUTIL_UNICODE_HXX

#include <com/sun/star/i18n/UnicodeScript.hpp>
#include <sal/types.h>
#include <unicode/uscript.h>
#include "i18nutildllapi.h"

typedef struct _ScriptTypeList {
    sal_Int16 from;
    sal_Int16 to;
    sal_Int16 value;
} ScriptTypeList;

class I18NUTIL_DLLPUBLIC unicode
{
public:

    static sal_Int16 SAL_CALL getUnicodeType( const sal_Unicode ch );
    static sal_Int16 SAL_CALL getUnicodeScriptType( const sal_Unicode ch, ScriptTypeList *typeList = NULL, sal_Int16 unknownType = 0 );
    static sal_Unicode SAL_CALL getUnicodeScriptStart(com::sun::star::i18n::UnicodeScript type);
    static sal_Unicode SAL_CALL getUnicodeScriptEnd(com::sun::star::i18n::UnicodeScript type);
    static sal_uInt8 SAL_CALL getUnicodeDirection( const sal_Unicode ch );
    static sal_Bool SAL_CALL isUpper( const sal_Unicode ch);
    static sal_Bool SAL_CALL isLower( const sal_Unicode ch);
    static sal_Bool SAL_CALL isDigit( const sal_Unicode ch);
    static sal_Bool SAL_CALL isControl( const sal_Unicode ch);
    static sal_Bool SAL_CALL isPrint( const sal_Unicode ch);
    static sal_Bool SAL_CALL isAlpha( const sal_Unicode ch);
    static sal_Bool SAL_CALL isSpace( const sal_Unicode ch);
    static sal_Bool SAL_CALL isWhiteSpace( const sal_Unicode ch);
    static sal_Bool SAL_CALL isAlphaDigit( const sal_Unicode ch);

    //Map an ISO 15924 script code to Latin/Asian/Complex/Weak
    static sal_Int16 SAL_CALL getScriptClassFromUScriptCode(UScriptCode eScript);

    //Return a language that can be written in a given ISO 15924 script code
    static OString SAL_CALL getExemplerLanguageForUScriptCode(UScriptCode eScript);
};

#endif



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
