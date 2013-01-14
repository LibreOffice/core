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
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/i18n/BreakIterator.hpp>
#include <com/sun/star/i18n/UnicodeType.hpp>
#include <com/sun/star/util/SearchFlags.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#include <com/sun/star/i18n/CharacterClassification.hpp>
#include <com/sun/star/i18n/KCharacterType.hpp>
#include <com/sun/star/i18n/Transliteration.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/weak.hxx>

#ifdef _MSC_VER
// get rid of that dumb compiler warning
// identifier was truncated to '255' characters in the debug information
// for STL template usage, if .pdb files are to be created
#pragma warning( disable: 4786 )
#endif

#include <string.h>

using namespace ::com::sun::star::util;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star;

static sal_Int32 COMPLEX_TRANS_MASK_TMP =
    TransliterationModules_ignoreBaFa_ja_JP |
    TransliterationModules_ignoreIterationMark_ja_JP |
    TransliterationModules_ignoreTiJi_ja_JP |
    TransliterationModules_ignoreHyuByu_ja_JP |
    TransliterationModules_ignoreSeZe_ja_JP |
    TransliterationModules_ignoreIandEfollowedByYa_ja_JP |
    TransliterationModules_ignoreKiKuFollowedBySa_ja_JP |
    TransliterationModules_ignoreProlongedSoundMark_ja_JP;
static const sal_Int32 COMPLEX_TRANS_MASK = COMPLEX_TRANS_MASK_TMP | TransliterationModules_IGNORE_KANA | TransliterationModules_FULLWIDTH_HALFWIDTH;
static const sal_Int32 SIMPLE_TRANS_MASK = ~COMPLEX_TRANS_MASK;
    // Above 2 transliteration is simple but need to take effect in
    // complex transliteration

TextSearch::TextSearch(const Reference < XComponentContext > & rxContext)
        : m_xContext( rxContext )
        , pJumpTable( 0 )
        , pJumpTable2( 0 )
        , pRegexMatcher( NULL )
        , pWLD( 0 )
{
    SearchOptions aOpt;
    aOpt.algorithmType = SearchAlgorithms_ABSOLUTE;
    aOpt.searchFlag = SearchFlags::ALL_IGNORE_CASE;
    //aOpt.Locale = ???;
    setOptions( aOpt );
}

TextSearch::~TextSearch()
{
    delete pRegexMatcher;
    delete pWLD;
    delete pJumpTable;
    delete pJumpTable2;
}

