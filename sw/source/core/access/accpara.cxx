 /*************************************************************************
 *
 *  $RCSfile: accpara.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: dvo $ $Date: 2002-02-20 15:22:26 $
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

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _UNOOBJ_HXX
#include <unoobj.hxx>
#endif

#pragma hdrstop

#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEROLE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLESTATETYPE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLETEXTTYPE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleTextType.hpp>
#endif

#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX_
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_WORDTYPE_HPP_
#include <com/sun/star/i18n/WordType.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_XBREAKITERATOR_HPP_
#include <com/sun/star/i18n/XBreakIterator.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_
#include <com/sun/star/uno/RuntimeException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_INDEXOUTOFBOUNDSEXCEPTION_HPP_
#include <com/sun/star/lang/INDEXOUTOFBOUNDSEXCEPTION.hpp>
#endif

#ifndef _BREAKIT_HXX
#include <breakit.hxx>
#endif

#ifndef _ACCPARA_HXX
#include "accpara.hxx"
#endif
#ifndef _ACCESS_HRC
#include "access.hrc"
#endif
#ifndef _ACCPORTIONS_HXX
#include "accportions.hxx"
#endif

using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::drafts::com::sun::star::accessibility;
using namespace ::rtl;

using ::com::sun::star::beans::PropertyValue;


const sal_Char sServiceName[] = "com.sun.star.text.AccessibleParagraphView";
const sal_Char sImplementationName[] = "SwAccessibleParagraph";
const xub_StrLen MAX_DESC_TEXT_LEN = 40;

void SwAccessibleParagraph::SetStates(
        ::utl::AccessibleStateSetHelper& rStateSet )
{
    SwAccessibleContext::SetStates( rStateSet );

    // MULTILINE
    rStateSet.AddState( AccessibleStateType::MULTILINE );

    // SELECTABLE
    rStateSet.AddState( AccessibleStateType::SELECTABLE );

    // TODO: SELECTED
}

SwAccessibleParagraph::SwAccessibleParagraph(
        sal_Int32 nPara,
        const Rectangle& rVisArea,
        const SwTxtFrm *pTxtFrm ) :
    SwAccessibleContext( AccessibleRole::PARAGRAPH, rVisArea, pTxtFrm ),
    pPortionData( NULL )
{
    const SwTxtNode *pTxtNd = pTxtFrm->GetTxtNode();
    sal_uInt16 nResId = (pTxtNd->GetOutlineNum() && !pTxtNd->GetNum())
                            ? STR_ACCESS_HEADING_NAME
                            : STR_ACCESS_PARAGRAPH_NAME;
    OUString sArg( OUString::valueOf( nPara ) );
    SetName( GetResource( nResId, &sArg ) );
}

SwAccessibleParagraph::~SwAccessibleParagraph()
{
    delete pPortionData;
}

void SwAccessibleParagraph::UpdatePortionData()
    throw( RuntimeException )
{
    CHECK_FOR_DEFUNC( XAccessibleContext );     // check for living frame

    // obtain the text frame
    DBG_ASSERT( GetFrm()->IsTxtFrm(), "The text frame has mutated!" );
    const SwTxtFrm* pFrm = static_cast<const SwTxtFrm*>( GetFrm() );

    // obtain the model string via the text node
    const String& rModelString = pFrm->GetTxtNode()->GetTxt();

    // build new portion data
    delete pPortionData;
    pPortionData = new SwAccessiblePortionData( rModelString );
    pFrm->VisitPortions( *pPortionData );

//     //
//     // replacement code, as long as portion visitor is not ready
//     //


//     // iterate over characters and replace 'specials'
//     sal_Int32 nLength = rModelString.Len();
//     const sal_Unicode* pData = rModelString.GetBuffer();
//     String sBreak(RTL_CONSTASCII_USTRINGPARAM("[BREAK]"));
//     String sIn(RTL_CONSTASCII_USTRINGPARAM("[IN]"));
//     const sal_Int32 nLineLength = 30;
//     USHORT nLast = 0;
//     for( USHORT i = 0; i < nLength; i++ )
//     {
//         sal_Unicode aChar = pData[i];
//         if( (aChar == CH_TXTATR_BREAKWORD) ||
//             (aChar == CH_TXTATR_INWORD) )
//         {
//             if( nLast != i )
//             {
//                 pPortionData->Text( i - nLast );
//                 nLast = i+1;
//             }
//             pPortionData->Special(
//                 1, (aChar == CH_TXTATR_BREAKWORD) ? sBreak : sIn, 0 );
//         }
//         else if ( (i % nLineLength) == (nLineLength-1))
//         {
//             // simulate line break
//             pPortionData->Text( i - nLast );
//             pPortionData->LineBreak();
//             nLast = i+1;
//         }
//     }
//     if( nLast < nLength )
//         pPortionData->Text( nLength - nLast );
//     pPortionData->Finish();

    DBG_ASSERT( pPortionData != NULL, "UpdatePortionData() failed" );
}


// text boundaries

void SwAccessibleParagraph::ClearPortionData()
{
    delete pPortionData;
    pPortionData = NULL;
}


void SwAccessibleParagraph::GetCharBoundary(
    Boundary& rBound,
    const OUString& rText,
    sal_Int32 nPos )
{
    rBound.startPos = nPos;
    rBound.endPos = nPos+1;
}

void SwAccessibleParagraph::GetWordBoundary(
    Boundary& rBound,
    const OUString& rText,
    sal_Int32 nPos )
{
//     DBG_ASSERT( pBreakIt != NULL, "We always need a break." );
//     DBG_ASSERT( pBreakIt->xBreak.is(), "No break-iterator." );

//     sal_Int32 nStart = 0;
//     sal_Int32 nEnd = 0;
//     const USHORT nWordType = WordType::DICTIONARY_WORD;

//     // instead of using xBreak->getWordBoundary, go right with the
//     // cursor, and then go back left. This behaviour is more
//     // consistent with GetSentenceBoundary.

//     if( pBreakIt->xBreak.is() )
//     {
//         const SwTxtNode* pNode = GetTxtNode();

//         // TODO: replace with model position
//         USHORT nModelPos = 0;

//         nEnd = pBreakIt->xBreak->nextWord(
//             rText, nPos, pBreakIt->GetLocale( pNode->GetLang( nModelPos ) ),
//             nWordType ).startPos;
//         if( (nEnd < 0) && (nEnd > rText.getLength()) )
//             nEnd = rText.getLength();

//         nStart = pBreakIt->xBreak->previousWord(
//             rText, nEnd-1, pBreakIt->GetLocale( pNode->GetLang( nModelPos ) ),
//             nWordType ).startPos;
//         if( (nStart < 0) && (nStart > rText.getLength()) )
//             nStart = nEnd;
//     }

//     rBound.startPos = nStart;
//     rBound.endPos = nEnd;

    GetPortionData().GetWordBoundary( rBound, nPos, GetTxtNode() );
}

void SwAccessibleParagraph::GetSentenceBoundary(
    Boundary& rBound,
    const OUString& rText,
    sal_Int32 nPos )
{
//     DBG_ASSERT( pBreakIt != NULL, "I need a break." );
//     DBG_ASSERT( pBreakIt->xBreak.is(), "No break-iterator." );

//     sal_Int32 nStart = 0;
//     sal_Int32 nEnd = 0;

//     // TODO: replace with model position
//     USHORT nModelPos = 0;
//     const SwTxtNode* pNode = GetTxtNode();

//     if( pBreakIt->xBreak.is() )
//     {
//         // goto end + beginning of sentence; check result values

//         nEnd = pBreakIt->xBreak->endOfSentence(
//             rText, nPos, pBreakIt->GetLocale( pNode->GetLang( nModelPos ) ) );
//         if( (nEnd < 0) && (nEnd > rText.getLength()) )
//             nEnd = rText.getLength();

//         nStart = pBreakIt->xBreak->beginOfSentence(
//             rText, nEnd, pBreakIt->GetLocale( pNode->GetLang( nModelPos ) ) );
//         if( (nStart < 0) && (nStart > rText.getLength()) )
//             nStart = nEnd;
//     }

//     rBound.startPos = nStart;
//     rBound.endPos = nEnd;

    GetPortionData().GetSentenceBoundary( rBound, nPos, GetTxtNode() );
}

void SwAccessibleParagraph::GetLineBoundary(
    Boundary& rBound,
    const OUString& rText,
    sal_Int32 nPos )
{
    GetPortionData().GetLineBoundary( rBound, nPos );
}

void SwAccessibleParagraph::GetParagraphBoundary(
    Boundary& rBound,
    const OUString& rText,
    sal_Int32 nPos )
{
    rBound.startPos = 0;
    rBound.endPos = rText.getLength();
}

void SwAccessibleParagraph::GetEmptyBoundary( Boundary& rBound )
{
    rBound.startPos = 0;
    rBound.endPos = 0;
}


void SwAccessibleParagraph::GetTextBoundary(
    Boundary& rBound,
    const OUString& rText,
    sal_Int32 nPos,
    sal_Int16 nTextType )
    throw (
        IndexOutOfBoundsException,
        RuntimeException)
{
    if( (nPos < 0) || (nPos >= rText.getLength()) )
        throw IndexOutOfBoundsException();

    switch( nTextType )
    {
        case AccessibleTextType::WORD:
            GetWordBoundary( rBound, rText, nPos );
            break;

        case AccessibleTextType::SENTENCE:
            GetSentenceBoundary( rBound, rText, nPos );
            break;

        case AccessibleTextType::PARAGRAPH:
            GetParagraphBoundary( rBound, rText, nPos );
            break;

        case AccessibleTextType::CHARACTER:
            GetCharBoundary( rBound, rText, nPos );
            break;

        case AccessibleTextType::LINE:
            GetLineBoundary( rBound, rText, nPos );
            break;

        default:
            // The specification asks us to return an empty string
            // if the text type is no valid.
            GetEmptyBoundary( rBound );
            break;
    }
}


const SwTxtNode* SwAccessibleParagraph::GetTxtNode()
{
    const SwFrm* pFrm = GetFrm();
    DBG_ASSERT( pFrm->IsTxtFrm(), "The text frame has mutated!" );

    const SwTxtNode* pNode = static_cast<const SwTxtFrm*>(pFrm)->GetTxtNode();
    DBG_ASSERT( pNode != NULL, "A text frame without a text node." );

    return pNode;
}

OUString SwAccessibleParagraph::GetString()
{
    return GetPortionData().GetAccesibleString();
}


OUString SAL_CALL SwAccessibleParagraph::getAccessibleDescription (void)
        throw (::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleContext )

    SwTxtFrm *pTxtFrm = PTR_CAST( SwTxtFrm, GetFrm() );
    if( !pTxtFrm )
    {
        THROW_RUNTIME_EXCEPTION( XAccessibleContext, "internal error (no text frame)" );
    }

    const SwTxtNode *pTxtNd = pTxtFrm->GetTxtNode();
    const String& rText = pTxtNd->GetTxt();
    xub_StrLen  nLen = (xub_StrLen)pBreakIt->xBreak->endOfSentence(
                                    rText, 0, pBreakIt->GetLocale(
                                                pTxtNd->GetLang( 0 ) ));
    if( nLen > MAX_DESC_TEXT_LEN )
    {
        nLen = (xub_StrLen)pBreakIt->xBreak->getWordBoundary(
                                rText, MAX_DESC_TEXT_LEN,
                                pBreakIt->GetLocale( pTxtNd->GetLang( MAX_DESC_TEXT_LEN ) ),
                                WordType::ANY_WORD, sal_True ).endPos;
    }

    OUString sArg1( rText.Copy( 0, nLen ) );

    sal_uInt16 nResId;
    OUString sArg2, *pArg2 = 0;
    if( pTxtNd->GetOutlineNum() && !pTxtNd->GetNum() )
    {
        sArg2 = OUString( pTxtNd->GetNumString() );
        if( sArg2.getLength() )
        {
            nResId = STR_ACCESS_HEADING_WITH_NUM_DESC;
            pArg2 = &sArg2;
        }
        else
        {
            nResId = STR_ACCESS_HEADING_DESC;
        }
    }
    else
    {
        nResId = STR_ACCESS_PARAGRAPH_DESC;
    }

    return GetResource( nResId, &sArg1, pArg2 );
}

Locale SAL_CALL SwAccessibleParagraph::getLocale (void)
        throw (::drafts::com::sun::star::accessibility::IllegalAccessibleComponentStateException, ::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    SwTxtFrm *pTxtFrm = PTR_CAST( SwTxtFrm, GetFrm() );
    if( !pTxtFrm )
    {
        THROW_RUNTIME_EXCEPTION( XAccessibleContext, "internal error (no text frame)" );
    }

    const SwTxtNode *pTxtNd = pTxtFrm->GetTxtNode();
    Locale aLoc( pBreakIt->GetLocale( pTxtNd->GetLang( 0 ) ) );

    return aLoc;
}

OUString SAL_CALL SwAccessibleParagraph::getImplementationName()
        throw( RuntimeException )
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM(sImplementationName));
}

sal_Bool SAL_CALL SwAccessibleParagraph::supportsService(
        const ::rtl::OUString& sTestServiceName)
    throw (::com::sun::star::uno::RuntimeException)
{
    return sTestServiceName.equalsAsciiL( sServiceName, sizeof(sServiceName)-1 );
}

Sequence< OUString > SAL_CALL SwAccessibleParagraph::getSupportedServiceNames()
        throw( ::com::sun::star::uno::RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = OUString( RTL_CONSTASCII_USTRINGPARAM(sServiceName) );
    return aRet;
}

//
//=====  XInterface  =======================================================
//

Any SwAccessibleParagraph::queryInterface( const Type& rType )
    throw (RuntimeException)
{
    Any aRet;
    if ( rType == ::getCppuType((Reference<XAccessibleText> *)0) )
    {
        Reference<XAccessibleText> aAccText = this;
        aRet <<= aAccText;
    }
    else if ( rType == ::getCppuType((Reference<XAccessibleEditableText> *)0) )
    {
        Reference<XAccessibleEditableText> aAccEditText = this;
        aRet <<= aAccEditText;
    }
    else
    {
        aRet = SwAccessibleContext::queryInterface(rType);
    }

    return aRet;
}


//
//=====  XAccesibleText  ===================================================
//

sal_Int32 SwAccessibleParagraph::getCaretPosition()
    throw (RuntimeException)
{
    // HACK: dummy implementation
    return 0;
}

sal_Unicode SwAccessibleParagraph::getCharacter( sal_Int32 nIndex )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    CHECK_FOR_DEFUNC( XAccessibleContext );

    OUString sText( GetString() );

    if( (nIndex >= 0) && (nIndex < sText.getLength()) )
    {
        return sText.getStr()[nIndex];
    }
    else
        throw IndexOutOfBoundsException();
}

Sequence<PropertyValue> SwAccessibleParagraph::getCharacterAttributes( sal_Int32 nIndex )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    // HACK: dummy implementation
    Sequence<PropertyValue> aSeq;
    return aSeq;
}

com::sun::star::awt::Rectangle SwAccessibleParagraph::getCharacterBounds( sal_Int32 nIndex )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    // HACK: dummy implementation
    com::sun::star::awt::Rectangle aRect;
    return aRect;
}

sal_Int32 SwAccessibleParagraph::getCharacterCount()
    throw (RuntimeException)
{
    CHECK_FOR_DEFUNC( XAccessibleContext );     // check for live frame

    return GetString().getLength();
}

sal_Int32 SwAccessibleParagraph::getIndexAtPoint( const com::sun::star::awt::Point& aPoint )
    throw (RuntimeException)
{
    // HACK: dummy implementation
    return 0;
}

OUString SwAccessibleParagraph::getSelectedText()
    throw (RuntimeException)
{
    // HACK: dummy implementation
    return OUString(RTL_CONSTASCII_USTRINGPARAM("Peter"));
}

sal_Int32 SwAccessibleParagraph::getSelectionStart()
    throw (RuntimeException)
{
    // HACK: dummy implementation
    return 0;
}

sal_Int32 SwAccessibleParagraph::getSelectionEnd()
    throw (RuntimeException)
{
    // HACK: dummy implementation
    return 0;
}

sal_Bool SwAccessibleParagraph::setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    // HACK: dummy implementation
    return sal_False;
}

OUString SwAccessibleParagraph::getText()
    throw (RuntimeException)
{
    CHECK_FOR_DEFUNC( XAccessibleContext );     // check for live frame

    return GetString();
}

OUString SwAccessibleParagraph::getTextRange(
    sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    CHECK_FOR_DEFUNC( XAccessibleContext );     // check for live frame

    OUString sText( GetString() );

    if ( (nStartIndex <= nEndIndex) &&
         (nStartIndex >= 0) &&
         (nEndIndex < sText.getLength()) )
    {
        return sText.copy(nStartIndex, nEndIndex-nStartIndex+1 );
    }
    else
        throw IndexOutOfBoundsException();
}

OUString SwAccessibleParagraph::getTextAtIndex(
    sal_Int32 nIndex, sal_Int16 nTextType )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    CHECK_FOR_DEFUNC( XAccessibleContext );     // check for live frame

    const OUString rText = GetString();

    Boundary aBound;
    GetTextBoundary( aBound, rText, nIndex, nTextType );

    DBG_ASSERT( aBound.startPos >= 0,               "illegal boundary" );
    DBG_ASSERT( aBound.startPos <= aBound.endPos,   "illegal boundary" );

    return rText.copy( aBound.startPos, aBound.endPos - aBound.startPos );
}

OUString SwAccessibleParagraph::getTextBeforeIndex(
    sal_Int32 nIndex, sal_Int16 nTextType )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    CHECK_FOR_DEFUNC( XAccessibleContext );     // check for live frame

    const OUString rText = GetString();

    Boundary aBound;
    GetTextBoundary( aBound, rText, nIndex, nTextType );
    if( aBound.startPos > 0 )
        GetTextBoundary( aBound, rText, aBound.startPos-1, nTextType );
    else
        GetEmptyBoundary( aBound );

    return rText.copy( aBound.startPos, aBound.endPos - aBound.startPos );
}

OUString SwAccessibleParagraph::getTextBehindIndex(
    sal_Int32 nIndex, sal_Int16 nTextType )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    CHECK_FOR_DEFUNC( XAccessibleContext );     // check for live frame

    const OUString rText = GetString();

    Boundary aBound;
    GetTextBoundary( aBound, rText, nIndex, nTextType );
    if( aBound.endPos < (rText.getLength()-1) )
        GetTextBoundary( aBound, rText, aBound.endPos+1, nTextType );
    else
        GetEmptyBoundary( aBound );

    return rText.copy( aBound.startPos, aBound.endPos - aBound.startPos );
}

sal_Bool SwAccessibleParagraph::copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    // HACK: dummy implementation
    return sal_False;
}


//
//=====  XAccesibleEditableText  ==========================================
//

sal_Bool SwAccessibleParagraph::cutText( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    // HACK: dummy implementation
    return sal_False;
}

sal_Bool SwAccessibleParagraph::pasteText( sal_Int32 nIndex )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    // HACK: dummy implementation
    return sal_False;
}

sal_Bool SwAccessibleParagraph::deleteText( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    return replaceText( nStartIndex, nEndIndex, OUString() );
}

sal_Bool SwAccessibleParagraph::insertText( const OUString& sText, sal_Int32 nIndex )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    return replaceText( nIndex, nIndex, sText );
}

sal_Bool SwAccessibleParagraph::replaceText(
    sal_Int32 nStartIndex, sal_Int32 nEndIndex,
    const OUString& sReplacement )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    CHECK_FOR_DEFUNC( XAccessibleContext );     // check for live frame

    const OUString& rText = GetString();

    if( (nStartIndex >= 0) &&
        (nEndIndex <= rText.getLength()) &&
        (nStartIndex <= nEndIndex) )
    {
        SwTxtNode* pNode = const_cast<SwTxtNode*>( GetTxtNode() );

        // create SwPosition for nStartIndex
        SwIndex aIndex(
            pNode, static_cast<sal_uInt32>(
                GetPortionData().GetModelPosition( nStartIndex )) );
        SwPosition aStartPos( *pNode, aIndex );

        // create SwPosition for nEndIndex
        SwPosition aEndPos( aStartPos );
        aEndPos.nContent = static_cast<sal_uInt32>(
            GetPortionData().GetModelPosition( nEndIndex ) );

        // now create XTextRange as helper and set string
        SwXTextRange::CreateTextRangeFromPosition(
            pNode->GetDoc(), aStartPos, &aEndPos)->setString( sReplacement );

        // delete portion data
        ClearPortionData();

        return sal_True;    // We always succeed! :-)
    }
    else
        throw IndexOutOfBoundsException();
}

sal_Bool SwAccessibleParagraph::setAttributes( sal_Int32 nStartIndex, sal_Int32 nEndIndex,
                        const Sequence<PropertyValue>& aAttributeSet )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    // HACK: dummy implementation
    return sal_False;
}

sal_Bool SwAccessibleParagraph::setText( const OUString& sText )
    throw (RuntimeException)
{
    return replaceText(0, GetString().getLength(), sText);
}

