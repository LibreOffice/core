/*************************************************************************
 *
 *  $RCSfile: breakiterator_unicode.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: bustamam $ $Date: 2002-03-26 06:26:55 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
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
#include <breakiterator_unicode.hxx>
#include <unicode.hxx>
#include <unicode/locid.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::rtl;

namespace com { namespace sun { namespace star { namespace i18n {

#define ERROR RuntimeException()
static UErrorCode status; // status is shared in all calls to Calendar, it has to be reset for each call.

#define LOAD_CHARACTER_BREAKITERATOR    0
#define LOAD_WORD_BREAKITERATOR     1
#define LOAD_SENTENCE_BREAKITERATOR 2
#define LOAD_LINE_BREAKITERATOR     3

#define ImplementName "com.sun.star.i18n.BreakIterator_Unicode";

BreakIterator_Unicode::BreakIterator_Unicode()
{
    characterBreak = wordBreak = sentenceBreak = lineBreak = NULL;
    cBreakIterator = ImplementName;
}


BreakIterator_Unicode::~BreakIterator_Unicode()
{
    if (characterBreak) delete characterBreak;
    if (wordBreak) delete wordBreak;
    if (sentenceBreak) delete sentenceBreak;
    if (lineBreak) delete lineBreak;
}

// loading ICU breakiterator on demand.
static icu::BreakIterator* SAL_CALL loadICUBreakIterator(const com::sun::star::lang::Locale& rLocale, sal_Int16 which) throw(RuntimeException)
{
    icu::BreakIterator *breakiterator = NULL;
    icu::Locale icuLocale(
        OUStringToOString(rLocale.Language, RTL_TEXTENCODING_ASCII_US).getStr(),
        OUStringToOString(rLocale.Country, RTL_TEXTENCODING_ASCII_US).getStr(),
        OUStringToOString(rLocale.Variant, RTL_TEXTENCODING_ASCII_US).getStr());

    status = U_ZERO_ERROR;
    switch (which) {
        case LOAD_CHARACTER_BREAKITERATOR:
        breakiterator =  icu::BreakIterator::createCharacterInstance(icuLocale, status);
        break;
        case LOAD_WORD_BREAKITERATOR:
        breakiterator = icu::BreakIterator::createWordInstance(icuLocale, status);
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


sal_Int32 SAL_CALL BreakIterator_Unicode::nextCharacters( const OUString& Text,
    sal_Int32 nStartPos, const lang::Locale &rLocale,
    sal_Int16 nCharacterIteratorMode, sal_Int32 nCount, sal_Int32& nDone )
    throw(RuntimeException)
{
    if (!characterBreak) characterBreak = loadICUBreakIterator(rLocale, LOAD_CHARACTER_BREAKITERATOR);

    characterBreak->setText(UnicodeString(Text.getStr(), Text.getLength()));
    for (nDone = 0; nDone < nCount; nDone++) {
        nStartPos = characterBreak->following(nStartPos);
        if (nStartPos == BreakIterator::DONE)
        return Text.getLength();
    }
    return nStartPos;
}

sal_Int32 SAL_CALL BreakIterator_Unicode::previousCharacters( const OUString& Text,
    sal_Int32 nStartPos, const lang::Locale& rLocale,
    sal_Int16 nCharacterIteratorMode, sal_Int32 nCount, sal_Int32& nDone )
    throw(RuntimeException)
{
    if (!characterBreak) characterBreak = loadICUBreakIterator(rLocale, LOAD_CHARACTER_BREAKITERATOR);

    characterBreak->setText(UnicodeString(Text.getStr(), Text.getLength()));
    for (nDone = 0; nDone < nCount; nDone++) {
        nStartPos = characterBreak->preceding(nStartPos);
        if (nStartPos == BreakIterator::DONE)
        return 0;
    }
    return nStartPos;
}

// Since ICU breakiterator does not handle CTL script properly, we have to seperate script segment
// to insure CTL script will be handled by our Breakiterator.
sal_Int32 SAL_CALL BreakIterator_Unicode::setTextByScriptBoundary(const OUString& Text,
    sal_Int32 nPos)
{
    if (cachedText != Text)
        cachedText = Text;
    else if ( nPos >= scriptStart && nPos < scriptStart + scriptLength) {
        wordBreak->setText(UnicodeString(Text.getStr() + scriptStart, scriptLength));
        return scriptStart;
    }

    sal_Int16 type, sType = getScriptClass(Text[nPos]);

    for (scriptStart = nPos - 1; scriptStart >= 0; scriptStart--) {
        type = getScriptClass(Text[scriptStart]);
        if (sType != type && type != ScriptType::WEAK)
        break;
    }
    scriptStart++;

    sal_Int32 len = Text.getLength();
    if (nPos < len) {
        for(nPos++; nPos < len; nPos++ ) {
        type = getScriptClass(Text[nPos]);
        if(sType != type && type != ScriptType::WEAK)
            break;
        }
    }
    scriptLength = nPos - scriptStart;
    wordBreak->setText(UnicodeString(Text.getStr() + scriptStart, scriptLength));
    return scriptStart;
}

Boundary SAL_CALL BreakIterator_Unicode::nextWord( const OUString& Text, sal_Int32 nStartPos,
    const lang::Locale& rLocale, sal_Int16 rWordType ) throw(RuntimeException)
{
    if (!wordBreak) wordBreak = loadICUBreakIterator(rLocale, LOAD_WORD_BREAKITERATOR);

    nStartPos -= setTextByScriptBoundary(Text, nStartPos);

    result.startPos = wordBreak->following(nStartPos);
    if( result.startPos >= scriptLength || result.startPos == BreakIterator::DONE )
        result.endPos = result.startPos;
    else {
        if ( (rWordType == WordType::ANYWORD_IGNOREWHITESPACES ||
            rWordType == WordType::DICTIONARY_WORD ) &&
            unicode::isWhiteSpace(Text[result.startPos + scriptStart]) )
        result.startPos = wordBreak->following(result.startPos);

        result.endPos = wordBreak->following(result.startPos);
        if(result.endPos == BreakIterator::DONE)
        result.endPos = result.startPos;
    }
    result.startPos += scriptStart;
    result.endPos += scriptStart;
    return result;
}


Boundary SAL_CALL BreakIterator_Unicode::previousWord(const OUString& Text, sal_Int32 nStartPos,
    const lang::Locale& rLocale, sal_Int16 rWordType) throw(RuntimeException)
{
    if (!wordBreak) wordBreak = loadICUBreakIterator(rLocale, LOAD_WORD_BREAKITERATOR);

    nStartPos -= setTextByScriptBoundary(Text, nStartPos > 0 ? nStartPos-1 : nStartPos);

    result.startPos = wordBreak->preceding(nStartPos);
    if( result.startPos < 0 || result.startPos == BreakIterator::DONE)
        result.endPos = result.startPos;
    else {
        if ( (rWordType == WordType::ANYWORD_IGNOREWHITESPACES ||
            rWordType == WordType::DICTIONARY_WORD) &&
            unicode::isWhiteSpace(Text[result.startPos + scriptStart]) )
        result.startPos = wordBreak->preceding(result.startPos);

        result.endPos = wordBreak->following(result.startPos);
        if(result.endPos == BreakIterator::DONE)
        result.endPos = result.startPos;
    }
    result.startPos += scriptStart;
    result.endPos += scriptStart;
    return result;
}


Boundary SAL_CALL BreakIterator_Unicode::getWordBoundary( const OUString& Text, sal_Int32 nPos, const lang::Locale& rLocale,
    sal_Int16 rWordType, sal_Bool bDirection ) throw(RuntimeException)
{
    if (!wordBreak) wordBreak = loadICUBreakIterator(rLocale, LOAD_WORD_BREAKITERATOR);

    nPos -= setTextByScriptBoundary(Text, nPos);

    if(wordBreak->isBoundary(nPos)) {
        result.startPos = result.endPos = nPos;
        if((bDirection || nPos == 0) && nPos < scriptLength) //forward
        result.endPos = wordBreak->following(nPos);
        else
        result.startPos = wordBreak->preceding(nPos);
    } else {
        if(nPos <= 0) {
        result.startPos = 0;
        result.endPos = scriptLength ? wordBreak->following((sal_Int32)0) : 0;
        } else if(nPos >= scriptLength) {
        result.startPos = wordBreak->preceding(scriptLength);
        result.endPos = scriptLength;
        } else {
        result.startPos = wordBreak->preceding(nPos);
        result.endPos = wordBreak->following(nPos);
        }
    }
    if (result.startPos == BreakIterator::DONE)
        result.startPos = result.endPos;
    else if (result.endPos == BreakIterator::DONE)
        result.endPos = result.startPos;

    result.startPos += scriptStart;
    result.endPos += scriptStart;
    return result;
}


sal_Int32 SAL_CALL BreakIterator_Unicode::beginOfSentence( const OUString& Text, sal_Int32 nStartPos,
    const lang::Locale &rLocale ) throw(RuntimeException)
{
    if (!sentenceBreak) sentenceBreak = loadICUBreakIterator(rLocale, LOAD_SENTENCE_BREAKITERATOR);

    sentenceBreak->setText(UnicodeString(Text.getStr(), Text.getLength()));
    return sentenceBreak->preceding(nStartPos);
}

sal_Int32 SAL_CALL BreakIterator_Unicode::endOfSentence( const OUString& Text, sal_Int32 nStartPos,
    const lang::Locale &rLocale ) throw(RuntimeException)
{
    if (!sentenceBreak) sentenceBreak = loadICUBreakIterator(rLocale, LOAD_SENTENCE_BREAKITERATOR);

    sentenceBreak->setText(UnicodeString(Text.getStr(), Text.getLength()));

    sal_Int32 nPos = sentenceBreak->following(nStartPos);

        while (--nPos >= 0 && unicode::isWhiteSpace(Text[nPos]));

        return ++nPos;
}

LineBreakResults SAL_CALL BreakIterator_Unicode::getLineBreak(
    const OUString& Text, sal_Int32 nStartPos,
    const lang::Locale& rLocale, sal_Int32 nMinBreakPos,
    const LineBreakHyphenationOptions& hOptions,
    const LineBreakUserOptions& bOptions ) throw(RuntimeException)
{
    LineBreakResults result;

    if (!lineBreak) lineBreak = loadICUBreakIterator(rLocale, LOAD_LINE_BREAKITERATOR);

    if (bOptions.allowPunctuationOutsideMargin &&
        bOptions.forbiddenBeginCharacters.indexOf(Text[nStartPos]) != -1 &&
        ++nStartPos == Text.getLength()) {
        result.breakIndex = nStartPos;
        result.breakType = BreakType::WORDBOUNDARY;
        return result;
    }

    lineBreak->setText(UnicodeString(Text.getStr(), Text.getLength()));

    if (lineBreak->isBoundary(nStartPos)) { //Line boundary break
        result.breakIndex = nStartPos;
        result.breakType = BreakType::WORDBOUNDARY;
    } else if (hOptions.rHyphenator.is()) { //Hyphenation break
        Boundary wBoundary = getWordBoundary( Text, nStartPos, rLocale,
                        WordType::ANYWORD_IGNOREWHITESPACES, false);
        Reference< linguistic2::XHyphenatedWord > aHyphenatedWord;
        aHyphenatedWord = hOptions.rHyphenator->hyphenate(Text.copy(wBoundary.startPos,
        wBoundary.endPos - wBoundary.startPos), rLocale,
        hOptions.hyphenIndex - wBoundary.startPos, hOptions.aHyphenationOptions);
        if (aHyphenatedWord.is()) {
        result.rHyphenatedWord = aHyphenatedWord;
        if(wBoundary.startPos + aHyphenatedWord->getHyphenationPos() + 1 < nMinBreakPos )
            result.breakIndex = -1;
        else
            result.breakIndex = wBoundary.startPos; //aHyphenatedWord->getHyphenationPos();
        result.breakType = BreakType::HYPHENATION;
        } else {
        result.breakIndex = lineBreak->preceding(nStartPos);
        result.breakType = BreakType::WORDBOUNDARY;;
        }
    } else { //word boundary break
        result.breakIndex = lineBreak->preceding(nStartPos);
        result.breakType = BreakType::WORDBOUNDARY;
    }

    if (0 < result.breakIndex && result.breakIndex < Text.getLength() && bOptions.applyForbiddenRules) {
        while (result.breakIndex > 0 &&
            (bOptions.forbiddenBeginCharacters.indexOf(Text[result.breakIndex]) != -1 ||
            bOptions.forbiddenEndCharacters.indexOf(Text[result.breakIndex - 1]) != -1))
        result.breakIndex--;
    }
    return result;
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
