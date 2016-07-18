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

#include <memory>
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
#include <sfx2/viewsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <unotools/charclass.hxx>
#include <unocrsr.hxx>
#include <unoport.hxx>
#include <doc.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <txtatr.hxx>
#include <acchyperlink.hxx>
#include <acchypertextdata.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <section.hxx>
#include <doctxm.hxx>
#include <comphelper/accessibletexthelper.hxx>
#include <algorithm>
#include <docufld.hxx>
#include <txtfld.hxx>
#include <fmtfld.hxx>
#include <modcfg.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "swmodule.hxx"
#include "redline.hxx"
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <wrong.hxx>
#include <editeng/brushitem.hxx>
#include <swatrset.hxx>
#include <frmatr.hxx>
#include <unosett.hxx>
#include <paratr.hxx>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <unomap.hxx>
#include <unoprnms.hxx>
#include <com/sun/star/text/WritingMode2.hpp>
#include <viewimp.hxx>
#include <textmarkuphelper.hxx>
#include <parachangetrackinginfo.hxx>
#include <com/sun/star/text/TextMarkupType.hpp>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <reffld.hxx>
#include <expfld.hxx>
#include <flddat.hxx>
#include <fldui.hrc>
#include "../../uibase/inc/fldmgr.hxx"
#include "fldbas.hxx"      // SwField

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::container;

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

