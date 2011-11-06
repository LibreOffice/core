/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef INCLUDED_I18NUTIL_UNICODE_HXX
#define INCLUDED_I18NUTIL_UNICODE_HXX

#include <com/sun/star/i18n/UnicodeScript.hpp>
#include <sal/types.h>

typedef struct _ScriptTypeList {
    sal_Int16 from;
    sal_Int16 to;
    sal_Int16 value;
} ScriptTypeList;

class unicode
{
public:

    static sal_Int16 SAL_CALL getUnicodeType( const sal_Unicode ch );
    static sal_Bool SAL_CALL isUnicodeScriptType( const sal_Unicode ch, sal_Int16 type);
    static sal_Int16 SAL_CALL getUnicodeScriptType( const sal_Unicode ch, ScriptTypeList *typeList = NULL, sal_Int16 unknownType = 0 );
    static sal_Unicode SAL_CALL getUnicodeScriptStart(com::sun::star::i18n::UnicodeScript type);
    static sal_Unicode SAL_CALL getUnicodeScriptEnd(com::sun::star::i18n::UnicodeScript type);
    static sal_uInt8 SAL_CALL getUnicodeDirection( const sal_Unicode ch );
    static sal_Int32 SAL_CALL getCharType( const sal_Unicode ch );
    static sal_Bool SAL_CALL isUpper( const sal_Unicode ch);
    static sal_Bool SAL_CALL isLower( const sal_Unicode ch);
    static sal_Bool SAL_CALL isTitle( const sal_Unicode ch);
    static sal_Bool SAL_CALL isDigit( const sal_Unicode ch);
    static sal_Bool SAL_CALL isControl( const sal_Unicode ch);
    static sal_Bool SAL_CALL isPrint( const sal_Unicode ch);
    static sal_Bool SAL_CALL isBase( const sal_Unicode ch);
    static sal_Bool SAL_CALL isAlpha( const sal_Unicode ch);
    static sal_Bool SAL_CALL isSpace( const sal_Unicode ch);
    static sal_Bool SAL_CALL isWhiteSpace( const sal_Unicode ch);
    static sal_Bool SAL_CALL isAlphaDigit( const sal_Unicode ch);
    static sal_Bool SAL_CALL isPunctuation( const sal_Unicode ch);
};

#endif



