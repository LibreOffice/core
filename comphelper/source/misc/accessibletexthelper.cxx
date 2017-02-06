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
    using namespace ::com::sun::star::beans;
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


    bool OCommonAccessibleText::implIsValidBoundary( i18n::Boundary& rBoundary, sal_Int32 nLength )
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


    void OCommonAccessibleText::implGetGlyphBoundary( i18n::Boundary& rBoundary, sal_Int32 nIndex )
    {
        OUString sText( implGetText() );

        if ( implIsValidIndex( nIndex, sText.getLength() ) )
        {
            Reference < i18n::XBreakIterator > xBreakIter = implGetBreakIterator();
            if ( xBreakIter.is() )
            {
                sal_Int32 nCount = 1;
                sal_Int32 nDone;
                sal_Int32 nStartIndex = xBreakIter->previousCharacters( sText, nIndex, implGetLocale(), i18n::CharacterIteratorMode::SKIPCELL, nCount, nDone );
                if ( nDone != 0 )
                    nStartIndex = xBreakIter->nextCharacters( sText, nStartIndex, implGetLocale(), i18n::CharacterIteratorMode::SKIPCELL, nCount, nDone );
                sal_Int32 nEndIndex = xBreakIter->nextCharacters( sText, nStartIndex, implGetLocale(), i18n::CharacterIteratorMode::SKIPCELL, nCount, nDone );
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


    bool OCommonAccessibleText::implGetWordBoundary( i18n::Boundary& rBoundary, sal_Int32 nIndex )
    {
        bool bWord = false;
        OUString sText( implGetText() );

        if ( implIsValidIndex( nIndex, sText.getLength() ) )
        {
            Reference < i18n::XBreakIterator > xBreakIter = implGetBreakIterator();
            if ( xBreakIter.is() )
            {
                rBoundary = xBreakIter->getWordBoundary( sText, nIndex, implGetLocale(), i18n::WordType::ANY_WORD, true );

                // it's a word, if the first character is an alpha-numeric character
                Reference< i18n::XCharacterClassification > xCharClass = implGetCharacterClassification();
                if ( xCharClass.is() )
                {
                    sal_Int32 nType = xCharClass->getCharacterType( sText, rBoundary.startPos, implGetLocale() );
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


    void OCommonAccessibleText::implGetSentenceBoundary( i18n::Boundary& rBoundary, sal_Int32 nIndex )
    {
        OUString sText( implGetText() );

        if ( implIsValidIndex( nIndex, sText.getLength() ) )
        {
            Locale aLocale = implGetLocale();
            Reference < i18n::XBreakIterator > xBreakIter = implGetBreakIterator();
            if ( xBreakIter.is() )
            {
                rBoundary.endPos = xBreakIter->endOfSentence( sText, nIndex, aLocale );
                rBoundary.startPos = xBreakIter->beginOfSentence( sText, rBoundary.endPos, aLocale );
            }
        }
        else
        {
            rBoundary.startPos = nIndex;
            rBoundary.endPos = nIndex;
        }
    }


    void OCommonAccessibleText::implGetParagraphBoundary( i18n::Boundary& rBoundary, sal_Int32 nIndex )
    {
        OUString sText( implGetText() );

        if ( implIsValidIndex( nIndex, sText.getLength() ) )
        {
            rBoundary.startPos = 0;
            rBoundary.endPos = sText.getLength();

            sal_Int32 nFound = sText.lastIndexOf( (sal_Unicode)'\n', nIndex );
            if ( nFound != -1 )
                rBoundary.startPos = nFound + 1;

            nFound = sText.indexOf( (sal_Unicode)'\n', nIndex );
            if ( nFound != -1 )
                rBoundary.endPos = nFound + 1;
        }
        else
        {
            rBoundary.startPos = nIndex;
            rBoundary.endPos = nIndex;
        }
    }


    void OCommonAccessibleText::implGetLineBoundary( i18n::Boundary& rBoundary, sal_Int32 nIndex )
    {
        OUString sText( implGetText() );
        sal_Int32 nLength = sText.getLength();

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


    sal_Unicode OCommonAccessibleText::getCharacter( sal_Int32 nIndex )
    {
        OUString sText( implGetText() );

        if ( !implIsValidIndex( nIndex, sText.getLength() ) )
            throw IndexOutOfBoundsException();

        return sText[nIndex];
    }


    sal_Int32 OCommonAccessibleText::getCharacterCount()
    {
        return implGetText().getLength();
    }


    OUString OCommonAccessibleText::getSelectedText()
    {
        OUString sText;
        sal_Int32 nStartIndex;
        sal_Int32 nEndIndex;

        implGetSelection( nStartIndex, nEndIndex );

        try
        {
            sText = getTextRange( nStartIndex, nEndIndex );
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


    OUString OCommonAccessibleText::getText()
    {
        return implGetText();
    }


    OUString OCommonAccessibleText::getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    {
        OUString sText( implGetText() );

        if ( !implIsValidRange( nStartIndex, nEndIndex, sText.getLength() ) )
            throw IndexOutOfBoundsException();

        sal_Int32 nMinIndex = ::std::min( nStartIndex, nEndIndex );
        sal_Int32 nMaxIndex = ::std::max( nStartIndex, nEndIndex );

        return sText.copy( nMinIndex, nMaxIndex - nMinIndex );
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
                    aResult.SegmentText = sText.copy( nIndex, 1 );
                    aResult.SegmentStart = nIndex;
                    aResult.SegmentEnd = nIndex+1;
                }
            }
            break;
            case AccessibleTextType::GLYPH:
            {
                // get glyph at index
                implGetGlyphBoundary( aBoundary, nIndex );
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
                bool bWord = implGetWordBoundary( aBoundary, nIndex );
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
                implGetSentenceBoundary( aBoundary, nIndex );
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
                implGetParagraphBoundary( aBoundary, nIndex );
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
                implGetLineBoundary( aBoundary, nIndex );
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
                    aResult.SegmentText = sText.copy( nIndex - 1, 1 );
                    aResult.SegmentStart = nIndex-1;
                    aResult.SegmentEnd = nIndex;
                }
            }
            break;
            case AccessibleTextType::GLYPH:
            {
                // get glyph at index
                implGetGlyphBoundary( aBoundary, nIndex );
                // get previous glyph
                if ( aBoundary.startPos > 0 )
                {
                    implGetGlyphBoundary( aBoundary, aBoundary.startPos - 1 );
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
                implGetWordBoundary( aBoundary, nIndex );
                // get previous word
                bool bWord = false;
                while ( !bWord && aBoundary.startPos > 0 )
                    bWord = implGetWordBoundary( aBoundary, aBoundary.startPos - 1 );
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
                implGetSentenceBoundary( aBoundary, nIndex );
                // get previous sentence
                if ( aBoundary.startPos > 0 )
                {
                    implGetSentenceBoundary( aBoundary, aBoundary.startPos - 1 );
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
                implGetParagraphBoundary( aBoundary, nIndex );
                // get previous paragraph
                if ( aBoundary.startPos > 0 )
                {
                    implGetParagraphBoundary( aBoundary, aBoundary.startPos - 1 );
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
                implGetLineBoundary( aBoundary, nIndex );
                // get previous line
                if ( aBoundary.startPos > 0 )
                {
                    implGetLineBoundary( aBoundary, aBoundary.startPos - 1 );
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
                    aResult.SegmentText = sText.copy( nIndex + 1, 1 );
                    aResult.SegmentStart = nIndex+1;
                    aResult.SegmentEnd = nIndex+2;
                }
            }
            break;
            case AccessibleTextType::GLYPH:
            {
                // get glyph at index
                implGetGlyphBoundary( aBoundary, nIndex );
                // get next glyph
                if ( aBoundary.endPos < nLength )
                {
                    implGetGlyphBoundary( aBoundary, aBoundary.endPos );
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
                implGetWordBoundary( aBoundary, nIndex );
                // get next word
                bool bWord = false;
                while ( !bWord && aBoundary.endPos < nLength )
                    bWord = implGetWordBoundary( aBoundary, aBoundary.endPos );
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
                implGetSentenceBoundary( aBoundary, nIndex );
                // get next sentence
                sal_Int32 nEnd = aBoundary.endPos;
                sal_Int32 nI = aBoundary.endPos;
                bool bFound = false;
                while ( !bFound && ++nI < nLength )
                {
                    implGetSentenceBoundary( aBoundary, nI );
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
                implGetParagraphBoundary( aBoundary, nIndex );
                // get next paragraph
                if ( aBoundary.endPos < nLength )
                {
                    implGetParagraphBoundary( aBoundary, aBoundary.endPos );
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
                implGetLineBoundary( aBoundary, nIndex );
                // get next line
                if ( aBoundary.endPos < nLength )
                {
                    implGetLineBoundary( aBoundary, aBoundary.endPos );
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
        const OUString& rOldString,
        const OUString& rNewString,
        css::uno::Any& rDeleted,
        css::uno::Any& rInserted) // throw()
    {
        sal_uInt32 nLenOld = rOldString.getLength();
        sal_uInt32 nLenNew = rNewString.getLength();

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
            aInsertedText.SegmentText = rNewString.copy( aInsertedText.SegmentStart, aInsertedText.SegmentEnd - aInsertedText.SegmentStart );

            rInserted <<= aInsertedText;
            return true;
        }

        // delete only
        if ((nLenOld > 0) && (0 == nLenNew))
        {
            aDeletedText.SegmentStart = 0;
            aDeletedText.SegmentEnd = nLenOld;
            aDeletedText.SegmentText = rOldString.copy( aDeletedText.SegmentStart, aDeletedText.SegmentEnd - aDeletedText.SegmentStart );

            rDeleted <<= aDeletedText;
            return true;
        }

        const sal_Unicode* pFirstDiffOld = rOldString.getStr();
        const sal_Unicode* pLastDiffOld  = rOldString.getStr() + nLenOld;
        const sal_Unicode* pFirstDiffNew = rNewString.getStr();
        const sal_Unicode* pLastDiffNew  = rNewString.getStr() + nLenNew;

        // find first difference
        while ((*pFirstDiffOld == *pFirstDiffNew) &&
               (pFirstDiffOld  <  pLastDiffOld) &&
               (pFirstDiffNew  <  pLastDiffNew))
        {
            pFirstDiffOld++;
            pFirstDiffNew++;
        }

        // equality test
        if ((0 == *pFirstDiffOld) && (0 == *pFirstDiffNew))
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
            aDeletedText.SegmentStart = pFirstDiffOld - rOldString.getStr();
            aDeletedText.SegmentEnd = pLastDiffOld  - rOldString.getStr();
            aDeletedText.SegmentText = rOldString.copy( aDeletedText.SegmentStart, aDeletedText.SegmentEnd - aDeletedText.SegmentStart );

            rDeleted <<= aDeletedText;
        }

        if (pFirstDiffNew < pLastDiffNew)
        {
            aInsertedText.SegmentStart = pFirstDiffNew - rNewString.getStr();
            aInsertedText.SegmentEnd = pLastDiffNew  - rNewString.getStr();
            aInsertedText.SegmentText = rNewString.copy( aInsertedText.SegmentStart, aInsertedText.SegmentEnd - aInsertedText.SegmentStart );

            rInserted <<= aInsertedText;
        }
        return true;
    }


    // OAccessibleTextHelper


    OAccessibleTextHelper::OAccessibleTextHelper( IMutex* _pExternalLock )
        :OAccessibleExtendedComponentHelper( _pExternalLock )
    {
    }


    // XInterface


    IMPLEMENT_FORWARD_XINTERFACE2( OAccessibleTextHelper, OAccessibleExtendedComponentHelper, OAccessibleTextHelper_Base )


    // XTypeProvider


    IMPLEMENT_FORWARD_XTYPEPROVIDER2( OAccessibleTextHelper, OAccessibleExtendedComponentHelper, OAccessibleTextHelper_Base )


    // XAccessibleText


    sal_Unicode OAccessibleTextHelper::getCharacter( sal_Int32 nIndex )
    {
        OExternalLockGuard aGuard( this );

        return OCommonAccessibleText::getCharacter( nIndex );
    }


    sal_Int32 OAccessibleTextHelper::getCharacterCount()
    {
        OExternalLockGuard aGuard( this );

        return OCommonAccessibleText::getCharacterCount();
    }


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


    OUString OAccessibleTextHelper::getText()
    {
        OExternalLockGuard aGuard( this );

        return OCommonAccessibleText::getText();
    }


    OUString OAccessibleTextHelper::getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    {
        OExternalLockGuard aGuard( this );

        return OCommonAccessibleText::getTextRange( nStartIndex, nEndIndex );
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
