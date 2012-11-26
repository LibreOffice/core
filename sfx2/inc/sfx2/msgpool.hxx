/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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

    const std::type_info& GetSlotType( sal_uInt16 nSlotId ) const;
};

//--------------------------------------------------------------------

// seeks to the first func in the current group

inline const SfxSlot* SfxSlotPool::FirstSlot()
{
    return SeekSlot(0);
}

#define SFX_SLOTPOOL() SfxSlotPool::GetSlotPool()

#endif