const SwTextNode* SwAccessibleParagraph::GetTextNode() const
{
    const SwFrame* pFrame = GetFrame();
    OSL_ENSURE( pFrame->IsTextFrame(), "The text frame has mutated!" );

    const SwTextNode* pNode = static_cast<const SwTextFrame*>(pFrame)->GetTextNode();
    OSL_ENSURE( pNode != nullptr, "A text frame without a text node." );

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

    if( pCaret != nullptr )
    {
        const SwTextNode* pNode = GetTextNode();

        // check whether the point points into 'our' node
        SwPosition* pPoint = pCaret->GetPoint();
        if( pNode->GetIndex() == pPoint->nNode.GetIndex() )
        {
            // same node? Then check whether it's also within 'our' part
            // of the paragraph
            const sal_Int32 nIndex = pPoint->nContent.GetIndex();
            if(!GetPortionData().IsValidCorePosition( nIndex ) ||
                ( GetPortionData().IsZeroCorePositionData() && nIndex== 0) )
            {
                const SwTextFrame *pTextFrame = dynamic_cast<const SwTextFrame*>( GetFrame()  );
                bool bFormat = (pTextFrame && pTextFrame->HasPara());
                if(bFormat)
                {
                    ClearPortionData();
                    UpdatePortionData();
                }
            }
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

bool SwAccessibleParagraph::GetSelection(
    sal_Int32& nStart, sal_Int32& nEnd)
{
    bool bRet = false;
    nStart = -1;
    nEnd = -1;

    // get the selection, and test whether it affects our text node
    SwPaM* pCursor = GetCursor( true ); // #i27301# - consider adjusted method signature
    if( pCursor != nullptr )
    {
        // get SwPosition for my node
        const SwTextNode* pNode = GetTextNode();
        sal_uLong nHere = pNode->GetIndex();

        // iterate over ring
        for(SwPaM& rTmpCursor : pCursor->GetRingContainer())
        {
            // ignore, if no mark
            if( rTmpCursor.HasMark() )
            {
                // check whether nHere is 'inside' pCursor
                SwPosition* pStart = rTmpCursor.Start();
                sal_uLong nStartIndex = pStart->nNode.GetIndex();
                SwPosition* pEnd = rTmpCursor.End();
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
                        const sal_Int32 nCoreStart = pStart->nContent.GetIndex();
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
                        const sal_Int32 nCoreEnd = pEnd->nContent.GetIndex();
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
                        bRet = true;
                    }
                }
                // else: this PaM doesn't point to this paragraph
            }
            // else: this PaM is collapsed and doesn't select anything
            if(bRet)
                break;
        }
    // else: nocursor -> no selection
    }
    return bRet;
}

// #i27301# - new parameter <_bForSelection>
SwPaM* SwAccessibleParagraph::GetCursor( const bool _bForSelection )
{
    // get the cursor shell; if we don't have any, we don't have a
    // cursor/selection either
    SwPaM* pCursor = nullptr;
    SwCursorShell* pCursorShell = SwAccessibleParagraph::GetCursorShell();
    // #i27301# - if cursor is retrieved for selection, the cursors for
    // a table selection has to be returned.
    if ( pCursorShell != nullptr &&
         ( _bForSelection || !pCursorShell->IsTableMode() ) )
    {
        SwFEShell *pFESh = dynamic_cast<const SwFEShell*>( pCursorShell) !=  nullptr
                            ? static_cast< SwFEShell * >( pCursorShell ) : nullptr;
        if( !pFESh ||
            !(pFESh->IsFrameSelected() || pFESh->IsObjSelected() > 0) )
        {
            // get the selection, and test whether it affects our text node
            pCursor = pCursorShell->GetCursor( false /* ??? */ );
        }
    }

    return pCursor;
}

bool SwAccessibleParagraph::IsHeading() const
{
    const SwTextNode *pTextNd = GetTextNode();
    return pTextNd->IsOutline();
}

void SwAccessibleParagraph::GetStates(
        ::utl::AccessibleStateSetHelper& rStateSet )
{
    SwAccessibleContext::GetStates( rStateSet );

    // MULTILINE
    rStateSet.AddState( AccessibleStateType::MULTI_LINE );

    // MULTISELECTABLE
    SwCursorShell *pCursorSh = GetCursorShell();
    if( pCursorSh )
        rStateSet.AddState( AccessibleStateType::MULTI_SELECTABLE );

    // FOCUSABLE
    if( pCursorSh )
        rStateSet.AddState( AccessibleStateType::FOCUSABLE );

    // FOCUSED (simulates node index of cursor)
    SwPaM* pCaret = GetCursor( false ); // #i27301# - consider adjusted method signature
    const SwTextNode* pTextNd = GetTextNode();
    if( pCaret != nullptr && pTextNd != nullptr &&
        pTextNd->GetIndex() == pCaret->GetPoint()->nNode.GetIndex() &&
        nOldCaretPos != -1)
    {
        vcl::Window *pWin = GetWindow();
        if( pWin && pWin->HasFocus() )
            rStateSet.AddState( AccessibleStateType::FOCUSED );
        ::rtl::Reference < SwAccessibleContext > xThis( this );
        GetMap()->SetCursorContext( xThis );
    }
}

void SwAccessibleParagraph::InvalidateContent_( bool bVisibleDataFired )
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
        uno::Reference< XAccessible > xparent = getAccessibleParent();
        uno::Reference< XAccessibleContext > xAccContext(xparent,uno::UNO_QUERY);
        if (xAccContext.is() && xAccContext->getAccessibleRole() == AccessibleRole::TABLE_CELL)
        {
            SwAccessibleContext* pPara = static_cast< SwAccessibleContext* >(xparent.get());
            if(pPara)
            {
                AccessibleEventObject aParaEvent;
                aParaEvent.EventId = AccessibleEventId::VALUE_CHANGED;
                pPara->FireAccessibleEvent(aParaEvent);
            }
        }
    }
    else if( !bVisibleDataFired )
    {
        FireVisibleDataEvent();
    }

    bool bNewIsHeading = IsHeading();
    //Get the real heading level, Heading1 ~ Heading10
    nHeadingLevel = GetRealHeadingLevel();
    bool bOldIsHeading;
    {
        osl::MutexGuard aGuard( m_Mutex );
        bOldIsHeading = bIsHeading;
        if( bIsHeading != bNewIsHeading )
            bIsHeading = bNewIsHeading;
    }

    if( bNewIsHeading != bOldIsHeading )
    {
        // The role has changed
        AccessibleEventObject aEvent;
        aEvent.EventId = AccessibleEventId::ROLE_CHANGED;

        FireAccessibleEvent( aEvent );
    }

    if( rText != sOldText )
    {
        OUString sNewDesc( GetDescription() );
        OUString sOldDesc;
        {
            osl::MutexGuard aGuard( m_Mutex );
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

void SwAccessibleParagraph::InvalidateCursorPos_()
{
    // The text is changed
    sal_Int32 nNew = GetCaretPos();
    sal_Int32 nOld;
    {
        osl::MutexGuard aGuard( m_Mutex );
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

    vcl::Window *pWin = GetWindow();
    if( nOld != nNew )
    {
        // The cursor's node position is simulated by the focus!
        if( pWin && pWin->HasFocus() && -1 == nOld )
            FireStateChangedEvent( AccessibleStateType::FOCUSED, true );

        AccessibleEventObject aEvent;
        aEvent.EventId = AccessibleEventId::CARET_CHANGED;
        aEvent.OldValue <<= nOld;
        aEvent.NewValue <<= nNew;

        FireAccessibleEvent( aEvent );

        if( pWin && pWin->HasFocus() && -1 == nNew )
            FireStateChangedEvent( AccessibleStateType::FOCUSED, false );
        //To send TEXT_SELECTION_CHANGED event
        sal_Int32 nStart=0;
        sal_Int32 nEnd  =0;
        bool bCurSelection=GetSelection(nStart,nEnd);
        if(m_bLastHasSelection || bCurSelection )
        {
            aEvent.EventId = AccessibleEventId::TEXT_SELECTION_CHANGED;
            aEvent.OldValue <<= uno::Any();
            aEvent.NewValue <<= uno::Any();
            FireAccessibleEvent(aEvent);
        }
        m_bLastHasSelection =bCurSelection;
    }
}

void SwAccessibleParagraph::InvalidateFocus_()
{
    vcl::Window *pWin = GetWindow();
    if( pWin )
    {
        sal_Int32 nPos;
        {
            osl::MutexGuard aGuard( m_Mutex );
            nPos = nOldCaretPos;
        }
        OSL_ENSURE( nPos != -1, "focus object should be selected" );

        FireStateChangedEvent( AccessibleStateType::FOCUSED,
                               pWin->HasFocus() && nPos != -1 );
    }
}

SwAccessibleParagraph::SwAccessibleParagraph(
        SwAccessibleMap* pInitMap,
        const SwTextFrame& rTextFrame )
    : SwClient( const_cast<SwTextNode*>(rTextFrame.GetTextNode()) ) // #i108125#
    , SwAccessibleContext( pInitMap, AccessibleRole::PARAGRAPH, &rTextFrame )
    , sDesc()
    , pPortionData( nullptr )
    , pHyperTextData( nullptr )
    , nOldCaretPos( -1 )
    , bIsHeading( false )
    //Get the real heading level, Heading1 ~ Heading10
    , nHeadingLevel (-1)
    , aSelectionHelper( *this )
    , mpParaChangeTrackInfo( new SwParaChangeTrackingInfo( rTextFrame ) ) // #i108125#
    , m_bLastHasSelection(false)  //To add TEXT_SELECTION_CHANGED event
{
    SolarMutexGuard aGuard;

    bIsHeading = IsHeading();
    //Get the real heading level, Heading1 ~ Heading10
    nHeadingLevel = GetRealHeadingLevel();
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
    if(GetRegisteredIn())
        GetRegisteredIn()->Remove(this);
}

bool SwAccessibleParagraph::HasCursor()
{
    osl::MutexGuard aGuard( m_Mutex );
    return nOldCaretPos != -1;
}

void SwAccessibleParagraph::UpdatePortionData()
    throw( uno::RuntimeException )
{
    // obtain the text frame
    OSL_ENSURE( GetFrame() != nullptr, "The text frame has vanished!" );
    OSL_ENSURE( GetFrame()->IsTextFrame(), "The text frame has mutated!" );
    const SwTextFrame* pFrame = static_cast<const SwTextFrame*>( GetFrame() );

    // build new portion data
    delete pPortionData;
    pPortionData = new SwAccessiblePortionData(
        pFrame->GetTextNode(), GetMap()->GetShell()->GetViewOptions() );
    pFrame->VisitPortions( *pPortionData );

    OSL_ENSURE( pPortionData != nullptr, "UpdatePortionData() failed" );
}

void SwAccessibleParagraph::ClearPortionData()
{
    delete pPortionData;
    pPortionData = nullptr;

    delete pHyperTextData;
    pHyperTextData = nullptr;
}

void SwAccessibleParagraph::ExecuteAtViewShell( sal_uInt16 nSlot )
{
    OSL_ENSURE( GetMap() != nullptr, "no map?" );
    SwViewShell* pViewShell = GetMap()->GetShell();

    OSL_ENSURE( pViewShell != nullptr, "View shell expected!" );
    SfxViewShell* pSfxShell = pViewShell->GetSfxViewShell();

    OSL_ENSURE( pSfxShell != nullptr, "SfxViewShell shell expected!" );
    if( !pSfxShell )
        return;

    SfxViewFrame *pFrame = pSfxShell->GetViewFrame();
    OSL_ENSURE( pFrame != nullptr, "View frame expected!" );
    if( !pFrame )
        return;

    SfxDispatcher *pDispatcher = pFrame->GetDispatcher();
    OSL_ENSURE( pDispatcher != nullptr, "Dispatcher expected!" );
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

    const sal_Int32 nStart = GetPortionData().GetModelPosition( nStartIndex );
    const sal_Int32 nEnd = (nEndIndex == -1) ? (nStart + 1) :
                        GetPortionData().GetModelPosition( nEndIndex );

    // create UNO cursor
    SwTextNode* pTextNode = const_cast<SwTextNode*>( GetTextNode() );
    SwIndex aIndex( pTextNode, nStart );
    SwPosition aStartPos( *pTextNode, aIndex );
    auto pUnoCursor(pTextNode->GetDoc()->CreateUnoCursor( aStartPos ));
    pUnoCursor->SetMark();
    pUnoCursor->GetMark()->nContent = nEnd;

    // create a (dummy) text portion to be returned
    uno::Reference<text::XText> aEmpty;
    SwXTextPortion* pPortion =
        new SwXTextPortion ( pUnoCursor.get(), aEmpty, PORTION_TEXT);

    return pPortion;
}

// range checking for parameter

bool SwAccessibleParagraph::IsValidChar(
    sal_Int32 nPos, sal_Int32 nLength)
{
    return (nPos >= 0) && (nPos < nLength);
}

bool SwAccessibleParagraph::IsValidPosition(
    sal_Int32 nPos, sal_Int32 nLength)
{
    return (nPos >= 0) && (nPos <= nLength);
}

bool SwAccessibleParagraph::IsValidRange(
    sal_Int32 nBegin, sal_Int32 nEnd, sal_Int32 nLength)
{
    return IsValidPosition(nBegin, nLength) && IsValidPosition(nEnd, nLength);
}

SwTOXSortTabBase* SwAccessibleParagraph::GetTOXSortTabBase()
{
    const SwTextNode* pTextNd = GetTextNode();
    if( pTextNd )
    {
        const SwSectionNode * pSectNd = pTextNd->FindSectionNode();
        if( pSectNd )
        {
            const SwSection * pSect = &pSectNd->GetSection();
            const  SwTOXBaseSection *pTOXBaseSect = static_cast<const SwTOXBaseSection *>(pSect);
            if( pSect->GetType() == TOX_CONTENT_SECTION )
            {
                SwTOXSortTabBase* pSortBase = nullptr;
                size_t nSize = pTOXBaseSect->GetTOXSortTabBases().size();

                for(size_t nIndex = 0; nIndex<nSize; nIndex++ )
                {
                    pSortBase = pTOXBaseSect->GetTOXSortTabBases()[nIndex];
                    if( pSortBase->pTOXNd == pTextNd )
                        break;
                }

                if (pSortBase)
                {
                    return pSortBase;
                }
            }
        }
    }
    return nullptr;
}

//the function is to check whether the position is in a redline range.
const SwRangeRedline* SwAccessibleParagraph::GetRedlineAtIndex( sal_Int32 )
{
    const SwRangeRedline* pRedline = nullptr;
    SwPaM* pCrSr = GetCursor( true );
    if ( pCrSr )
    {
        SwPosition* pStart = pCrSr->Start();
        const SwTextNode* pNode = GetTextNode();
        if ( pNode )
        {
            const SwDoc* pDoc = pNode->GetDoc();
            if ( pDoc )
            {
                pRedline = pDoc->getIDocumentRedlineAccess().GetRedline( *pStart, nullptr );
            }
        }
    }

    return pRedline;
}

// text boundaries

bool SwAccessibleParagraph::GetCharBoundary(
    i18n::Boundary& rBound,
    const OUString&,
    sal_Int32 nPos )
{
    if( GetPortionData().FillBoundaryIFDateField( rBound,  nPos) )
        return true;

    rBound.startPos = nPos;
    rBound.endPos = nPos+1;
    return true;
}

bool SwAccessibleParagraph::GetWordBoundary(
    i18n::Boundary& rBound,
    const OUString& rText,
    sal_Int32 nPos )
{
    bool bRet = false;

    // now ask the Break-Iterator for the word
    OSL_ENSURE( g_pBreakIt != nullptr, "We always need a break." );
    OSL_ENSURE( g_pBreakIt->GetBreakIter().is(), "No break-iterator." );
    if( g_pBreakIt->GetBreakIter().is() )
    {
        // get locale for this position
        const sal_Int32 nModelPos = GetPortionData().GetModelPosition( nPos );
        lang::Locale aLocale = g_pBreakIt->GetLocale(
                              GetTextNode()->GetLang( nModelPos ) );

        // which type of word are we interested in?
        // (DICTIONARY_WORD includes punctuation, ANY_WORD doesn't.)
        const sal_Int16 nWordType = i18n::WordType::ANY_WORD;

        // get word boundary, as the Break-Iterator sees fit.
        rBound = g_pBreakIt->GetBreakIter()->getWordBoundary(
            rText, nPos, aLocale, nWordType, true );

        // If we have a break-iterator let's trust that it
        // does the right thing.
        bRet = true;
    }
    else
    {
        // no break Iterator -> no word
        rBound.startPos = nPos;
        rBound.endPos = nPos;
    }

    return bRet;
}

bool SwAccessibleParagraph::GetSentenceBoundary(
    i18n::Boundary& rBound,
    const OUString& rText,
    sal_Int32 nPos )
{
    const sal_Unicode* pStr = rText.getStr();
    if (pStr)
    {
        while( pStr[nPos] == sal_Unicode(' ') && nPos < rText.getLength())
            nPos++;
    }

    GetPortionData().GetSentenceBoundary( rBound, nPos );
    return true;
}

bool SwAccessibleParagraph::GetLineBoundary(
    i18n::Boundary& rBound,
    const OUString& rText,
    sal_Int32 nPos )
{
    if( rText.getLength() == nPos )
        GetPortionData().GetLastLineBoundary( rBound );
    else
        GetPortionData().GetLineBoundary( rBound, nPos );
    return true;
}

bool SwAccessibleParagraph::GetParagraphBoundary(
    i18n::Boundary& rBound,
    const OUString& rText,
    sal_Int32 )
{
    rBound.startPos = 0;
    rBound.endPos = rText.getLength();
    return true;
}

bool SwAccessibleParagraph::GetAttributeBoundary(
    i18n::Boundary& rBound,
    const OUString&,
    sal_Int32 nPos )
{
    GetPortionData().GetAttributeBoundary( rBound, nPos );
    return true;
}

bool SwAccessibleParagraph::GetGlyphBoundary(
    i18n::Boundary& rBound,
    const OUString& rText,
    sal_Int32 nPos )
{
    bool bRet = false;

    // ask the Break-Iterator for the glyph by moving one cell
    // forward, and then one cell back
    OSL_ENSURE( g_pBreakIt != nullptr, "We always need a break." );
    OSL_ENSURE( g_pBreakIt->GetBreakIter().is(), "No break-iterator." );
    if( g_pBreakIt->GetBreakIter().is() )
    {
        // get locale for this position
        const sal_Int32 nModelPos = GetPortionData().GetModelPosition( nPos );
        lang::Locale aLocale = g_pBreakIt->GetLocale(
                              GetTextNode()->GetLang( nModelPos ) );

        // get word boundary, as the Break-Iterator sees fit.
        const sal_Int16 nIterMode = i18n::CharacterIteratorMode::SKIPCELL;
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

bool SwAccessibleParagraph::GetTextBoundary(
    i18n::Boundary& rBound,
    const OUString& rText,
    sal_Int32 nPos,
    sal_Int16 nTextType )
    throw (
        lang::IndexOutOfBoundsException,
        lang::IllegalArgumentException,
        uno::RuntimeException,
        std::exception)
{
    // error checking
    if( !( AccessibleTextType::LINE == nTextType
                ? IsValidPosition( nPos, rText.getLength() )
                : IsValidChar( nPos, rText.getLength() ) ) )
        throw lang::IndexOutOfBoundsException();

    bool bRet;

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
            //Solve the problem of returning wrong LINE and PARAGRAPH
            if((nPos == rText.getLength()) && nPos > 0)
                bRet = GetLineBoundary( rBound, rText, nPos - 1);
            else
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

OUString SAL_CALL SwAccessibleParagraph::getAccessibleDescription()
        throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleContext );

    osl::MutexGuard aGuard2( m_Mutex );
    if( sDesc.isEmpty() )
        sDesc = GetDescription();

    return sDesc;
}

lang::Locale SAL_CALL SwAccessibleParagraph::getLocale()
        throw (IllegalAccessibleComponentStateException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    const SwTextFrame *pTextFrame = dynamic_cast<const SwTextFrame*>( GetFrame()  );
    if( !pTextFrame )
    {
        THROW_RUNTIME_EXCEPTION( XAccessibleContext, "internal error (no text frame)" );
    }

    const SwTextNode *pTextNd = pTextFrame->GetTextNode();
    lang::Locale aLoc( g_pBreakIt->GetLocale( pTextNd->GetLang( 0 ) ) );

    return aLoc;
}

// #i27138# - paragraphs are in relation CONTENT_FLOWS_FROM and/or CONTENT_FLOWS_TO
uno::Reference<XAccessibleRelationSet> SAL_CALL SwAccessibleParagraph::getAccessibleRelationSet()
    throw ( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    CHECK_FOR_DEFUNC( XAccessibleContext );

    utl::AccessibleRelationSetHelper* pHelper = new utl::AccessibleRelationSetHelper();

    const SwTextFrame* pTextFrame = dynamic_cast<const SwTextFrame*>(GetFrame());
    OSL_ENSURE( pTextFrame,
            "<SwAccessibleParagraph::getAccessibleRelationSet()> - missing text frame");
    if ( pTextFrame )
    {
        const SwContentFrame* pPrevContentFrame( pTextFrame->FindPrevCnt() );
        if ( pPrevContentFrame )
        {
            uno::Sequence< uno::Reference<XInterface> > aSequence { GetMap()->GetContext( pPrevContentFrame ) };
            AccessibleRelation aAccRel( AccessibleRelationType::CONTENT_FLOWS_FROM,
                                        aSequence );
            pHelper->AddRelation( aAccRel );
        }

        const SwContentFrame* pNextContentFrame( pTextFrame->FindNextCnt( true ) );
        if ( pNextContentFrame )
        {
            uno::Sequence< uno::Reference<XInterface> > aSequence { GetMap()->GetContext( pNextContentFrame ) };
            AccessibleRelation aAccRel( AccessibleRelationType::CONTENT_FLOWS_TO,
                                        aSequence );
            pHelper->AddRelation( aAccRel );
        }
    }

    return pHelper;
}

void SAL_CALL SwAccessibleParagraph::grabFocus()
        throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleContext );

    // get cursor shell
    SwCursorShell *pCursorSh = GetCursorShell();
    SwPaM *pCursor = GetCursor( false ); // #i27301# - consider new method signature
    const SwTextFrame *pTextFrame = static_cast<const SwTextFrame*>( GetFrame() );
    const SwTextNode* pTextNd = pTextFrame->GetTextNode();

    if( pCursorSh != nullptr && pTextNd != nullptr &&
        ( pCursor == nullptr ||
           pCursor->GetPoint()->nNode.GetIndex() != pTextNd->GetIndex() ||
          !pTextFrame->IsInside( pCursor->GetPoint()->nContent.GetIndex()) ) )
    {
        // create pam for selection
        SwIndex aIndex( const_cast< SwTextNode * >( pTextNd ),
                        pTextFrame->GetOfst() );
        SwPosition aStartPos( *pTextNd, aIndex );
        SwPaM aPaM( aStartPos );

        // set PaM at cursor shell
        Select( aPaM );

    }

    // ->#i13955#
    vcl::Window * pWindow = GetWindow();

    if (pWindow != nullptr)
        pWindow->GrabFocus();
    // <-#i13955#
}

// #i71385#
static bool lcl_GetBackgroundColor( Color & rColor,
                             const SwFrame* pFrame,
                             SwCursorShell* pCursorSh )
{
    const SvxBrushItem* pBackgrdBrush = nullptr;
    const Color* pSectionTOXColor = nullptr;
    SwRect aDummyRect;

    //UUUU
    drawinglayer::attribute::SdrAllFillAttributesHelperPtr aFillAttributes;

    if ( pFrame &&
         pFrame->GetBackgroundBrush( aFillAttributes, pBackgrdBrush, pSectionTOXColor, aDummyRect, false ) )
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
    else if ( pCursorSh )
    {
        rColor = pCursorSh->Imp()->GetRetoucheColor();
        return true;
    }

    return false;
}

sal_Int32 SAL_CALL SwAccessibleParagraph::getForeground()
                                throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

    Color aBackgroundCol;

    if ( lcl_GetBackgroundColor( aBackgroundCol, GetFrame(), GetCursorShell() ) )
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
                                throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

    Color aBackgroundCol;

    if ( lcl_GetBackgroundColor( aBackgroundCol, GetFrame(), GetCursorShell() ) )
    {
        return aBackgroundCol.GetColor();
    }

    return SwAccessibleContext::getBackground();
}

OUString SAL_CALL SwAccessibleParagraph::getImplementationName()
        throw( uno::RuntimeException, std::exception )
{
    return OUString(sImplementationName);
}

sal_Bool SAL_CALL SwAccessibleParagraph::supportsService(
        const OUString& sTestServiceName)
    throw (uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, sTestServiceName);
}

