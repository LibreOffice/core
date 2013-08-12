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


#include <txtfrm.hxx>
#include <flyfrm.hxx>
#include <ndtxt.hxx>
#include <pam.hxx>
#include <unotextrange.hxx>
#include <unocrsrhelper.hxx>
#include <crstate.hxx>
#include <accmap.hxx>
#include <fesh.hxx>
#include <viewopt.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleTextType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <unotools/accessiblestatesethelper.hxx>
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#include <breakit.hxx>
#include <accpara.hxx>
#include <access.hrc>
#include <accportions.hxx>
#include <sfx2/viewsh.hxx>          // for ExecuteAtViewShell(...)
#include <sfx2/viewfrm.hxx>         // for ExecuteAtViewShell(...)
#include <sfx2/dispatch.hxx>        // for ExecuteAtViewShell(...)
#include <unotools/charclass.hxx>   // for GetWordBoundary
#include <unocrsr.hxx>
#include <unoport.hxx>
#include <doc.hxx>
#include <crsskip.hxx>
#include <txtatr.hxx>
#include <acchyperlink.hxx>
#include <acchypertextdata.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <comphelper/accessibletexthelper.hxx>
#include <unomap.hxx>
#include <unoprnms.hxx>
#include <com/sun/star/text/WritingMode2.hpp>
#include <editeng/brushitem.hxx>
#include <viewimp.hxx>
#include <boost/scoped_ptr.hpp>
#include <textmarkuphelper.hxx>
// #i10825#
#include <parachangetrackinginfo.hxx>
#include <com/sun/star/text/TextMarkupType.hpp>
#include <comphelper/stlunosequence.hxx> // #i92233#
#include <comphelper/servicehelper.hxx>

#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

using beans::PropertyValue;
using beans::XMultiPropertySet;
using beans::UnknownPropertyException;
using beans::PropertyState_DIRECT_VALUE;

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
    OSL_ENSURE( pFrm->IsTxtFrm(), "The text frame has mutated!" );

    const SwTxtNode* pNode = static_cast<const SwTxtFrm*>(pFrm)->GetTxtNode();
    OSL_ENSURE( pNode != NULL, "A text frame without a text node." );

    return pNode;
}

OUString SwAccessibleParagraph::GetString()
{
    return GetPortionData().GetAccessibleString();
}

OUString SwAccessibleParagraph::GetDescription()
{
    return OUString(); // provide empty description for paragraphs
}

