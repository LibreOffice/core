/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "textsearch.hxx"
#include "levdis.hxx"
#include <com/sun/star/i18n/BreakIterator.hpp>
#include <com/sun/star/util/SearchAlgorithms2.hpp>
#include <com/sun/star/util/SearchFlags.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#include <com/sun/star/i18n/CharacterClassification.hpp>
#include <com/sun/star/i18n/KCharacterType.hpp>
#include <com/sun/star/i18n/Transliteration.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>
#include <i18nutil/transliteration.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>

#include <unicode/regex.h>

using namespace ::com::sun::star::util;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star;

const TransliterationFlags COMPLEX_TRANS_MASK =
    TransliterationFlags::ignoreBaFa_ja_JP |
    TransliterationFlags::ignoreIterationMark_ja_JP |
    TransliterationFlags::ignoreTiJi_ja_JP |
    TransliterationFlags::ignoreHyuByu_ja_JP |
    TransliterationFlags::ignoreSeZe_ja_JP |
    TransliterationFlags::ignoreIandEfollowedByYa_ja_JP |
    TransliterationFlags::ignoreKiKuFollowedBySa_ja_JP |
    TransliterationFlags::ignoreProlongedSoundMark_ja_JP;

namespace
{
TransliterationFlags maskComplexTrans( TransliterationFlags n )
{
    // IGNORE_KANA and FULLWIDTH_HALFWIDTH are simple but need to take effect
    // in complex transliteration.
    return
        n & (COMPLEX_TRANS_MASK |                       // all set ignore bits
        TransliterationFlags::IGNORE_KANA |            // plus IGNORE_KANA bit
        TransliterationFlags::FULLWIDTH_HALFWIDTH);    // and the FULLWIDTH_HALFWIDTH value
}

bool isComplexTrans( TransliterationFlags n )
{
    return bool(n & COMPLEX_TRANS_MASK);
}

TransliterationFlags maskSimpleTrans( TransliterationFlags n )
{
    return n & ~COMPLEX_TRANS_MASK;
}

bool isSimpleTrans( TransliterationFlags n )
{
    return bool(maskSimpleTrans(n));
}

// Regex patterns are case sensitive.
TransliterationFlags maskSimpleRegexTrans( TransliterationFlags n )
{
    TransliterationFlags m = (n & TransliterationFlags::IGNORE_MASK) & ~TransliterationFlags::IGNORE_CASE;
    TransliterationFlags v = n & TransliterationFlags::NON_IGNORE_MASK;
    if (v == TransliterationFlags::UPPERCASE_LOWERCASE || v == TransliterationFlags::LOWERCASE_UPPERCASE)
        v = TransliterationFlags::NONE;
    return (m | v) & ~COMPLEX_TRANS_MASK;
}

bool isSimpleRegexTrans( TransliterationFlags n )
{
    return bool(maskSimpleRegexTrans(n));
}
};

TextSearch::TextSearch(const Reference < XComponentContext > & rxContext)
        : m_xContext( rxContext )
{
    SearchOptions2 aOpt;
    aOpt.AlgorithmType2 = SearchAlgorithms2::ABSOLUTE;
    aOpt.algorithmType = SearchAlgorithms_ABSOLUTE;
    aOpt.searchFlag = SearchFlags::ALL_IGNORE_CASE;
    //aOpt.Locale = ???;
    setOptions( aOpt );
}

TextSearch::~TextSearch()
{
    pRegexMatcher.reset();
    pWLD.reset();
    pJumpTable.reset();
    pJumpTable2.reset();
}

void TextSearch::setOptions2( const SearchOptions2& rOptions )
{
    std::unique_lock g(m_aMutex);

    aSrchPara = rOptions;

    pRegexMatcher.reset();
    pWLD.reset();
    pJumpTable.reset();
    pJumpTable2.reset();
    maWildcardReversePattern.clear();
    maWildcardReversePattern2.clear();
    TransliterationFlags transliterateFlags = static_cast<TransliterationFlags>(aSrchPara.transliterateFlags);
    bSearchApostrophe = false;
    bool bReplaceApostrophe = false;
    if (aSrchPara.AlgorithmType2 == SearchAlgorithms2::REGEXP)
    {
        // RESrchPrepare will consider aSrchPara.transliterateFlags when
        // picking the actual regex pattern
        // (sSrchStr|sSrchStr2|SearchOptions2::searchString) and setting
        // case-insensitivity. Create transliteration instance, if any, without
        // ignore-case so later in TextSearch::searchForward() the string to
        // match is not case-altered, leave case-(in)sensitive to regex engine.
        transliterateFlags &= ~TransliterationFlags::IGNORE_CASE;
    }
    else if ( aSrchPara.searchString.indexOf('\'') > - 1 )
    {
        bSearchApostrophe = true;
        bReplaceApostrophe = aSrchPara.searchString.indexOf(u'\u2019') > -1;
    }

    // Create Transliteration class
    if( isSimpleTrans( transliterateFlags) )
    {
        if( !xTranslit.is() )
            xTranslit.set( Transliteration::create( m_xContext ) );
        xTranslit->loadModule(
             static_cast<TransliterationModules>(maskSimpleTrans(transliterateFlags)),
             aSrchPara.Locale);
    }
    else if( xTranslit.is() )
        xTranslit = nullptr;

    // Create Transliteration for 2<->1, 2<->2 transliteration
    if ( isComplexTrans( transliterateFlags) )
    {
        if( !xTranslit2.is() )
            xTranslit2.set( Transliteration::create( m_xContext ) );
        // Load transliteration module
        xTranslit2->loadModule(
             static_cast<TransliterationModules>(maskComplexTrans(transliterateFlags)),
             aSrchPara.Locale);
    }

    if ( !xBreak.is() )
        xBreak = css::i18n::BreakIterator::create( m_xContext );

    sSrchStr = aSrchPara.searchString;

    // Transliterate search string.
    if (aSrchPara.AlgorithmType2 == SearchAlgorithms2::REGEXP)
    {
        if (isSimpleRegexTrans(transliterateFlags))
        {
            if (maskSimpleRegexTrans(transliterateFlags) !=
                    maskSimpleTrans(transliterateFlags))
            {
                css::uno::Reference< XExtendedTransliteration > xTranslitPattern(
                         Transliteration::create( m_xContext ));
                if (xTranslitPattern.is())
                {
                    xTranslitPattern->loadModule(
                            static_cast<TransliterationModules>(maskSimpleRegexTrans(transliterateFlags)),
                            aSrchPara.Locale);
                    sSrchStr = xTranslitPattern->transliterateString2String(
                            aSrchPara.searchString, 0, aSrchPara.searchString.getLength());
                }
            }
            else
            {
                if (xTranslit.is())
                    sSrchStr = xTranslit->transliterateString2String(
                            aSrchPara.searchString, 0, aSrchPara.searchString.getLength());
            }
            // xTranslit2 complex transliterated sSrchStr2 is not used in
            // regex, see TextSearch::searchForward() and
            // TextSearch::searchBackward()
        }
    }
    else
    {
        if ( xTranslit.is() && isSimpleTrans(transliterateFlags) )
            sSrchStr = xTranslit->transliterateString2String(
                    aSrchPara.searchString, 0, aSrchPara.searchString.getLength());

        if ( xTranslit2.is() && isComplexTrans(transliterateFlags) )
            sSrchStr2 = xTranslit2->transliterateString2String(
                    aSrchPara.searchString, 0, aSrchPara.searchString.getLength());
    }

    if ( bReplaceApostrophe )
        sSrchStr = sSrchStr.replace(u'\u2019', '\'');

    // Take the new SearchOptions2::AlgorithmType2 field and ignore
    // SearchOptions::algorithmType
    switch( aSrchPara.AlgorithmType2)
    {
        case SearchAlgorithms2::REGEXP:
            fnForward = &TextSearch::RESrchFrwrd;
            fnBackward = &TextSearch::RESrchBkwrd;
            RESrchPrepare( aSrchPara);
            break;

        case SearchAlgorithms2::APPROXIMATE:
            fnForward = &TextSearch::ApproxSrchFrwrd;
            fnBackward = &TextSearch::ApproxSrchBkwrd;

            pWLD.reset( new WLevDistance( sSrchStr.getStr(), aSrchPara.changedChars,
                    aSrchPara.insertedChars, aSrchPara.deletedChars,
                    0 != (SearchFlags::LEV_RELAXED & aSrchPara.searchFlag ) ) );

            nLimit = pWLD->GetLimit();
            break;

        case SearchAlgorithms2::WILDCARD:
            mcWildcardEscapeChar = static_cast<sal_uInt32>(aSrchPara.WildcardEscapeCharacter);
            mbWildcardAllowSubstring = ((aSrchPara.searchFlag & SearchFlags::WILD_MATCH_SELECTION) == 0);
            fnForward = &TextSearch::WildcardSrchFrwrd;
            fnBackward = &TextSearch::WildcardSrchBkwrd;
            break;

        default:
            SAL_WARN("i18npool","TextSearch::setOptions2 - default what?");
            [[fallthrough]];
        case SearchAlgorithms2::ABSOLUTE:
            fnForward = &TextSearch::NSrchFrwrd;
            fnBackward = &TextSearch::NSrchBkwrd;
            break;
    }
}