uno::Sequence< OUString > SAL_CALL SwAccessibleParagraph::getSupportedServiceNames()
        throw( uno::RuntimeException, std::exception )
{
    uno::Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = sServiceName;
    pArray[1] = sAccessibleServiceName;
    return aRet;
}

uno::Sequence< OUString > const & getAttributeNames()
{
    static uno::Sequence< OUString >* pNames = nullptr;

    if( pNames == nullptr )
    {
        // Add the font name to attribute list
        uno::Sequence< OUString >* pSeq = new uno::Sequence< OUString >( 13 );

        OUString* pStrings = pSeq->getArray();

        // sorted list of strings
        sal_Int32 i = 0;

        pStrings[i++] = UNO_NAME_CHAR_BACK_COLOR;
        pStrings[i++] = UNO_NAME_CHAR_COLOR;
        pStrings[i++] = UNO_NAME_CHAR_CONTOURED;
        pStrings[i++] = UNO_NAME_CHAR_EMPHASIS;
        pStrings[i++] = UNO_NAME_CHAR_ESCAPEMENT;
        pStrings[i++] = UNO_NAME_CHAR_FONT_NAME;
        pStrings[i++] = UNO_NAME_CHAR_HEIGHT;
        pStrings[i++] = UNO_NAME_CHAR_POSTURE;
        pStrings[i++] = UNO_NAME_CHAR_SHADOWED;
        pStrings[i++] = UNO_NAME_CHAR_STRIKEOUT;
        pStrings[i++] = UNO_NAME_CHAR_UNDERLINE;
        pStrings[i++] = UNO_NAME_CHAR_UNDERLINE_COLOR;
        pStrings[i++] = UNO_NAME_CHAR_WEIGHT;
        assert(i == pSeq->getLength());
        pNames = pSeq;
    }
    return *pNames;
}

uno::Sequence< OUString > const & getSupplementalAttributeNames()
{
    static uno::Sequence< OUString >* pNames = nullptr;

    if( pNames == nullptr )
    {
        uno::Sequence< OUString >* pSeq = new uno::Sequence< OUString >( 9 );

        OUString* pStrings = pSeq->getArray();

        // sorted list of strings
        sal_Int32 i = 0;

        pStrings[i++] = UNO_NAME_NUMBERING_LEVEL;
        pStrings[i++] = UNO_NAME_NUMBERING_RULES;
        pStrings[i++] = UNO_NAME_PARA_ADJUST;
        pStrings[i++] = UNO_NAME_PARA_BOTTOM_MARGIN;
        pStrings[i++] = UNO_NAME_PARA_FIRST_LINE_INDENT;
        pStrings[i++] = UNO_NAME_PARA_LEFT_MARGIN;
        pStrings[i++] = UNO_NAME_PARA_LINE_SPACING;
        pStrings[i++] = UNO_NAME_PARA_RIGHT_MARGIN;
        pStrings[i++] = UNO_NAME_TABSTOPS;
        assert(i == pSeq->getLength());
        pNames = pSeq;
    }
    return *pNames;
}

// XInterface

uno::Any SwAccessibleParagraph::queryInterface( const uno::Type& rType )
    throw (uno::RuntimeException, std::exception)
{
    uno::Any aRet;
    if ( rType == cppu::UnoType<XAccessibleText>::get())
    {
        uno::Reference<XAccessibleText> aAccText = static_cast<XAccessibleText *>(*this); // resolve ambiguity
        aRet <<= aAccText;
    }
    else if ( rType == cppu::UnoType<XAccessibleEditableText>::get())
    {
        uno::Reference<XAccessibleEditableText> aAccEditText = this;
        aRet <<= aAccEditText;
    }
    else if ( rType == cppu::UnoType<XAccessibleSelection>::get())
    {
        uno::Reference<XAccessibleSelection> aAccSel = this;
        aRet <<= aAccSel;
    }
    else if ( rType == cppu::UnoType<XAccessibleHypertext>::get())
    {
        uno::Reference<XAccessibleHypertext> aAccHyp = this;
        aRet <<= aAccHyp;
    }
    // #i63870#
    // add interface com::sun:star:accessibility::XAccessibleTextAttributes
    else if ( rType == cppu::UnoType<XAccessibleTextAttributes>::get())
    {
        uno::Reference<XAccessibleTextAttributes> aAccTextAttr = this;
        aRet <<= aAccTextAttr;
    }
    // #i89175#
    // add interface com::sun:star:accessibility::XAccessibleTextMarkup
    else if ( rType == cppu::UnoType<XAccessibleTextMarkup>::get())
    {
        uno::Reference<XAccessibleTextMarkup> aAccTextMarkup = this;
        aRet <<= aAccTextMarkup;
    }
    // add interface com::sun:star:accessibility::XAccessibleMultiLineText
    else if ( rType == cppu::UnoType<XAccessibleMultiLineText>::get())
    {
        uno::Reference<XAccessibleMultiLineText> aAccMultiLineText = this;
        aRet <<= aAccMultiLineText;
    }
    else if ( rType == cppu::UnoType<XAccessibleTextSelection>::get())
    {
        uno::Reference< css::accessibility::XAccessibleTextSelection > aTextExtension = this;
        aRet <<= aTextExtension;
    }
    else if ( rType == cppu::UnoType<XAccessibleExtendedAttributes>::get())
    {
        uno::Reference<XAccessibleExtendedAttributes> xAttr = this;
        aRet <<= xAttr;
    }
    else
    {
        aRet = SwAccessibleContext::queryInterface(rType);
    }

    return aRet;
}

// XTypeProvider
uno::Sequence< uno::Type > SAL_CALL SwAccessibleParagraph::getTypes() throw(uno::RuntimeException, std::exception)
{
    uno::Sequence< uno::Type > aTypes( SwAccessibleContext::getTypes() );

    sal_Int32 nIndex = aTypes.getLength();
    // #i63870# - add type accessibility::XAccessibleTextAttributes
    // #i89175# - add type accessibility::XAccessibleTextMarkup and
    // accessibility::XAccessibleMultiLineText
    aTypes.realloc( nIndex + 6 );

    uno::Type* pTypes = aTypes.getArray();
    pTypes[nIndex++] = cppu::UnoType<XAccessibleEditableText>::get();
    pTypes[nIndex++] = cppu::UnoType<XAccessibleTextAttributes>::get();
    pTypes[nIndex++] = ::cppu::UnoType<XAccessibleSelection>::get();
    pTypes[nIndex++] = cppu::UnoType<XAccessibleTextMarkup>::get();
    pTypes[nIndex++] = cppu::UnoType<XAccessibleMultiLineText>::get();
    pTypes[nIndex] = cppu::UnoType<XAccessibleHypertext>::get();

    return aTypes;
}

uno::Sequence< sal_Int8 > SAL_CALL SwAccessibleParagraph::getImplementationId()
        throw(uno::RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}

// XAccesibleText

sal_Int32 SwAccessibleParagraph::getCaretPosition()
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    sal_Int32 nRet = GetCaretPos();
    {
        osl::MutexGuard aOldCaretPosGuard( m_Mutex );
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
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    // parameter checking
    sal_Int32 nLength = GetString().getLength();
    if ( ! IsValidPosition( nIndex, nLength ) )
    {
        throw lang::IndexOutOfBoundsException();
    }

    bool bRet = false;

    // get cursor shell
    SwCursorShell* pCursorShell = GetCursorShell();
    if( pCursorShell != nullptr )
    {
        // create pam for selection
        SwTextNode* pNode = const_cast<SwTextNode*>( GetTextNode() );
        SwIndex aIndex( pNode, GetPortionData().GetModelPosition(nIndex));
        SwPosition aStartPos( *pNode, aIndex );
        SwPaM aPaM( aStartPos );

        // set PaM at cursor shell
        bRet = Select( aPaM );
    }

    return bRet;
}

sal_Unicode SwAccessibleParagraph::getCharacter( sal_Int32 nIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    OUString sText( GetString() );

    // return character (if valid)
    if( IsValidChar(nIndex, sText.getLength() ) )
    {
        return sText[nIndex];
    }
    else
        throw lang::IndexOutOfBoundsException();
}

css::uno::Sequence< css::style::TabStop > SwAccessibleParagraph::GetCurrentTabStop( sal_Int32 nIndex  )
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    /*  #i12332# The position after the string needs special treatment.
        IsValidChar -> IsValidPosition
    */
    if( ! (IsValidPosition( nIndex, GetString().getLength() ) ) )
        throw lang::IndexOutOfBoundsException();

    /*  #i12332#  */
    bool bBehindText = false;
    if ( nIndex == GetString().getLength() )
        bBehindText = true;

    // get model position & prepare GetCharRect() arguments
    SwCursorMoveState aMoveState;
    aMoveState.m_bRealHeight = true;
    aMoveState.m_bRealWidth = true;
    SwSpecialPos aSpecialPos;
    SwTextNode* pNode = const_cast<SwTextNode*>( GetTextNode() );

    /*  #i12332# FillSpecialPos does not accept nIndex ==
         GetString().getLength(). In that case nPos is set to the
         length of the string in the core. This way GetCharRect
         returns the rectangle for a cursor at the end of the
         paragraph. */
    const sal_Int32 nPos = bBehindText
        ? pNode->GetText().getLength()
        : GetPortionData().FillSpecialPos(nIndex, aSpecialPos, aMoveState.m_pSpecialPos );

    // call GetCharRect
    SwRect aCoreRect;
    SwIndex aIndex( pNode, nPos );
    SwPosition aPosition( *pNode, aIndex );
    GetFrame()->GetCharRect( aCoreRect, aPosition, &aMoveState );

    // already get the caret position
    css::uno::Sequence< css::style::TabStop > tabs;
    const sal_Int32 nStrLen = GetTextNode()->GetText().getLength();
    if( nStrLen > 0 )
    {
        SwFrame* pTFrame = const_cast<SwFrame*>(GetFrame());
        tabs = pTFrame->GetTabStopInfo(aCoreRect.Left());
    }

    if( tabs.hasElements() )
    {
        // translate core coordinates into accessibility coordinates
        vcl::Window *pWin = GetWindow();
        CHECK_FOR_WINDOW( XAccessibleComponent, pWin );

        SwRect aTmpRect(0, 0, tabs[0].Position, 0);

        Rectangle aScreenRect( GetMap()->CoreToPixel( aTmpRect.SVRect() ));
        SwRect aFrameLogBounds( GetBounds( *(GetMap()) ) ); // twip rel to doc root

        Point aFramePixPos( GetMap()->CoreToPixel( aFrameLogBounds.SVRect() ).TopLeft() );
        aScreenRect.Move( -aFramePixPos.X(), -aFramePixPos.Y() );

        tabs[0].Position = aScreenRect.GetWidth();
    }

    return tabs;
}

struct IndexCompare
{
    const PropertyValue* pValues;
    explicit IndexCompare( const PropertyValue* pVals ) : pValues(pVals) {}
    bool operator() ( sal_Int32 a, sal_Int32 b ) const
    {
        return (pValues[a].Name < pValues[b].Name);
    }
};

