 /*************************************************************************
 *
 *  $RCSfile: accpara.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: dvo $ $Date: 2002-03-21 11:07:26 $
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
#ifndef _CRSTATE_HXX
#include <crstate.hxx>
#endif
#ifndef _ACCMAP_HXX
#include <accmap.hxx>
#endif
#ifndef _CRSRSH_HXX
#include <crsrsh.hxx>
#endif


#pragma hdrstop

#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
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
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTID_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleEventId.hpp>
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

#ifndef _SFXSIDS_HRC
#include <sfx2/sfxsids.hrc>     // for copy/cut/pasteText(...)
#endif
#ifndef _SFXVIEWSH_HXX
#include <sfx2/viewsh.hxx>      // for ExecuteAtViewShell(...)
#endif
#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>    // for ExecuteAtViewShell(...)
#endif

#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>   // for GetWordBoundary
#endif
#ifndef _SWTYPES_HXX
#include "swtypes.hxx"
#endif

#include <algorithm>


using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::drafts::com::sun::star::accessibility;
using namespace ::rtl;

using ::com::sun::star::beans::PropertyValue;
using std::max;
using std::min;


const sal_Char sServiceName[] = "com.sun.star.text.AccessibleParagraphView";
const sal_Char sImplementationName[] = "SwAccessibleParagraph";
const xub_StrLen MAX_DESC_TEXT_LEN = 40;
const SwTxtNode* SwAccessibleParagraph::GetTxtNode() const
{
    const SwFrm* pFrm = GetFrm();
    DBG_ASSERT( pFrm->IsTxtFrm(), "The text frame has mutated!" );

    const SwTxtNode* pNode = static_cast<const SwTxtFrm*>(pFrm)->GetTxtNode();
    DBG_ASSERT( pNode != NULL, "A text frame without a text node." );

    return pNode;
}

OUString SwAccessibleParagraph::GetString()
{
    return GetPortionData().GetAccessibleString();
}

OUString SwAccessibleParagraph::GetDescription()
{
    const OUString& rText = GetString();

    // the description contains the first sentence up to
    // MAX_DESC_TEXT_LEN characters (including the next full word)
    Boundary aBound;
    if( rText.getLength() > 0 )
    {
        GetSentenceBoundary( aBound, rText, 0 );
        if( aBound.endPos > MAX_DESC_TEXT_LEN )
        {
            GetWordBoundary( aBound, rText, MAX_DESC_TEXT_LEN );
            aBound.startPos = 0;
        }
    }
    else
        GetEmptyBoundary( aBound );
    OUString sArg1( rText.copy( aBound.startPos, aBound.endPos ) );

    sal_Int16 nResId;
    if( IsHeading() )
    {
        nResId = STR_ACCESS_HEADING_DESC;
    }
    else
    {
        nResId = STR_ACCESS_PARAGRAPH_DESC;
    }

    return GetResource( nResId, &sArg1 );
}

sal_Int32 SwAccessibleParagraph::GetCaretPos()
{
    sal_Int32 nRet = -1;

    // get the selection's point, and test whether it's in our node
    SwPaM* pCaret = GetCrsr();  // caret is first PaM in PaM-ring
    if( pCaret != NULL )
    {
        const SwTxtNode* pNode = GetTxtNode();

        // check whether the point points into 'our' node
        SwPosition* pPoint = pCaret->GetPoint();
        if( pNode->GetIndex() == pPoint->nNode.GetIndex() )
        {
            // Yes, it's us!
            nRet = GetPortionData().GetAccessiblePosition(
                pPoint->nContent.GetIndex() );

            DBG_ASSERT( nRet >= 0, "invalid cursor?" );
            DBG_ASSERT( nRet <= GetPortionData().GetAccessibleString().
                                              getLength(), "invalid cursor?" );
        }
        // else: not in this paragraph
    }
    // else: no cursor -> no caret

    return nRet;
}

sal_Bool SwAccessibleParagraph::GetSelection(
    sal_Int32& nStart, sal_Int32& nEnd)
{
    sal_Bool bRet = sal_False;
    nStart = -1;
    nEnd = -1;

    // get the selection, and test whether it affects our text node
    SwPaM* pCrsr = GetCrsr();
    if( pCrsr != NULL )
    {
        // get SwPosition for my node
        const SwTxtNode* pNode = GetTxtNode();
        ULONG nHere = pNode->GetIndex();

        // iterate over ring
        SwPaM* pRingStart = pCrsr;
        do
        {
            // ignore, if no mark
            if( pCrsr->HasMark() )
            {
                // check whether nHere is 'inside' pCrsr
                SwPosition* pStart = pCrsr->Start();
                SwPosition* pEnd = pCrsr->End();
                if( ( nHere >= pStart->nNode.GetIndex() ) &&
                    ( nHere <= pEnd->nNode.GetIndex() )      )
                {
                    // Yup, we are selected!
                    bRet = sal_True;
                    nStart = static_cast<sal_Int32>(
                        ( nHere > pStart->nNode.GetIndex() ) ? 0
                        : pStart->nContent.GetIndex() );
                    nEnd = static_cast<sal_Int32>(
                        ( nHere < pEnd->nNode.GetIndex() ) ? pNode->Len()
                        : pEnd->nContent.GetIndex() );
                }
                // else: this PaM doesn't point to this paragraph
            }
            // else: this PaM is collapsed and doesn't select anything

            // next PaM in ring
            pCrsr = static_cast<SwPaM*>( pCrsr->GetNext() );
        }
        while( !bRet && (pCrsr != pRingStart) );
    }
    // else: nocursor -> no selection

    return bRet;
}

SwPaM* SwAccessibleParagraph::GetCrsr()
{
    // get the cursor shell; if we don't have any, we don't have a
    // cursor/selection either
    SwPaM* pCrsr = NULL;
    SwCrsrShell* pCrsrShell = SwAccessibleParagraph::GetCrsrShell();
    if( pCrsrShell != NULL )
    {
        // get the selection, and test whether it affects our text node
        pCrsr = pCrsrShell->GetCrsr( FALSE /* ??? */ );
    }

    return pCrsr;
}

