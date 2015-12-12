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
#ifndef INCLUDED_SFX2_OBJFACE_HXX
#define INCLUDED_SFX2_OBJFACE_HXX

#include <rtl/ustring.hxx>
#include <sal/config.h>
#include <sal/types.h>
#include <sfx2/dllapi.h>
#include <sfx2/msg.hxx>
#include <tools/resid.hxx>

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
    sal_uInt16              nCount;         // number of slots in SlotMap
    SfxInterfaceId          nClassId;       // Id of interface
    bool                    bSuperClass;    // Whether children inherit its toolbars etc
    SfxInterface_Impl*      pImpData;

    SfxSlot*                operator[]( sal_uInt16 nPos ) const;

public:
                            SfxInterface( const char *pClass,
                                          bool bSuperClass,
                                          SfxInterfaceId nClassId,
                                          const SfxInterface* pGeno,
                                          SfxSlot &rMessages, sal_uInt16 nMsgCount );
    virtual                 ~SfxInterface();

    void                    SetSlotMap( SfxSlot& rMessages, sal_uInt16 nMsgCount );
    inline sal_uInt16           Count() const;

    const SfxSlot*          GetRealSlot( const SfxSlot * ) const;
    const SfxSlot*          GetRealSlot( sal_uInt16 nSlotId ) const;
    const SfxSlot*  GetSlot( sal_uInt16 nSlotId ) const;
    const SfxSlot*          GetSlot( const OUString& rCommand ) const;

    const char*             GetClassName() const { return pName; }
    bool                    UseAsSuperClass() const { return bSuperClass; }

    const SfxInterface*     GetGenoType() const { return pGenoType; }

    void                    RegisterObjectBar(sal_uInt16, sal_uInt32 nResId);
    void                    RegisterObjectBar(sal_uInt16, sal_uInt32 nResId, sal_uInt32 nFeature);
    void                    RegisterChildWindow(sal_uInt16, bool bContext = false);
    void                    RegisterChildWindow(sal_uInt16, bool bContext, sal_uInt32 nFeature);
    void                    RegisterStatusBar( const ResId& );
    sal_uInt32              GetObjectBarId(sal_uInt16 nNo) const;
    sal_uInt16              GetObjectBarPos( sal_uInt16 nNo ) const;
    sal_uInt32              GetObjectBarFeature( sal_uInt16 nNo ) const;
    sal_uInt16              GetObjectBarCount() const;
    bool                    IsObjectBarVisible( sal_uInt16 nNo) const;
    sal_uInt32              GetChildWindowFeature( sal_uInt16 nNo ) const;
    sal_uInt32              GetChildWindowId( sal_uInt16 nNo ) const;
    sal_uInt16              GetChildWindowCount() const;
    void                    RegisterPopupMenu( const ResId& );
    void                    RegisterPopupMenu( const OUString& );
    const ResId&            GetPopupMenuResId() const;
    const OUString&         GetPopupMenuName() const;
    const ResId&            GetStatusBarResId() const;

    void                    Register( SfxModule* );

    SAL_DLLPRIVATE bool     ContainsSlot_Impl( const SfxSlot *pSlot ) const
                            { return pSlot >= pSlots && pSlot < pSlots + Count(); }
};



// returns the number of functions in this cluster

inline sal_uInt16 SfxInterface::Count() const
{
    return nCount;
}



// returns a function by position in the array

inline SfxSlot* SfxInterface::operator[]( sal_uInt16 nPos ) const
{
    return nPos < nCount? pSlots+nPos: nullptr;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