OUString SwAccessibleParagraph::GetFieldTypeNameAtIndex(sal_Int32 nIndex)
{
    OUString strTypeName;
    SwFieldMgr aMgr;
    SwTextField* pTextField = nullptr;
    SwTextNode* pTextNd = const_cast<SwTextNode*>( GetTextNode() );
    SwIndex fldIndex( pTextNd, nIndex );
    sal_Int32 nFieldIndex = GetPortionData().GetFieldIndex(nIndex);
    if (nFieldIndex >= 0)
    {
        const SwpHints* pSwpHints = GetTextNode()->GetpSwpHints();
        if (pSwpHints)
        {
            const size_t nSize = pSwpHints->Count();
            for( size_t i = 0; i < nSize; ++i )
            {
                const SwTextAttr* pHt = pSwpHints->Get(i);
                if ( ( pHt->Which() == RES_TXTATR_FIELD
                       || pHt->Which() == RES_TXTATR_ANNOTATION
                       || pHt->Which() == RES_TXTATR_INPUTFIELD )
                     && (nFieldIndex-- == 0))
                {
                    pTextField = const_cast<SwTextField*>(
                                static_txtattr_cast<SwTextField const*>(pHt));
                    break;
                }
                else if (pHt->Which() == RES_TXTATR_REFMARK
                         && (nFieldIndex-- == 0))
                    strTypeName = "set reference";
            }
        }
    }
    if (pTextField)
    {
        const SwField* pField = (pTextField->GetFormatField()).GetField();
        if (pField)
        {
            strTypeName = SwFieldType::GetTypeStr(pField->GetTypeId());
            const sal_uInt16 nWhich = pField->GetTyp()->Which();
            OUString sEntry;
            sal_Int32 subType = 0;
            switch (nWhich)
            {
            case RES_DOCSTATFLD:
                subType = static_cast<const SwDocStatField*>(pField)->GetSubType();
                break;
            case RES_GETREFFLD:
                {
                    switch( pField->GetSubType() )
                    {
                    case REF_BOOKMARK:
                        {
                            const SwGetRefField* pRefField = dynamic_cast<const SwGetRefField*>(pField);
                            if ( pRefField && pRefField->IsRefToHeadingCrossRefBookmark() )
                                sEntry = "Headings";
                            else if ( pRefField && pRefField->IsRefToNumItemCrossRefBookmark() )
                                sEntry = "Numbered Paragraphs";
                            else
                                sEntry = "Bookmarks";
                        }
                        break;
                    case REF_FOOTNOTE:
                        sEntry = "Footnotes";
                        break;
                    case REF_ENDNOTE:
                        sEntry = "Endnotes";
                        break;
                    case REF_SETREFATTR:
                        sEntry = "Insert Reference";
                        break;
                    case REF_SEQUENCEFLD:
                        sEntry = static_cast<const SwGetRefField*>(pField)->GetSetRefName();
                        break;
                    }
                    //Get format string
                    strTypeName = sEntry;
                    // <pField->GetFormat() >= 0> is always true as <pField->GetFormat()> is unsigned
//                    if (pField->GetFormat() >= 0)
                    {
                        sEntry = aMgr.GetFormatStr( pField->GetTypeId(), pField->GetFormat() );
                        if (sEntry.getLength() > 0)
                        {
                            strTypeName += "-";
                            strTypeName += sEntry;
                        }
                    }
                }
                break;
            case RES_DATETIMEFLD:
                subType = static_cast<const SwDateTimeField*>(pField)->GetSubType();
                break;
            case RES_JUMPEDITFLD:
                {
                    const sal_uInt16 nFormat= pField->GetFormat();
                    const sal_uInt16 nSize = aMgr.GetFormatCount(pField->GetTypeId(), false);
                    if (nFormat < nSize)
                    {
                        sEntry = aMgr.GetFormatStr(pField->GetTypeId(), nFormat);
                        if (sEntry.getLength() > 0)
                        {
                            strTypeName += "-";
                            strTypeName += sEntry;
                        }
                    }
                }
                break;
            case RES_EXTUSERFLD:
                subType = static_cast<const SwExtUserField*>(pField)->GetSubType();
                break;
            case RES_HIDDENTXTFLD:
            case RES_SETEXPFLD:
                {
                    sEntry = pField->GetTyp()->GetName();
                    if (sEntry.getLength() > 0)
                    {
                        strTypeName += "-";
                        strTypeName += sEntry;
                    }
                }
                break;
            case RES_DOCINFOFLD:
                subType = pField->GetSubType();
                subType &= 0x00ff;
                break;
            case RES_REFPAGESETFLD:
                {
                    const SwRefPageSetField* pRPld = static_cast<const SwRefPageSetField*>(pField);
                    bool bOn = pRPld->IsOn();
                    strTypeName += "-";
                    if (bOn)
                        strTypeName += "on";
                    else
                        strTypeName += "off";
                }
                break;
            case RES_AUTHORFLD:
                {
                    strTypeName += "-";
                    strTypeName += aMgr.GetFormatStr(pField->GetTypeId(), pField->GetFormat() & 0xff);
                }
                break;
            }
            if (subType > 0 || (subType == 0 && (nWhich == RES_DOCINFOFLD || nWhich == RES_EXTUSERFLD || nWhich == RES_DOCSTATFLD)))
            {
                std::vector<OUString> aLst;
                aMgr.GetSubTypes(pField->GetTypeId(), aLst);
                if (static_cast<size_t>(subType) < aLst.size())
                    sEntry = aLst[subType];
                if (sEntry.getLength() > 0)
                {
                    if (nWhich == RES_DOCINFOFLD)
                    {
                        strTypeName = sEntry;
                        sal_uInt32 nSize = aMgr.GetFormatCount(pField->GetTypeId(), false);
                        const sal_uInt16 nExSub = pField->GetSubType() & 0xff00;
                        if (nSize > 0 && nExSub > 0)
                        {
                            //Get extra subtype string
                            strTypeName += "-";
                            sEntry = aMgr.GetFormatStr(pField->GetTypeId(), nExSub/0x0100-1);
                            strTypeName += sEntry;
                        }
                    }
                    else
                    {
                        strTypeName += "-";
                        strTypeName += sEntry;
                    }
                }
            }
        }
    }
    return strTypeName;
}

// #i63870# - re-implement method on behalf of methods
// <_getDefaultAttributesImpl(..)> and <_getRunAttributesImpl(..)>
uno::Sequence<PropertyValue> SwAccessibleParagraph::getCharacterAttributes(
    sal_Int32 nIndex,
    const uno::Sequence< OUString >& aRequestedAttributes )
    throw (lang::IndexOutOfBoundsException,
           uno::RuntimeException,
           std::exception)
{

    SolarMutexGuard aGuard;
    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    const OUString& rText = GetString();

    if( ! IsValidChar( nIndex, rText.getLength()+1 ) )
        throw lang::IndexOutOfBoundsException();

    bool bSupplementalMode = false;
    uno::Sequence< OUString > aNames = aRequestedAttributes;
    if (aNames.getLength() == 0)
    {
        bSupplementalMode = true;
        aNames = getAttributeNames();
    }
    // retrieve default character attributes
    tAccParaPropValMap aDefAttrSeq;
    _getDefaultAttributesImpl( aNames, aDefAttrSeq, true );

    // retrieved run character attributes
    tAccParaPropValMap aRunAttrSeq;
    _getRunAttributesImpl( nIndex, aNames, aRunAttrSeq );

    // merge default and run attributes
    std::vector< PropertyValue > aValues( aDefAttrSeq.size() );
    sal_Int32 i = 0;
    for ( tAccParaPropValMap::const_iterator aDefIter = aDefAttrSeq.begin();
          aDefIter != aDefAttrSeq.end();
          ++aDefIter )
    {
        tAccParaPropValMap::const_iterator aRunIter =
                                        aRunAttrSeq.find( aDefIter->first );
        if ( aRunIter != aRunAttrSeq.end() )
        {
            aValues[i] = aRunIter->second;
        }
        else
        {
            aValues[i] = aDefIter->second;
        }
        ++i;
    }
    if( bSupplementalMode )
    {
        uno::Sequence< OUString > aSupplementalNames = aRequestedAttributes;
        if (aSupplementalNames.getLength() == 0)
            aSupplementalNames = getSupplementalAttributeNames();

        tAccParaPropValMap aSupplementalAttrSeq;
        _getSupplementalAttributesImpl( nIndex, aSupplementalNames, aSupplementalAttrSeq );

        aValues.resize( aValues.size() + aSupplementalAttrSeq.size() );

        for ( tAccParaPropValMap::const_iterator aSupplementalIter = aSupplementalAttrSeq.begin();
            aSupplementalIter != aSupplementalAttrSeq.end();
            ++aSupplementalIter )
        {
            aValues[i] = aSupplementalIter->second;
            ++i;
        }

        _correctValues( nIndex, aValues );

        aValues.resize( aValues.size() + 1 );

        OUString strTypeName = GetFieldTypeNameAtIndex(nIndex);
        if (!strTypeName.isEmpty())
        {
            aValues.resize( aValues.size() + 1 );
            PropertyValue& rValueFT = aValues[aValues.size() - 1];
            rValueFT.Name = "FieldType";
            rValueFT.Value <<= strTypeName.toAsciiLowerCase();
            rValueFT.Handle = -1;
            rValueFT.State = PropertyState_DIRECT_VALUE;
        }

        //sort property values
        // build sorted index array
        sal_Int32 nLength = aValues.size();
        sal_Int32* pIndices = new sal_Int32[nLength];
        for( i = 0; i < nLength; i++ )
            pIndices[i] = i;
        sort( &pIndices[0], &pIndices[nLength], IndexCompare(aValues.data()) );
        // create sorted sequences according to index array
        uno::Sequence<PropertyValue> aNewValues( nLength );
        PropertyValue* pNewValues = aNewValues.getArray();
        for( i = 0; i < nLength; i++ )
        {
            pNewValues[i] = aValues[pIndices[i]];
        }
        delete[] pIndices;
        return aNewValues;
    }

    return comphelper::containerToSequence(aValues);
}

static void SetPutRecursive(SfxItemSet &targetSet, const SfxItemSet &sourceSet)
{
    const SfxItemSet *const pParentSet = sourceSet.GetParent();
    if (pParentSet)
        SetPutRecursive(targetSet, *pParentSet);
    targetSet.Put(sourceSet);
}

