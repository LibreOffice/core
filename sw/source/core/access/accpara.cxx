 /*************************************************************************
 *
 *  $RCSfile: accpara.cxx,v $
 *
 *  $Revision: 1.57 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-19 10:55:34 $
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


#pragma hdrstop

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
#ifndef _FESH_HXX
#include "fesh.hxx"
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif
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

#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLESTATETYPE_HPP_
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLETEXTTYPE_HPP_
#include <com/sun/star/accessibility/AccessibleTextType.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTID_HPP_
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#endif

#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX_
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_CHARACTERITERATORMODE_HPP_
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
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
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XMULTIPROPERTYSET_HPP_
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYSTATE_HPP_
#include <com/sun/star/beans/PropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_UNKNOWNPROPERTYEXCEPTION_HPP_
#include <com/sun/star/beans/UnknownPropertyException.hpp>
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
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>      // for ExecuteAtViewShell(...)
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

// for get/setCharacterAttribute(...)
#ifndef _UNOCRSR_HXX
#include "unocrsr.hxx"
#endif
#ifndef _UNOOBJ_HXX
#include "unoobj.hxx"
#endif
#ifndef _UNOPORT_HXX
#include "unoport.hxx"
#endif
#ifndef _DOC_HXX
#include "doc.hxx"
#endif
#ifndef _CRSSKIP_HXX
#include "crsskip.hxx"
#endif
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif
#ifndef _TXTATR_HXX
#include <txtatr.hxx>
#endif
#ifndef _ACCHYPERLINK_HXX
#include <acchyperlink.hxx>
#endif
#ifndef _ACCHYPERTEXTDATA_HXX
#include <acchypertextdata.hxx>
#endif

#include <comphelper/accessibletexthelper.hxx>

#include <algorithm>

using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;
using namespace ::rtl;

using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::beans::XMultiPropertySet;
using ::com::sun::star::beans::UnknownPropertyException;
using ::com::sun::star::beans::PropertyState_DIRECT_VALUE;
using std::max;
using std::min;
using std::sort;

namespace com { namespace sun { namespace star {
    namespace text {
        class XText;
    }
} } }


const sal_Char sServiceName[] = "com.sun.star.text.AccessibleParagraphView";
const sal_Char sImplementationName[] = "com.sun.star.comp.Writer.SwAccessibleParagraphView";
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
            // same node? Then check whether it's also within 'our' part
            // of the paragraph
            USHORT nIndex = pPoint->nContent.GetIndex();
            if( GetPortionData().IsValidCorePosition( nIndex ) )
            {
                // Yes, it's us!
                nRet = GetPortionData().GetAccessiblePosition( nIndex );

                DBG_ASSERT( nRet >= 0, "invalid cursor?" );
                DBG_ASSERT( nRet <= GetPortionData().GetAccessibleString().
                                              getLength(), "invalid cursor?" );
            }
            // else: in this paragraph, but in different frame
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
                ULONG nStartIndex = pStart->nNode.GetIndex();
                SwPosition* pEnd = pCrsr->End();
                ULONG nEndIndex = pEnd->nNode.GetIndex();
                if( ( nHere >= nStartIndex ) &&
                    ( nHere <= nEndIndex )      )
                {
                    // translate start and end positions

                    // start position
                    sal_Int32 nLocalStart = -1;
                    if( nHere > nStartIndex )
                    {
                        // selection starts in previous node:
                        // then our local selection starts with the paragraph
                        nLocalStart = 0;
                    }
                    else
                    {
                        DBG_ASSERT( nHere == nStartIndex,
                                    "miscalculated index" );

                        // selection starts in this node:
                        // then check whether it's before or inside our part of
                        // the paragraph, and if so, get the proper position
                        USHORT nCoreStart = pStart->nContent.GetIndex();
                        if( nCoreStart <
                            GetPortionData().GetFirstValidCorePosition() )
                        {
                            nLocalStart = 0;
                        }
                        else if( nCoreStart <=
                                 GetPortionData().GetLastValidCorePosition() )
                        {
                            DBG_ASSERT(
                                GetPortionData().IsValidCorePosition(
                                                                  nCoreStart ),
                                 "problem determining valid core position" );

                            nLocalStart =
                                GetPortionData().GetAccessiblePosition(
                                                                  nCoreStart );
                        }
                    }

                    // end position
                    sal_Int32 nLocalEnd = -1;
                    if( nHere < nEndIndex )
                    {
                        // selection ends in following node:
                        // then our local selection extends to the end
                        nLocalEnd = GetPortionData().GetAccessibleString().
                                                                   getLength();
                    }
                    else
                    {
                        DBG_ASSERT( nHere == nStartIndex,
                                    "miscalculated index" );

                        // selection ends in this node: then select everything
                        // before our part of the node
                        USHORT nCoreEnd = pEnd->nContent.GetIndex();
                        if( nCoreEnd >
                                GetPortionData().GetLastValidCorePosition() )
                        {
                            // selection extends beyond out part of this para
                            nLocalEnd = GetPortionData().GetAccessibleString().
                                                                   getLength();
                        }
                        else if( nCoreEnd >=
                                 GetPortionData().GetFirstValidCorePosition() )
                        {
                            // selection is inside our part of this para
                            DBG_ASSERT(
                                GetPortionData().IsValidCorePosition(
                                                                  nCoreEnd ),
                                 "problem determining valid core position" );

                            nLocalEnd = GetPortionData().GetAccessiblePosition(
                                                                   nCoreEnd );
                        }
                    }

                    if( ( nLocalStart != -1 ) && ( nLocalEnd != -1 ) )
                    {
                        nStart = nLocalStart;
                        nEnd = nLocalEnd;
                        bRet = sal_True;
                    }
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
    if( pCrsrShell != NULL && !pCrsrShell->IsTableMode() )
    {
        SwFEShell *pFESh = pCrsrShell->ISA( SwFEShell )
                            ? static_cast< SwFEShell * >( pCrsrShell ) : 0;
        if( !pFESh ||
            !(pFESh->IsFrmSelected() || pFESh->IsObjSelected() > 0) )
        {
            // get the selection, and test whether it affects our text node
            pCrsr = pCrsrShell->GetCrsr( FALSE /* ??? */ );
        }
    }

    return pCrsr;
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
    rStateSet.AddState( AccessibleStateType::MULTI_LINE );

    // MULTISELECTABLE
    SwCrsrShell *pCrsrSh = GetCrsrShell();
    if( pCrsrSh )
        rStateSet.AddState( AccessibleStateType::MULTI_SELECTABLE );

    // FOCUSABLE
    if( pCrsrSh )
        rStateSet.AddState( AccessibleStateType::FOCUSABLE );

    // FOCUSED (simulates node index of cursor)
    SwPaM* pCaret = GetCrsr();
    const SwTxtNode* pTxtNd = GetTxtNode();
    if( pCaret != 0 && pTxtNd != 0 &&
        pTxtNd->GetIndex() == pCaret->GetPoint()->nNode.GetIndex() &&
        nOldCaretPos != -1)
    {
        Window *pWin = GetWindow();
        if( pWin && pWin->HasFocus() )
            rStateSet.AddState( AccessibleStateType::FOCUSED );
        ::vos::ORef < SwAccessibleContext > xThis( this );
        GetMap()->SetCursorContext( xThis );
    }
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
        aEvent.EventId = AccessibleEventId::TEXT_CHANGED;

        // determine exact changes between sOldText and rText
        comphelper::OCommonAccessibleText::implInitTextChangedEvent(
            sOldText, rText,
            aEvent.OldValue, aEvent.NewValue );

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
            aEvent.EventId = AccessibleEventId::DESCRIPTION_CHANGED;
            aEvent.OldValue <<= sOldDesc;
            aEvent.NewValue <<= sNewDesc;

            FireAccessibleEvent( aEvent );
        }
    }
}

