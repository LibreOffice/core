/*************************************************************************
 *
 *  $RCSfile: breakiteratorImpl.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-30 14:37:54 $
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

#include <breakiteratorImpl.hxx>
#include <i18nutil/unicode.hxx>
#include <rtl/ustrbuf.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::rtl;

namespace com { namespace sun { namespace star { namespace i18n {

BreakIteratorImpl::BreakIteratorImpl( const Reference < XMultiServiceFactory >& rxMSF ) : xMSF( rxMSF )
{
}

BreakIteratorImpl::BreakIteratorImpl()
{
}

BreakIteratorImpl::~BreakIteratorImpl()
{
        // Clear lookuptable
        for (sal_Int32 l = 0; l < lookupTable.size(); l++)
            delete lookupTable[l];
        lookupTable.clear();
}

#define LBI getLocaleSpecificBreakIterator(rLocale)

sal_Int32 SAL_CALL BreakIteratorImpl::nextCharacters( const OUString& Text, sal_Int32 nStartPos,
        const Locale &rLocale, sal_Int16 nCharacterIteratorMode, sal_Int32 nCount, sal_Int32& nDone )
        throw(RuntimeException)
{
        if (nCount < 0) throw RuntimeException();

        return LBI->nextCharacters( Text, nStartPos, rLocale, nCharacterIteratorMode, nCount, nDone);
}

sal_Int32 SAL_CALL BreakIteratorImpl::previousCharacters( const OUString& Text, sal_Int32 nStartPos,
        const Locale& rLocale, sal_Int16 nCharacterIteratorMode, sal_Int32 nCount, sal_Int32& nDone )
        throw(RuntimeException)
{
        if (nCount < 0) throw RuntimeException();

        return LBI->previousCharacters( Text, nStartPos, rLocale, nCharacterIteratorMode, nCount, nDone);
}

static sal_Int32 skipSpace(const OUString& Text, sal_Int32 nPos, sal_Int32 len, sal_Int16 rWordType, sal_Bool bDirection)
{
        switch (rWordType) {
            case WordType::ANYWORD_IGNOREWHITESPACES:
                if (bDirection)
                    while (nPos < len && unicode::isWhiteSpace(Text[nPos])) nPos++;
                else
                    while (nPos > 0 && unicode::isWhiteSpace(Text[nPos-1])) nPos--;
            break;
            case WordType::DICTIONARY_WORD:
                if (bDirection)
                    while (nPos < len && (unicode::isWhiteSpace(Text[nPos]) ||
                            ! (Text[nPos] == 0x002E || unicode::isAlphaDigit(Text[nPos])))) nPos++;
                else
                    while (nPos > 0 && (unicode::isWhiteSpace(Text[nPos-1]) ||
                            ! (Text[nPos-1] == 0x002E || unicode::isAlphaDigit(Text[nPos-1])))) nPos--;
            break;
            case WordType::WORD_COUNT:
                if (bDirection)
                    while (nPos < len && (unicode::isWhiteSpace(Text[nPos]) ||
                                        ! unicode::isAlphaDigit(Text[nPos]))) nPos++;
                else
                    while (nPos > 0 && (unicode::isWhiteSpace(Text[nPos-1]) ||
                                        ! unicode::isAlphaDigit(Text[nPos-1]))) nPos--;
            break;
        }
        return nPos;
}

Boundary SAL_CALL BreakIteratorImpl::nextWord( const OUString& Text, sal_Int32 nStartPos,
        const Locale& rLocale, sal_Int16 rWordType ) throw(RuntimeException)
{
        sal_Int32 len = Text.getLength();
        if( nStartPos < 0 || len == 0 )
            result.endPos = result.startPos = 0;
        else if (nStartPos >= len)
            result.endPos = result.startPos = len;
        else {
            result = LBI->nextWord(Text, nStartPos, rLocale, rWordType);

            nStartPos = skipSpace(Text, result.startPos, len, rWordType, sal_True);

            if ( nStartPos != result.startPos) {
                if( nStartPos >= len )
                    result.startPos = result.endPos = len;
                else
                    result = LBI->getWordBoundary(Text, nStartPos, rLocale, rWordType, sal_True);
            }
        }
        return result;
}

static inline sal_Bool SAL_CALL isCJK( const Locale& rLocale ) {
        return rLocale.Language.equalsAscii("zh") || rLocale.Language.equalsAscii("ja") || rLocale.Language.equalsAscii("ko");
}

Boundary SAL_CALL BreakIteratorImpl::previousWord( const OUString& Text, sal_Int32 nStartPos,
        const Locale& rLocale, sal_Int16 rWordType) throw(RuntimeException)
{
        sal_Int32 len = Text.getLength();
        if( nStartPos <= 0 || len == 0 ) {
            result.endPos = result.startPos = 0;
            return result;
        } else if (nStartPos > len) {
            result.endPos = result.startPos = len;
            return result;
        }

        sal_Int32 nPos = skipSpace(Text, nStartPos, len, rWordType, sal_False);

        // if some spaces are skiped, and the script type is Asian with no CJK rLocale, we have to return
        // (nStartPos, -1) for caller to send correct rLocale for loading correct dictionary.
        if (nPos != nStartPos && !isCJK(rLocale) && getScriptClass(Text[nPos-1]) == ScriptType::ASIAN) {
            result.startPos = nPos;
            result.endPos = -1;
            return result;
        }

        return LBI->previousWord(Text, nPos, rLocale, rWordType);
}


Boundary SAL_CALL BreakIteratorImpl::getWordBoundary( const OUString& Text, sal_Int32 nPos, const Locale& rLocale,
        sal_Int16 rWordType, sal_Bool bDirection ) throw(RuntimeException)
{
        sal_Int32 len = Text.getLength();
        if( nPos < 0 || len == 0 )
            result.endPos = result.startPos = 0;
        else if (nPos > len)
            result.endPos = result.startPos = len;
        else {
            sal_Int32 next, prev;
            next = skipSpace(Text, nPos, len, rWordType, sal_True);
            prev = skipSpace(Text, nPos, len, rWordType, sal_False);
            if (prev == 0 && next == len) {
                result.endPos = result.startPos = nPos;
            } else if (prev == 0 && ! bDirection) {
                result.endPos = result.startPos = 0;
            } else if (next == len && bDirection) {
                result.endPos = result.startPos = len;
            } else {
                if (next != prev) {
                    if (next == nPos && next != len)
                        bDirection = sal_True;
                    else if (prev == nPos && prev != 0)
                        bDirection = sal_False;
                    else
                        nPos = bDirection ? next : prev;
                }
                result = LBI->getWordBoundary(Text, nPos, rLocale, rWordType, bDirection);
            }
        }
        return result;
}

sal_Bool SAL_CALL BreakIteratorImpl::isBeginWord( const OUString& Text, sal_Int32 nPos,
        const Locale& rLocale, sal_Int16 rWordType ) throw(RuntimeException)
{
        sal_Int32 len = Text.getLength();

        if (nPos < 0 || nPos >= len) return sal_False;

        sal_Int32 tmp = skipSpace(Text, nPos, len, rWordType, sal_True);

        if (tmp != nPos) return sal_False;

        result = getWordBoundary(Text, nPos, rLocale, rWordType, sal_True);

        return result.startPos == nPos;
}

sal_Bool SAL_CALL BreakIteratorImpl::isEndWord( const OUString& Text, sal_Int32 nPos,
        const Locale& rLocale, sal_Int16 rWordType ) throw(RuntimeException)
{
        sal_Int32 len = Text.getLength();

        if (nPos <= 0 || nPos > len) return sal_False;

        sal_Int32 tmp = skipSpace(Text, nPos, len, rWordType, sal_False);

        if (tmp != nPos) return sal_False;

        result = getWordBoundary(Text, nPos, rLocale, rWordType, sal_False);

        return result.endPos == nPos;
}

sal_Int32 SAL_CALL BreakIteratorImpl::beginOfSentence( const OUString& Text, sal_Int32 nStartPos,
        const Locale &rLocale ) throw(RuntimeException)
{
        if (nStartPos < 0 || nStartPos > Text.getLength())
            return -1;
        return LBI->beginOfSentence(Text, nStartPos, rLocale);
}

sal_Int32 SAL_CALL BreakIteratorImpl::endOfSentence( const OUString& Text, sal_Int32 nStartPos,
        const Locale &rLocale ) throw(RuntimeException)
{
        if (nStartPos < 0 || nStartPos > Text.getLength())
            return -1;
        return LBI->endOfSentence(Text, nStartPos, rLocale);
}

LineBreakResults SAL_CALL BreakIteratorImpl::getLineBreak( const OUString& Text, sal_Int32 nStartPos,
        const Locale& rLocale, sal_Int32 nMinBreakPos, const LineBreakHyphenationOptions& hOptions,
        const LineBreakUserOptions& bOptions ) throw(RuntimeException)
{
        return LBI->getLineBreak(Text, nStartPos, rLocale, nMinBreakPos, hOptions, bOptions);
}

sal_Int16 SAL_CALL BreakIteratorImpl::getScriptType( const OUString& Text, sal_Int32 nPos )
        throw(RuntimeException)
{
        return getScriptClass(Text[nPos]);
}

sal_Int32 SAL_CALL BreakIteratorImpl::beginOfScript( const OUString& Text,
        sal_Int32 nStartPos, sal_Int16 ScriptType ) throw(RuntimeException)
{
        if (nStartPos < 0 || nStartPos >= Text.getLength())
            return -1;

        if(ScriptType != getScriptClass(Text[nStartPos]))
            return -1;

        while (--nStartPos >= 0 && ScriptType == getScriptClass(Text[nStartPos])) {}

        return  ++nStartPos;
}

sal_Int32 SAL_CALL BreakIteratorImpl::endOfScript( const OUString& Text,
        sal_Int32 nStartPos, sal_Int16 ScriptType ) throw(RuntimeException)
{
        if (nStartPos < 0 || nStartPos >= Text.getLength())
            return -1;

        if(ScriptType != getScriptClass(Text[nStartPos]))
            return -1;

        sal_Int32 strLen = Text.getLength();
        while(++nStartPos < strLen ) {
            sal_Int16 currentCharScriptType = getScriptClass(Text[nStartPos]);
            if(ScriptType != currentCharScriptType && currentCharScriptType != ScriptType::WEAK)
                break;
        }
        return  nStartPos;
}

sal_Int32  SAL_CALL BreakIteratorImpl::previousScript( const OUString& Text,
        sal_Int32 nStartPos, sal_Int16 ScriptType ) throw(RuntimeException)
{
        if (nStartPos < 0)
            nStartPos = 0;
        if (nStartPos > Text.getLength())
            nStartPos = Text.getLength();

        sal_Int16 numberOfChange = (ScriptType == getScriptClass(Text[nStartPos])) ? 3 : 2;

        while (numberOfChange > 0 && --nStartPos >= 0) {
            if (((numberOfChange % 2) == 0) ^ (ScriptType != getScriptClass(Text[nStartPos])))
                numberOfChange--;
            else if (nStartPos == 0) {
                if (numberOfChange > 0)
                    numberOfChange--;
                nStartPos--;
            }
        }
        return numberOfChange == 0 ? nStartPos + 1 : -1;
}

sal_Int32 SAL_CALL BreakIteratorImpl::nextScript( const OUString& Text, sal_Int32 nStartPos,
        sal_Int16 ScriptType ) throw(RuntimeException)

{
        if (nStartPos < 0)
            nStartPos = 0;
        if (nStartPos > Text.getLength())
            nStartPos = Text.getLength();

        sal_Int16 numberOfChange = (ScriptType == getScriptClass(Text[nStartPos])) ? 2 : 1;
        sal_Int32 strLen = Text.getLength();

        while (numberOfChange > 0 && ++nStartPos < strLen) {
            sal_Int16 currentCharScriptType = getScriptClass(Text[nStartPos]);
            if ((numberOfChange == 1) ? (ScriptType == currentCharScriptType) :
                    (ScriptType != currentCharScriptType && currentCharScriptType != ScriptType::WEAK))
                numberOfChange--;
        }
        return numberOfChange == 0 ? nStartPos : -1;
}


sal_Int32 SAL_CALL BreakIteratorImpl::beginOfCharBlock( const OUString& Text, sal_Int32 nStartPos,
        const Locale& rLocale, sal_Int16 CharType ) throw(RuntimeException)
{
        if (CharType == CharType::ANY_CHAR) return 0;
        if (nStartPos < 0 || nStartPos >= Text.getLength()) return -1;
        if (CharType != unicode::getUnicodeType(Text[nStartPos])) return -1;

        while(nStartPos-- > 0 && CharType == unicode::getUnicodeType(Text[nStartPos])) {}
        return nStartPos + 1; // begin of char block is inclusive
}

sal_Int32 SAL_CALL BreakIteratorImpl::endOfCharBlock( const OUString& Text, sal_Int32 nStartPos,
        const Locale& rLocale, sal_Int16 CharType ) throw(RuntimeException)
{
        sal_Int32 strLen = Text.getLength();

        if (CharType == CharType::ANY_CHAR) return strLen; // end of char block is exclusive
        if (nStartPos < 0 || nStartPos >= strLen) return -1;
        if (CharType != unicode::getUnicodeType(Text[nStartPos])) return -1;

        while(++nStartPos < strLen && CharType == unicode::getUnicodeType(Text[nStartPos])) {}
        return nStartPos; // end of char block is exclusive
}

sal_Int32 SAL_CALL BreakIteratorImpl::nextCharBlock( const OUString& Text, sal_Int32 nStartPos,
        const Locale& rLocale, sal_Int16 CharType ) throw(RuntimeException)
{
        if (CharType == CharType::ANY_CHAR) return -1;
        if (nStartPos < 0 || nStartPos >= Text.getLength()) return -1;

        sal_Int16 numberOfChange = (CharType == unicode::getUnicodeType(Text[nStartPos])) ? 2 : 1;
        sal_Int32 strLen = Text.getLength();

        while (numberOfChange > 0 && ++nStartPos < strLen) {
            if ((numberOfChange == 1) ^ (CharType != unicode::getUnicodeType(Text[nStartPos])))
                numberOfChange--;
        }
        return numberOfChange == 0 ? nStartPos : -1;
}

sal_Int32 SAL_CALL BreakIteratorImpl::previousCharBlock( const OUString& Text, sal_Int32 nStartPos,
        const Locale& rLocale, sal_Int16 CharType ) throw(RuntimeException)
{
        if(CharType == CharType::ANY_CHAR) return -1;
        if (nStartPos < 0 || nStartPos >= Text.getLength()) return -1;

        sal_Int16 numberOfChange = (CharType == unicode::getUnicodeType(Text[nStartPos])) ? 3 : 2;

        while (numberOfChange > 0 && --nStartPos >= 0) {
            if (((numberOfChange % 2) == 0) ^ (CharType != unicode::getUnicodeType(Text[nStartPos])))
                numberOfChange--;
            if (nStartPos == 0 && numberOfChange > 0) {
                numberOfChange--;
                nStartPos--;
            }
        }
        return numberOfChange == 0 ? nStartPos + 1 : -1;
}


sal_Int16 SAL_CALL BreakIteratorImpl::getWordType( const OUString& Text,
        sal_Int32 nPos, const Locale& rLocale ) throw(RuntimeException)
{
        return 0;
}

static ScriptTypeList typeList[] = {
    { UnicodeScript_kBasicLatin, UnicodeScript_kArmenian,   ScriptType::LATIN },    // 0-9,
    { UnicodeScript_kHebrew, UnicodeScript_kMyanmar,        ScriptType::COMPLEX },  // 10-27,
    { UnicodeScript_kGeorgian, UnicodeScript_kGeorgian,     ScriptType::LATIN },    // 28,
    { UnicodeScript_kHangulJamo, UnicodeScript_kHangulJamo, ScriptType::ASIAN },    // 29,
    { UnicodeScript_kEthiopic, UnicodeScript_kRunic,        ScriptType::LATIN },    // 30-34,
    { UnicodeScript_kKhmer, UnicodeScript_kMongolian,       ScriptType::COMPLEX },  // 35-36,
    { UnicodeScript_kLatinExtendedAdditional,
      UnicodeScript_kGreekExtended,                         ScriptType::LATIN },    // 37-38,
    { UnicodeScript_kCJKRadicalsSupplement,
      UnicodeScript_kHangulSyllable,                        ScriptType::ASIAN },    // 57-73,
    { UnicodeScript_kCJKCompatibilityIdeograph,
      UnicodeScript_kCJKCompatibilityIdeograph,             ScriptType::ASIAN },    // 78,
    { UnicodeScript_kArabicPresentationA,
      UnicodeScript_kArabicPresentationA,                   ScriptType::COMPLEX },  // 80,
    { UnicodeScript_kCJKCompatibilityForm,
      UnicodeScript_kCJKCompatibilityForm,                  ScriptType::ASIAN },    // 82,
    { UnicodeScript_kArabicPresentationB,
      UnicodeScript_kArabicPresentationB,                   ScriptType::COMPLEX },  // 84,
    { UnicodeScript_kHalfwidthFullwidthForm,
      UnicodeScript_kHalfwidthFullwidthForm,                ScriptType::ASIAN },    // 86,
    { UnicodeScript_kScriptCount,
      UnicodeScript_kScriptCount,                           ScriptType::WEAK }      // 88
};

sal_Int16  BreakIteratorImpl::getScriptClass(sal_Unicode currentChar )
{
        static sal_Unicode lastChar = 0;
        static sal_Int16 nRet = 0;

        if (currentChar != lastChar) {
            lastChar = currentChar;

            //JP 21.9.2001: handle specific characters - always as weak
            //                  definition of 1 - this breaks a word
            //                  2 - this can be inside a word
            //                  0x20 & 0xA0 - Bug 102975, declare western space and non-break space as WEAK char.
            if( 1 == currentChar || 2 == currentChar || 0x20 == currentChar || 0xA0 == currentChar)
                nRet = ScriptType::WEAK;
            else
                nRet = unicode::getUnicodeScriptType( currentChar, typeList, ScriptType::WEAK );
        }
        return nRet;
}

static inline sal_Bool operator == (const Locale& l1, const Locale& l2) {
        return l1.Language == l2.Language && l1.Country == l2.Country && l1.Variant == l2.Variant;
}

sal_Bool SAL_CALL BreakIteratorImpl::createLocaleSpecificBreakIterator(const OUString& aLocaleName) throw( RuntimeException )
{
        // to share service between same Language but different Country code, like zh_CN and zh_TW
        for (sal_Int32 l = 0; l < lookupTable.size(); l++) {
            lookupTableItem *listItem = lookupTable[l];
            if (aLocaleName == listItem->aLocale.Language) {
                xBI = listItem->xBI;
                return sal_True;
            }
        }

        Reference < uno::XInterface > xI = xMSF->createInstance(
            OUString::createFromAscii("com.sun.star.i18n.BreakIterator_") + aLocaleName);

        if ( xI.is() ) {
            xI->queryInterface( getCppuType((const Reference< XBreakIterator>*)0) ) >>= xBI;
            if (xBI.is()) {
                lookupTable.push_back(new lookupTableItem(Locale(aLocaleName, aLocaleName, aLocaleName), xBI));
                return sal_True;
            }
        }
        return sal_False;
}

Reference < XBreakIterator > SAL_CALL
BreakIteratorImpl::getLocaleSpecificBreakIterator(const Locale& rLocale) throw (RuntimeException)
{
        if (xBI.is() && rLocale == aLocale)
            return xBI;
        else if (xMSF.is()) {
            aLocale = rLocale;

            for (sal_Int32 i = 0; i < lookupTable.size(); i++) {
                lookupTableItem *listItem = lookupTable[i];
                if (rLocale == listItem->aLocale)
                    return xBI = listItem->xBI;
            }

            sal_Unicode under = (sal_Unicode)'_';

            sal_Int32 l = rLocale.Language.getLength();
            sal_Int32 c = rLocale.Country.getLength();
            sal_Int32 v = rLocale.Variant.getLength();
            OUStringBuffer aBuf(l+c+v+3);

            if ((l > 0 && c > 0 && v > 0 &&
                    // load service with name <base>_<lang>_<country>_<varian>
                    createLocaleSpecificBreakIterator(aBuf.append(rLocale.Language).append(under).append(
                                    rLocale.Country).append(under).append(rLocale.Variant).makeStringAndClear())) ||
                (l > 0 && c > 0 &&
                    // load service with name <base>_<lang>_<country>
                    createLocaleSpecificBreakIterator(aBuf.append(rLocale.Language).append(under).append(
                                    rLocale.Country).makeStringAndClear())) ||
                (l > 0 && c > 0 && rLocale.Language.compareToAscii("zh") == 0 &&
                                    (rLocale.Country.compareToAscii("HK") == 0 ||
                                    rLocale.Country.compareToAscii("MO") == 0) &&
                    // if the country code is HK or MO, one more step to try TW.
                    createLocaleSpecificBreakIterator(aBuf.append(rLocale.Language).append(under).appendAscii(
                                    "TW").makeStringAndClear())) ||
                (l > 0 &&
                    // load service with name <base>_<lang>
                    createLocaleSpecificBreakIterator(rLocale.Language)) ||
                    // load default service with name <base>_Unicode
                    createLocaleSpecificBreakIterator(OUString::createFromAscii("Unicode"))) {
                lookupTable.push_back( new lookupTableItem(aLocale, xBI) );
                return xBI;
            }
        }
        throw RuntimeException();
}

const sal_Char cBreakIterator[] = "com.sun.star.i18n.BreakIterator";

OUString SAL_CALL
BreakIteratorImpl::getImplementationName(void) throw( RuntimeException )
{
        return OUString::createFromAscii(cBreakIterator);
}

sal_Bool SAL_CALL
BreakIteratorImpl::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
        return !rServiceName.compareToAscii(cBreakIterator);
}

Sequence< OUString > SAL_CALL
BreakIteratorImpl::getSupportedServiceNames(void) throw( RuntimeException )
{
        Sequence< OUString > aRet(1);
        aRet[0] = OUString::createFromAscii(cBreakIterator);
        return aRet;
}

} } } }

