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
#ifndef INCLUDED_SFX2_MSGPOOL_HXX
#define INCLUDED_SFX2_MSGPOOL_HXX

#include <rtl/ustring.hxx>
#include <sal/config.h>
#include <sal/types.h>
#include <sfx2/dllapi.h>
#include <sfx2/msg.hxx>
#include <string>
#include <vector>

class SfxInterface;
class SfxSlot;

typedef std::basic_string< sal_uInt16 > SfxSlotGroupArr_Impl;
typedef std::vector<SfxInterface*> SfxInterfaceArr_Impl;

class SFX2_DLLPUBLIC SfxSlotPool
{
    SfxSlotGroupArr_Impl*       _pGroups;
    SfxSlotPool*                _pParentPool;
    SfxInterfaceArr_Impl*       _pInterfaces;
    sal_uInt16                      _nCurGroup;
    sal_uInt16                      _nCurInterface;
    sal_uInt16                      _nCurMsg;

private:
    const SfxSlot* SeekSlot( sal_uInt16 nObject );

public:
    SfxSlotPool(SfxSlotPool* pParent=nullptr);
    ~SfxSlotPool();

    void                RegisterInterface( SfxInterface& rFace );
    void                ReleaseInterface( SfxInterface& rFace );
    SfxInterface*       FirstInterface();

    static SfxSlotPool& GetSlotPool( SfxViewFrame *pFrame=nullptr );

    sal_uInt16              GetGroupCount();
    OUString            SeekGroup( sal_uInt16 nNo );
    const SfxSlot*      FirstSlot();
    const SfxSlot*      NextSlot();
    const SfxSlot*      GetSlot( sal_uInt16 nId );
    const SfxSlot*      GetUnoSlot( const OUString& rUnoName );
    const std::type_info*  GetSlotType( sal_uInt16 nSlotId ) const;
};



// seeks to the first func in the current group

inline const SfxSlot* SfxSlotPool::FirstSlot()
{
    return SeekSlot(0);
}

#define SFX_SLOTPOOL() SfxSlotPool::GetSlotPool()

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