void TextSearch::setOptions( const SearchOptions& rOptions )
{
    sal_Int16 nAlgorithmType2;
    switch (rOptions.algorithmType)
    {
        case SearchAlgorithms_REGEXP:
            nAlgorithmType2 = SearchAlgorithms2::REGEXP;
            break;
        case SearchAlgorithms_APPROXIMATE:
            nAlgorithmType2 = SearchAlgorithms2::APPROXIMATE;
            break;
        default:
            SAL_WARN("i18npool","TextSearch::setOptions - default what?");
            [[fallthrough]];
        case SearchAlgorithms_ABSOLUTE:
            nAlgorithmType2 = SearchAlgorithms2::ABSOLUTE;
            break;
    }
    // It would be nice if an inherited struct had a ctor that takes an
    // instance of the object the struct derived from...
    SearchOptions2 aOptions2(
            rOptions.algorithmType,
            rOptions.searchFlag,
            rOptions.searchString,
            rOptions.replaceString,
            rOptions.Locale,
            rOptions.changedChars,
            rOptions.deletedChars,
            rOptions.insertedChars,
            rOptions.transliterateFlags,
            nAlgorithmType2,
            0   // no wildcard search, no escape character...
            );
    setOptions2( aOptions2);
}

static sal_Int32 FindPosInSeq_Impl( const Sequence <sal_Int32>& rOff, sal_Int32 nPos )
{
    auto pOff = std::find_if(rOff.begin(), rOff.end(),
        [nPos](const sal_Int32 nOff) { return nOff >= nPos; });
    return static_cast<sal_Int32>(std::distance(rOff.begin(), pOff));
}

SearchResult TextSearch::searchForward( const OUString& searchStr, sal_Int32 startPos, sal_Int32 endPos )
{
    std::unique_lock g(m_aMutex);

    SearchResult sres;

    OUString in_str(searchStr);

    // in non-regex mode, allow searching typographical apostrophe with the ASCII one
    // to avoid regression after using automatic conversion to U+2019 during typing in Writer
    bool bReplaceApostrophe = bSearchApostrophe && in_str.indexOf(u'\u2019') > -1;

    bUsePrimarySrchStr = true;

    if ( xTranslit.is() )
    {
        // apply normal transliteration (1<->1, 1<->0)

        sal_Int32 nInStartPos = startPos;
        if (pRegexMatcher && startPos > 0)
        {
            // tdf#89665, tdf#75806: An optimization to avoid transliterating the whole string, yet
            // transliterate enough of the leading text to allow sensible look-behind assertions.
            // 100 is chosen arbitrarily in the hope that look-behind assertions would largely fit.
            // See http://userguide.icu-project.org/strings/regexp for look-behind assertion syntax.
            // When search regex doesn't start with an assertion, 3 is to allow startPos to be in
            // the middle of a surrogate pair, preceded by another surrogate pair.
            const sal_Int32 nMaxLeadingLen = aSrchPara.searchString.startsWith("(?") ? 100 : 3;
            nInStartPos -= std::min(nMaxLeadingLen, startPos);
        }
        sal_Int32 nInEndPos = endPos;
        if (pRegexMatcher && endPos < searchStr.getLength())
        {
            // tdf#65038: ditto for look-ahead assertions
            const sal_Int32 nMaxTrailingLen = aSrchPara.searchString.endsWith(")") ? 100 : 3;
            nInEndPos += std::min(nMaxTrailingLen, searchStr.getLength() - endPos);
        }

        css::uno::Sequence<sal_Int32> offset(nInEndPos - nInStartPos);
        in_str = xTranslit->transliterate(searchStr, nInStartPos, nInEndPos - nInStartPos, offset);

        if ( bReplaceApostrophe )
            in_str = in_str.replace(u'\u2019', '\'');

        // JP 20.6.2001: also the start and end positions must be corrected!
        sal_Int32 newStartPos =
            (startPos == 0) ? 0 : FindPosInSeq_Impl( offset, startPos );

        sal_Int32 newEndPos = (endPos < searchStr.getLength())
            ? FindPosInSeq_Impl( offset, endPos )
            : in_str.getLength();

        sres = (this->*fnForward)( in_str, newStartPos, newEndPos );

        // Map offsets back to untransliterated string.
        const sal_Int32 nOffsets = offset.getLength();
        if (nOffsets)
        {
            auto sres_startOffsetRange = asNonConstRange(sres.startOffset);
            auto sres_endOffsetRange = asNonConstRange(sres.endOffset);
            // For regex nGroups is the number of groups+1 with group 0 being
            // the entire match.
            const sal_Int32 nGroups = sres.startOffset.getLength();
            for ( sal_Int32 k = 0; k < nGroups; k++ )
            {
                const sal_Int32 nStart = sres.startOffset[k];
                // Result offsets are negative (-1) if a group expression was
                // not matched.
                if (nStart >= 0)
                    sres_startOffsetRange[k] = (nStart < nOffsets ? offset[nStart] : (offset[nOffsets - 1] + 1));
                // JP 20.6.2001: end is ever exclusive and then don't return
                //               the position of the next character - return the
                //               next position behind the last found character!
                //               "a b c" find "b" must return 2,3 and not 2,4!!!
                const sal_Int32 nStop = sres.endOffset[k];
                if (nStop >= 0)
                {
                    if (nStop > 0)
                        sres_endOffsetRange[k] = offset[(nStop <= nOffsets ? nStop : nOffsets) - 1] + 1;
                    else
                        sres_endOffsetRange[k] = offset[0];
                }
            }
        }
    }
    else
    {
        if ( bReplaceApostrophe )
            in_str = in_str.replace(u'\u2019', '\'');

        sres = (this->*fnForward)( in_str, startPos, endPos );
    }

    if ( xTranslit2.is() && aSrchPara.AlgorithmType2 != SearchAlgorithms2::REGEXP)
    {
        SearchResult sres2;

        in_str = searchStr;
        css::uno::Sequence <sal_Int32> offset( in_str.getLength());

        in_str = xTranslit2->transliterate( searchStr, 0, in_str.getLength(), offset );

        if( startPos )
            startPos = FindPosInSeq_Impl( offset, startPos );

        if( endPos < searchStr.getLength() )
            endPos = FindPosInSeq_Impl( offset, endPos );
        else
            endPos = in_str.getLength();

        bUsePrimarySrchStr = false;
        sres2 = (this->*fnForward)( in_str, startPos, endPos );
        auto sres2_startOffsetRange = asNonConstRange(sres2.startOffset);
        auto sres2_endOffsetRange = asNonConstRange(sres2.endOffset);

        for ( int k = 0; k < sres2.startOffset.getLength(); k++ )
        {
            if (sres2.startOffset[k])
                sres2_startOffsetRange[k] = offset[sres2.startOffset[k]-1] + 1;
            if (sres2.endOffset[k])
                sres2_endOffsetRange[k] = offset[sres2.endOffset[k]-1] + 1;
        }

        // pick first and long one
        if ( sres.subRegExpressions == 0)
            return sres2;
        if ( sres2.subRegExpressions == 1)
        {
            if ( sres.startOffset[0] > sres2.startOffset[0])
                return sres2;
            else if ( sres.startOffset[0] == sres2.startOffset[0] &&
                    sres.endOffset[0] < sres2.endOffset[0])
                return sres2;
        }
    }

    return sres;
}

