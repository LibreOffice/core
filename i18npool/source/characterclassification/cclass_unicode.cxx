/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cclass_unicode.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:04:48 $
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

#include <cclass_unicode.hxx>
#include <com/sun/star/i18n/UnicodeScript.hpp>
#include <com/sun/star/i18n/UnicodeType.hpp>
#include <i18nutil/unicode.hxx>
#include <i18nutil/x_rtl_ustring.h>
#include <breakiteratorImpl.hxx>

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
    sal_Int32 len = Text.getLength();
    if (nPos >= len)
        return OUString();
    if (nCount + nPos > len)
        nCount = len - nPos;

    trans->setMappingType(MappingTypeToUpper, rLocale);
    return trans->transliterateString2String(Text, nPos, nCount);
}

OUString SAL_CALL
cclass_Unicode::toLower( const OUString& Text, sal_Int32 nPos, sal_Int32 nCount, const Locale& rLocale ) throw(RuntimeException) {
    sal_Int32 len = Text.getLength();
    if (nPos >= len)
        return OUString();
    if (nCount + nPos > len)
        nCount = len - nPos;

    trans->setMappingType(MappingTypeToLower, rLocale);
    return trans->transliterateString2String(Text, nPos, nCount);
}

OUString SAL_CALL
cclass_Unicode::toTitle( const OUString& Text, sal_Int32 nPos, sal_Int32 nCount, const Locale& rLocale ) throw(RuntimeException) {
    sal_Int32 len = Text.getLength();
    if (nPos >= len)
        return OUString();
    if (nCount + nPos > len)
        nCount = len - nPos;

    trans->setMappingType(MappingTypeToTitle, rLocale);
    rtl_uString* pStr = x_rtl_uString_new_WithLength( nCount, 1 );
    sal_Unicode* out = pStr->buffer;
    BreakIteratorImpl brk(xMSF);
    Boundary bdy = brk.getWordBoundary(Text, nPos, rLocale,
                WordType::ANYWORD_IGNOREWHITESPACES, sal_True);
    for (sal_Int32 i = nPos; i < nCount + nPos; i++, out++) {
        if (i >= bdy.endPos)
            bdy = brk.nextWord(Text, bdy.endPos, rLocale,
                        WordType::ANYWORD_IGNOREWHITESPACES);
        *out = (i == bdy.startPos) ?
            trans->transliterateChar2Char(Text[i]) : Text[i];
    }
    *out = 0;
    return OUString( pStr, SAL_NO_ACQUIRE );
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

