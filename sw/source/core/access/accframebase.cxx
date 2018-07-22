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

#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <unotools/accessiblestatesethelper.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <frmfmt.hxx>
#include <ndnotxt.hxx>
#include <flyfrm.hxx>
#include <cntfrm.hxx>
#include <fmtcntnt.hxx>
#include <ndindex.hxx>
#include <fesh.hxx>
#include <hints.hxx>
#include <accmap.hxx>
#include "accframebase.hxx"

#include <crsrsh.hxx>
#include <txtfrm.hxx>
#include <notxtfrm.hxx>
#include <ndtxt.hxx>
#include <dcontact.hxx>
#include <fmtanchr.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

bool SwAccessibleFrameBase::IsSelected()
{
    bool bRet = false;

    assert(GetMap());
    const SwViewShell *pVSh = GetMap()->GetShell();
    assert(pVSh);
    if( auto pFESh = dynamic_cast<const SwFEShell*>(pVSh) )
    {
        const SwFrame *pFlyFrame = pFESh->GetSelectedFlyFrame();
        if( pFlyFrame == GetFrame() )
            bRet = true;
    }

    return bRet;
}

void SwAccessibleFrameBase::GetStates(
        ::utl::AccessibleStateSetHelper& rStateSet )
{
    SwAccessibleContext::GetStates( rStateSet );

    const SwViewShell *pVSh = GetMap()->GetShell();
    assert(pVSh);
    bool bSelectable =  dynamic_cast<const SwFEShell*>( pVSh) !=  nullptr;

    // SELECTABLE
    if( bSelectable )
        rStateSet.AddState( AccessibleStateType::SELECTABLE );

    // FOCUSABLE
    if( bSelectable )
        rStateSet.AddState( AccessibleStateType::FOCUSABLE );

    // SELECTED and FOCUSED
    if( IsSelected() )
    {
        rStateSet.AddState( AccessibleStateType::SELECTED );
        assert(m_bIsSelected && "bSelected out of sync");
        ::rtl::Reference < SwAccessibleContext > xThis( this );
        GetMap()->SetCursorContext( xThis );

        vcl::Window *pWin = GetWindow();
        if( pWin && pWin->HasFocus() )
            rStateSet.AddState( AccessibleStateType::FOCUSED );
    }
    if( GetSelectedState() )
        rStateSet.AddState( AccessibleStateType::SELECTED );
}

SwNodeType SwAccessibleFrameBase::GetNodeType( const SwFlyFrame *pFlyFrame )
{
    SwNodeType nType = SwNodeType::Text;
    if( pFlyFrame->Lower() )
    {
         if( pFlyFrame->Lower()->IsNoTextFrame() )
        {
            const SwNoTextFrame *const pContentFrame =
                static_cast<const SwNoTextFrame *>(pFlyFrame->Lower());
            nType = pContentFrame->GetNode()->GetNodeType();
        }
    }
    else
    {
        const SwFrameFormat *pFrameFormat = pFlyFrame->GetFormat();
        const SwFormatContent& rContent = pFrameFormat->GetContent();
        const SwNodeIndex *pNdIdx = rContent.GetContentIdx();
        if( pNdIdx )
        {
            const SwContentNode *pCNd =
                (pNdIdx->GetNodes())[pNdIdx->GetIndex()+1]->GetContentNode();
            if( pCNd )
                nType = pCNd->GetNodeType();
        }
    }

    return nType;
}

SwAccessibleFrameBase::SwAccessibleFrameBase(
        std::shared_ptr<SwAccessibleMap> const& pInitMap,
        sal_Int16 nInitRole,
        const SwFlyFrame* pFlyFrame  ) :
    SwAccessibleContext( pInitMap, nInitRole, pFlyFrame ),
    m_bIsSelected( false )
{
    const SwFrameFormat *pFrameFormat = pFlyFrame->GetFormat();
    const_cast< SwFrameFormat * >( pFrameFormat )->Add( this );

    SetName( pFrameFormat->GetName() );

    m_bIsSelected = IsSelected();
}

