/*************************************************************************
 *
 *  $RCSfile: msgpool.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:29 $
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

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _RSCSFX_HXX //autogen
#include <rsc/rscsfx.hxx>
#endif
#pragma hdrstop

#include "msgpool.hxx"
#include "minarray.hxx"
#include "msg.hxx"
#include "app.hxx"
#include "objface.hxx"
#include "idpool.hxx"
#include "sfxtypes.hxx"
#include "slotinfo.hxx"
#include "macrconf.hxx"
#include "sfxresid.hxx"
#include "arrdecl.hxx"

#include "sfx.hrc"

//====================================================================

struct SfxSIDRegistration_Impl
{
    String          _aGroup;
    String          _aName;
    USHORT          _nSID;
};

struct SfxSlotType_Impl
{
    USHORT  nId;
    TypeId  nType;

    SfxSlotType_Impl( USHORT nTheId, TypeId nTheType ):
        nId(nTheId), nType(nTheType)
    {}
};

DECL_2BYTEARRAY(SfxSlotGroupArr_Impl, USHORT, 6, 4);
DECL_PTRARRAY(SfxInterfaceArr_Impl, SfxInterface*, 6, 3);
DECL_PTRARRAY(SfxSlotTypeArr_Impl, SfxSlotType_Impl*, 8, 8);


//====================================================================

SfxSlotPool::SfxSlotPool( SfxSlotPool *pParent, ResMgr* pResManager )
 : _pParentPool( pParent )
 , _pIdPool(0)
 , _pTypes(0)
 , _pGroups(0)
 , _nCurGroup(0)
 , _pInterfaces(0)
 , _nCurInterface(0)
 , _nCurMsg(0)
 , _pResMgr( pResManager )
 , _pUnoSlots( 0 )
{
    if ( !_pResMgr )
        _pResMgr = Resource::GetResManager();
}

//====================================================================

SfxSlotPool::~SfxSlotPool()
{
    _pParentPool = 0;
    for ( SfxInterface *pIF = FirstInterface(); pIF; pIF = FirstInterface() )
        delete pIF;
    delete _pInterfaces;
    delete _pGroups;
    delete _pUnoSlots;
    if ( _pTypes )
    {
        for ( USHORT n =_pTypes->Count(); n--; )
            delete _pTypes->GetObject(n);
        delete _pTypes;
    }
    delete _pIdPool;
}

//====================================================================

// registers the availability of the Interface of functions

void SfxSlotPool::RegisterInterface( SfxInterface& rInterface )
{
    DBG_MEMTEST();

    // add to the list of SfxObjectInterface instances
    if ( _pInterfaces == 0 )
        _pInterfaces = new SfxInterfaceArr_Impl;
    _pInterfaces->Append(&rInterface);

    // bei einem (einzelnen) Null-Slot abbrechen (aus syntaktischen Gr"unden
    // enthalten interfaces immer mindestens einen Slot)
    if ( rInterface.Count() == 1 && !rInterface[0]->nSlotId )
        return;

    // possibly add Interface-id and group-ids of funcs to the list of groups
    if ( !_pGroups )
    {
        _pGroups = new SfxSlotGroupArr_Impl;

        if ( _pParentPool )
        {
            // Die Groups im parent Slotpool sind auch hier bekannt
            SfxSlotGroupArr_Impl& rGroups = *_pParentPool->_pGroups;
            for ( USHORT n=0; n<rGroups.Count(); n++ )
                _pGroups->Append( rGroups[n] );
        }
    }

    if ( !_pTypes )
        _pTypes = new SfxSlotTypeArr_Impl;
    for ( USHORT nFunc = 0; nFunc < rInterface.Count(); ++nFunc )
    {
        SfxSlot *pDef = rInterface[nFunc];
        if ( pDef->GetUnoName() )
        {
            if ( !_pUnoSlots )
                _pUnoSlots = new SfxSlotArr_Impl;
            _pUnoSlots->Insert( pDef, _pUnoSlots->Count() );
        }

        if ( pDef->GetGroupId() && /* pDef->GetGroupId() != GID_INTERN && */
             !_pGroups->Contains(pDef->GetGroupId()) )
        {
            if (pDef->GetGroupId() == GID_INTERN)
                _pGroups->Insert(0, pDef->GetGroupId());
            else
                _pGroups->Append(pDef->GetGroupId());
        }

        const TypeId &rTypeId = pDef->GetType()->Type();
        if ( rTypeId != TYPE(SfxVoidItem) && rTypeId != 0 )
        {
            USHORT nPos;
            for ( nPos = 0; nPos < _pTypes->Count(); ++nPos )
            {
                if ( _pTypes->GetObject(nPos)->nId == pDef->GetSlotId() )
                {
                    DBG_ASSERT( rTypeId == _pTypes->GetObject(nPos)->nType,
                                "same slot id with unequal item types" );
                }
                else if ( _pTypes->GetObject(nPos)->nId > pDef->GetSlotId() )
                    break;
            }
            if ( nPos >= _pTypes->Count() ||
                 _pTypes->GetObject(nPos)->nId > pDef->GetSlotId() )
                _pTypes->Append( new SfxSlotType_Impl( pDef->GetSlotId(), rTypeId ) );
        }
    }
}

