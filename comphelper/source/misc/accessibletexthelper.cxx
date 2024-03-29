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

#include <comphelper/accessiblecontexthelper.hxx>
#include <comphelper/accessibletexthelper.hxx>
#include <com/sun/star/accessibility/AccessibleTextType.hpp>
#include <com/sun/star/i18n/BreakIterator.hpp>
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#include <com/sun/star/i18n/CharacterClassification.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/i18n/KCharacterType.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/accessibility/TextSegment.hpp>

#include <algorithm>


namespace comphelper
{


    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::accessibility;


    // OCommonAccessibleText


    OCommonAccessibleText::OCommonAccessibleText()
    {
    }


    OCommonAccessibleText::~OCommonAccessibleText()
    {
    }


    Reference < i18n::XBreakIterator > const & OCommonAccessibleText::implGetBreakIterator()
    {
        if ( !m_xBreakIter.is() )
        {
            Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
            m_xBreakIter = i18n::BreakIterator::create(xContext);
        }

        return m_xBreakIter;
    }


    Reference < i18n::XCharacterClassification > const & OCommonAccessibleText::implGetCharacterClassification()
    {
        if ( !m_xCharClass.is() )
        {
            m_xCharClass = i18n::CharacterClassification::create( ::comphelper::getProcessComponentContext() );
        }

        return m_xCharClass;
    }


    bool OCommonAccessibleText::implIsValidBoundary( i18n::Boundary const & rBoundary, sal_Int32 nLength )
    {
        return ( rBoundary.startPos >= 0 ) && ( rBoundary.startPos < nLength ) && ( rBoundary.endPos >= 0 ) && ( rBoundary.endPos <= nLength );
    }


    bool OCommonAccessibleText::implIsValidIndex( sal_Int32 nIndex, sal_Int32 nLength )
    {
        return ( nIndex >= 0 ) && ( nIndex < nLength );
    }


