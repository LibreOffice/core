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

#include <sal/log.hxx>
#include <osl/diagnose.h>

// due to pSlotPool
#include <appdata.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/module.hxx>

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
    // swap out _vInterfaces because ~SfxInterface() might call ReleaseInterface()
    std::vector<SfxInterface*> tmpInterfaces;
    tmpInterfaces.swap(_vInterfaces);
    for ( SfxInterface *pIF : tmpInterfaces )
        delete pIF;
}

namespace
{
    const char* getGidResId(SfxGroupId nId)
    {
        if (nId == SfxGroupId::Intern)
            return STR_GID_INTERN;
        else if (nId == SfxGroupId::Application)
            return STR_GID_APPLICATION;
        else if (nId == SfxGroupId::View)
            return STR_GID_VIEW;
        else if (nId == SfxGroupId::Document)
            return STR_GID_DOCUMENT;
        else if (nId == SfxGroupId::Edit)
            return STR_GID_EDIT;
        else if (nId == SfxGroupId::Macro)
            return STR_GID_MACRO;
        else if (nId == SfxGroupId::Options)
            return STR_GID_OPTIONS;
        else if (nId == SfxGroupId::Math)
            return STR_GID_MATH;
        else if (nId == SfxGroupId::Navigator)
            return STR_GID_NAVIGATOR;
        else if (nId == SfxGroupId::Insert)
            return STR_GID_INSERT;
        else if (nId == SfxGroupId::Format)
            return STR_GID_FORMAT;
        else if (nId == SfxGroupId::Template)
            return STR_GID_TEMPLATE;
        else if (nId == SfxGroupId::Text)
            return STR_GID_TEXT;
        else if (nId == SfxGroupId::Frame)
            return STR_GID_FRAME;
        else if (nId == SfxGroupId::Graphic)
            return STR_GID_GRAPHIC;
        else if (nId == SfxGroupId::Table)
            return STR_GID_TABLE;
        else if (nId == SfxGroupId::Enumeration)
            return STR_GID_ENUMERATION;
        else if (nId == SfxGroupId::Data)
            return STR_GID_DATA;
        else if (nId == SfxGroupId::Special)
            return STR_GID_SPECIAL;
        else if (nId == SfxGroupId::Image)
            return STR_GID_IMAGE;
        else if (nId == SfxGroupId::Chart)
            return STR_GID_CHART;
        else if (nId == SfxGroupId::Explorer)
            return STR_GID_EXPLORER;
        else if (nId == SfxGroupId::Connector)
            return STR_GID_CONNECTOR;
        else if (nId == SfxGroupId::Modify)
            return STR_GID_MODIFY;
        else if (nId == SfxGroupId::Drawing)
            return STR_GID_DRAWING;
        else if (nId == SfxGroupId::Controls)
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
        if ( rDef.GetGroupId() != SfxGroupId::NONE &&
             std::find(_vGroups.begin(), _vGroups.end(), rDef.GetGroupId()) == _vGroups.end() )
        {
            if (rDef.GetGroupId() == SfxGroupId::Intern)
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