SearchResult TextSearch::searchBackward( const OUString& searchStr, sal_Int32 startPos, sal_Int32 endPos )
{
    std::unique_lock g(m_aMutex);

    SearchResult sres;

    OUString in_str(searchStr);

    // in non-regex mode, allow searching typographical apostrophe with the ASCII one
    // to avoid regression after using automatic conversion to U+2019 during typing in Writer
    bool bReplaceApostrophe = bSearchApostrophe && in_str.indexOf(u'\u2019') > -1;

    bUsePrimarySrchStr = true;

    if ( xTranslit.is() )
    {
        // apply only simple 1<->1 transliteration here
        css::uno::Sequence<sal_Int32> offset(startPos - endPos);
        in_str = xTranslit->transliterate( searchStr, endPos, startPos - endPos, offset );

        if ( bReplaceApostrophe )
            in_str = in_str.replace(u'\u2019', '\'');

        // JP 20.6.2001: also the start and end positions must be corrected!
        sal_Int32 const newStartPos = (startPos < searchStr.getLength())
            ? FindPosInSeq_Impl( offset, startPos )
            : in_str.getLength();

        sal_Int32 const newEndPos =
            (endPos == 0) ? 0 : FindPosInSeq_Impl( offset, endPos );

        // TODO: this would need nExtraOffset handling to avoid $ matching
        // if (pRegexMatcher && startPos < searchStr.getLength())
        // but that appears to be impossible with ICU regex

        sres = (this->*fnBackward)( in_str, newStartPos, newEndPos );

        // Map offsets back to untransliterated string.
        const sal_Int32 nOffsets = offset.getLength();
        if (nOffsets)
        {
            auto sres_startOffsetRange = asNonConstRange(sres.startOffset);
            auto sres_endOffsetRange = asNonConstRange(sres.endOffset);
            // For regex nGroups is the number of groups+1 with group 0 being
            // the entire match.
            const sal_Int32 nGroups = sres.startOffset.getLength();
            for ( sal_Int32 k = 0; k < nGroups; k++ )
            {
                const sal_Int32 nStart = sres.startOffset[k];
                // Result offsets are negative (-1) if a group expression was
                // not matched.
                if (nStart >= 0)
                {
                    if (nStart > 0)
                        sres_startOffsetRange[k] = offset[(nStart <= nOffsets ? nStart : nOffsets) - 1] + 1;
                    else
                        sres_startOffsetRange[k] = offset[0];
                }
                // JP 20.6.2001: end is ever exclusive and then don't return
                //               the position of the next character - return the
                //               next position behind the last found character!
                //               "a b c" find "b" must return 2,3 and not 2,4!!!
                const sal_Int32 nStop = sres.endOffset[k];
                if (nStop >= 0)
                    sres_endOffsetRange[k] = (nStop < nOffsets ? offset[nStop] : (offset[nOffsets - 1] + 1));
            }
        }
    }
    else
    {
        if ( bReplaceApostrophe )
            in_str = in_str.replace(u'\u2019', '\'');

        sres = (this->*fnBackward)( in_str, startPos, endPos );
    }

    if ( xTranslit2.is() && aSrchPara.AlgorithmType2 != SearchAlgorithms2::REGEXP )
    {
        SearchResult sres2;

        in_str = searchStr;
        css::uno::Sequence <sal_Int32> offset( in_str.getLength());

        in_str = xTranslit2->transliterate(searchStr, 0, in_str.getLength(), offset);

        if( startPos < searchStr.getLength() )
            startPos = FindPosInSeq_Impl( offset, startPos );
        else
            startPos = in_str.getLength();

        if( endPos )
            endPos = FindPosInSeq_Impl( offset, endPos );

        bUsePrimarySrchStr = false;
        sres2 = (this->*fnBackward)( in_str, startPos, endPos );
        auto sres2_startOffsetRange = asNonConstRange(sres2.startOffset);
        auto sres2_endOffsetRange = asNonConstRange(sres2.endOffset);

        for( int k = 0; k < sres2.startOffset.getLength(); k++ )
        {
            if (sres2.startOffset[k])
                sres2_startOffsetRange[k] = offset[sres2.startOffset[k]-1]+1;
            if (sres2.endOffset[k])
                sres2_endOffsetRange[k] = offset[sres2.endOffset[k]-1]+1;
        }

        // pick last and long one
        if ( sres.subRegExpressions == 0 )
            return sres2;
        if ( sres2.subRegExpressions == 1 )
        {
            if ( sres.startOffset[0] < sres2.startOffset[0] )
                return sres2;
            if ( sres.startOffset[0] == sres2.startOffset[0] &&
                    sres.endOffset[0] > sres2.endOffset[0] )
                return sres2;
        }
    }

    return sres;
}


bool TextSearch::IsDelimiter( const OUString& rStr, sal_Int32 nPos ) const
{
    bool bRet = true;
    if( '\x7f' != rStr[nPos])
    {
        if ( !xCharClass.is() )
             xCharClass = CharacterClassification::create( m_xContext );
        sal_Int32 nCType = xCharClass->getCharacterType( rStr, nPos,
                aSrchPara.Locale );
        if( 0 != (( KCharacterType::DIGIT | KCharacterType::ALPHA |
                        KCharacterType::LETTER ) & nCType ) )
            bRet = false;
    }
    return bRet;
}

// --------- helper methods for Boyer-Moore like text searching ----------
// TODO: use ICU's regex UREGEX_LITERAL mode instead when it becomes available

void TextSearch::MakeForwardTab()
{
    // create the jumptable for the search text

    if( pJumpTable && bIsForwardTab )
    {
        return; // the jumpTable is ok
    }
    bIsForwardTab = true;

    sal_Int32 n, nLen = sSrchStr.getLength();
    pJumpTable.reset( new TextSearchJumpTable );

    for( n = 0; n < nLen - 1; ++n )
    {
        sal_Unicode cCh = sSrchStr[n];
        sal_Int32 nDiff = nLen - n - 1;
        TextSearchJumpTable::value_type aEntry( cCh, nDiff );

        ::std::pair< TextSearchJumpTable::iterator, bool > aPair =
            pJumpTable->insert( aEntry );
        if ( !aPair.second )
            (*(aPair.first)).second = nDiff;
    }
}

