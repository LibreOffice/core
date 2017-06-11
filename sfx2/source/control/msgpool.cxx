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

#include <tools/stream.hxx>
#include <rsc/rscsfx.hxx>

// due to pSlotPool
#include "appdata.hxx"
#include <sfx2/msgpool.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objface.hxx>
#include "sfxtypes.hxx"
#include <sfx2/sfxresid.hxx>
#include "arrdecl.hxx"
#include <sfx2/module.hxx>

#include <sfx2/sfx.hrc>
#include <sfx2/strings.hrc>

SfxSlotPool::SfxSlotPool(SfxSlotPool *pParent)
 : _pParentPool( pParent )
 , _nCurGroup(0)
 , _nCurInterface(0)
 , _nCurMsg(0)
{
}

SfxSlotPool::~SfxSlotPool()
{
    _pParentPool = nullptr;
    for ( SfxInterface *pIF = FirstInterface(); pIF; pIF = FirstInterface() )
        delete pIF;
}

namespace
{
    const char* getGidResId(SfxGroupId nId)
    {
        if (nId == GID_INTERN)
            return STR_GID_INTERN;
        else if (nId == GID_APPLICATION)
            return STR_GID_APPLICATION;
        else if (nId == GID_VIEW)
            return STR_GID_VIEW;
        else if (nId == GID_DOCUMENT)
            return STR_GID_DOCUMENT;
        else if (nId == GID_EDIT)
            return STR_GID_EDIT;
        else if (nId == GID_MACRO)
            return STR_GID_MACRO;
        else if (nId == GID_OPTIONS)
            return STR_GID_OPTIONS;
        else if (nId == GID_MATH)
            return STR_GID_MATH;
        else if (nId == GID_NAVIGATOR)
            return STR_GID_NAVIGATOR;
        else if (nId == GID_INSERT)
            return STR_GID_INSERT;
        else if (nId == GID_FORMAT)
            return STR_GID_FORMAT;
        else if (nId == GID_TEMPLATE)
            return STR_GID_TEMPLATE;
        else if (nId == GID_TEXT)
            return STR_GID_TEXT;
        else if (nId == GID_FRAME)
            return STR_GID_FRAME;
        else if (nId == GID_GRAPHIC)
            return STR_GID_GRAPHIC;
        else if (nId == GID_TABLE)
            return STR_GID_TABLE;
        else if (nId == GID_ENUMERATION)
            return STR_GID_ENUMERATION;
        else if (nId == GID_DATA)
            return STR_GID_DATA;
        else if (nId == GID_SPECIAL)
            return STR_GID_SPECIAL;
        else if (nId == GID_IMAGE)
            return STR_GID_IMAGE;
        else if (nId == GID_CHART)
            return STR_GID_CHART;
        else if (nId == GID_EXPLORER)
            return STR_GID_EXPLORER;
        else if (nId == GID_CONNECTOR)
            return STR_GID_CONNECTOR;
        else if (nId == GID_MODIFY)
            return STR_GID_MODIFY;
        else if (nId == GID_DRAWING)
            return STR_GID_DRAWING;
        else if (nId == GID_CONTROLS)
            return STR_GID_CONTROLS;
        return nullptr;
    }
}

// registers the availability of the Interface of functions

void SfxSlotPool::RegisterInterface( SfxInterface& rInterface )
{
    // add to the list of SfxObjectInterface instances
    _vInterfaces.push_back(&rInterface);

    // Stop at a (single) Null-slot (for syntactic reasons the interfaces
    // always contain at least one slot)
    if ( rInterface.Count() != 0 && !rInterface.pSlots[0].nSlotId )
        return;

    // possibly add Interface-id and group-ids of funcs to the list of groups
    if ( _pParentPool )
    {
        // The Groups in parent Slotpool are also known here
        _vGroups.insert( _vGroups.end(), _pParentPool->_vGroups.begin(), _pParentPool->_vGroups.end() );
    }

    for ( size_t nFunc = 0; nFunc < rInterface.Count(); ++nFunc )
    {
        SfxSlot &rDef = rInterface.pSlots[nFunc];
        if ( rDef.GetGroupId() && /* rDef.GetGroupId() != GID_INTERN && */
             std::find(_vGroups.begin(), _vGroups.end(), rDef.GetGroupId()) == _vGroups.end() )
        {
            if (rDef.GetGroupId() == GID_INTERN)
                _vGroups.insert(_vGroups.begin(), rDef.GetGroupId());
            else
                _vGroups.push_back(rDef.GetGroupId());
        }
    }
}


const std::type_info* SfxSlotPool::GetSlotType( sal_uInt16 nId ) const
{
    const SfxSlot* pSlot = GetSlot( nId );
    return pSlot ? pSlot->GetType()->Type() : nullptr;
}


// unregisters the availability of the Interface of functions

void SfxSlotPool::ReleaseInterface( SfxInterface& rInterface )
{
    // remove from the list of SfxInterface instances
    auto i = std::find(_vInterfaces.begin(), _vInterfaces.end(), &rInterface);
    if(i != _vInterfaces.end())
        _vInterfaces.erase(i);
}

// get the first SfxMessage for a special Id (e.g. for getting check-mode)

const SfxSlot* SfxSlotPool::GetSlot( sal_uInt16 nId ) const
{
    // First, search their own interfaces
    for (SfxInterface* _pInterface : _vInterfaces)
    {
        const SfxSlot *pDef = _pInterface->GetSlot(nId);
        if ( pDef )
            return pDef;
    }

    // Then try any of the possible existing parent
    return _pParentPool ? _pParentPool->GetSlot( nId ) : nullptr;
}