// #i63870#
void SwAccessibleParagraph::_getDefaultAttributesImpl(
        const uno::Sequence< OUString >& aRequestedAttributes,
        tAccParaPropValMap& rDefAttrSeq,
        const bool bOnlyCharAttrs )
{
    // retrieve default attributes
    const SwTextNode* pTextNode( GetTextNode() );
    std::unique_ptr<SfxItemSet> pSet;
    if ( !bOnlyCharAttrs )
    {
        pSet.reset( new SfxItemSet( const_cast<SwAttrPool&>(pTextNode->GetDoc()->GetAttrPool()),
                               RES_CHRATR_BEGIN, RES_CHRATR_END - 1,
                               RES_PARATR_BEGIN, RES_PARATR_END - 1,
                               RES_FRMATR_BEGIN, RES_FRMATR_END - 1,
                               0 ) );
    }
    else
    {
        pSet.reset( new SfxItemSet( const_cast<SwAttrPool&>(pTextNode->GetDoc()->GetAttrPool()),
                               RES_CHRATR_BEGIN, RES_CHRATR_END - 1,
                               0 ) );
    }
    // #i82637# - From the perspective of the a11y API the default character
    // attributes are the character attributes, which are set at the paragraph style
    // of the paragraph. The character attributes set at the automatic paragraph
    // style of the paragraph are treated as run attributes.
    //    pTextNode->SwContentNode::GetAttr( *pSet );
    // get default paragraph attributes, if needed, and merge these into <pSet>
    if ( !bOnlyCharAttrs )
    {
        SfxItemSet aParaSet( const_cast<SwAttrPool&>(pTextNode->GetDoc()->GetAttrPool()),
                             RES_PARATR_BEGIN, RES_PARATR_END - 1,
                             RES_FRMATR_BEGIN, RES_FRMATR_END - 1,
                             0 );
        pTextNode->SwContentNode::GetAttr( aParaSet );
        pSet->Put( aParaSet );
    }
    // get default character attributes and merge these into <pSet>
    OSL_ENSURE( pTextNode->GetTextColl(),
            "<SwAccessibleParagraph::_getDefaultAttributesImpl(..)> - missing paragraph style. Serious defect!" );
    if ( pTextNode->GetTextColl() )
    {
        SfxItemSet aCharSet( const_cast<SwAttrPool&>(pTextNode->GetDoc()->GetAttrPool()),
                             RES_CHRATR_BEGIN, RES_CHRATR_END - 1,
                             0 );
        SetPutRecursive( aCharSet, pTextNode->GetTextColl()->GetAttrSet() );
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
        if ( !bOnlyCharAttrs && pTextNode->GetTextColl() )
        {
            if ( aDefAttrSeq.find( UNO_NAME_PARA_STYLE_NAME ) == aDefAttrSeq.end() )
            {
                PropertyValue rPropVal;
                rPropVal.Name = UNO_NAME_PARA_STYLE_NAME;
                uno::Any aVal( uno::makeAny( pTextNode->GetTextColl()->GetName() ) );
                rPropVal.Value = aVal;
                rPropVal.Handle = -1;
                rPropVal.State = beans::PropertyState_DEFAULT_VALUE;

                aDefAttrSeq[rPropVal.Name] = rPropVal;
            }
        }

        // #i73371#
        // resolve value text::WritingMode2::PAGE of property value entry WritingMode
        if ( !bOnlyCharAttrs && GetFrame() )
        {
            tAccParaPropValMap::iterator aIter = aDefAttrSeq.find( UNO_NAME_WRITING_MODE );
            if ( aIter != aDefAttrSeq.end() )
            {
                PropertyValue rPropVal( aIter->second );
                sal_Int16 nVal = rPropVal.Value.get<sal_Int16>();
                if ( nVal == text::WritingMode2::PAGE )
                {
                    const SwFrame* pUpperFrame( GetFrame()->GetUpper() );
                    while ( pUpperFrame )
                    {
                        if ( pUpperFrame->GetType() &
                               ( SwFrameType::Page | SwFrameType::Fly | SwFrameType::Section | SwFrameType::Tab | SwFrameType::Cell ) )
                        {
                            if ( pUpperFrame->IsVertical() )
                            {
                                nVal = text::WritingMode2::TB_RL;
                            }
                            else if ( pUpperFrame->IsRightToLeft() )
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

                        if ( const SwFlyFrame* pFlyFrame = dynamic_cast<const SwFlyFrame*>(pUpperFrame) )
                        {
                            pUpperFrame = pFlyFrame->GetAnchorFrame();
                        }
                        else
                        {
                            pUpperFrame = pUpperFrame->GetUpper();
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
        throw ( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    tAccParaPropValMap aDefAttrSeq;
    _getDefaultAttributesImpl( aRequestedAttributes, aDefAttrSeq );

    // #i92233#
    static const char sMMToPixelRatio[] = "MMToPixelRatio";
    bool bProvideMMToPixelRatio( false );
    {
        if ( aRequestedAttributes.getLength() == 0 )
        {
            bProvideMMToPixelRatio = true;
        }
        else
        {
            const OUString* aRequestedAttrIter =
                  std::find( aRequestedAttributes.begin(), aRequestedAttributes.end(), sMMToPixelRatio );
            if ( aRequestedAttrIter != aRequestedAttributes.end() )
                bProvideMMToPixelRatio = true;
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
    SwPaM* pPaM( nullptr );
    {
        const SwTextNode* pTextNode( GetTextNode() );
        SwPosition* pStartPos = new SwPosition( *pTextNode );
        pStartPos->nContent.Assign( const_cast<SwTextNode*>(pTextNode), nIndex );
        SwPosition* pEndPos = new SwPosition( *pTextNode );
        pEndPos->nContent.Assign( const_cast<SwTextNode*>(pTextNode), nIndex+1 );

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
    //    SwXTextCursor::GetCursorAttr( *pPaM, aSet, sal_True, sal_True );
    // get character attributes from automatic paragraph style and merge these into <aSet>
    {
        const SwTextNode* pTextNode( GetTextNode() );
        if ( pTextNode->HasSwAttrSet() )
        {
            SfxItemSet aAutomaticParaStyleCharAttrs( pPaM->GetDoc()->GetAttrPool(),
                                                     RES_CHRATR_BEGIN, RES_CHRATR_END -1,
                                                     0 );
            aAutomaticParaStyleCharAttrs.Put( *(pTextNode->GetpSwAttrSet()), false );
            aSet.Put( aAutomaticParaStyleCharAttrs );
        }
    }
    // get character attributes at <pPaM> and merge these into <aSet>
    {
        SfxItemSet aCharAttrsAtPaM( pPaM->GetDoc()->GetAttrPool(),
                                    RES_CHRATR_BEGIN, RES_CHRATR_END -1,
                                    0 );
        SwUnoCursorHelper::GetCursorAttr(*pPaM, aCharAttrsAtPaM, true);
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
                const SfxPoolItem* pItem( nullptr );
                // #i82637# - Found character attributes, whose value equals the value of
                // the corresponding default character attributes, are excluded.
                if ( aSet.GetItemState( aPropIt->nWID, true, &pItem ) == SfxItemState::SET )
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
                uno::RuntimeException, std::exception )
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

    return comphelper::mapValuesToSequence( aRunAttrSeq );
}

void SwAccessibleParagraph::_getSupplementalAttributesImpl(
        const sal_Int32,
        const uno::Sequence< OUString >& aRequestedAttributes,
        tAccParaPropValMap& rSupplementalAttrSeq )
{
    const SwTextNode* pTextNode( GetTextNode() );
    std::unique_ptr<SfxItemSet> pSet;
    pSet.reset( new SfxItemSet( const_cast<SwAttrPool&>(pTextNode->GetDoc()->GetAttrPool()),
        RES_PARATR_ADJUST, RES_PARATR_ADJUST,
        RES_PARATR_TABSTOP, RES_PARATR_TABSTOP,
        RES_PARATR_LINESPACING, RES_PARATR_LINESPACING,
        RES_UL_SPACE, RES_UL_SPACE,
        RES_LR_SPACE, RES_LR_SPACE,
        RES_PARATR_NUMRULE, RES_PARATR_NUMRULE,
        RES_PARATR_LIST_BEGIN, RES_PARATR_LIST_END-1,
        0 ) );

    if ( pTextNode->HasBullet() || pTextNode->HasNumber() )
    {
        pSet->Put( pTextNode->GetAttr(RES_PARATR_LIST_LEVEL) );
    }
    pSet->Put( pTextNode->SwContentNode::GetAttr(RES_UL_SPACE) );
    pSet->Put( pTextNode->SwContentNode::GetAttr(RES_LR_SPACE) );
    pSet->Put( pTextNode->SwContentNode::GetAttr(RES_PARATR_ADJUST) );

    tAccParaPropValMap aSupplementalAttrSeq;
    {
        const SfxItemPropertyMapEntry* pPropMap(
                aSwMapProvider.GetPropertyMapEntries( PROPERTY_MAP_ACCESSIBILITY_TEXT_ATTRIBUTE ) );
        while ( !pPropMap->aName.isEmpty() )
        {
            const SfxPoolItem* pItem = pSet->GetItem( pPropMap->nWID );
            if ( pItem )
            {
                uno::Any aVal;
                pItem->QueryValue( aVal, pPropMap->nMemberId );

                PropertyValue rPropVal;
                rPropVal.Name = pPropMap->aName;
                rPropVal.Value = aVal;
                rPropVal.Handle = -1;
                rPropVal.State = beans::PropertyState_DEFAULT_VALUE;

                aSupplementalAttrSeq[rPropVal.Name] = rPropVal;
            }

            ++pPropMap;
        }
    }

    const OUString* pSupplementalAttrs = aRequestedAttributes.getConstArray();
    const sal_Int32 nSupplementalLength = aRequestedAttributes.getLength();

    for( sal_Int32 index = 0; index < nSupplementalLength; ++index )
    {
        tAccParaPropValMap::const_iterator const aIter = aSupplementalAttrSeq.find( pSupplementalAttrs[index] );
        if ( aIter != aSupplementalAttrSeq.end() )
        {
            rSupplementalAttrSeq[ aIter->first ] = aIter->second;
        }
    }
}

void SwAccessibleParagraph::_correctValues( const sal_Int32 nIndex,
                                            std::vector< PropertyValue >& rValues)
{
    PropertyValue ChangeAttr, ChangeAttrColor;

    const SwRangeRedline* pRedline = GetRedlineAtIndex( nIndex );
    if ( pRedline )
    {

        const SwModuleOptions *pOpt = SW_MOD()->GetModuleConfig();
        AuthorCharAttr aChangeAttr;
        if ( pOpt )
        {
            switch( pRedline->GetType())
            {
            case nsRedlineType_t::REDLINE_INSERT:
                aChangeAttr = pOpt->GetInsertAuthorAttr();
                break;
            case nsRedlineType_t::REDLINE_DELETE:
                aChangeAttr = pOpt->GetDeletedAuthorAttr();
                break;
            case nsRedlineType_t::REDLINE_FORMAT:
                aChangeAttr = pOpt->GetFormatAuthorAttr();
                break;
            }
        }
        switch( aChangeAttr.nItemId )
        {
        case SID_ATTR_CHAR_WEIGHT:
            ChangeAttr.Name = UNO_NAME_CHAR_WEIGHT;
            ChangeAttr.Value <<= awt::FontWeight::BOLD;
            break;
        case SID_ATTR_CHAR_POSTURE:
            ChangeAttr.Name = UNO_NAME_CHAR_POSTURE;
            ChangeAttr.Value <<= awt::FontSlant_ITALIC; //char posture
            break;
        case SID_ATTR_CHAR_STRIKEOUT:
            ChangeAttr.Name = UNO_NAME_CHAR_STRIKEOUT;
            ChangeAttr.Value <<= awt::FontStrikeout::SINGLE; //char strikeout
            break;
        case SID_ATTR_CHAR_UNDERLINE:
            ChangeAttr.Name = UNO_NAME_CHAR_UNDERLINE;
            ChangeAttr.Value <<= aChangeAttr.nAttr; //underline line
            break;
        }
        if( aChangeAttr.nColor != COL_NONE_COLOR )
        {
            if( aChangeAttr.nItemId == SID_ATTR_BRUSH )
            {
                ChangeAttrColor.Name = UNO_NAME_CHAR_BACK_COLOR;
                if( aChangeAttr.nColor == COL_TRANSPARENT )//char backcolor
                    ChangeAttrColor.Value <<= COL_BLUE;
                else
                    ChangeAttrColor.Value <<= aChangeAttr.nColor;
            }
            else
            {
                ChangeAttrColor.Name = UNO_NAME_CHAR_COLOR;
                if( aChangeAttr.nColor == COL_TRANSPARENT )//char color
                    ChangeAttrColor.Value <<= COL_BLUE;
                else
                    ChangeAttrColor.Value <<= aChangeAttr.nColor;
            }
        }
    }

    const SwTextNode* pTextNode( GetTextNode() );

    sal_Int32 nValues = rValues.size();
    for (sal_Int32 i = 0;  i < nValues;  ++i)
    {
        PropertyValue& rValue = rValues[i];

        if (rValue.Name == ChangeAttr.Name )
        {
            rValue.Value = ChangeAttr.Value;
            continue;
        }

        if (rValue.Name == ChangeAttrColor.Name )
        {
            rValue.Value = ChangeAttrColor.Value;
            continue;
        }

        //back color
        if (rValue.Name == UNO_NAME_CHAR_BACK_COLOR)
        {
            uno::Any &anyChar = rValue.Value;
            sal_uInt32 crBack = static_cast<sal_uInt32>( reinterpret_cast<sal_uIntPtr>(anyChar.pReserved));
            if (COL_AUTO == crBack)
            {
                uno::Reference<XAccessibleComponent> xComponent(this);
                if (xComponent.is())
                {
                    crBack = (sal_uInt32)xComponent->getBackground();
                }
                rValue.Value <<= crBack;
            }
            continue;
        }

        //char color
        if (rValue.Name == UNO_NAME_CHAR_COLOR)
        {
            if( GetPortionData().IsInGrayPortion( nIndex ) )
                 rValue.Value <<= SwViewOption::GetFieldShadingsColor().GetColor();
            uno::Any &anyChar = rValue.Value;
            sal_uInt32 crChar = static_cast<sal_uInt32>( reinterpret_cast<sal_uIntPtr>(anyChar.pReserved));

            if( COL_AUTO == crChar )
            {
                uno::Reference<XAccessibleComponent> xComponent(this);
                if (xComponent.is())
                {
                    Color cr(xComponent->getBackground());
                    crChar = cr.IsDark() ? COL_WHITE : COL_BLACK;
                    rValue.Value <<= crChar;
                }
            }
            continue;
        }

        // UnderLine
        if (rValue.Name == UNO_NAME_CHAR_UNDERLINE)
        {
            //misspelled word
            SwCursorShell* pCursorShell = GetCursorShell();
            if( pCursorShell != nullptr && pCursorShell->GetViewOptions() && pCursorShell->GetViewOptions()->IsOnlineSpell())
            {
                const SwWrongList* pWrongList = pTextNode->GetWrong();
                if( nullptr != pWrongList )
                {
                    sal_Int32 nBegin = nIndex;
                    sal_Int32 nLen = 1;
                    if( pWrongList->InWrongWord(nBegin,nLen) && !pTextNode->IsSymbol(nBegin) )
                    {
                        rValue.Value <<= (sal_uInt16)LINESTYLE_WAVE;
                    }
                }
            }
            continue;
        }

        // UnderLineColor
        if (rValue.Name == UNO_NAME_CHAR_UNDERLINE_COLOR)
        {
            //misspelled word
            SwCursorShell* pCursorShell = GetCursorShell();
            if( pCursorShell != nullptr && pCursorShell->GetViewOptions() && pCursorShell->GetViewOptions()->IsOnlineSpell())
            {
                const SwWrongList* pWrongList = pTextNode->GetWrong();
                if( nullptr != pWrongList )
                {
                    sal_Int32 nBegin = nIndex;
                    sal_Int32 nLen = 1;
                    if( pWrongList->InWrongWord(nBegin,nLen) && !pTextNode->IsSymbol(nBegin) )
                    {
                        rValue.Value <<= (sal_Int32)0x00ff0000;
                        continue;
                    }
                }
            }

            uno::Any &anyChar = rValue.Value;
            sal_uInt32 crUnderline = static_cast<sal_uInt32>( reinterpret_cast<sal_uIntPtr>(anyChar.pReserved));
            if ( COL_AUTO == crUnderline )
            {
                uno::Reference<XAccessibleComponent> xComponent(this);
                if (xComponent.is())
                {
                    Color cr(xComponent->getBackground());
                    crUnderline = cr.IsDark() ? COL_WHITE : COL_BLACK;
                    rValue.Value <<= crUnderline;
                }
            }

            continue;
        }

        //tab stop
        if (rValue.Name == UNO_NAME_TABSTOPS)
        {
            css::uno::Sequence< css::style::TabStop > tabs = GetCurrentTabStop( nIndex );
            if( !tabs.hasElements() )
            {
                tabs.realloc(1);
                css::style::TabStop ts;
                css::awt::Rectangle rc0 = getCharacterBounds(0);
                css::awt::Rectangle rc1 = getCharacterBounds(nIndex);
                if( rc1.X - rc0.X >= 48 )
                    ts.Position = (rc1.X - rc0.X) - (rc1.X - rc0.X - 48)% 47 + 47;
                else
                    ts.Position = 48;
                ts.DecimalChar = ' ';
                ts.FillChar = ' ';
                ts.Alignment = css::style::TabAlign_LEFT;
                tabs[0] = ts;
            }
            rValue.Value <<= tabs;
            continue;
        }

        //footnote & endnote
        if (rValue.Name == UNO_NAME_CHAR_ESCAPEMENT)
        {
            if ( GetPortionData().IsIndexInFootnode(nIndex) )
            {
                rValue.Value <<= (sal_Int32)101;
            }
            continue;
        }
    }
}

awt::Rectangle SwAccessibleParagraph::getCharacterBounds(
    sal_Int32 nIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    // #i12332# The position after the string needs special treatment.
    // IsValidChar -> IsValidPosition
    if( ! (IsValidPosition( nIndex, GetString().getLength() ) ) )
        throw lang::IndexOutOfBoundsException();

    // #i12332#
    bool bBehindText = false;
    if ( nIndex == GetString().getLength() )
        bBehindText = true;

    // get model position & prepare GetCharRect() arguments
    SwCursorMoveState aMoveState;
    aMoveState.m_bRealHeight = true;
    aMoveState.m_bRealWidth = true;
    SwSpecialPos aSpecialPos;
    SwTextNode* pNode = const_cast<SwTextNode*>( GetTextNode() );

    /**  #i12332# FillSpecialPos does not accept nIndex ==
         GetString().getLength(). In that case nPos is set to the
         length of the string in the core. This way GetCharRect
         returns the rectangle for a cursor at the end of the
         paragraph. */
    const sal_Int32 nPos = bBehindText
        ? pNode->GetText().getLength()
        : GetPortionData().FillSpecialPos(nIndex, aSpecialPos, aMoveState.m_pSpecialPos );

    // call GetCharRect
    SwRect aCoreRect;
    SwIndex aIndex( pNode, nPos );
    SwPosition aPosition( *pNode, aIndex );
    GetFrame()->GetCharRect( aCoreRect, aPosition, &aMoveState );

    // translate core coordinates into accessibility coordinates
    vcl::Window *pWin = GetWindow();
    CHECK_FOR_WINDOW( XAccessibleComponent, pWin );

    Rectangle aScreenRect( GetMap()->CoreToPixel( aCoreRect.SVRect() ));
    SwRect aFrameLogBounds( GetBounds( *(GetMap()) ) ); // twip rel to doc root

    Point aFramePixPos( GetMap()->CoreToPixel( aFrameLogBounds.SVRect() ).TopLeft() );
    aScreenRect.Move( -aFramePixPos.getX(), -aFramePixPos.getY() );

    // convert into AWT Rectangle
    return awt::Rectangle(
        aScreenRect.Left(), aScreenRect.Top(),
        aScreenRect.GetWidth(), aScreenRect.GetHeight() );
}

sal_Int32 SwAccessibleParagraph::getCharacterCount()
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    return GetString().getLength();
}

sal_Int32 SwAccessibleParagraph::getIndexAtPoint( const awt::Point& rPoint )
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    // construct SwPosition (where GetCursorOfst() will put the result into)
    SwTextNode* pNode = const_cast<SwTextNode*>( GetTextNode() );
    SwIndex aIndex( pNode, 0);
    SwPosition aPos( *pNode, aIndex );

    // construct Point (translate into layout coordinates)
    vcl::Window *pWin = GetWindow();
    CHECK_FOR_WINDOW( XAccessibleComponent, pWin );
    Point aPoint( rPoint.X, rPoint.Y );
    SwRect aLogBounds( GetBounds( *(GetMap()), GetFrame() ) ); // twip rel to doc root
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
    OSL_ENSURE( GetFrame() != nullptr, "The text frame has vanished!" );
    OSL_ENSURE( GetFrame()->IsTextFrame(), "The text frame has mutated!" );
    const SwTextFrame* pFrame = static_cast<const SwTextFrame*>( GetFrame() );
    SwCursorMoveState aMoveState;
    aMoveState.m_bPosMatchesBounds = true;
    const bool bSuccess = pFrame->GetCursorOfst( &aPos, aCorePoint, &aMoveState );

    SwIndex aContentIdx = aPos.nContent;
    const sal_Int32 nIndex = aContentIdx.GetIndex();
    if ( nIndex > 0 )
    {
        SwRect aResultRect;
        pFrame->GetCharRect( aResultRect, aPos );
        bool bVert = pFrame->IsVertical();
        bool bR2L = pFrame->IsRightToLeft();

        if ( (!bVert && aResultRect.Pos().getX() > aCorePoint.getX()) ||
             ( bVert && aResultRect.Pos().getY() > aCorePoint.getY()) ||
             ( bR2L  && aResultRect.Right()   < aCorePoint.getX()) )
        {
            SwIndex aIdxPrev( pNode, nIndex - 1);
            SwPosition aPosPrev( *pNode, aIdxPrev );
            SwRect aResultRectPrev;
            pFrame->GetCharRect( aResultRectPrev, aPosPrev );
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
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    sal_Int32 nStart, nEnd;
    bool bSelected = GetSelection( nStart, nEnd );
    return bSelected
           ? GetString().copy( nStart, nEnd - nStart )
           : OUString();
}

sal_Int32 SwAccessibleParagraph::getSelectionStart()
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    sal_Int32 nStart, nEnd;
    GetSelection( nStart, nEnd );
    return nStart;
}

sal_Int32 SwAccessibleParagraph::getSelectionEnd()
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    sal_Int32 nStart, nEnd;
    GetSelection( nStart, nEnd );
    return nEnd;
}

sal_Bool SwAccessibleParagraph::setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    // parameter checking
    sal_Int32 nLength = GetString().getLength();
    if ( ! IsValidRange( nStartIndex, nEndIndex, nLength ) )
    {
        throw lang::IndexOutOfBoundsException();
    }

    bool bRet = false;

    // get cursor shell
    SwCursorShell* pCursorShell = GetCursorShell();
    if( pCursorShell != nullptr )
    {
        // create pam for selection
        SwTextNode* pNode = const_cast<SwTextNode*>( GetTextNode() );
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
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    return GetString();
}

OUString SwAccessibleParagraph::getTextRange(
    sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
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

/*accessibility::*/TextSegment SwAccessibleParagraph::getTextAtIndex( sal_Int32 nIndex, sal_Int16 nTextType ) throw (lang::IndexOutOfBoundsException, lang::IllegalArgumentException, uno::RuntimeException, std::exception)
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
    bool bWord = GetTextBoundary( aBound, rText, nIndex, nTextType );

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

/*accessibility::*/TextSegment SwAccessibleParagraph::getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 nTextType ) throw (lang::IndexOutOfBoundsException, lang::IllegalArgumentException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    const OUString rText = GetString();

    /*accessibility::*/TextSegment aResult;
    aResult.SegmentStart = -1;
    aResult.SegmentEnd = -1;
    //If nIndex = 0, then nobefore text so return -1 directly.
    if( nIndex == 0 )
            return aResult;
    //Tab will be return when call WORDTYPE

    // get starting pos
    i18n::Boundary aBound;
    if (nIndex ==  rText.getLength())
        aBound.startPos = aBound.endPos = nIndex;
    else
    {
        bool bTmp = GetTextBoundary( aBound, rText, nIndex, nTextType );

        if ( ! bTmp )
            aBound.startPos = aBound.endPos = nIndex;
    }

    // now skip to previous word
    if (nTextType==2 || nTextType == 3)
    {
        i18n::Boundary preBound = aBound;
        while(preBound.startPos==aBound.startPos && nIndex > 0)
        {
            nIndex = min( nIndex, preBound.startPos ) - 1;
            if( nIndex < 0 ) break;
            GetTextBoundary( preBound, rText, nIndex, nTextType );
        }
        //if (nIndex>0)
        if (nIndex>=0)
        //Tab will be return when call WORDTYPE
        {
            aResult.SegmentText = rText.copy( preBound.startPos, preBound.endPos - preBound.startPos );
            aResult.SegmentStart = preBound.startPos;
            aResult.SegmentEnd = preBound.endPos;
        }
    }
    else
    {
        bool bWord = false;
        while( !bWord )
        {
            nIndex = min( nIndex, aBound.startPos ) - 1;
            if( nIndex >= 0 )
            {
                bWord = GetTextBoundary( aBound, rText, nIndex, nTextType );
            }
            else
                break;  // exit if beginning of string is reached
        }

        if (bWord && nIndex<rText.getLength())
        {
            aResult.SegmentText = rText.copy( aBound.startPos, aBound.endPos - aBound.startPos );
            aResult.SegmentStart = aBound.startPos;
            aResult.SegmentEnd = aBound.endPos;
        }
    }
    return aResult;
}

/*accessibility::*/TextSegment SwAccessibleParagraph::getTextBehindIndex( sal_Int32 nIndex, sal_Int16 nTextType ) throw (lang::IndexOutOfBoundsException, lang::IllegalArgumentException, uno::RuntimeException, std::exception)
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
    bool bWord = false;
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

/*
        sal_Bool bWord = sal_False;
    bWord = GetTextBoundary( aBound, rText, nIndex, nTextType );

        if (nTextType==2)
        {
                Boundary nexBound=aBound;

        // real current word
        if( nIndex <= aBound.endPos && nIndex >= aBound.startPos )
        {
            while(nexBound.endPos==aBound.endPos&&nIndex<rText.getLength())
            {
                // nIndex = max( (sal_Int32)(nIndex), nexBound.endPos) + 1;
                nIndex = max( (sal_Int32)(nIndex), nexBound.endPos) ;
                const sal_Unicode* pStr = rText.getStr();
                if (pStr)
                {
                    if( pStr[nIndex] == sal_Unicode(' ') )
                        nIndex++;
                }
                if( nIndex < rText.getLength() )
                {
                    bWord = GetTextBoundary( nexBound, rText, nIndex, nTextType );
                }
            }
        }

        if (bWord && nIndex<rText.getLength())
        {
            aResult.SegmentText = rText.copy( nexBound.startPos, nexBound.endPos - nexBound.startPos );
            aResult.SegmentStart = nexBound.startPos;
            aResult.SegmentEnd = nexBound.endPos;
        }

    }
    else
    {
        bWord = sal_False;
        while( !bWord )
        {
            nIndex = max( (sal_Int32)(nIndex+1), aBound.endPos );
            if( nIndex < rText.getLength() )
            {
                bWord = GetTextBoundary( aBound, rText, nIndex, nTextType );
            }
            else
                break;  // exit if end of string is reached
        }
        if (bWord && nIndex<rText.getLength())
        {
            aResult.SegmentText = rText.copy( aBound.startPos, aBound.endPos - aBound.startPos );
            aResult.SegmentStart = aBound.startPos;
            aResult.SegmentEnd = aBound.endPos;
        }
    }
*/
    return aResult;
}

sal_Bool SwAccessibleParagraph::copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );
    SolarMutexGuard aGuard;

    // select and copy (through dispatch mechanism)
    setSelection( nStartIndex, nEndIndex );
    ExecuteAtViewShell( SID_COPY );
    return true;
}

// XAccesibleEditableText

sal_Bool SwAccessibleParagraph::cutText( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    CHECK_FOR_DEFUNC( XAccessibleEditableText );
    SolarMutexGuard aGuard;

    if( !IsEditableState() )
        return false;

    // select and cut (through dispatch mechanism)
    setSelection( nStartIndex, nEndIndex );
    ExecuteAtViewShell( SID_CUT );
    return true;
}

sal_Bool SwAccessibleParagraph::pasteText( sal_Int32 nIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    CHECK_FOR_DEFUNC( XAccessibleEditableText );
    SolarMutexGuard aGuard;

    if( !IsEditableState() )
        return false;

    // select and paste (through dispatch mechanism)
    setSelection( nIndex, nIndex );
    ExecuteAtViewShell( SID_PASTE );
    return true;
}

sal_Bool SwAccessibleParagraph::deleteText( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    return replaceText( nStartIndex, nEndIndex, OUString() );
}

sal_Bool SwAccessibleParagraph::insertText( const OUString& sText, sal_Int32 nIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    return replaceText( nIndex, nIndex, sText );
}

sal_Bool SwAccessibleParagraph::replaceText(
    sal_Int32 nStartIndex, sal_Int32 nEndIndex,
    const OUString& sReplacement )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleEditableText );

    const OUString& rText = GetString();

    if( IsValidRange( nStartIndex, nEndIndex, rText.getLength() ) )
    {
        if( !IsEditableState() )
            return false;

        SwTextNode* pNode = const_cast<SwTextNode*>( GetTextNode() );

        // translate positions
        sal_Int32 nStart;
        sal_Int32 nEnd;
        bool bSuccess = GetPortionData().GetEditableRange(
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

sal_Bool SwAccessibleParagraph::setAttributes(
    sal_Int32 nStartIndex,
    sal_Int32 nEndIndex,
    const uno::Sequence<PropertyValue>& rAttributeSet )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    CHECK_FOR_DEFUNC( XAccessibleEditableText );

    const OUString& rText = GetString();

    if( ! IsValidRange( nStartIndex, nEndIndex, rText.getLength() ) )
        throw lang::IndexOutOfBoundsException();

    if( !IsEditableState() )
        return false;

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

    // create sorted sequences according to index array
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
    bool bRet = true;
    try
    {
        xPortion->setPropertyValues( aNames, aValues );
    }
    catch (const UnknownPropertyException&)
    {
        // error handling through return code!
        bRet = false;
    }

    return bRet;
}

sal_Bool SwAccessibleParagraph::setText( const OUString& sText )
    throw (uno::RuntimeException, std::exception)
{
    return replaceText(0, GetString().getLength(), sText);
}

// XAccessibleSelection

void SwAccessibleParagraph::selectAccessibleChild(
    sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException,
            uno::RuntimeException, std::exception )
{
    CHECK_FOR_DEFUNC( XAccessibleSelection );

    aSelectionHelper.selectAccessibleChild(nChildIndex);
}

sal_Bool SwAccessibleParagraph::isAccessibleChildSelected(
    sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException,
            uno::RuntimeException, std::exception )
{
    CHECK_FOR_DEFUNC( XAccessibleSelection );

    return aSelectionHelper.isAccessibleChildSelected(nChildIndex);
}

void SwAccessibleParagraph::clearAccessibleSelection(  )
    throw ( uno::RuntimeException, std::exception )
{
    CHECK_FOR_DEFUNC( XAccessibleSelection );
}

void SwAccessibleParagraph::selectAllAccessibleChildren(  )
    throw ( uno::RuntimeException, std::exception )
{
    CHECK_FOR_DEFUNC( XAccessibleSelection );

    aSelectionHelper.selectAllAccessibleChildren();
}

sal_Int32 SwAccessibleParagraph::getSelectedAccessibleChildCount(  )
    throw ( uno::RuntimeException, std::exception )
{
    CHECK_FOR_DEFUNC( XAccessibleSelection );

    return aSelectionHelper.getSelectedAccessibleChildCount();
}

uno::Reference<XAccessible> SwAccessibleParagraph::getSelectedAccessibleChild(
    sal_Int32 nSelectedChildIndex )
    throw ( lang::IndexOutOfBoundsException,
            uno::RuntimeException, std::exception)
{
    CHECK_FOR_DEFUNC( XAccessibleSelection );

    return aSelectionHelper.getSelectedAccessibleChild(nSelectedChildIndex);
}

// index has to be treated as global child index.
void SwAccessibleParagraph::deselectAccessibleChild(
    sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException,
            uno::RuntimeException, std::exception )
{
    CHECK_FOR_DEFUNC( XAccessibleSelection );

    aSelectionHelper.deselectAccessibleChild( nChildIndex );
}

// XAccessibleHypertext

class SwHyperlinkIter_Impl
{
    const SwpHints *pHints;
    sal_Int32 nStt;
    sal_Int32 nEnd;
    size_t nPos;

public:
    explicit SwHyperlinkIter_Impl( const SwTextFrame *pTextFrame );
    const SwTextAttr *next();
    size_t getCurrHintPos() const { return nPos-1; }

    sal_Int32 startIdx() const { return nStt; }
    sal_Int32 endIdx() const { return nEnd; }
};

SwHyperlinkIter_Impl::SwHyperlinkIter_Impl( const SwTextFrame *pTextFrame ) :
    pHints( pTextFrame->GetTextNode()->GetpSwpHints() ),
    nStt( pTextFrame->GetOfst() ),
    nPos( 0 )
{
    const SwTextFrame *pFollFrame = pTextFrame->GetFollow();
    nEnd = pFollFrame ? pFollFrame->GetOfst() : pTextFrame->GetTextNode()->Len();
}

const SwTextAttr *SwHyperlinkIter_Impl::next()
{
    const SwTextAttr *pAttr = nullptr;
    if( pHints )
    {
        while( !pAttr && nPos < pHints->Count() )
        {
            const SwTextAttr *pHt = pHints->Get(nPos);
            if( RES_TXTATR_INETFMT == pHt->Which() )
            {
                const sal_Int32 nHtStt = pHt->GetStart();
                const sal_Int32 nHtEnd = *pHt->GetAnyEnd();
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
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleHypertext );

    sal_Int32 nCount = 0;
    // #i77108# - provide hyperlinks also in editable documents.

    const SwTextFrame *pTextFrame = static_cast<const SwTextFrame*>( GetFrame() );
    SwHyperlinkIter_Impl aIter( pTextFrame );
    while( aIter.next() )
        nCount++;

    return nCount;
}

uno::Reference< XAccessibleHyperlink > SAL_CALL
    SwAccessibleParagraph::getHyperLink( sal_Int32 nLinkIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    CHECK_FOR_DEFUNC( XAccessibleHypertext );

    uno::Reference< XAccessibleHyperlink > xRet;

    const SwTextFrame *pTextFrame = static_cast<const SwTextFrame*>( GetFrame() );
    SwHyperlinkIter_Impl aHIter( pTextFrame );
    sal_Int32 nTIndex = -1;
    SwTOXSortTabBase* pTBase = GetTOXSortTabBase();
    SwTextAttr* pHt = const_cast<SwTextAttr*>(aHIter.next());
    while( (nLinkIndex < getHyperLinkCount()) && nTIndex < nLinkIndex)
    {
        sal_Int32 nHStt = -1;
        bool bH = false;

        if( pHt )
            nHStt = pHt->GetStart();
        bool bTOC = false;
        // Inside TOC & get the first link
        if( pTBase && nTIndex == -1 )
        {
            nTIndex++;
            bTOC = true;
        }
        else if( nHStt >= 0 )
        {
              // only hyperlink available
            nTIndex++;
            bH = true;
        }

        if( nTIndex == nLinkIndex )
        {   // found
            if( bH )
            {   // it's a hyperlink
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
                        {
                            const sal_Int32 nTmpHStt= GetPortionData().GetAccessiblePosition(
                                max( aHIter.startIdx(), pHt->GetStart() ) );
                            const sal_Int32 nTmpHEnd= GetPortionData().GetAccessiblePosition(
                                min( aHIter.endIdx(), *pHt->GetAnyEnd() ) );
                            xRet = new SwAccessibleHyperlink( aHIter.getCurrHintPos(),
                                this, nTmpHStt, nTmpHEnd );
                        }
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
            break;
        }

        // iterate next
        if( bH )
            // iterate next hyperlink
            pHt = const_cast<SwTextAttr*>(aHIter.next());
        else if(bTOC)
            continue;
        else
            // no candidate, exit
            break;
    }
    if( !xRet.is() )
        throw lang::IndexOutOfBoundsException();

    return xRet;
}

sal_Int32 SAL_CALL SwAccessibleParagraph::getHyperLinkIndex( sal_Int32 nCharIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
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
        const SwTextFrame *pTextFrame = static_cast<const SwTextFrame*>( GetFrame() );
        SwHyperlinkIter_Impl aHIter( pTextFrame );

        const sal_Int32 nIdx = GetPortionData().GetModelPosition( nCharIndex );
        sal_Int32 nPos = 0;
        const SwTextAttr *pHt = aHIter.next();
        while( pHt && !(nIdx >= pHt->GetStart() && nIdx < *pHt->GetAnyEnd()) )
        {
            pHt = aHIter.next();
            nPos++;
        }

        if( pHt )
            nRet = nPos;
    }

    if (nRet == -1)
        throw lang::IndexOutOfBoundsException();
    else
        return nRet;
    //return nRet;
}

// #i71360#, #i108125# - adjustments for change tracking text markup
sal_Int32 SAL_CALL SwAccessibleParagraph::getTextMarkupCount( sal_Int32 nTextMarkupType )
                                        throw (lang::IllegalArgumentException,
                                               uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

    std::unique_ptr<SwTextMarkupHelper> pTextMarkupHelper;
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
            pTextMarkupHelper.reset( new SwTextMarkupHelper( GetPortionData(), *GetTextNode() ) );
        }
    }

    return pTextMarkupHelper->getTextMarkupCount( nTextMarkupType );
}

//MSAA Extension Implementation in app  module
sal_Bool SAL_CALL SwAccessibleParagraph::scrollToPosition( const css::awt::Point&, sal_Bool )
    throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
{
    return false;
}

sal_Int32 SAL_CALL SwAccessibleParagraph::getSelectedPortionCount(  )
    throw (css::uno::RuntimeException,
           std::exception)
{
    SolarMutexGuard g;

    sal_Int32 nSeleted = 0;
    SwPaM* pCursor = GetCursor( true );
    if( pCursor != nullptr )
    {
        // get SwPosition for my node
        const SwTextNode* pNode = GetTextNode();
        sal_uLong nHere = pNode->GetIndex();

        // iterate over ring
        for(SwPaM& rTmpCursor : pCursor->GetRingContainer())
        {
            // ignore, if no mark
            if( rTmpCursor.HasMark() )
            {
                // check whether nHere is 'inside' pCursor
                SwPosition* pStart = rTmpCursor.Start();
                sal_uLong nStartIndex = pStart->nNode.GetIndex();
                SwPosition* pEnd = rTmpCursor.End();
                sal_uLong nEndIndex = pEnd->nNode.GetIndex();
                if( ( nHere >= nStartIndex ) &&
                    ( nHere <= nEndIndex )      )
                {
                    nSeleted++;
                }
                // else: this PaM doesn't point to this paragraph
            }
            // else: this PaM is collapsed and doesn't select anything
        }
    }
    return nSeleted;

}

sal_Int32 SAL_CALL SwAccessibleParagraph::getSeletedPositionStart( sal_Int32 nSelectedPortionIndex )
    throw (css::lang::IndexOutOfBoundsException,
           css::uno::RuntimeException,
           std::exception)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    sal_Int32 nStart, nEnd;
    /*sal_Bool bSelected = */GetSelectionAtIndex(nSelectedPortionIndex, nStart, nEnd );
    return nStart;
}

sal_Int32 SAL_CALL SwAccessibleParagraph::getSeletedPositionEnd( sal_Int32 nSelectedPortionIndex )
    throw (css::lang::IndexOutOfBoundsException,
           css::uno::RuntimeException,
           std::exception)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    sal_Int32 nStart, nEnd;
    /*sal_Bool bSelected = */GetSelectionAtIndex(nSelectedPortionIndex, nStart, nEnd );
    return nEnd;
}

sal_Bool SAL_CALL SwAccessibleParagraph::removeSelection( sal_Int32 selectionIndex )
    throw (css::lang::IndexOutOfBoundsException,
           css::uno::RuntimeException,
           std::exception)
{
    SolarMutexGuard g;

    if(selectionIndex < 0) return false;

    sal_Int32 nSelected = selectionIndex;

    // get the selection, and test whether it affects our text node
    SwPaM* pCursor = GetCursor( true );

    if( pCursor != nullptr )
    {
        bool bRet = false;

        // get SwPosition for my node
        const SwTextNode* pNode = GetTextNode();
        sal_uLong nHere = pNode->GetIndex();

        // iterate over ring
        SwPaM* pRingStart = pCursor;
        do
        {
            // ignore, if no mark
            if( pCursor->HasMark() )
            {
                // check whether nHere is 'inside' pCursor
                SwPosition* pStart = pCursor->Start();
                sal_uLong nStartIndex = pStart->nNode.GetIndex();
                SwPosition* pEnd = pCursor->End();
                sal_uLong nEndIndex = pEnd->nNode.GetIndex();
                if( ( nHere >= nStartIndex ) &&
                    ( nHere <= nEndIndex )      )
                {
                    if( nSelected == 0 )
                    {
                        pCursor->MoveTo(nullptr);
                        delete pCursor;
                        bRet = true;
                    }
                    else
                    {
                        nSelected--;
                    }
                }
            }
            // else: this PaM is collapsed and doesn't select anything
            if(!bRet)
                pCursor = pCursor->GetNext();
        }
        while( !bRet && (pCursor != pRingStart) );
    }
    return true;
}

sal_Int32 SAL_CALL SwAccessibleParagraph::addSelection( sal_Int32, sal_Int32 startOffset, sal_Int32 endOffset)
    throw (css::lang::IndexOutOfBoundsException,
           css::uno::RuntimeException,
           std::exception)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    // parameter checking
    sal_Int32 nLength = GetString().getLength();
    if ( ! IsValidRange( startOffset, endOffset, nLength ) )
    {
        throw lang::IndexOutOfBoundsException();
    }

    sal_Int32 nSelectedCount = getSelectedPortionCount();
    for ( sal_Int32 i = nSelectedCount ; i >= 0 ; i--)
    {
        sal_Int32 nStart, nEnd;
        bool bSelected = GetSelectionAtIndex(i, nStart, nEnd );
        if(bSelected)
        {
            if(nStart <= nEnd )
            {
                if (( startOffset>=nStart && startOffset <=nEnd ) ||     //startOffset in a selection
                       ( endOffset>=nStart && endOffset <=nEnd )     ||  //endOffset in a selection
                    ( startOffset <= nStart && endOffset >=nEnd)  ||       //start and  end include the old selection
                    ( startOffset >= nStart && endOffset <=nEnd) )
                {
                    removeSelection(i);
                }

            }
            else
            {
                if (( startOffset>=nEnd && startOffset <=nStart ) ||     //startOffset in a selection
                       ( endOffset>=nEnd && endOffset <=nStart )     || //endOffset in a selection
                    ( startOffset <= nStart && endOffset >=nEnd)  ||       //start and  end include the old selection
                    ( startOffset >= nStart && endOffset <=nEnd) )

                {
                    removeSelection(i);
                }
            }
        }

    }

    // get cursor shell
    SwCursorShell* pCursorShell = GetCursorShell();
    if( pCursorShell != nullptr )
    {
        // create pam for selection
        pCursorShell->StartAction();
        SwPaM* aPaM = pCursorShell->CreateCursor();
        aPaM->SetMark();
        aPaM->GetPoint()->nContent = GetPortionData().GetModelPosition(startOffset);
        aPaM->GetMark()->nContent =  GetPortionData().GetModelPosition(endOffset);
        pCursorShell->EndAction();
    }

    return 0;
}

/*accessibility::*/TextSegment SAL_CALL
        SwAccessibleParagraph::getTextMarkup( sal_Int32 nTextMarkupIndex,
                                              sal_Int32 nTextMarkupType )
                                        throw (lang::IndexOutOfBoundsException,
                                               lang::IllegalArgumentException,
                                               uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

    std::unique_ptr<SwTextMarkupHelper> pTextMarkupHelper;
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
            pTextMarkupHelper.reset( new SwTextMarkupHelper( GetPortionData(), *GetTextNode() ) );
        }
    }

    return pTextMarkupHelper->getTextMarkup( nTextMarkupIndex, nTextMarkupType );
}

uno::Sequence< /*accessibility::*/TextSegment > SAL_CALL
        SwAccessibleParagraph::getTextMarkupAtIndex( sal_Int32 nCharIndex,
                                                     sal_Int32 nTextMarkupType )
                                        throw (lang::IndexOutOfBoundsException,
                                               lang::IllegalArgumentException,
                                               uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

    // parameter checking
    const sal_Int32 nLength = GetString().getLength();
    if ( ! IsValidPosition( nCharIndex, nLength ) )
    {
        throw lang::IndexOutOfBoundsException();
    }

    std::unique_ptr<SwTextMarkupHelper> pTextMarkupHelper;
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
            pTextMarkupHelper.reset( new SwTextMarkupHelper( GetPortionData(), *GetTextNode() ) );
        }
    }

    return pTextMarkupHelper->getTextMarkupAtIndex( nCharIndex, nTextMarkupType );
}