sal_Int32 SwAccessibleParagraph::GetCaretPos()
{
    sal_Int32 nRet = -1;

    // get the selection's point, and test whether it's in our node
    // #i27301# - consider adjusted method signature
    SwPaM* pCaret = GetCursor( false );  // caret is first PaM in PaM-ring

    if( pCaret != NULL )
    {
        const SwTxtNode* pNode = GetTxtNode();

        // check whether the point points into 'our' node
        SwPosition* pPoint = pCaret->GetPoint();
        if( pNode->GetIndex() == pPoint->nNode.GetIndex() )
        {
            // same node? Then check whether it's also within 'our' part
            // of the paragraph
            sal_uInt16 nIndex = pPoint->nContent.GetIndex();
            if( GetPortionData().IsValidCorePosition( nIndex ) )
            {
                // Yes, it's us!
                // consider that cursor/caret is in front of the list label
                if ( pCaret->IsInFrontOfLabel() )
                {
                    nRet = 0;
                }
                else
                {
                    nRet = GetPortionData().GetAccessiblePosition( nIndex );
                }

                OSL_ENSURE( nRet >= 0, "invalid cursor?" );
                OSL_ENSURE( nRet <= GetPortionData().GetAccessibleString().
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
    SwPaM* pCrsr = GetCursor( true ); // #i27301# - consider adjusted method signature
    if( pCrsr != NULL )
    {
        // get SwPosition for my node
        const SwTxtNode* pNode = GetTxtNode();
        sal_uLong nHere = pNode->GetIndex();

        // iterate over ring
        SwPaM* pRingStart = pCrsr;
        do
        {
            // ignore, if no mark
            if( pCrsr->HasMark() )
            {
                // check whether nHere is 'inside' pCrsr
                SwPosition* pStart = pCrsr->Start();
                sal_uLong nStartIndex = pStart->nNode.GetIndex();
                SwPosition* pEnd = pCrsr->End();
                sal_uLong nEndIndex = pEnd->nNode.GetIndex();
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
                        OSL_ENSURE( nHere == nStartIndex,
                                    "miscalculated index" );

                        // selection starts in this node:
                        // then check whether it's before or inside our part of
                        // the paragraph, and if so, get the proper position
                        sal_uInt16 nCoreStart = pStart->nContent.GetIndex();
                        if( nCoreStart <
                            GetPortionData().GetFirstValidCorePosition() )
                        {
                            nLocalStart = 0;
                        }
                        else if( nCoreStart <=
                                 GetPortionData().GetLastValidCorePosition() )
                        {
                            OSL_ENSURE(
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
                        OSL_ENSURE( nHere == nEndIndex,
                                    "miscalculated index" );

                        // selection ends in this node: then select everything
                        // before our part of the node
                        sal_uInt16 nCoreEnd = pEnd->nContent.GetIndex();
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
                            OSL_ENSURE(
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

// #i27301# - new parameter <_bForSelection>
SwPaM* SwAccessibleParagraph::GetCursor( const bool _bForSelection )
{
    // get the cursor shell; if we don't have any, we don't have a
    // cursor/selection either
    SwPaM* pCrsr = NULL;
    SwCrsrShell* pCrsrShell = SwAccessibleParagraph::GetCrsrShell();
    // #i27301# - if cursor is retrieved for selection, the cursors for
    // a table selection has to be returned.
    if ( pCrsrShell != NULL &&
         ( _bForSelection || !pCrsrShell->IsTableMode() ) )
    {
        SwFEShell *pFESh = pCrsrShell->ISA( SwFEShell )
                            ? static_cast< SwFEShell * >( pCrsrShell ) : 0;
        if( !pFESh ||
            !(pFESh->IsFrmSelected() || pFESh->IsObjSelected() > 0) )
        {
            // get the selection, and test whether it affects our text node
            pCrsr = pCrsrShell->GetCrsr( sal_False /* ??? */ );
        }
    }

    return pCrsr;
}

sal_Bool SwAccessibleParagraph::IsHeading() const
{
    const SwTxtNode *pTxtNd = GetTxtNode();
    return pTxtNd->IsOutline();
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
    SwPaM* pCaret = GetCursor( false ); // #i27301# - consider adjusted method signature
    const SwTxtNode* pTxtNd = GetTxtNode();
    if( pCaret != 0 && pTxtNd != 0 &&
        pTxtNd->GetIndex() == pCaret->GetPoint()->nNode.GetIndex() &&
        nOldCaretPos != -1)
    {
        Window *pWin = GetWindow();
        if( pWin && pWin->HasFocus() )
            rStateSet.AddState( AccessibleStateType::FOCUSED );
        ::rtl::Reference < SwAccessibleContext > xThis( this );
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
        osl::MutexGuard aGuard( aMutex );
        bOldIsHeading = bIsHeading;
        if( bIsHeading != bNewIsHeading )
            bIsHeading = bNewIsHeading;
    }

    if( bNewIsHeading != bOldIsHeading || rText != sOldText )
    {
        OUString sNewDesc( GetDescription() );
        OUString sOldDesc;
        {
            osl::MutexGuard aGuard( aMutex );
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
        osl::MutexGuard aGuard( aMutex );
        nOld = nOldCaretPos;
        nOldCaretPos = nNew;
    }
    if( -1 != nNew )
    {
        // remember that object as the one that has the caret. This is
        // necessary to notify that object if the cursor leaves it.
        ::rtl::Reference < SwAccessibleContext > xThis( this );
        GetMap()->SetCursorContext( xThis );
    }

    Window *pWin = GetWindow();
    if( nOld != nNew )
    {
        // The cursor's node position is simulated by the focus!
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
            osl::MutexGuard aGuard( aMutex );
            nPos = nOldCaretPos;
        }
        OSL_ENSURE( nPos != -1, "focus object should be selected" );

        FireStateChangedEvent( AccessibleStateType::FOCUSED,
                               pWin->HasFocus() && nPos != -1 );
    }
}

SwAccessibleParagraph::SwAccessibleParagraph(
        SwAccessibleMap& rInitMap,
        const SwTxtFrm& rTxtFrm )
    : SwClient( const_cast<SwTxtNode*>(rTxtFrm.GetTxtNode()) ) // #i108125#
    , SwAccessibleContext( &rInitMap, AccessibleRole::PARAGRAPH, &rTxtFrm )
    , sDesc()
    , pPortionData( NULL )
    , pHyperTextData( NULL )
    , nOldCaretPos( -1 )
    , bIsHeading( sal_False )
    , aSelectionHelper( *this )
    , mpParaChangeTrackInfo( new SwParaChangeTrackingInfo( rTxtFrm ) ) // #i108125#
{
    SolarMutexGuard aGuard;

    bIsHeading = IsHeading();
    SetName( OUString() ); // set an empty accessibility name for paragraphs

    // If this object has the focus, then it is remembered by the map itself.
    nOldCaretPos = GetCaretPos();
}

SwAccessibleParagraph::~SwAccessibleParagraph()
{
    SolarMutexGuard aGuard;

    delete pPortionData;
    delete pHyperTextData;
    delete mpParaChangeTrackInfo; // #i108125#
}

sal_Bool SwAccessibleParagraph::HasCursor()
{
    osl::MutexGuard aGuard( aMutex );
    return nOldCaretPos != -1;
}

void SwAccessibleParagraph::UpdatePortionData()
    throw( uno::RuntimeException )
{
    // obtain the text frame
    OSL_ENSURE( GetFrm() != NULL, "The text frame has vanished!" );
    OSL_ENSURE( GetFrm()->IsTxtFrm(), "The text frame has mutated!" );
    const SwTxtFrm* pFrm = static_cast<const SwTxtFrm*>( GetFrm() );

    // build new portion data
    delete pPortionData;
    pPortionData = new SwAccessiblePortionData(
        pFrm->GetTxtNode(), GetMap()->GetShell()->GetViewOptions() );
    pFrm->VisitPortions( *pPortionData );

    OSL_ENSURE( pPortionData != NULL, "UpdatePortionData() failed" );
}

void SwAccessibleParagraph::ClearPortionData()
{
    delete pPortionData;
    pPortionData = NULL;

    delete pHyperTextData;
    pHyperTextData = 0;
}

void SwAccessibleParagraph::ExecuteAtViewShell( sal_uInt16 nSlot )
{
    OSL_ENSURE( GetMap() != NULL, "no map?" );
    ViewShell* pViewShell = GetMap()->GetShell();

    OSL_ENSURE( pViewShell != NULL, "View shell expected!" );
    SfxViewShell* pSfxShell = pViewShell->GetSfxViewShell();

    OSL_ENSURE( pSfxShell != NULL, "SfxViewShell shell expected!" );
    if( !pSfxShell )
        return;

    SfxViewFrame *pFrame = pSfxShell->GetViewFrame();
    OSL_ENSURE( pFrame != NULL, "View frame expected!" );
    if( !pFrame )
        return;

    SfxDispatcher *pDispatcher = pFrame->GetDispatcher();
    OSL_ENSURE( pDispatcher != NULL, "Dispatcher expected!" );
    if( !pDispatcher )
        return;

    pDispatcher->Execute( nSlot );
}

SwXTextPortion* SwAccessibleParagraph::CreateUnoPortion(
    sal_Int32 nStartIndex,
    sal_Int32 nEndIndex )
{
    OSL_ENSURE( (IsValidChar(nStartIndex, GetString().getLength()) &&
                 (nEndIndex == -1)) ||
                IsValidRange(nStartIndex, nEndIndex, GetString().getLength()),
                "please check parameters before calling this method" );

    sal_uInt16 nStart = GetPortionData().GetModelPosition( nStartIndex );
    sal_uInt16 nEnd = (nEndIndex == -1) ? (nStart + 1) :
                        GetPortionData().GetModelPosition( nEndIndex );

    // create UNO cursor
    SwTxtNode* pTxtNode = const_cast<SwTxtNode*>( GetTxtNode() );
    SwIndex aIndex( pTxtNode, nStart );
    SwPosition aStartPos( *pTxtNode, aIndex );
    SwUnoCrsr* pUnoCursor = pTxtNode->GetDoc()->CreateUnoCrsr( aStartPos );
    pUnoCursor->SetMark();
    pUnoCursor->GetMark()->nContent = nEnd;

    // create a (dummy) text portion to be returned
    uno::Reference<text::XText> aEmpty;
    SwXTextPortion* pPortion =
        new SwXTextPortion ( pUnoCursor, aEmpty, PORTION_TEXT);
    delete pUnoCursor;

    return pPortion;
}

// range checking for parameter

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

// text boundaries

sal_Bool SwAccessibleParagraph::GetCharBoundary(
    i18n::Boundary& rBound,
    const OUString&,
    sal_Int32 nPos )
{
    rBound.startPos = nPos;
    rBound.endPos = nPos+1;
    return sal_True;
}

sal_Bool SwAccessibleParagraph::GetWordBoundary(
    i18n::Boundary& rBound,
    const OUString& rText,
    sal_Int32 nPos )
{
    sal_Bool bRet = sal_False;

    // now ask the Break-Iterator for the word
    OSL_ENSURE( g_pBreakIt != NULL, "We always need a break." );
    OSL_ENSURE( g_pBreakIt->GetBreakIter().is(), "No break-iterator." );
    if( g_pBreakIt->GetBreakIter().is() )
    {
        // get locale for this position
        sal_uInt16 nModelPos = GetPortionData().GetModelPosition( nPos );
        lang::Locale aLocale = g_pBreakIt->GetLocale(
                              GetTxtNode()->GetLang( nModelPos ) );

        // which type of word are we interested in?
        // (DICTIONARY_WORD includes punctuation, ANY_WORD doesn't.)
        const sal_uInt16 nWordType = i18n::WordType::ANY_WORD;

        // get word boundary, as the Break-Iterator sees fit.
        rBound = g_pBreakIt->GetBreakIter()->getWordBoundary(
            rText, nPos, aLocale, nWordType, sal_True );

        // It's a word if the first character is an alpha-numeric character.
        bRet = GetAppCharClass().isLetterNumeric(
            OUString(rText[rBound.startPos]) );
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
    i18n::Boundary& rBound,
    const OUString&,
    sal_Int32 nPos )
{
    GetPortionData().GetSentenceBoundary( rBound, nPos );
    return sal_True;
}

sal_Bool SwAccessibleParagraph::GetLineBoundary(
    i18n::Boundary& rBound,
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
    i18n::Boundary& rBound,
    const OUString& rText,
    sal_Int32 )
{
    rBound.startPos = 0;
    rBound.endPos = rText.getLength();
    return sal_True;
}

sal_Bool SwAccessibleParagraph::GetAttributeBoundary(
    i18n::Boundary& rBound,
    const OUString&,
    sal_Int32 nPos )
{
    GetPortionData().GetAttributeBoundary( rBound, nPos );
    return sal_True;
}

sal_Bool SwAccessibleParagraph::GetGlyphBoundary(
    i18n::Boundary& rBound,
    const OUString& rText,
    sal_Int32 nPos )
{
    sal_Bool bRet = sal_False;

    // ask the Break-Iterator for the glyph by moving one cell
    // forward, and then one cell back
    OSL_ENSURE( g_pBreakIt != NULL, "We always need a break." );
    OSL_ENSURE( g_pBreakIt->GetBreakIter().is(), "No break-iterator." );
    if( g_pBreakIt->GetBreakIter().is() )
    {
        // get locale for this position
        sal_uInt16 nModelPos = GetPortionData().GetModelPosition( nPos );
        lang::Locale aLocale = g_pBreakIt->GetLocale(
                              GetTxtNode()->GetLang( nModelPos ) );

        // get word boundary, as the Break-Iterator sees fit.
        const sal_uInt16 nIterMode = i18n::CharacterIteratorMode::SKIPCELL;
        sal_Int32 nDone = 0;
        rBound.endPos = g_pBreakIt->GetBreakIter()->nextCharacters(
             rText, nPos, aLocale, nIterMode, 1, nDone );
        rBound.startPos = g_pBreakIt->GetBreakIter()->previousCharacters(
             rText, rBound.endPos, aLocale, nIterMode, 1, nDone );
        bRet = ((rBound.startPos <= nPos) && (nPos <= rBound.endPos));
        OSL_ENSURE( rBound.startPos <= nPos, "start pos too high" );
        OSL_ENSURE( rBound.endPos >= nPos, "end pos too low" );
    }
    else
    {
        // no break Iterator -> no glyph
        rBound.startPos = nPos;
        rBound.endPos = nPos;
    }

    return bRet;
}


sal_Bool SwAccessibleParagraph::GetTextBoundary(
    i18n::Boundary& rBound,
    const OUString& rText,
    sal_Int32 nPos,
    sal_Int16 nTextType )
    throw (
        lang::IndexOutOfBoundsException,
        lang::IllegalArgumentException,
        uno::RuntimeException)
{
    // error checking
    if( !( AccessibleTextType::LINE == nTextType
                ? IsValidPosition( nPos, rText.getLength() )
                : IsValidChar( nPos, rText.getLength() ) ) )
        throw lang::IndexOutOfBoundsException();

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
            throw lang::IllegalArgumentException( );
    }

    return bRet;
}

OUString SAL_CALL SwAccessibleParagraph::getAccessibleDescription (void)
        throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleContext );

    osl::MutexGuard aGuard2( aMutex );
    if( sDesc.isEmpty() )
        sDesc = GetDescription();

    return sDesc;
}

lang::Locale SAL_CALL SwAccessibleParagraph::getLocale (void)
        throw (IllegalAccessibleComponentStateException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    SwTxtFrm *pTxtFrm = PTR_CAST( SwTxtFrm, GetFrm() );
    if( !pTxtFrm )
    {
        THROW_RUNTIME_EXCEPTION( XAccessibleContext, "internal error (no text frame)" );
    }

    const SwTxtNode *pTxtNd = pTxtFrm->GetTxtNode();
    lang::Locale aLoc( g_pBreakIt->GetLocale( pTxtNd->GetLang( 0 ) ) );

    return aLoc;
}

// #i27138# - paragraphs are in relation CONTENT_FLOWS_FROM and/or CONTENT_FLOWS_TO
uno::Reference<XAccessibleRelationSet> SAL_CALL SwAccessibleParagraph::getAccessibleRelationSet()
    throw ( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    CHECK_FOR_DEFUNC( XAccessibleContext );

    utl::AccessibleRelationSetHelper* pHelper = new utl::AccessibleRelationSetHelper();

    const SwTxtFrm* pTxtFrm = dynamic_cast<const SwTxtFrm*>(GetFrm());
    OSL_ENSURE( pTxtFrm,
            "<SwAccessibleParagraph::getAccessibleRelationSet()> - missing text frame");
    if ( pTxtFrm )
    {
        const SwCntntFrm* pPrevCntFrm( pTxtFrm->FindPrevCnt( true ) );
        if ( pPrevCntFrm )
        {
            uno::Sequence< uno::Reference<XInterface> > aSequence(1);
            aSequence[0] = GetMap()->GetContext( pPrevCntFrm );
            AccessibleRelation aAccRel( AccessibleRelationType::CONTENT_FLOWS_FROM,
                                        aSequence );
            pHelper->AddRelation( aAccRel );
        }

        const SwCntntFrm* pNextCntFrm( pTxtFrm->FindNextCnt( true ) );
        if ( pNextCntFrm )
        {
            uno::Sequence< uno::Reference<XInterface> > aSequence(1);
            aSequence[0] = GetMap()->GetContext( pNextCntFrm );
            AccessibleRelation aAccRel( AccessibleRelationType::CONTENT_FLOWS_TO,
                                        aSequence );
            pHelper->AddRelation( aAccRel );
        }
    }

    return pHelper;
}

void SAL_CALL SwAccessibleParagraph::grabFocus()
        throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleContext );

    // get cursor shell
    SwCrsrShell *pCrsrSh = GetCrsrShell();
    SwPaM *pCrsr = GetCursor( false ); // #i27301# - consider new method signature
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

    // ->#i13955#
    Window * pWindow = GetWindow();

    if (pWindow != NULL)
        pWindow->GrabFocus();
    // <-#i13955#
}

// #i71385#
static bool lcl_GetBackgroundColor( Color & rColor,
                             const SwFrm* pFrm,
                             SwCrsrShell* pCrsrSh )
{
    const SvxBrushItem* pBackgrdBrush = 0;
    const XFillStyleItem* pFillStyleItem = 0;
    const XFillGradientItem* pFillGradientItem = 0;
    const Color* pSectionTOXColor = 0;
    SwRect aDummyRect;
    if ( pFrm &&
         pFrm->GetBackgroundBrush( pBackgrdBrush, pFillStyleItem, pFillGradientItem, pSectionTOXColor, aDummyRect, false ) )
    {
        if ( pSectionTOXColor )
        {
            rColor = *pSectionTOXColor;
            return true;
        }
        else
        {
            rColor =  pBackgrdBrush->GetColor();
            return true;
        }
    }
    else if ( pCrsrSh )
    {
        rColor = pCrsrSh->Imp()->GetRetoucheColor();
        return true;
    }

    return false;
}

sal_Int32 SAL_CALL SwAccessibleParagraph::getForeground()
                                throw (uno::RuntimeException)
{
    Color aBackgroundCol;

    if ( lcl_GetBackgroundColor( aBackgroundCol, GetFrm(), GetCrsrShell() ) )
    {
        if ( aBackgroundCol.IsDark() )
        {
            return COL_WHITE;
        }
        else
        {
            return COL_BLACK;
        }
    }

    return SwAccessibleContext::getForeground();
}

sal_Int32 SAL_CALL SwAccessibleParagraph::getBackground()
                                throw (uno::RuntimeException)
{
    Color aBackgroundCol;

    if ( lcl_GetBackgroundColor( aBackgroundCol, GetFrm(), GetCrsrShell() ) )
    {
        return aBackgroundCol.GetColor();
    }

    return SwAccessibleContext::getBackground();
}

OUString SAL_CALL SwAccessibleParagraph::getImplementationName()
        throw( uno::RuntimeException )
{
    return OUString(sImplementationName);
}

sal_Bool SAL_CALL SwAccessibleParagraph::supportsService(
        const OUString& sTestServiceName)
    throw (uno::RuntimeException)
{
    return sTestServiceName.equalsAsciiL( sServiceName,
                                          sizeof(sServiceName)-1 ) ||
           sTestServiceName.equalsAsciiL( sAccessibleServiceName,
                                             sizeof(sAccessibleServiceName)-1 );
}

uno::Sequence< OUString > SAL_CALL SwAccessibleParagraph::getSupportedServiceNames()
        throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = OUString( sServiceName );
    pArray[1] = OUString( sAccessibleServiceName );
    return aRet;
}

// XInterface

uno::Any SwAccessibleParagraph::queryInterface( const uno::Type& rType )
    throw (uno::RuntimeException)
{
    uno::Any aRet;
    if ( rType == ::getCppuType((uno::Reference<XAccessibleText> *)0) )
    {
        uno::Reference<XAccessibleText> aAccText = (XAccessibleText *) *this; // resolve ambiguity
        aRet <<= aAccText;
    }
    else if ( rType == ::getCppuType((uno::Reference<XAccessibleEditableText> *)0) )
    {
        uno::Reference<XAccessibleEditableText> aAccEditText = this;
        aRet <<= aAccEditText;
    }
    else if ( rType == ::getCppuType((uno::Reference<XAccessibleSelection> *)0) )
    {
        uno::Reference<XAccessibleSelection> aAccSel = this;
        aRet <<= aAccSel;
    }
    else if ( rType == ::getCppuType((uno::Reference<XAccessibleHypertext> *)0) )
    {
        uno::Reference<XAccessibleHypertext> aAccHyp = this;
        aRet <<= aAccHyp;
    }
    // #i63870#
    // add interface com::sun:star:accessibility::XAccessibleTextAttributes
    else if ( rType == ::getCppuType((uno::Reference<XAccessibleTextAttributes> *)0) )
    {
        uno::Reference<XAccessibleTextAttributes> aAccTextAttr = this;
        aRet <<= aAccTextAttr;
    }
    // #i89175#
    // add interface com::sun:star:accessibility::XAccessibleTextMarkup
    else if ( rType == ::getCppuType((uno::Reference<XAccessibleTextMarkup> *)0) )
    {
        uno::Reference<XAccessibleTextMarkup> aAccTextMarkup = this;
        aRet <<= aAccTextMarkup;
    }
    // add interface com::sun:star:accessibility::XAccessibleMultiLineText
    else if ( rType == ::getCppuType((uno::Reference<XAccessibleMultiLineText> *)0) )
    {
        uno::Reference<XAccessibleMultiLineText> aAccMultiLineText = this;
        aRet <<= aAccMultiLineText;
    }
    else
    {
        aRet = SwAccessibleContext::queryInterface(rType);
    }

    return aRet;
}

// XTypeProvider
uno::Sequence< uno::Type > SAL_CALL SwAccessibleParagraph::getTypes() throw(uno::RuntimeException)
{
    uno::Sequence< uno::Type > aTypes( SwAccessibleContext::getTypes() );

    sal_Int32 nIndex = aTypes.getLength();
    // #i63870# - add type accessibility::XAccessibleTextAttributes
    // #i89175# - add type accessibility::XAccessibleTextMarkup and
    // accessibility::XAccessibleMultiLineText
    aTypes.realloc( nIndex + 6 );

    uno::Type* pTypes = aTypes.getArray();
    pTypes[nIndex++] = ::getCppuType( static_cast< uno::Reference< XAccessibleEditableText > * >( 0 ) );
    pTypes[nIndex++] = ::getCppuType( static_cast< uno::Reference< XAccessibleTextAttributes > * >( 0 ) );
    pTypes[nIndex++] = ::getCppuType( static_cast< uno::Reference< XAccessibleSelection > * >( 0 ) );
    pTypes[nIndex++] = ::getCppuType( static_cast< uno::Reference< XAccessibleTextMarkup > * >( 0 ) );
    pTypes[nIndex++] = ::getCppuType( static_cast< uno::Reference< XAccessibleMultiLineText > * >( 0 ) );
    pTypes[nIndex] = ::getCppuType( static_cast< uno::Reference< XAccessibleHypertext > * >( 0 ) );

    return aTypes;
}

namespace
{
    class theSwAccessibleParagraphImplementationId : public rtl::Static< UnoTunnelIdInit, theSwAccessibleParagraphImplementationId > {};
}

uno::Sequence< sal_Int8 > SAL_CALL SwAccessibleParagraph::getImplementationId()
        throw(uno::RuntimeException)
{
    return theSwAccessibleParagraphImplementationId::get().getSeq();
}

// XAccesibleText

sal_Int32 SwAccessibleParagraph::getCaretPosition()
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    sal_Int32 nRet = GetCaretPos();
    {
        osl::MutexGuard aOldCaretPosGuard( aMutex );
        OSL_ENSURE( nRet == nOldCaretPos, "caret pos out of sync" );
        nOldCaretPos = nRet;
    }
    if( -1 != nRet )
    {
        ::rtl::Reference < SwAccessibleContext > xThis( this );
        GetMap()->SetCursorContext( xThis );
    }

    return nRet;
}

sal_Bool SAL_CALL SwAccessibleParagraph::setCaretPosition( sal_Int32 nIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    // parameter checking
    sal_Int32 nLength = GetString().getLength();
    if ( ! IsValidPosition( nIndex, nLength ) )
    {
        throw lang::IndexOutOfBoundsException();
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
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    OUString sText( GetString() );

    // return character (if valid)
    if( IsValidChar(nIndex, sText.getLength() ) )
    {
        return sText.getStr()[nIndex];
    }
    else
        throw lang::IndexOutOfBoundsException();
}

// #i63870# - re-implement method on behalf of methods
// <_getDefaultAttributesImpl(..)> and <_getRunAttributesImpl(..)>
uno::Sequence<PropertyValue> SwAccessibleParagraph::getCharacterAttributes(
    sal_Int32 nIndex,
    const uno::Sequence< OUString >& aRequestedAttributes )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{

    SolarMutexGuard aGuard;
    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    const OUString& rText = GetString();

    if( ! IsValidChar( nIndex, rText.getLength() ) )
        throw lang::IndexOutOfBoundsException();

    // retrieve default character attributes
    tAccParaPropValMap aDefAttrSeq;
    _getDefaultAttributesImpl( aRequestedAttributes, aDefAttrSeq, true );

    // retrieved run character attributes
    tAccParaPropValMap aRunAttrSeq;
    _getRunAttributesImpl( nIndex, aRequestedAttributes, aRunAttrSeq );

    // merge default and run attributes
    uno::Sequence< PropertyValue > aValues( aDefAttrSeq.size() );
    PropertyValue* pValues = aValues.getArray();
    sal_Int32 i = 0;
    for ( tAccParaPropValMap::const_iterator aDefIter = aDefAttrSeq.begin();
          aDefIter != aDefAttrSeq.end();
          ++aDefIter )
    {
        tAccParaPropValMap::const_iterator aRunIter =
                                        aRunAttrSeq.find( aDefIter->first );
        if ( aRunIter != aRunAttrSeq.end() )
        {
            pValues[i] = aRunIter->second;
        }
        else
        {
            pValues[i] = aDefIter->second;
        }
        ++i;
    }

    return aValues;
}

// #i63870#
void SwAccessibleParagraph::_getDefaultAttributesImpl(
        const uno::Sequence< OUString >& aRequestedAttributes,
        tAccParaPropValMap& rDefAttrSeq,
        const bool bOnlyCharAttrs )
{
    // retrieve default attributes
    const SwTxtNode* pTxtNode( GetTxtNode() );
    ::boost::scoped_ptr<SfxItemSet> pSet;
    if ( !bOnlyCharAttrs )
    {
        pSet.reset( new SfxItemSet( const_cast<SwAttrPool&>(pTxtNode->GetDoc()->GetAttrPool()),
                               RES_CHRATR_BEGIN, RES_CHRATR_END - 1,
                               RES_PARATR_BEGIN, RES_PARATR_END - 1,
                               RES_FRMATR_BEGIN, RES_FRMATR_END - 1,
                               0 ) );
    }
    else
    {
        pSet.reset( new SfxItemSet( const_cast<SwAttrPool&>(pTxtNode->GetDoc()->GetAttrPool()),
                               RES_CHRATR_BEGIN, RES_CHRATR_END - 1,
                               0 ) );
    }
    // #i82637# - From the perspective of the a11y API the default character
    // attributes are the character attributes, which are set at the paragraph style
    // of the paragraph. The character attributes set at the automatic paragraph
    // style of the paragraph are treated as run attributes.
    //    pTxtNode->SwCntntNode::GetAttr( *pSet );
    // get default paragraph attributes, if needed, and merge these into <pSet>
    if ( !bOnlyCharAttrs )
    {
        SfxItemSet aParaSet( const_cast<SwAttrPool&>(pTxtNode->GetDoc()->GetAttrPool()),
                             RES_PARATR_BEGIN, RES_PARATR_END - 1,
                             RES_FRMATR_BEGIN, RES_FRMATR_END - 1,
                             0 );
        pTxtNode->SwCntntNode::GetAttr( aParaSet );
        pSet->Put( aParaSet );
    }
    // get default character attributes and merge these into <pSet>
    OSL_ENSURE( pTxtNode->GetTxtColl(),
            "<SwAccessibleParagraph::_getDefaultAttributesImpl(..)> - missing paragraph style. Serious defect, please inform OD!" );
    if ( pTxtNode->GetTxtColl() )
    {
        SfxItemSet aCharSet( const_cast<SwAttrPool&>(pTxtNode->GetDoc()->GetAttrPool()),
                             RES_CHRATR_BEGIN, RES_CHRATR_END - 1,
                             0 );
        aCharSet.Put( pTxtNode->GetTxtColl()->GetAttrSet() );
        pSet->Put( aCharSet );
    }

    // build-up sequence containing the run attributes <rDefAttrSeq>
    tAccParaPropValMap aDefAttrSeq;
    {
        const SfxItemPropertyMap& rPropMap =
                    aSwMapProvider.GetPropertySet( PROPERTY_MAP_TEXT_CURSOR )->getPropertyMap();
        PropertyEntryVector_t aPropertyEntries = rPropMap.getPropertyEntries();
        PropertyEntryVector_t::const_iterator aPropIt = aPropertyEntries.begin();
        while ( aPropIt != aPropertyEntries.end() )
        {
            const SfxPoolItem* pItem = pSet->GetItem( aPropIt->nWID );
            if ( pItem )
            {
                uno::Any aVal;
                pItem->QueryValue( aVal, aPropIt->nMemberId );

                PropertyValue rPropVal;
                rPropVal.Name = aPropIt->sName;
                rPropVal.Value = aVal;
                rPropVal.Handle = -1;
                rPropVal.State = beans::PropertyState_DEFAULT_VALUE;

                aDefAttrSeq[rPropVal.Name] = rPropVal;
            }
            ++aPropIt;
        }

        // #i72800#
        // add property value entry for the paragraph style
        if ( !bOnlyCharAttrs && pTxtNode->GetTxtColl() )
        {
            const OUString sParaStyleName =
                    OUString::createFromAscii(
                            GetPropName( UNO_NAME_PARA_STYLE_NAME ).pName );
            if ( aDefAttrSeq.find( sParaStyleName ) == aDefAttrSeq.end() )
            {
                PropertyValue rPropVal;
                rPropVal.Name = sParaStyleName;
                uno::Any aVal( uno::makeAny( pTxtNode->GetTxtColl()->GetName() ) );
                rPropVal.Value = aVal;
                rPropVal.Handle = -1;
                rPropVal.State = beans::PropertyState_DEFAULT_VALUE;

                aDefAttrSeq[rPropVal.Name] = rPropVal;
            }
        }

        // #i73371#
        // resolve value text::WritingMode2::PAGE of property value entry WritingMode
        if ( !bOnlyCharAttrs && GetFrm() )
        {
            const OUString sWritingMode =
                    OUString::createFromAscii(
                            GetPropName( UNO_NAME_WRITING_MODE ).pName );
            tAccParaPropValMap::iterator aIter = aDefAttrSeq.find( sWritingMode );
            if ( aIter != aDefAttrSeq.end() )
            {
                PropertyValue rPropVal( aIter->second );
                sal_Int16 nVal = rPropVal.Value.get<sal_Int16>();
                if ( nVal == text::WritingMode2::PAGE )
                {
                    const SwFrm* pUpperFrm( GetFrm()->GetUpper() );
                    while ( pUpperFrm )
                    {
                        if ( pUpperFrm->GetType() &
                               ( FRM_PAGE | FRM_FLY | FRM_SECTION | FRM_TAB | FRM_CELL ) )
                        {
                            if ( pUpperFrm->IsVertical() )
                            {
                                nVal = text::WritingMode2::TB_RL;
                            }
                            else if ( pUpperFrm->IsRightToLeft() )
                            {
                                nVal = text::WritingMode2::RL_TB;
                            }
                            else
                            {
                                nVal = text::WritingMode2::LR_TB;
                            }
                            rPropVal.Value <<= nVal;
                            aDefAttrSeq[rPropVal.Name] = rPropVal;
                            break;
                        }

                        if ( dynamic_cast<const SwFlyFrm*>(pUpperFrm) )
                        {
                            pUpperFrm = dynamic_cast<const SwFlyFrm*>(pUpperFrm)->GetAnchorFrm();
                        }
                        else
                        {
                            pUpperFrm = pUpperFrm->GetUpper();
                        }
                    }
                }
            }
        }
    }

    if ( aRequestedAttributes.getLength() == 0 )
    {
        rDefAttrSeq = aDefAttrSeq;
    }
    else
    {
        const OUString* pReqAttrs = aRequestedAttributes.getConstArray();
        const sal_Int32 nLength = aRequestedAttributes.getLength();
        for( sal_Int32 i = 0; i < nLength; ++i )
        {
            tAccParaPropValMap::const_iterator const aIter = aDefAttrSeq.find( pReqAttrs[i] );
            if ( aIter != aDefAttrSeq.end() )
            {
                rDefAttrSeq[ aIter->first ] = aIter->second;
            }
        }
    }
}

uno::Sequence< PropertyValue > SwAccessibleParagraph::getDefaultAttributes(
        const uno::Sequence< OUString >& aRequestedAttributes )
        throw ( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    tAccParaPropValMap aDefAttrSeq;
    _getDefaultAttributesImpl( aRequestedAttributes, aDefAttrSeq );

    // #i92233#
    static OUString sMMToPixelRatio("MMToPixelRatio");
    bool bProvideMMToPixelRatio( false );
    {
        if ( aRequestedAttributes.getLength() == 0 )
        {
            bProvideMMToPixelRatio = true;
        }
        else
        {
            const OUString* aRequestedAttrIter =
                  ::std::find( ::comphelper::stl_begin( aRequestedAttributes ),
                               ::comphelper::stl_end( aRequestedAttributes ),
                               sMMToPixelRatio );
            if ( aRequestedAttrIter != ::comphelper::stl_end( aRequestedAttributes ) )
            {
                bProvideMMToPixelRatio = true;
            }
        }
    }

    uno::Sequence< PropertyValue > aValues( aDefAttrSeq.size() +
                                            ( bProvideMMToPixelRatio ? 1 : 0 ) );
    PropertyValue* pValues = aValues.getArray();
    sal_Int32 i = 0;
    for ( tAccParaPropValMap::const_iterator aIter  = aDefAttrSeq.begin();
          aIter != aDefAttrSeq.end();
          ++aIter )
    {
        pValues[i] = aIter->second;
        ++i;
    }

    // #i92233#
    if ( bProvideMMToPixelRatio )
    {
        PropertyValue rPropVal;
        rPropVal.Name = sMMToPixelRatio;
        const Size a100thMMSize( 1000, 1000 );
        const Size aPixelSize = GetMap()->LogicToPixel( a100thMMSize );
        const float fRatio = ((float)a100thMMSize.Width()/100)/aPixelSize.Width();
        rPropVal.Value = uno::makeAny( fRatio );
        rPropVal.Handle = -1;
        rPropVal.State = beans::PropertyState_DEFAULT_VALUE;
        pValues[ aValues.getLength() - 1 ] = rPropVal;
    }

    return aValues;
}

void SwAccessibleParagraph::_getRunAttributesImpl(
        const sal_Int32 nIndex,
        const uno::Sequence< OUString >& aRequestedAttributes,
        tAccParaPropValMap& rRunAttrSeq )
{
    // create PaM for character at position <nIndex>
    SwPaM* pPaM( 0 );
    {
        const SwTxtNode* pTxtNode( GetTxtNode() );
        SwPosition* pStartPos = new SwPosition( *pTxtNode );
        pStartPos->nContent.Assign( const_cast<SwTxtNode*>(pTxtNode), static_cast<sal_uInt16>(nIndex) );
        SwPosition* pEndPos = new SwPosition( *pTxtNode );
        pEndPos->nContent.Assign( const_cast<SwTxtNode*>(pTxtNode), static_cast<sal_uInt16>(nIndex+1) );

        pPaM = new SwPaM( *pStartPos, *pEndPos );

        delete pStartPos;
        delete pEndPos;
    }

    // retrieve character attributes for the created PaM <pPaM>
    SfxItemSet aSet( pPaM->GetDoc()->GetAttrPool(),
                     RES_CHRATR_BEGIN, RES_CHRATR_END -1,
                     0 );
    // #i82637#
    // From the perspective of the a11y API the character attributes, which
    // are set at the automatic paragraph style of the paragraph, are treated
    // as run attributes.
    //    SwXTextCursor::GetCrsrAttr( *pPaM, aSet, sal_True, sal_True );
    // get character attributes from automatic paragraph style and merge these into <aSet>
    {
        const SwTxtNode* pTxtNode( GetTxtNode() );
        if ( pTxtNode->HasSwAttrSet() )
        {
            SfxItemSet aAutomaticParaStyleCharAttrs( pPaM->GetDoc()->GetAttrPool(),
                                                     RES_CHRATR_BEGIN, RES_CHRATR_END -1,
                                                     0 );
            aAutomaticParaStyleCharAttrs.Put( *(pTxtNode->GetpSwAttrSet()), sal_False );
            aSet.Put( aAutomaticParaStyleCharAttrs );
        }
    }
    // get character attributes at <pPaM> and merge these into <aSet>
    {
        SfxItemSet aCharAttrsAtPaM( pPaM->GetDoc()->GetAttrPool(),
                                    RES_CHRATR_BEGIN, RES_CHRATR_END -1,
                                    0 );
        SwUnoCursorHelper::GetCrsrAttr(*pPaM, aCharAttrsAtPaM, sal_True, sal_True);
        aSet.Put( aCharAttrsAtPaM );
    }

    // build-up sequence containing the run attributes <rRunAttrSeq>
    {
        tAccParaPropValMap aRunAttrSeq;
        {
            tAccParaPropValMap aDefAttrSeq;
            uno::Sequence< OUString > aDummy;
            _getDefaultAttributesImpl( aDummy, aDefAttrSeq, true ); // #i82637#

            const SfxItemPropertyMap& rPropMap =
                    aSwMapProvider.GetPropertySet( PROPERTY_MAP_TEXT_CURSOR )->getPropertyMap();
            PropertyEntryVector_t aPropertyEntries = rPropMap.getPropertyEntries();
            PropertyEntryVector_t::const_iterator aPropIt = aPropertyEntries.begin();
            while ( aPropIt != aPropertyEntries.end() )
            {
                const SfxPoolItem* pItem( 0 );
                // #i82637# - Found character attributes, whose value equals the value of
                // the corresponding default character attributes, are excluded.
                if ( aSet.GetItemState( aPropIt->nWID, sal_True, &pItem ) == SFX_ITEM_SET )
                {
                    uno::Any aVal;
                    pItem->QueryValue( aVal, aPropIt->nMemberId );

                    PropertyValue rPropVal;
                    rPropVal.Name = aPropIt->sName;
                    rPropVal.Value = aVal;
                    rPropVal.Handle = -1;
                    rPropVal.State = PropertyState_DIRECT_VALUE;

                    tAccParaPropValMap::const_iterator aDefIter =
                                            aDefAttrSeq.find( rPropVal.Name );
                    if ( aDefIter == aDefAttrSeq.end() ||
                         rPropVal.Value != aDefIter->second.Value )
                    {
                        aRunAttrSeq[rPropVal.Name] = rPropVal;
                    }
                }

                ++aPropIt;
            }
        }

        if ( aRequestedAttributes.getLength() == 0 )
        {
            rRunAttrSeq = aRunAttrSeq;
        }
        else
        {
            const OUString* pReqAttrs = aRequestedAttributes.getConstArray();
            const sal_Int32 nLength = aRequestedAttributes.getLength();
            for( sal_Int32 i = 0; i < nLength; ++i )
            {
                tAccParaPropValMap::iterator aIter = aRunAttrSeq.find( pReqAttrs[i] );
                if ( aIter != aRunAttrSeq.end() )
                {
                    rRunAttrSeq[ (*aIter).first ] = (*aIter).second;
                }
            }
        }
    }

    delete pPaM;
}

uno::Sequence< PropertyValue > SwAccessibleParagraph::getRunAttributes(
        sal_Int32 nIndex,
        const uno::Sequence< OUString >& aRequestedAttributes )
        throw ( lang::IndexOutOfBoundsException,
                uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    {
        const OUString& rText = GetString();
        if ( !IsValidChar( nIndex, rText.getLength() ) )
        {
            throw lang::IndexOutOfBoundsException();
        }
    }

    tAccParaPropValMap aRunAttrSeq;
    _getRunAttributesImpl( nIndex, aRequestedAttributes, aRunAttrSeq );

    uno::Sequence< PropertyValue > aValues( aRunAttrSeq.size() );
    PropertyValue* pValues = aValues.getArray();
    sal_Int32 i = 0;
    for ( tAccParaPropValMap::const_iterator aIter  = aRunAttrSeq.begin();
          aIter != aRunAttrSeq.end();
          ++aIter )
    {
        pValues[i] = aIter->second;
        ++i;
    }

    return aValues;
}

awt::Rectangle SwAccessibleParagraph::getCharacterBounds(
    sal_Int32 nIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );


    // #i12332# The position after the string needs special treatment.
    // IsValidChar -> IsValidPosition
    if( ! (IsValidPosition( nIndex, GetString().getLength() ) ) )
        throw lang::IndexOutOfBoundsException();

    // #i12332#
    sal_Bool bBehindText = sal_False;
    if ( nIndex == GetString().getLength() )
        bBehindText = sal_True;

    // get model position & prepare GetCharRect() arguments
    SwCrsrMoveState aMoveState;
    aMoveState.bRealHeight = sal_True;
    aMoveState.bRealWidth = sal_True;
    SwSpecialPos aSpecialPos;
    SwTxtNode* pNode = const_cast<SwTxtNode*>( GetTxtNode() );

    sal_uInt16 nPos = 0;

    /**  #i12332# FillSpecialPos does not accept nIndex ==
         GetString().getLength(). In that case nPos is set to the
         length of the string in the core. This way GetCharRect
         returns the rectangle for a cursor at the end of the
         paragraph. */
    if (bBehindText)
    {
        nPos = pNode->GetTxt().getLength();
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
    SwRect aFrmLogBounds( GetBounds( *(GetMap()) ) ); // twip rel to doc root

    Point aFrmPixPos( GetMap()->CoreToPixel( aFrmLogBounds.SVRect() ).TopLeft() );
    aScreenRect.Move( -aFrmPixPos.getX(), -aFrmPixPos.getY() );

    // convert into AWT Rectangle
    return awt::Rectangle(
        aScreenRect.Left(), aScreenRect.Top(),
        aScreenRect.GetWidth(), aScreenRect.GetHeight() );
}

sal_Int32 SwAccessibleParagraph::getCharacterCount()
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    return GetString().getLength();
}

sal_Int32 SwAccessibleParagraph::getIndexAtPoint( const awt::Point& rPoint )
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;


    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    // construct SwPosition (where GetCrsrOfst() will put the result into)
    SwTxtNode* pNode = const_cast<SwTxtNode*>( GetTxtNode() );
    SwIndex aIndex( pNode, 0);
    SwPosition aPos( *pNode, aIndex );

    // construct Point (translate into layout coordinates)
    Window *pWin = GetWindow();
    CHECK_FOR_WINDOW( XAccessibleComponent, pWin );
    Point aPoint( rPoint.X, rPoint.Y );
    SwRect aLogBounds( GetBounds( *(GetMap()), GetFrm() ) ); // twip rel to doc root
    Point aPixPos( GetMap()->CoreToPixel( aLogBounds.SVRect() ).TopLeft() );
    aPoint.setX(aPoint.getX() + aPixPos.getX());
    aPoint.setY(aPoint.getY() + aPixPos.getY());
    MapMode aMapMode = pWin->GetMapMode();
    Point aCorePoint( GetMap()->PixelToCore( aPoint ) );
    if( !aLogBounds.IsInside( aCorePoint ) )
    {
        // #i12332# rPoint is may also be in rectangle returned by
        // getCharacterBounds(getCharacterCount()

        awt::Rectangle aRectEndPos =
            getCharacterBounds(getCharacterCount());

        if (rPoint.X - aRectEndPos.X >= 0 &&
            rPoint.X - aRectEndPos.X < aRectEndPos.Width &&
            rPoint.Y - aRectEndPos.Y >= 0 &&
            rPoint.Y - aRectEndPos.Y < aRectEndPos.Height)
            return getCharacterCount();

        return -1;
    }

    // ask core for position
    OSL_ENSURE( GetFrm() != NULL, "The text frame has vanished!" );
    OSL_ENSURE( GetFrm()->IsTxtFrm(), "The text frame has mutated!" );
    const SwTxtFrm* pFrm = static_cast<const SwTxtFrm*>( GetFrm() );
    SwCrsrMoveState aMoveState;
    aMoveState.bPosMatchesBounds = sal_True;
    sal_Bool bSuccess = pFrm->GetCrsrOfst( &aPos, aCorePoint, &aMoveState );

    SwIndex aCntntIdx = aPos.nContent;
    const xub_StrLen nIndex = aCntntIdx.GetIndex();
    if ( nIndex > 0 )
    {
        SwRect aResultRect;
        pFrm->GetCharRect( aResultRect, aPos );
        bool bVert = pFrm->IsVertical();
        bool bR2L = pFrm->IsRightToLeft();

        if ( (!bVert && aResultRect.Pos().getX() > aCorePoint.getX()) ||
             ( bVert && aResultRect.Pos().getY() > aCorePoint.getY()) ||
             ( bR2L  && aResultRect.Right()   < aCorePoint.getX()) )
        {
            SwIndex aIdxPrev( pNode, nIndex - 1);
            SwPosition aPosPrev( *pNode, aIdxPrev );
            SwRect aResultRectPrev;
            pFrm->GetCharRect( aResultRectPrev, aPosPrev );
            if ( (!bVert && aResultRectPrev.Pos().getX() < aCorePoint.getX() && aResultRect.Pos().getY() == aResultRectPrev.Pos().getY()) ||
                 ( bVert && aResultRectPrev.Pos().getY() < aCorePoint.getY() && aResultRect.Pos().getX() == aResultRectPrev.Pos().getX()) ||
                 (  bR2L && aResultRectPrev.Right()   > aCorePoint.getX() && aResultRect.Pos().getY() == aResultRectPrev.Pos().getY()) )
                aPos = aPosPrev;
        }
    }

    return bSuccess ?
        GetPortionData().GetAccessiblePosition( aPos.nContent.GetIndex() )
        : -1L;
}

OUString SwAccessibleParagraph::getSelectedText()
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    sal_Int32 nStart, nEnd;
    sal_Bool bSelected = GetSelection( nStart, nEnd );
    return bSelected
           ? GetString().copy( nStart, nEnd - nStart )
           : OUString();
}

sal_Int32 SwAccessibleParagraph::getSelectionStart()
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    sal_Int32 nStart, nEnd;
    GetSelection( nStart, nEnd );
    return nStart;
}

sal_Int32 SwAccessibleParagraph::getSelectionEnd()
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    sal_Int32 nStart, nEnd;
    GetSelection( nStart, nEnd );
    return nEnd;
}

sal_Bool SwAccessibleParagraph::setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    // parameter checking
    sal_Int32 nLength = GetString().getLength();
    if ( ! IsValidRange( nStartIndex, nEndIndex, nLength ) )
    {
        throw lang::IndexOutOfBoundsException();
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
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    return GetString();
}

OUString SwAccessibleParagraph::getTextRange(
    sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    OUString sText( GetString() );

    if ( IsValidRange( nStartIndex, nEndIndex, sText.getLength() ) )
    {
        OrderRange( nStartIndex, nEndIndex );
        return sText.copy(nStartIndex, nEndIndex-nStartIndex );
    }
    else
        throw lang::IndexOutOfBoundsException();
}

/*accessibility::*/TextSegment SwAccessibleParagraph::getTextAtIndex( sal_Int32 nIndex, sal_Int16 nTextType ) throw (lang::IndexOutOfBoundsException, lang::IllegalArgumentException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    /*accessibility::*/TextSegment aResult;
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
    i18n::Boundary aBound;
    sal_Bool bWord = GetTextBoundary( aBound, rText, nIndex, nTextType );

    OSL_ENSURE( aBound.startPos >= 0,               "illegal boundary" );
    OSL_ENSURE( aBound.startPos <= aBound.endPos,   "illegal boundary" );

    // return word (if present)
    if ( bWord )
    {
        aResult.SegmentText = rText.copy( aBound.startPos, aBound.endPos - aBound.startPos );
        aResult.SegmentStart = aBound.startPos;
        aResult.SegmentEnd = aBound.endPos;
    }

    return aResult;
}

/*accessibility::*/TextSegment SwAccessibleParagraph::getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 nTextType ) throw (lang::IndexOutOfBoundsException, lang::IllegalArgumentException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    const OUString rText = GetString();

    /*accessibility::*/TextSegment aResult;
    aResult.SegmentStart = -1;
    aResult.SegmentEnd = -1;

    // get starting pos
    i18n::Boundary aBound;
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

/*accessibility::*/TextSegment SwAccessibleParagraph::getTextBehindIndex( sal_Int32 nIndex, sal_Int16 nTextType ) throw (lang::IndexOutOfBoundsException, lang::IllegalArgumentException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    /*accessibility::*/TextSegment aResult;
    aResult.SegmentStart = -1;
    aResult.SegmentEnd = -1;
    const OUString rText = GetString();

    // implement the silly specification that first position after
    // text must return an empty string, rather than throwing an
    // IndexOutOfBoundsException
    if( nIndex == rText.getLength() )
        return aResult;


    // get first word, then skip to next word
    i18n::Boundary aBound;
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
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );
    SolarMutexGuard aGuard;

    // select and copy (through dispatch mechanism)
    setSelection( nStartIndex, nEndIndex );
    ExecuteAtViewShell( SID_COPY );
    return sal_True;
}