SwCrsrShell* SwAccessibleParagraph::GetCrsrShell()
{
    // first, get the view shell
    DBG_ASSERT( GetMap() != NULL, "no map?" );
    ViewShell* pViewShell = GetMap()->GetShell();
    DBG_ASSERT( pViewShell != NULL,
                "No view shell? Then what are you looking at?" );

    SwCrsrShell* pCrsrShell = NULL;

    // see if our view shell is a cursor shell
    if( pViewShell->ISA( SwCrsrShell ) )
    {
        pCrsrShell = static_cast<SwCrsrShell*>( pViewShell );
    }

    return pCrsrShell;
}

sal_Bool SwAccessibleParagraph::IsHeading() const
{
    const SwTxtNode *pTxtNd = GetTxtNode();
    return (pTxtNd->GetOutlineNum() && !pTxtNd->GetNum());
}

void SwAccessibleParagraph::GetStates(
        ::utl::AccessibleStateSetHelper& rStateSet )
{
    SwAccessibleContext::GetStates( rStateSet );

    // MULTILINE
    rStateSet.AddState( AccessibleStateType::MULTILINE );

    // SELECTABLE
    rStateSet.AddState( AccessibleStateType::SELECTABLE );

    // TODO: SELECTED
}

void SwAccessibleParagraph::_InvalidateContent( sal_Bool bVisibleDataFired )
{
    OUString sOldText( GetString() );

    ClearPortionData();

    const OUString& rText = GetString();

    if( rText != sOldText )
    {
        // The text is changed
        AccessibleEventObject aEvent;
        aEvent.EventId = AccessibleEventId::ACCESSIBLE_TEXT_EVENT;

        FireAccessibleEvent( aEvent );
    }
    else if( !bVisibleDataFired )
    {
        FireVisibleDataEvent();
    }

    sal_Bool bNewIsHeading = IsHeading();
    sal_Bool bOldIsHeading;
    {
        vos::OGuard aGuard( aMutex );
        bOldIsHeading = bIsHeading;
        if( bIsHeading != bNewIsHeading )
            bIsHeading = bNewIsHeading;
    }


    if( bNewIsHeading != bOldIsHeading || rText != sOldText )
    {
        OUString sNewDesc( GetDescription() );
        OUString sOldDesc;
        {
            vos::OGuard aGuard( aMutex );
            sOldDesc = sDesc;
            if( sDesc != sNewDesc )
                sDesc = sNewDesc;
        }

        if( sNewDesc != sOldDesc )
        {
            // The text is changed
            AccessibleEventObject aEvent;
            aEvent.EventId = AccessibleEventId::ACCESSIBLE_DESCRIPTION_EVENT;
            aEvent.OldValue <<= sOldDesc;
            aEvent.NewValue <<= sNewDesc;

            FireAccessibleEvent( aEvent );
        }
    }
}