void TextSearch::MakeForwardTab2()
{
    // create the jumptable for the search text
    if( pJumpTable2 && bIsForwardTab )
    {
        return;        // the jumpTable is ok
    }
    bIsForwardTab = true;

    sal_Int32 n, nLen = sSrchStr2.getLength();
    pJumpTable2.reset( new TextSearchJumpTable );

    for( n = 0; n < nLen - 1; ++n )
    {
        sal_Unicode cCh = sSrchStr2[n];
        sal_Int32 nDiff = nLen - n - 1;

        TextSearchJumpTable::value_type aEntry( cCh, nDiff );
        ::std::pair< TextSearchJumpTable::iterator, bool > aPair =
            pJumpTable2->insert( aEntry );
        if ( !aPair.second )
            (*(aPair.first)).second = nDiff;
    }
}

void TextSearch::MakeBackwardTab()
{
    // create the jumptable for the search text
    if( pJumpTable && !bIsForwardTab)
    {
        return;   // the jumpTable is ok
    }
    bIsForwardTab = false;

    sal_Int32 n, nLen = sSrchStr.getLength();
    pJumpTable.reset( new TextSearchJumpTable );

    for( n = nLen-1; n > 0; --n )
    {
        sal_Unicode cCh = sSrchStr[n];
        TextSearchJumpTable::value_type aEntry( cCh, n );
        ::std::pair< TextSearchJumpTable::iterator, bool > aPair =
            pJumpTable->insert( aEntry );
        if ( !aPair.second )
            (*(aPair.first)).second = n;
    }
}

void TextSearch::MakeBackwardTab2()
{
    // create the jumptable for the search text
    if( pJumpTable2 && !bIsForwardTab )
    {
        return;    // the jumpTable is ok
    }
    bIsForwardTab = false;

    sal_Int32 n, nLen = sSrchStr2.getLength();
    pJumpTable2.reset( new TextSearchJumpTable );

    for( n = nLen-1; n > 0; --n )
    {
        sal_Unicode cCh = sSrchStr2[n];
        TextSearchJumpTable::value_type aEntry( cCh, n );
        ::std::pair< TextSearchJumpTable::iterator, bool > aPair =
            pJumpTable2->insert( aEntry );
        if ( !aPair.second )
            (*(aPair.first)).second = n;
    }
}

sal_Int32 TextSearch::GetDiff( const sal_Unicode cChr ) const
{
    TextSearchJumpTable *pJump;
    OUString sSearchKey;

    if ( bUsePrimarySrchStr ) {
        pJump = pJumpTable.get();
        sSearchKey = sSrchStr;
    } else {
        pJump = pJumpTable2.get();
        sSearchKey = sSrchStr2;
    }

    TextSearchJumpTable::const_iterator iLook = pJump->find( cChr );
    if ( iLook == pJump->end() )
        return sSearchKey.getLength();
    return (*iLook).second;
}


SearchResult TextSearch::NSrchFrwrd( const OUString& searchStr, sal_Int32 startPos, sal_Int32 endPos )
{
    SearchResult aRet;
    aRet.subRegExpressions = 0;

    OUString sSearchKey = bUsePrimarySrchStr ? sSrchStr : sSrchStr2;

    sal_Int32 nSuchIdx = searchStr.getLength();
    sal_Int32 nEnd = endPos;
    if( !nSuchIdx || !sSearchKey.getLength() || sSearchKey.getLength() > nSuchIdx )
        return aRet;


    if( nEnd < sSearchKey.getLength() )   // position inside the search region ?
        return aRet;

    nEnd -= sSearchKey.getLength();

    if (bUsePrimarySrchStr)
      MakeForwardTab();                   // create the jumptable
    else
      MakeForwardTab2();

    for (sal_Int32 nCmpIdx = startPos; // start position for the search
            nCmpIdx <= nEnd;
            nCmpIdx += GetDiff( searchStr[nCmpIdx + sSearchKey.getLength()-1]))
    {
        nSuchIdx = sSearchKey.getLength() - 1;
        while( nSuchIdx >= 0 && sSearchKey[nSuchIdx] == searchStr[nCmpIdx + nSuchIdx])
        {
            if( nSuchIdx == 0 )
            {
                if( SearchFlags::NORM_WORD_ONLY & aSrchPara.searchFlag )
                {
                    sal_Int32 nFndEnd = nCmpIdx + sSearchKey.getLength();
                    bool bAtStart = !nCmpIdx;
                    bool bAtEnd = nFndEnd == endPos;
                    bool bDelimBefore = bAtStart || IsDelimiter( searchStr, nCmpIdx-1 );
                    bool bDelimBehind = bAtEnd || IsDelimiter(  searchStr, nFndEnd );
                    //  *       1 -> only one word in the paragraph
                    //  *       2 -> at begin of paragraph
                    //  *       3 -> at end of paragraph
                    //  *       4 -> inside the paragraph
                    if( !(  ( bAtStart && bAtEnd ) ||           // 1
                                ( bAtStart && bDelimBehind ) ||     // 2
                                ( bAtEnd && bDelimBefore ) ||       // 3
                                ( bDelimBefore && bDelimBehind )))  // 4
                        break;
                }

                aRet.subRegExpressions = 1;
                aRet.startOffset = { nCmpIdx };
                aRet.endOffset = { nCmpIdx + sSearchKey.getLength() };

                return aRet;
            }
            else
                nSuchIdx--;
        }
    }
    return aRet;
}

SearchResult TextSearch::NSrchBkwrd( const OUString& searchStr, sal_Int32 startPos, sal_Int32 endPos )
{
    SearchResult aRet;
    aRet.subRegExpressions = 0;

    OUString sSearchKey = bUsePrimarySrchStr ? sSrchStr : sSrchStr2;

    sal_Int32 nSuchIdx = searchStr.getLength();
    sal_Int32 nEnd = endPos;
    if( nSuchIdx == 0 || sSearchKey.isEmpty() || sSearchKey.getLength() > nSuchIdx)
        return aRet;

    if (bUsePrimarySrchStr)
        MakeBackwardTab();                  // create the jumptable
    else
        MakeBackwardTab2();

    if( nEnd == nSuchIdx )                  // end position for the search
        nEnd = sSearchKey.getLength();
    else
        nEnd += sSearchKey.getLength();

    sal_Int32 nCmpIdx = startPos;          // start position for the search

    while (nCmpIdx >= nEnd)
    {
        nSuchIdx = 0;
        while( nSuchIdx < sSearchKey.getLength() && sSearchKey[nSuchIdx] ==
                searchStr[nCmpIdx + nSuchIdx - sSearchKey.getLength()] )
            nSuchIdx++;
        if( nSuchIdx >= sSearchKey.getLength() )
        {
            if( SearchFlags::NORM_WORD_ONLY & aSrchPara.searchFlag )
            {
                sal_Int32 nFndStt = nCmpIdx - sSearchKey.getLength();
                bool bAtStart = !nFndStt;
                bool bAtEnd = nCmpIdx == startPos;
                bool bDelimBehind = bAtEnd || IsDelimiter( searchStr, nCmpIdx );
                bool bDelimBefore = bAtStart || // begin of paragraph
                    IsDelimiter( searchStr, nFndStt-1 );
                //  *       1 -> only one word in the paragraph
                //  *       2 -> at begin of paragraph
                //  *       3 -> at end of paragraph
                //  *       4 -> inside the paragraph
                if( ( bAtStart && bAtEnd ) ||           // 1
                        ( bAtStart && bDelimBehind ) ||     // 2
                        ( bAtEnd && bDelimBefore ) ||       // 3
                        ( bDelimBefore && bDelimBehind ))   // 4
                {
                    aRet.subRegExpressions = 1;
                    aRet.startOffset = { nCmpIdx };
                    aRet.endOffset = { nCmpIdx - sSearchKey.getLength() };
                    return aRet;
                }
            }
            else
            {
                aRet.subRegExpressions = 1;
                aRet.startOffset = { nCmpIdx };
                aRet.endOffset = { nCmpIdx - sSearchKey.getLength() };
                return aRet;
            }
        }
        nSuchIdx = GetDiff( searchStr[nCmpIdx - sSearchKey.getLength()] );
        if( nCmpIdx < nSuchIdx )
            return aRet;
        nCmpIdx -= nSuchIdx;
    }
    return aRet;
}