void SwAccessibleParagraph::_InvalidateCursorPos()
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
        // remember that object as the one that has the caret. This is
        // neccessary to notify that object if the cursor leaves it.
        ::vos::ORef < SwAccessibleContext > xThis( this );
        GetMap()->SetCursorContext( xThis );
    }

    Window *pWin = GetWindow();
    if( nOld != nNew )
    {
        // The cursor's node position is sumilated by the focus!
        if( pWin && pWin->HasFocus() && -1 == nOld )
            FireStateChangedEvent( AccessibleStateType::FOCUSED, sal_True );


        AccessibleEventObject aEvent;
        aEvent.EventId = AccessibleEventId::CARET_CHANGED;
        aEvent.OldValue <<= nOld;
        aEvent.NewValue <<= nNew;

        FireAccessibleEvent( aEvent );

        if( pWin && pWin->HasFocus() && -1 == nNew )
            FireStateChangedEvent( AccessibleStateType::FOCUSED, sal_False );
    }
}

void SwAccessibleParagraph::_InvalidateFocus()
{
    Window *pWin = GetWindow();
    if( pWin )
    {
        sal_Int32 nPos;
        {
            vos::OGuard aGuard( aMutex );
            nPos = nOldCaretPos;
        }
        ASSERT( nPos != -1, "focus object should be selected" );

        FireStateChangedEvent( AccessibleStateType::FOCUSED,
                               pWin->HasFocus() && nPos != -1 );
    }
}

SwAccessibleParagraph::SwAccessibleParagraph(
        SwAccessibleMap *pMap,
        sal_Int32 nPara,
        const SwTxtFrm *pTxtFrm ) :
    SwAccessibleContext( pMap, AccessibleRole::PARAGRAPH, pTxtFrm ),
    pPortionData( NULL ),
    pHyperTextData( NULL ),
    nOldCaretPos( -1 ),
    aSelectionHelper( *this )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    bIsHeading = IsHeading();
    sal_uInt16 nResId = bIsHeading ? STR_ACCESS_HEADING_NAME
                                   : STR_ACCESS_PARAGRAPH_NAME;
    OUString sArg( OUString::valueOf( nPara ) );
    SetName( GetResource( nResId, &sArg ) );

    // If this object has the focus, then it is remembered by the map itself.
    nOldCaretPos = GetCaretPos();
}

SwAccessibleParagraph::~SwAccessibleParagraph()
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    delete pPortionData;
    delete pHyperTextData;
}

sal_Bool SwAccessibleParagraph::HasCursor()
{
    vos::OGuard aGuard( aMutex );
    return nOldCaretPos != -1;
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
    pPortionData = new SwAccessiblePortionData(
        pFrm->GetTxtNode(), GetMap()->GetShell()->GetViewOptions() );
    pFrm->VisitPortions( *pPortionData );

    DBG_ASSERT( pPortionData != NULL, "UpdatePortionData() failed" );
}

void SwAccessibleParagraph::ClearPortionData()
{
    delete pPortionData;
    pPortionData = NULL;

    delete pHyperTextData;
    pHyperTextData = 0;
}