void SwAccessibleParagraph::_InvalidateCaretPos()
{
    // The text is changed
    sal_Int32 nNew = GetCaretPos();
    sal_Int32 nOld;
    {
        vos::OGuard aGuard( aMutex );
        nOld = nOldCaretPos;
        nOldCaretPos = nNew;
    }
    if( -1 != nNew )
    {
        ::vos::ORef < SwAccessibleContext > xThis( this );
        GetMap()->SetCaretContext( xThis );
    }

    if( nOld != nNew )
    {
        AccessibleEventObject aEvent;
        aEvent.EventId = AccessibleEventId::ACCESSIBLE_CARET_EVENT;
        aEvent.OldValue <<= nOld;
        aEvent.NewValue <<= nNew;

        FireAccessibleEvent( aEvent );
    }
}

SwAccessibleParagraph::SwAccessibleParagraph(
        SwAccessibleMap *pMap,
        sal_Int32 nPara,
        const SwTxtFrm *pTxtFrm ) :
    SwAccessibleContext( pMap, AccessibleRole::PARAGRAPH, pTxtFrm ),
    pPortionData( NULL ),
    nOldCaretPos( -1 )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    bIsHeading = IsHeading();
    sal_uInt16 nResId = bIsHeading ? STR_ACCESS_HEADING_NAME
                                   : STR_ACCESS_PARAGRAPH_NAME;
    OUString sArg( OUString::valueOf( nPara ) );
    SetName( GetResource( nResId, &sArg ) );
}

SwAccessibleParagraph::~SwAccessibleParagraph()
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    delete pPortionData;
}

void SwAccessibleParagraph::UpdatePortionData()
    throw( RuntimeException )
{
    // obtain the text frame
    DBG_ASSERT( GetFrm() != NULL, "The text frame has vanished!" );
    DBG_ASSERT( GetFrm()->IsTxtFrm(), "The text frame has mutated!" );
    const SwTxtFrm* pFrm = static_cast<const SwTxtFrm*>( GetFrm() );

    // build new portion data
    delete pPortionData;
    pPortionData = new SwAccessiblePortionData( pFrm->GetTxtNode() );
    pFrm->VisitPortions( *pPortionData );

    DBG_ASSERT( pPortionData != NULL, "UpdatePortionData() failed" );
}

void SwAccessibleParagraph::ClearPortionData()
{
    delete pPortionData;
    pPortionData = NULL;
}