void TextSearch::setOptions( const SearchOptions& rOptions ) throw( RuntimeException )
{
    aSrchPara = rOptions;

    delete pRegexMatcher, pRegexMatcher = NULL;
    delete pWLD, pWLD = 0;
    delete pJumpTable, pJumpTable = 0;
    delete pJumpTable2, pJumpTable2 = 0;

    // Create Transliteration class
    if( aSrchPara.transliterateFlags & SIMPLE_TRANS_MASK )
    {
        if( !xTranslit.is() )
            xTranslit.set( Transliteration::create( m_xContext ) );
        xTranslit->loadModule(
             (TransliterationModules)( aSrchPara.transliterateFlags & SIMPLE_TRANS_MASK ),
             aSrchPara.Locale);
    }
    else if( xTranslit.is() )
        xTranslit = 0;

    // Create Transliteration for 2<->1, 2<->2 transliteration
    if ( aSrchPara.transliterateFlags & COMPLEX_TRANS_MASK )
    {
        if( !xTranslit2.is() )
            xTranslit2.set( Transliteration::create( m_xContext ) );
        // Load transliteration module
        xTranslit2->loadModule(
             (TransliterationModules)( aSrchPara.transliterateFlags & COMPLEX_TRANS_MASK ),
             aSrchPara.Locale);
    }

    if ( !xBreak.is() )
        xBreak = com::sun::star::i18n::BreakIterator::create( m_xContext );

    sSrchStr = aSrchPara.searchString;

    // use transliteration here
    if ( xTranslit.is() && aSrchPara.transliterateFlags & SIMPLE_TRANS_MASK )
        sSrchStr = xTranslit->transliterateString2String(
            aSrchPara.searchString, 0, aSrchPara.searchString.getLength());

    if ( xTranslit2.is() && aSrchPara.transliterateFlags & COMPLEX_TRANS_MASK )
        sSrchStr2 = xTranslit2->transliterateString2String(
            aSrchPara.searchString, 0, aSrchPara.searchString.getLength());

    // When start or end of search string is a complex script type, we need to
    // make sure the result boundary is not located in the middle of cell.
    checkCTLStart = (xBreak.is() && (xBreak->getScriptType(sSrchStr, 0) ==
                ScriptType::COMPLEX));
    checkCTLEnd = (xBreak.is() && (xBreak->getScriptType(sSrchStr,
                    sSrchStr.getLength()-1) == ScriptType::COMPLEX));

    switch( aSrchPara.algorithmType)
    {
        case SearchAlgorithms_REGEXP:
            fnForward = &TextSearch::RESrchFrwrd;
            fnBackward = &TextSearch::RESrchBkwrd;
            RESrchPrepare( aSrchPara);
            break;

        case SearchAlgorithms_APPROXIMATE:
            fnForward = &TextSearch::ApproxSrchFrwrd;
            fnBackward = &TextSearch::ApproxSrchBkwrd;

            pWLD = new WLevDistance( sSrchStr.getStr(), aSrchPara.changedChars,
                    aSrchPara.insertedChars, aSrchPara.deletedChars,
                    0 != (SearchFlags::LEV_RELAXED & aSrchPara.searchFlag ) );

            nLimit = pWLD->GetLimit();
            break;

        default:
            fnForward = &TextSearch::NSrchFrwrd;
            fnBackward = &TextSearch::NSrchBkwrd;
            break;
    }
}

sal_Int32 FindPosInSeq_Impl( const Sequence <sal_Int32>& rOff, sal_Int32 nPos )
{
    sal_Int32 nRet = 0, nEnd = rOff.getLength();
    while( nRet < nEnd && nPos > rOff[ nRet ] ) ++nRet;
    return nRet;
}

sal_Bool TextSearch::isCellStart(const OUString& searchStr, sal_Int32 nPos)
        throw( RuntimeException )
{
    sal_Int32 nDone;
    return nPos == xBreak->previousCharacters(searchStr, nPos+1,
            aSrchPara.Locale, CharacterIteratorMode::SKIPCELL, 1, nDone);
}