// XAccesibleEditableText

sal_Bool SwAccessibleParagraph::cutText( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    CHECK_FOR_DEFUNC( XAccessibleEditableText );
    SolarMutexGuard aGuard;

    if( !IsEditableState() )
        return sal_False;

    // select and cut (through dispatch mechanism)
    setSelection( nStartIndex, nEndIndex );
    ExecuteAtViewShell( SID_CUT );
    return sal_True;
}

sal_Bool SwAccessibleParagraph::pasteText( sal_Int32 nIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    CHECK_FOR_DEFUNC( XAccessibleEditableText );
    SolarMutexGuard aGuard;

    if( !IsEditableState() )
        return sal_False;

    // select and paste (through dispatch mechanism)
    setSelection( nIndex, nIndex );
    ExecuteAtViewShell( SID_PASTE );
    return sal_True;
}

sal_Bool SwAccessibleParagraph::deleteText( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    return replaceText( nStartIndex, nEndIndex, OUString() );
}

sal_Bool SwAccessibleParagraph::insertText( const OUString& sText, sal_Int32 nIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    return replaceText( nIndex, nIndex, sText );
}

sal_Bool SwAccessibleParagraph::replaceText(
    sal_Int32 nStartIndex, sal_Int32 nEndIndex,
    const OUString& sReplacement )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleEditableText );

    const OUString& rText = GetString();

    if( IsValidRange( nStartIndex, nEndIndex, rText.getLength() ) )
    {
        if( !IsEditableState() )
            return sal_False;

        SwTxtNode* pNode = const_cast<SwTxtNode*>( GetTxtNode() );

        // translate positions
        sal_uInt16 nStart, nEnd;
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
            const uno::Reference<text::XTextRange> xRange(
                SwXTextRange::CreateXTextRange(
                    *pNode->GetDoc(), aStartPos, &aEndPos));
            xRange->setString(sReplacement);

            // delete portion data
            ClearPortionData();
        }

        return bSuccess;
    }
    else
        throw lang::IndexOutOfBoundsException();
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
    const uno::Sequence<PropertyValue>& rAttributeSet )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    CHECK_FOR_DEFUNC( XAccessibleEditableText );

    const OUString& rText = GetString();

    if( ! IsValidRange( nStartIndex, nEndIndex, rText.getLength() ) )
        throw lang::IndexOutOfBoundsException();

    if( !IsEditableState() )
        return sal_False;

    // create a (dummy) text portion for the sole purpose of calling
    // setPropertyValue on it
    uno::Reference<XMultiPropertySet> xPortion = CreateUnoPortion( nStartIndex,
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
    uno::Sequence< OUString > aNames( nLength );
    OUString* pNames = aNames.getArray();
    uno::Sequence< uno::Any > aValues( nLength );
    uno::Any* pValues = aValues.getArray();
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
    catch (const UnknownPropertyException&)
    {
        // error handling through return code!
        bRet = sal_False;
    }

    return bRet;
}