void SwAccessibleParagraph::ExecuteAtViewShell( UINT16 nSlot )
{
    DBG_ASSERT( GetMap() != NULL, "no map?" );
    ViewShell* pViewShell = GetMap()->GetShell();

    DBG_ASSERT( pViewShell != NULL, "View shell exptected!" );
    SfxViewShell* pSfxShell = pViewShell->GetSfxViewShell();
    if( pSfxShell != NULL )
    {
        pSfxShell->GetDispatcher()->Execute( nSlot );
    }
}


//
// range checking for parameter
//

sal_Bool SwAccessibleParagraph::IsValidChar(
    sal_Int32 nPos, sal_Int32 nLength)
{
    return (nPos >= 0) && (nPos < nLength);
}

sal_Bool SwAccessibleParagraph::IsValidPosition(
    sal_Int32 nPos, sal_Int32 nLength)
{
    return (nPos >= 0) && (nPos <= nLength);
}

sal_Bool SwAccessibleParagraph::IsValidRange(
    sal_Int32 nBegin, sal_Int32 nEnd, sal_Int32 nLength)
{
    return (nBegin >= 0) && (nBegin <= nEnd) && (nEnd <= nLength);
}


//
// text boundaries
//


sal_Bool SwAccessibleParagraph::GetCharBoundary(
    Boundary& rBound,
    const OUString& rText,
    sal_Int32 nPos )
{
    rBound.startPos = nPos;
    rBound.endPos = nPos+1;
    return sal_True;
}

sal_Bool SwAccessibleParagraph::GetWordBoundary(
    Boundary& rBound,
    const OUString& rText,
    sal_Int32 nPos )
{
    sal_Bool bRet = sal_False;

    // now ask the Break-Iterator for the word
    DBG_ASSERT( pBreakIt != NULL, "We always need a break." );
    DBG_ASSERT( pBreakIt->xBreak.is(), "No break-iterator." );
    if( pBreakIt->xBreak.is() )
    {
        // get locale for this position
        USHORT nModelPos = GetPortionData().GetModelPosition( nPos );
        Locale aLocale = pBreakIt->GetLocale(
                              GetTxtNode()->GetLang( nModelPos ) );

        // which type of word are we interested in?
        // (DICTIONARY_WORD includes punctuation, ANY_WORD doesn't.)
        const USHORT nWordType = WordType::ANY_WORD;

        // get word boundary, as the Break-Iterator sees fit.
        rBound = pBreakIt->xBreak->getWordBoundary(
            rText, nPos, aLocale, nWordType, sal_True );

        // It's a word if the first character is an alpha-numeric character.
        bRet = GetAppCharClass().isLetterNumeric(
            rText.getStr()[ rBound.startPos ] );
    }
    else
    {
        // no break Iterator -> no word
        rBound.startPos = nPos;
        rBound.endPos = nPos;
    }

    return bRet;
}

sal_Bool SwAccessibleParagraph::GetSentenceBoundary(
    Boundary& rBound,
    const OUString& rText,
    sal_Int32 nPos )
{
    GetPortionData().GetSentenceBoundary( rBound, nPos );
    return sal_True;
}

sal_Bool SwAccessibleParagraph::GetLineBoundary(
    Boundary& rBound,
    const OUString& rText,
    sal_Int32 nPos )
{
    GetPortionData().GetLineBoundary( rBound, nPos );
    return sal_True;
}

sal_Bool SwAccessibleParagraph::GetParagraphBoundary(
    Boundary& rBound,
    const OUString& rText,
    sal_Int32 nPos )
{
    rBound.startPos = 0;
    rBound.endPos = rText.getLength();
    return sal_True;
}

sal_Bool SwAccessibleParagraph::GetAttributeBoundary(
    Boundary& rBound,
    const OUString& rText,
    sal_Int32 nPos )
{
    GetPortionData().GetAttributeBoundary( rBound, nPos );
    return sal_True;
}

void SwAccessibleParagraph::GetEmptyBoundary( Boundary& rBound )
{
    rBound.startPos = 0;
    rBound.endPos = 0;
}


