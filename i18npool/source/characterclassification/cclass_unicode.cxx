/*************************************************************************
 *
 *  $RCSfile: cclass_unicode.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-08 15:46:36 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <cclass_unicode.hxx>
#include <com/sun/star/i18n/UnicodeScript.hpp>
#include <com/sun/star/i18n/UnicodeType.hpp>
#include <unicode.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::rtl;

namespace com { namespace sun { namespace star { namespace i18n {
//  ----------------------------------------------------
//  class cclass_Unicode
//  ----------------------------------------------------;

cclass_Unicode::cclass_Unicode( uno::Reference < XMultiServiceFactory > xSMgr ) : xMSF( xSMgr ),
        pTable( NULL ),
        pStart( NULL ),
        pCont( NULL ),
        nStartTypes( 0 ),
        nContTypes( 0 ),
        eState( ssGetChar ),
        cGroupSep( ',' ),
        cDecimalSep( '.' )
{
    trans = new Transliteration_casemapping();
    cClass = "com.sun.star.i18n.CharacterClassification_Unicode";
}

cclass_Unicode::~cclass_Unicode() {
    destroyParserTable();
    delete trans;
}


OUString SAL_CALL
cclass_Unicode::toUpper( const OUString& Text, sal_Int32 nPos, sal_Int32 nCount, const Locale& rLocale ) throw(RuntimeException) {
    trans->setMappingType(MappingTypeToUpper, rLocale);
    return trans->transliterateString2String(Text, nPos, nCount);
}

OUString SAL_CALL
cclass_Unicode::toLower( const OUString& Text, sal_Int32 nPos, sal_Int32 nCount, const Locale& rLocale ) throw(RuntimeException) {
    trans->setMappingType(MappingTypeToLower, rLocale);
    return trans->transliterateString2String(Text, nPos, nCount);
}

OUString SAL_CALL
cclass_Unicode::toTitle( const OUString& Text, sal_Int32 nPos, sal_Int32 nCount, const Locale& rLocale ) throw(RuntimeException) {
    trans->setMappingType(MappingTypeToTitle, rLocale);
    return trans->transliterateString2String(Text, nPos, nCount);
}

sal_Int16 SAL_CALL
cclass_Unicode::getType( const OUString& Text, sal_Int32 nPos ) throw(RuntimeException) {
    if ( Text.getLength() <= nPos ) return 0;
    return unicode::getUnicodeType(Text[nPos]);
}

sal_Int16 SAL_CALL
cclass_Unicode::getCharacterDirection( const OUString& Text, sal_Int32 nPos ) throw(RuntimeException) {
    if ( Text.getLength() <= nPos ) return 0;
    return unicode::getUnicodeDirection(Text[nPos]);
}


sal_Int16 SAL_CALL
cclass_Unicode::getScript( const OUString& Text, sal_Int32 nPos ) throw(RuntimeException) {
    if ( Text.getLength() <= nPos ) return 0;
    return unicode::getUnicodeScriptType(Text[nPos], (ScriptTypeList*) 0, 0);
}


sal_Int32 SAL_CALL
cclass_Unicode::getCharacterType( const OUString& Text, sal_Int32 nPos, const Locale& rLocale ) throw(RuntimeException) {
    if ( Text.getLength() <= nPos ) return 0;
    return unicode::getCharType(Text[nPos]);
}

sal_Int32 SAL_CALL
cclass_Unicode::getStringType( const OUString& Text, sal_Int32 nPos, sal_Int32 nCount, const Locale& rLocale ) throw(RuntimeException) {
    if ( Text.getLength() <= nPos ) return 0;

    if ( Text.getLength() < nPos + nCount )
        nCount = Text.getLength() - nPos;

    sal_Int32 result = 0;
    for (int i = 0; i < nCount; i++)
    result |= unicode::getCharType(Text[nPos+i]);
    return result;
}

ParseResult SAL_CALL cclass_Unicode::parseAnyToken(
            const OUString& Text,
            sal_Int32 nPos,
            const Locale& rLocale,
            sal_Int32 startCharTokenType,
            const OUString& userDefinedCharactersStart,
            sal_Int32 contCharTokenType,
            const OUString& userDefinedCharactersCont )
                throw(RuntimeException)
{
    ParseResult r;
    if ( Text.getLength() <= nPos )
        return r;

    setupParserTable( rLocale,
        startCharTokenType, userDefinedCharactersStart,
        contCharTokenType, userDefinedCharactersCont );
    parseText( r, Text, nPos );

    return r;
}


ParseResult SAL_CALL cclass_Unicode::parsePredefinedToken(
            sal_Int32 nTokenType,
            const OUString& Text,
            sal_Int32 nPos,
            const Locale& rLocale,
            sal_Int32 startCharTokenType,
            const OUString& userDefinedCharactersStart,
            sal_Int32 contCharTokenType,
            const OUString& userDefinedCharactersCont )
                throw(RuntimeException)
{
    ParseResult r;
    if ( Text.getLength() <= nPos )
        return r;

    setupParserTable( rLocale,
        startCharTokenType, userDefinedCharactersStart,
        contCharTokenType, userDefinedCharactersCont );
    parseText( r, Text, nPos, nTokenType );

    return r;
}

OUString SAL_CALL cclass_Unicode::getImplementationName() throw( RuntimeException )
{
    return OUString::createFromAscii(cClass);
}


sal_Bool SAL_CALL cclass_Unicode::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return !rServiceName.compareToAscii(cClass);
}

Sequence< OUString > SAL_CALL cclass_Unicode::getSupportedServiceNames() throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString::createFromAscii(cClass);
    return aRet;
}

} } } }

