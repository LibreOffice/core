/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <unotools/accessiblestatesethelper.hxx>
#include <vos/mutex.hxx>
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

#ifndef _CRSRSH_HXX
#include <crsrsh.hxx>
#endif
#ifndef _FESH_HXX
#include "fesh.hxx"
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _DCONTACT_HXX
#include <dcontact.hxx>
#endif
#ifndef _FMTANCHR_HXX
#include <fmtanchr.hxx>
#endif
using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using ::rtl::OUString;

sal_Bool SwAccessibleFrameBase::IsSelected()
{
    sal_Bool bRet = sal_False;

    DBG_ASSERT( GetMap(), "no map?" );
    const ViewShell *pVSh = GetMap()->GetShell();
    DBG_ASSERT( pVSh, "no shell?" );
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

    const ViewShell *pVSh = GetMap()->GetShell();
    DBG_ASSERT( pVSh, "no shell?" );
    sal_Bool bSelectable =  pVSh->ISA( SwFEShell );

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
        ASSERT( bIsSelected, "bSelected out of sync" );
        ::vos::ORef < SwAccessibleContext > xThis( this );
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
    vos::OGuard aGuard(Application::GetSolarMutex());

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
        vos::OGuard aGuard( aMutex );
        bOldSelected = bIsSelected;
        bIsSelected = bNewSelected;
    }

    if( bNewSelected )
    {
        // remember that object as the one that has the caret. This is
        // neccessary to notify that object if the cursor leaves it.
        ::vos::ORef < SwAccessibleContext > xThis( this );
        GetMap()->SetCursorContext( xThis );
    }

    if( bOldSelected != bNewSelected )
    {
        Window *pWin = GetWindow();
        if( pWin && pWin->HasFocus() && bNewSelected )
            FireStateChangedEvent( AccessibleStateType::FOCUSED, bNewSelected );
        //FireStateChangedEvent( AccessibleStateType::SELECTED, bNewSelected );
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
            vos::OGuard aGuard( aMutex );
            bSelected = bIsSelected;
        }
        ASSERT( bSelected, "focus object should be selected" );

        FireStateChangedEvent( AccessibleStateType::FOCUSED,
                               pWin->HasFocus() && bSelected );
    }
}

sal_Bool SwAccessibleFrameBase::HasCursor()
{
    vos::OGuard aGuard( aMutex );
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
            ASSERT( pFrmFmt == GetRegisteredIn(), "invalid frame" );

            OUString sOldName( GetName() );
            ASSERT( !pOld ||
                    static_cast < const SwStringMsgPoolItem * >( pOld )->GetString() == String( sOldName ),
                    "invalid old name" );

            const String& rNewName = pFrmFmt->GetName();
            SetName( rNewName );
            ASSERT( !pNew ||
                    static_cast < const SwStringMsgPoolItem * >( pNew )->GetString() == rNewName,
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
        // mba: it seems that this class intentionally does not call code in base class SwClient
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
        // mba: former call to base class method removed as it is meant to handle only RES_OBJECTDYING
        break;
    }
}

void SwAccessibleFrameBase::Dispose( sal_Bool bRecursive )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if( GetRegisteredIn() )
        GetRegisteredInNonConst()->Remove( this );

    SwAccessibleContext::Dispose( bRecursive );
}
//Get the selection cursor of the document.
SwPaM* SwAccessibleFrameBase::GetCrsr()
{
    // get the cursor shell; if we don't have any, we don't have a
    // cursor/selection either
    SwPaM* pCrsr = NULL;
    SwCrsrShell* pCrsrShell = GetCrsrShell();
    if( pCrsrShell != NULL && !pCrsrShell->IsTableMode() )
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
//Return the selected state of the object.
//when the object's anchor are in the selection cursor, we should return true.
sal_Bool SwAccessibleFrameBase::GetSelectedState( )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if(GetMap()->IsDocumentSelAll())
    {
        return sal_True;
    }

    // SELETED.
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
                    if( ( nHere >= nStartIndex ) && (nHere <= nEndIndex)  )
                    {
                        if( pAnchor.GetAnchorId() == FLY_AS_CHAR )
                        {
                            if( (nHere == nStartIndex) && (pIndex >= pStart->nContent.GetIndex()) || (nHere > nStartIndex) )
                                if( (nHere == nEndIndex) && (pIndex < pEnd->nContent.GetIndex()) || (nHere < nEndIndex) )
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
                    // else: this PaM doesn't point to this paragraph
                }
                // else: this PaM is collapsed and doesn't select anything

                // next PaM in ring
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
