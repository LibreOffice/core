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

#include <memory>
#include <rtl/ustring.hxx>
#include <sal/config.h>
#include <sal/types.h>
#include <sfx2/dllapi.h>
#include <sfx2/msg.hxx>
#include <sfx2/toolbarids.hxx>

struct SfxInterface_Impl;
class  SfxModule;

#define SFX_OBJECTBAR_APPLICATION       0
#define SFX_OBJECTBAR_OBJECT            1
#define SFX_OBJECTBAR_TOOLS             2
#define SFX_OBJECTBAR_MACRO             3
#define SFX_OBJECTBAR_FULLSCREEN        4
#define SFX_OBJECTBAR_RECORDING         5
#define SFX_OBJECTBAR_COMMONTASK        6
#define SFX_OBJECTBAR_OPTIONS           7
#define SFX_OBJECTBAR_NAVIGATION        12
#define SFX_OBJECTBAR_MAX               13

enum class StatusBarId : sal_uInt32
{
    None = 0,
    GenericStatusBar = 4,
    WriterStatusBar = 20013,
    MathStatusBar = 20816,
    DrawStatusBar = 23007,
    CalcStatusBar = 26005,
    BasicIdeStatusBar = 30805
};

class SFX2_DLLPUBLIC SfxInterface final
{
friend class SfxSlotPool;

    const char*             pName;          // Sfx-internal name of interface
    const SfxInterface*     pGenoType;      // base interface
    SfxSlot*                pSlots;         // SlotMap
    sal_uInt16              nCount;         // number of slots in SlotMap
    SfxInterfaceId          nClassId;       // Id of interface
    bool                    bSuperClass;    // Whether children inherit its toolbars etc
    std::unique_ptr<SfxInterface_Impl>      pImplData;

public:
                            SfxInterface( const char *pClass,
                                          bool bSuperClass,
                                          SfxInterfaceId nClassId,
                                          const SfxInterface* pGeno,
                                          SfxSlot &rMessages, sal_uInt16 nMsgCount );
                            ~SfxInterface();

    void                    SetSlotMap( SfxSlot& rMessages, sal_uInt16 nMsgCount );
    inline sal_uInt16           Count() const;

    const SfxSlot*          GetRealSlot( const SfxSlot * ) const;
    const SfxSlot*  GetSlot( sal_uInt16 nSlotId ) const;
    const SfxSlot*          GetSlot( const OUString& rCommand ) const;

    const char*             GetClassName() const { return pName; }
    bool                    UseAsSuperClass() const { return bSuperClass; }

    const SfxInterface*     GetGenoType() const { return pGenoType; }

    void                    RegisterObjectBar(sal_uInt16, SfxVisibilityFlags nFlags, ToolbarId eId);
    void                    RegisterObjectBar(sal_uInt16, SfxVisibilityFlags nFlags, ToolbarId eId, SfxShellFeature nFeature);
    void                    RegisterChildWindow(sal_uInt16, bool bContext = false);
    void                    RegisterChildWindow(sal_uInt16, bool bContext, SfxShellFeature nFeature);
    void                    RegisterStatusBar(StatusBarId eId);
    ToolbarId               GetObjectBarId(sal_uInt16 nNo) const;
    sal_uInt16              GetObjectBarPos( sal_uInt16 nNo ) const;
    SfxVisibilityFlags      GetObjectBarFlags( sal_uInt16 nNo ) const;
    SfxShellFeature         GetObjectBarFeature(sal_uInt16 nNo) const;
    sal_uInt16              GetObjectBarCount() const;
    bool                    IsObjectBarVisible( sal_uInt16 nNo) const;
    SfxShellFeature         GetChildWindowFeature(sal_uInt16 nNo) const;
    sal_uInt32              GetChildWindowId( sal_uInt16 nNo ) const;
    sal_uInt16              GetChildWindowCount() const;
    void                    RegisterPopupMenu( const OUString& );
    const OUString&         GetPopupMenuName() const;
    StatusBarId             GetStatusBarId() const;

    void                    Register( const SfxModule* );

    SAL_DLLPRIVATE bool     ContainsSlot_Impl( const SfxSlot *pSlot ) const
                            { return pSlot >= pSlots && pSlot < pSlots + Count(); }
};


// returns the number of functions in this cluster

inline sal_uInt16 SfxInterface::Count() const
{
    return nCount;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
