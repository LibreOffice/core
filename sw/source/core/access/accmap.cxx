 /*************************************************************************
 *
 *  $RCSfile: accmap.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: mib $ $Date: 2002-03-08 13:26:29 $
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
#ifndef _DOC_HXX
#include <doc.hxx>
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
};

struct SwAccessibleEvent_Impl
{
public:
    enum EventType { INVALID_CONTENT, POS_CHANGED, CHILD_POS_CHANGED, DISPOSE };

private:
    EventType   eType;                  // The event type
    WeakReference < XAccessible > xAcc; // The object that fires the event
    SwRect      aOldFrm;                // the old bounds for CHILD_POS_CHANGED
    const SwFrm *pFrm;                  // the child for CHILD_POS_CHANGED and
                                        // the same as xAcc for any other
                                        // event type

public:
    SwAccessibleEvent_Impl( EventType eT, SwAccessibleContext *pA ) :
        eType( eT ), xAcc( pA ), pFrm( pA->GetFrm() ) {}
    SwAccessibleEvent_Impl( EventType eT, const SwFrm *pF ) :
        eType( eT ), pFrm( pF ) {}
    SwAccessibleEvent_Impl( EventType eT, SwAccessibleContext *pA,
                            const SwFrm *pF, const SwRect& rR ) :
        eType( eT ), xAcc( pA ), pFrm( pF ), aOldFrm( rR ) {}

    inline EventType    GetType() const { return eType; }
    inline ::vos::ORef < SwAccessibleContext > GetContext() const;
    inline const SwRect& GetOldFrm() const { return aOldFrm; }
    inline const SwFrm *GetFrm() const { return pFrm; }
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
        const SwRootFrm *pRootFrm = GetShell()->GetDoc()->GetRootFrm();
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
        case SwAccessibleEvent_Impl::INVALID_CONTENT:
            // All events include a INVALID_CONTENT, so the only action that
            // needs to be done is to put the event to the back. That's done
            // automatically.
            ASSERT( aEvent.GetType()!=SwAccessibleEvent_Impl::CHILD_POS_CHANGED,
                    "invalid event combination" );
            ASSERT( aEvent.GetType() != SwAccessibleEvent_Impl::DISPOSE,
                    "dispose events should not be stored" );
            break;
        case SwAccessibleEvent_Impl::POS_CHANGED:
            // If the the old event is not a DISPOSE event, the new event
            // includes the old one.
            ASSERT( aEvent.GetType()!=SwAccessibleEvent_Impl::CHILD_POS_CHANGED,
                    "invalid event combination" );
            ASSERT( aEvent.GetType() != SwAccessibleEvent_Impl::DISPOSE,
                    "dispose events should not be stored" );
            if( aEvent.GetType() != SwAccessibleEvent_Impl::DISPOSE )
                aEvent = rEvent;
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
        pMap = new SwAccessibleContextMap_Impl;
    const SwRootFrm *pRootFrm = GetShell()->GetDoc()->GetRootFrm();
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

void SwAccessibleMap::RemoveContext( SwAccessibleContext *pAcc )
{
    vos::OGuard aGuard( aMutex );

    if( pMap )
    {
        SwAccessibleContextMap_Impl::iterator aIter =
            pMap->find( pAcc->GetFrm() );
        if( aIter != pMap->end() )
        {
            pMap->erase( aIter );
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
                            SwAccessibleEvent_Impl::POS_CHANGED, pAccImpl );
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
                            SwAccessibleEvent_Impl::INVALID_CONTENT, pAccImpl );
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
            }

            aIter++;
        }
    }

    delete pEventMap;
    pEventMap = 0;

    delete pEvents;
    pEvents = 0;
}
