/*************************************************************************
 *
 *  $RCSfile: accessibletexthelper.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 17:27:52 $
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

// includes --------------------------------------------------------------

#ifndef COMPHELPER_ACCESSIBLE_TEXT_HELPER_HXX
#include <comphelper/accessibletexthelper.hxx>
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLETEXTTYPE_HPP_
#include <com/sun/star/accessibility/AccessibleTextType.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_CHARACTERITERATORMODE_HPP_
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_WORDTYPE_HPP_
#include <com/sun/star/i18n/WordType.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_KCHARACTERTYPE_HPP_
#include <com/sun/star/i18n/KCharacterType.hpp>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _COM_SUN_STAR_AWT_SELECTION_HPP_
#include <com/sun/star/awt/Selection.hpp>
#endif

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

        if ( implIsValidIndex( nIndex, sText.getLength() ) )
        {
            rBoundary.startPos = 0;
            rBoundary.endPos = implGetText().getLength();
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

    ::rtl::OUString OCommonAccessibleText::getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ::rtl::OUString sResult;
        ::rtl::OUString sText( implGetText() );
        sal_Int32 nLength = sText.getLength();
        i18n::Boundary aBoundary;

        if ( !implIsValidIndex( nIndex, nLength ) && nIndex != nLength )
            throw IndexOutOfBoundsException();

        switch ( aTextType )
        {
            case AccessibleTextType::CHARACTER:
            {
                if ( implIsValidIndex( nIndex, nLength ) )
                    sResult = sText.copy( nIndex, 1 );
            }
            break;
            case AccessibleTextType::GLYPH:
            {
                // get glyph at index
                implGetGlyphBoundary( aBoundary, nIndex );
                if ( implIsValidBoundary( aBoundary, nLength ) )
                    sResult = sText.copy( aBoundary.startPos, aBoundary.endPos - aBoundary.startPos );
            }
            break;
            case AccessibleTextType::WORD:
            {
                // get word at index
                sal_Bool bWord = implGetWordBoundary( aBoundary, nIndex );
                if ( bWord && implIsValidBoundary( aBoundary, nLength ) )
                    sResult = sText.copy( aBoundary.startPos, aBoundary.endPos - aBoundary.startPos );
            }
            break;
            case AccessibleTextType::SENTENCE:
            {
                // get sentence at index
                implGetSentenceBoundary( aBoundary, nIndex );
                if ( implIsValidBoundary( aBoundary, nLength ) )
                    sResult = sText.copy( aBoundary.startPos, aBoundary.endPos - aBoundary.startPos );
            }
            break;
            case AccessibleTextType::PARAGRAPH:
            {
                // get paragraph at index
                implGetParagraphBoundary( aBoundary, nIndex );
                if ( implIsValidBoundary( aBoundary, nLength ) )
                    sResult = sText.copy( aBoundary.startPos, aBoundary.endPos - aBoundary.startPos );
            }
            break;
            case AccessibleTextType::LINE:
            {
                // get line at index
                implGetLineBoundary( aBoundary, nIndex );
                if ( implIsValidBoundary( aBoundary, nLength ) )
                    sResult = sText.copy( aBoundary.startPos, aBoundary.endPos - aBoundary.startPos );
            }
            break;
            default:
            {
                // unknown text type
            }
        }

        return sResult;
    }

    // -----------------------------------------------------------------------------

    ::rtl::OUString OCommonAccessibleText::getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ::rtl::OUString sResult;
        ::rtl::OUString sText( implGetText() );
        sal_Int32 nLength = sText.getLength();
        i18n::Boundary aBoundary;

        if ( !implIsValidIndex( nIndex, nLength ) && nIndex != nLength )
            throw IndexOutOfBoundsException();

        switch ( aTextType )
        {
            case AccessibleTextType::CHARACTER:
            {
                if ( implIsValidIndex( nIndex - 1, nLength ) )
                    sResult = sText.copy( nIndex - 1, 1 );
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
                        sResult = sText.copy( aBoundary.startPos, aBoundary.endPos - aBoundary.startPos );
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
                    sResult = sText.copy( aBoundary.startPos, aBoundary.endPos - aBoundary.startPos );
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
                        sResult = sText.copy( aBoundary.startPos, aBoundary.endPos - aBoundary.startPos );
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
                        sResult = sText.copy( aBoundary.startPos, aBoundary.endPos - aBoundary.startPos );
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
                        sResult = sText.copy( aBoundary.startPos, aBoundary.endPos - aBoundary.startPos );
                }
            }
            break;
            default:
            {
                // unknown text type
            }
        }

        return sResult;
    }

    // -----------------------------------------------------------------------------

    ::rtl::OUString OCommonAccessibleText::getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ::rtl::OUString sResult;
        ::rtl::OUString sText( implGetText() );
        sal_Int32 nLength = sText.getLength();
        i18n::Boundary aBoundary;

        if ( !implIsValidIndex( nIndex, nLength ) && nIndex != nLength )
            throw IndexOutOfBoundsException();

        switch ( aTextType )
        {
            case AccessibleTextType::CHARACTER:
            {
                if ( implIsValidIndex( nIndex + 1, nLength ) )
                    sResult = sText.copy( nIndex + 1, 1 );
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
                        sResult = sText.copy( aBoundary.startPos, aBoundary.endPos - aBoundary.startPos );
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
                    sResult = sText.copy( aBoundary.startPos, aBoundary.endPos - aBoundary.startPos );
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
                    sResult = sText.copy( aBoundary.startPos, aBoundary.endPos - aBoundary.startPos );
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
                        sResult = sText.copy( aBoundary.startPos, aBoundary.endPos - aBoundary.startPos );
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
                        sResult = sText.copy( aBoundary.startPos, aBoundary.endPos - aBoundary.startPos );
                }
            }
            break;
            default:
            {
                // unknown text type
            }
        }

        return sResult;
    }

    // -----------------------------------------------------------------------------
    bool OCommonAccessibleText::implInitTextChangedEvent(
        const rtl::OUString& rOldString,
        const rtl::OUString& rNewString,
        ::com::sun::star::uno::Any& rDeleteSelection,
        ::com::sun::star::uno::Any& rInsertSelection) // throw()
    {
        sal_uInt32 nLenOld = rOldString.getLength();
        sal_uInt32 nLenNew = rNewString.getLength();

        // equal
        if ((0 == nLenOld) && (0 == nLenNew))
            return false;

        ::com::sun::star::awt::Selection DelSel;
        ::com::sun::star::awt::Selection InsSel;

        DelSel.Min = -1;
        DelSel.Max = -1;
        InsSel.Min = -1;
        InsSel.Max = -1;

        // insert only
        if ((0 == nLenOld) && (nLenNew > 0))
        {
            InsSel.Min = 0;
            InsSel.Max = nLenNew;
            rInsertSelection <<= InsSel;
            return true;
        }

        // delete only
        if ((nLenOld > 0) && (0 == nLenNew))
        {
            DelSel.Min = 0;
            DelSel.Max = nLenOld;
            rDeleteSelection <<= DelSel;
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
        while (((pLastDiffOld - 1) > pFirstDiffOld) &&
               ((pLastDiffNew - 1) > pFirstDiffNew) &&
               (pLastDiffOld[-1]  == pLastDiffNew[-1]))
        {
            pLastDiffOld--;
            pLastDiffNew--;
        }

        if (pFirstDiffOld < pLastDiffOld)
        {
            DelSel.Min = pFirstDiffOld - rOldString.getStr();
            DelSel.Max = pLastDiffOld  - rOldString.getStr();

            rDeleteSelection <<= DelSel;
        }

        if (pFirstDiffNew < pLastDiffNew)
        {
            InsSel.Min = pFirstDiffNew - rNewString.getStr();
            InsSel.Max = pLastDiffNew  - rNewString.getStr();

            rInsertSelection <<= InsSel;
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

    ::rtl::OUString OAccessibleTextHelper::getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        return OCommonAccessibleText::getTextAtIndex( nIndex, aTextType );
    }

    // -----------------------------------------------------------------------------

    ::rtl::OUString OAccessibleTextHelper::getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        return OCommonAccessibleText::getTextBeforeIndex( nIndex, aTextType );
    }

    // -----------------------------------------------------------------------------

    ::rtl::OUString OAccessibleTextHelper::getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        return OCommonAccessibleText::getTextBehindIndex( nIndex, aTextType );
    }

    // -----------------------------------------------------------------------------

//..............................................................................
}   // namespace comphelper
//..............................................................................