void TextSearch::RESrchPrepare( const css::util::SearchOptions2& rOptions)
{
    TransliterationFlags transliterateFlags = static_cast<TransliterationFlags>(rOptions.transliterateFlags);
    // select the transliterated pattern string
    const OUString& rPatternStr =
        (isSimpleTrans(transliterateFlags) ? sSrchStr
        : (isComplexTrans(transliterateFlags) ? sSrchStr2 : rOptions.searchString));

    sal_uInt32 nIcuSearchFlags = UREGEX_UWORD; // request UAX#29 unicode capability
    // map css::util::SearchFlags to ICU uregex.h flags
    // TODO: REG_EXTENDED, REG_NOT_BEGINOFLINE, REG_NOT_ENDOFLINE
    // REG_NEWLINE is neither properly defined nor used anywhere => not implemented
    // REG_NOSUB is not used anywhere => not implemented
    // NORM_WORD_ONLY is only used for SearchAlgorithm==Absolute
    // LEV_RELAXED is only used for SearchAlgorithm==Approximate
    // Note that the search flag ALL_IGNORE_CASE is deprecated in UNO
    // probably because the transliteration flag IGNORE_CASE handles it as well.
    if( (rOptions.searchFlag & css::util::SearchFlags::ALL_IGNORE_CASE) != 0
    ||  (transliterateFlags & TransliterationFlags::IGNORE_CASE))
        nIcuSearchFlags |= UREGEX_CASE_INSENSITIVE;
    UErrorCode nIcuErr = U_ZERO_ERROR;
    // assumption: transliteration didn't mangle regexp control chars
    icu::UnicodeString aIcuSearchPatStr( reinterpret_cast<const UChar*>(rPatternStr.getStr()), rPatternStr.getLength());
#ifndef DISABLE_WORDBOUND_EMULATION
    // for convenience specific syntax elements of the old regex engine are emulated
    // - by replacing \< with "word-break followed by a look-ahead word-char"
    static const icu::UnicodeString aChevronPatternB( "\\\\<", -1, icu::UnicodeString::kInvariant);
    static const icu::UnicodeString aChevronReplaceB( "\\\\b(?=\\\\w)", -1, icu::UnicodeString::kInvariant);
    static icu::RegexMatcher aChevronMatcherB( aChevronPatternB, 0, nIcuErr);
    aChevronMatcherB.reset( aIcuSearchPatStr);
    aIcuSearchPatStr = aChevronMatcherB.replaceAll( aChevronReplaceB, nIcuErr);
    aChevronMatcherB.reset();
    // - by replacing \> with "look-behind word-char followed by a word-break"
    static const icu::UnicodeString aChevronPatternE( "\\\\>", -1, icu::UnicodeString::kInvariant);
    static const icu::UnicodeString aChevronReplaceE( "(?<=\\\\w)\\\\b", -1, icu::UnicodeString::kInvariant);
    static icu::RegexMatcher aChevronMatcherE( aChevronPatternE, 0, nIcuErr);
    aChevronMatcherE.reset( aIcuSearchPatStr);
    aIcuSearchPatStr = aChevronMatcherE.replaceAll( aChevronReplaceE, nIcuErr);
    aChevronMatcherE.reset();
#endif
    pRegexMatcher.reset( new icu::RegexMatcher( aIcuSearchPatStr, nIcuSearchFlags, nIcuErr) );
    if (nIcuErr)
    {
        SAL_INFO( "i18npool", "TextSearch::RESrchPrepare UErrorCode " << nIcuErr);
        pRegexMatcher.reset();
    }
    else
    {
        // Pathological patterns may result in exponential run time making the
        // application appear to be frozen. Limit that. Documentation for this
        // call says
        // https://unicode-org.github.io/icu-docs/apidoc/released/icu4c/classicu_1_1RegexMatcher.html#a6ebcfcab4fe6a38678c0291643a03a00
        // "The units of the limit are steps of the match engine.
        // Correspondence with actual processor time will depend on the speed
        // of the processor and the details of the specific pattern, but will
        // typically be on the order of milliseconds."
        // Just what is a good value? 42 is always an answer ... the 23 enigma
        // as well... which on the dev's machine is roughly 50 seconds with the
        // pattern of fdo#70627.
        /* TODO: make this a configuration settable value and possibly take
         * complexity of expression into account and maybe even length of text
         * to be matched; currently (2013-11-25) that is at most one 64k
         * paragraph per RESrchFrwrd()/RESrchBkwrd() call. */
        pRegexMatcher->setTimeLimit( 23*1000, nIcuErr);
    }
}


static bool lcl_findRegex(std::unique_ptr<icu::RegexMatcher> const& pRegexMatcher,
                          sal_Int32 nStartPos, sal_Int32 nEndPos, UErrorCode& rIcuErr)
{
    pRegexMatcher->region(nStartPos, nEndPos, rIcuErr);
    pRegexMatcher->useAnchoringBounds(false); // use whole text's anchoring bounds, not region's
    pRegexMatcher->useTransparentBounds(true); // take text outside of the region into account for
                                               // look-ahead/behind assertions

    if (!pRegexMatcher->find(rIcuErr))
    {
        /* TODO: future versions could pass the UErrorCode or translations
         * thereof to the caller, for example to inform the user of
         * U_REGEX_TIME_OUT. The strange thing though is that an error is set
         * only after the second call that returns immediately and not if
         * timeout occurred on the first call?!? */
        SAL_INFO( "i18npool", "lcl_findRegex UErrorCode " << rIcuErr);
        return false;
    }
    return true;
}

