 /*************************************************************************
 *
 *  $RCSfile: accmap.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: mib $ $Date: 2002-03-19 12:49:27 $
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
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>
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
    sal_Bool bLocked;
    WeakReference < XAccessible > xCaretContext;
};

struct SwAccessibleEvent_Impl
{
public:
    enum EventType { INVALID_CONTENT, POS_CHANGED, CHILD_POS_CHANGED, DISPOSE,
                         CARET_OR_STATES };

private:
    SwRect      aOldFrm;                // the old bounds for CHILD_POS_CHANGED
    WeakReference < XAccessible > xAcc; // The object that fires the event
    const SwFrm *pFrm;                  // the child for CHILD_POS_CHANGED and
                                        // the same as xAcc for any other
                                        // event type
    EventType   eType;                  // The event type
    sal_uInt8   nStates;                // check states or update caret pos

    SwAccessibleEvent_Impl& operator==( const SwAccessibleEvent_Impl& );

public:
    SwAccessibleEvent_Impl( EventType eT, SwAccessibleContext *pA,
                             const SwFrm *pF ) :
        eType( eT ), xAcc( pA ), pFrm( pF ), nStates( 0 )
    {}
    SwAccessibleEvent_Impl( EventType eT, const SwFrm *pF ) :
        eType( eT ), pFrm( pF ), nStates( 0 )
    {
        ASSERT( SwAccessibleEvent_Impl::DISPOSE == eType,
                "wrong event constructor, DISPOSE only" );
    }
    SwAccessibleEvent_Impl( EventType eT, SwAccessibleContext *pA,
                            const SwFrm *pF, const SwRect& rR ) :
        eType( eT ), xAcc( pA ), pFrm( pF ), aOldFrm( rR ), nStates( 0 )
    {
        ASSERT( SwAccessibleEvent_Impl::CHILD_POS_CHANGED == eType,
                "wrong event constructor, CHILD_POS_CHANGED only" );
    }
    SwAccessibleEvent_Impl( EventType eT, SwAccessibleContext *pA,
                            const SwFrm *pF, sal_uInt8 nSt  ) :
        eType( eT ), xAcc( pA ), pFrm( pF ), nStates( nSt )
    {
        ASSERT( SwAccessibleEvent_Impl::CARET_OR_STATES == eType,
                "wrong event constructor, CARET_OR_STATES only" );
    }

    inline void SetType( EventType eT ){ eType = eT; }
    inline EventType    GetType() const { return eType; }

    inline ::vos::ORef < SwAccessibleContext > GetContext() const;

    inline const SwRect& GetOldFrm() const { return aOldFrm; }

    inline const SwFrm *GetFrm() const { return pFrm; }

    inline SetStates( sal_uInt8 nSt ) { nStates |= nSt; }
    inline sal_Bool IsUpdateCaretPos() const { return (nStates & ACC_STATE_CARET) != 0; }
    inline sal_Bool IsCheckStates() const { return (nStates & ACC_STATE_MASK) != 0; }
    inline sal_uInt8 GetStates() const { return nStates & ACC_STATE_MASK; }
    inline sal_uInt8 GetAllStates() const { return nStates; }
};

inline ::vos::ORef < SwAccessibleContext >
    SwAccessibleEvent_Impl::GetContext() const
{
    Reference < XAccessible > xTmp( xAcc );
    ::vos::ORef < SwAccessibleContext > xAccImpl(
         static_cast< SwAccessibleContext * >( xTmp.get() ) );

    return xAccImpl;
}


typedef ::std::list < SwAccessibleEvent_Impl > _SwAccessibleEventList_Impl;

class SwAccessibleEventList_Impl: public _SwAccessibleEventList_Impl
{
};

typedef ::std::map < const SwFrm *, SwAccessibleEventList_Impl::iterator, SwFrmFunc > _SwAccessibleEventMap_Impl;

class SwAccessibleEventMap_Impl: public _SwAccessibleEventMap_Impl
{
};


SwAccessibleMap::SwAccessibleMap( ViewShell *pSh ) :
    pMap( 0  ),
    pEvents( 0  ),
    pEventMap( 0  ),
    pVSh( pSh ),
    nPara( 1 ),
    nFootnote( 1 ),
    nEndnote( 1 )
{
}

SwAccessibleMap::~SwAccessibleMap()
{
    vos::OGuard aGuard( aMutex );
    if( pMap )
    {
        Reference < XAccessible > xAcc;
        const SwRootFrm *pRootFrm = GetShell()->GetLayout();
        SwAccessibleContextMap_Impl::iterator aIter = pMap->find( pRootFrm );
        if( aIter != pMap->end() )
            xAcc = (*aIter).second;
        if( !xAcc.is() )
            xAcc = new SwAccessibleDocument( this );
        SwAccessibleDocument *pAcc =
            static_cast< SwAccessibleDocument * >( xAcc.get() );
        pAcc->Dispose( sal_True );

        ASSERT( !pMap || pMap->empty(),
                "Map should be empty after disposing the root frame" );
    }

    delete pMap;

    ASSERT( !(pEvents || pEventMap), "pending events" );
    delete pEventMap;
    delete pEvents;
}

void SwAccessibleMap::AppendEvent( const SwAccessibleEvent_Impl& rEvent )
{
    vos::OGuard aGuard( aEventMutex );

    if( !pEvents )
        pEvents = new SwAccessibleEventList_Impl;
    if( !pEventMap )
        pEventMap = new SwAccessibleEventMap_Impl;

    SwAccessibleEventMap_Impl::iterator aIter =
        pEventMap->find( rEvent.GetFrm() );
    if( aIter != pEventMap->end() )
    {
        SwAccessibleEvent_Impl aEvent( *(*aIter).second );
        sal_Bool bAppendEvent = sal_True;
        switch( rEvent.GetType() )
        {
        case SwAccessibleEvent_Impl::CARET_OR_STATES:
            // A CARET_POS or CHECK_STATES event is added to any other
            // event only. It is broadcasted after any other event, so the
            // event should be put to the back.
            ASSERT( aEvent.GetType()!=SwAccessibleEvent_Impl::CHILD_POS_CHANGED,
                    "invalid event combination" );
            ASSERT( aEvent.GetType() != SwAccessibleEvent_Impl::DISPOSE,
                    "dispose events should not be stored" );
            aEvent.SetStates( rEvent.GetAllStates() );
            break;
        case SwAccessibleEvent_Impl::INVALID_CONTENT:
            // All events except CARET_OR_STATES include a INVALID_CONTENT,
            // so the only action that needs to be done is to put the event
            // to the back. That's done automatically.
            ASSERT( aEvent.GetType()!=SwAccessibleEvent_Impl::CHILD_POS_CHANGED,
                    "invalid event combination" );
            ASSERT( aEvent.GetType() != SwAccessibleEvent_Impl::DISPOSE,
                    "dispose events should not be stored" );
            if( aEvent.GetType() == SwAccessibleEvent_Impl::CARET_OR_STATES )
                aEvent.SetType( SwAccessibleEvent_Impl::INVALID_CONTENT );
            break;
        case SwAccessibleEvent_Impl::POS_CHANGED:
            // If the the old event is not a DISPOSE event, the new event
            // includes the old one.
            ASSERT( aEvent.GetType()!=SwAccessibleEvent_Impl::CHILD_POS_CHANGED,
                    "invalid event combination" );
            ASSERT( aEvent.GetType() != SwAccessibleEvent_Impl::DISPOSE,
                    "dispose events should not be stored" );
            aEvent.SetType( SwAccessibleEvent_Impl::POS_CHANGED );
            break;
        case SwAccessibleEvent_Impl::CHILD_POS_CHANGED:
            // CHILD_POS_CHANGED events can only follow CHILD_POS_CHANGED
            // events. The only action that needs to be done again is
            // to put the old event to the back. The new one cannot be used,
            // because we are interested in the old frame bounds.
            ASSERT( aEvent.GetType()==SwAccessibleEvent_Impl::CHILD_POS_CHANGED,
                    "invalid event combination" );
            break;
        case SwAccessibleEvent_Impl::DISPOSE:
            // DISPOSE events overwrite all others. They are not stored
            // but executed immidiatly to avoid broadcasting of defuntional
            // objects. So what needs to be done here is to remove all
            // events for the frame in question.
            ASSERT( aEvent.GetType()!=SwAccessibleEvent_Impl::CHILD_POS_CHANGED,
                    "invalid event combination" );
            ASSERT( aEvent.GetType() != SwAccessibleEvent_Impl::DISPOSE,
                    "dispose events should not be stored" );
            bAppendEvent = sal_False;
            break;
        }
        if( bAppendEvent )
        {
            pEvents->erase( (*aIter).second );
            (*aIter).second = pEvents->insert( pEvents->end(), aEvent );
        }
        else
        {
            pEvents->erase( (*aIter).second );
            pEventMap->erase( aIter );
        }
    }
    else if( SwAccessibleEvent_Impl::DISPOSE != rEvent.GetType() )
    {
        SwAccessibleEventMap_Impl::value_type aEntry( rEvent.GetFrm(),
                pEvents->insert( pEvents->end(), rEvent ) );
        pEventMap->insert( aEntry );
    }

}

Reference< XAccessible > SwAccessibleMap::GetDocumentView()
{
    vos::OGuard aGuard( aMutex );

    Reference < XAccessible > xAcc;

    if( !pMap )
    {
        pMap = new SwAccessibleContextMap_Impl;
        pMap->bLocked = sal_False;
    }
    ASSERT( !pMap->bLocked, "Map is locked" );
    pMap->bLocked = sal_True;

    const SwRootFrm *pRootFrm = GetShell()->GetLayout();
    SwAccessibleContextMap_Impl::iterator aIter = pMap->find( pRootFrm );
    if( aIter != pMap->end() )
        xAcc = (*aIter).second;
    if( xAcc.is() )
    {
        SwAccessibleDocument *pAcc =
            static_cast< SwAccessibleDocument * >( xAcc.get() );
        pAcc->SetVisArea( GetShell()->VisArea().SVRect() );
    }
    else
    {
        xAcc = new SwAccessibleDocument( this );
        if( aIter != pMap->end() )
        {
            (*aIter).second = xAcc;
        }
        else
        {
            SwAccessibleContextMap_Impl::value_type aEntry( pRootFrm, xAcc );
            pMap->insert( aEntry );
        }
    }

    pMap->bLocked = sal_False;

    return xAcc;
}

Reference< XAccessible> SwAccessibleMap::GetContext( const SwFrm *pFrm,
                                                     sal_Bool bCreate )
{
    vos::OGuard aGuard( aMutex );

    Reference < XAccessible > xAcc;

    if( !pMap && bCreate )
        pMap = new SwAccessibleContextMap_Impl;
    if( pMap )
    {
        SwAccessibleContextMap_Impl::iterator aIter = pMap->find( pFrm );
        if( aIter != pMap->end() )
            xAcc = (*aIter).second;

        if( !xAcc.is() && bCreate )
        {
            switch( pFrm->GetType() )
            {
            case FRM_TXT:
                xAcc = new SwAccessibleParagraph( this, nPara++,
                                static_cast< const SwTxtFrm * >( pFrm ) );
                break;
            case FRM_HEADER:
                xAcc = new SwAccessibleHeaderFooter( this,
                                static_cast< const SwHeaderFrm *>( pFrm ) );
                break;
            case FRM_FOOTER:
                xAcc = new SwAccessibleHeaderFooter( this,
                                static_cast< const SwFooterFrm *>( pFrm ) );
                break;
            case FRM_FTN:
                {
                    const SwFtnFrm *pFtnFrm =
                        static_cast < const SwFtnFrm * >( pFrm );
                    sal_Bool bIsEndnote =
                        SwAccessibleFootnote::IsEndnote( pFtnFrm );
                    xAcc = new SwAccessibleFootnote( this, bIsEndnote,
                                (bIsEndnote ? nEndnote++ : nFootnote++),
                                  pFtnFrm );
                }
                break;
            }

            ASSERT( xAcc.is(), "unknown frame type" );
            if( xAcc.is() )
            {
                if( aIter != pMap->end() )
                {
                    (*aIter).second = xAcc;
                }
                else
                {
                    SwAccessibleContextMap_Impl::value_type aEntry( pFrm, xAcc );
                    pMap->insert( aEntry );
                }
            }
        }
    }

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
    vos::OGuard aGuard( aMutex );

    if( pMap )
    {
        SwAccessibleContextMap_Impl::iterator aIter =
            pMap->find( pFrm );
        if( aIter != pMap->end() )
        {
            pMap->erase( aIter );

            // Remove reference to old caret object
            Reference < XAccessible > xOldAcc( pMap->xCaretContext );
            if( xOldAcc.is() )
            {
                SwAccessibleContext *pOldAccImpl =
                    static_cast< SwAccessibleContext *>( xOldAcc.get() );
                ASSERT( pOldAccImpl->GetFrm(), "old caret context is disposed" );
                if( pOldAccImpl->GetFrm() == pFrm )
                {
                    xOldAcc.clear();    // get an empty ref
                    pMap->xCaretContext = xOldAcc;
                }
            }

            if( pMap->empty() )
            {
                delete pMap;
                pMap = 0;
            }
        }
    }
}

void SwAccessibleMap::DisposeFrm( const SwFrm *pFrm )
{
    if( pFrm->IsAccessibleFrm() )
    {
        vos::OGuard aGuard( aMutex );

        if( pMap )
        {
            SwAccessibleContextMap_Impl::iterator aIter = pMap->find( pFrm );
            if( aIter != pMap->end() )
            {
                Reference < XAccessible > xAcc = (*aIter).second;
                if( xAcc.is() )
                {
                    SwAccessibleContext *pAccImpl =
                        static_cast< SwAccessibleContext *>( xAcc.get() );
                    pAccImpl->Dispose();
                }
            }
        }
        if( pEvents )
        {
            vos::OGuard aGuard( aEventMutex );
            SwAccessibleEventMap_Impl::iterator aIter =
                pEventMap->find( pFrm );
            if( aIter != pEventMap->end() )
            {
                SwAccessibleEvent_Impl aEvent(
                            SwAccessibleEvent_Impl::DISPOSE, pFrm );
                AppendEvent( aEvent );
            }
        }
    }
}

void SwAccessibleMap::MoveFrm( const SwFrm *pFrm, const SwRect& rOldFrm )
{
    if( pFrm->IsAccessibleFrm() )
    {
        vos::OGuard aGuard( aMutex );

        if( pMap )
        {
            SwAccessibleContextMap_Impl::iterator aIter = pMap->find( pFrm );
            if( aIter != pMap->end() )
            {
                // If there is an accesible object already it is
                // notified directly.
                Reference < XAccessible > xAcc = (*aIter).second;
                if( xAcc.is() )
                {
                    SwAccessibleContext *pAccImpl =
                        static_cast< SwAccessibleContext *>( xAcc.get() );
                    if( GetShell()->ActionPend() )
                    {
                        SwAccessibleEvent_Impl aEvent(
                            SwAccessibleEvent_Impl::POS_CHANGED, pAccImpl,
                            pFrm );
                        AppendEvent( aEvent );
                    }
                    else
                    {
                        pAccImpl->PosChanged();
                    }
                }
            }
            else
            {
                // Otherwise we look if the parent is accessible.
                // If not, there is nothing to do.
                const SwLayoutFrm *pUpper = pFrm->GetUpper();
                while( pUpper && !pUpper->IsAccessibleFrm() )
                    pUpper = pUpper->GetUpper();

                if( pUpper )
                {
                    aIter = pMap->find( pUpper );
                    if( aIter != pMap->end() )
                    {
                        Reference < XAccessible > xAcc = (*aIter).second;
                        if( xAcc.is() )
                        {
                            SwAccessibleContext *pAccImpl =
                                static_cast< SwAccessibleContext *>(xAcc.get());
                            if( GetShell()->ActionPend() )
                            {
                                SwAccessibleEvent_Impl aEvent(
                                    SwAccessibleEvent_Impl::CHILD_POS_CHANGED,
                                    pAccImpl, pFrm, rOldFrm );
                                AppendEvent( aEvent );
                            }
                            else
                            {
                                pAccImpl->ChildPosChanged( pFrm, rOldFrm );
                            }
                        }
                    }
                }
            }
        }
    }
}

void SwAccessibleMap::InvalidateFrmContent( const SwFrm *pFrm )
{
    if( pFrm->IsAccessibleFrm() )
    {
        vos::OGuard aGuard( aMutex );

        if( pMap )
        {
            SwAccessibleContextMap_Impl::iterator aIter = pMap->find( pFrm );
            if( aIter != pMap->end() )
            {
                Reference < XAccessible > xAcc = (*aIter).second;
                if( xAcc.is() )
                {
                    SwAccessibleContext *pAccImpl =
                        static_cast< SwAccessibleContext *>( xAcc.get() );
                    if( GetShell()->ActionPend() )
                    {
                        SwAccessibleEvent_Impl aEvent(
                            SwAccessibleEvent_Impl::INVALID_CONTENT, pAccImpl,
                            pFrm );
                        AppendEvent( aEvent );
                    }
                    else
                    {
                        pAccImpl->InvalidateContent();
                    }
                }
            }
        }
    }
}

void SwAccessibleMap::InvalidateCaretPosition( const SwFrm *pFrm )
{
    ASSERT( pFrm->IsAccessibleFrm(),
            "Unnecessary call to InvalidateCaretPosition" );

    if( pFrm->IsAccessibleFrm() )
    {
        Reference < XAccessible > xOldAcc;
        Reference < XAccessible > xAcc;

        {
            vos::OGuard aGuard( aMutex );

            if( pMap )
            {
                xOldAcc = pMap->xCaretContext;
                pMap->xCaretContext = xAcc; // clear reference

                SwAccessibleContextMap_Impl::iterator aIter =
                    pMap->find( pFrm );
                if( aIter != pMap->end() )
                    xAcc = (*aIter).second;
            }
        }
        if( xOldAcc.is() && xOldAcc != xAcc )
        {
            SwAccessibleContext *pOldAccImpl =
                static_cast< SwAccessibleContext *>( xOldAcc.get() );
            ASSERT( pOldAccImpl->GetFrm(), "old caret context is disposed" );
            // If there is no frame, then the object is disposed already
            if( pOldAccImpl->GetFrm() )
            {
                if( GetShell()->ActionPend() )
                {
                    SwAccessibleEvent_Impl aEvent(
                        SwAccessibleEvent_Impl::CARET_OR_STATES, pOldAccImpl,
                        pOldAccImpl->GetFrm(), ACC_STATE_CARET );
                    AppendEvent( aEvent );
                }
                else
                {
                    pOldAccImpl->InvalidateCaretPos();
                }
            }
        }
        if( xAcc.is() )
        {
            SwAccessibleContext *pAccImpl =
                static_cast< SwAccessibleContext *>( xAcc.get() );
            if( GetShell()->ActionPend() )
            {
                SwAccessibleEvent_Impl aEvent(
                    SwAccessibleEvent_Impl::CARET_OR_STATES, pAccImpl,
                    pFrm, ACC_STATE_CARET );
                AppendEvent( aEvent );
            }
            else
            {
                pAccImpl->InvalidateCaretPos();
            }
        }
    }
}

void SwAccessibleMap::SetCaretContext(
        const ::vos::ORef < SwAccessibleContext >& rCaretContext )
{
    vos::OGuard aGuard( aMutex );
    if( pMap )
    {
        Reference < XAccessible > xAcc( rCaretContext.getBodyPtr() );
        pMap->xCaretContext = xAcc;
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
        pAccImpl->CheckStates( nStates );
    }
}

void SwAccessibleMap::FireEvents()
{
    vos::OGuard aGuard( aEventMutex );
    if( pEvents )
    {
        SwAccessibleEventList_Impl::iterator aIter = pEvents->begin();
        while( aIter != pEvents->end() )
        {
            ::vos::ORef < SwAccessibleContext > xAccImpl( (*aIter).GetContext() );
            if( xAccImpl.isValid() )
            {
                switch( (*aIter).GetType() )
                {
                case SwAccessibleEvent_Impl::INVALID_CONTENT:
                    xAccImpl->InvalidateContent();
                    break;
                case SwAccessibleEvent_Impl::POS_CHANGED:
                    xAccImpl->PosChanged();
                    break;
                case SwAccessibleEvent_Impl::CHILD_POS_CHANGED:
                    xAccImpl->ChildPosChanged( (*aIter).GetFrm(),
                                               (*aIter).GetOldFrm() );
                    break;
                case SwAccessibleEvent_Impl::DISPOSE:
                    ASSERT( xAccImpl.isValid(),
                            "dispose event has been stored" );
                    break;
                }
                if( SwAccessibleEvent_Impl::DISPOSE != (*aIter).GetType() )
                {
                    if( (*aIter).IsUpdateCaretPos() )
                        xAccImpl->InvalidateCaretPos();
                    if( (*aIter).IsCheckStates() )
                        xAccImpl->CheckStates( (*aIter).GetStates() );
                }
            }

            aIter++;
        }
    }

    delete pEventMap;
    pEventMap = 0;

    delete pEvents;
    pEvents = 0;
}
