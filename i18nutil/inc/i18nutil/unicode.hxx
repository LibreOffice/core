/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unicode.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:39:33 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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