void SwAccessibleFrameBase::InvalidateCursorPos_()
{
    bool bNewSelected = IsSelected();
    bool bOldSelected;

    {
        osl::MutexGuard aGuard( m_Mutex );
        bOldSelected = m_bIsSelected;
        m_bIsSelected = bNewSelected;
    }

    if( bNewSelected )
    {
        // remember that object as the one that has the caret. This is
        // necessary to notify that object if the cursor leaves it.
        ::rtl::Reference < SwAccessibleContext > xThis( this );
        GetMap()->SetCursorContext( xThis );
    }

    if( bOldSelected != bNewSelected )
    {
        vcl::Window *pWin = GetWindow();
        if( pWin && pWin->HasFocus() && bNewSelected )
            FireStateChangedEvent( AccessibleStateType::FOCUSED, bNewSelected );
        if( pWin && pWin->HasFocus() && !bNewSelected )
            FireStateChangedEvent( AccessibleStateType::FOCUSED, bNewSelected );
        if(bNewSelected)
        {
            uno::Reference< XAccessible > xParent( GetWeakParent() );
            if( xParent.is() )
            {
                SwAccessibleContext *pAcc =
                    static_cast <SwAccessibleContext *>( xParent.get() );

                AccessibleEventObject aEvent;
                aEvent.EventId = AccessibleEventId::SELECTION_CHANGED;
                uno::Reference< XAccessible > xChild(this);
                aEvent.NewValue <<= xChild;
                pAcc->FireAccessibleEvent( aEvent );
            }
        }
    }
}

void SwAccessibleFrameBase::InvalidateFocus_()
{
    vcl::Window *pWin = GetWindow();
    if( pWin )
    {
        bool bSelected;

        {
            osl::MutexGuard aGuard( m_Mutex );
            bSelected = m_bIsSelected;
        }
        assert(bSelected && "focus object should be selected");

        FireStateChangedEvent( AccessibleStateType::FOCUSED,
                               pWin->HasFocus() && bSelected );
    }
}

bool SwAccessibleFrameBase::HasCursor()
{
    osl::MutexGuard aGuard( m_Mutex );
    return m_bIsSelected;
}

SwAccessibleFrameBase::~SwAccessibleFrameBase()
{
}

void SwAccessibleFrameBase::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
    sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0 ;
    switch( nWhich )
    {
    case RES_NAME_CHANGED:
        {
            const SwFlyFrame *pFlyFrame = static_cast< const SwFlyFrame * >( GetFrame() );
            if(  pFlyFrame )
            {
                const SwFrameFormat *pFrameFormat = pFlyFrame->GetFormat();
                assert(pFrameFormat == GetRegisteredIn() && "invalid frame");

                const OUString sOldName( GetName() );
                assert( !pOld ||
                        static_cast<const SwStringMsgPoolItem *>(pOld)->GetString() == GetName());

                SetName( pFrameFormat->GetName() );
                assert( !pNew ||
                        static_cast<const SwStringMsgPoolItem *>(pNew)->GetString() == GetName());

                if( sOldName != GetName() )
                {
                    AccessibleEventObject aEvent;
                    aEvent.EventId = AccessibleEventId::NAME_CHANGED;
                    aEvent.OldValue <<= sOldName;
                    aEvent.NewValue <<= GetName();
                    FireAccessibleEvent( aEvent );
                }
            }
            break;
        }
    case RES_OBJECTDYING:
        // mba: it seems that this class intentionally does not call code in base class SwClient
        if( pOld && ( GetRegisteredIn() == static_cast< SwModify *>( static_cast< const SwPtrMsgPoolItem * >( pOld )->pObject ) ) )
            EndListeningAll();
        break;

    case RES_FMT_CHG:
        if( pOld &&
            static_cast< const SwFormatChg * >(pNew)->pChangedFormat == GetRegisteredIn() &&
            static_cast< const SwFormatChg * >(pOld)->pChangedFormat->IsFormatInDTOR() )
            EndListeningAll();
        break;

    default:
        // mba: former call to base class method removed as it is meant to handle only RES_OBJECTDYING
        break;
    }
}