void SwAccessibleParagraph::ExecuteAtViewShell( UINT16 nSlot )
{
    DBG_ASSERT( GetMap() != NULL, "no map?" );
    ViewShell* pViewShell = GetMap()->GetShell();

    DBG_ASSERT( pViewShell != NULL, "View shell exptected!" );
    SfxViewShell* pSfxShell = pViewShell->GetSfxViewShell();

    DBG_ASSERT( pSfxShell != NULL, "SfxViewShell shell exptected!" );
    if( !pSfxShell )
        return;

    SfxViewFrame *pFrame = pSfxShell->GetViewFrame();
    DBG_ASSERT( pFrame != NULL, "View frame exptected!" );
    if( !pFrame )
        return;

    SfxDispatcher *pDispatcher = pFrame->GetDispatcher();
    DBG_ASSERT( pDispatcher != NULL, "Dispatcher exptected!" );
    if( !pDispatcher )
        return;

    pDispatcher->Execute( nSlot );
}

SwXTextPortion* SwAccessibleParagraph::CreateUnoPortion(
    sal_Int32 nStartIndex,
    sal_Int32 nEndIndex )
{
    DBG_ASSERT( (IsValidChar(nStartIndex, GetString().getLength()) &&
                 (nEndIndex == -1)) ||
                IsValidRange(nStartIndex, nEndIndex, GetString().getLength()),
                "please check parameters before calling this method" );

    USHORT nStart = GetPortionData().GetModelPosition( nStartIndex );
    USHORT nEnd = (nEndIndex == -1) ? (nStart + 1) :
                        GetPortionData().GetModelPosition( nEndIndex );

    // create UNO cursor
    SwTxtNode* pTxtNode = const_cast<SwTxtNode*>( GetTxtNode() );
    SwIndex aIndex( pTxtNode, nStart );
    SwPosition aStartPos( *pTxtNode, aIndex );
    SwUnoCrsr* pUnoCursor = pTxtNode->GetDoc()->CreateUnoCrsr( aStartPos );
    pUnoCursor->SetMark();
    pUnoCursor->GetMark()->nContent = nEnd;

    // create a (dummy) text portion to be returned
    Reference<com::sun::star::text::XText> aEmpty;
    SwXTextPortion* pPortion =
        new SwXTextPortion ( pUnoCursor, aEmpty, PORTION_TEXT);
    delete pUnoCursor;

    return pPortion;
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
    return IsValidPosition(nBegin, nLength) && IsValidPosition(nEnd, nLength);
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
    if( rText.getLength() == nPos )
        GetPortionData().GetLastLineBoundary( rBound );
    else
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

sal_Bool SwAccessibleParagraph::GetGlyphBoundary(
    Boundary& rBound,
    const OUString& rText,
    sal_Int32 nPos )
{
    sal_Bool bRet = sal_False;

    // ask the Break-Iterator for the glyph by moving one cell
    // forward, and then one cell back
    DBG_ASSERT( pBreakIt != NULL, "We always need a break." );
    DBG_ASSERT( pBreakIt->xBreak.is(), "No break-iterator." );
    if( pBreakIt->xBreak.is() )
    {
        // get locale for this position
        USHORT nModelPos = GetPortionData().GetModelPosition( nPos );
        Locale aLocale = pBreakIt->GetLocale(
                              GetTxtNode()->GetLang( nModelPos ) );

        // get word boundary, as the Break-Iterator sees fit.
        const USHORT nIterMode = CharacterIteratorMode::SKIPCELL;
        sal_Int32 nDone = 0;
        rBound.endPos = pBreakIt->xBreak->nextCharacters(
             rText, nPos, aLocale, nIterMode, 1, nDone );
        rBound.startPos = pBreakIt->xBreak->previousCharacters(
             rText, rBound.endPos, aLocale, nIterMode, 1, nDone );

        DBG_ASSERT( rBound.startPos <= nPos, "start pos too high" );
        DBG_ASSERT( rBound.endPos >= nPos, "end pos too low" );
    }
    else
    {
        // no break Iterator -> no glyph
        rBound.startPos = nPos;
        rBound.endPos = nPos;
    }

    return bRet;
}


sal_Bool SwAccessibleParagraph::GetEmptyBoundary( Boundary& rBound )
{
    rBound.startPos = 0;
    rBound.endPos = 0;
    return sal_True;
}


sal_Bool SwAccessibleParagraph::GetTextBoundary(
    Boundary& rBound,
    const OUString& rText,
    sal_Int32 nPos,
    sal_Int16 nTextType )
    throw (
        IndexOutOfBoundsException,
        IllegalArgumentException,
        RuntimeException)
{
    // error checking
    if( !( AccessibleTextType::LINE == nTextType
                ? IsValidPosition( nPos, rText.getLength() )
                : IsValidChar( nPos, rText.getLength() ) ) )
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

        case AccessibleTextType::ATTRIBUTE_RUN:
            bRet = GetAttributeBoundary( rBound, rText, nPos );
            break;

        case AccessibleTextType::GLYPH:
            bRet = GetGlyphBoundary( rBound, rText, nPos );
            break;

        default:
            throw IllegalArgumentException( );
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
        throw (::com::sun::star::accessibility::IllegalAccessibleComponentStateException, ::com::sun::star::uno::RuntimeException)
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

void SAL_CALL SwAccessibleParagraph::grabFocus()
        throw (::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleContext );

    // get cursor shell
    SwCrsrShell *pCrsrSh = GetCrsrShell();
    SwPaM *pCrsr = GetCrsr();
    const SwTxtFrm *pTxtFrm = static_cast<const SwTxtFrm*>( GetFrm() );
    const SwTxtNode* pTxtNd = pTxtFrm->GetTxtNode();

    if( pCrsrSh != 0 && pTxtNd != 0 &&
        ( pCrsr == 0 ||
           pCrsr->GetPoint()->nNode.GetIndex() != pTxtNd->GetIndex() ||
          !pTxtFrm->IsInside( pCrsr->GetPoint()->nContent.GetIndex()) ) )
    {
        // create pam for selection
        SwIndex aIndex( const_cast< SwTxtNode * >( pTxtNd ),
                        pTxtFrm->GetOfst() );
        SwPosition aStartPos( *pTxtNd, aIndex );
        SwPaM aPaM( aStartPos );

        // set PaM at cursor shell
        Select( aPaM );


    }

    /* ->#i13955# */
    Window * pWindow = GetWindow();

    if (pWindow != NULL)
        pWindow->GrabFocus();
    /* <-#i13955# */
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
    return sTestServiceName.equalsAsciiL( sServiceName,
                                          sizeof(sServiceName)-1 ) ||
           sTestServiceName.equalsAsciiL( sAccessibleServiceName,
                                             sizeof(sAccessibleServiceName)-1 );
}

Sequence< OUString > SAL_CALL SwAccessibleParagraph::getSupportedServiceNames()
        throw( ::com::sun::star::uno::RuntimeException )
{
    Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = OUString( RTL_CONSTASCII_USTRINGPARAM(sServiceName) );
    pArray[1] = OUString( RTL_CONSTASCII_USTRINGPARAM(sAccessibleServiceName) );
    return aRet;
}

Sequence<OUString> getAttributeNames()
{
    static Sequence<OUString>* pNames = NULL;

    if( pNames == NULL )
    {
        Sequence<OUString>* pSeq = new Sequence<OUString>( 15 );
        OUString* pStrings = pSeq->getArray();

        // sorted list of strings
        sal_Int32 i = 0;

#define CHAR_BACK_COLOR_POS 0

#define STR(x) pStrings[i++] = OUString(RTL_CONSTASCII_USTRINGPARAM(x))
        STR("CharBackColor");
        STR("CharColor");
        STR("CharEscapement");
        STR("CharHeight");
        STR("CharPosture");
        STR("CharStrikeout");
        STR("CharUnderline");
        STR("CharWeight");
        STR("ParaAdjust");
        STR("ParaBottomMargin");
        STR("ParaFirstLineIndent");
        STR("ParaLeftMargin");
        STR("ParaLineSpacing");
        STR("ParaRightMargin");
        STR("ParaTabStops");
#undef STR

        DBG_ASSERT( i == pSeq->getLength(), "Please adjust length" );
        if( i != pSeq->getLength() )
            pSeq->realloc( i );

        pNames = pSeq;
    }

    return *pNames;
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
        Reference<XAccessibleText> aAccText = (XAccessibleText *) *this; // resolve ambiguity
        aRet <<= aAccText;
    }
    else if ( rType == ::getCppuType((Reference<XAccessibleEditableText> *)0) )
    {
        Reference<XAccessibleEditableText> aAccEditText = this;
        aRet <<= aAccEditText;
    }
    else if ( rType == ::getCppuType((Reference<XAccessibleSelection> *)0) )
    {
        Reference<XAccessibleSelection> aAccSel = this;
        aRet <<= aAccSel;
    }
    else if ( rType == ::getCppuType((Reference<XAccessibleHypertext> *)0) )
    {
        Reference<XAccessibleHypertext> aAccHyp = this;
        aRet <<= aAccHyp;
    }
    else
    {
        aRet = SwAccessibleContext::queryInterface(rType);
    }

    return aRet;
}