sal_Bool SwAccessibleParagraph::GetTextBoundary(
    Boundary& rBound,
    const OUString& rText,
    sal_Int32 nPos,
    sal_Int16 nTextType )
    throw (
        IndexOutOfBoundsException,
        RuntimeException)
{
    // error checking
    if( ! IsValidChar( nPos, rText.getLength() ) )
        throw IndexOutOfBoundsException();

    sal_Bool bRet;

    switch( nTextType )
    {
        case AccessibleTextType::WORD:
            bRet = GetWordBoundary( rBound, rText, nPos );
            break;

        case AccessibleTextType::SENTENCE:
            bRet = GetSentenceBoundary( rBound, rText, nPos );
            break;

        case AccessibleTextType::PARAGRAPH:
            bRet = GetParagraphBoundary( rBound, rText, nPos );
            break;

        case AccessibleTextType::CHARACTER:
            bRet = GetCharBoundary( rBound, rText, nPos );
            break;

        case AccessibleTextType::LINE:
            bRet = GetLineBoundary( rBound, rText, nPos );
            break;

        case /* AccessibleTextType::ATTRIBUTE */ 6:
            bRet = GetAttributeBoundary( rBound, rText, nPos );
            break;

        default:
            // The specification asks us to return an empty string
            // if the text type is no valid.
            GetEmptyBoundary( rBound );
            bRet = sal_True;
            break;
    }

    return bRet;
}

OUString SAL_CALL SwAccessibleParagraph::getAccessibleDescription (void)
        throw (::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleContext );

    vos::OGuard aGuard2( aMutex );
    if( !sDesc.getLength() )
        sDesc = GetDescription();

    return sDesc;
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
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleContext );

    sal_Int32 nRet = GetCaretPos();
    {
        vos::OGuard aGuard( aMutex );
        nOldCaretPos = nRet;
    }
    if( -1 != nRet )
    {
        ::vos::ORef < SwAccessibleContext > xThis( this );
        GetMap()->SetCaretContext( xThis );
    }

    return nRet;
}

sal_Unicode SwAccessibleParagraph::getCharacter( sal_Int32 nIndex )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleContext );

    OUString sText( GetString() );

    // return character (if valid)
    if( IsValidChar(nIndex, sText.getLength() ) )
    {
        return sText.getStr()[nIndex];
    }
    else
        throw IndexOutOfBoundsException();
}

Sequence<PropertyValue> SwAccessibleParagraph::getCharacterAttributes(
    sal_Int32 nIndex )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    // HACK: dummy implementation
    Sequence<PropertyValue> aSeq;
    return aSeq;
}

com::sun::star::awt::Rectangle SwAccessibleParagraph::getCharacterBounds(
    sal_Int32 nIndex )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleContext ); // we have a frame?

    if( ! IsValidChar( nIndex, GetString().getLength() ) )
        throw IndexOutOfBoundsException();

    // get model position & prepare GetCharRect() arguments
    SwCrsrMoveState aMoveState;
    aMoveState.bRealHeight = TRUE;
    aMoveState.bRealWidth = TRUE;
    SwSpecialPos aSpecialPos;
    USHORT nPos = GetPortionData().FillSpecialPos(
        nIndex, aSpecialPos, aMoveState.pSpecialPos );

    // call GetCharRect
    SwRect aCoreRect;
    SwTxtNode* pNode = const_cast<SwTxtNode*>( GetTxtNode() );
    SwIndex aIndex( pNode, nPos );
    SwPosition aPosition( *pNode, aIndex );
    GetFrm()->GetCharRect( aCoreRect, aPosition, &aMoveState );

    // translate core coordinates into accessibility coordinates
    Window *pWin = GetWindow();
    CHECK_FOR_WINDOW( XAccessibleComponent, pWin );
    aCoreRect -= GetBounds().TopLeft();
    MapMode aMapMode = pWin->GetMapMode();
    aMapMode.SetOrigin( Point() );
    Rectangle aScreenRect( pWin->LogicToPixel( aCoreRect.SVRect(), aMapMode ));

    // convert into AWT Rectangle
    return com::sun::star::awt::Rectangle(
        aScreenRect.Left(), aScreenRect.Top(),
        aScreenRect.GetWidth(), aScreenRect.GetHeight() );
}

