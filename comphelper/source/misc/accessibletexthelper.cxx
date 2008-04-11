/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: accessibletexthelper.cxx,v $
 * $Revision: 1.9 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_comphelper.hxx"

// includes --------------------------------------------------------------
#include <comphelper/accessibletexthelper.hxx>
#include <com/sun/star/accessibility/AccessibleTextType.hpp>
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#ifndef _COM_SUN_STAR_TEXT_WORDTYPE_HPP_
#include <com/sun/star/i18n/WordType.hpp>
#endif
#include <com/sun/star/i18n/KCharacterType.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/accessibility/TextSegment.hpp>

#include <algorithm>

//..............................................................................
namespace comphelper
{
//..............................................................................

    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::accessibility;

    //==============================================================================
    // OCommonAccessibleText
    //==============================================================================

    OCommonAccessibleText::OCommonAccessibleText()
    {
    }

    // -----------------------------------------------------------------------------

    OCommonAccessibleText::~OCommonAccessibleText()
    {
    }

    // -----------------------------------------------------------------------------

    Reference < i18n::XBreakIterator > OCommonAccessibleText::implGetBreakIterator()
    {
        if ( !m_xBreakIter.is() )
        {
            Reference< lang::XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
            if ( xMSF.is() )
            {
                m_xBreakIter = Reference< i18n::XBreakIterator >
                    ( xMSF->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.i18n.BreakIterator" ) ) ), UNO_QUERY );
            }
        }

        return m_xBreakIter;
    }

    // -----------------------------------------------------------------------------

    Reference < i18n::XCharacterClassification > OCommonAccessibleText::implGetCharacterClassification()
    {
        if ( !m_xCharClass.is() )
        {
            Reference< lang::XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
            if ( xMSF.is() )
            {
                m_xCharClass = Reference< i18n::XCharacterClassification >
                    ( xMSF->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.i18n.CharacterClassification" ) ) ), UNO_QUERY );
            }
        }

        return m_xCharClass;
    }

    // -----------------------------------------------------------------------------

    sal_Bool OCommonAccessibleText::implIsValidBoundary( i18n::Boundary& rBoundary, sal_Int32 nLength )
    {
        return ( rBoundary.startPos >= 0 ) && ( rBoundary.startPos < nLength ) && ( rBoundary.endPos >= 0 ) && ( rBoundary.endPos <= nLength );
    }

    // -----------------------------------------------------------------------------

    sal_Bool OCommonAccessibleText::implIsValidIndex( sal_Int32 nIndex, sal_Int32 nLength )
    {
        return ( nIndex >= 0 ) && ( nIndex < nLength );
    }

    // -----------------------------------------------------------------------------

    sal_Bool OCommonAccessibleText::implIsValidRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex, sal_Int32 nLength )
    {
        return ( nStartIndex >= 0 ) && ( nStartIndex <= nLength ) && ( nEndIndex >= 0 ) && ( nEndIndex <= nLength );
    }

    // -----------------------------------------------------------------------------

    void OCommonAccessibleText::implGetGlyphBoundary( i18n::Boundary& rBoundary, sal_Int32 nIndex )
    {
        ::rtl::OUString sText( implGetText() );

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

    // -----------------------------------------------------------------------------

    sal_Bool OCommonAccessibleText::implGetWordBoundary( i18n::Boundary& rBoundary, sal_Int32 nIndex )
    {
        sal_Bool bWord = sal_False;
        ::rtl::OUString sText( implGetText() );

        if ( implIsValidIndex( nIndex, sText.getLength() ) )
        {
            Reference < i18n::XBreakIterator > xBreakIter = implGetBreakIterator();
            if ( xBreakIter.is() )
            {
                rBoundary = xBreakIter->getWordBoundary( sText, nIndex, implGetLocale(), i18n::WordType::ANY_WORD, sal_True );

                // it's a word, if the first character is an alpha-numeric character
                Reference< i18n::XCharacterClassification > xCharClass = implGetCharacterClassification();
                if ( xCharClass.is() )
                {
                    sal_Int32 nType = xCharClass->getCharacterType( sText, rBoundary.startPos, implGetLocale() );
                    if ( ( nType & ( i18n::KCharacterType::LETTER | i18n::KCharacterType::DIGIT ) ) != 0 )
                        bWord = sal_True;
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

    // -----------------------------------------------------------------------------

    void OCommonAccessibleText::implGetSentenceBoundary( i18n::Boundary& rBoundary, sal_Int32 nIndex )
    {
        ::rtl::OUString sText( implGetText() );

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

    // -----------------------------------------------------------------------------

    void OCommonAccessibleText::implGetParagraphBoundary( i18n::Boundary& rBoundary, sal_Int32 nIndex )
    {
        ::rtl::OUString sText( implGetText() );

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

    // -----------------------------------------------------------------------------

    void OCommonAccessibleText::implGetLineBoundary( i18n::Boundary& rBoundary, sal_Int32 nIndex )
    {
        ::rtl::OUString sText( implGetText() );
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

    // -----------------------------------------------------------------------------

    sal_Unicode OCommonAccessibleText::getCharacter( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ::rtl::OUString sText( implGetText() );

        if ( !implIsValidIndex( nIndex, sText.getLength() ) )
            throw IndexOutOfBoundsException();

        return sText.getStr()[nIndex];
    }

    // -----------------------------------------------------------------------------

    sal_Int32 OCommonAccessibleText::getCharacterCount() throw (RuntimeException)
    {
        return implGetText().getLength();
    }

    // -----------------------------------------------------------------------------

    ::rtl::OUString OCommonAccessibleText::getSelectedText() throw (RuntimeException)
    {
        ::rtl::OUString sText;
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

    // -----------------------------------------------------------------------------

    sal_Int32 OCommonAccessibleText::getSelectionStart() throw (RuntimeException)
    {
        sal_Int32 nStartIndex;
        sal_Int32 nEndIndex;

        implGetSelection( nStartIndex, nEndIndex );

        return nStartIndex;
    }

    // -----------------------------------------------------------------------------

    sal_Int32 OCommonAccessibleText::getSelectionEnd() throw (RuntimeException)
    {
        sal_Int32 nStartIndex;
        sal_Int32 nEndIndex;

        implGetSelection( nStartIndex, nEndIndex );

        return nEndIndex;
    }

    // -----------------------------------------------------------------------------

    ::rtl::OUString OCommonAccessibleText::getText() throw (RuntimeException)
    {
        return implGetText();
    }

    // -----------------------------------------------------------------------------

    ::rtl::OUString OCommonAccessibleText::getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ::rtl::OUString sText( implGetText() );

        if ( !implIsValidRange( nStartIndex, nEndIndex, sText.getLength() ) )
            throw IndexOutOfBoundsException();

        sal_Int32 nMinIndex = ::std::min( nStartIndex, nEndIndex );
        sal_Int32 nMaxIndex = ::std::max( nStartIndex, nEndIndex );

        return sText.copy( nMinIndex, nMaxIndex - nMinIndex );
    }

    // -----------------------------------------------------------------------------

    TextSegment OCommonAccessibleText::getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (IndexOutOfBoundsException, IllegalArgumentException, RuntimeException)
    {
        ::rtl::OUString sText( implGetText() );
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
                sal_Bool bWord = implGetWordBoundary( aBoundary, nIndex );
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

    // -----------------------------------------------------------------------------

    TextSegment OCommonAccessibleText::getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (IndexOutOfBoundsException, IllegalArgumentException, RuntimeException)
    {
        ::rtl::OUString sText( implGetText() );
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
                sal_Bool bWord = sal_False;
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

    // -----------------------------------------------------------------------------

    TextSegment OCommonAccessibleText::getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (IndexOutOfBoundsException, IllegalArgumentException, RuntimeException)
    {
        ::rtl::OUString sText( implGetText() );
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
                sal_Bool bWord = sal_False;
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
                sal_Bool bFound = sal_False;
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

    // -----------------------------------------------------------------------------
    bool OCommonAccessibleText::implInitTextChangedEvent(
        const rtl::OUString& rOldString,
        const rtl::OUString& rNewString,
        ::com::sun::star::uno::Any& rDeleted,
        ::com::sun::star::uno::Any& rInserted) // throw()
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

    //==============================================================================
    // OAccessibleTextHelper
    //==============================================================================

    OAccessibleTextHelper::OAccessibleTextHelper()
    {
    }

    // -----------------------------------------------------------------------------

    OAccessibleTextHelper::OAccessibleTextHelper( IMutex* _pExternalLock )
        :OAccessibleExtendedComponentHelper( _pExternalLock )
    {
    }

    // -----------------------------------------------------------------------------
    // XInterface
    // -----------------------------------------------------------------------------

    IMPLEMENT_FORWARD_XINTERFACE2( OAccessibleTextHelper, OAccessibleExtendedComponentHelper, OAccessibleTextHelper_Base )

    // -----------------------------------------------------------------------------
    // XTypeProvider
    // -----------------------------------------------------------------------------

    IMPLEMENT_FORWARD_XTYPEPROVIDER2( OAccessibleTextHelper, OAccessibleExtendedComponentHelper, OAccessibleTextHelper_Base )

    // -----------------------------------------------------------------------------
    // XAccessibleText
    // -----------------------------------------------------------------------------

    sal_Unicode OAccessibleTextHelper::getCharacter( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        return OCommonAccessibleText::getCharacter( nIndex );
    }

    // -----------------------------------------------------------------------------

    sal_Int32 OAccessibleTextHelper::getCharacterCount() throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        return OCommonAccessibleText::getCharacterCount();
    }

    // -----------------------------------------------------------------------------

    ::rtl::OUString OAccessibleTextHelper::getSelectedText() throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        return OCommonAccessibleText::getSelectedText();
    }

    // -----------------------------------------------------------------------------

    sal_Int32 OAccessibleTextHelper::getSelectionStart() throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        return OCommonAccessibleText::getSelectionStart();
    }

    // -----------------------------------------------------------------------------

    sal_Int32 OAccessibleTextHelper::getSelectionEnd() throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        return OCommonAccessibleText::getSelectionEnd();
    }

    // -----------------------------------------------------------------------------

    ::rtl::OUString OAccessibleTextHelper::getText() throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        return OCommonAccessibleText::getText();
    }

    // -----------------------------------------------------------------------------

    ::rtl::OUString OAccessibleTextHelper::getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        return OCommonAccessibleText::getTextRange( nStartIndex, nEndIndex );
    }

    // -----------------------------------------------------------------------------

    TextSegment OAccessibleTextHelper::getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (IndexOutOfBoundsException, IllegalArgumentException, RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        return OCommonAccessibleText::getTextAtIndex( nIndex, aTextType );
    }

    // -----------------------------------------------------------------------------

    TextSegment OAccessibleTextHelper::getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (IndexOutOfBoundsException, IllegalArgumentException, RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        return OCommonAccessibleText::getTextBeforeIndex( nIndex, aTextType );
    }

    // -----------------------------------------------------------------------------

    TextSegment OAccessibleTextHelper::getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (IndexOutOfBoundsException, IllegalArgumentException, RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        return OCommonAccessibleText::getTextBehindIndex( nIndex, aTextType );
    }

    // -----------------------------------------------------------------------------

//..............................................................................
}   // namespace comphelper
//..............................................................................