sal_Bool SwAccessibleParagraph::setText( const OUString& sText )
    throw (uno::RuntimeException)
{
    return replaceText(0, GetString().getLength(), sText);
}

// XAccessibleSelection

void SwAccessibleParagraph::selectAccessibleChild(
    sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException,
            uno::RuntimeException )
{
    CHECK_FOR_DEFUNC( XAccessibleSelection );

    aSelectionHelper.selectAccessibleChild(nChildIndex);
}

sal_Bool SwAccessibleParagraph::isAccessibleChildSelected(
    sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException,
            uno::RuntimeException )
{
    CHECK_FOR_DEFUNC( XAccessibleSelection );

    return aSelectionHelper.isAccessibleChildSelected(nChildIndex);
}

void SwAccessibleParagraph::clearAccessibleSelection(  )
    throw ( uno::RuntimeException )
{
    CHECK_FOR_DEFUNC( XAccessibleSelection );

    aSelectionHelper.clearAccessibleSelection();
}

void SwAccessibleParagraph::selectAllAccessibleChildren(  )
    throw ( uno::RuntimeException )
{
    CHECK_FOR_DEFUNC( XAccessibleSelection );

    aSelectionHelper.selectAllAccessibleChildren();
}

sal_Int32 SwAccessibleParagraph::getSelectedAccessibleChildCount(  )
    throw ( uno::RuntimeException )
{
    CHECK_FOR_DEFUNC( XAccessibleSelection );

    return aSelectionHelper.getSelectedAccessibleChildCount();
}