sal_Int32 SwAccessibleParagraph::getCharacterCount()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleContext );

    return GetString().getLength();
}

sal_Int32 SwAccessibleParagraph::getIndexAtPoint( const com::sun::star::awt::Point& rPoint )
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleContext );

    // construct SwPosition (where GetCrsrOfst() will put the result into)
    SwTxtNode* pNode = const_cast<SwTxtNode*>( GetTxtNode() );
    SwIndex aIndex( pNode, 0);
    SwPosition aPos( *pNode, aIndex );

    // construct Point (translate into layout coordinates)
    Window *pWin = GetWindow();
    CHECK_FOR_WINDOW( XAccessibleComponent, pWin );
    Point aPoint( rPoint.X, rPoint.Y );
    MapMode aMapMode = pWin->GetMapMode();
    aMapMode.SetOrigin( Point() );
    Point aCorePoint = pWin->PixelToLogic( aPoint, aMapMode );
    aCorePoint += GetBounds().TopLeft();

    // ask core for position
    DBG_ASSERT( GetFrm() != NULL, "The text frame has vanished!" );
    DBG_ASSERT( GetFrm()->IsTxtFrm(), "The text frame has mutated!" );
    const SwTxtFrm* pFrm = static_cast<const SwTxtFrm*>( GetFrm() );
    sal_Bool bSuccess = pFrm->GetCrsrOfst( &aPos, aCorePoint );

    return bSuccess ?
        GetPortionData().GetAccessiblePosition( aPos.nContent.GetIndex() )
        : 0;
}

OUString SwAccessibleParagraph::getSelectedText()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleContext ); // we have a frame?

    sal_Int32 nStart, nEnd;
    sal_Bool bSelected = GetSelection( nStart, nEnd );
    return bSelected ? GetString().copy( nStart, nEnd - nStart ) : OUString();
}

sal_Int32 SwAccessibleParagraph::getSelectionStart()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleContext ); // we have a frame?

    sal_Int32 nStart, nEnd;
    GetSelection( nStart, nEnd );
    return nStart;
}

sal_Int32 SwAccessibleParagraph::getSelectionEnd()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleContext ); // we have a frame?

    sal_Int32 nStart, nEnd;
    GetSelection( nStart, nEnd );
    return nEnd;
}

sal_Bool SwAccessibleParagraph::setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleContext );

    // parameter checking
    sal_Int32 nLength = GetString().getLength();
    if ( ! IsValidRange( nStartIndex, nEndIndex, nLength ) )
    {
        throw IndexOutOfBoundsException();
    }

    sal_Bool bRet = sal_False;

    // get cursor shell
    SwCrsrShell* pCrsrShell = GetCrsrShell();
    if( pCrsrShell != NULL )
    {
        // create pam for selection
        SwTxtNode* pNode = const_cast<SwTxtNode*>( GetTxtNode() );
        SwIndex aIndex( pNode, GetPortionData().GetModelPosition(nStartIndex));
        SwPosition aStartPos( *pNode, aIndex );
        SwPaM aPaM( aStartPos );
        aPaM.SetMark();
        aPaM.GetPoint()->nContent =
            GetPortionData().GetModelPosition(nEndIndex);

        // set PaM at cursor shell
        pCrsrShell->KillPams();
        pCrsrShell->SetSelection( aPaM );
        bRet = sal_True;
    }

    return bRet;
}

OUString SwAccessibleParagraph::getText()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleContext );

    return GetString();
}

