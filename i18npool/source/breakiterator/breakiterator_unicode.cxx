/*************************************************************************
 *
 *  $RCSfile: breakiterator_unicode.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-07 15:13:54 $
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

#include <breakiterator_unicode.hxx>
#include <i18nutil/unicode.hxx>
#include <unicode/locid.h>
#include <unicode/rbbi.h>
#include <icu/cmemory.h>
#include <icu/rbbidata.h>
#include <icu/ucmndata.h>

U_CDECL_BEGIN
extern const char dict_word_brk[], edit_word_brk[], count_word_brk[], line_brk[], dict_word_ca_brk[];
U_CDECL_END

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::rtl;

namespace com { namespace sun { namespace star { namespace i18n {

#define ERROR RuntimeException()

#define LOAD_CHARACTER_BREAKITERATOR    0
#define LOAD_SENTENCE_BREAKITERATOR     1
#define LOAD_LINE_BREAKITERATOR         2

#define ImplementName "com.sun.star.i18n.BreakIterator_Unicode";

BreakIterator_Unicode::BreakIterator_Unicode()
{
        characterBreak = dictWordBreak = editWordBreak = countWordBreak = sentenceBreak = lineBreak = NULL;
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

static icu::BreakIterator* loadICURuleBasedBreakIterator(const char* pRule) throw(RuntimeException)
{
        /* TODO: Instead of malloc'ing data for each instance it could be
         * registered with ICU's cache using udata_setAppData(), and instances
         * be created with udata_open() and the UDataMemory be passed to the
         * appropriate RuleBasedBreakIterator ctor, which would also be the
         * cleaner way to do things. Unfortunately that ctor needed is
         * protected :-( what for, anyways? How's that supposed to work? Only
         * the RBBIRuleBuilder uses it). Find out why and how to change, if not
         * by patching ICU ...
         */

        // Allow the data to be optionally prepended with an alignment-forcing
        // double value, same as in UDataMemory_normalizeDataPointer()
        // implemented in icu/source/common/udatamem.c. However, that function
        // isn't exported.
        const DataHeader *pdh = (const DataHeader *)pRule;
        if( !(pdh->dataHeader.magic1==0xda && pdh->dataHeader.magic2==0x27))
            pdh = (const DataHeader *)((const double *)pdh+1);

        // Get the RBBIDataHeader behind the DataHeader, same as in the
        // RBBIDataWrapper ctor that takes an UDataMemory, implemented in
        // icu/source/common/rbbidata.cpp
        const RBBIDataHeader* pHeader = (const RBBIDataHeader*)((char *)&(pdh->info) + pdh->info.size);

        // Use RBBIDataHeader to get fLength from compiled rule data to malloc
        // RBBIDataHeader, which will be passed to contructor and released by
        // destructor of RuleBasedBreakIterator.
        RBBIDataHeader* pData = (RBBIDataHeader*) ((char*) uprv_malloc(pHeader->fLength));

        if ( pData == NULL ) throw ERROR;

        uprv_memcpy((char*) pData, (char*) pHeader, pHeader->fLength);

        UErrorCode status = U_ZERO_ERROR;
        icu::BreakIterator* breakiterator = new RuleBasedBreakIterator(pData, status);

        if ( !U_SUCCESS(status) ) throw ERROR;

        return breakiterator;
}


// loading ICU breakiterator on demand.
static icu::BreakIterator* SAL_CALL loadICUBreakIterator(const com::sun::star::lang::Locale& rLocale,
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
            case LOAD_SENTENCE_BREAKITERATOR:
                breakiterator = icu::BreakIterator::createSentenceInstance(icuLocale, status);
                break;
            case LOAD_LINE_BREAKITERATOR:
                breakiterator = loadICURuleBasedBreakIterator(line_brk);
                break;
        }
        if ( !U_SUCCESS(status) ) throw ERROR;

        return breakiterator;
}

icu::BreakIterator* SAL_CALL BreakIterator_Unicode::loadICUWordBreakIterator(const lang::Locale& rLocale,
        sal_Int16 rWordType) throw(RuntimeException)
{
        if (rWordType == WordType::WORD_COUNT) {
            if (!countWordBreak) countWordBreak = loadICURuleBasedBreakIterator(count_word_brk);
            return countWordBreak;
        }
        else if (rWordType == WordType::DICTIONARY_WORD) {
            if (!dictWordBreak) dictWordBreak = loadICURuleBasedBreakIterator(dict_word_brk);
            return dictWordBreak;
        } else {
            if (!editWordBreak) editWordBreak = loadICURuleBasedBreakIterator(edit_word_brk);
            return editWordBreak;
        }
}

icu::BreakIterator* SAL_CALL BreakIterator_ca::loadICUWordBreakIterator(const lang::Locale& rLocale,
        sal_Int16 rWordType) throw(RuntimeException)
{
        if (rWordType == WordType::DICTIONARY_WORD) {
            if (! dictWordBreak)
                dictWordBreak = loadICURuleBasedBreakIterator(dict_word_ca_brk);
            return dictWordBreak;
        } else
            return BreakIterator_Unicode::loadICUWordBreakIterator(rLocale, rWordType);
}

sal_Int32 SAL_CALL BreakIterator_Unicode::nextCharacters( const OUString& Text,
        sal_Int32 nStartPos, const lang::Locale &rLocale,
        sal_Int16 nCharacterIteratorMode, sal_Int32 nCount, sal_Int32& nDone )
        throw(RuntimeException)
{
        if (nCharacterIteratorMode == CharacterIteratorMode::SKIPCELL ) { // for CELL mode
            if (!characterBreak) characterBreak = loadICUBreakIterator(rLocale, LOAD_CHARACTER_BREAKITERATOR);

            characterBreak->setText(UnicodeString(Text.getStr(), Text.getLength()));
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
            if (!characterBreak) characterBreak = loadICUBreakIterator(rLocale, LOAD_CHARACTER_BREAKITERATOR);

            characterBreak->setText(UnicodeString(Text.getStr(), Text.getLength()));
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
        icu::BreakIterator* wordBreak = loadICUWordBreakIterator(rLocale, rWordType);
        wordBreak->setText(UnicodeString(Text.getStr(), Text.getLength()));

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
        icu::BreakIterator* wordBreak = loadICUWordBreakIterator(rLocale, rWordType);
        wordBreak->setText(UnicodeString(Text.getStr(), Text.getLength()));

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
        icu::BreakIterator* wordBreak = loadICUWordBreakIterator(rLocale, rWordType);
        sal_Int32 len = Text.getLength();
        wordBreak->setText(UnicodeString(Text.getStr(), len));

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
        LineBreakResults lbr;

        if (!lineBreak) lineBreak = loadICUBreakIterator(rLocale, LOAD_LINE_BREAKITERATOR);

        lineBreak->setText(UnicodeString(Text.getStr(), Text.getLength()));

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
