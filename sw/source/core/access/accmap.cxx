 /*************************************************************************
 *
 *  $RCSfile: accmap.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: vg $ $Date: 2002-04-19 12:59:38 $
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

#pragma hdrstop

#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif
#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif

#include <map>
#include <list>
#ifndef _ACCMAP_HXX
#include <accmap.hxx>
#endif
#ifndef _ACCCONTEXT_HXX
#include <acccontext.hxx>
#endif
#ifndef _ACCDOC_HXX
#include <accdoc.hxx>
#endif
#ifndef _ACCPARA_HXX
#include <accpara.hxx>
#endif
#ifndef _ACCHEADERFOOTER_HXX
#include <accheaderfooter.hxx>
#endif
#ifndef _ACCFOOTNOTE_HXX
#include <accfootnote.hxx>
#endif
#ifndef _ACCTEXTFRAME_HXX
#include <acctextframe.hxx>
#endif
#ifndef _ACCGRAPHIC_HXX
#include <accgraphic.hxx>
#endif
#ifndef _ACCEMBEDDED_HXX
#include <accembedded.hxx>
#endif
#ifndef _ACCCELL_HXX
#include <acccell.hxx>
#endif
#ifndef _ACCTABLE_HXX
#include <acctable.hxx>
#endif
#ifndef _FESH_HXX
#include "fesh.hxx"
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>
#endif
#ifndef _HFFRM_HXX
#include <hffrm.hxx>
#endif
#ifndef _FTNFRM_HXX
#include <ftnfrm.hxx>
#endif
#ifndef _CELLFRM_HXX
#include <cellfrm.hxx>
#endif
#ifndef _TABFRM_HXX
#include <tabfrm.hxx>
#endif
#ifndef _NDTYP_HXX
#include <ndtyp.hxx>
#endif

using namespace ::com::sun::star::uno;
using namespace ::drafts::com::sun::star::accessibility;
using namespace ::rtl;

struct SwFrmFunc
{
    sal_Bool operator()( const SwFrm * p1,
                         const SwFrm * p2) const
    {
        return p1 < p2;
    }
};

typedef ::std::map < const SwFrm *, WeakReference < XAccessible >, SwFrmFunc > _SwAccessibleContextMap_Impl;

class SwAccessibleContextMap_Impl: public _SwAccessibleContextMap_Impl
{
public:

#ifndef PRODUCT
    sal_Bool mbLocked;
#endif
    WeakReference < XAccessible > mxCursorContext;

    SwAccessibleContextMap_Impl()
#ifndef PRODUCT
        : mbLocked( sal_False )
#endif
    {}

};

//------------------------------------------------------------------------------
struct SwAccessibleEvent_Impl
{
public:
    enum EventType { CARET_OR_STATES, INVALID_CONTENT, POS_CHANGED,
                     CHILD_POS_CHANGED, DISPOSE };

private:
    SwRect      maOldBox;               // the old bounds for CHILD_POS_CHANGED
                                        // and POS_CHANGED
    WeakReference < XAccessible > mxAcc;    // The object that fires the event
    SwFrmOrObj  maFrmOrObj;             // the child for CHILD_POS_CHANGED and
                                        // the same as xAcc for any other
                                        // event type
    EventType   meType;                 // The event type
    sal_uInt8   mnStates;               // check states or update caret pos

    SwAccessibleEvent_Impl& operator==( const SwAccessibleEvent_Impl& );

public:
    SwAccessibleEvent_Impl( EventType eT, SwAccessibleContext *pA,
                             const SwFrmOrObj& rFrmOrObj ) :
        meType( eT ), mxAcc( pA ), maFrmOrObj( rFrmOrObj ), mnStates( 0 )
    {}
    SwAccessibleEvent_Impl( EventType eT, const SwFrmOrObj& rFrmOrObj ) :
        meType( eT ), maFrmOrObj( rFrmOrObj ), mnStates( 0 )
    {
        ASSERT( SwAccessibleEvent_Impl::DISPOSE == meType,
                "wrong event constructor, DISPOSE only" );
    }
    SwAccessibleEvent_Impl( EventType eT, SwAccessibleContext *pA,
                            const SwFrmOrObj& rFrmOrObj, const SwRect& rR ) :
        meType( eT ), mxAcc( pA ), maFrmOrObj( rFrmOrObj ), maOldBox( rR ),
        mnStates( 0 )
    {
        ASSERT( SwAccessibleEvent_Impl::CHILD_POS_CHANGED == meType,
                "wrong event constructor, CHILD_POS_CHANGED only" );
    }
    SwAccessibleEvent_Impl( EventType eT, SwAccessibleContext *pA,
                            const SwFrmOrObj& rFrmOrObj, sal_uInt8 nSt  ) :
        meType( eT ), mxAcc( pA ), maFrmOrObj( rFrmOrObj ), mnStates( nSt )
    {
        ASSERT( SwAccessibleEvent_Impl::CARET_OR_STATES == meType,
                "wrong event constructor, CARET_OR_STATES only" );
    }

    inline void SetType( EventType eT ){ meType = eT; }
    inline EventType    GetType() const { return meType; }

    inline ::vos::ORef < SwAccessibleContext > GetContext() const;

    inline const SwRect& GetOldBox() const { return maOldBox; }
    inline void SetOldBox( const SwRect& rOldBox ) { maOldBox = rOldBox; }

    inline const SwFrm *GetFrm() const { return maFrmOrObj.GetSwFrm(); }

    inline void SetStates( sal_uInt8 nSt ) { mnStates |= nSt; }
    inline sal_Bool IsUpdateCursorPos() const { return (mnStates & ACC_STATE_CARET) != 0; }
    inline sal_Bool IsInvalidateStates() const { return (mnStates & ACC_STATE_MASK) != 0; }
    inline sal_uInt8 GetStates() const { return mnStates & ACC_STATE_MASK; }
    inline sal_uInt8 GetAllStates() const { return mnStates; }
};

inline ::vos::ORef < SwAccessibleContext >
    SwAccessibleEvent_Impl::GetContext() const
{
    Reference < XAccessible > xTmp( mxAcc );
    ::vos::ORef < SwAccessibleContext > xAccImpl(
         static_cast< SwAccessibleContext * >( xTmp.get() ) );

    return xAccImpl;
}

//------------------------------------------------------------------------------
typedef ::std::list < SwAccessibleEvent_Impl > _SwAccessibleEventList_Impl;

class SwAccessibleEventList_Impl: public _SwAccessibleEventList_Impl
{
    sal_Bool mbFiring;

public:

    SwAccessibleEventList_Impl() : mbFiring( sal_False ) {}

    inline void SetFiring() { mbFiring = sal_True; }
    inline sal_Bool IsFiring() const { return mbFiring; }
};

//------------------------------------------------------------------------------
typedef ::std::map < const SwFrm *, SwAccessibleEventList_Impl::iterator, SwFrmFunc > _SwAccessibleEventMap_Impl;

class SwAccessibleEventMap_Impl: public _SwAccessibleEventMap_Impl
{
};

//------------------------------------------------------------------------------
static sal_Bool AreInSameTable( const Reference< XAccessible >& rAcc,
                                 const SwFrm *pFrm )
{
    sal_Bool bRet = sal_False;

    if( pFrm && pFrm->IsCellFrm() && rAcc.is() )
    {
        // Is it in the same table? We check that
        // by comparing the last table frame in the
        // follow chain, because that's cheaper than
        // searching the first one.
        SwAccessibleContext *pAccImpl =
            static_cast< SwAccessibleContext *>( rAcc.get() );
        if( pAccImpl->GetFrm()->IsCellFrm() )
        {
            const SwTabFrm *pTabFrm1 = pAccImpl->GetFrm()->FindTabFrm();
            while( pTabFrm1->GetFollow() )
                   pTabFrm1 = pTabFrm1->GetFollow();

            const SwTabFrm *pTabFrm2 = pFrm->FindTabFrm();
            while( pTabFrm2->GetFollow() )
                   pTabFrm2 = pTabFrm2->GetFollow();

            bRet = (pTabFrm1 == pTabFrm2);
        }
    }

    return bRet;
}

void SwAccessibleMap::FireEvent( const SwAccessibleEvent_Impl& rEvent )
{
    ::vos::ORef < SwAccessibleContext > xAccImpl( rEvent.GetContext() );
    if( xAccImpl.isValid() )
    {
        switch( rEvent.GetType() )
        {
        case SwAccessibleEvent_Impl::INVALID_CONTENT:
            xAccImpl->InvalidateContent();
            break;
        case SwAccessibleEvent_Impl::POS_CHANGED:
            xAccImpl->InvalidatePosOrSize( rEvent.GetOldBox() );
            break;
        case SwAccessibleEvent_Impl::CHILD_POS_CHANGED:
            xAccImpl->InvalidateChildPosOrSize( rEvent.GetFrm(),
                                       rEvent.GetOldBox() );
            break;
        case SwAccessibleEvent_Impl::DISPOSE:
            ASSERT( xAccImpl.isValid(),
                    "dispose event has been stored" );
            break;
        }
        if( SwAccessibleEvent_Impl::DISPOSE != rEvent.GetType() )
        {
            if( rEvent.IsUpdateCursorPos() )
                xAccImpl->InvalidateCursorPos();
            if( rEvent.IsInvalidateStates() )
                xAccImpl->InvalidateStates( rEvent.GetStates() );
        }
    }
}

void SwAccessibleMap::AppendEvent( const SwAccessibleEvent_Impl& rEvent )
{
    vos::OGuard aGuard( maEventMutex );

    if( !mpEvents )
        mpEvents = new SwAccessibleEventList_Impl;
    if( !mpEventMap )
        mpEventMap = new SwAccessibleEventMap_Impl;

    if( mpEvents->IsFiring() )
    {
        // While events are fired new ones are generated. They have to be fired
        // now. This does not work for DISPOSE events!
        ASSERT( rEvent.GetType() != SwAccessibleEvent_Impl::DISPOSE,
                "dispose event while firing events" );
        FireEvent( rEvent );
    }
    else
    {

        SwAccessibleEventMap_Impl::iterator aIter =
            mpEventMap->find( rEvent.GetFrm() );
        if( aIter != mpEventMap->end() )
        {
            SwAccessibleEvent_Impl aEvent( *(*aIter).second );
            ASSERT( aEvent.GetType() != SwAccessibleEvent_Impl::DISPOSE,
                    "dispose events should not be stored" );
            sal_Bool bAppendEvent = sal_True;
            switch( rEvent.GetType() )
            {
            case SwAccessibleEvent_Impl::CARET_OR_STATES:
                // A CARET_OR_STATES event is added to any other
                // event only. It is broadcasted after any other event, so the
                // event should be put to the back.
                ASSERT( aEvent.GetType() !=
                            SwAccessibleEvent_Impl::CHILD_POS_CHANGED,
                        "invalid event combination" );
                aEvent.SetStates( rEvent.GetAllStates() );
                break;
            case SwAccessibleEvent_Impl::INVALID_CONTENT:
                // An INVALID_CONTENT event overwrites a CARET_OR_STATES
                // event (but keeps its flags) and it is contained in a
                // POS_CHANGED event.
                // Therefor, the event's type has to be adapted and the event
                // has to be put at the end.
                ASSERT( aEvent.GetType() !=
                               SwAccessibleEvent_Impl::CHILD_POS_CHANGED,
                        "invalid event combination" );
                if( aEvent.GetType() ==
                        SwAccessibleEvent_Impl::CARET_OR_STATES )
                    aEvent.SetType( SwAccessibleEvent_Impl::INVALID_CONTENT );
                break;
            case SwAccessibleEvent_Impl::POS_CHANGED:
                // A pos changed event overwrites CARET_STATES (keeping its
                // flags) as well as INVALID_CONTENT. The old box position
                // has to be stored however if the old event is not a
                // POS_CHANGED itself.
                ASSERT( aEvent.GetType() !=
                            SwAccessibleEvent_Impl::CHILD_POS_CHANGED,
                        "invalid event combination" );
                if( aEvent.GetType() != SwAccessibleEvent_Impl::POS_CHANGED )
                    aEvent.SetOldBox( rEvent.GetOldBox() );
                aEvent.SetType( SwAccessibleEvent_Impl::POS_CHANGED );
                break;
            case SwAccessibleEvent_Impl::CHILD_POS_CHANGED:
                // CHILD_POS_CHANGED events can only follow CHILD_POS_CHANGED
                // events. The only action that needs to be done again is
                // to put the old event to the back. The new one cannot be used,
                // because we are interested in the old frame bounds.
                ASSERT( aEvent.GetType() ==
                            SwAccessibleEvent_Impl::CHILD_POS_CHANGED,
                        "invalid event combination" );
                break;
            case SwAccessibleEvent_Impl::DISPOSE:
                // DISPOSE events overwrite all others. They are not stored
                // but executed immediatly to avoid broadcasting of
                // defunctional objects. So what needs to be done here is to
                // remove all events for the frame in question.
                ASSERT( aEvent.GetType() !=
                            SwAccessibleEvent_Impl::CHILD_POS_CHANGED,
                        "invalid event combination" );
                bAppendEvent = sal_False;
                break;
            }
            if( bAppendEvent )
            {
                mpEvents->erase( (*aIter).second );
                (*aIter).second = mpEvents->insert( mpEvents->end(), aEvent );
            }
            else
            {
                mpEvents->erase( (*aIter).second );
                mpEventMap->erase( aIter );
            }
        }
        else if( SwAccessibleEvent_Impl::DISPOSE != rEvent.GetType() )
        {
            SwAccessibleEventMap_Impl::value_type aEntry( rEvent.GetFrm(),
                    mpEvents->insert( mpEvents->end(), rEvent ) );
            mpEventMap->insert( aEntry );
        }
    }
}

void SwAccessibleMap::InvalidateCursorPosition(
        const Reference< XAccessible >& rAcc )
{
    SwAccessibleContext *pAccImpl =
        static_cast< SwAccessibleContext *>( rAcc.get() );
    ASSERT( pAccImpl, "no caret context" );
    ASSERT( pAccImpl->GetFrm(), "caret context is disposed" );
    if( GetShell()->ActionPend() )
    {
        SwAccessibleEvent_Impl aEvent(
            SwAccessibleEvent_Impl::CARET_OR_STATES, pAccImpl,
            pAccImpl->GetFrm(), ACC_STATE_CARET );
        AppendEvent( aEvent );
    }
    else
    {
        pAccImpl->InvalidateCursorPos();
    }
}


SwAccessibleMap::SwAccessibleMap( ViewShell *pSh ) :
    mpMap( 0  ),
    mpEvents( 0  ),
    mpEventMap( 0  ),
    mpVSh( pSh ),
    mnPara( 1 ),
    mnFootnote( 1 ),
    mnEndnote( 1 )
{
}

SwAccessibleMap::~SwAccessibleMap()
{
    Reference < XAccessible > xAcc;
    {
        vos::OGuard aGuard( maMutex );
        if( mpMap )
        {
            const SwRootFrm *pRootFrm = GetShell()->GetLayout();
            SwAccessibleContextMap_Impl::iterator aIter = mpMap->find( pRootFrm );
            if( aIter != mpMap->end() )
                xAcc = (*aIter).second;
            if( !xAcc.is() )
                xAcc = new SwAccessibleDocument( this );
        }
    }

    SwAccessibleDocument *pAcc =
        static_cast< SwAccessibleDocument * >( xAcc.get() );
    pAcc->Dispose( sal_True );

    {
        vos::OGuard aGuard( maMutex );
#ifndef PRODUCT
        ASSERT( !mpMap || mpMap->empty(),
                "Map should be empty after disposing the root frame" );
        if( mpMap )
        {
            SwAccessibleContextMap_Impl::iterator aIter = mpMap->begin();
            while( aIter != mpMap->end() )
            {
                Reference < XAccessible > xTmp = (*aIter).second;
                if( xTmp.is() )
                {
                    SwAccessibleContext *pTmp =
                        static_cast< SwAccessibleContext * >( xTmp.get() );
                }
            }
        }
#endif
        delete mpMap;
        mpMap = 0;
    }

    {
        vos::OGuard aGuard( maEventMutex );
        ASSERT( !(mpEvents || mpEventMap), "pending events" );
        delete mpEventMap;
        mpEventMap = 0;
        delete mpEvents;
        mpEvents = 0;
    }
}

Reference< XAccessible > SwAccessibleMap::GetDocumentView()
{
    Reference < XAccessible > xAcc;
    sal_Bool bSetVisArea = sal_False;

    {
        vos::OGuard aGuard( maMutex );

        if( !mpMap )
        {
            mpMap = new SwAccessibleContextMap_Impl;
#ifndef PRODUCT
            mpMap->mbLocked = sal_False;
#endif
        }

#ifndef PRODUCT
        ASSERT( !mpMap->mbLocked, "Map is locked" );
        mpMap->mbLocked = sal_True;
#endif

        const SwRootFrm *pRootFrm = GetShell()->GetLayout();
        SwAccessibleContextMap_Impl::iterator aIter = mpMap->find( pRootFrm );
        if( aIter != mpMap->end() )
            xAcc = (*aIter).second;
        if( xAcc.is() )
        {
            bSetVisArea = sal_True; // Set VisArea when map mutex is not
                                    // locked
        }
        else
        {
            xAcc = new SwAccessibleDocument( this );
            if( aIter != mpMap->end() )
            {
                (*aIter).second = xAcc;
            }
            else
            {
                SwAccessibleContextMap_Impl::value_type aEntry( pRootFrm, xAcc );
                mpMap->insert( aEntry );
            }
        }

#ifndef PRODUCT
        mpMap->mbLocked = sal_False;
#endif
    }

    if( bSetVisArea )
    {
        SwAccessibleDocument *pAcc =
            static_cast< SwAccessibleDocument * >( xAcc.get() );
        pAcc->SetVisArea();
    }

    return xAcc;
}

Reference< XAccessible> SwAccessibleMap::GetContext( const SwFrm *pFrm,
                                                     sal_Bool bCreate )
{
    Reference < XAccessible > xAcc;
    Reference < XAccessible > xOldCursorAcc;

    {
        vos::OGuard aGuard( maMutex );

        if( !mpMap && bCreate )
            mpMap = new SwAccessibleContextMap_Impl;
        if( mpMap )
        {
            SwAccessibleContextMap_Impl::iterator aIter = mpMap->find( pFrm );
            if( aIter != mpMap->end() )
                xAcc = (*aIter).second;

            if( !xAcc.is() && bCreate )
            {
                SwAccessibleContext *pAcc = 0;
                switch( pFrm->GetType() )
                {
                case FRM_TXT:
                    pAcc = new SwAccessibleParagraph( this, mnPara++,
                                    static_cast< const SwTxtFrm * >( pFrm ) );
                    break;
                case FRM_HEADER:
                    pAcc = new SwAccessibleHeaderFooter( this,
                                    static_cast< const SwHeaderFrm *>( pFrm ) );
                    break;
                case FRM_FOOTER:
                    pAcc = new SwAccessibleHeaderFooter( this,
                                    static_cast< const SwFooterFrm *>( pFrm ) );
                    break;
                case FRM_FTN:
                    {
                        const SwFtnFrm *pFtnFrm =
                            static_cast < const SwFtnFrm * >( pFrm );
                        sal_Bool bIsEndnote =
                            SwAccessibleFootnote::IsEndnote( pFtnFrm );
                        pAcc = new SwAccessibleFootnote( this, bIsEndnote,
                                    (bIsEndnote ? mnEndnote++ : mnFootnote++),
                                    pFtnFrm );
                    }
                    break;
                case FRM_FLY:
                    {
                        const SwFlyFrm *pFlyFrm =
                            static_cast < const SwFlyFrm * >( pFrm );
                        switch( SwAccessibleFrameBase::GetNodeType( pFlyFrm ) )
                        {
                        case ND_GRFNODE:
                            pAcc = new SwAccessibleGraphic( this, pFlyFrm );
                            break;
                        case ND_OLENODE:
                            pAcc = new SwAccessibleEmbeddedObject( this, pFlyFrm );
                            break;
                        default:
                            pAcc = new SwAccessibleTextFrame( this, pFlyFrm );
                            break;
                        }
                    }
                    break;
                case FRM_CELL:
                    pAcc = new SwAccessibleCell( this,
                                    static_cast< const SwCellFrm *>( pFrm ) );
                    break;
                case FRM_TAB:
                    pAcc = new SwAccessibleTable( this,
                                    static_cast< const SwTabFrm *>( pFrm ) );
                    break;
                }
                xAcc = pAcc;

                ASSERT( xAcc.is(), "unknown frame type" );
                if( xAcc.is() )
                {
                    if( aIter != mpMap->end() )
                    {
                        (*aIter).second = xAcc;
                    }
                    else
                    {
                        SwAccessibleContextMap_Impl::value_type aEntry( pFrm, xAcc );
                        mpMap->insert( aEntry );
                    }

                    if( pAcc->HasCursor() &&
                        !AreInSameTable( mpMap->mxCursorContext, pFrm ) )
                    {
                        // If the new context has the focus, and if we know
                        // another context that had the focus, then the focus
                        // just moves from the old context to the new one. We
                        // have to send a focus event and a caret event for
                        // the old context then. We have to to that know,
                        // because after we have left this method, anyone might
                        // call getStates for the new context and will get a
                        // focused state then. Sending the focus changes event
                        // after that seems to be strange. However, we cannot
                        // send a focus event fo the new context now, because
                        // noone except us knows it. In any case, we remeber
                        // the new context as the one that has the focus
                        // currently.

                        xOldCursorAcc = mpMap->mxCursorContext;
                        mpMap->mxCursorContext = xAcc;
                    }
                }
            }
        }
    }

    // Invalidate focus for old object when map is not locked
    if( xOldCursorAcc.is() )
        InvalidateCursorPosition( xOldCursorAcc );

    return xAcc;
}

::vos::ORef < SwAccessibleContext > SwAccessibleMap::GetContextImpl(
            const SwFrm *pFrm,
            sal_Bool bCreate )
{
    Reference < XAccessible > xAcc( GetContext( pFrm, bCreate ) );

    ::vos::ORef < SwAccessibleContext > xAccImpl(
         static_cast< SwAccessibleContext * >( xAcc.get() ) );

    return xAccImpl;
}

void SwAccessibleMap::RemoveContext( const SwFrm *pFrm )
{
    vos::OGuard aGuard( maMutex );

    if( mpMap )
    {
        SwAccessibleContextMap_Impl::iterator aIter =
            mpMap->find( pFrm );
        if( aIter != mpMap->end() )
        {
            mpMap->erase( aIter );

            // Remove reference to old caret object
            Reference < XAccessible > xOldAcc( mpMap->mxCursorContext );
            if( xOldAcc.is() )
            {
                SwAccessibleContext *pOldAccImpl =
                    static_cast< SwAccessibleContext *>( xOldAcc.get() );
                ASSERT( pOldAccImpl->GetFrm(), "old caret context is disposed" );
                if( pOldAccImpl->GetFrm() == pFrm )
                {
                    xOldAcc.clear();    // get an empty ref
                    mpMap->mxCursorContext = xOldAcc;
                }
            }

            if( mpMap->empty() )
            {
                delete mpMap;
                mpMap = 0;
            }
        }
    }
}


void SwAccessibleMap::Dispose( const SwFrm *pFrm, sal_Bool bRecursive )
{
    // Indeed, the following assert checks the frame's accessible flag,
    // because that's the one that is evaluated in the layout. The frame
    // might not be accessible anyway. That's the case for cell frames that
    // contain further cells.
    ASSERT( pFrm->IsAccessibleFrm(),
            "non accessible frame should be disposed" );

    SwFrmOrObj aFrmOrObj( pFrm );
    Reference < XAccessible > xAcc;
    Reference < XAccessible > xParentAcc;
    if( aFrmOrObj.IsAccessible() )
    {
        // get accessible context for frame
        {
            vos::OGuard aGuard( maMutex );

            if( mpMap )
            {
                SwAccessibleContextMap_Impl::iterator aIter =
                    mpMap->find( pFrm );
                if( aIter != mpMap->end() )
                {
                    xAcc = (*aIter).second;
                }
                else
                {
                    // Otherwise we look if the parent is accessible.
                    // If not, there is nothing to do.
                    const SwFrm *pParent =
                        SwAccessibleFrame::GetParent( aFrmOrObj.GetSwFrm() );

                    if( pParent )
                    {
                        aIter = mpMap->find( pParent );
                        if( aIter != mpMap->end() )
                        {
                            xParentAcc = (*aIter).second;
                        }
                    }
                }
            }
        }

        // remove events stored for the frame
        {
            vos::OGuard aGuard( maEventMutex );
            if( mpEvents )
            {
                SwAccessibleEventMap_Impl::iterator aIter =
                    mpEventMap->find( aFrmOrObj.GetSwFrm() );
                if( aIter != mpEventMap->end() )
                {
                    SwAccessibleEvent_Impl aEvent(
                            SwAccessibleEvent_Impl::DISPOSE, aFrmOrObj );
                    AppendEvent( aEvent );
                }
            }
        }

        // If the frame is accessible and there is a context for it, dispose
        // the frame. If the frame is no context for it but disposing should
        // take place recursive, the frame's children have to be disposed
        // anyway, so we have to create the context then.
        if( xAcc.is() )
        {
            SwAccessibleContext *pAccImpl =
                static_cast< SwAccessibleContext *>( xAcc.get() );
            pAccImpl->Dispose( bRecursive );
        }
        else if( xParentAcc.is() )
        {
            // If the frame is a cell frame, the table must be notified.
            // If we are in an action, a table model change event will
            // be broadcasted at the end of the action to give the table
            // a chance to generate a single table change event.

            SwAccessibleContext *pAccImpl =
                static_cast< SwAccessibleContext *>( xParentAcc.get() );
            pAccImpl->DisposeChild( aFrmOrObj.GetSwFrm(), bRecursive );
        }
    }
}

void SwAccessibleMap::InvalidatePosOrSize( const SwFrm *pFrm,
                                           const SwRect& rOldBox )
{
    SwFrmOrObj aFrmOrObj( pFrm );
    if( aFrmOrObj.IsAccessible() )
    {
        Reference < XAccessible > xAcc;
        Reference < XAccessible > xParentAcc;
        {
            vos::OGuard aGuard( maMutex );

            if( mpMap )
            {
                SwAccessibleContextMap_Impl::iterator aIter =
                    mpMap->find( aFrmOrObj.GetSwFrm() );
                if( aIter != mpMap->end() )
                {
                    // If there is an accesible object already it is
                    // notified directly.
                    xAcc = (*aIter).second;
                }
                else
                {
                    // Otherwise we look if the parent is accessible.
                    // If not, there is nothing to do.
                    const SwFrm *pParent =
                        SwAccessibleFrame::GetParent( aFrmOrObj.GetSwFrm() );

                    if( pParent )
                    {
                        aIter = mpMap->find( pParent );
                        if( aIter != mpMap->end() )
                        {
                            xParentAcc = (*aIter).second;
                        }
                    }
                }
            }
        }

        if( xAcc.is() )
        {
            ASSERT( !rOldBox.IsEmpty(), "context should have a size" );
            SwAccessibleContext *pAccImpl =
                static_cast< SwAccessibleContext *>( xAcc.get() );
            if( GetShell()->ActionPend() )
            {
                SwAccessibleEvent_Impl aEvent(
                    SwAccessibleEvent_Impl::POS_CHANGED, pAccImpl,
                    aFrmOrObj, rOldBox );
                AppendEvent( aEvent );
            }
            else
            {
                pAccImpl->InvalidatePosOrSize( rOldBox );
            }
        }
        else if( xParentAcc.is() )
        {
            SwAccessibleContext *pAccImpl =
                static_cast< SwAccessibleContext *>(xParentAcc.get());
            if( GetShell()->ActionPend() )
            {
                SwAccessibleEvent_Impl aEvent(
                    SwAccessibleEvent_Impl::CHILD_POS_CHANGED,
                    pAccImpl, aFrmOrObj, rOldBox );
                AppendEvent( aEvent );
            }
            else
            {
                pAccImpl->InvalidateChildPosOrSize( aFrmOrObj.GetSwFrm(),
                                                    rOldBox );
            }
        }
    }
}

void SwAccessibleMap::InvalidateContent( const SwFrm *pFrm )
{
    SwFrmOrObj aFrmOrObj( pFrm );
    if( aFrmOrObj.IsAccessible() )
    {
        Reference < XAccessible > xAcc;
        {
            vos::OGuard aGuard( maMutex );

            if( mpMap )
            {
                SwAccessibleContextMap_Impl::iterator aIter =
                    mpMap->find( aFrmOrObj.GetSwFrm() );
                if( aIter != mpMap->end() )
                    xAcc = (*aIter).second;
            }
        }

        if( xAcc.is() )
        {
            SwAccessibleContext *pAccImpl =
                static_cast< SwAccessibleContext *>( xAcc.get() );
            if( GetShell()->ActionPend() )
            {
                SwAccessibleEvent_Impl aEvent(
                    SwAccessibleEvent_Impl::INVALID_CONTENT, pAccImpl,
                    aFrmOrObj );
                AppendEvent( aEvent );
            }
            else
            {
                pAccImpl->InvalidateContent();
            }
        }
    }
}

void SwAccessibleMap::InvalidateCursorPosition( const SwFrm *pFrm )
{
    SwFrmOrObj aFrmOrObj( pFrm );
    const ViewShell *pVSh = GetShell();
    if( pVSh->ISA( SwCrsrShell ) )
    {
        const SwCrsrShell *pCSh = static_cast< const SwCrsrShell * >( pVSh );
        if( pCSh->IsTableMode() )
        {
            while( aFrmOrObj.GetSwFrm() && !aFrmOrObj.GetSwFrm()->IsCellFrm() )
                aFrmOrObj = aFrmOrObj.GetSwFrm()->GetUpper();
        }
        else if( pVSh->ISA( SwFEShell ) )
        {
            sal_uInt16 nObjCount;
            const SwFEShell *pFESh = static_cast< const SwFEShell * >( pVSh );
            const SwFrm *pFlyFrm = pFESh->GetCurrFlyFrm();
            if( pFlyFrm )
            {
                ASSERT( !pFrm || pFrm->FindFlyFrm() == pFlyFrm,
                        "cursor is not contained in fly frame" );
                aFrmOrObj = pFlyFrm;
            }
            else if( (nObjCount = pFESh->IsObjSelected()) > 0 )
            {
                aFrmOrObj = static_cast<const SwFrm *>( 0 );
            }
        }
    }

    ASSERT( aFrmOrObj.IsAccessible(), "frame is not accessible" );

    Reference < XAccessible > xOldAcc;
    Reference < XAccessible > xAcc;

    {
        vos::OGuard aGuard( maMutex );

        if( mpMap )
        {
            xOldAcc = mpMap->mxCursorContext;
            mpMap->mxCursorContext = xAcc;  // clear reference

            if( aFrmOrObj.IsAccessible() )
            {
                SwAccessibleContextMap_Impl::iterator aIter =
                    mpMap->find( aFrmOrObj.GetSwFrm() );
                if( aIter != mpMap->end() )
                    xAcc = (*aIter).second;

                // For cells, some extra thoughts are necessary,
                // because invalidating the cursor for one cell
                // invalidates the cursor for all cells of the same
                // table. For this reason, we don't want to
                // invalidate the cursor for the old cursor object
                // and the new one if they are within the same table,
                // because this would result in doing the work twice.
                // Moreover, we have to make sure to invalidate the
                // cursor even if the current cell has no accessible object.
                // If the old cursor objects exists and is in the same
                // table, its the best choice, because using it avoids
                // an unnessarary cursor invalidation cycle when creating
                // a new object for the current cell.
                if( aFrmOrObj.GetSwFrm()->IsCellFrm() )
                {
                    if( xOldAcc.is() &&
                         AreInSameTable( xOldAcc, aFrmOrObj.GetSwFrm() ) )
                    {
                        if( xAcc.is() )
                            xOldAcc = xAcc; // avoid extra invalidation
                        else
                            xAcc = xOldAcc; // make sure ate least one
                    }
                    if( !xAcc.is() )
                        xAcc = GetContext( aFrmOrObj.GetSwFrm(), sal_True );
                }
            }
        }
    }

    if( xOldAcc.is() && xOldAcc != xAcc )
        InvalidateCursorPosition( xOldAcc );
    if( xAcc.is() )
        InvalidateCursorPosition( xAcc );
}

void SwAccessibleMap::SetCursorContext(
        const ::vos::ORef < SwAccessibleContext >& rCursorContext )
{
    vos::OGuard aGuard( maMutex );
    if( mpMap )
    {
        Reference < XAccessible > xAcc( rCursorContext.getBodyPtr() );
        mpMap->mxCursorContext = xAcc;
    }
}

void SwAccessibleMap::InvalidateStates( sal_uInt8 nStates )
{
    Reference< XAccessible > xAcc( GetDocumentView() );
    SwAccessibleContext *pAccImpl =
        static_cast< SwAccessibleContext *>( xAcc.get() );
    if( GetShell()->ActionPend() )
    {
        SwAccessibleEvent_Impl aEvent(
                SwAccessibleEvent_Impl::CARET_OR_STATES, pAccImpl,
                        pAccImpl->GetFrm(), nStates );
        AppendEvent( aEvent );
    }
    else
    {
        pAccImpl->InvalidateStates( nStates );
    }
}

void SwAccessibleMap::FireEvents()
{
    vos::OGuard aGuard( maEventMutex );
    if( mpEvents )
    {
        mpEvents->SetFiring();
        SwAccessibleEventList_Impl::iterator aIter = mpEvents->begin();
        while( aIter != mpEvents->end() )
        {
            FireEvent( *aIter );
            ++aIter;
        }
    }

    delete mpEventMap;
    mpEventMap = 0;

    delete mpEvents;
    mpEvents = 0;
}