OUString SwAccessibleParagraph::getTextRange(
    sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleContext );

    OUString sText( GetString() );

    if ( IsValidRange( nStartIndex, nEndIndex, sText.getLength() ) )
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
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleContext );

    const OUString rText = GetString();

    // with error checking
    Boundary aBound;
    sal_Bool bIsWord = GetTextBoundary( aBound, rText, nIndex, nTextType );

    DBG_ASSERT( aBound.startPos >= 0,               "illegal boundary" );
    DBG_ASSERT( aBound.startPos <= aBound.endPos,   "illegal boundary" );

    // return word (if present)
    return bIsWord ?
        rText.copy( aBound.startPos, aBound.endPos - aBound.startPos ) :
        OUString();
}

OUString SwAccessibleParagraph::getTextBeforeIndex(
    sal_Int32 nIndex, sal_Int16 nTextType )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleContext );

    const OUString rText = GetString();

    // get first word
    Boundary aBound;
    GetTextBoundary( aBound, rText, nIndex, nTextType );

    // now skip to previous word
    sal_Bool bWord = sal_False;
    while( !bWord )
    {
        nIndex = min( nIndex, aBound.startPos ) - 1;
        if( nIndex >= 0 )
            bWord = GetTextBoundary( aBound, rText, nIndex, nTextType );
        else
            break;  // exit if beginning of string is reached
    }

    return bWord ?
        rText.copy( aBound.startPos, aBound.endPos - aBound.startPos ) :
        OUString();
}

OUString SwAccessibleParagraph::getTextBehindIndex(
    sal_Int32 nIndex, sal_Int16 nTextType )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleContext );

    const OUString rText = GetString();

    // get first word, then skip to next word
    Boundary aBound;
    GetTextBoundary( aBound, rText, nIndex, nTextType );
    sal_Bool bWord = sal_False;
    while( !bWord )
    {
        nIndex = max( sal_Int32(nIndex+1), aBound.endPos );
        if( nIndex < rText.getLength() )
            bWord = GetTextBoundary( aBound, rText, nIndex, nTextType );
        else
            break;  // exit if end of string is reached
    }

    return bWord ?
        rText.copy( aBound.startPos, aBound.endPos - aBound.startPos ) :
        OUString();
}

sal_Bool SwAccessibleParagraph::copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    CHECK_FOR_DEFUNC( XAccessibleContext );
    vos::OGuard aGuard(Application::GetSolarMutex());

    // select and copy (through dispatch mechanism)
    setSelection( nStartIndex, nEndIndex );
    ExecuteAtViewShell( SID_COPY );
    return sal_True;
}


//
//=====  XAccesibleEditableText  ==========================================
//

sal_Bool SwAccessibleParagraph::cutText( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    CHECK_FOR_DEFUNC( XAccessibleContext );
    vos::OGuard aGuard(Application::GetSolarMutex());

    // select and cut (through dispatch mechanism)
    setSelection( nStartIndex, nEndIndex );
    ExecuteAtViewShell( SID_CUT );
    return sal_True;
}

sal_Bool SwAccessibleParagraph::pasteText( sal_Int32 nIndex )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    CHECK_FOR_DEFUNC( XAccessibleContext );
    vos::OGuard aGuard(Application::GetSolarMutex());

    // select and paste (through dispatch mechanism)
    setSelection( nIndex, nIndex );
    ExecuteAtViewShell( SID_PASTE );
    return sal_True;
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

    if( IsValidRange( nStartIndex, nEndIndex, rText.getLength() ) )
    {
        SwTxtNode* pNode = const_cast<SwTxtNode*>( GetTxtNode() );

        // create SwPosition for nStartIndex
        SwIndex aIndex( pNode, GetPortionData().GetModelPosition(nStartIndex));
        SwPosition aStartPos( *pNode, aIndex );

        // create SwPosition for nEndIndex
        SwPosition aEndPos( aStartPos );
        aEndPos.nContent = GetPortionData().GetModelPosition( nEndIndex );

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

