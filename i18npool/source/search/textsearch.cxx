/*************************************************************************
 *
 *  $RCSfile: textsearch.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-08 16:01:00 $
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


#include "textsearch.hxx"
#include "levdis.hxx"
#include "../regexp/exprclas.hxx"

#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_UNICODETYPE_HPP_
#include <com/sun/star/i18n/UnicodeType.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_SEARCHFLAGS_HPP_
#include <com/sun/star/util/SearchFlags.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_WORDTYPE_HPP_
#include <com/sun/star/i18n/WordType.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HPP_
#include <com/sun/star/i18n/ScriptType.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_CHARACTERITERATORMODE_HPP_
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_KCHARACTERTYPE_HPP_
#include <com/sun/star/i18n/KCharacterType.hpp>
#endif

#ifdef _MSC_VER
// get rid of that dumb compiler warning
// identifier was truncated to '255' characters in the debug information
// for STL template usage, if .pdb files are to be created
#pragma warning( disable: 4786 )
#endif

using namespace ::com::sun::star::util;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::i18n;
using namespace ::drafts::com::sun::star::i18n;
using namespace ::rtl;

TextSearch::TextSearch(const Reference < XMultiServiceFactory > & rxMSF)
        : pRegExp( 0 )
        , pWLD( 0 )
        , pJumpTable( 0 )
        , xMSF( rxMSF )
{
    SearchOptions aOpt;
    aOpt.algorithmType = SearchAlgorithms_ABSOLUTE;
    aOpt.searchFlag = SearchFlags::ALL_IGNORE_CASE;
    //aOpt.Locale = ???;
    setOptions( aOpt );
}

TextSearch::~TextSearch()
{
    delete pRegExp;
    delete pWLD;
    delete pJumpTable;
}

void TextSearch::setOptions( const SearchOptions& rOptions ) throw( RuntimeException )
{
    aSrchPara = rOptions;

    delete pRegExp, pRegExp = 0;
    delete pWLD, pWLD = 0;
    delete pJumpTable, pJumpTable = 0;

    // Create Transliteration class
    if( aSrchPara.transliterateFlags )
    {
        if( !xTranslit.is() )
        {
            Reference < XInterface > xI = xMSF->createInstance(
                    OUString::createFromAscii(
                        "com.sun.star.i18n.Transliteration"));
            if ( xI.is() )
                xI->queryInterface( ::getCppuType(
                            (const Reference< XExtendedTransliteration >*)0))
                    >>= xTranslit;
        }

        // Load transliteration module
        if( xTranslit.is() )
            xTranslit->loadModule(
                    (TransliterationModules)aSrchPara.transliterateFlags,
                    aSrchPara.Locale);
    }
    else if( xTranslit.is() )
        xTranslit = 0;

    if ( !xBreak.is() )
    {
        Reference < XInterface > xI = xMSF->createInstance(
                OUString::createFromAscii( "com.sun.star.i18n.BreakIterator"));
        if( xI.is() )
            xI->queryInterface( ::getCppuType(
                        (const Reference< XBreakIterator >*)0))
                >>= xBreak;
    }

    sSrchStr = aSrchPara.searchString;

    // use transliteration here, but only if not RegEx, which does it different
    if ( aSrchPara.algorithmType != SearchAlgorithms_REGEXP && xTranslit.is() &&
            aSrchPara.transliterateFlags )
        sSrchStr = xTranslit->transliterateString2String(
                aSrchPara.searchString, 0, aSrchPara.searchString.getLength());

    // When start or end of search string is a complex script type, we need to
    // make sure the result boundary is not located in the middle of cell.
    checkCTLStart = (xBreak.is() && (xBreak->getScriptType(sSrchStr, 0) ==
                ScriptType::COMPLEX));
    checkCTLEnd = (xBreak.is() && (xBreak->getScriptType(sSrchStr,
                    sSrchStr.getLength()-1) == ScriptType::COMPLEX));

    if ( aSrchPara.algorithmType == SearchAlgorithms_REGEXP )
    {
        fnForward = &TextSearch::RESrchFrwrd;
        fnBackward = &TextSearch::RESrchBkwrd;

        pRegExp = new Regexpr( aSrchPara, xTranslit );
    }
    else
    {
        if ( aSrchPara.algorithmType == SearchAlgorithms_APPROXIMATE )
        {
            fnForward = &TextSearch::ApproxSrchFrwrd;
            fnBackward = &TextSearch::ApproxSrchBkwrd;

            pWLD = new WLevDistance( sSrchStr.getStr(), aSrchPara.changedChars,
                    aSrchPara.insertedChars, aSrchPara.deletedChars,
                    0 != (SearchFlags::LEV_RELAXED & aSrchPara.searchFlag ) );

            nLimit = pWLD->GetLimit();
        }
        else
        {
            fnForward = &TextSearch::NSrchFrwrd;
            fnBackward = &TextSearch::NSrchBkwrd;
        }
    }
}

sal_Int32 FindPosInSeq_Impl( const Sequence <long>& rOff, sal_Int32 nPos )
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

    if ( xTranslit.is() )
    {
        com::sun::star::uno::Sequence <sal_Int32> offset( in_str.getLength());

        in_str = xTranslit->transliterate( searchStr, 0, in_str.getLength(), offset);

        // JP 20.6.2001: also the start and end positions must be corrected!
        if( startPos )
            startPos = FindPosInSeq_Impl( offset, startPos );

        if( endPos < searchStr.getLength() )
            endPos = FindPosInSeq_Impl( offset, endPos );
        else
            endPos = in_str.getLength();

        sres = (this->*fnForward)( in_str, startPos, endPos );

        for ( int k = 0; k < sres.startOffset.getLength(); k++ )
        {
            if (sres.startOffset[k])
                sres.startOffset[k] = offset[sres.startOffset[k]-1] + 1;
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

    return sres;
}

SearchResult TextSearch::searchBackward( const OUString& searchStr, sal_Int32 startPos, sal_Int32 endPos )
        throw(RuntimeException)
{
    SearchResult sres;

    OUString in_str(searchStr);

    if ( xTranslit.is() )
    {
        com::sun::star::uno::Sequence <sal_Int32> offset( in_str.getLength());

        in_str = xTranslit->transliterate(searchStr, 0, in_str.getLength(), offset);

        // JP 20.6.2001: the start and end positions must be corrected too!
        if( startPos < searchStr.getLength() )
            startPos = FindPosInSeq_Impl( offset, startPos );
        else
            startPos = in_str.getLength();

        if( endPos )
            endPos = FindPosInSeq_Impl( offset, endPos );

        sres = (this->*fnBackward)( in_str, startPos, endPos );

        for( int k = 0; k < sres.startOffset.getLength(); k++ )
        {
            // JP 20.6.2001: start is ever exclusive and then don't return
            //               the position of the prev character - return the
            //               prev position before the first found character!
            //               "a b c" find "b" must return 3,2 and not 4,2!!!
            if (sres.startOffset[k])
                sres.startOffset[k] = offset[sres.startOffset[k]-1]+1;
            if (sres.endOffset[k])
                sres.endOffset[k] = offset[sres.endOffset[k]-1]+1;
        }
    }
    else
    {
        sres = (this->*fnBackward)( in_str, startPos, endPos );
    }

    return sres;
}



//--------------- die Wort-Trennner ----------------------------------

bool TextSearch::IsDelimiter( const OUString& rStr, sal_Int32 nPos ) const
{
    bool bRet = 1;
    if( '\x7f' != rStr[nPos])
    {
        if ( !xCharClass.is() )
        {
            Reference < XInterface > xI = xMSF->createInstance(
                    OUString::createFromAscii( "com.sun.star.i18n.CharacterClassification"));
            if( xI.is() )
                xI->queryInterface( ::getCppuType(
                            (const Reference< XCharacterClassification >*)0))
                    >>= xCharClass;
        }
        if ( xCharClass.is() )
        {
            sal_Int32 nCType = xCharClass->getCharacterType( rStr, nPos,
                    aSrchPara.Locale );
            if( 0 != (( KCharacterType::DIGIT | KCharacterType::ALPHA |
                            KCharacterType::LETTER ) & nCType ) )
                bRet = 0;
        }
    }
    return bRet;
}



// --------- methods for the kind of boyer-morre search ------------------

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

sal_Int32 TextSearch::GetDiff( const sal_Unicode cChr ) const
{
    TextSearchJumpTable::const_iterator iLook = pJumpTable->find( cChr );
    if ( iLook == pJumpTable->end() )
        return sSrchStr.getLength();
    return (*iLook).second;
}


SearchResult TextSearch::NSrchFrwrd( const OUString& searchStr, sal_Int32 startPos, sal_Int32 endPos )
        throw(RuntimeException)
{
    SearchResult aRet;
    aRet.subRegExpressions = 0;

    OUString aStr( searchStr );
    sal_Int32 nSuchIdx = aStr.getLength();
    sal_Int32 nEnde = endPos;
    if( !nSuchIdx || !sSrchStr.getLength() || sSrchStr.getLength() > nSuchIdx )
        return aRet;


    if( nEnde < sSrchStr.getLength() )  // position inside the search region ?
        return aRet;

    nEnde -= sSrchStr.getLength();

    MakeForwardTab();                   // create the jumptable

    for (sal_Int32 nCmpIdx = startPos; // start position for the search
            nCmpIdx <= nEnde;
            nCmpIdx += GetDiff( aStr[nCmpIdx + sSrchStr.getLength()-1]))
    {
        // if the match would be the completed cells, skip it.
        if ( (checkCTLStart && !isCellStart( aStr, nCmpIdx )) || (checkCTLEnd
                    && !isCellStart( aStr, nCmpIdx + sSrchStr.getLength())) )
            continue;

        nSuchIdx = sSrchStr.getLength() - 1;
        while( nSuchIdx >= 0 && sSrchStr[nSuchIdx] == aStr[nCmpIdx + nSuchIdx])
        {
            if( nSuchIdx == 0 )
            {
                if( SearchFlags::NORM_WORD_ONLY & aSrchPara.searchFlag )
                {
                    sal_Int32 nFndEnd = nCmpIdx + sSrchStr.getLength();
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
                aRet.endOffset[ 0 ] = nCmpIdx + sSrchStr.getLength();
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

    OUString aStr( searchStr );
    sal_Int32 nSuchIdx = aStr.getLength();
    sal_Int32 nEnde = endPos;
    if( nSuchIdx == 0 || sSrchStr.getLength() == 0 || sSrchStr.getLength() > nSuchIdx)
        return aRet;

    MakeBackwardTab();                      // create the jumptable

    if( nEnde == nSuchIdx )                 // end position for the search
        nEnde = sSrchStr.getLength();
    else
        nEnde += sSrchStr.getLength();

    sal_Int32 nCmpIdx = startPos;          // start position for the search

    while (nCmpIdx >= nEnde)
    {
        // if the match would be the completed cells, skip it.
        if ( (!checkCTLStart || isCellStart( aStr, nCmpIdx -
                        sSrchStr.getLength() )) && (!checkCTLEnd ||
                    isCellStart( aStr, nCmpIdx)))
        {
            nSuchIdx = 0;
            while( nSuchIdx < sSrchStr.getLength() && sSrchStr[nSuchIdx] ==
                    aStr[nCmpIdx + nSuchIdx - sSrchStr.getLength()] )
                nSuchIdx++;
            if( nSuchIdx >= sSrchStr.getLength() )
            {
                if( SearchFlags::NORM_WORD_ONLY & aSrchPara.searchFlag )
                {
                    sal_Int32 nFndStt = nCmpIdx - sSrchStr.getLength();
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
                        aRet.endOffset[ 0 ] = nCmpIdx - sSrchStr.getLength();
                        return aRet;
                    }
                }
                else
                {
                    aRet.subRegExpressions = 1;
                    aRet.startOffset.realloc( 1 );
                    aRet.startOffset[ 0 ] = nCmpIdx;
                    aRet.endOffset.realloc( 1 );
                    aRet.endOffset[ 0 ] = nCmpIdx - sSrchStr.getLength();
                    return aRet;
                }
            }
        }
        nSuchIdx = GetDiff( aStr[nCmpIdx - sSrchStr.getLength()] );
        if( nCmpIdx < nSuchIdx )
            return aRet;
        nCmpIdx -= nSuchIdx;
    }
    return aRet;
}



//---------------------------------------------------------------------------
// ------- Methoden fuer die Suche ueber Regular-Expressions --------------

SearchResult TextSearch::RESrchFrwrd( const OUString& searchStr,
                                      sal_Int32 startPos, sal_Int32 endPos )
            throw(RuntimeException)
{
    SearchResult aRet;
    sal_Int32 nOffset = 0;
    aRet.subRegExpressions = 0;
    OUString aStr( searchStr );

    bool bSearchInSel = (0 != (( SearchFlags::REG_NOT_BEGINOFLINE |
                    SearchFlags::REG_NOT_ENDOFLINE ) & aSrchPara.searchFlag ));

    // search only in the subString
    if (bSearchInSel)
        aStr = aStr.copy(startPos, endPos - startPos);

    if ( !bSearchInSel && startPos )
        nOffset = startPos;

    pRegExp->set_line(aStr.getStr(), aStr.getLength());

    struct re_registers regs;

    // Clear structure
    memset((void *)&regs, 0, sizeof(struct re_registers));
    if ( ! pRegExp->re_search(&regs, nOffset) )
    {
        if( regs.num_of_match > 0 &&
                (regs.start[0] != -1 && regs.end[0] != -1) )
        {
            aRet.subRegExpressions = 1;
            aRet.startOffset.realloc(1);
            aRet.endOffset.realloc(1);

            nOffset = bSearchInSel ? startPos : 0;
            aRet.startOffset[0] = regs.start[0] + nOffset;
            aRet.endOffset[0] = regs.end[0] + nOffset;
        }
        if ( regs.num_regs > 0 )
        {
            if ( regs.start )
                free(regs.start);
            if ( regs.end )
                free(regs.end);
        }
    }

    return aRet;
}

/*
 * Sucht das Muster aSrchPara.sSrchStr rueckwaerts im String rStr
 */