//====== XTypeProvider ====================================================
Sequence< Type > SAL_CALL SwAccessibleParagraph::getTypes() throw(RuntimeException)
{
    Sequence< Type > aTypes( SwAccessibleContext::getTypes() );

    sal_Int32 nIndex = aTypes.getLength();
    aTypes.realloc( nIndex + 3 );

    Type* pTypes = aTypes.getArray();
    pTypes[nIndex++] = ::getCppuType( static_cast< Reference< XAccessibleEditableText > * >( 0 ) );
    pTypes[nIndex++] = ::getCppuType( static_cast< Reference< XAccessibleSelection > * >( 0 ) );
    pTypes[nIndex] = ::getCppuType( static_cast< Reference< XAccessibleHypertext > * >( 0 ) );

    return aTypes;
}

Sequence< sal_Int8 > SAL_CALL SwAccessibleParagraph::getImplementationId()
        throw(RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    static Sequence< sal_Int8 > aId( 16 );
    static sal_Bool bInit = sal_False;
    if(!bInit)
    {
        rtl_createUuid( (sal_uInt8 *)(aId.getArray() ), 0, sal_True );
        bInit = sal_True;
    }
    return aId;
}


//
//=====  XAccesibleText  ===================================================
//

sal_Int32 SwAccessibleParagraph::getCaretPosition()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    sal_Int32 nRet = GetCaretPos();
    {
        vos::OGuard aGuard( aMutex );
        ASSERT( nRet == nOldCaretPos, "caret pos out of sync" );
        nOldCaretPos = nRet;
    }
    if( -1 != nRet )
    {
        ::vos::ORef < SwAccessibleContext > xThis( this );
        GetMap()->SetCursorContext( xThis );
    }

    return nRet;
}

sal_Bool SAL_CALL SwAccessibleParagraph::setCaretPosition( sal_Int32 nIndex )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    // parameter checking
    sal_Int32 nLength = GetString().getLength();
    if ( ! IsValidPosition( nIndex, nLength ) )
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
        SwIndex aIndex( pNode, GetPortionData().GetModelPosition(nIndex));
        SwPosition aStartPos( *pNode, aIndex );
        SwPaM aPaM( aStartPos );

        // set PaM at cursor shell
        bRet = Select( aPaM );
    }

    return bRet;
}

