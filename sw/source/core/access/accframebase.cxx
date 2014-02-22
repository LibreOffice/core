/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
#include "fesh.hxx"
#include <hints.hxx>
#include "accmap.hxx"
#include "accframebase.hxx"

#include <crsrsh.hxx>
#include <txtfrm.hxx>
#include <ndtxt.hxx>
#include <dcontact.hxx>
#include <fmtanchr.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

sal_Bool SwAccessibleFrameBase::IsSelected()
{
    sal_Bool bRet = sal_False;

    OSL_ENSURE( GetMap(), "no map?" );
    const SwViewShell *pVSh = GetMap()->GetShell();
    OSL_ENSURE( pVSh, "no shell?" );
    if( pVSh->ISA( SwFEShell ) )
    {
        const SwFEShell *pFESh = static_cast< const SwFEShell * >( pVSh );
        const SwFrm *pFlyFrm = pFESh->GetCurrFlyFrm();
        if( pFlyFrm == GetFrm() )
            bRet = sal_True;
    }

    return bRet;
}

void SwAccessibleFrameBase::GetStates(
        ::utl::AccessibleStateSetHelper& rStateSet )
{
    SwAccessibleContext::GetStates( rStateSet );

    const SwViewShell *pVSh = GetMap()->GetShell();
    OSL_ENSURE( pVSh, "no shell?" );
    sal_Bool bSelectable =  pVSh->ISA( SwFEShell );

    
    if( bSelectable )
        rStateSet.AddState( AccessibleStateType::SELECTABLE );

    
    if( bSelectable )
        rStateSet.AddState( AccessibleStateType::FOCUSABLE );

    
    if( IsSelected() )
    {
        rStateSet.AddState( AccessibleStateType::SELECTED );
        OSL_ENSURE( bIsSelected, "bSelected out of sync" );
        ::rtl::Reference < SwAccessibleContext > xThis( this );
        GetMap()->SetCursorContext( xThis );

        Window *pWin = GetWindow();
        if( pWin && pWin->HasFocus() )
            rStateSet.AddState( AccessibleStateType::FOCUSED );
    }
    if( GetSelectedState() )
        rStateSet.AddState( AccessibleStateType::SELECTED );
}

sal_uInt8 SwAccessibleFrameBase::GetNodeType( const SwFlyFrm *pFlyFrm )
{
    sal_uInt8 nType = ND_TEXTNODE;
    if( pFlyFrm->Lower() )
    {
         if( pFlyFrm->Lower()->IsNoTxtFrm() )
        {
            const SwCntntFrm *pCntFrm =
                static_cast<const SwCntntFrm *>( pFlyFrm->Lower() );
            nType = pCntFrm->GetNode()->GetNodeType();
        }
    }
    else
    {
        const SwFrmFmt *pFrmFmt = pFlyFrm->GetFmt();
        const SwFmtCntnt& rCntnt = pFrmFmt->GetCntnt();
        const SwNodeIndex *pNdIdx = rCntnt.GetCntntIdx();
        if( pNdIdx )
        {
            const SwCntntNode *pCNd =
                (pNdIdx->GetNodes())[pNdIdx->GetIndex()+1]->GetCntntNode();
            if( pCNd )
                nType = pCNd->GetNodeType();
        }
    }

    return nType;
}

SwAccessibleFrameBase::SwAccessibleFrameBase(
        SwAccessibleMap* pInitMap,
        sal_Int16 nInitRole,
        const SwFlyFrm* pFlyFrm  ) :
    SwAccessibleContext( pInitMap, nInitRole, pFlyFrm ),
    bIsSelected( sal_False )
{
    SolarMutexGuard aGuard;

    const SwFrmFmt *pFrmFmt = pFlyFrm->GetFmt();
    const_cast< SwFrmFmt * >( pFrmFmt )->Add( this );

    SetName( pFrmFmt->GetName() );

    bIsSelected = IsSelected();
}