// #i89175#
sal_Int32 SAL_CALL SwAccessibleParagraph::getLineNumberAtIndex( sal_Int32 nIndex )
                                        throw (lang::IndexOutOfBoundsException,
                                               uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

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
                                               uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

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
                                        throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

    const sal_Int32 nLineNoOfCaret = getNumberOfLineWithCaret();

    if ( nLineNoOfCaret >= 0 &&
         nLineNoOfCaret < GetPortionData().GetLineCount() )
    {
        return getTextAtLineNumber( nLineNoOfCaret );
    }

    return /*accessibility::*/TextSegment();
}

sal_Int32 SAL_CALL SwAccessibleParagraph::getNumberOfLineWithCaret()
                                        throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

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
            SwCursorShell* pCursorShell = SwAccessibleParagraph::GetCursorShell();
            if ( pCursorShell != nullptr )
            {
                const awt::Rectangle aCharRect = getCharacterBounds( nCaretPos );

                const SwRect& aCursorCoreRect = pCursorShell->GetCharRect();
                // translate core coordinates into accessibility coordinates
                vcl::Window *pWin = GetWindow();
                CHECK_FOR_WINDOW( XAccessibleComponent, pWin );

                Rectangle aScreenRect( GetMap()->CoreToPixel( aCursorCoreRect.SVRect() ));

                SwRect aFrameLogBounds( GetBounds( *(GetMap()) ) ); // twip rel to doc root
                Point aFramePixPos( GetMap()->CoreToPixel( aFrameLogBounds.SVRect() ).TopLeft() );
                aScreenRect.Move( -aFramePixPos.getX(), -aFramePixPos.getY() );

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

bool SwAccessibleParagraph::GetSelectionAtIndex(
    sal_Int32& nIndex, sal_Int32& nStart, sal_Int32& nEnd)
{
        if(nIndex < 0) return false;

    bool bRet = false;
    nStart = -1;
    nEnd = -1;
    sal_Int32 nSelected = nIndex;

    // get the selection, and test whether it affects our text node
    SwPaM* pCursor = GetCursor( true );
    if( pCursor != nullptr )
    {
        // get SwPosition for my node
        const SwTextNode* pNode = GetTextNode();
        sal_uLong nHere = pNode->GetIndex();

        // iterate over ring
        for(SwPaM& rTmpCursor : pCursor->GetRingContainer())
        {
            // ignore, if no mark
            if( rTmpCursor.HasMark() )
            {
                // check whether nHere is 'inside' pCursor
                SwPosition* pStart = rTmpCursor.Start();
                sal_uLong nStartIndex = pStart->nNode.GetIndex();
                SwPosition* pEnd = rTmpCursor.End();
                sal_uLong nEndIndex = pEnd->nNode.GetIndex();
                if( ( nHere >= nStartIndex ) &&
                    ( nHere <= nEndIndex )      )
                {
                    if( nSelected == 0 )
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
                            assert(nHere == nStartIndex);

                            // selection starts in this node:
                            // then check whether it's before or inside our part of
                            // the paragraph, and if so, get the proper position
                            const sal_Int32 nCoreStart = pStart->nContent.GetIndex();
                            if( nCoreStart <
                                GetPortionData().GetFirstValidCorePosition() )
                            {
                                nLocalStart = 0;
                            }
                            else if( nCoreStart <=
                                     GetPortionData().GetLastValidCorePosition() )
                            {
                                SAL_WARN_IF(
                                    GetPortionData().IsValidCorePosition(
                                                                  nCoreStart),
                                    "sw.a11y",
                                    "problem determining valid core position");

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
                            assert(nHere == nEndIndex);

                            // selection ends in this node: then select everything
                            // before our part of the node
                            const sal_Int32 nCoreEnd = pEnd->nContent.GetIndex();
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
                                SAL_WARN_IF(
                                    GetPortionData().IsValidCorePosition(
                                                                  nCoreEnd),
                                    "sw.a11y",
                                    "problem determining valid core position");

                                nLocalEnd = GetPortionData().GetAccessiblePosition(
                                                                       nCoreEnd );
                            }
                        }

                        if( ( nLocalStart != -1 ) && ( nLocalEnd != -1 ) )
                        {
                            nStart = nLocalStart;
                            nEnd = nLocalEnd;
                            bRet = true;
                        }
                    } // if hit the index
                    else
                    {
                        nSelected--;
                    }
                }
                // else: this PaM doesn't point to this paragraph
            }
            // else: this PaM is collapsed and doesn't select anything
            if(bRet)
                break;
        }
    }
    // else: nocursor -> no selection

    if( bRet )
    {
        sal_Int32 nCaretPos = GetCaretPos();
        if( nStart == nCaretPos )
        {
            sal_Int32 tmp = nStart;
            nStart = nEnd;
            nEnd = tmp;
        }
    }
    return bRet;
}

