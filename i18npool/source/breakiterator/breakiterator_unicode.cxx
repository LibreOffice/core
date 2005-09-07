/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: breakiterator_unicode.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:01:31 $
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
        characterBreak = dictWordBreak = editWordBreak = countWordBreak = sentenceBreak = lineBreak = NULL;
        characterText = dictWordText = editWordText = countWordText = sentenceText = lineText = OUString();
        cBreakIterator = ImplementName;
}


BreakIterator_Unicode::~BreakIterator_Unicode()
{
        if (characterBreak) delete characterBreak;
        if (dictWordBreak) delete dictWordBreak;
        if (editWordBreak) delete editWordBreak;
        if (countWordBreak) delete countWordBreak;
        if (sentenceBreak) delete sentenceBreak;
        if (lineBreak) delete lineBreak;
}

static icu::BreakIterator* loadICURuleBasedBreakIterator(const char* name) throw(RuntimeException)
{
        UErrorCode status = U_ZERO_ERROR;

        udata_setAppData("OpenOffice", OpenOffice_icu_dat, &status);
        if ( !U_SUCCESS(status) ) throw ERROR;

        status = U_ZERO_ERROR;
        icu::BreakIterator* breakiterator = new RuleBasedBreakIterator(udata_open("OpenOffice", "brk", name, &status), status);
        if ( !U_SUCCESS(status) ) throw ERROR;

        return breakiterator;
}


// loading ICU breakiterator on demand.
icu::BreakIterator* SAL_CALL BreakIterator_Unicode::loadICUBreakIterator(const com::sun::star::lang::Locale& rLocale,
        sal_Int16 which) throw(RuntimeException)
{
        icu::BreakIterator *breakiterator = NULL;
        icu::Locale icuLocale(
                OUStringToOString(rLocale.Language, RTL_TEXTENCODING_ASCII_US).getStr(),
                OUStringToOString(rLocale.Country, RTL_TEXTENCODING_ASCII_US).getStr(),
                OUStringToOString(rLocale.Variant, RTL_TEXTENCODING_ASCII_US).getStr());

        UErrorCode status = U_ZERO_ERROR;
        switch (which) {
            case LOAD_CHARACTER_BREAKITERATOR:
                breakiterator =  icu::BreakIterator::createCharacterInstance(icuLocale, status);
                break;
            case LOAD_WORD_BREAKITERATOR:
                breakiterator =  icu::BreakIterator::createWordInstance(icuLocale, status);
                break;
            case LOAD_SENTENCE_BREAKITERATOR:
                breakiterator = icu::BreakIterator::createSentenceInstance(icuLocale, status);
                break;
            case LOAD_LINE_BREAKITERATOR:
                breakiterator = icu::BreakIterator::createLineInstance(icuLocale, status);
                break;
        }
        if ( !U_SUCCESS(status) ) throw ERROR;

        return breakiterator;
}

icu::BreakIterator* SAL_CALL BreakIterator_Unicode::loadICUWordBreakIterator(const OUString& Text, sal_Int32 nStartPos, const lang::Locale& rLocale,
        sal_Int16 rWordType) throw(RuntimeException)
{
        sal_Bool newBreak = sal_False;
        if (rWordType == WordType::WORD_COUNT) {
            if (!countWordBreak) {
                newBreak = sal_True;
                if (rLocale.Language.compareToAscii("th") == 0)
                    countWordBreak = loadICUBreakIterator(rLocale, LOAD_WORD_BREAKITERATOR);
                else
                    countWordBreak = loadICURuleBasedBreakIterator("count_word");
            }
            if (newBreak || !countWordText.equals(Text)) {
                countWordText = Text;
                countWordBreak->setText(UnicodeString(Text.getStr(), Text.getLength()));
            }
            return countWordBreak;
        }
        else if (rWordType == WordType::DICTIONARY_WORD) {
            if (!dictWordBreak) {
                newBreak = sal_True;
                if (rLocale.Language.compareToAscii("th") == 0)
                    dictWordBreak = loadICUBreakIterator(rLocale, LOAD_WORD_BREAKITERATOR);
                else
                    dictWordBreak = loadICURuleBasedBreakIterator("dict_word");
            }
            if (newBreak || !dictWordText.equals(Text)) {
                dictWordText = Text;
                dictWordBreak->setText(UnicodeString(Text.getStr(), Text.getLength()));
            }
            return dictWordBreak;
        } else {
            if (!editWordBreak) {
                newBreak = sal_True;
                if (rLocale.Language.compareToAscii("th") == 0)
                    editWordBreak = loadICUBreakIterator(rLocale, LOAD_WORD_BREAKITERATOR);
                else
                    editWordBreak = loadICURuleBasedBreakIterator("edit_word");
            }
            if (newBreak || !editWordText.equals(Text)) {
                editWordText = Text;
                editWordBreak->setText(UnicodeString(Text.getStr(), Text.getLength()));
                if (nStartPos != 0 && rLocale.Language.compareToAscii("th") == 0)
                    editWordBreak->following(0);
            }
            return editWordBreak;
        }
}