void SwAccessibleFrameBase::_InvalidateCursorPos()
{
    sal_Bool bNewSelected = IsSelected();
    sal_Bool bOldSelected;

    {
        osl::MutexGuard aGuard( aMutex );
        bOldSelected = bIsSelected;
        bIsSelected = bNewSelected;
    }

    if( bNewSelected )
    {
        
        
        ::rtl::Reference < SwAccessibleContext > xThis( this );
        GetMap()->SetCursorContext( xThis );
    }

    if( bOldSelected != bNewSelected )
    {
        Window *pWin = GetWindow();
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

void SwAccessibleFrameBase::_InvalidateFocus()
{
    Window *pWin = GetWindow();
    if( pWin )
    {
        sal_Bool bSelected;

        {
            osl::MutexGuard aGuard( aMutex );
            bSelected = bIsSelected;
        }
        OSL_ENSURE( bSelected, "focus object should be selected" );

        FireStateChangedEvent( AccessibleStateType::FOCUSED,
                               pWin->HasFocus() && bSelected );
    }
}

sal_Bool SwAccessibleFrameBase::HasCursor()
{
    osl::MutexGuard aGuard( aMutex );
    return bIsSelected;
}

SwAccessibleFrameBase::~SwAccessibleFrameBase()
{
}

void SwAccessibleFrameBase::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
    sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0 ;
    const SwFlyFrm *pFlyFrm = static_cast< const SwFlyFrm * >( GetFrm() );
    switch( nWhich )
    {
    case RES_NAME_CHANGED:
        if(  pFlyFrm )
        {
            const SwFrmFmt *pFrmFmt = pFlyFrm->GetFmt();
            OSL_ENSURE( pFrmFmt == GetRegisteredIn(), "invalid frame" );

            const OUString sOldName( GetName() );
            OSL_ENSURE( !pOld ||
                    static_cast < const SwStringMsgPoolItem * >( pOld )->GetString() == GetName(),
                    "invalid old name" );

            SetName( pFrmFmt->GetName() );
            OSL_ENSURE( !pNew ||
                    static_cast < const SwStringMsgPoolItem * >( pNew )->GetString() == GetName(),
                    "invalid new name" );

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
    case RES_OBJECTDYING:
        
        if( pOld && ( GetRegisteredIn() == static_cast< SwModify *>( static_cast< const SwPtrMsgPoolItem * >( pOld )->pObject ) ) )
            GetRegisteredInNonConst()->Remove( this );
        break;

    case RES_FMT_CHG:
        if( pOld &&
            static_cast< const SwFmtChg * >(pNew)->pChangedFmt == GetRegisteredIn() &&
            static_cast< const SwFmtChg * >(pOld)->pChangedFmt->IsFmtInDTOR() )
            GetRegisteredInNonConst()->Remove( this );
        break;

    default:
        
        break;
    }
}

void SwAccessibleFrameBase::Dispose( sal_Bool bRecursive )
{
    SolarMutexGuard aGuard;

    if( GetRegisteredIn() )
        GetRegisteredInNonConst()->Remove( this );

    SwAccessibleContext::Dispose( bRecursive );
}


SwPaM* SwAccessibleFrameBase::GetCrsr()
{
    
    
    SwPaM* pCrsr = NULL;
    SwCrsrShell* pCrsrShell = GetCrsrShell();
    if( pCrsrShell != NULL && !pCrsrShell->IsTableMode() )
    {
        SwFEShell *pFESh = pCrsrShell->ISA( SwFEShell )
                            ? static_cast< SwFEShell * >( pCrsrShell ) : 0;
        if( !pFESh ||
            !(pFESh->IsFrmSelected() || pFESh->IsObjSelected() > 0) )
        {
            
            pCrsr = pCrsrShell->GetCrsr( sal_False /* ??? */ );
        }
    }

    return pCrsr;
}



sal_Bool SwAccessibleFrameBase::GetSelectedState( )
{
    SolarMutexGuard aGuard;

    if(GetMap()->IsDocumentSelAll())
    {
        return sal_True;
    }

    
    SwFlyFrm* pFlyFrm = getFlyFrm();
    const SwFrmFmt *pFrmFmt = pFlyFrm->GetFmt();
    const SwFmtAnchor& pAnchor = pFrmFmt->GetAnchor();
    const SwPosition *pPos = pAnchor.GetCntntAnchor();
    if( !pPos )
        return sal_False;
    int pIndex = pPos->nContent.GetIndex();
    if( pPos->nNode.GetNode().GetTxtNode() )
    {
        SwPaM* pCrsr = GetCrsr();
        if( pCrsr != NULL )
        {
            const SwTxtNode* pNode = pPos->nNode.GetNode().GetTxtNode();
            sal_uLong nHere = pNode->GetIndex();

            
            SwPaM* pRingStart = pCrsr;
            do
            {
                
                if( pCrsr->HasMark() )
                {
                    
                    SwPosition* pStart = pCrsr->Start();
                    sal_uLong nStartIndex = pStart->nNode.GetIndex();
                    SwPosition* pEnd = pCrsr->End();
                    sal_uLong nEndIndex = pEnd->nNode.GetIndex();
                    if( ( nHere >= nStartIndex ) && (nHere <= nEndIndex)  )
                    {
                        if( pAnchor.GetAnchorId() == FLY_AS_CHAR )
                        {
                            if( ((nHere == nStartIndex) && (pIndex >= pStart->nContent.GetIndex())) || (nHere > nStartIndex) )
                                if( ((nHere == nEndIndex) && (pIndex < pEnd->nContent.GetIndex())) || (nHere < nEndIndex) )
                                    return sal_True;
                        }
                        else if( pAnchor.GetAnchorId() == FLY_AT_PARA )
                        {
                            if( ((nHere > nStartIndex) || pStart->nContent.GetIndex() ==0 )
                                && (nHere < nEndIndex ) )
                                return sal_True;
                        }
                        break;
                    }
                    
                }
                

                
                pCrsr = static_cast<SwPaM*>( pCrsr->GetNext() );
            }
            while( pCrsr != pRingStart );
        }
    }
    return sal_False;
}

SwFlyFrm* SwAccessibleFrameBase::getFlyFrm() const
{
    SwFlyFrm* pFlyFrm = NULL;

    const SwFrm* pFrm = GetFrm();
    DBG_ASSERT( pFrm != NULL, "frame expected" );
    if( pFrm->IsFlyFrm() )
    {
        pFlyFrm = static_cast<SwFlyFrm*>( const_cast<SwFrm*>( pFrm ) );
    }

    return pFlyFrm;
}

sal_Bool SwAccessibleFrameBase::SetSelectedState( sal_Bool )
{
    sal_Bool bParaSeleted = GetSelectedState() || IsSelected();

    if(bIsSeletedInDoc != bParaSeleted)
    {
        bIsSeletedInDoc = bParaSeleted;
        FireStateChangedEvent( AccessibleStateType::SELECTED, bParaSeleted );
        return sal_True;
    }
    return sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