SearchResult TextSearch::RESrchFrwrd( const OUString& searchStr,
                                      sal_Int32 startPos, sal_Int32 endPos )
{
    SearchResult aRet;
    aRet.subRegExpressions = 0;
    if( !pRegexMatcher)
        return aRet;

    if( endPos > searchStr.getLength())
        endPos = searchStr.getLength();

    // use the ICU RegexMatcher to find the matches
    UErrorCode nIcuErr = U_ZERO_ERROR;
    const icu::UnicodeString aSearchTargetStr(false, reinterpret_cast<const UChar*>(searchStr.getStr()),
                                        searchStr.getLength());
    pRegexMatcher->reset( aSearchTargetStr);
    // search until there is a valid match
    for(;;)
    {
        if (!lcl_findRegex( pRegexMatcher, startPos, endPos, nIcuErr))
            return aRet;

        // #i118887# ignore zero-length matches e.g. "a*" in "bc"
        int nStartOfs = pRegexMatcher->start( nIcuErr);
        int nEndOfs = pRegexMatcher->end( nIcuErr);
        if( nStartOfs < nEndOfs)
            break;
        // If the zero-length match is behind the string, do not match it again
        // and again until startPos reaches there. A match behind the string is
        // a "$" anchor.
        if (nStartOfs == endPos)
            break;
        // try at next position if there was a zero-length match
        if( ++startPos >= endPos)
            return aRet;
    }

    // extract the result of the search
    const int nGroupCount = pRegexMatcher->groupCount();
    aRet.subRegExpressions = nGroupCount + 1;
    aRet.startOffset.realloc( aRet.subRegExpressions);
    auto pstartOffset = aRet.startOffset.getArray();
    aRet.endOffset.realloc( aRet.subRegExpressions);
    auto pendOffset = aRet.endOffset.getArray();
    pstartOffset[0] = pRegexMatcher->start( nIcuErr);
    pendOffset[0]   = pRegexMatcher->end( nIcuErr);
    for( int i = 1; i <= nGroupCount; ++i) {
        pstartOffset[i] = pRegexMatcher->start( i, nIcuErr);
        pendOffset[i]   = pRegexMatcher->end( i, nIcuErr);
    }

    return aRet;
}

SearchResult TextSearch::RESrchBkwrd( const OUString& searchStr,
                                      sal_Int32 startPos, sal_Int32 endPos )
{
    // NOTE: for backwards search callers provide startPos/endPos inverted!
    SearchResult aRet;
    aRet.subRegExpressions = 0;
    if( !pRegexMatcher)
        return aRet;

    if( startPos > searchStr.getLength())
        startPos = searchStr.getLength();

    // use the ICU RegexMatcher to find the matches
    // TODO: use ICU's backward searching once it becomes available
    //       as its replacement using forward search is not as good as the real thing
    UErrorCode nIcuErr = U_ZERO_ERROR;
    const icu::UnicodeString aSearchTargetStr(false, reinterpret_cast<const UChar*>(searchStr.getStr()),
                                        searchStr.getLength());
    pRegexMatcher->reset( aSearchTargetStr);
    if (!lcl_findRegex( pRegexMatcher, endPos, startPos, nIcuErr))
        return aRet;

    // find the last match
    int nLastPos = 0;
    int nFoundEnd = 0;
    int nGoodPos = 0, nGoodEnd = 0;
    bool bFirst = true;
    do {
        nLastPos = pRegexMatcher->start( nIcuErr);
        nFoundEnd = pRegexMatcher->end( nIcuErr);
        if (nLastPos < nFoundEnd)
        {
            // remember last non-zero-length match
            nGoodPos = nLastPos;
            nGoodEnd = nFoundEnd;
        }
        if( nFoundEnd >= startPos)
            break;
        bFirst = false;
        if( nFoundEnd == nLastPos)
            ++nFoundEnd;
    } while( lcl_findRegex( pRegexMatcher, nFoundEnd, startPos, nIcuErr));

    // Ignore all zero-length matches except "$" anchor on first match.
    if (nGoodPos == nGoodEnd)
    {
        if (bFirst && nLastPos == startPos)
            nGoodPos = nLastPos;
        else
            return aRet;
    }

    // find last match again to get its details
    lcl_findRegex( pRegexMatcher, nGoodPos, startPos, nIcuErr);

    // fill in the details of the last match
    const int nGroupCount = pRegexMatcher->groupCount();
    aRet.subRegExpressions = nGroupCount + 1;
    aRet.startOffset.realloc( aRet.subRegExpressions);
    auto pstartOffset = aRet.startOffset.getArray();
    aRet.endOffset.realloc( aRet.subRegExpressions);
    auto pendOffset = aRet.endOffset.getArray();
    // NOTE: existing users of backward search seem to expect startOfs/endOfs being inverted!
    pstartOffset[0] = pRegexMatcher->end( nIcuErr);
    pendOffset[0]   = pRegexMatcher->start( nIcuErr);
    for( int i = 1; i <= nGroupCount; ++i) {
        pstartOffset[i] = pRegexMatcher->end( i, nIcuErr);
        pendOffset[i]   = pRegexMatcher->start( i, nIcuErr);
    }

    return aRet;
}


// search for words phonetically
SearchResult TextSearch::ApproxSrchFrwrd( const OUString& searchStr,
                                          sal_Int32 startPos, sal_Int32 endPos )
{
    SearchResult aRet;
    aRet.subRegExpressions = 0;

    if( !xBreak.is() )
        return aRet;

    sal_Int32 nStt, nEnd;

    Boundary aWBnd = xBreak->getWordBoundary( searchStr, startPos,
            aSrchPara.Locale,
            WordType::ANYWORD_IGNOREWHITESPACES, true );

    do
    {
        if( aWBnd.startPos >= endPos )
            break;
        nStt = aWBnd.startPos < startPos ? startPos : aWBnd.startPos;
        nEnd = std::min(aWBnd.endPos, endPos);

        if( nStt < nEnd &&
                pWLD->WLD( searchStr.getStr() + nStt, nEnd - nStt ) <= nLimit )
        {
            aRet.subRegExpressions = 1;
            aRet.startOffset = { nStt };
            aRet.endOffset = { nEnd };
            break;
        }

        nStt = nEnd - 1;
        aWBnd = xBreak->nextWord( searchStr, nStt, aSrchPara.Locale,
                WordType::ANYWORD_IGNOREWHITESPACES);
    } while( aWBnd.startPos != aWBnd.endPos ||
            (aWBnd.endPos != searchStr.getLength() && aWBnd.endPos != nEnd) );
    // #i50244# aWBnd.endPos != nEnd : in case there is _no_ word (only
    // whitespace) in searchStr, getWordBoundary() returned startPos,startPos
    // and nextWord() does also => don't loop forever.
    return aRet;
}

SearchResult TextSearch::ApproxSrchBkwrd( const OUString& searchStr,
                                          sal_Int32 startPos, sal_Int32 endPos )
{
    SearchResult aRet;
    aRet.subRegExpressions = 0;

    if( !xBreak.is() )
        return aRet;

    sal_Int32 nStt, nEnd;

    Boundary aWBnd = xBreak->getWordBoundary( searchStr, startPos,
            aSrchPara.Locale,
            WordType::ANYWORD_IGNOREWHITESPACES, true );

    do
    {
        if( aWBnd.endPos <= endPos )
            break;
        nStt = aWBnd.startPos < endPos ? endPos : aWBnd.startPos;
        nEnd = std::min(aWBnd.endPos, startPos);

        if( nStt < nEnd &&
                pWLD->WLD( searchStr.getStr() + nStt, nEnd - nStt ) <= nLimit )
        {
            aRet.subRegExpressions = 1;
            aRet.startOffset = { nEnd };
            aRet.endOffset = { nStt };
            break;
        }
        if( !nStt )
            break;

        aWBnd = xBreak->previousWord( searchStr, nStt, aSrchPara.Locale,
                WordType::ANYWORD_IGNOREWHITESPACES);
    } while( aWBnd.startPos != aWBnd.endPos || aWBnd.endPos != searchStr.getLength() );
    return aRet;
}


namespace {
void setWildcardMatch( css::util::SearchResult& rRes, sal_Int32 nStartOffset, sal_Int32 nEndOffset )
{
    rRes.subRegExpressions = 1;
    rRes.startOffset = { nStartOffset };
    rRes.endOffset = { nEndOffset };
}
}