//====================================================================

TypeId SfxSlotPool::GetSlotType( USHORT nId ) const
{
    for ( USHORT nPos = 0; nPos < _pTypes->Count(); ++nPos )
    {
        if ( _pTypes->GetObject(nPos)->nId == nId )
            return _pTypes->GetObject(nPos)->nType;
    }
    return _pParentPool ? _pParentPool->GetSlotType( nId ) : 0;
}

//====================================================================

// unregisters the availability of the Interface of functions

void SfxSlotPool::ReleaseInterface( SfxInterface& rInterface )
{
    DBG_MEMTEST();
    DBG_ASSERT( _pInterfaces, "releasing SfxInterface, but there are none" );
    // remove from the list of SfxInterface instances
    _pInterfaces->Remove(&rInterface);
}

//--------------------------------------------------------------------

// get the first SfxMessage for a special Id (e.g. for getting check-mode)

const SfxSlot* SfxSlotPool::GetSlot( USHORT nId )
{
    DBG_MEMTEST();
    DBG_ASSERT( _pInterfaces != 0, "no Interfaces registered" );

    // Zun"achst die eigenen Interfaces absuchen
    for ( USHORT nInterf = 0; nInterf < _pInterfaces->Count(); ++nInterf )
    {
        const SfxSlot *pDef =
            _pInterfaces->GetObject(nInterf)->GetSlot(nId);
        if ( pDef )
            return pDef;
    }

    // Dann beim eventuell vorhandenen parent versuchen
    return _pParentPool ? _pParentPool->GetSlot( nId ) : 0;
}

//--------------------------------------------------------------------

// returns the pool of unused ids for macros, menus etc.

IdPool& SfxSlotPool::GetIdPool()
{
    DBG_MEMTEST();
    DBG_ASSERT( _pInterfaces != 0 && _pInterfaces->Count(),
                "no Interfaces registered" );

    if ( !_pIdPool )
    {
        _pIdPool = new IdPool();

        // create a set of all registered function-ids
        BitSet aRegIds;
        for ( USHORT nInterface = 0; nInterface < _pInterfaces->Count(); ++nInterface )
        {
            SfxInterface* pInterface = (*_pInterfaces)[nInterface];
            for ( USHORT nFunc = 0; nFunc < pInterface->Count(); ++nFunc )
                aRegIds |= (*pInterface)[nFunc]->GetGroupId();
        }

        //lock all registered ids
        _pIdPool->Lock(aRegIds);
    }

    return *_pIdPool;
}


//--------------------------------------------------------------------

// stores the id persistent to a stream (expand macros-names)

void SfxSlotPool::StoreId( SvStream& rStream, USHORT nId )
{
    DBG_MEMTEST();
    DBG_ASSERT( _pInterfaces != 0, "no Interfaces registered" );

    rStream << nId;
    //! later: write 0 and macro-lib/name
}


//--------------------------------------------------------------------

// load a persistent id from a stream (assign id if a macro)

void SfxSlotPool::LoadId( SvStream& rStream, USHORT &rId )
{
    DBG_MEMTEST();
    DBG_ASSERT( _pInterfaces != 0, "no Interfaces registered" );

    rStream >> (unsigned short&) rId;
}


//--------------------------------------------------------------------

// skips to the next group

