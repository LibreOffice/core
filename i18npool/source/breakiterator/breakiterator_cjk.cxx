/*************************************************************************
 *
 *  $RCSfile: breakiterator_cjk.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-30 14:38:05 $
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

#define BREAKITERATOR_ALL
#include <breakiterator_cjk.hxx>
#include <i18nutil/unicode.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::rtl;

namespace com { namespace sun { namespace star { namespace i18n {

//      ----------------------------------------------------
//      class BreakIterator_CJK
//      ----------------------------------------------------;

BreakIterator_CJK::BreakIterator_CJK() : dict(NULL)
{
        cBreakIterator = "com.sun.star.i18n.BreakIterator_CJK";
}

Boundary SAL_CALL
BreakIterator_CJK::previousWord(const OUString& text, sal_Int32 anyPos,
        const lang::Locale& nLocale, sal_Int16 wordType) throw(RuntimeException)
{
        if (dict) {
            result = dict->previousWord(text.getStr(), anyPos, text.getLength(), wordType);
            // #109813# for non-CJK, single character word, fallback to ICU breakiterator.
            if (result.endPos - result.startPos != 1 ||
                    getScriptType(text, result.startPos) == ScriptType::ASIAN)
                return result;
        }
        return BreakIterator_Unicode::previousWord(text, anyPos, nLocale, wordType);
}

Boundary SAL_CALL
BreakIterator_CJK::nextWord(const OUString& text, sal_Int32 anyPos,
        const lang::Locale& nLocale, sal_Int16 wordType) throw(RuntimeException)
{
        if (dict) {
            result = dict->nextWord(text.getStr(), anyPos, text.getLength(), wordType);
            // #109813# for non-CJK, single character word, fallback to ICU breakiterator.
            if (result.endPos - result.startPos != 1 ||
                    getScriptType(text, result.startPos) == ScriptType::ASIAN)
                return result;
        }
        return BreakIterator_Unicode::nextWord(text, anyPos, nLocale, wordType);
}

Boundary SAL_CALL
BreakIterator_CJK::getWordBoundary( const OUString& text, sal_Int32 anyPos,
        const lang::Locale& nLocale, sal_Int16 wordType, sal_Bool bDirection )
        throw(RuntimeException)
{
        if (dict) {
            result = dict->getWordBoundary(text.getStr(), anyPos, text.getLength(), wordType, bDirection);
            // #109813# for non-CJK, single character word, fallback to ICU breakiterator.
            if (result.endPos - result.startPos != 1 ||
                    getScriptType(text, result.startPos) == ScriptType::ASIAN)
                return result;
        }
        return BreakIterator_Unicode::getWordBoundary(text, anyPos, nLocale, wordType, bDirection);
}

LineBreakResults SAL_CALL BreakIterator_CJK::getLineBreak(
        const OUString& Text, sal_Int32 nStartPos,
        const lang::Locale& rLocale, sal_Int32 nMinBreakPos,
        const LineBreakHyphenationOptions& hOptions,
        const LineBreakUserOptions& bOptions ) throw(RuntimeException)
{
        LineBreakResults lbr;

        if (bOptions.allowPunctuationOutsideMargin &&
                bOptions.forbiddenBeginCharacters.indexOf(Text[nStartPos]) != -1 &&
                ++nStartPos == Text.getLength()) {
            ; // do nothing
        } else if (bOptions.applyForbiddenRules && 0 < nStartPos && nStartPos < Text.getLength()) {
            while (nStartPos > 0 &&
                    (bOptions.forbiddenBeginCharacters.indexOf(Text[nStartPos]) != -1 ||
                    bOptions.forbiddenEndCharacters.indexOf(Text[nStartPos-1]) != -1))
                nStartPos--;
        }

        lbr.breakIndex = nStartPos;
        lbr.breakType = BreakType::WORDBOUNDARY;
        return lbr;
}
//      ----------------------------------------------------
//      class BreakIterator_zh
//      ----------------------------------------------------;
BreakIterator_zh::BreakIterator_zh()
{
        dict = new xdictionary("zh");
        cBreakIterator = "com.sun.star.i18n.BreakIterator_zh";
}

BreakIterator_zh::~BreakIterator_zh()
{
        delete dict;
}

//      ----------------------------------------------------
//      class BreakIterator_ja
//      ----------------------------------------------------;
BreakIterator_ja::BreakIterator_ja()
{
        dict = new xdictionary("ja");
        dict->setJapaneseWordBreak();
        cBreakIterator = "com.sun.star.i18n.BreakIterator_ja";
}

BreakIterator_ja::~BreakIterator_ja()
{
        delete dict;
}

//      ----------------------------------------------------
//      class BreakIterator_ko
//      ----------------------------------------------------;
BreakIterator_ko::BreakIterator_ko()
{
        cBreakIterator = "com.sun.star.i18n.BreakIterator_ko";
}

BreakIterator_ko::~BreakIterator_ko()
{
}

} } } }
