/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: msgpool.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 21:24:47 $
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
#ifndef _SFXMSGPOOL_HXX
#define _SFXMSGPOOL_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_SFX2_DLLAPI_H
#include "sfx2/dllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif

#include <sfx2/msg.hxx>

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _SVARRAY_HXX
#include <svtools/svarray.hxx>
#endif

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
    USHORT                      _nCurGroup;
    USHORT                      _nCurInterface;
    USHORT                      _nCurMsg;
    SfxSlotArr_Impl*            _pUnoSlots;

private:
    const SfxSlot* SeekSlot( USHORT nObject );

public:
                        SfxSlotPool( SfxSlotPool* pParent=0, ResMgr* pMgr=0);
                        ~SfxSlotPool();

    void                RegisterInterface( SfxInterface& rFace );
    void                ReleaseInterface( SfxInterface& rFace );
    SfxInterface*       FirstInterface();
    SfxInterface*       NextInterface();

    static SfxSlotPool& GetSlotPool( SfxViewFrame *pFrame=NULL );

    USHORT              GetGroupCount();
    String              SeekGroup( USHORT nNo );
    const SfxSlot*      FirstSlot();
    const SfxSlot*      NextSlot();
    const SfxSlot*      GetSlot( USHORT nId );
    const SfxSlot*      GetUnoSlot( USHORT nId );
    const SfxSlot*      GetUnoSlot( const String& rUnoName );
    TypeId              GetSlotType( USHORT nSlotId ) const;
};

//--------------------------------------------------------------------

// seeks to the first func in the current group

inline const SfxSlot* SfxSlotPool::FirstSlot()
{
    return SeekSlot(0);
}

#define SFX_SLOTPOOL() SfxSlotPool::GetSlotPool()

#endif