sal_Unicode SwAccessibleParagraph::getCharacter( sal_Int32 nIndex )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

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
    sal_Int32 nIndex, const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aRequestedAttributes )
    throw (IndexOutOfBoundsException, RuntimeException)
{

    vos::OGuard aGuard(Application::GetSolarMutex());
    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    const OUString& rText = GetString();

    if( ! IsValidChar( nIndex, rText.getLength() ) )
        throw IndexOutOfBoundsException();

    // create a (dummy) text portion for the sole purpose of calling
    // getPropertyValues on it
    Reference<XMultiPropertySet> xPortion = CreateUnoPortion( nIndex, nIndex );

    // get values
    Sequence<OUString> aNames = getAttributeNames();
    sal_Int32 nLength = aNames.getLength();
    Sequence<Any> aAnys( nLength );
    aAnys = xPortion->getPropertyValues( aNames );

    // copy names + anys into return sequence
    Sequence<PropertyValue> aValues( aNames.getLength() );
    const OUString* pNames = aNames.getConstArray();
    const Any* pAnys = aAnys.getConstArray();
    PropertyValue* pValues = aValues.getArray();
    for( sal_Int32 i = 0; i < nLength; i++ )
    {
        PropertyValue& rValue = pValues[i];
        rValue.Name = pNames[i];
        rValue.Value = pAnys[i];
        rValue.Handle = -1;                         // handle not supported
        rValue.State = PropertyState_DIRECT_VALUE;  // states not supported
    }

    // adjust background color if we're in a gray portion
    DBG_ASSERT( pValues[CHAR_BACK_COLOR_POS].Name.
                equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("CharBackColor")),
                "Please adjust CHAR_BACK_COLOR_POS constant." );
    if( GetPortionData().IsInGrayPortion( nIndex ) )
        pValues[CHAR_BACK_COLOR_POS].Value <<= SwViewOption::GetFieldShadingsColor().GetColor();

    return aValues;
}

com::sun::star::awt::Rectangle SwAccessibleParagraph::getCharacterBounds(
    sal_Int32 nIndex )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );


    /*  #i12332# The position after the string needs special treatment.
        IsValidChar -> IsValidPosition
    */
    if( ! (IsValidPosition( nIndex, GetString().getLength() ) ) )
        throw IndexOutOfBoundsException();

    /*  #i12332#  */
    sal_Bool bBehindText = sal_False;
    if ( nIndex == GetString().getLength() )
        bBehindText = sal_True;

    // get model position & prepare GetCharRect() arguments
    SwCrsrMoveState aMoveState;
    aMoveState.bRealHeight = TRUE;
    aMoveState.bRealWidth = TRUE;
    SwSpecialPos aSpecialPos;
    SwTxtNode* pNode = const_cast<SwTxtNode*>( GetTxtNode() );

    USHORT nPos = 0;

    /*  #i12332# FillSpecialPos does not accept nIndex ==
         GetString().getLength(). In that case nPos is set to the
         length of the string in the core. This way GetCharRect
         returns the rectangle for a cursor at the end of the
         paragraph. */
    if (bBehindText)
    {
        nPos = pNode->GetTxt().Len();
    }
    else
        nPos = GetPortionData().FillSpecialPos
            (nIndex, aSpecialPos, aMoveState.pSpecialPos );

    // call GetCharRect
    SwRect aCoreRect;
    SwIndex aIndex( pNode, nPos );
    SwPosition aPosition( *pNode, aIndex );
    GetFrm()->GetCharRect( aCoreRect, aPosition, &aMoveState );

    // translate core coordinates into accessibility coordinates
    Window *pWin = GetWindow();
    CHECK_FOR_WINDOW( XAccessibleComponent, pWin );

    Rectangle aScreenRect( GetMap()->CoreToPixel( aCoreRect.SVRect() ));
    SwRect aFrmLogBounds( GetBounds() ); // twip rel to doc root

    Point aFrmPixPos( GetMap()->CoreToPixel( aFrmLogBounds.SVRect() ).TopLeft() );
    aScreenRect.Move( -aFrmPixPos.X(), -aFrmPixPos.Y() );

    // convert into AWT Rectangle
    return com::sun::star::awt::Rectangle(
        aScreenRect.Left(), aScreenRect.Top(),
        aScreenRect.GetWidth(), aScreenRect.GetHeight() );
}

sal_Int32 SwAccessibleParagraph::getCharacterCount()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    return GetString().getLength();
}

sal_Int32 SwAccessibleParagraph::getIndexAtPoint( const com::sun::star::awt::Point& rPoint )
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());


    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    // construct SwPosition (where GetCrsrOfst() will put the result into)
    SwTxtNode* pNode = const_cast<SwTxtNode*>( GetTxtNode() );
    SwIndex aIndex( pNode, 0);
    SwPosition aPos( *pNode, aIndex );

    // construct Point (translate into layout coordinates)
    Window *pWin = GetWindow();
    CHECK_FOR_WINDOW( XAccessibleComponent, pWin );
    Point aPoint( rPoint.X, rPoint.Y );
    SwRect aLogBounds( GetBounds( GetFrm() ) ); // twip rel to doc root
    Point aPixPos( GetMap()->CoreToPixel( aLogBounds.SVRect() ).TopLeft() );
    aPoint.X() += aPixPos.X();
    aPoint.Y() += aPixPos.Y();
    MapMode aMapMode = pWin->GetMapMode();
    Point aCorePoint( GetMap()->PixelToCore( aPoint ) );
    if( !aLogBounds.IsInside( aCorePoint ) )
    {
        /* #i12332# rPoint is may also be in rectangle returned by
            getCharacterBounds(getCharacterCount() */

        com::sun::star::awt::Rectangle aRectEndPos =
            getCharacterBounds(getCharacterCount());

        if (rPoint.X - aRectEndPos.X >= 0 &&
            rPoint.X - aRectEndPos.X < aRectEndPos.Width &&
            rPoint.Y - aRectEndPos.Y >= 0 &&
            rPoint.Y - aRectEndPos.Y < aRectEndPos.Height)
            return getCharacterCount();

        return -1;
    }

    // ask core for position
    DBG_ASSERT( GetFrm() != NULL, "The text frame has vanished!" );
    DBG_ASSERT( GetFrm()->IsTxtFrm(), "The text frame has mutated!" );
    const SwTxtFrm* pFrm = static_cast<const SwTxtFrm*>( GetFrm() );
    SwCrsrMoveState aMoveState;
    aMoveState.bPosMatchesBounds = TRUE;
    sal_Bool bSuccess = pFrm->GetCrsrOfst( &aPos, aCorePoint, &aMoveState );

    return bSuccess ?
        GetPortionData().GetAccessiblePosition( aPos.nContent.GetIndex() )
        : -1L;
}