// skips to the next group

OUString SfxSlotPool::SeekGroup( sal_uInt16 nNo )
{
    // if the group exists, use it
    if (  nNo < _vGroups.size() )
    {
        _nCurGroup = nNo;
        if ( _pParentPool )
        {
            // In most cases, the order of the IDs agree
            sal_uInt16 nParentCount = _pParentPool->_vGroups.size();
            if ( nNo < nParentCount && _vGroups[nNo] == _pParentPool->_vGroups[nNo] )
                _pParentPool->_nCurGroup = nNo;
            else
            {
                // Otherwise search. If the group is not found in the parent
                // pool, _nCurGroup is set outside the valid range
                sal_uInt16 i;
                for ( i=1; i<nParentCount; i++ )
                    if ( _vGroups[nNo] == _pParentPool->_vGroups[i] )
                        break;
                _pParentPool->_nCurGroup = i;
            }
        }

        const char* pResId = getGidResId(_vGroups[_nCurGroup]);
        if (!pResId)
        {
            OSL_FAIL( "GroupId-Name not defined in SFX!" );
            return OUString();
        }

        return SfxResId(pResId);
    }

    return OUString();
}


sal_uInt16 SfxSlotPool::GetGroupCount() const
{
    return _vGroups.size();
}


// internal search loop

const SfxSlot* SfxSlotPool::SeekSlot( sal_uInt16 nStartInterface )
{
    // The numbering starts at the interfaces of the parent pool
    sal_uInt16 nFirstInterface = _pParentPool ? _pParentPool->_vInterfaces.size() : 0;

    // have reached the end of the Parent-Pools?
    if ( nStartInterface < nFirstInterface &&
         _pParentPool->_nCurGroup >= _pParentPool->_vGroups.size() )
        nStartInterface = nFirstInterface;

    // Is the Interface still in the Parent-Pool?
    if ( nStartInterface < nFirstInterface )
    {
        SAL_WARN_IF(!_pParentPool, "sfx.control", "No parent pool!");
        _nCurInterface = nStartInterface;
        return _pParentPool->SeekSlot( nStartInterface );
    }

    // find the first func-def with the current group id
    sal_uInt16 nCount = _vInterfaces.size() + nFirstInterface;
    for ( _nCurInterface = nStartInterface;
            _nCurInterface < nCount;
          ++_nCurInterface )
    {
        SfxInterface* pInterface = _vInterfaces[_nCurInterface-nFirstInterface];
        for ( _nCurMsg = 0;
              _nCurMsg < pInterface->Count();
              ++_nCurMsg )
        {
            const SfxSlot& rMsg = pInterface->pSlots[_nCurMsg];
            if (rMsg.GetGroupId() == _vGroups.at(_nCurGroup))
                return &rMsg;
        }
    }

    return nullptr;
}


// skips to the next func in the current group

const SfxSlot* SfxSlotPool::NextSlot()
{
    // The numbering starts at the interfaces of the parent pool
    sal_uInt16 nFirstInterface = _pParentPool ? _pParentPool->_vInterfaces.size() : 0;

    if ( _nCurInterface < nFirstInterface && _nCurGroup >= _pParentPool->_vGroups.size() )
        _nCurInterface = nFirstInterface;

    if ( _nCurInterface < nFirstInterface )
    {
        SAL_WARN_IF(!_pParentPool, "sfx.control", "No parent pool!");
        const SfxSlot *pSlot = _pParentPool->NextSlot();
        _nCurInterface = _pParentPool->_nCurInterface;
        if ( pSlot )
            return pSlot;
        if ( _nCurInterface == nFirstInterface )
            // parent pool is ready
            return SeekSlot( nFirstInterface );
    }

    sal_uInt16 nInterface = _nCurInterface - nFirstInterface;
    // possibly we are already at the end
    if ( nInterface >= _vInterfaces.size() )
        return nullptr;

    // look for further matching func-defs within the same Interface
    SfxInterface* pInterface = _vInterfaces[nInterface];
    while ( ++_nCurMsg < pInterface->Count() )
    {
        SfxSlot& rMsg = pInterface->pSlots[_nCurMsg];
        if (rMsg.GetGroupId() == _vGroups.at(_nCurGroup))
            return &rMsg;
    }

    return SeekSlot(++_nCurInterface );
}


// Query SlotName with help text


SfxInterface* SfxSlotPool::FirstInterface()
{
    _nCurInterface = 0;
    if ( _vInterfaces.empty() )
        return nullptr;
    return _pParentPool ? _pParentPool->FirstInterface() : _vInterfaces[0];
}


const SfxSlot* SfxSlotPool::GetUnoSlot( const OUString& rName ) const
{
    const SfxSlot *pSlot = nullptr;
    for (auto const & nInterface: _vInterfaces)
    {
        pSlot = nInterface->GetSlot( rName );
        if ( pSlot )
            break;
    }

    if ( !pSlot && _pParentPool )
        pSlot = _pParentPool->GetUnoSlot( rName );

    return pSlot;
}

SfxSlotPool& SfxSlotPool::GetSlotPool( SfxViewFrame *pFrame )
{
    SfxModule *pMod = SfxModule::GetActiveModule( pFrame );
    if ( pMod && pMod->GetSlotPool() )
        return *pMod->GetSlotPool();
    else
        return *SfxGetpApp()->Get_Impl()->pSlotPool;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
