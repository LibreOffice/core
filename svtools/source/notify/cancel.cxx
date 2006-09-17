/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cancel.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 15:18:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"
#define _SFX_CANCEL_CXX
#include "cancel.hxx"

#include <vos/mutex.hxx>
#include <tools/debug.hxx>

#include "smplhint.hxx"
#include "cnclhint.hxx"

#ifndef INCLUDED_RTL_INSTANCE_HXX
#include <rtl/instance.hxx>
#endif

namespace { struct lclMutex : public rtl::Static< ::vos::OMutex, lclMutex >{}; }

//=========================================================================

SfxCancelManager::SfxCancelManager( SfxCancelManager *pParent )
:   _pParent( pParent )
{
}

//-------------------------------------------------------------------------

SfxCancelManager::~SfxCancelManager()
{
    DBG_ASSERT( _pParent || !_aJobs.Count(), "deleting SfxCancelManager in use" );
    for ( USHORT n = _aJobs.Count(); n--; )
        _aJobs.GetObject(n)->SetManager( _pParent );
}

//-------------------------------------------------------------------------

BOOL SfxCancelManager::CanCancel() const

/*  [Beschreibung]

    Liefert TRUE wenn an diesem CancelManager oder an einem Parent
    ein Job l"auft.
*/

{
    ::vos::OGuard aGuard( lclMutex::get() );
    return _aJobs.Count() > 0 || ( _pParent && _pParent->CanCancel() );
}

//-------------------------------------------------------------------------

void SfxCancelManager::Cancel( BOOL bDeep )

/*  [Beschreibung]

    Diese Methode markiert alle angemeldeten <SfxCancellable>-Instanzen
    als suspendiert.
*/

{
    ::vos::OGuard aGuard( lclMutex::get() );
    SfxCancelManagerWeak xWeak( this );
    for ( USHORT n = _aJobs.Count(); n-- && xWeak.Is(); )
        if ( n < _aJobs.Count() )
            _aJobs.GetObject(n)->Cancel();
    if ( xWeak.Is() && _pParent )
        _pParent->Cancel( bDeep );
}

//-------------------------------------------------------------------------

void SfxCancelManager::InsertCancellable( SfxCancellable *pJob )

/*  [Beschreibung]

    Diese interne Methode tr"agt 'pJob' in die Liste der unterbrechbaren
    Jobs ein und Broadcastet dies. Jeder <SfxCancellable> darf nur
    maximal einmal angemeldet sein, dies geschiet in seinem Ctor.
*/

{
#ifdef GPF_ON_EMPTY_TITLE
    if ( !pJob->GetTitle() )
    {
        DBG_ERROR( "SfxCancellable: empty titles not allowed (Vermummungsverbot)" )
        *(int*)0 = 0;
    }
#endif

    ::vos::OClearableGuard aGuard( lclMutex::get() );
    _aJobs.C40_INSERT( SfxCancellable, pJob, _aJobs.Count() );

    aGuard.clear();
    Broadcast( SfxSimpleHint( SFX_HINT_CANCELLABLE ) );
}

//-------------------------------------------------------------------------


void SfxCancelManager::RemoveCancellable( SfxCancellable *pJob )

/*  [Beschreibung]

    Diese interne Methode tr"agt 'pJob' aus die Liste der unterbrechbaren
    Jobs aus und Broadcastet dies. Dieser Aufruf mu\s paarig nach einem
    <InsertCancellable> erfolgen und wird im Dtor des <SfxCancellable>
    ausgel"ost.
*/

{
    ::vos::OClearableGuard aGuard( lclMutex::get() );
    const SfxCancellable *pTmp = pJob;
    USHORT nPos = _aJobs.GetPos( pTmp );
    if ( nPos != 0xFFFF )
    {
        _aJobs.Remove( nPos , 1 );
        aGuard.clear();
        Broadcast( SfxSimpleHint( SFX_HINT_CANCELLABLE ) );
        Broadcast( SfxCancelHint( pJob, SFXCANCELHINT_REMOVED ) );
    }
}

//-------------------------------------------------------------------------

SfxCancellable::~SfxCancellable()
{
    SfxCancelManager* pMgr = _pMgr;
    if ( pMgr )
        pMgr->RemoveCancellable( this );
}

//-------------------------------------------------------------------------

void SfxCancellable::Cancel()

/*  [Description]

    This virtual function is called when the user hits the cancel-button.
    If you overload it, you can stop your activities. Please always call
    'SfxCancellable::Cancel()'.
*/

{
#ifdef GFP_ON_NO_CANCEL
    if ( _bCancelled < 5 )
        ++_bCancelled;
    else
    {
        delete this;
    }
#else
    _bCancelled = TRUE;
#endif
}

//-------------------------------------------------------------------------

void SfxCancellable::SetManager( SfxCancelManager *pMgr )
{
    SfxCancelManager* pTmp = _pMgr;
    if ( pTmp )
        pTmp->RemoveCancellable( this );
    _pMgr = pMgr;
    if ( pMgr )
        pMgr->InsertCancellable( this );
}

//-------------------------------------------------------------------------

TYPEINIT1(SfxCancelHint, SfxHint);

SfxCancelHint::SfxCancelHint( SfxCancellable* pJob, USHORT _nAction )
{
    pCancellable = pJob;
    nAction = _nAction;
}