void SwAccessibleFrameBase::Dispose(bool bRecursive, bool bCanSkipInvisible)
{
    SolarMutexGuard aGuard;
    EndListeningAll();
    SwAccessibleContext::Dispose(bRecursive, bCanSkipInvisible);
}

//Get the selection cursor of the document.
SwPaM* SwAccessibleFrameBase::GetCursor()
{
    // get the cursor shell; if we don't have any, we don't have a
    // cursor/selection either
    SwPaM* pCursor = nullptr;
    SwCursorShell* pCursorShell = GetCursorShell();
    if( pCursorShell != nullptr && !pCursorShell->IsTableMode() )
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

//Return the selected state of the object.
//when the object's anchor are in the selection cursor, we should return true.
bool SwAccessibleFrameBase::GetSelectedState( )
{
    SolarMutexGuard aGuard;

    if(GetMap()->IsDocumentSelAll())
    {
        return true;
    }

    // SELECTED.
    SwFlyFrame* pFlyFrame = getFlyFrame();
    const SwFrameFormat *pFrameFormat = pFlyFrame->GetFormat();
    const SwFormatAnchor& rAnchor = pFrameFormat->GetAnchor();
    const SwPosition *pPos = rAnchor.GetContentAnchor();
    if( !pPos )
        return false;
    int nIndex = pPos->nContent.GetIndex();
    if( pPos->nNode.GetNode().GetTextNode() )
    {
        SwPaM* pCursor = GetCursor();
        if( pCursor != nullptr )
        {
            const SwTextNode* pNode = pPos->nNode.GetNode().GetTextNode();
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
                    if( ( nHere >= nStartIndex ) && (nHere <= nEndIndex)  )
                    {
                        if( rAnchor.GetAnchorId() == RndStdIds::FLY_AS_CHAR )
                        {
                            if( ((nHere == nStartIndex) && (nIndex >= pStart->nContent.GetIndex())) || (nHere > nStartIndex) )
                                if( ((nHere == nEndIndex) && (nIndex < pEnd->nContent.GetIndex())) || (nHere < nEndIndex) )
                                    return true;
                        }
                        else if( rAnchor.GetAnchorId() == RndStdIds::FLY_AT_PARA )
                        {
                            if( ((nHere > nStartIndex) || pStart->nContent.GetIndex() ==0 )
                                && (nHere < nEndIndex ) )
                                return true;
                        }
                        break;
                    }
                    // else: this PaM doesn't point to this paragraph
                }
                // else: this PaM is collapsed and doesn't select anything

                // next PaM in ring
                pCursor = pCursor->GetNext();
            }
            while( pCursor != pRingStart );
        }
    }
    return false;
}

SwFlyFrame* SwAccessibleFrameBase::getFlyFrame() const
{
    SwFlyFrame* pFlyFrame = nullptr;

    const SwFrame* pFrame = GetFrame();
    assert(pFrame);
    if( pFrame->IsFlyFrame() )
    {
        pFlyFrame = static_cast<SwFlyFrame*>( const_cast<SwFrame*>( pFrame ) );
    }

    return pFlyFrame;
}

bool SwAccessibleFrameBase::SetSelectedState( bool )
{
    bool bParaSelected = GetSelectedState() || IsSelected();

    if (m_isSelectedInDoc != bParaSelected)
    {
        m_isSelectedInDoc = bParaSelected;
        FireStateChangedEvent( AccessibleStateType::SELECTED, bParaSelected );
        return true;
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
