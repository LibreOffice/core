 /*************************************************************************
 *
 *  $RCSfile: accframebase.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 16:11:21 $
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

#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLESTATETYPE_HPP_
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTID_HPP_
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#endif

#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX_
#include <unotools/accessiblestatesethelper.hxx>
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

#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _NDNOTXT_HXX
#include <ndnotxt.hxx>
#endif
#ifndef _FLYFRM_HXX
#include <flyfrm.hxx>
#endif
#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
#endif
#ifndef _FMTCNTNT_HXX
#include <fmtcntnt.hxx>
#endif
#ifndef _NDINDEX_HXX
#include <ndindex.hxx>
#endif
#ifndef _FESH_HXX
#include "fesh.hxx"
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif

#ifndef _ACCMAP_HXX
#include "accmap.hxx"
#endif
#ifndef _ACCFRAMEBASE_HXX
#include "accframebase.hxx"
#endif

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;
using namespace ::rtl;

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
        SwAccessibleMap *pMap, sal_Int16 nRole,
        const SwFlyFrm *pFlyFrm ) :
    SwAccessibleContext( pMap, nRole, pFlyFrm ),
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
        FireStateChangedEvent( AccessibleStateType::SELECTED, bNewSelected );
        if( pWin && pWin->HasFocus() && !bNewSelected )
            FireStateChangedEvent( AccessibleStateType::FOCUSED, bNewSelected );

        Reference< XAccessible > xParent( GetWeakParent() );
        if( xParent.is() )
        {
            SwAccessibleContext *pAcc =
                static_cast <SwAccessibleContext *>( xParent.get() );

            AccessibleEventObject aEvent;
            aEvent.EventId = AccessibleEventId::SELECTION_CHANGED;
            pAcc->FireAccessibleEvent( aEvent );
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

void SwAccessibleFrameBase::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
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
                    static_cast < SwStringMsgPoolItem * >( pOld )->GetString() == String( sOldName ),
                    "invalid old name" );

            const String& rNewName = pFrmFmt->GetName();
            SetName( rNewName );
            ASSERT( !pNew ||
                    static_cast < SwStringMsgPoolItem * >( pNew )->GetString() == rNewName,
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
        if( GetRegisteredIn() ==
                static_cast< SwModify *>( static_cast< SwPtrMsgPoolItem * >( pOld )->pObject ) )
            pRegisteredIn->Remove( this );
        break;

    case RES_FMT_CHG:
        if( static_cast< SwFmtChg * >(pNew)->pChangedFmt == GetRegisteredIn() &&
            static_cast< SwFmtChg * >(pOld)->pChangedFmt->IsFmtInDTOR() )
            pRegisteredIn->Remove( this );
        break;
    default:
        SwClient::Modify( pOld, pNew );
        break;
    }
}

void SwAccessibleFrameBase::Dispose( sal_Bool bRecursive )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if( GetRegisteredIn() )
        pRegisteredIn->Remove( this );

    SwAccessibleContext::Dispose( bRecursive );
}