SearchResult TextSearch::WildcardSrchFrwrd( const OUString& searchStr, sal_Int32 nStartPos, sal_Int32 nEndPos )
{
    SearchResult aRes;
    aRes.subRegExpressions = 0;     // no match
    sal_Int32 nStartOffset = nStartPos;
    sal_Int32 nEndOffset = nEndPos;

    const sal_Int32 nStringLen = searchStr.getLength();

    // Forward nStartPos inclusive, nEndPos exclusive, but allow for empty
    // string match with [0,0).
    if (nStartPos < 0 || nEndPos > nStringLen || nEndPos < nStartPos ||
            (nStartPos == nStringLen && (nStringLen != 0 || nStartPos != nEndPos)))
        return aRes;

    const OUString& rPattern = (bUsePrimarySrchStr ? sSrchStr : sSrchStr2);
    const sal_Int32 nPatternLen = rPattern.getLength();

    // Handle special cases empty pattern and/or string outside of the loop to
    // not add performance penalties there and simplify.
    if (nStartPos == nEndPos)
    {
        sal_Int32 i = 0;
        while (i < nPatternLen && rPattern[i] == '*')
            ++i;
        if (i == nPatternLen)
            setWildcardMatch( aRes, nStartOffset, nEndOffset);
        return aRes;
    }

    // Empty pattern does not match any non-empty string.
    if (!nPatternLen)
        return aRes;

    bool bRewind = false;
    sal_uInt32 cPattern = 0;
    sal_Int32 nPattern = 0;
    sal_Int32 nAfterFakePattern = nPattern;
    if (mbWildcardAllowSubstring)
    {
        // Fake a leading '*' wildcard.
        cPattern = '*';
        bRewind = true;
        // Assume a non-'*' pattern character follows. If it is a '*' instead
        // that will be handled in the loop by setting nPat.
        sal_uInt32 cu = rPattern.iterateCodePoints( &nAfterFakePattern);
        if (cu == mcWildcardEscapeChar && mcWildcardEscapeChar && nAfterFakePattern < nPatternLen)
            rPattern.iterateCodePoints( &nAfterFakePattern);
    }

    sal_Int32 nString = nStartPos, nPat = -1, nStr = -1, nLastAsterisk = -1;
    sal_uInt32 cPatternAfterAsterisk = 0;
    bool bEscaped = false, bEscapedAfterAsterisk = false;

    // The loop code tries to avoid costly calls to iterateCodePoints() when
    // possible.

    do
    {
        if (bRewind)
        {
            // Reuse cPattern after '*', nPattern was correspondingly
            // incremented to point behind cPattern.
            bRewind = false;
        }
        else if (nPattern < nPatternLen)
        {
            // nPattern will be incremented by iterateCodePoints().
            cPattern = rPattern.iterateCodePoints( &nPattern);
            if (cPattern == mcWildcardEscapeChar && mcWildcardEscapeChar && nPattern < nPatternLen)
            {
                bEscaped = true;
                cPattern = rPattern.iterateCodePoints( &nPattern);
            }
        }
        else
        {
            // A trailing '*' is handled below.
            if (mbWildcardAllowSubstring)
            {
                // If the pattern is consumed and substring match allowed we're good.
                setWildcardMatch( aRes, nStartOffset, nString);
                return aRes;
            }
            else if (nString < nEndPos && nLastAsterisk >= 0)
            {
                // If substring match is not allowed try a greedy '*' match.
                nPattern = nLastAsterisk;
                continue;   // do
            }
            else
                return aRes;
        }

        if (cPattern == '*' && !bEscaped)
        {
            // '*' is one code unit, so not using iterateCodePoints() is ok.
            while (nPattern < nPatternLen && rPattern[nPattern] == '*')
                ++nPattern;

            if (nPattern >= nPatternLen)
            {
                // Last pattern is '*', remaining string matches.
                setWildcardMatch( aRes, nStartOffset, nEndOffset);
                return aRes;
            }

            nLastAsterisk = nPattern;   // Remember last encountered '*'.

            // cPattern will be the next non-'*' character, nPattern
            // incremented.
            cPattern = rPattern.iterateCodePoints( &nPattern);
            if (cPattern == mcWildcardEscapeChar && mcWildcardEscapeChar && nPattern < nPatternLen)
            {
                bEscaped = true;
                cPattern = rPattern.iterateCodePoints( &nPattern);
            }

            cPatternAfterAsterisk = cPattern;
            bEscapedAfterAsterisk = bEscaped;
            nPat = nPattern;    // Remember position of pattern behind '*', already incremented.
            nStr = nString;     // Remember the current string to be matched.
        }

        if (nString >= nEndPos)
            // Whatever follows in pattern, string will not match.
            return aRes;

        // nString will be incremented by iterateCodePoints().
        sal_uInt32 cString = searchStr.iterateCodePoints( &nString);

        if ((cPattern != '?' || bEscaped) && cPattern != cString)
        {
            if (nPat == -1)
                // Non-match already without any '*' pattern.
                return aRes;

            bRewind = true;
            nPattern = nPat;                    // Rewind pattern to character behind '*', already incremented.
            cPattern = cPatternAfterAsterisk;
            bEscaped = bEscapedAfterAsterisk;
            searchStr.iterateCodePoints( &nStr);
            nString = nStr;                     // Restore incremented remembered string position.
            if (nPat == nAfterFakePattern)
            {
                // Next start offset will be the next character.
                nStartOffset = nString;
            }
        }
        else
        {
            // An unescaped '?' pattern matched any character, or characters
            // matched. Reset only escaped state.
            bEscaped = false;
        }
    }
    while (nString < nEndPos);

    if (bRewind)
        return aRes;

    // Eat trailing '*' pattern that matches anything, including nothing.
    // '*' is one code unit, so not using iterateCodePoints() is ok.
    while (nPattern < nPatternLen && rPattern[nPattern] == '*')
        ++nPattern;

    if (nPattern == nPatternLen)
        setWildcardMatch( aRes, nStartOffset, nEndOffset);
    return aRes;
}