String SfxSlotPool::SeekGroup( USHORT nNo )
{
    DBG_MEMTEST();
    DBG_ASSERT( _pInterfaces != 0, "no Interfaces registered" );

    // if the group exists, use it
    if ( _pGroups && nNo < _pGroups->Count() )
    {
        _nCurGroup = nNo;
        if ( _pParentPool )
        {
            // Meistens stimmt die Reihenfolge der Ids "uberein
            USHORT nParentCount = _pParentPool->_pGroups->Count();
            if ( nNo < nParentCount && (*_pGroups)[nNo] == (*_pParentPool->_pGroups)[nNo] )
                _pParentPool->_nCurGroup = nNo;
            else
            {
                // Ansonsten mu\s gesucht werden
                // Wenn die Gruppe im parent pool nicht gefunden wird, wird
                // _nCurGroup au\serhalb des g"ultigen Bereiches gesetzt
                USHORT i;
                for ( i=1; i<nParentCount; i++ )
                    if ( (*_pGroups)[nNo] == (*_pParentPool->_pGroups)[i] )
                        break;
                _pParentPool->_nCurGroup = i;
            }
        }

        SfxResId aResId( (*_pGroups)[_nCurGroup] );
        aResId.SetRT(RSC_STRING);
        if ( !aResId.GetResMgr()->IsAvailable(aResId) )
        {
            DBG_ERROR( "GroupId-Name nicht im SFX definiert!" );
            return String();
        }

        return String( aResId );
    }

    return String();
}


//--------------------------------------------------------------------

USHORT SfxSlotPool::GetGroupCount()
{
    return _pGroups->Count();
}


//--------------------------------------------------------------------

// internal search loop

const SfxSlot* SfxSlotPool::SeekSlot( USHORT nStartInterface )
{
    DBG_MEMTEST();
    DBG_ASSERT( _pInterfaces != 0, "no Interfaces registered" );

    // Die Numerierung der interfaces startet beim parent pool
    USHORT nFirstInterface = _pParentPool ? _pParentPool->_pInterfaces->Count() : 0;

    // sind wir am Ende des Parent-Pools angekommen?
    if ( nStartInterface < nFirstInterface &&
         _pParentPool->_nCurGroup >= _pParentPool->_pGroups->Count() )
        nStartInterface = nFirstInterface;

    // liegt das Interface noch im Parent-Pool?
    if ( nStartInterface < nFirstInterface )
    {
        DBG_ASSERT( _pParentPool, "Kein parent pool!" );
        _nCurInterface = nStartInterface;
        return _pParentPool->SeekSlot( nStartInterface );
    }

    // find the first func-def with the current group id
    USHORT nCount = _pInterfaces->Count() + nFirstInterface;
    for ( _nCurInterface = nStartInterface;
            _nCurInterface < nCount;
          ++_nCurInterface )
    {
        SfxInterface* pInterface = (*_pInterfaces)[_nCurInterface-nFirstInterface];
        for ( _nCurMsg = 0;
              _nCurMsg < pInterface->Count();
              ++_nCurMsg )
        {
            const SfxSlot* pMsg = (*pInterface)[_nCurMsg];
            if ( pMsg->GetGroupId() == _pGroups->GetObject(_nCurGroup) )
                return pMsg;
        }
    }

    return 0;
}

//--------------------------------------------------------------------

// skips to the next func in the current group

const SfxSlot* SfxSlotPool::NextSlot()
{
    DBG_MEMTEST();
    DBG_ASSERT( _pInterfaces != 0, "no Interfaces registered" );

    // Die Numerierung der interfaces startet beim parent pool
    USHORT nFirstInterface = _pParentPool ? _pParentPool->_pInterfaces->Count() : 0;

    if ( _nCurInterface < nFirstInterface && _nCurGroup >= _pParentPool->_pGroups->Count() )
        _nCurInterface = nFirstInterface;

    if ( _nCurInterface < nFirstInterface )
    {
        DBG_ASSERT( _pParentPool, "Kein parent pool!" );
        const SfxSlot *pSlot = _pParentPool->NextSlot();
        _nCurInterface = _pParentPool->_nCurInterface;
        if ( pSlot )
            return pSlot;
        if ( _nCurInterface == nFirstInterface )
            // parent pool ist fertig
            return SeekSlot( nFirstInterface );
    }

    USHORT nInterface = _nCurInterface - nFirstInterface;
    // possibly we are already at the end
    if ( nInterface >= _pInterfaces->Count() )
        return 0;

    // look for further matching func-defs within the same Interface
    SfxInterface* pInterface = (*_pInterfaces)[nInterface];
    while ( ++_nCurMsg < pInterface->Count() )
    {
        SfxSlot* pMsg = (*pInterface)[_nCurMsg];
        if ( pMsg->GetGroupId() == _pGroups->GetObject(_nCurGroup) )
            return pMsg;
    }

    return SeekSlot(++_nCurInterface );
}


//--------------------------------------------------------------------

// SlotName erfragen, gfs. mit HilfeText

