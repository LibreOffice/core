/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: unicode.hxx,v $
 * $Revision: 1.6 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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