icu::BreakIterator* SAL_CALL BreakIterator_ca::loadICUWordBreakIterator(const OUString& Text, sal_Int32 nStartPos, const lang::Locale& rLocale,
        sal_Int16 rWordType) throw(RuntimeException)
{
        sal_Bool newBreak = sal_False;
        if (rWordType == WordType::DICTIONARY_WORD) {
            if (! dictWordBreak) {
                newBreak = sal_True;
                dictWordBreak = loadICURuleBasedBreakIterator("dict_word_ca");
            }
            if (newBreak || !dictWordText.equals(Text)) {
                dictWordText = Text;
                dictWordBreak->setText(UnicodeString(Text.getStr(), Text.getLength()));
            }
            return dictWordBreak;
        } else
            return BreakIterator_Unicode::loadICUWordBreakIterator(Text, nStartPos, rLocale, rWordType);
}

icu::BreakIterator* SAL_CALL BreakIterator_hu::loadICUWordBreakIterator(const OUString& Text, sal_Int32 nStartPos, const lang::Locale& rLocale,
        sal_Int16 rWordType) throw(RuntimeException)
{
        sal_Bool newBreak = sal_False;
        if (rWordType == WordType::DICTIONARY_WORD) {
            if (!dictWordBreak) {
                newBreak = sal_True;
                dictWordBreak = loadICURuleBasedBreakIterator("dict_word_hu");
            }
            if (newBreak || !dictWordText.equals(Text)) {
                dictWordText = Text;
                dictWordBreak->setText(UnicodeString(Text.getStr(), Text.getLength()));
            }
            return dictWordBreak;
        } else
            return BreakIterator_Unicode::loadICUWordBreakIterator(Text, nStartPos, rLocale, rWordType);
}

