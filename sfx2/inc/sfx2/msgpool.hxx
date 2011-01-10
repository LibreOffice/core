/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SFXMSGPOOL_HXX
#define _SFXMSGPOOL_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"

#include <sfx2/msg.hxx>
#include <tools/string.hxx>
#include <svl/svarray.hxx>

class SfxInterface;
class SfxSlot;
class SfxInterfaceArr_Impl;
class SfxSlotGroupArr_Impl;
class SfxSlotTypeArr_Impl;
class SfxSlotArr_Impl;

//=========================================================================

class SFX2_DLLPUBLIC SfxSlotPool
{
    SfxSlotGroupArr_Impl*       _pGroups;
    SfxSlotTypeArr_Impl*        _pTypes;
    SfxSlotPool*                _pParentPool;
    ResMgr*                     _pResMgr;
    SfxInterfaceArr_Impl*       _pInterfaces;
    sal_uInt16                      _nCurGroup;
    sal_uInt16                      _nCurInterface;
    sal_uInt16                      _nCurMsg;
    SfxSlotArr_Impl*            _pUnoSlots;

private:
    const SfxSlot* SeekSlot( sal_uInt16 nObject );

public:
                        SfxSlotPool( SfxSlotPool* pParent=0, ResMgr* pMgr=0);
                        ~SfxSlotPool();

    void                RegisterInterface( SfxInterface& rFace );
    void                ReleaseInterface( SfxInterface& rFace );
    SfxInterface*       FirstInterface();
    SfxInterface*       NextInterface();

    static SfxSlotPool& GetSlotPool( SfxViewFrame *pFrame=NULL );

    sal_uInt16              GetGroupCount();
    String              SeekGroup( sal_uInt16 nNo );
    const SfxSlot*      FirstSlot();
    const SfxSlot*      NextSlot();
    const SfxSlot*      GetSlot( sal_uInt16 nId );
    const SfxSlot*      GetUnoSlot( sal_uInt16 nId );
    const SfxSlot*      GetUnoSlot( const String& rUnoName );
    TypeId              GetSlotType( sal_uInt16 nSlotId ) const;
};

//--------------------------------------------------------------------

// seeks to the first func in the current group

inline const SfxSlot* SfxSlotPool::FirstSlot()
{
    return SeekSlot(0);
}

#define SFX_SLOTPOOL() SfxSlotPool::GetSlotPool()

#endif

