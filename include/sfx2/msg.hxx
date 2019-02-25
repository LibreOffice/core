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
#ifndef INCLUDED_SFX2_MSG_HXX
#define INCLUDED_SFX2_MSG_HXX

#include <sfx2/shell.hxx>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <svl/poolitem.hxx>
#include <sfx2/dllapi.h>
#include <o3tl/typed_flags_set.hxx>
#include <sfx2/groupid.hxx>
#include <functional>

#include <climits>

class SfxItemPool;
class SfxItemSet;
class SfxPoolItem;
class SfxRequest;

enum class SfxSlotMode {
    NONE            =    0x0000L, // default

    TOGGLE          =    0x0004L, // inverted for Execute old value
    AUTOUPDATE      =    0x0008L, // invalidated the status automatically after execute
    ASYNCHRON       =    0x0020L, // via Post-Message

    NORECORD        =    0x0100L, // no recording
    RECORDPERITEM   =    0x0200L, // each item, one statement
    RECORDPERSET    =    0x0400L, // The whole Set is a Statement, default
    RECORDABSOLUTE  = 0x1000000L, // Recording with absolute Target

    METHOD          =    0x4000L,

    FASTCALL        =    0x8000L, // No test if disabled before Execute

    MENUCONFIG      =   0x20000L, // configurable Menu
    TOOLBOXCONFIG   =   0x40000L, // configurable Toolboxen
    ACCELCONFIG     =   0x80000L, // configurable keys

    CONTAINER       =  0x100000L, // Operated by the container at InPlace
    READONLYDOC     =  0x200000L  // also available for read-only Documents
};

namespace o3tl
{
    template<> struct typed_flags<SfxSlotMode> : is_typed_flags<SfxSlotMode, 0x13ec72cL> {};
}

#define SFX_EXEC_STUB( aShellClass, aExecMethod) \
 void SfxStub##aShellClass##aExecMethod( \
   SfxShell *pShell, SfxRequest& rReq) \
  { \
      ::tools::detail::castTo<aShellClass*>(pShell)->aExecMethod( rReq ); \
  }

#define SFX_STATE_STUB( aShellClass, aStateMethod) \
 void  SfxStub##aShellClass##aStateMethod( \
   SfxShell *pShell, SfxItemSet& rSet) \
  { \
      static_cast<aShellClass*>(pShell)->aStateMethod( rSet ); \
  }

#define SFX_STUB_PTR( aShellClass, aMethod ) \
   &SfxStub##aShellClass##aMethod

#define SFX_STUB_PTR_EXEC_NONE &SfxShell::EmptyExecStub

#define SFX_STUB_PTR_STATE_NONE &SfxShell::EmptyStateStub


enum class SfxSlotKind
{
    Standard,
    Attribute
};


struct SfxTypeAttrib
{
    sal_uInt16                  nAID;
    const char* pName;
};

template<class T> SfxPoolItem* createSfxPoolItem()
{
    return T::CreateDefault();
}
struct SfxType
{
    std::function<SfxPoolItem* ()> const createSfxPoolItemFunc;
    const std::type_info*   pType;
    sal_uInt16 const        nAttribs;
    SfxTypeAttrib   aAttrib[1]; // variable length

    const std::type_info* Type() const{return pType;}
    std::unique_ptr<SfxPoolItem> CreateItem() const
                    { return std::unique_ptr<SfxPoolItem>(createSfxPoolItemFunc()); }
};

struct SfxType0
{
    std::function<SfxPoolItem* ()> const createSfxPoolItemFunc;
    const std::type_info*    pType;
    sal_uInt16 const         nAttribs;
    const std::type_info*    Type() const { return pType;}
};
#define SFX_DECL_TYPE(n)    struct SfxType##n                   \
                            {                                   \
                                std::function<SfxPoolItem* ()> createSfxPoolItemFunc; \
                                const std::type_info* pType; \
                                sal_uInt16          nAttribs;       \
                                SfxTypeAttrib   aAttrib[n];     \
                            }

#define SFX_TYPE(Class) &a##Class##_Impl

SFX_DECL_TYPE(1);
SFX_DECL_TYPE(2);
SFX_DECL_TYPE(3);
SFX_DECL_TYPE(4);
SFX_DECL_TYPE(5);
SFX_DECL_TYPE(6);
SFX_DECL_TYPE(7);
SFX_DECL_TYPE(8);
SFX_DECL_TYPE(10); // for SfxDocInfoItem
SFX_DECL_TYPE(11);