OUString SwAccessibleParagraph::getSelectedText()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    sal_Int32 nStart, nEnd;
    sal_Bool bSelected = GetSelection( nStart, nEnd );
    return bSelected ? GetString().copy( nStart, nEnd - nStart ) : OUString();
}

sal_Int32 SwAccessibleParagraph::getSelectionStart()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    sal_Int32 nStart, nEnd;
    GetSelection( nStart, nEnd );
    return nStart;
}

sal_Int32 SwAccessibleParagraph::getSelectionEnd()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    sal_Int32 nStart, nEnd;
    GetSelection( nStart, nEnd );
    return nEnd;
}

sal_Bool SwAccessibleParagraph::setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

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
        bRet = Select( aPaM );
    }

    return bRet;
}

OUString SwAccessibleParagraph::getText()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    return GetString();
}

OUString SwAccessibleParagraph::getTextRange(
    sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    OUString sText( GetString() );

    if ( IsValidRange( nStartIndex, nEndIndex, sText.getLength() ) )
    {
        OrderRange( nStartIndex, nEndIndex );
        return sText.copy(nStartIndex, nEndIndex-nStartIndex );
    }
    else
        throw IndexOutOfBoundsException();
}

::com::sun::star::accessibility::TextSegment SwAccessibleParagraph::getTextAtIndex( sal_Int32 nIndex, sal_Int16 nTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    ::com::sun::star::accessibility::TextSegment aResult;
    aResult.SegmentStart = -1;
    aResult.SegmentEnd = -1;

    const OUString rText = GetString();
    // implement the silly specification that first position after
    // text must return an empty string, rather than throwing an
    // IndexOutOfBoundsException, except for LINE, where the last
    // line is returned
    if( nIndex == rText.getLength() && AccessibleTextType::LINE != nTextType )
        return aResult;

    // with error checking
    Boundary aBound;
    sal_Bool bWord = GetTextBoundary( aBound, rText, nIndex, nTextType );

    DBG_ASSERT( aBound.startPos >= 0,               "illegal boundary" );
    DBG_ASSERT( aBound.startPos <= aBound.endPos,   "illegal boundary" );

    // return word (if present)
    if ( bWord )
    {
        aResult.SegmentText = rText.copy( aBound.startPos, aBound.endPos - aBound.startPos );
        aResult.SegmentStart = aBound.startPos;
        aResult.SegmentEnd = aBound.endPos;
    }

    return aResult;
}

::com::sun::star::accessibility::TextSegment SwAccessibleParagraph::getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 nTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    const OUString rText = GetString();

    ::com::sun::star::accessibility::TextSegment aResult;
    aResult.SegmentStart = -1;
    aResult.SegmentEnd = -1;

    // get starting pos
    Boundary aBound;
    if (nIndex ==  rText.getLength())
        aBound.startPos = aBound.endPos = nIndex;
    else
    {
        sal_Bool bTmp = GetTextBoundary( aBound, rText, nIndex, nTextType );

        if ( ! bTmp )
            aBound.startPos = aBound.endPos = nIndex;
    }

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

    if ( bWord )
    {
        aResult.SegmentText = rText.copy( aBound.startPos, aBound.endPos - aBound.startPos );
        aResult.SegmentStart = aBound.startPos;
        aResult.SegmentEnd = aBound.endPos;
    };
    return aResult;
}

::com::sun::star::accessibility::TextSegment SwAccessibleParagraph::getTextBehindIndex( sal_Int32 nIndex, sal_Int16 nTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    ::com::sun::star::accessibility::TextSegment aResult;
    aResult.SegmentStart = -1;
    aResult.SegmentEnd = -1;
    const OUString rText = GetString();

    // implement the silly specification that first position after
    // text must return an empty string, rather than throwing an
    // IndexOutOfBoundsException
    if( nIndex == rText.getLength() )
        return aResult;


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

    if ( bWord )
    {
        aResult.SegmentText = rText.copy( aBound.startPos, aBound.endPos - aBound.startPos );
        aResult.SegmentStart = aBound.startPos;
        aResult.SegmentEnd = aBound.endPos;
    }
    return aResult;
}

sal_Bool SwAccessibleParagraph::copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );
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
    CHECK_FOR_DEFUNC( XAccessibleEditableText );
    vos::OGuard aGuard(Application::GetSolarMutex());

    if( !IsEditableState() )
        return sal_False;

    // select and cut (through dispatch mechanism)
    setSelection( nStartIndex, nEndIndex );
    ExecuteAtViewShell( SID_CUT );
    return sal_True;
}