SearchResult TextSearch::WildcardSrchBkwrd( const OUString& searchStr, sal_Int32 nStartPos, sal_Int32 nEndPos )
{
    SearchResult aRes;
    aRes.subRegExpressions = 0;     // no match

    sal_Int32 nStartOffset = nStartPos;
    sal_Int32 nEndOffset = nEndPos;

    const sal_Int32 nStringLen = searchStr.getLength();

    // Backward nStartPos exclusive, nEndPos inclusive, but allow for empty
    // string match with (0,0].
    if (nStartPos > nStringLen || nEndPos < 0 || nStartPos < nEndPos ||
            (nEndPos == nStringLen && (nStringLen != 0 || nStartPos != nEndPos)))
        return aRes;

    const OUString& rPattern = (bUsePrimarySrchStr ? sSrchStr : sSrchStr2);
    sal_Int32 nPatternLen = rPattern.getLength();

    // Handle special cases empty pattern and/or string outside of the loop to
    // not add performance penalties there and simplify.
    if (nStartPos == nEndPos)
    {
        sal_Int32 i = 0;
        while (i < nPatternLen && rPattern[i] == '*')
            ++i;
        if (i == nPatternLen)
            setWildcardMatch( aRes, nStartOffset, nEndOffset);
        return aRes;
    }

    // Empty pattern does not match any non-empty string.
    if (!nPatternLen)
        return aRes;

    // Reverse escaped patterns to ease the handling of escapes, keeping escape
    // and following character as one sequence in backward direction.
    if ((bUsePrimarySrchStr && maWildcardReversePattern.isEmpty()) ||
            (!bUsePrimarySrchStr && maWildcardReversePattern2.isEmpty()))
    {
        OUStringBuffer aPatternBuf( rPattern);
        sal_Int32 nIndex = 0;
        while (nIndex < nPatternLen)
        {
            const sal_Int32 nOld = nIndex;
            const sal_uInt32 cu = rPattern.iterateCodePoints( &nIndex);
            if (cu == mcWildcardEscapeChar)
            {
                if (nIndex < nPatternLen)
                {
                    if (nIndex - nOld == 1)
                    {
                        // Simply move code units, we already memorized the one
                        // in 'cu'.
                        const sal_Int32 nOld2 = nIndex;
                        rPattern.iterateCodePoints( &nIndex);
                        for (sal_Int32 i=0; i < nIndex - nOld2; ++i)
                            aPatternBuf[nOld+i] = rPattern[nOld2+i];
                        aPatternBuf[nIndex-1] = static_cast<sal_Unicode>(cu);
                    }
                    else
                    {
                        // Copy the escape character code units first in the
                        // unlikely case that it would not be of BMP.
                        assert(nIndex - nOld == 2);  // it's UTF-16, so...
                        sal_Unicode buf[2];
                        buf[0] = rPattern[nOld];
                        buf[1] = rPattern[nOld+1];
                        const sal_Int32 nOld2 = nIndex;
                        rPattern.iterateCodePoints( &nIndex);
                        for (sal_Int32 i=0; i < nIndex - nOld2; ++i)
                            aPatternBuf[nOld+i] = rPattern[nOld2+i];
                        aPatternBuf[nIndex-2] = buf[0];
                        aPatternBuf[nIndex-1] = buf[1];
                    }
                }
                else
                {
                    // Trailing escape would become leading escape, do what?
                    // Eliminate.
                    aPatternBuf.remove( nOld, nIndex - nOld);
                }
            }
        }
        if (bUsePrimarySrchStr)
            maWildcardReversePattern = aPatternBuf.makeStringAndClear();
        else
            maWildcardReversePattern2 = aPatternBuf.makeStringAndClear();
    }
    const OUString& rReversePattern = (bUsePrimarySrchStr ? maWildcardReversePattern : maWildcardReversePattern2);
    nPatternLen = rReversePattern.getLength();

    bool bRewind = false;
    sal_uInt32 cPattern = 0;
    sal_Int32 nPattern = nPatternLen;
    sal_Int32 nAfterFakePattern = nPattern;
    if (mbWildcardAllowSubstring)
    {
        // Fake a trailing '*' wildcard.
        cPattern = '*';
        bRewind = true;
        // Assume a non-'*' pattern character follows. If it is a '*' instead
        // that will be handled in the loop by setting nPat.
        sal_uInt32 cu = rReversePattern.iterateCodePoints( &nAfterFakePattern, -1);
        if (cu == mcWildcardEscapeChar && mcWildcardEscapeChar && nAfterFakePattern > 0)
            rReversePattern.iterateCodePoints( &nAfterFakePattern, -1);
    }

    sal_Int32 nString = nStartPos, nPat = -1, nStr = -1, nLastAsterisk = -1;
    sal_uInt32 cPatternAfterAsterisk = 0;
    bool bEscaped = false, bEscapedAfterAsterisk = false;

    // The loop code tries to avoid costly calls to iterateCodePoints() when
    // possible.

    do
    {
        if (bRewind)
        {
            // Reuse cPattern after '*', nPattern was correspondingly
            // decremented to point before cPattern.
            bRewind = false;
        }
        else if (nPattern > 0)
        {
            // nPattern will be decremented by iterateCodePoints().
            cPattern = rReversePattern.iterateCodePoints( &nPattern, -1);
            if (cPattern == mcWildcardEscapeChar && mcWildcardEscapeChar && nPattern > 0)
            {
                bEscaped = true;
                cPattern = rReversePattern.iterateCodePoints( &nPattern, -1);
            }
        }
        else
        {
            // A trailing '*' is handled below.
            if (mbWildcardAllowSubstring)
            {
                // If the pattern is consumed and substring match allowed we're good.
                setWildcardMatch( aRes, nStartOffset, nString);
                return aRes;
            }
            else if (nString > nEndPos && nLastAsterisk >= 0)
            {
                // If substring match is not allowed try a greedy '*' match.
                nPattern = nLastAsterisk;
                continue;   // do
            }
            else
                return aRes;
        }

        if (cPattern == '*' && !bEscaped)
        {
            // '*' is one code unit, so not using iterateCodePoints() is ok.
            while (nPattern > 0 && rReversePattern[nPattern-1] == '*')
                --nPattern;

            if (nPattern <= 0)
            {
                // First pattern is '*', remaining string matches.
                setWildcardMatch( aRes, nStartOffset, nEndOffset);
                return aRes;
            }

            nLastAsterisk = nPattern;   // Remember last encountered '*'.

            // cPattern will be the previous non-'*' character, nPattern
            // decremented.
            cPattern = rReversePattern.iterateCodePoints( &nPattern, -1);
            if (cPattern == mcWildcardEscapeChar && mcWildcardEscapeChar && nPattern > 0)
            {
                bEscaped = true;
                cPattern = rReversePattern.iterateCodePoints( &nPattern, -1);
            }

            cPatternAfterAsterisk = cPattern;
            bEscapedAfterAsterisk = bEscaped;
            nPat = nPattern;    // Remember position of pattern before '*', already decremented.
            nStr = nString;     // Remember the current string to be matched.
        }

        if (nString <= nEndPos)
            // Whatever leads in pattern, string will not match.
            return aRes;

        // nString will be decremented by iterateCodePoints().
        sal_uInt32 cString = searchStr.iterateCodePoints( &nString, -1);

        if ((cPattern != '?' || bEscaped) && cPattern != cString)
        {
            if (nPat == -1)
                // Non-match already without any '*' pattern.
                return aRes;

            bRewind = true;
            nPattern = nPat;                    // Rewind pattern to character before '*', already decremented.
            cPattern = cPatternAfterAsterisk;
            bEscaped = bEscapedAfterAsterisk;
            searchStr.iterateCodePoints( &nStr, -1);
            nString = nStr;                     // Restore decremented remembered string position.
            if (nPat == nAfterFakePattern)
            {
                // Next start offset will be this character (exclusive).
                nStartOffset = nString;
            }
        }
        else
        {
            // An unescaped '?' pattern matched any character, or characters
            // matched. Reset only escaped state.
            bEscaped = false;
        }
    }
    while (nString > nEndPos);

    if (bRewind)
        return aRes;

    // Eat leading '*' pattern that matches anything, including nothing.
    // '*' is one code unit, so not using iterateCodePoints() is ok.
    while (nPattern > 0 && rReversePattern[nPattern-1] == '*')
        --nPattern;

    if (nPattern == 0)
        setWildcardMatch( aRes, nStartOffset, nEndOffset);
    return aRes;
}


OUString SAL_CALL
TextSearch::getImplementationName()
{
    return u"com.sun.star.util.TextSearch_i18n"_ustr;
}

sal_Bool SAL_CALL TextSearch::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL
TextSearch::getSupportedServiceNames()
{
    return { u"com.sun.star.util.TextSearch"_ustr, u"com.sun.star.util.TextSearch2"_ustr };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
i18npool_TextSearch_get_implementation(
    css::uno::XComponentContext* context , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new TextSearch(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