String SfxSlotPool::GetSlotName_Impl(USHORT nId, String *pHelpText) const
{
    // ist es eine Makro-SlotId?
    if ( SfxMacroConfig::IsMacroSlot( nId ) )
    {
        const SfxMacroInfo *pMacro = SFX_APP()->GetMacroConfig()->GetMacroInfo(nId);
        if ( pMacro )
            return pMacro->GetMacroName();
        else
            return String();
    }

    // Zun"achst den eigenen ResManager testen, dann evtl. den des parent pools
    ResId aResId( nId, _pResMgr );
    aResId.SetRT( RSC_SFX_SLOT_INFO );
    if( !Resource::GetResManager()->IsAvailable( aResId ) && _pParentPool )
        aResId.SetResMgr( _pParentPool->_pResMgr );
    if( Resource::GetResManager()->IsAvailable( aResId ) )
    {
        SfxSlotInfo aInfo(aResId);
        if(pHelpText)
            *pHelpText = aInfo.GetHelpText();
        return aInfo.GetName();
    }

#ifdef DBG_UTIL
    String aError(DEFINE_CONST_UNICODE("Slot ohne ConfigName: "));
    aError += String::CreateFromInt32( nId );
//  DBG_ERROR( aError );
    return aError;
#endif
    return String();
}


//--------------------------------------------------------------------

// SlotName erfragen, gfs. mit HilfeText

String SfxSlotPool::GetSlotHelpText_Impl(USHORT nId) const
{
    String aHelpText;
    GetSlotName_Impl( nId, &aHelpText );
    return aHelpText;
}

//--------------------------------------------------------------------
#ifdef STARBASIC


// assignes a request-id for a specific macro

USHORT SfxSlotPool::RegisterMacro( sbToken aToken )
{
    DBG_MEMTEST();
    DBG_ASSERT( pInterfaces != 0, "no Interfaces registered" );
    DBG_ASSERT( TRUE, "not implemented - call MI" );
    return 0;
}


//--------------------------------------------------------------------

// frees the request-id of a specific macro

void SfxSlotPool::ReleaseMacro( sbToken aToken )
{
    DBG_MEMTEST();
    DBG_ASSERT( pInterfaces != 0, "no Interfaces registered" );
    DBG_ASSERT( TRUE, "not implemented - call MI" );
}

#endif

//--------------------------------------------------------------------

SfxInterface* SfxSlotPool::FirstInterface()
{
    _nCurInterface = 0;
    if ( !_pInterfaces || !_pInterfaces->Count() )
        return 0;
    return _pParentPool ? _pParentPool->FirstInterface() : (*_pInterfaces)[0];
}


//--------------------------------------------------------------------

SfxInterface* SfxSlotPool::NextInterface()
{
    _nCurInterface++;
    USHORT nFirstInterface = _pParentPool ? _pParentPool->_pInterfaces->Count() : 0;
    if ( _nCurInterface < nFirstInterface )
        return (*_pParentPool->_pInterfaces)[_nCurInterface];
    USHORT nInterface = _nCurInterface - nFirstInterface;
    return nInterface < _pInterfaces->Count() ? (*_pInterfaces)[nInterface] : 0;
}

//--------------------------------------------------------------------

USHORT SfxSlotPool::RegisterSID( const String &rGroup, const String &rName )
{
    return 0;
}

//--------------------------------------------------------------------

void SfxSlotPool::ReleaseSID( const String &rGroup, const String &rName )
{
}

const SfxSlot* SfxSlotPool::GetUnoSlot( USHORT nId )
{
    const SfxSlot *pSlot = NULL;
    if ( _pParentPool )
        pSlot = _pParentPool->GetUnoSlot( nId );

    if ( !pSlot && _pUnoSlots )
    {
        USHORT nCount = _pUnoSlots->Count();
        for ( USHORT n=0; n<nCount; n++ )
        {
            if ( (*_pUnoSlots)[n]->GetSlotId() == nId )
            {
                pSlot = (*_pUnoSlots)[n];
                break;
            }
        }
    }

    return pSlot;
}

const SfxSlot* SfxSlotPool::GetUnoSlot( const String& rName )
{
    const SfxSlot *pSlot = NULL;
    if ( _pParentPool )
        pSlot = _pParentPool->GetUnoSlot( rName );

    if ( !pSlot && _pUnoSlots )
    {
        USHORT nCount = _pUnoSlots->Count();
        for ( USHORT n=0; n<nCount; n++ )
        {
            if ( rName.EqualsAscii((*_pUnoSlots)[n]->GetUnoName()) )
            {
                pSlot = (*_pUnoSlots)[n];
                break;
            }
        }
    }

    return pSlot;
}