SearchResult TextSearch::searchForward( const OUString& searchStr, sal_Int32 startPos, sal_Int32 endPos )
        throw( RuntimeException )
{
    SearchResult sres;

    OUString in_str(searchStr);
    sal_Int32 newStartPos = startPos;
    sal_Int32 newEndPos = endPos;

    bUsePrimarySrchStr = true;

    if ( xTranslit.is() )
    {
        // apply normal transliteration (1<->1, 1<->0)
        com::sun::star::uno::Sequence <sal_Int32> offset( in_str.getLength());
        in_str = xTranslit->transliterate( searchStr, 0, in_str.getLength(), offset );

        // JP 20.6.2001: also the start and end positions must be corrected!
        if( startPos )
            newStartPos = FindPosInSeq_Impl( offset, startPos );

        if( endPos < searchStr.getLength() )
            newEndPos = FindPosInSeq_Impl( offset, endPos );
        else
            newEndPos = in_str.getLength();

        sres = (this->*fnForward)( in_str, newStartPos, newEndPos );

        for ( int k = 0; k < sres.startOffset.getLength(); k++ )
        {
            if (sres.startOffset[k])
                sres.startOffset[k] = offset[sres.startOffset[k]];
            // JP 20.6.2001: end is ever exclusive and then don't return
            //               the position of the next character - return the
            //               next position behind the last found character!
            //               "a b c" find "b" must return 2,3 and not 2,4!!!
            if (sres.endOffset[k])
                sres.endOffset[k] = offset[sres.endOffset[k]-1] + 1;
        }
    }
    else
    {
        sres = (this->*fnForward)( in_str, startPos, endPos );
    }

    if ( xTranslit2.is() && aSrchPara.algorithmType != SearchAlgorithms_REGEXP)
    {
        SearchResult sres2;

        in_str = OUString(searchStr);
        com::sun::star::uno::Sequence <sal_Int32> offset( in_str.getLength());

        in_str = xTranslit2->transliterate( searchStr, 0, in_str.getLength(), offset );

        if( startPos )
            startPos = FindPosInSeq_Impl( offset, startPos );

        if( endPos < searchStr.getLength() )
            endPos = FindPosInSeq_Impl( offset, endPos );
        else
            endPos = in_str.getLength();

    bUsePrimarySrchStr = false;
        sres2 = (this->*fnForward)( in_str, startPos, endPos );

        for ( int k = 0; k < sres2.startOffset.getLength(); k++ )
        {
            if (sres2.startOffset[k])
          sres2.startOffset[k] = offset[sres2.startOffset[k]-1] + 1;
            if (sres2.endOffset[k])
          sres2.endOffset[k] = offset[sres2.endOffset[k]-1] + 1;
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
        throw(RuntimeException)
{
    SearchResult sres;

    OUString in_str(searchStr);
    sal_Int32 newStartPos = startPos;
    sal_Int32 newEndPos = endPos;

    bUsePrimarySrchStr = true;

    if ( xTranslit.is() )
    {
        // apply only simple 1<->1 transliteration here
        com::sun::star::uno::Sequence <sal_Int32> offset( in_str.getLength());
    in_str = xTranslit->transliterate( searchStr, 0, in_str.getLength(), offset );

        // JP 20.6.2001: also the start and end positions must be corrected!
        if( startPos < searchStr.getLength() )
            newStartPos = FindPosInSeq_Impl( offset, startPos );
        else
            newStartPos = in_str.getLength();

        if( endPos )
            newEndPos = FindPosInSeq_Impl( offset, endPos );

        sres = (this->*fnBackward)( in_str, newStartPos, newEndPos );

        for ( int k = 0; k < sres.startOffset.getLength(); k++ )
        {
            if (sres.startOffset[k])
                sres.startOffset[k] = offset[sres.startOffset[k] - 1] + 1;
            // JP 20.6.2001: end is ever exclusive and then don't return
            //               the position of the next character - return the
            //               next position behind the last found character!
            //               "a b c" find "b" must return 2,3 and not 2,4!!!
            if (sres.endOffset[k])
                sres.endOffset[k] = offset[sres.endOffset[k]];
        }
    }
    else
    {
        sres = (this->*fnBackward)( in_str, startPos, endPos );
    }

    if ( xTranslit2.is() && aSrchPara.algorithmType != SearchAlgorithms_REGEXP )
    {
    SearchResult sres2;

    in_str = OUString(searchStr);
        com::sun::star::uno::Sequence <sal_Int32> offset( in_str.getLength());

        in_str = xTranslit2->transliterate(searchStr, 0, in_str.getLength(), offset);

        if( startPos < searchStr.getLength() )
            startPos = FindPosInSeq_Impl( offset, startPos );
        else
            startPos = in_str.getLength();

        if( endPos )
            endPos = FindPosInSeq_Impl( offset, endPos );

    bUsePrimarySrchStr = false;
    sres2 = (this->*fnBackward)( in_str, startPos, endPos );

        for( int k = 0; k < sres2.startOffset.getLength(); k++ )
        {
            if (sres2.startOffset[k])
                sres2.startOffset[k] = offset[sres2.startOffset[k]-1]+1;
            if (sres2.endOffset[k])
                sres2.endOffset[k] = offset[sres2.endOffset[k]-1]+1;
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

//---------------------------------------------------------------------

bool TextSearch::IsDelimiter( const OUString& rStr, sal_Int32 nPos ) const
{
    bool bRet = 1;
    if( '\x7f' != rStr[nPos])
    {
        if ( !xCharClass.is() )
             xCharClass = CharacterClassification::create( m_xContext );
        sal_Int32 nCType = xCharClass->getCharacterType( rStr, nPos,
                aSrchPara.Locale );
        if( 0 != (( KCharacterType::DIGIT | KCharacterType::ALPHA |
                        KCharacterType::LETTER ) & nCType ) )
            bRet = 0;
    }
    return bRet;
}

// --------- helper methods for Boyer-Moore like text searching ----------
// TODO: use ICU's regex UREGEX_LITERAL mode instead when it becomes available

void TextSearch::MakeForwardTab()
{
    // create the jumptable for the search text
    if( pJumpTable )
    {
        if( bIsForwardTab )
            return ;                                        // the jumpTable is ok
        delete pJumpTable;
    }
    bIsForwardTab = true;

    sal_Int32 n, nLen = sSrchStr.getLength();
    pJumpTable = new TextSearchJumpTable;

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
    if( pJumpTable2 )
    {
        if( bIsForwardTab )
            return ;                                        // the jumpTable is ok
        delete pJumpTable2;
    }
    bIsForwardTab = true;

    sal_Int32 n, nLen = sSrchStr2.getLength();
    pJumpTable2 = new TextSearchJumpTable;

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
    if( pJumpTable )
    {
        if( !bIsForwardTab )
            return ;                                        // the jumpTable is ok
        delete pJumpTable;
    }
    bIsForwardTab = false;

    sal_Int32 n, nLen = sSrchStr.getLength();
    pJumpTable = new TextSearchJumpTable;

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
    if( pJumpTable2 )
    {
        if( !bIsForwardTab )
            return ;                                        // the jumpTable is ok
        delete pJumpTable2;
    }
    bIsForwardTab = false;

    sal_Int32 n, nLen = sSrchStr2.getLength();
    pJumpTable2 = new TextSearchJumpTable;

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
      pJump = pJumpTable;
      sSearchKey = sSrchStr;
    } else {
      pJump = pJumpTable2;
      sSearchKey = sSrchStr2;
    }

    TextSearchJumpTable::const_iterator iLook = pJump->find( cChr );
    if ( iLook == pJump->end() )
        return sSearchKey.getLength();
    return (*iLook).second;
}


// TextSearch::NSrchFrwrd is mis-optimized on unxsoli (#i105945#)
SearchResult TextSearch::NSrchFrwrd( const OUString& searchStr, sal_Int32 startPos, sal_Int32 endPos )
        throw(RuntimeException)
{
    SearchResult aRet;
    aRet.subRegExpressions = 0;

    OUString sSearchKey = bUsePrimarySrchStr ? sSrchStr : sSrchStr2;

    OUString aStr( searchStr );
    sal_Int32 nSuchIdx = aStr.getLength();
    sal_Int32 nEnde = endPos;
    if( !nSuchIdx || !sSearchKey.getLength() || sSearchKey.getLength() > nSuchIdx )
        return aRet;


    if( nEnde < sSearchKey.getLength() )  // position inside the search region ?
        return aRet;

    nEnde -= sSearchKey.getLength();

    if (bUsePrimarySrchStr)
      MakeForwardTab();                   // create the jumptable
    else
      MakeForwardTab2();

    for (sal_Int32 nCmpIdx = startPos; // start position for the search
            nCmpIdx <= nEnde;
            nCmpIdx += GetDiff( aStr[nCmpIdx + sSearchKey.getLength()-1]))
    {
        // if the match would be the completed cells, skip it.
        if ( (checkCTLStart && !isCellStart( aStr, nCmpIdx )) || (checkCTLEnd
                    && !isCellStart( aStr, nCmpIdx + sSearchKey.getLength())) )
            continue;

        nSuchIdx = sSearchKey.getLength() - 1;
        while( nSuchIdx >= 0 && sSearchKey[nSuchIdx] == aStr[nCmpIdx + nSuchIdx])
        {
            if( nSuchIdx == 0 )
            {
                if( SearchFlags::NORM_WORD_ONLY & aSrchPara.searchFlag )
                {
                    sal_Int32 nFndEnd = nCmpIdx + sSearchKey.getLength();
                    bool bAtStart = !nCmpIdx;
                    bool bAtEnd = nFndEnd == endPos;
                    bool bDelimBefore = bAtStart || IsDelimiter( aStr, nCmpIdx-1 );
                    bool bDelimBehind = IsDelimiter(  aStr, nFndEnd );
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
                aRet.startOffset.realloc( 1 );
                aRet.startOffset[ 0 ] = nCmpIdx;
                aRet.endOffset.realloc( 1 );
                aRet.endOffset[ 0 ] = nCmpIdx + sSearchKey.getLength();

                return aRet;
            }
            else
                nSuchIdx--;
        }
    }
    return aRet;
}

SearchResult TextSearch::NSrchBkwrd( const OUString& searchStr, sal_Int32 startPos, sal_Int32 endPos )
        throw(RuntimeException)
{
    SearchResult aRet;
    aRet.subRegExpressions = 0;

    OUString sSearchKey = bUsePrimarySrchStr ? sSrchStr : sSrchStr2;

    OUString aStr( searchStr );
    sal_Int32 nSuchIdx = aStr.getLength();
    sal_Int32 nEnde = endPos;
    if( nSuchIdx == 0 || sSearchKey.isEmpty() || sSearchKey.getLength() > nSuchIdx)
        return aRet;

    if (bUsePrimarySrchStr)
      MakeBackwardTab();                      // create the jumptable
    else
      MakeBackwardTab2();

    if( nEnde == nSuchIdx )                 // end position for the search
        nEnde = sSearchKey.getLength();
    else
        nEnde += sSearchKey.getLength();

    sal_Int32 nCmpIdx = startPos;          // start position for the search

    while (nCmpIdx >= nEnde)
    {
        // if the match would be the completed cells, skip it.
        if ( (!checkCTLStart || isCellStart( aStr, nCmpIdx -
                        sSearchKey.getLength() )) && (!checkCTLEnd ||
                    isCellStart( aStr, nCmpIdx)))
        {
            nSuchIdx = 0;
            while( nSuchIdx < sSearchKey.getLength() && sSearchKey[nSuchIdx] ==
                    aStr[nCmpIdx + nSuchIdx - sSearchKey.getLength()] )
                nSuchIdx++;
            if( nSuchIdx >= sSearchKey.getLength() )
            {
                if( SearchFlags::NORM_WORD_ONLY & aSrchPara.searchFlag )
                {
                    sal_Int32 nFndStt = nCmpIdx - sSearchKey.getLength();
                    bool bAtStart = !nFndStt;
                    bool bAtEnd = nCmpIdx == startPos;
                    bool bDelimBehind = IsDelimiter( aStr, nCmpIdx );
                    bool bDelimBefore = bAtStart || // begin of paragraph
                        IsDelimiter( aStr, nFndStt-1 );
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
                        aRet.startOffset.realloc( 1 );
                        aRet.startOffset[ 0 ] = nCmpIdx;
                        aRet.endOffset.realloc( 1 );
                        aRet.endOffset[ 0 ] = nCmpIdx - sSearchKey.getLength();
                        return aRet;
                    }
                }
                else
                {
                    aRet.subRegExpressions = 1;
                    aRet.startOffset.realloc( 1 );
                    aRet.startOffset[ 0 ] = nCmpIdx;
                    aRet.endOffset.realloc( 1 );
                    aRet.endOffset[ 0 ] = nCmpIdx - sSearchKey.getLength();
                    return aRet;
                }
            }
        }
        nSuchIdx = GetDiff( aStr[nCmpIdx - sSearchKey.getLength()] );
        if( nCmpIdx < nSuchIdx )
            return aRet;
        nCmpIdx -= nSuchIdx;
    }
    return aRet;
}

void TextSearch::RESrchPrepare( const ::com::sun::star::util::SearchOptions& rOptions)
{
    // select the transliterated pattern string
    const OUString& rPatternStr =
        (rOptions.transliterateFlags & SIMPLE_TRANS_MASK) ? sSrchStr
        : ((rOptions.transliterateFlags & COMPLEX_TRANS_MASK) ? sSrchStr2 : rOptions.searchString);

    sal_uInt32 nIcuSearchFlags = UREGEX_UWORD; // request UAX#29 unicode capability
    // map com::sun::star::util::SearchFlags to ICU uregex.h flags
    // TODO: REG_EXTENDED, REG_NOT_BEGINOFLINE, REG_NOT_ENDOFLINE
    // REG_NEWLINE is neither properly defined nor used anywhere => not implemented
    // REG_NOSUB is not used anywhere => not implemented
    // NORM_WORD_ONLY is only used for SearchAlgorithm==Absolute
    // LEV_RELAXED is only used for SearchAlgorithm==Approximate
    // why is even ALL_IGNORE_CASE deprecated in UNO? because of transliteration taking care of it???
    if( (rOptions.searchFlag & com::sun::star::util::SearchFlags::ALL_IGNORE_CASE) != 0)
        nIcuSearchFlags |= UREGEX_CASE_INSENSITIVE;
    UErrorCode nIcuErr = U_ZERO_ERROR;
    // assumption: transliteration didn't mangle regexp control chars
    IcuUniString aIcuSearchPatStr( (const UChar*)rPatternStr.getStr(), rPatternStr.getLength());
#ifndef DISABLE_WORDBOUND_EMULATION
    // for conveniance specific syntax elements of the old regex engine are emulated
    // - by replacing \< with "word-break followed by a look-ahead word-char"
    static const IcuUniString aChevronPatternB( "\\\\<", -1, IcuUniString::kInvariant);
    static const IcuUniString aChevronReplaceB( "\\\\b(?=\\\\w)", -1, IcuUniString::kInvariant);
    static RegexMatcher aChevronMatcherB( aChevronPatternB, 0, nIcuErr);
    aChevronMatcherB.reset( aIcuSearchPatStr);
    aIcuSearchPatStr = aChevronMatcherB.replaceAll( aChevronReplaceB, nIcuErr);
    aChevronMatcherB.reset();
    // - by replacing \> with "look-behind word-char followed by a word-break"
    static const IcuUniString aChevronPatternE( "\\\\>", -1, IcuUniString::kInvariant);
    static const IcuUniString aChevronReplaceE( "(?<=\\\\w)\\\\b", -1, IcuUniString::kInvariant);
    static RegexMatcher aChevronMatcherE( aChevronPatternE, 0, nIcuErr);
    aChevronMatcherE.reset( aIcuSearchPatStr);
    aIcuSearchPatStr = aChevronMatcherE.replaceAll( aChevronReplaceE, nIcuErr);
    aChevronMatcherE.reset();
#endif
    pRegexMatcher = new RegexMatcher( aIcuSearchPatStr, nIcuSearchFlags, nIcuErr);
    if( nIcuErr)
        { delete pRegexMatcher; pRegexMatcher = NULL;}
}

//---------------------------------------------------------------------------

SearchResult TextSearch::RESrchFrwrd( const OUString& searchStr,
                                      sal_Int32 startPos, sal_Int32 endPos )
            throw(RuntimeException)
{
    SearchResult aRet;
    aRet.subRegExpressions = 0;
    if( !pRegexMatcher)
        return aRet;

    if( endPos > searchStr.getLength())
        endPos = searchStr.getLength();

    // use the ICU RegexMatcher to find the matches
    UErrorCode nIcuErr = U_ZERO_ERROR;
    const IcuUniString aSearchTargetStr( (const UChar*)searchStr.getStr(), endPos);
    pRegexMatcher->reset( aSearchTargetStr);
    // search until there is a valid match
    for(;;)
    {
        if( !pRegexMatcher->find( startPos, nIcuErr))
            return aRet;

        // #i118887# ignore zero-length matches e.g. "a*" in "bc"
        int nStartOfs = pRegexMatcher->start( nIcuErr);
        int nEndOfs = pRegexMatcher->end( nIcuErr);
        if( nStartOfs < nEndOfs)
            break;
        // try at next position if there was a zero-length match
        if( ++startPos >= endPos)
            return aRet;
    }

    // extract the result of the search
    const int nGroupCount = pRegexMatcher->groupCount();
    aRet.subRegExpressions = nGroupCount + 1;
    aRet.startOffset.realloc( aRet.subRegExpressions);
    aRet.endOffset.realloc( aRet.subRegExpressions);
    aRet.startOffset[0] = pRegexMatcher->start( nIcuErr);
    aRet.endOffset[0]   = pRegexMatcher->end( nIcuErr);
    for( int i = 1; i <= nGroupCount; ++i) {
        aRet.startOffset[i] = pRegexMatcher->start( i, nIcuErr);
        aRet.endOffset[i]   = pRegexMatcher->end( i, nIcuErr);
    }

    return aRet;
}

SearchResult TextSearch::RESrchBkwrd( const OUString& searchStr,
                                      sal_Int32 startPos, sal_Int32 endPos )
            throw(RuntimeException)
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
    const IcuUniString aSearchTargetStr( (const UChar*)searchStr.getStr(), startPos);
    pRegexMatcher->reset( aSearchTargetStr);
    if( !pRegexMatcher->find( endPos, nIcuErr))
        return aRet;

    // find the last match
    int nLastPos = 0;
    do {
        nLastPos = pRegexMatcher->start( nIcuErr);
    } while( pRegexMatcher->find( nLastPos + 1, nIcuErr));

    // find last match again to get its details
    pRegexMatcher->find( nLastPos, nIcuErr);

    // fill in the details of the last match
    const int nGroupCount = pRegexMatcher->groupCount();
    aRet.subRegExpressions = nGroupCount + 1;
    aRet.startOffset.realloc( aRet.subRegExpressions);
    aRet.endOffset.realloc( aRet.subRegExpressions);
    // NOTE: existing users of backward search seem to expect startOfs/endOfs being inverted!
    aRet.startOffset[0] = pRegexMatcher->end( nIcuErr);
    aRet.endOffset[0]   = pRegexMatcher->start( nIcuErr);
    for( int i = 1; i <= nGroupCount; ++i) {
        aRet.startOffset[i] = pRegexMatcher->end( i, nIcuErr);
        aRet.endOffset[i]   = pRegexMatcher->start( i, nIcuErr);
    }

    return aRet;
}

//---------------------------------------------------------------------------

// search for words phonetically
SearchResult TextSearch::ApproxSrchFrwrd( const OUString& searchStr,
                                          sal_Int32 startPos, sal_Int32 endPos )
            throw(RuntimeException)
{
    SearchResult aRet;
    aRet.subRegExpressions = 0;

    if( !xBreak.is() )
        return aRet;

    OUString aWTemp( searchStr );

    register sal_Int32 nStt, nEnd;

    Boundary aWBnd = xBreak->getWordBoundary( aWTemp, startPos,
            aSrchPara.Locale,
            WordType::ANYWORD_IGNOREWHITESPACES, sal_True );

    do
    {
        if( aWBnd.startPos >= endPos )
            break;
        nStt = aWBnd.startPos < startPos ? startPos : aWBnd.startPos;
        nEnd = aWBnd.endPos > endPos ? endPos : aWBnd.endPos;

        if( nStt < nEnd &&
                pWLD->WLD( aWTemp.getStr() + nStt, nEnd - nStt ) <= nLimit )
        {
            aRet.subRegExpressions = 1;
            aRet.startOffset.realloc( 1 );
            aRet.startOffset[ 0 ] = nStt;
            aRet.endOffset.realloc( 1 );
            aRet.endOffset[ 0 ] = nEnd;
            break;
        }

        nStt = nEnd - 1;
        aWBnd = xBreak->nextWord( aWTemp, nStt, aSrchPara.Locale,
                WordType::ANYWORD_IGNOREWHITESPACES);
    } while( aWBnd.startPos != aWBnd.endPos ||
            (aWBnd.endPos != aWTemp.getLength() && aWBnd.endPos != nEnd) );
    // #i50244# aWBnd.endPos != nEnd : in case there is _no_ word (only
    // whitespace) in searchStr, getWordBoundary() returned startPos,startPos
    // and nextWord() does also => don't loop forever.
    return aRet;
}

SearchResult TextSearch::ApproxSrchBkwrd( const OUString& searchStr,
                                          sal_Int32 startPos, sal_Int32 endPos )
            throw(RuntimeException)
{
    SearchResult aRet;
    aRet.subRegExpressions = 0;

    if( !xBreak.is() )
        return aRet;

    OUString aWTemp( searchStr );

    register sal_Int32 nStt, nEnd;

    Boundary aWBnd = xBreak->getWordBoundary( aWTemp, startPos,
            aSrchPara.Locale,
            WordType::ANYWORD_IGNOREWHITESPACES, sal_True );

    do
    {
        if( aWBnd.endPos <= endPos )
            break;
        nStt = aWBnd.startPos < endPos ? endPos : aWBnd.startPos;
        nEnd = aWBnd.endPos > startPos ? startPos : aWBnd.endPos;

        if( nStt < nEnd &&
                pWLD->WLD( aWTemp.getStr() + nStt, nEnd - nStt ) <= nLimit )
        {
            aRet.subRegExpressions = 1;
            aRet.startOffset.realloc( 1 );
            aRet.startOffset[ 0 ] = nEnd;
            aRet.endOffset.realloc( 1 );
            aRet.endOffset[ 0 ] = nStt;
            break;
        }
        if( !nStt )
            break;

        aWBnd = xBreak->previousWord( aWTemp, nStt, aSrchPara.Locale,
                WordType::ANYWORD_IGNOREWHITESPACES);
    } while( aWBnd.startPos != aWBnd.endPos || aWBnd.endPos != aWTemp.getLength() );
    return aRet;
}


static const sal_Char cSearchName[] = "com.sun.star.util.TextSearch";
static const sal_Char cSearchImpl[] = "com.sun.star.util.TextSearch_i18n";

static OUString getServiceName_Static()
{
    return OUString::createFromAscii( cSearchName );
}

static OUString getImplementationName_Static()
{
    return OUString::createFromAscii( cSearchImpl );
}

OUString SAL_CALL
TextSearch::getImplementationName()
                throw( RuntimeException )
{
    return getImplementationName_Static();
}

sal_Bool SAL_CALL
TextSearch::supportsService(const OUString& rServiceName)
                throw( RuntimeException )
{
    return rServiceName == cSearchName;
}

Sequence< OUString > SAL_CALL
TextSearch::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    aRet[0] = getServiceName_Static();
    return aRet;
}

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
SAL_CALL TextSearch_CreateInstance(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::lang::XMultiServiceFactory >& rxMSF )
{
    return ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XInterface >(
                (::cppu::OWeakObject*) new TextSearch(
                        comphelper::getComponentContext( rxMSF ) ) );
}

extern "C"
{
SAL_DLLPUBLIC_EXPORT void* SAL_CALL
i18nsearch_component_getFactory( const sal_Char* sImplementationName,
                                 void* _pServiceManager,
                                 SAL_UNUSED_PARAMETER void* )
{
    void* pRet = NULL;

    ::com::sun::star::lang::XMultiServiceFactory* pServiceManager =
        reinterpret_cast< ::com::sun::star::lang::XMultiServiceFactory* >
            ( _pServiceManager );
    ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XSingleServiceFactory > xFactory;

    if ( 0 == rtl_str_compare( sImplementationName, cSearchImpl) )
    {
        ::com::sun::star::uno::Sequence< OUString > aServiceNames(1);
        aServiceNames[0] = getServiceName_Static();
        xFactory = ::cppu::createSingleFactory(
                pServiceManager, getImplementationName_Static(),
                &TextSearch_CreateInstance, aServiceNames );
    }

    if ( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }

    return pRet;
}

} // extern "C"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