uno::Reference<XAccessible> SwAccessibleParagraph::getSelectedAccessibleChild(
    sal_Int32 nSelectedChildIndex )
    throw ( lang::IndexOutOfBoundsException,
            uno::RuntimeException)
{
    CHECK_FOR_DEFUNC( XAccessibleSelection );

    return aSelectionHelper.getSelectedAccessibleChild(nSelectedChildIndex);
}

// index has to be treated as global child index.
void SwAccessibleParagraph::deselectAccessibleChild(
    sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException,
            uno::RuntimeException )
{
    CHECK_FOR_DEFUNC( XAccessibleSelection );

    aSelectionHelper.deselectAccessibleChild( nChildIndex );
}

// XAccessibleHypertext

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
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleHypertext );

    sal_Int32 nCount = 0;
    // #i77108# - provide hyperlinks also in editable documents.

    const SwTxtFrm *pTxtFrm = static_cast<const SwTxtFrm*>( GetFrm() );
    SwHyperlinkIter_Impl aIter( pTxtFrm );
    while( aIter.next() )
        nCount++;

    return nCount;
}

uno::Reference< XAccessibleHyperlink > SAL_CALL
    SwAccessibleParagraph::getHyperLink( sal_Int32 nLinkIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    CHECK_FOR_DEFUNC( XAccessibleHypertext );

    uno::Reference< XAccessibleHyperlink > xRet;

    // #i77108#
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
        throw lang::IndexOutOfBoundsException();

    return xRet;
}