    bool OCommonAccessibleText::implIsValidRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex, sal_Int32 nLength )
    {
        return ( nStartIndex >= 0 ) && ( nStartIndex <= nLength ) && ( nEndIndex >= 0 ) && ( nEndIndex <= nLength );
    }


    void OCommonAccessibleText::implGetGlyphBoundary( const OUString& rText, i18n::Boundary& rBoundary, sal_Int32 nIndex )
    {
        if ( implIsValidIndex( nIndex, rText.getLength() ) )
        {
            Reference < i18n::XBreakIterator > xBreakIter = implGetBreakIterator();
            if ( xBreakIter.is() )
            {
                sal_Int32 nCount = 1;
                sal_Int32 nDone;
                sal_Int32 nStartIndex = xBreakIter->previousCharacters( rText, nIndex, implGetLocale(), i18n::CharacterIteratorMode::SKIPCELL, nCount, nDone );
                if ( nDone != 0 )
                    nStartIndex = xBreakIter->nextCharacters( rText, nStartIndex, implGetLocale(), i18n::CharacterIteratorMode::SKIPCELL, nCount, nDone );
                sal_Int32 nEndIndex = xBreakIter->nextCharacters( rText, nStartIndex, implGetLocale(), i18n::CharacterIteratorMode::SKIPCELL, nCount, nDone );
                if ( nDone != 0 )
                {
                    rBoundary.startPos = nStartIndex;
                    rBoundary.endPos = nEndIndex;
                }
            }
        }
        else
        {
            rBoundary.startPos = nIndex;
            rBoundary.endPos = nIndex;
        }
    }


    bool OCommonAccessibleText::implGetWordBoundary( const OUString& rText, i18n::Boundary& rBoundary, sal_Int32 nIndex )
    {
        bool bWord = false;

        if ( implIsValidIndex( nIndex, rText.getLength() ) )
        {
            Reference < i18n::XBreakIterator > xBreakIter = implGetBreakIterator();
            if ( xBreakIter.is() )
            {
                rBoundary = xBreakIter->getWordBoundary( rText, nIndex, implGetLocale(), i18n::WordType::ANY_WORD, true );

                // it's a word, if the first character is an alpha-numeric character
                Reference< i18n::XCharacterClassification > xCharClass = implGetCharacterClassification();
                if ( xCharClass.is() )
                {
                    sal_Int32 nType = xCharClass->getCharacterType( rText, rBoundary.startPos, implGetLocale() );
                    if ( ( nType & ( i18n::KCharacterType::LETTER | i18n::KCharacterType::DIGIT ) ) != 0 )
                        bWord = true;
                }
            }
        }
        else
        {
            rBoundary.startPos = nIndex;
            rBoundary.endPos = nIndex;
        }

        return bWord;
    }


    void OCommonAccessibleText::implGetSentenceBoundary( const OUString& rText, i18n::Boundary& rBoundary, sal_Int32 nIndex )
    {
        if ( implIsValidIndex( nIndex, rText.getLength() ) )
        {
            Locale aLocale = implGetLocale();
            Reference < i18n::XBreakIterator > xBreakIter = implGetBreakIterator();
            if ( xBreakIter.is() )
            {
                rBoundary.endPos = xBreakIter->endOfSentence( rText, nIndex, aLocale );
                rBoundary.startPos = xBreakIter->beginOfSentence( rText, rBoundary.endPos, aLocale );
            }
        }
        else
        {
            rBoundary.startPos = nIndex;
            rBoundary.endPos = nIndex;
        }
    }


    void OCommonAccessibleText::implGetParagraphBoundary( const OUString& rText, i18n::Boundary& rBoundary, sal_Int32 nIndex )
    {
        if ( implIsValidIndex( nIndex, rText.getLength() ) )
        {
            rBoundary.startPos = 0;
            rBoundary.endPos = rText.getLength();

            sal_Int32 nFound = rText.lastIndexOf( '\n', nIndex );
            if ( nFound != -1 )
                rBoundary.startPos = nFound + 1;

            nFound = rText.indexOf( '\n', nIndex );
            if ( nFound != -1 )
                rBoundary.endPos = nFound + 1;
        }
        else
        {
            rBoundary.startPos = nIndex;
            rBoundary.endPos = nIndex;
        }
    }


    void OCommonAccessibleText::implGetLineBoundary( const OUString& rText, i18n::Boundary& rBoundary, sal_Int32 nIndex )
    {
        sal_Int32 nLength = rText.getLength();

        if ( implIsValidIndex( nIndex, nLength ) || nIndex == nLength )
        {
            rBoundary.startPos = 0;
            rBoundary.endPos = nLength;
        }
        else
        {
            rBoundary.startPos = nIndex;
            rBoundary.endPos = nIndex;
        }
    }


    sal_Unicode OCommonAccessibleText::implGetCharacter( std::u16string_view rText, sal_Int32 nIndex )
    {
        if ( !implIsValidIndex( nIndex, rText.size() ) )
            throw IndexOutOfBoundsException();

        return rText[nIndex];
    }

    OUString OCommonAccessibleText::getSelectedText()
    {
        OUString sText;
        sal_Int32 nStartIndex;
        sal_Int32 nEndIndex;

        implGetSelection( nStartIndex, nEndIndex );

        try
        {
            sText = implGetTextRange( implGetText(), nStartIndex, nEndIndex );
        }
        catch ( IndexOutOfBoundsException& )
        {
        }

        return sText;
    }


    sal_Int32 OCommonAccessibleText::getSelectionStart()
    {
        sal_Int32 nStartIndex;
        sal_Int32 nEndIndex;

        implGetSelection( nStartIndex, nEndIndex );

        return nStartIndex;
    }


    sal_Int32 OCommonAccessibleText::getSelectionEnd()
    {
        sal_Int32 nStartIndex;
        sal_Int32 nEndIndex;

        implGetSelection( nStartIndex, nEndIndex );

        return nEndIndex;
    }


    OUString OCommonAccessibleText::implGetTextRange( std::u16string_view rText, sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    {

        if ( !implIsValidRange( nStartIndex, nEndIndex, rText.size() ) )
            throw IndexOutOfBoundsException();

        sal_Int32 nMinIndex = std::min( nStartIndex, nEndIndex );
        sal_Int32 nMaxIndex = std::max( nStartIndex, nEndIndex );

        return OUString(rText.substr( nMinIndex, nMaxIndex - nMinIndex ));
    }

    TextSegment OCommonAccessibleText::getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType )
    {
        OUString sText( implGetText() );
        sal_Int32 nLength = sText.getLength();

        if ( !implIsValidIndex( nIndex, nLength ) && nIndex != nLength )
            throw IndexOutOfBoundsException();

        i18n::Boundary aBoundary;
        TextSegment aResult;
        aResult.SegmentStart = -1;
        aResult.SegmentEnd = -1;

        switch ( aTextType )
        {
            case AccessibleTextType::CHARACTER:
            {
                if ( implIsValidIndex( nIndex, nLength ) )
                {
                    auto nIndexEnd = nIndex;
                    sText.iterateCodePoints(&nIndexEnd);

                    aResult.SegmentText = sText.copy( nIndex, nIndexEnd - nIndex );
                    aResult.SegmentStart = nIndex;
                    aResult.SegmentEnd = nIndexEnd;
                }
            }
            break;
            case AccessibleTextType::GLYPH:
            {
                // get glyph at index
                implGetGlyphBoundary( sText, aBoundary, nIndex );
                if ( implIsValidBoundary( aBoundary, nLength ) )
                {
                    aResult.SegmentText = sText.copy( aBoundary.startPos, aBoundary.endPos - aBoundary.startPos );
                    aResult.SegmentStart = aBoundary.startPos;
                    aResult.SegmentEnd = aBoundary.endPos;
                }
            }
            break;
            case AccessibleTextType::WORD:
            {
                // get word at index
                bool bWord = implGetWordBoundary( sText, aBoundary, nIndex );
                if ( bWord && implIsValidBoundary( aBoundary, nLength ) )
                {
                    aResult.SegmentText = sText.copy( aBoundary.startPos, aBoundary.endPos - aBoundary.startPos );
                    aResult.SegmentStart = aBoundary.startPos;
                    aResult.SegmentEnd = aBoundary.endPos;
                }
            }
            break;
            case AccessibleTextType::SENTENCE:
            {
                // get sentence at index
                implGetSentenceBoundary( sText, aBoundary, nIndex );
                if ( implIsValidBoundary( aBoundary, nLength ) )
                {
                    aResult.SegmentText = sText.copy( aBoundary.startPos, aBoundary.endPos - aBoundary.startPos );
                    aResult.SegmentStart = aBoundary.startPos;
                    aResult.SegmentEnd = aBoundary.endPos;
                }
            }
            break;
            case AccessibleTextType::PARAGRAPH:
            {
                // get paragraph at index
                implGetParagraphBoundary( sText, aBoundary, nIndex );
                if ( implIsValidBoundary( aBoundary, nLength ) )
                {
                    aResult.SegmentText = sText.copy( aBoundary.startPos, aBoundary.endPos - aBoundary.startPos );
                    aResult.SegmentStart = aBoundary.startPos;
                    aResult.SegmentEnd = aBoundary.endPos;
                }
            }
            break;
            case AccessibleTextType::LINE:
            {
                // get line at index
                implGetLineBoundary( sText, aBoundary, nIndex );
                if ( implIsValidBoundary( aBoundary, nLength ) )
                {
                    aResult.SegmentText = sText.copy( aBoundary.startPos, aBoundary.endPos - aBoundary.startPos );
                    aResult.SegmentStart = aBoundary.startPos;
                    aResult.SegmentEnd = aBoundary.endPos;
                }
            }
            break;
            case AccessibleTextType::ATTRIBUTE_RUN:
            {
                // TODO: implGetAttributeRunBoundary() (incompatible!)

                aResult.SegmentText = sText;
                aResult.SegmentStart = 0;
                aResult.SegmentEnd = nLength;
            }
            break;
            default:
            {
                // unknown text type
            }
        }

        return aResult;
    }


    TextSegment OCommonAccessibleText::getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType )
    {
        OUString sText( implGetText() );
        sal_Int32 nLength = sText.getLength();

        if ( !implIsValidIndex( nIndex, nLength ) && nIndex != nLength )
            throw IndexOutOfBoundsException();

        i18n::Boundary aBoundary;
        TextSegment aResult;
        aResult.SegmentStart = -1;
        aResult.SegmentEnd = -1;

        switch ( aTextType )
        {
            case AccessibleTextType::CHARACTER:
            {
                if ( implIsValidIndex( nIndex - 1, nLength ) )
                {
                    sText.iterateCodePoints(&nIndex, -1);
                    auto nIndexEnd = nIndex;
                    sText.iterateCodePoints(&nIndexEnd);
                    aResult.SegmentText = sText.copy(nIndex, nIndexEnd - nIndex);
                    aResult.SegmentStart = nIndex;
                    aResult.SegmentEnd = nIndexEnd;
                }
            }
            break;
            case AccessibleTextType::GLYPH:
            {
                // get glyph at index
                implGetGlyphBoundary( sText, aBoundary, nIndex );
                // get previous glyph
                if ( aBoundary.startPos > 0 )
                {
                    implGetGlyphBoundary( sText, aBoundary, aBoundary.startPos - 1 );
                    if ( implIsValidBoundary( aBoundary, nLength ) )
                    {
                        aResult.SegmentText = sText.copy( aBoundary.startPos, aBoundary.endPos - aBoundary.startPos );
                        aResult.SegmentStart = aBoundary.startPos;
                        aResult.SegmentEnd = aBoundary.endPos;
                    }
                }
            }
            break;
            case AccessibleTextType::WORD:
            {
                // get word at index
                implGetWordBoundary( sText, aBoundary, nIndex );
                // get previous word
                bool bWord = false;
                while ( !bWord && aBoundary.startPos > 0 )
                    bWord = implGetWordBoundary( sText, aBoundary, aBoundary.startPos - 1 );
                if ( bWord && implIsValidBoundary( aBoundary, nLength ) )
                {
                    aResult.SegmentText = sText.copy( aBoundary.startPos, aBoundary.endPos - aBoundary.startPos );
                    aResult.SegmentStart = aBoundary.startPos;
                    aResult.SegmentEnd = aBoundary.endPos;
                }
            }
            break;
            case AccessibleTextType::SENTENCE:
            {
                // get sentence at index
                implGetSentenceBoundary( sText, aBoundary, nIndex );
                // get previous sentence
                if ( aBoundary.startPos > 0 )
                {
                    implGetSentenceBoundary( sText, aBoundary, aBoundary.startPos - 1 );
                    if ( implIsValidBoundary( aBoundary, nLength ) )
                    {
                        aResult.SegmentText = sText.copy( aBoundary.startPos, aBoundary.endPos - aBoundary.startPos );
                        aResult.SegmentStart = aBoundary.startPos;
                        aResult.SegmentEnd = aBoundary.endPos;
                    }
                }
            }
            break;
            case AccessibleTextType::PARAGRAPH:
            {
                // get paragraph at index
                implGetParagraphBoundary( sText, aBoundary, nIndex );
                // get previous paragraph
                if ( aBoundary.startPos > 0 )
                {
                    implGetParagraphBoundary( sText, aBoundary, aBoundary.startPos - 1 );
                    if ( implIsValidBoundary( aBoundary, nLength ) )
                    {
                        aResult.SegmentText = sText.copy( aBoundary.startPos, aBoundary.endPos - aBoundary.startPos );
                        aResult.SegmentStart = aBoundary.startPos;
                        aResult.SegmentEnd = aBoundary.endPos;
                    }
                }
            }
            break;
            case AccessibleTextType::LINE:
            {
                // get line at index
                implGetLineBoundary( sText, aBoundary, nIndex );
                // get previous line
                if ( aBoundary.startPos > 0 )
                {
                    implGetLineBoundary( sText, aBoundary, aBoundary.startPos - 1 );
                    if ( implIsValidBoundary( aBoundary, nLength ) )
                    {
                        aResult.SegmentText = sText.copy( aBoundary.startPos, aBoundary.endPos - aBoundary.startPos );
                        aResult.SegmentStart = aBoundary.startPos;
                        aResult.SegmentEnd = aBoundary.endPos;
                    }
                }
            }
            break;
            case AccessibleTextType::ATTRIBUTE_RUN:
            {
                // TODO: implGetAttributeRunBoundary() (incompatible!)
            }
            break;
            default:
            {
                // unknown text type
            }
        }

        return aResult;
    }


    TextSegment OCommonAccessibleText::getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType )
    {
        OUString sText( implGetText() );
        sal_Int32 nLength = sText.getLength();

        if ( !implIsValidIndex( nIndex, nLength ) && nIndex != nLength )
            throw IndexOutOfBoundsException();

        i18n::Boundary aBoundary;
        TextSegment aResult;
        aResult.SegmentStart = -1;
        aResult.SegmentEnd = -1;

        switch ( aTextType )
        {
            case AccessibleTextType::CHARACTER:
            {
                if ( implIsValidIndex( nIndex + 1, nLength ) )
                {
                    sText.iterateCodePoints(&nIndex);
                    auto nIndexEnd = nIndex;
                    sText.iterateCodePoints(&nIndexEnd);
                    aResult.SegmentText = sText.copy(nIndex, nIndexEnd - nIndex);
                    aResult.SegmentStart = nIndex;
                    aResult.SegmentEnd = nIndexEnd;
                }
            }
            break;
            case AccessibleTextType::GLYPH:
            {
                // get glyph at index
                implGetGlyphBoundary( sText, aBoundary, nIndex );
                // get next glyph
                if ( aBoundary.endPos < nLength )
                {
                    implGetGlyphBoundary( sText, aBoundary, aBoundary.endPos );
                    if ( implIsValidBoundary( aBoundary, nLength ) )
                    {
                        aResult.SegmentText = sText.copy( aBoundary.startPos, aBoundary.endPos - aBoundary.startPos );
                        aResult.SegmentStart = aBoundary.startPos;
                        aResult.SegmentEnd = aBoundary.endPos;
                    }
                }
            }
            break;
            case AccessibleTextType::WORD:
            {
                // get word at index
                implGetWordBoundary( sText, aBoundary, nIndex );
                // get next word
                bool bWord = false;
                while ( !bWord && aBoundary.endPos < nLength )
                    bWord = implGetWordBoundary( sText, aBoundary, aBoundary.endPos );
                if ( bWord && implIsValidBoundary( aBoundary, nLength ) )
                {
                    aResult.SegmentText = sText.copy( aBoundary.startPos, aBoundary.endPos - aBoundary.startPos );
                    aResult.SegmentStart = aBoundary.startPos;
                    aResult.SegmentEnd = aBoundary.endPos;
                }
            }
            break;
            case AccessibleTextType::SENTENCE:
            {
                // get sentence at index
                implGetSentenceBoundary( sText, aBoundary, nIndex );
                // get next sentence
                sal_Int32 nEnd = aBoundary.endPos;
                sal_Int32 nI = aBoundary.endPos;
                bool bFound = false;
                while ( !bFound && ++nI < nLength )
                {
                    implGetSentenceBoundary( sText, aBoundary, nI );
                    bFound = ( aBoundary.endPos > nEnd );
                }
                if ( bFound && implIsValidBoundary( aBoundary, nLength ) )
                {
                    aResult.SegmentText = sText.copy( aBoundary.startPos, aBoundary.endPos - aBoundary.startPos );
                    aResult.SegmentStart = aBoundary.startPos;
                    aResult.SegmentEnd = aBoundary.endPos;
                }
            }
            break;
            case AccessibleTextType::PARAGRAPH:
            {
                // get paragraph at index
                implGetParagraphBoundary( sText, aBoundary, nIndex );
                // get next paragraph
                if ( aBoundary.endPos < nLength )
                {
                    implGetParagraphBoundary( sText, aBoundary, aBoundary.endPos );
                    if ( implIsValidBoundary( aBoundary, nLength ) )
                    {
                        aResult.SegmentText = sText.copy( aBoundary.startPos, aBoundary.endPos - aBoundary.startPos );
                        aResult.SegmentStart = aBoundary.startPos;
                        aResult.SegmentEnd = aBoundary.endPos;
                    }
                }
            }
            break;
            case AccessibleTextType::LINE:
            {
                // get line at index
                implGetLineBoundary( sText, aBoundary, nIndex );
                // get next line
                if ( aBoundary.endPos < nLength )
                {
                    implGetLineBoundary( sText, aBoundary, aBoundary.endPos );
                    if ( implIsValidBoundary( aBoundary, nLength ) )
                    {
                        aResult.SegmentText = sText.copy( aBoundary.startPos, aBoundary.endPos - aBoundary.startPos );
                        aResult.SegmentStart = aBoundary.startPos;
                        aResult.SegmentEnd = aBoundary.endPos;
                    }
                }
            }
            break;
            case AccessibleTextType::ATTRIBUTE_RUN:
            {
                // TODO: implGetAttributeRunBoundary() (incompatible!)
            }
            break;
            default:
            {
                // unknown text type
            }
        }

        return aResult;
    }


    bool OCommonAccessibleText::implInitTextChangedEvent(
        std::u16string_view rOldString,
        std::u16string_view rNewString,
        css::uno::Any& rDeleted,
        css::uno::Any& rInserted) // throw()
    {
        size_t nLenOld = rOldString.size();
        size_t nLenNew = rNewString.size();

        // equal
        if ((0 == nLenOld) && (0 == nLenNew))
            return false;

        TextSegment aDeletedText;
        TextSegment aInsertedText;

        aDeletedText.SegmentStart = -1;
        aDeletedText.SegmentEnd = -1;
        aInsertedText.SegmentStart = -1;
        aInsertedText.SegmentEnd = -1;

        // insert only
        if ((0 == nLenOld) && (nLenNew > 0))
        {
            aInsertedText.SegmentStart = 0;
            aInsertedText.SegmentEnd = nLenNew;
            aInsertedText.SegmentText = rNewString.substr( aInsertedText.SegmentStart, aInsertedText.SegmentEnd - aInsertedText.SegmentStart );

            rInserted <<= aInsertedText;
            return true;
        }

        // delete only
        if ((nLenOld > 0) && (0 == nLenNew))
        {
            aDeletedText.SegmentStart = 0;
            aDeletedText.SegmentEnd = nLenOld;
            aDeletedText.SegmentText = rOldString.substr( aDeletedText.SegmentStart, aDeletedText.SegmentEnd - aDeletedText.SegmentStart );

            rDeleted <<= aDeletedText;
            return true;
        }

        auto pFirstDiffOld = rOldString.begin();
        auto pLastDiffOld  = rOldString.end();
        auto pFirstDiffNew = rNewString.begin();
        auto pLastDiffNew  = rNewString.end();

        // find first difference
        while ((pFirstDiffOld < pLastDiffOld) && (pFirstDiffNew < pLastDiffNew)
               && (*pFirstDiffOld == *pFirstDiffNew))
        {
            pFirstDiffOld++;
            pFirstDiffNew++;
        }

        // equality test
        if (pFirstDiffOld == pLastDiffOld && pFirstDiffNew == pLastDiffNew)
            return false;

        // find last difference
        while ( ( pLastDiffOld > pFirstDiffOld) &&
                ( pLastDiffNew > pFirstDiffNew) &&
                (pLastDiffOld[-1]  == pLastDiffNew[-1]))
        {
            pLastDiffOld--;
            pLastDiffNew--;
        }

        if (pFirstDiffOld < pLastDiffOld)
        {
            aDeletedText.SegmentStart = pFirstDiffOld - rOldString.begin();
            aDeletedText.SegmentEnd = pLastDiffOld  - rOldString.begin();
            aDeletedText.SegmentText = rOldString.substr( aDeletedText.SegmentStart, aDeletedText.SegmentEnd - aDeletedText.SegmentStart );

            rDeleted <<= aDeletedText;
        }

        if (pFirstDiffNew < pLastDiffNew)
        {
            aInsertedText.SegmentStart = pFirstDiffNew - rNewString.begin();
            aInsertedText.SegmentEnd = pLastDiffNew  - rNewString.begin();
            aInsertedText.SegmentText = rNewString.substr( aInsertedText.SegmentStart, aInsertedText.SegmentEnd - aInsertedText.SegmentStart );

            rInserted <<= aInsertedText;
        }
        return true;
    }


    // OAccessibleTextHelper


    OAccessibleTextHelper::OAccessibleTextHelper( )
    {
    }


    // XAccessibleText


    OUString OAccessibleTextHelper::getSelectedText()
    {
        OExternalLockGuard aGuard( this );

        return OCommonAccessibleText::getSelectedText();
    }


    sal_Int32 OAccessibleTextHelper::getSelectionStart()
    {
        OExternalLockGuard aGuard( this );

        return OCommonAccessibleText::getSelectionStart();
    }


    sal_Int32 OAccessibleTextHelper::getSelectionEnd()
    {
        OExternalLockGuard aGuard( this );

        return OCommonAccessibleText::getSelectionEnd();
    }


    TextSegment OAccessibleTextHelper::getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType )
    {
        OExternalLockGuard aGuard( this );

        return OCommonAccessibleText::getTextAtIndex( nIndex, aTextType );
    }


    TextSegment OAccessibleTextHelper::getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType )
    {
        OExternalLockGuard aGuard( this );

        return OCommonAccessibleText::getTextBeforeIndex( nIndex, aTextType );
    }


    TextSegment OAccessibleTextHelper::getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType )
    {
        OExternalLockGuard aGuard( this );

        return OCommonAccessibleText::getTextBehindIndex( nIndex, aTextType );
    }


}   // namespace comphelper


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