sal_Bool SwAccessibleParagraph::pasteText( sal_Int32 nIndex )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    CHECK_FOR_DEFUNC( XAccessibleEditableText );
    vos::OGuard aGuard(Application::GetSolarMutex());

    if( !IsEditableState() )
        return sal_False;

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

    CHECK_FOR_DEFUNC( XAccessibleEditableText );

    const OUString& rText = GetString();

    if( IsValidRange( nStartIndex, nEndIndex, rText.getLength() ) )
    {
        if( !IsEditableState() )
            return sal_False;

        SwTxtNode* pNode = const_cast<SwTxtNode*>( GetTxtNode() );

        // translate positions
        USHORT nStart, nEnd;
        sal_Bool bSuccess = GetPortionData().GetEditableRange(
                                        nStartIndex, nEndIndex, nStart, nEnd );

        // edit only if the range is editable
        if( bSuccess )
        {
            // create SwPosition for nStartIndex
            SwIndex aIndex( pNode, nStart );
            SwPosition aStartPos( *pNode, aIndex );

            // create SwPosition for nEndIndex
            SwPosition aEndPos( aStartPos );
            aEndPos.nContent = nEnd;

            // now create XTextRange as helper and set string
            SwXTextRange::CreateTextRangeFromPosition(
                pNode->GetDoc(), aStartPos, &aEndPos)->setString(sReplacement);

            // delete portion data
            ClearPortionData();
        }

        return bSuccess;
    }
    else
        throw IndexOutOfBoundsException();
}

struct IndexCompare
{
    const PropertyValue* pValues;
    IndexCompare( const PropertyValue* pVals ) : pValues(pVals) {}
    bool operator() ( const sal_Int32& a, const sal_Int32& b ) const
    {
        return (pValues[a].Name < pValues[b].Name) ? true : false;
    }
};


sal_Bool SwAccessibleParagraph::setAttributes(
    sal_Int32 nStartIndex,
    sal_Int32 nEndIndex,
    const Sequence<PropertyValue>& rAttributeSet )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    CHECK_FOR_DEFUNC( XAccessibleEditableText );

    const OUString& rText = GetString();

    if( ! IsValidRange( nStartIndex, nEndIndex, rText.getLength() ) )
        throw IndexOutOfBoundsException();

    if( !IsEditableState() )
        return sal_False;


    // create a (dummy) text portion for the sole purpose of calling
    // setPropertyValue on it
    Reference<XMultiPropertySet> xPortion = CreateUnoPortion( nStartIndex,
                                                              nEndIndex );

    // build sorted index array
    sal_Int32 nLength = rAttributeSet.getLength();
    const PropertyValue* pPairs = rAttributeSet.getConstArray();
    sal_Int32* pIndices = new sal_Int32[nLength];
    sal_Int32 i;
    for( i = 0; i < nLength; i++ )
        pIndices[i] = i;
    sort( &pIndices[0], &pIndices[nLength], IndexCompare(pPairs) );

    // create sorted sequences accoring to index array
    Sequence<OUString> aNames( nLength );
    OUString* pNames = aNames.getArray();
    Sequence<Any> aValues( nLength );
    Any* pValues = aValues.getArray();
    for( i = 0; i < nLength; i++ )
    {
        const PropertyValue& rVal = pPairs[pIndices[i]];
        pNames[i]  = rVal.Name;
        pValues[i] = rVal.Value;
    }
    delete[] pIndices;

    // now set the values
    sal_Bool bRet = sal_True;
    try
    {
        xPortion->setPropertyValues( aNames, aValues );
    }
    catch( UnknownPropertyException e )
    {
        // error handling through return code!
        bRet = sal_False;
    }

    return bRet;
}

sal_Bool SwAccessibleParagraph::setText( const OUString& sText )
    throw (RuntimeException)
{
    return replaceText(0, GetString().getLength(), sText);
}

//=====  XAccessibleSelection  ============================================

void SwAccessibleParagraph::selectAccessibleChild(
    sal_Int32 nChildIndex )
    throw ( IndexOutOfBoundsException,
            RuntimeException )
{
    CHECK_FOR_DEFUNC( XAccessibleSelection );

    aSelectionHelper.selectAccessibleChild(nChildIndex);
}

sal_Bool SwAccessibleParagraph::isAccessibleChildSelected(
    sal_Int32 nChildIndex )
    throw ( IndexOutOfBoundsException,
            RuntimeException )
{
    CHECK_FOR_DEFUNC( XAccessibleSelection );

    return aSelectionHelper.isAccessibleChildSelected(nChildIndex);
}

void SwAccessibleParagraph::clearAccessibleSelection(  )
    throw ( RuntimeException )
{
    CHECK_FOR_DEFUNC( XAccessibleSelection );

    aSelectionHelper.clearAccessibleSelection();
}

void SwAccessibleParagraph::selectAllAccessibleChildren(  )
    throw ( RuntimeException )
{
    CHECK_FOR_DEFUNC( XAccessibleSelection );

    aSelectionHelper.selectAllAccessibleChildren();
}

sal_Int32 SwAccessibleParagraph::getSelectedAccessibleChildCount(  )
    throw ( RuntimeException )
{
    CHECK_FOR_DEFUNC( XAccessibleSelection );

    return aSelectionHelper.getSelectedAccessibleChildCount();
}

Reference<XAccessible> SwAccessibleParagraph::getSelectedAccessibleChild(
    sal_Int32 nSelectedChildIndex )
    throw ( IndexOutOfBoundsException,
            RuntimeException)
{
    CHECK_FOR_DEFUNC( XAccessibleSelection );

    return aSelectionHelper.getSelectedAccessibleChild(nSelectedChildIndex);
}