SearchResult TextSearch::RESrchBkwrd( const OUString& searchStr,
                                      sal_Int32 startPos, sal_Int32 endPos )
            throw(RuntimeException)
{
    SearchResult aRet;
    aRet.subRegExpressions = 0;
    OUString aStr( searchStr );

    sal_Int32 nOffset = 0;
    sal_Int32 nStrEnde = aStr.getLength() == endPos ? 0 : endPos;

    bool bSearchInSel = (0 != (( SearchFlags::REG_NOT_BEGINOFLINE |
                    SearchFlags::REG_NOT_ENDOFLINE ) & aSrchPara.searchFlag ));

    // search only in the subString
    if( bSearchInSel )
        aStr = aStr.copy( nStrEnde, startPos - nStrEnde );

    if( startPos )
        nOffset = startPos - 1;

    // set the length to negative for reverse search
    pRegExp->set_line( aStr.getStr(), -(aStr.getLength()) );
    struct re_registers regs;

    // Clear structure
    memset((void *)&regs, 0, sizeof(struct re_registers));
    if ( ! pRegExp->re_search(&regs, nOffset) )
    {
        if( regs.num_of_match > 0 &&
                (regs.start[0] != -1 && regs.end[0] != -1) )
        {
            aRet.subRegExpressions = 1;
            aRet.startOffset.realloc(1);
            aRet.endOffset.realloc(1);
            //      aRet.startOffset[0] = regs.start[0];
            //      aRet.endOffset[0] = regs.end[0];

            nOffset = bSearchInSel ? nStrEnde : 0;
            aRet.startOffset[0] = regs.end[0] + nOffset;
            aRet.endOffset[0] = regs.start[0] + nOffset;
        }
        if ( regs.num_regs > 0 )
        {
            if ( regs.start )
                free(regs.start);
            if ( regs.end )
                free(regs.end);
        }
    }

    return aRet;
}

// Phonetische Suche von Worten
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
    } while( aWBnd.startPos != aWBnd.endPos || aWBnd.endPos != aWTemp.getLength() );
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

OUString SAL_CALL
TextSearch::getImplementationName(void)
                throw( RuntimeException )
{
    return OUString::createFromAscii("com.sun.star.i18n.TextSearch");
}

const sal_Char cSearch[] = "com.sun.star.i18n.TextSearch";

sal_Bool SAL_CALL
TextSearch::supportsService(const OUString& rServiceName)
                throw( RuntimeException )
{
    return !rServiceName.compareToAscii(cSearch);
}

Sequence< OUString > SAL_CALL
TextSearch::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString::createFromAscii(cSearch);
    return aRet;
}