sal_Int32 SAL_CALL BreakIterator_Unicode::nextCharacters( const OUString& Text,
        sal_Int32 nStartPos, const lang::Locale &rLocale,
        sal_Int16 nCharacterIteratorMode, sal_Int32 nCount, sal_Int32& nDone )
        throw(RuntimeException)
{
        if (nCharacterIteratorMode == CharacterIteratorMode::SKIPCELL ) { // for CELL mode
            sal_Bool newBreak = sal_False;
            if (!characterBreak) {
                newBreak = sal_True;
                characterBreak = loadICUBreakIterator(rLocale, LOAD_CHARACTER_BREAKITERATOR);
            }

            if (newBreak || !characterText.equals(Text)) {
                characterText = Text;
                characterBreak->setText(UnicodeString(Text.getStr(), Text.getLength()));
            }
            for (nDone = 0; nDone < nCount; nDone++) {
                nStartPos = characterBreak->following(nStartPos);
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
            sal_Bool newBreak = sal_False;
            if (!characterBreak) {
                newBreak = sal_True;
                characterBreak = loadICUBreakIterator(rLocale, LOAD_CHARACTER_BREAKITERATOR);
            }

            if (newBreak || !characterText.equals(Text)) {
                characterText = Text;
                characterBreak->setText(UnicodeString(Text.getStr(), Text.getLength()));
            }
            for (nDone = 0; nDone < nCount; nDone++) {
                nStartPos = characterBreak->preceding(nStartPos);
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
        icu::BreakIterator* wordBreak = loadICUWordBreakIterator(Text, nStartPos, rLocale, rWordType);

        result.startPos = wordBreak->following(nStartPos);
        if( result.startPos >= Text.getLength() || result.startPos == BreakIterator::DONE )
            result.endPos = result.startPos;
        else {
            if ( (rWordType == WordType::ANYWORD_IGNOREWHITESPACES ||
                    rWordType == WordType::DICTIONARY_WORD ) &&
                        unicode::isWhiteSpace(Text[result.startPos]) )
                result.startPos = wordBreak->following(result.startPos);

            result.endPos = wordBreak->following(result.startPos);
            if(result.endPos == BreakIterator::DONE)
                result.endPos = result.startPos;
        }
        return result;
}


Boundary SAL_CALL BreakIterator_Unicode::previousWord(const OUString& Text, sal_Int32 nStartPos,
        const lang::Locale& rLocale, sal_Int16 rWordType) throw(RuntimeException)
{
        icu::BreakIterator* wordBreak = loadICUWordBreakIterator(Text, nStartPos, rLocale, rWordType);

        result.startPos = wordBreak->preceding(nStartPos);
        if( result.startPos < 0 || result.startPos == BreakIterator::DONE)
            result.endPos = result.startPos;
        else {
            if ( (rWordType == WordType::ANYWORD_IGNOREWHITESPACES ||
                    rWordType == WordType::DICTIONARY_WORD) &&
                        unicode::isWhiteSpace(Text[result.startPos]) )
                result.startPos = wordBreak->preceding(result.startPos);

            result.endPos = wordBreak->following(result.startPos);
            if(result.endPos == BreakIterator::DONE)
                result.endPos = result.startPos;
        }
        return result;
}


Boundary SAL_CALL BreakIterator_Unicode::getWordBoundary( const OUString& Text, sal_Int32 nPos, const lang::Locale& rLocale,
        sal_Int16 rWordType, sal_Bool bDirection ) throw(RuntimeException)
{
        icu::BreakIterator* wordBreak = loadICUWordBreakIterator(Text, nPos, rLocale, rWordType);
        sal_Int32 len = Text.getLength();

        if(wordBreak->isBoundary(nPos)) {
            result.startPos = result.endPos = nPos;
            if((bDirection || nPos == 0) && nPos < len) //forward
                result.endPos = wordBreak->following(nPos);
            else
                result.startPos = wordBreak->preceding(nPos);
        } else {
            if(nPos <= 0) {
                result.startPos = 0;
                result.endPos = len ? wordBreak->following((sal_Int32)0) : 0;
            } else if(nPos >= len) {
                result.startPos = wordBreak->preceding(len);
                result.endPos = len;
            } else {
                result.startPos = wordBreak->preceding(nPos);
                result.endPos = wordBreak->following(nPos);
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
        sal_Bool newBreak = sal_False;
        if (!sentenceBreak) {
            newBreak = sal_True;
            sentenceBreak = loadICUBreakIterator(rLocale, LOAD_SENTENCE_BREAKITERATOR);
        }

        sal_Int32 len = Text.getLength();
        if (newBreak || !sentenceText.equals(Text)) {
            sentenceText = Text;
            sentenceBreak->setText(UnicodeString(Text.getStr(), len));
        }
        if (len > 0 && nStartPos == len)
            nStartPos--; // issue #i27703# treat end position as part of last sentence
        if (!sentenceBreak->isBoundary(nStartPos))
            nStartPos = sentenceBreak->preceding(nStartPos);

        // skip preceding space.
        while (nStartPos < len && unicode::isWhiteSpace(Text[nStartPos])) nStartPos++;

        return nStartPos;
}

sal_Int32 SAL_CALL BreakIterator_Unicode::endOfSentence( const OUString& Text, sal_Int32 nStartPos,
        const lang::Locale &rLocale ) throw(RuntimeException)
{
        sal_Bool newBreak = sal_False;
        if (!sentenceBreak) {
            newBreak = sal_True;
            sentenceBreak = loadICUBreakIterator(rLocale, LOAD_SENTENCE_BREAKITERATOR);
        }

        sal_Int32 len = Text.getLength();
        if (newBreak || !sentenceText.equals(Text)) {
            sentenceText = Text;
            sentenceBreak->setText(UnicodeString(Text.getStr(), len));
        }
        if (len > 0 && nStartPos == len)
            nStartPos--; // issue #i27703# treat end position as part of last sentence
        nStartPos = sentenceBreak->following(nStartPos);

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

        sal_Bool newBreak = sal_False;
        if (!lineBreak) {
            newBreak = sal_True;
            if (rLocale.Language.compareToAscii("th") == 0)
                lineBreak = loadICUBreakIterator(rLocale, LOAD_LINE_BREAKITERATOR);
            else
                lineBreak = loadICURuleBasedBreakIterator("line");
        }

        if (newBreak || !lineText.equals(Text)) {
            lineText = Text;
            lineBreak->setText(UnicodeString(Text.getStr(), Text.getLength()));
        }

        if (lineBreak->isBoundary(nStartPos)) { //Line boundary break
            lbr.breakIndex = nStartPos;
            lbr.breakType = BreakType::WORDBOUNDARY;
        } else if (hOptions.rHyphenator.is()) { //Hyphenation break
            Boundary wBoundary = getWordBoundary( Text, nStartPos, rLocale,
                                            WordType::DICTIONARY_WORD, false);
            Reference< linguistic2::XHyphenatedWord > aHyphenatedWord;
            aHyphenatedWord = hOptions.rHyphenator->hyphenate(Text.copy(wBoundary.startPos,
                wBoundary.endPos - wBoundary.startPos), rLocale,
                hOptions.hyphenIndex - wBoundary.startPos, hOptions.aHyphenationOptions);
            if (aHyphenatedWord.is()) {
                lbr.rHyphenatedWord = aHyphenatedWord;
                if(wBoundary.startPos + aHyphenatedWord->getHyphenationPos() + 1 < nMinBreakPos )
                    lbr.breakIndex = -1;
                else
                    lbr.breakIndex = wBoundary.startPos; //aHyphenatedWord->getHyphenationPos();
                lbr.breakType = BreakType::HYPHENATION;
            } else {
                lbr.breakIndex = lineBreak->preceding(nStartPos);
                lbr.breakType = BreakType::WORDBOUNDARY;;
            }
        } else { //word boundary break
            lbr.breakIndex = lineBreak->preceding(nStartPos);
            lbr.breakType = BreakType::WORDBOUNDARY;
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
