/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _SFXOBJFACE_HXX
#define _SFXOBJFACE_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"
#include <tools/string.hxx>
#include <tools/resid.hxx>

#include <sfx2/msg.hxx>

struct SfxFormalArgument;
struct SfxTypeLibImpl;
struct SfxInterface_Impl;
class  SfxConfigManager;
class  SfxConfigItem;
class  SfxIFConfig_Impl;
class  SfxObjectUIArr_Impl ;
class  SfxModule;
class  SvStream;

class SFX2_DLLPUBLIC SfxInterface
{
friend class SfxIFConfig_Impl;
friend class SfxSlotPool;

    const char*             pName;          // Sfx-internal name of interface
    const SfxInterface*     pGenoType;      // base interface
    SfxSlot*                pSlots;         // SlotMap
    USHORT                  nCount;         // number of slots in SlotMap
    SfxInterfaceId          nClassId;       // Id of interface
    ResId                   aNameResId;     // ResId of external interface name
    SfxInterface_Impl*      pImpData;

    SfxSlot*                operator[]( USHORT nPos ) const;

public:
                            SfxInterface( const char *pClass,
                                          const ResId& rResId,
                                          SfxInterfaceId nClassId,
                                          const SfxInterface* pGeno,
                                          SfxSlot &rMessages, USHORT nMsgCount );
    virtual                 ~SfxInterface();

    void                    SetSlotMap( SfxSlot& rMessages, USHORT nMsgCount );
    inline USHORT           Count() const;

    const SfxSlot*          GetRealSlot( const SfxSlot * ) const;
    const SfxSlot*          GetRealSlot( USHORT nSlotId ) const;
    virtual const SfxSlot*  GetSlot( USHORT nSlotId ) const;
    const SfxSlot*          GetSlot( const String& rCommand ) const;

    const char*             GetClassName() const { return pName; }
    int                     HasName() const { return 0 != aNameResId.GetId(); }
    String                  GetName() const
                            { return String(aNameResId); }
    ResMgr*                 GetResMgr() const
                            { return aNameResId.GetResMgr(); }

    const SfxInterface*     GetGenoType() const { return pGenoType; }
    const SfxInterface*     GetRealInterfaceForSlot( const SfxSlot* ) const;

    void                    RegisterObjectBar( USHORT, const ResId&, const String* pST=0 );
    void                    RegisterObjectBar( USHORT, const ResId&, sal_uInt32 nFeature, const String* pST=0 );
    void                    RegisterChildWindow( USHORT, BOOL bContext, const String* pST=0 );
    void                    RegisterChildWindow( USHORT, BOOL bContext, sal_uInt32 nFeature, const String* pST=0 );
    void                    RegisterStatusBar( const ResId& );
    const ResId&            GetObjectBarResId( USHORT nNo ) const;
    USHORT                  GetObjectBarPos( USHORT nNo ) const;
    sal_uInt32              GetObjectBarFeature( USHORT nNo ) const;
    USHORT                  GetObjectBarCount() const;
    void                    SetObjectBarPos( USHORT nPos, USHORT nId );
    const String*           GetObjectBarName( USHORT nNo ) const;
    BOOL                    IsObjectBarVisible( USHORT nNo) const;
    sal_uInt32              GetChildWindowFeature( USHORT nNo ) const;
    sal_uInt32              GetChildWindowId( USHORT nNo ) const;
    USHORT                  GetChildWindowCount() const;
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

inline USHORT SfxInterface::Count() const
{
    return nCount;
}

//--------------------------------------------------------------------

// returns a function by position in the array

inline SfxSlot* SfxInterface::operator[]( USHORT nPos ) const
{
    return nPos < nCount? pSlots+nPos: 0;
}

class SfxIFConfig_Impl
{
friend class SfxInterface;
    USHORT                  nCount;
    SfxObjectUIArr_Impl*    pObjectBars;

public:
                    SfxIFConfig_Impl();
                    ~SfxIFConfig_Impl();
    BOOL            Store(SvStream&);
    void            RegisterObjectBar( USHORT, const ResId&, sal_uInt32 nFeature, const String* pST=0 );
    USHORT          GetType();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
