/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: breakiterator_unicode.cxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: obo $ $Date: 2005-11-16 10:18:37 $
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
#include <breakiterator_unicode.hxx>
#include <i18nutil/unicode.hxx>
#include <unicode/locid.h>
#include <unicode/rbbi.h>
#include <unicode/udata.h>

U_CDECL_BEGIN
extern const char OpenOffice_icu_dat[];
U_CDECL_END

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::rtl;

namespace com { namespace sun { namespace star { namespace i18n {

#define ERROR RuntimeException()

#define ImplementName "com.sun.star.i18n.BreakIterator_Unicode";


BreakIterator_Unicode::BreakIterator_Unicode()
{
        wordRule="word";
        lineRule="line";
        aBreakIterator = NULL;
        aText = OUString();
        cBreakIterator = ImplementName;
}


BreakIterator_Unicode::~BreakIterator_Unicode()
{
        if (aBreakIterator) delete aBreakIterator;
}

// loading ICU breakiterator on demand.
void SAL_CALL BreakIterator_Unicode::loadICUBreakIterator(const com::sun::star::lang::Locale& rLocale,
        sal_Int16 rBreakType, sal_Int16 rWordType, const sal_Char *rule, const OUString& rText) throw(RuntimeException)
{
    sal_Bool newBreak = sal_False;
    UErrorCode status = U_ZERO_ERROR;
    if (!aBreakIterator || rBreakType != aBreakType || rWordType != aWordType ||
            rLocale.Language != aLocale.Language || rLocale.Country != aLocale.Country ||
            rLocale.Variant != aLocale.Variant) {
        if (aBreakIterator) {
            delete aBreakIterator;
            aBreakIterator=NULL;
        }
        if (rule) {
            if (rWordType)
                rule = rWordType == WordType::WORD_COUNT ? "count_word" :
                        rWordType == WordType::DICTIONARY_WORD ? "dict_word" : "edit_word";

            status = U_ZERO_ERROR;
            udata_setAppData("OpenOffice", OpenOffice_icu_dat, &status);
            if ( !U_SUCCESS(status) ) throw ERROR;

            status = U_ZERO_ERROR;
            aBreakIterator = new RuleBasedBreakIterator(udata_open("OpenOffice", "brk",
                    OUStringToOString(OUString::createFromAscii(rule)+OUString::createFromAscii("_")+rLocale.Language,
                    RTL_TEXTENCODING_ASCII_US).getStr(), &status), status);
            if (!U_SUCCESS(status) ) {
                status = U_ZERO_ERROR;
                aBreakIterator = new RuleBasedBreakIterator(udata_open("OpenOffice", "brk", rule, &status), status);
                if (!U_SUCCESS(status) ) aBreakIterator=NULL;
            }
        }

        if (!aBreakIterator) {
            icu::Locale icuLocale(
                    OUStringToOString(rLocale.Language, RTL_TEXTENCODING_ASCII_US).getStr(),
                    OUStringToOString(rLocale.Country, RTL_TEXTENCODING_ASCII_US).getStr(),
                    OUStringToOString(rLocale.Variant, RTL_TEXTENCODING_ASCII_US).getStr());

            status = U_ZERO_ERROR;
            switch (rBreakType) {
                case LOAD_CHARACTER_BREAKITERATOR:
                    aBreakIterator =  icu::BreakIterator::createCharacterInstance(icuLocale, status);
                    break;
                case LOAD_WORD_BREAKITERATOR:
                    aBreakIterator =  icu::BreakIterator::createWordInstance(icuLocale, status);
                    break;
                case LOAD_SENTENCE_BREAKITERATOR:
                    aBreakIterator = icu::BreakIterator::createSentenceInstance(icuLocale, status);
                    break;
                case LOAD_LINE_BREAKITERATOR:
                    aBreakIterator = icu::BreakIterator::createLineInstance(icuLocale, status);
                    break;
            }
            if ( !U_SUCCESS(status) ) {
                aBreakIterator=NULL;
                throw ERROR;
            }
        }
        if (aBreakIterator) {
            aLocale=rLocale;
            aWordType=rWordType;
            aBreakType=rBreakType;
            newBreak=sal_True;
        } else {
            throw ERROR;
        }
    }

    if (newBreak || !aText.equals(rText)) {
        aText = rText;
        aBreakIterator->setText(UnicodeString(aText.getStr(), aText.getLength()));
    }
}


sal_Int32 SAL_CALL BreakIterator_Unicode::nextCharacters( const OUString& Text,
        sal_Int32 nStartPos, const lang::Locale &rLocale,
        sal_Int16 nCharacterIteratorMode, sal_Int32 nCount, sal_Int32& nDone )
        throw(RuntimeException)
{
        if (nCharacterIteratorMode == CharacterIteratorMode::SKIPCELL ) { // for CELL mode
            loadICUBreakIterator(rLocale, LOAD_CHARACTER_BREAKITERATOR, 0, "char", Text);
            for (nDone = 0; nDone < nCount; nDone++) {
                nStartPos = aBreakIterator->following(nStartPos);
                if (nStartPos == BreakIterator::DONE)
                    return Text.getLength();
            }
        } else { // for CHARACTER mode
            nDone = Text.getLength() - nStartPos;
            if (nDone > nCount)
                nDone = nCount;
            nStartPos += nDone;
        }
        return nStartPos;
}

sal_Int32 SAL_CALL BreakIterator_Unicode::previousCharacters( const OUString& Text,
        sal_Int32 nStartPos, const lang::Locale& rLocale,
        sal_Int16 nCharacterIteratorMode, sal_Int32 nCount, sal_Int32& nDone )
        throw(RuntimeException)
{
        if (nCharacterIteratorMode == CharacterIteratorMode::SKIPCELL ) { // for CELL mode
            loadICUBreakIterator(rLocale, LOAD_CHARACTER_BREAKITERATOR, 0, "char", Text);
            for (nDone = 0; nDone < nCount; nDone++) {
                nStartPos = aBreakIterator->preceding(nStartPos);
                if (nStartPos == BreakIterator::DONE)
                    return 0;
            }
        } else { // for BS to delete one char and CHARACTER mode.
            nDone = (nStartPos > nCount) ? nCount : nStartPos;
            nStartPos -= nDone;
        }
        return nStartPos;
}


Boundary SAL_CALL BreakIterator_Unicode::nextWord( const OUString& Text, sal_Int32 nStartPos,
    const lang::Locale& rLocale, sal_Int16 rWordType ) throw(RuntimeException)
{
        loadICUBreakIterator(rLocale, LOAD_WORD_BREAKITERATOR, rWordType, wordRule, Text);

        result.startPos = aBreakIterator->following(nStartPos);
        if( result.startPos >= Text.getLength() || result.startPos == BreakIterator::DONE )
            result.endPos = result.startPos;
        else {
            if ( (rWordType == WordType::ANYWORD_IGNOREWHITESPACES ||
                    rWordType == WordType::DICTIONARY_WORD ) &&
                        unicode::isWhiteSpace(Text[result.startPos]) )
                result.startPos = aBreakIterator->following(result.startPos);

            result.endPos = aBreakIterator->following(result.startPos);
            if(result.endPos == BreakIterator::DONE)
                result.endPos = result.startPos;
        }
        return result;
}


Boundary SAL_CALL BreakIterator_Unicode::previousWord(const OUString& Text, sal_Int32 nStartPos,
        const lang::Locale& rLocale, sal_Int16 rWordType) throw(RuntimeException)
{
        loadICUBreakIterator(rLocale, LOAD_WORD_BREAKITERATOR, rWordType, wordRule, Text);

        result.startPos = aBreakIterator->preceding(nStartPos);
        if( result.startPos < 0 || result.startPos == BreakIterator::DONE)
            result.endPos = result.startPos;
        else {
            if ( (rWordType == WordType::ANYWORD_IGNOREWHITESPACES ||
                    rWordType == WordType::DICTIONARY_WORD) &&
                        unicode::isWhiteSpace(Text[result.startPos]) )
                result.startPos = aBreakIterator->preceding(result.startPos);

            result.endPos = aBreakIterator->following(result.startPos);
            if(result.endPos == BreakIterator::DONE)
                result.endPos = result.startPos;
        }
        return result;
}


Boundary SAL_CALL BreakIterator_Unicode::getWordBoundary( const OUString& Text, sal_Int32 nPos, const lang::Locale& rLocale,
        sal_Int16 rWordType, sal_Bool bDirection ) throw(RuntimeException)
{
        loadICUBreakIterator(rLocale, LOAD_WORD_BREAKITERATOR, rWordType, wordRule, Text);
        sal_Int32 len = Text.getLength();

        if(aBreakIterator->isBoundary(nPos)) {
            result.startPos = result.endPos = nPos;
            if((bDirection || nPos == 0) && nPos < len) //forward
                result.endPos = aBreakIterator->following(nPos);
            else
                result.startPos = aBreakIterator->preceding(nPos);
        } else {
            if(nPos <= 0) {
                result.startPos = 0;
                result.endPos = len ? aBreakIterator->following((sal_Int32)0) : 0;
            } else if(nPos >= len) {
                result.startPos = aBreakIterator->preceding(len);
                result.endPos = len;
            } else {
                result.startPos = aBreakIterator->preceding(nPos);
                result.endPos = aBreakIterator->following(nPos);
            }
        }
        if (result.startPos == BreakIterator::DONE)
            result.startPos = result.endPos;
        else if (result.endPos == BreakIterator::DONE)
            result.endPos = result.startPos;

        return result;
}


sal_Int32 SAL_CALL BreakIterator_Unicode::beginOfSentence( const OUString& Text, sal_Int32 nStartPos,
        const lang::Locale &rLocale ) throw(RuntimeException)
{
        loadICUBreakIterator(rLocale, LOAD_SENTENCE_BREAKITERATOR, 0, NULL, Text);

        sal_Int32 len = Text.getLength();
        if (len > 0 && nStartPos == len)
            nStartPos--; // issue #i27703# treat end position as part of last sentence
        if (!aBreakIterator->isBoundary(nStartPos))
            nStartPos = aBreakIterator->preceding(nStartPos);

        // skip preceding space.
        while (nStartPos < len && unicode::isWhiteSpace(Text[nStartPos])) nStartPos++;

        return nStartPos;
}

sal_Int32 SAL_CALL BreakIterator_Unicode::endOfSentence( const OUString& Text, sal_Int32 nStartPos,
        const lang::Locale &rLocale ) throw(RuntimeException)
{
        loadICUBreakIterator(rLocale, LOAD_SENTENCE_BREAKITERATOR, 0, NULL, Text);

        sal_Int32 len = Text.getLength();
        if (len > 0 && nStartPos == len)
            nStartPos--; // issue #i27703# treat end position as part of last sentence
        nStartPos = aBreakIterator->following(nStartPos);

        while (--nStartPos >= 0 && unicode::isWhiteSpace(Text[nStartPos]));

        return ++nStartPos;
}

LineBreakResults SAL_CALL BreakIterator_Unicode::getLineBreak(
        const OUString& Text, sal_Int32 nStartPos,
        const lang::Locale& rLocale, sal_Int32 nMinBreakPos,
        const LineBreakHyphenationOptions& hOptions,
        const LineBreakUserOptions& bOptions ) throw(RuntimeException)
{
        LineBreakResults lbr;

        loadICUBreakIterator(rLocale, LOAD_LINE_BREAKITERATOR, 0, lineRule, Text);

        sal_Bool GlueSpace=sal_True;
        while (GlueSpace) {
            if (aBreakIterator->isBoundary(nStartPos)) { //Line boundary break
                lbr.breakIndex = nStartPos;
                lbr.breakType = BreakType::WORDBOUNDARY;
            } else if (hOptions.rHyphenator.is()) { //Hyphenation break
                Boundary wBoundary = getWordBoundary( Text, nStartPos, rLocale,
                                                WordType::DICTIONARY_WORD, false);
                Reference< linguistic2::XHyphenatedWord > aHyphenatedWord;
                aHyphenatedWord = hOptions.rHyphenator->hyphenate(Text.copy(wBoundary.startPos,
                    wBoundary.endPos - wBoundary.startPos), rLocale,
                    (sal_Int16) (hOptions.hyphenIndex - wBoundary.startPos), hOptions.aHyphenationOptions);
                if (aHyphenatedWord.is()) {
                    lbr.rHyphenatedWord = aHyphenatedWord;
                    if(wBoundary.startPos + aHyphenatedWord->getHyphenationPos() + 1 < nMinBreakPos )
                        lbr.breakIndex = -1;
                    else
                        lbr.breakIndex = wBoundary.startPos; //aHyphenatedWord->getHyphenationPos();
                    lbr.breakType = BreakType::HYPHENATION;
                } else {
                    lbr.breakIndex = aBreakIterator->preceding(nStartPos);
                    lbr.breakType = BreakType::WORDBOUNDARY;;
                }
            } else { //word boundary break
                lbr.breakIndex = aBreakIterator->preceding(nStartPos);
                lbr.breakType = BreakType::WORDBOUNDARY;
            }

#define WJ 0x2060   // Word Joiner
            GlueSpace=sal_False;
            if (lbr.breakType == BreakType::WORDBOUNDARY) {
                nStartPos = lbr.breakIndex;
                if (Text[nStartPos--] == WJ)
                    GlueSpace=sal_True;
                while (nStartPos >= 0 &&
                    (unicode::isWhiteSpace(Text[nStartPos]) || Text[nStartPos] == WJ)) {
                    if (Text[nStartPos--] == WJ)
                        GlueSpace=sal_True;
                }
                if (GlueSpace && nStartPos < 0)  {
                    lbr.breakIndex = 0;
                    break;
                }
            }
        }

        return lbr;
}



OUString SAL_CALL
BreakIterator_Unicode::getImplementationName(void) throw( RuntimeException )
{
        return OUString::createFromAscii(cBreakIterator);
}

sal_Bool SAL_CALL
BreakIterator_Unicode::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
        return !rServiceName.compareToAscii(cBreakIterator);
}

Sequence< OUString > SAL_CALL
BreakIterator_Unicode::getSupportedServiceNames(void) throw( RuntimeException )
{
        Sequence< OUString > aRet(1);
        aRet[0] = OUString::createFromAscii(cBreakIterator);
        return aRet;
}

} } } }