sal_Int16 SAL_CALL SwAccessibleParagraph::getAccessibleRole() throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

    //Get the real heading level, Heading1 ~ Heading10
    if (nHeadingLevel > 0)
    {
        return AccessibleRole::HEADING;
    }
    else
    {
        return AccessibleRole::PARAGRAPH;
    }
}

//Get the real heading level, Heading1 ~ Heading10
sal_Int32 SwAccessibleParagraph::GetRealHeadingLevel()
{
    uno::Reference< css::beans::XPropertySet > xPortion = CreateUnoPortion( 0, 0 );
    uno::Any styleAny = xPortion->getPropertyValue( "ParaStyleName" );
    OUString sValue;
    if (styleAny >>= sValue)
    {
        sal_Int32 length = sValue.getLength();
        if (length == 9 || length == 10)
        {
            OUString headStr = sValue.copy(0, 7);
            if (headStr == "Heading")
            {
                OUString intStr = sValue.copy(8);
                sal_Int32 headingLevel = intStr.toInt32();
                return headingLevel;
            }
        }
    }
    return -1;
}

uno::Any SAL_CALL SwAccessibleParagraph::getExtendedAttributes()
        throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

    uno::Any Ret;
    OUString strHeading("heading-level:");
    if( nHeadingLevel >= 0 )
        strHeading += OUString::number(nHeadingLevel);
    strHeading += ";";

    strHeading += strHeading.copy(8); // tdf#84102: expose the same attribute with the name "level"

    Ret <<= strHeading;

    return Ret;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
