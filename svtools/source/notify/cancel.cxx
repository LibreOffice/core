/*************************************************************************
 *
 *  $RCSfile: cancel.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:59:03 $
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

#define _SFX_CANCEL_CXX
#include <tools/debug.hxx>
#include "cancel.hxx"
#include "smplhint.hxx"
#include "cnclhint.hxx"

#ifndef _SV_SOUND_HXX //autogen
#include <vcl/sound.hxx>
#endif

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
    return _aJobs.Count() > 0 || ( _pParent && _pParent->CanCancel() );
}

//-------------------------------------------------------------------------

void SfxCancelManager::Cancel( BOOL bDeep )

/*  [Beschreibung]

    Diese Methode markiert alle angemeldeten <SfxCancellable>-Instanzen
    als suspendiert.
*/

{
    SfxCancelManagerWeak xWeak( this );
    for ( USHORT n = _aJobs.Count(); n-- && xWeak.Is(); )
        if ( n < _aJobs.Count() )
            _aJobs.GetObject(n)->Cancel();
    if ( xWeak.Is() && _pParent )
        _pParent->Cancel( bDeep );
}

//-------------------------------------------------------------------------

void SfxCancelManager::SFX_INSERT_CANCELLABLE( SfxCancellable *pJob )

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

    _aJobs.C40_INSERT( SfxCancellable, pJob, _aJobs.Count() );
    Broadcast( SfxSimpleHint( SFX_HINT_CANCELLABLE ) );
}

//-------------------------------------------------------------------------


void SfxCancelManager::SFX_REMOVE_CANCELLABLE( SfxCancellable *pJob )

/*  [Beschreibung]

    Diese interne Methode tr"agt 'pJob' aus die Liste der unterbrechbaren
    Jobs aus und Broadcastet dies. Dieser Aufruf mu\s paarig nach einem
    <InsertCancellable> erfolgen und wird im Dtor des <SfxCancellable>
    ausgel"ost.
*/

{
    const SfxCancellable *pTmp = pJob;
    _aJobs.Remove( _aJobs.GetPos( pTmp ), 1 );
    Broadcast( SfxSimpleHint( SFX_HINT_CANCELLABLE ) );
    Broadcast( SfxCancelHint( pJob, SFXCANCELHINT_REMOVED ) );
}

//-------------------------------------------------------------------------

SfxCancellable::~SfxCancellable()
{
    if ( _pMgr )
        _pMgr->SFX_REMOVE_CANCELLABLE( this );
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
        Sound::Beep();
        delete this;
    }
#else
    _bCancelled = TRUE;
#endif
}

//-------------------------------------------------------------------------

void SfxCancellable::SetManager( SfxCancelManager *pMgr )
{
    if ( _pMgr )
        _pMgr->SFX_REMOVE_CANCELLABLE( this );
    _pMgr = pMgr;
    if ( _pMgr )
        _pMgr->SFX_INSERT_CANCELLABLE( this );
}

//-------------------------------------------------------------------------

TYPEINIT1(SfxCancelHint, SfxHint);

SfxCancelHint::SfxCancelHint( SfxCancellable* pJob, USHORT _nAction )
{
    pCancellable = pJob;
    nAction = _nAction;
}