sal_Int32 SAL_CALL SwAccessibleParagraph::getHyperLinkIndex( sal_Int32 nCharIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    CHECK_FOR_DEFUNC( XAccessibleHypertext );

    // parameter checking
    sal_Int32 nLength = GetString().getLength();
    if ( ! IsValidPosition( nCharIndex, nLength ) )
    {
        throw lang::IndexOutOfBoundsException();
    }

    sal_Int32 nRet = -1;
    // #i77108#
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

// #i71360#, #i108125# - adjustments for change tracking text markup
sal_Int32 SAL_CALL SwAccessibleParagraph::getTextMarkupCount( sal_Int32 nTextMarkupType )
                                        throw (lang::IllegalArgumentException,
                                               uno::RuntimeException)
{
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    std::auto_ptr<SwTextMarkupHelper> pTextMarkupHelper;
    SAL_WNODEPRECATED_DECLARATIONS_POP
    switch ( nTextMarkupType )
    {
        case text::TextMarkupType::TRACK_CHANGE_INSERTION:
        case text::TextMarkupType::TRACK_CHANGE_DELETION:
        case text::TextMarkupType::TRACK_CHANGE_FORMATCHANGE:
        {
            pTextMarkupHelper.reset( new SwTextMarkupHelper(
                GetPortionData(),
                *(mpParaChangeTrackInfo->getChangeTrackingTextMarkupList( nTextMarkupType ) )) );
        }
        break;
        default:
        {
            pTextMarkupHelper.reset( new SwTextMarkupHelper( GetPortionData(), *GetTxtNode() ) );
        }
    }

    return pTextMarkupHelper->getTextMarkupCount( nTextMarkupType );
}

/*accessibility::*/TextSegment SAL_CALL
        SwAccessibleParagraph::getTextMarkup( sal_Int32 nTextMarkupIndex,
                                              sal_Int32 nTextMarkupType )
                                        throw (lang::IndexOutOfBoundsException,
                                               lang::IllegalArgumentException,
                                               uno::RuntimeException)
{
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    std::auto_ptr<SwTextMarkupHelper> pTextMarkupHelper;
    SAL_WNODEPRECATED_DECLARATIONS_POP
    switch ( nTextMarkupType )
    {
        case text::TextMarkupType::TRACK_CHANGE_INSERTION:
        case text::TextMarkupType::TRACK_CHANGE_DELETION:
        case text::TextMarkupType::TRACK_CHANGE_FORMATCHANGE:
        {
            pTextMarkupHelper.reset( new SwTextMarkupHelper(
                GetPortionData(),
                *(mpParaChangeTrackInfo->getChangeTrackingTextMarkupList( nTextMarkupType ) )) );
        }
        break;
        default:
        {
            pTextMarkupHelper.reset( new SwTextMarkupHelper( GetPortionData(), *GetTxtNode() ) );
        }
    }

    return pTextMarkupHelper->getTextMarkup( nTextMarkupIndex, nTextMarkupType );
}

uno::Sequence< /*accessibility::*/TextSegment > SAL_CALL
        SwAccessibleParagraph::getTextMarkupAtIndex( sal_Int32 nCharIndex,
                                                     sal_Int32 nTextMarkupType )
                                        throw (lang::IndexOutOfBoundsException,
                                               lang::IllegalArgumentException,
                                               uno::RuntimeException)
{
    // parameter checking
    const sal_Int32 nLength = GetString().getLength();
    if ( ! IsValidPosition( nCharIndex, nLength ) )
    {
        throw lang::IndexOutOfBoundsException();
    }

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    std::auto_ptr<SwTextMarkupHelper> pTextMarkupHelper;
    SAL_WNODEPRECATED_DECLARATIONS_POP
    switch ( nTextMarkupType )
    {
        case text::TextMarkupType::TRACK_CHANGE_INSERTION:
        case text::TextMarkupType::TRACK_CHANGE_DELETION:
        case text::TextMarkupType::TRACK_CHANGE_FORMATCHANGE:
        {
            pTextMarkupHelper.reset( new SwTextMarkupHelper(
                GetPortionData(),
                *(mpParaChangeTrackInfo->getChangeTrackingTextMarkupList( nTextMarkupType ) )) );
        }
        break;
        default:
        {
            pTextMarkupHelper.reset( new SwTextMarkupHelper( GetPortionData(), *GetTxtNode() ) );
        }
    }

    return pTextMarkupHelper->getTextMarkupAtIndex( nCharIndex, nTextMarkupType );
}

// #i89175#
sal_Int32 SAL_CALL SwAccessibleParagraph::getLineNumberAtIndex( sal_Int32 nIndex )
                                        throw (lang::IndexOutOfBoundsException,
                                               uno::RuntimeException)
{
    // parameter checking
    const sal_Int32 nLength = GetString().getLength();
    if ( ! IsValidPosition( nIndex, nLength ) )
    {
        throw lang::IndexOutOfBoundsException();
    }

    const sal_Int32 nLineNo = GetPortionData().GetLineNo( nIndex );
    return nLineNo;
}

/*accessibility::*/TextSegment SAL_CALL
        SwAccessibleParagraph::getTextAtLineNumber( sal_Int32 nLineNo )
                                        throw (lang::IndexOutOfBoundsException,
                                               uno::RuntimeException)
{
    // parameter checking
    if ( nLineNo < 0 ||
         nLineNo >= GetPortionData().GetLineCount() )
    {
        throw lang::IndexOutOfBoundsException();
    }

    i18n::Boundary aLineBound;
    GetPortionData().GetBoundaryOfLine( nLineNo, aLineBound );

    /*accessibility::*/TextSegment aTextAtLine;
    const OUString rText = GetString();
    aTextAtLine.SegmentText = rText.copy( aLineBound.startPos,
                                          aLineBound.endPos - aLineBound.startPos );
    aTextAtLine.SegmentStart = aLineBound.startPos;
    aTextAtLine.SegmentEnd = aLineBound.endPos;

    return aTextAtLine;
}

/*accessibility::*/TextSegment SAL_CALL SwAccessibleParagraph::getTextAtLineWithCaret()
                                        throw (uno::RuntimeException)
{
    const sal_Int32 nLineNoOfCaret = getNumberOfLineWithCaret();

    if ( nLineNoOfCaret >= 0 &&
         nLineNoOfCaret < GetPortionData().GetLineCount() )
    {
        return getTextAtLineNumber( nLineNoOfCaret );
    }

    return /*accessibility::*/TextSegment();
}

sal_Int32 SAL_CALL SwAccessibleParagraph::getNumberOfLineWithCaret()
                                        throw (uno::RuntimeException)
{
    const sal_Int32 nCaretPos = getCaretPosition();
    const sal_Int32 nLength = GetString().getLength();
    if ( !IsValidPosition( nCaretPos, nLength ) )
    {
        return -1;
    }

    sal_Int32 nLineNo = GetPortionData().GetLineNo( nCaretPos );

    // special handling for cursor positioned at end of text line via End key
    if ( nCaretPos != 0 )
    {
        i18n::Boundary aLineBound;
        GetPortionData().GetBoundaryOfLine( nLineNo, aLineBound );
        if ( nCaretPos == aLineBound.startPos )
        {
            SwCrsrShell* pCrsrShell = SwAccessibleParagraph::GetCrsrShell();
            if ( pCrsrShell != 0 )
            {
                const awt::Rectangle aCharRect = getCharacterBounds( nCaretPos );

                const SwRect& aCursorCoreRect = pCrsrShell->GetCharRect();
                // translate core coordinates into accessibility coordinates
                Window *pWin = GetWindow();
                CHECK_FOR_WINDOW( XAccessibleComponent, pWin );

                Rectangle aScreenRect( GetMap()->CoreToPixel( aCursorCoreRect.SVRect() ));

                SwRect aFrmLogBounds( GetBounds( *(GetMap()) ) ); // twip rel to doc root
                Point aFrmPixPos( GetMap()->CoreToPixel( aFrmLogBounds.SVRect() ).TopLeft() );
                aScreenRect.Move( -aFrmPixPos.getX(), -aFrmPixPos.getY() );

                // convert into AWT Rectangle
                const awt::Rectangle aCursorRect( aScreenRect.Left(),
                                                  aScreenRect.Top(),
                                                  aScreenRect.GetWidth(),
                                                  aScreenRect.GetHeight() );

                if ( aCharRect.X != aCursorRect.X ||
                     aCharRect.Y != aCursorRect.Y )
                {
                    --nLineNo;
                }
            }
        }
    }

    return nLineNo;
}

// #i108125#
void SwAccessibleParagraph::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    mpParaChangeTrackInfo->reset();

    CheckRegistration( pOld, pNew );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