SFX_DECL_TYPE(13); // for SwAddPrinterItem, Sd...
SFX_DECL_TYPE(14);
SFX_DECL_TYPE(16); // for SwDocDisplayItem
SFX_DECL_TYPE(17); // for SvxAddressItem
SFX_DECL_TYPE(23); // for SvxSearchItem

// all SfxTypes must be in this header
#undef SFX_DECL_TYPE

#define SFX_SLOT_ARG( aShellClass, id, GroupId, ExecMethodPtr, StateMethodPtr, Flags, ItemClass, nArg0, nArgs, Name, Prop ) \
               { id, GroupId, Flags | Prop, \
                 USHRT_MAX, 0, \
                 ExecMethodPtr, \
                 StateMethodPtr, \
                 (const SfxType*) &a##ItemClass##_Impl, \
                 0, \
                 &a##aShellClass##Args_Impl[nArg0], nArgs, SfxDisableFlags::NONE, Name \
               }

#define SFX_NEW_SLOT_ARG( aShellClass, id, GroupId, pNext, ExecMethodPtr, StateMethodPtr, Flags, DisableFlags, ItemClass, nArg0, nArgs, Prop, UnoName ) \
               { id, GroupId, Flags | Prop, \
                 USHRT_MAX, 0, \
                 ExecMethodPtr, \
                 StateMethodPtr, \
                 (const SfxType*) &a##ItemClass##_Impl, \
                 pNext, \
                 &a##aShellClass##Args_Impl[nArg0], nArgs, DisableFlags, UnoName \
               }

struct SfxFormalArgument
{
    const SfxType*   pType;    // Type of the parameter (SfxPoolItem subclass)
    const char*      pName;    // Name of the sParameters
    sal_uInt16 const nSlotId;  // Slot-Id for identification of the Parameters

    std::unique_ptr<SfxPoolItem> CreateItem() const
                            { return pType->CreateItem(); }
};


class SfxSlot
{
public:
    sal_uInt16    nSlotId;   // Unique slot-ID in Shell
    SfxGroupId    nGroupId;  // for configuration region
    SfxSlotMode   nFlags;    // arithmetic ordered Flags

    sal_uInt16    nMasterSlotId;  // Enum-Slot for example Which-Id
    sal_uInt16    nValue;         // Value, in case of Enum-Slot

    SfxExecFunc   fnExec;   // Function to be executed
    SfxStateFunc  fnState;  // Function for Status

    const SfxType*  pType;       // SfxPoolItem-Type (Status)

    const SfxSlot*  pNextSlot;   // with the same Status-Method

    const SfxFormalArgument*  pFirstArgDef;  // first formal Argument-Definition
    sal_uInt16                nArgDefCount;  // Number of formal Arguments
    SfxDisableFlags           nDisableFlags; // DisableFlags that need to be
                                             // present, so that the Slot
                                             // can be enabled
    const char*     pUnoName;      // UnoName for the Slots

public:

    SfxSlotKind         GetKind() const;
    sal_uInt16          GetSlotId() const;
    SfxSlotMode         GetMode() const;
    bool                IsMode( SfxSlotMode nMode ) const;
    SfxGroupId          GetGroupId() const;
    sal_uInt16          GetWhich( const SfxItemPool &rPool ) const;
    const SfxType*  GetType() const { return pType; }
    const char*     GetUnoName() const { return pUnoName; }
    SFX2_DLLPUBLIC OString    GetCommand() const;
    SFX2_DLLPUBLIC OUString    GetCommandString() const;

    sal_uInt16          GetFormalArgumentCount() const { return nArgDefCount; }
    const SfxFormalArgument& GetFormalArgument( sal_uInt16 nNo ) const
                    { return pFirstArgDef[nNo]; }

    SfxExecFunc     GetExecFnc() const { return fnExec; }
    SfxStateFunc    GetStateFnc() const { return fnState; }

    const SfxSlot*  GetNextSlot() const { return pNextSlot; }
};


// returns the id of the function

inline sal_uInt16 SfxSlot::GetSlotId() const
{
    return nSlotId;
}

// returns  a bitfield with flags

inline SfxSlotMode SfxSlot::GetMode() const
{
    return nFlags;
}


// determines if the specified mode is assigned

inline bool SfxSlot::IsMode( SfxSlotMode nMode ) const
{
    return bool(nFlags & nMode);
}


// returns the id of the associated group

inline SfxGroupId SfxSlot::GetGroupId() const
{
    return nGroupId;

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
