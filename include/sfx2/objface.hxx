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
#ifndef _SFXOBJFACE_HXX
#define _SFXOBJFACE_HXX

#include "sal/config.h"

#include "rtl/ustring.hxx"
#include "sfx2/dllapi.h"
#include "sal/types.h"
#include <tools/string.hxx>
#include <tools/resid.hxx>

#include <sfx2/msg.hxx>

struct SfxFormalArgument;
struct SfxInterface_Impl;
class  SfxConfigItem;
class  SfxModule;
class  SvStream;

class SFX2_DLLPUBLIC SfxInterface
{
friend class SfxSlotPool;

    const char*             pName;          // Sfx-internal name of interface
    const SfxInterface*     pGenoType;      // base interface
    SfxSlot*                pSlots;         // SlotMap
    sal_uInt16                  nCount;         // number of slots in SlotMap
    SfxInterfaceId          nClassId;       // Id of interface
    ResId                   aNameResId;     // ResId of external interface name
    SfxInterface_Impl*      pImpData;

    SfxSlot*                operator[]( sal_uInt16 nPos ) const;

public:
                            SfxInterface( const char *pClass,
                                          const ResId& rResId,
                                          SfxInterfaceId nClassId,
                                          const SfxInterface* pGeno,
                                          SfxSlot &rMessages, sal_uInt16 nMsgCount );
    virtual                 ~SfxInterface();

    void                    SetSlotMap( SfxSlot& rMessages, sal_uInt16 nMsgCount );
    inline sal_uInt16           Count() const;

    const SfxSlot*          GetRealSlot( const SfxSlot * ) const;
    const SfxSlot*          GetRealSlot( sal_uInt16 nSlotId ) const;
    virtual const SfxSlot*  GetSlot( sal_uInt16 nSlotId ) const;
    const SfxSlot*          GetSlot( const OUString& rCommand ) const;

    const char*             GetClassName() const { return pName; }
    bool                    HasName() const { return 0 != aNameResId.GetId(); }
    OUString           GetName() const
                            { return HasName() ? aNameResId.toString() : OUString(); }
    ResMgr*                 GetResMgr() const
                            { return aNameResId.GetResMgr(); }

    const SfxInterface*     GetGenoType() const { return pGenoType; }
    const SfxInterface*     GetRealInterfaceForSlot( const SfxSlot* ) const;

    void                    RegisterObjectBar( sal_uInt16, const ResId&, const OUString* pST=0 );
    void                    RegisterObjectBar( sal_uInt16, const ResId&, sal_uInt32 nFeature, const OUString* pST=0 );
    void                    RegisterChildWindow( sal_uInt16, sal_Bool bContext, const OUString* pST=0 );
    void                    RegisterChildWindow( sal_uInt16, sal_Bool bContext, sal_uInt32 nFeature, const OUString* pST=0 );
    void                    RegisterStatusBar( const ResId& );
    const ResId&            GetObjectBarResId( sal_uInt16 nNo ) const;
    sal_uInt16              GetObjectBarPos( sal_uInt16 nNo ) const;
    sal_uInt32              GetObjectBarFeature( sal_uInt16 nNo ) const;
    sal_uInt16              GetObjectBarCount() const;
    void                    SetObjectBarPos( sal_uInt16 nPos, sal_uInt16 nId );
    const OUString*         GetObjectBarName( sal_uInt16 nNo ) const;
    sal_Bool                IsObjectBarVisible( sal_uInt16 nNo) const;
    sal_uInt32              GetChildWindowFeature( sal_uInt16 nNo ) const;
    sal_uInt32              GetChildWindowId( sal_uInt16 nNo ) const;
    sal_uInt16              GetChildWindowCount() const;
    void                    RegisterPopupMenu( const ResId& );
    const ResId&            GetPopupMenuResId() const;
    const ResId&            GetStatusBarResId() const;

    void                    Register( SfxModule* );

    SAL_DLLPRIVATE int      ContainsSlot_Impl( const SfxSlot *pSlot ) const
                            { return pSlot >= pSlots && pSlot < pSlots + Count(); }
    SAL_DLLPRIVATE ResMgr*  GetResManager_Impl() const
                            { return aNameResId.GetResMgr(); }
};

//--------------------------------------------------------------------

// returns the number of functions in this cluster

inline sal_uInt16 SfxInterface::Count() const
{
    return nCount;
}

//--------------------------------------------------------------------

// returns a function by position in the array

inline SfxSlot* SfxInterface::operator[]( sal_uInt16 nPos ) const
{
    return nPos < nCount? pSlots+nPos: 0;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