void SwAccessibleParagraph::deselectAccessibleChild(
    sal_Int32 nSelectedChildIndex )
    throw ( IndexOutOfBoundsException,
            RuntimeException )
{
    CHECK_FOR_DEFUNC( XAccessibleSelection );

    aSelectionHelper.deselectAccessibleChild(nSelectedChildIndex);
}

//=====  XAccessibleHypertext  ============================================

class SwHyperlinkIter_Impl
{
    const SwpHints *pHints;
    xub_StrLen nStt;
    xub_StrLen nEnd;
    sal_uInt16 nPos;

public:
    SwHyperlinkIter_Impl( const SwTxtFrm *pTxtFrm );
    const SwTxtAttr *next();
    sal_uInt16 getCurrHintPos() const { return nPos-1; }

    xub_StrLen startIdx() const { return nStt; }
    xub_StrLen endIdx() const { return nEnd; }
};

SwHyperlinkIter_Impl::SwHyperlinkIter_Impl( const SwTxtFrm *pTxtFrm ) :
    pHints( pTxtFrm->GetTxtNode()->GetpSwpHints() ),
    nStt( pTxtFrm->GetOfst() ),
    nPos( 0 )
{
    const SwTxtFrm *pFollFrm = pTxtFrm->GetFollow();
    nEnd = pFollFrm ? pFollFrm->GetOfst() : pTxtFrm->GetTxtNode()->Len();
}

const SwTxtAttr *SwHyperlinkIter_Impl::next()
{
    const SwTxtAttr *pAttr = 0;
    if( pHints )
    {
        while( !pAttr && nPos < pHints->Count() )
        {
            const SwTxtAttr *pHt = (*pHints)[nPos];
            if( RES_TXTATR_INETFMT == pHt->Which() )
            {
                xub_StrLen nHtStt = *pHt->GetStart();
                xub_StrLen nHtEnd = *pHt->GetAnyEnd();
                if( nHtEnd > nHtStt &&
                    ( (nHtStt >= nStt && nHtStt < nEnd) ||
                      (nHtEnd > nStt && nHtEnd <= nEnd) ) )
                {
                    pAttr = pHt;
                }
            }
            ++nPos;
        }
    }

    return pAttr;
};

sal_Int32 SAL_CALL SwAccessibleParagraph::getHyperLinkCount()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleHypertext );

    sal_Int32 nCount = 0;
    if( !IsEditableState() )
    {
        const SwTxtFrm *pTxtFrm = static_cast<const SwTxtFrm*>( GetFrm() );
        SwHyperlinkIter_Impl aIter( pTxtFrm );
        while( aIter.next() )
            nCount++;
    }

    return nCount;
}

Reference< XAccessibleHyperlink > SAL_CALL
    SwAccessibleParagraph::getHyperLink( sal_Int32 nLinkIndex )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    CHECK_FOR_DEFUNC( XAccessibleHypertext );

    Reference< XAccessibleHyperlink > xRet;

    if( !IsEditableState() )
    {
        const SwTxtFrm *pTxtFrm = static_cast<const SwTxtFrm*>( GetFrm() );
        SwHyperlinkIter_Impl aHIter( pTxtFrm );
        while( nLinkIndex-- )
            aHIter.next();

        const SwTxtAttr *pHt = aHIter.next();
        if( pHt )
        {
            if( !pHyperTextData )
                pHyperTextData = new SwAccessibleHyperTextData;
            SwAccessibleHyperTextData::iterator aIter =
                pHyperTextData ->find( pHt );
            if( aIter != pHyperTextData->end() )
            {
                xRet = (*aIter).second;
            }
            if( !xRet.is() )
            {
                sal_Int32 nHStt= GetPortionData().GetAccessiblePosition(
                                max( aHIter.startIdx(), *pHt->GetStart() ) );
                sal_Int32 nHEnd= GetPortionData().GetAccessiblePosition(
                                min( aHIter.endIdx(), *pHt->GetAnyEnd() ) );
                xRet = new SwAccessibleHyperlink( aHIter.getCurrHintPos(),
                                                  this, nHStt, nHEnd );
                if( aIter != pHyperTextData->end() )
                {
                    (*aIter).second = xRet;
                }
                else
                {
                    SwAccessibleHyperTextData::value_type aEntry( pHt, xRet );
                    pHyperTextData->insert( aEntry );
                }
            }
        }
    }

    if( !xRet.is() )
        throw IndexOutOfBoundsException();

    return xRet;
}

sal_Int32 SAL_CALL SwAccessibleParagraph::getHyperLinkIndex( sal_Int32 nCharIndex )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    CHECK_FOR_DEFUNC( XAccessibleHypertext );

    // parameter checking
    sal_Int32 nLength = GetString().getLength();
    if ( ! IsValidPosition( nCharIndex, nLength ) )
    {
        throw IndexOutOfBoundsException();
    }

    sal_Int32 nRet = -1;
    if( !IsEditableState() )
    {
        const SwTxtFrm *pTxtFrm = static_cast<const SwTxtFrm*>( GetFrm() );
        SwHyperlinkIter_Impl aHIter( pTxtFrm );

        xub_StrLen nIdx = GetPortionData().GetModelPosition( nCharIndex );
        sal_Int32 nPos = 0;
        const SwTxtAttr *pHt = aHIter.next();
        while( pHt && !(nIdx >= *pHt->GetStart() && nIdx < *pHt->GetAnyEnd()) )
        {
            pHt = aHIter.next();
            nPos++;
        }

        if( pHt )
            nRet = nPos;

    }

    return nRet;
}
