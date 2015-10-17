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

#include <tools/rtti.hxx>
#include <sfx2/shell.hxx>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <sfx2/dllapi.h>
#include <svl/itemset.hxx>
#include <o3tl/typed_flags_set.hxx>

enum class SfxSlotMode {
    NONE            =    0x0000L, // exclusiv to VOLATILE, default
    CACHABLE        =    0x0001L, // exclusiv to VOLATILE, default
    VOLATILE        =    0x0002L, // per Timer every 2s get new, exclusive to CACHABLE

    TOGGLE          =    0x0004L, // inverted for Execute old value
    AUTOUPDATE      =    0x0008L, // invalidated the status automatically after execute
    SYNCHRON        =    0x0010L, // exclusive to ASYNCHRON, default
    ASYNCHRON       =    0x0020L, // via Post-Message, exclusive to SYNCHRON
    HASDIALOG       =    0x0080L, // Coordinates for dialogue after reconfig

    NORECORD        =    0x0100L, // no recording
    RECORDPERITEM   =    0x0200L, // each item, one statement
    RECORDPERSET    =    0x0400L, // The whole Set is a Statement, default
    RECORDMANUAL    =    0x0800L, // Recording by the application developer is default
    RECORDABSOLUTE  = 0x1000000L, // Recording with absolute Target
    STANDARD        =   0x00411L, // CACHABLE | SYNCHRON | RECORDPERSET;

    PROPGET         =    0x1000L, // get property
    PROPSET         =    0x2000L, // set property, exclusive to METHOD
    METHOD          =    0x4000L, // Method, exclusiv to PROPSET

    FASTCALL        =    0x8000L, // No test if disabled before Execute

    STATUSBARCONFIG =   0x10000L, // configurable status row
    MENUCONFIG      =   0x20000L, // configurable Menu
    TOOLBOXCONFIG   =   0x40000L, // configurable Toolboxen
    ACCELCONFIG     =   0x80000L, // configurable keys

    CONTAINER       =  0x100000L, // Operated by the container at InPlace
    READONLYDOC     =  0x200000L, // also available for read-only Documents
    IMAGEROTATION   =  0x400000L, // Rotate image on Vertical/Bi-directional writing
    IMAGEREFLECTION =  0x800000L  // Mirror image on Vertical/Bi-directional writing
};

namespace o3tl
{
    template<> struct typed_flags<SfxSlotMode> : is_typed_flags<SfxSlotMode, 0x1ffffbfL> {};
}


class SfxRequest;

#define SFX_EXEC_STUB( aShellClass, aExecMethod) \
 void SfxStub##aShellClass##aExecMethod( \
   SfxShell *pShell, SfxRequest& rReq) \
  { \
      static_cast<aShellClass*>(pShell)->aExecMethod( rReq ); \
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



enum SfxSlotKind
{
    SFX_KIND_STANDARD,
    SFX_KIND_ENUM,
    SFX_KIND_ATTR
};



struct SfxTypeAttrib
{
    sal_uInt16                  nAID;
    const char* pName;
};

struct SfxType
{
    TypeId          aTypeId;
    sal_uInt16          nAttribs;
    SfxTypeAttrib   aAttrib[1]; // variable length

    const TypeId&   Type() const
                    { return aTypeId; }
    SfxPoolItem*    CreateItem() const
                    { return static_cast<SfxPoolItem*>(aTypeId()); }
};

struct SfxType0
{
    TypeId          aTypeId;
    sal_uInt16          nAttribs;

    const TypeId&   Type() const
                    { return aTypeId; }
};

#define SFX_DECL_TYPE(n)    struct SfxType##n                   \
                            {                                   \
                                TypeId          aTypeId;        \
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
SFX_DECL_TYPE(21); // for SvxSearchItem

// all SfxTypes must be in this header
#undef SFX_DECL_TYPE

#define SFX_SLOT_ARG( aShellClass, id, GroupId, ExecMethodPtr, StateMethodPtr, Flags, ItemClass, nArg0, nArgs, Name, Prop ) \
               { id, GroupId, id, Flags | Prop, \
                 USHRT_MAX, 0, \
                 ExecMethodPtr, \
                 StateMethodPtr, \
                 (const SfxType*) &a##ItemClass##_Impl, \
                 0, 0, \
                 &a##aShellClass##Args_Impl[nArg0], nArgs, 0, Name \
               }

#define SFX_SLOT( aShellClass, id, GroupId, ExecMethodPtr, StateMethodPtr, Flags, ItemClass ) \
               { id, GroupId, id, Flags, \
                 0, 0, \
                 ExecMethodPtr, \
                 StateMethodPtr, \
                 (const SfxType*) &a##ItemClass##_Impl, \
                 0, 0, 0, 0, 0 \
               }

#define SFX_NEW_SLOT_ARG( aShellClass, id, hid, GroupId, pLinked, pNext, ExecMethodPtr, StateMethodPtr, Flags, DisableFlags, ItemClass, nArg0, nArgs, Prop, UnoName ) \
               { id, GroupId, hid, Flags | Prop, \
                 USHRT_MAX, 0, \
                 ExecMethodPtr, \
                 StateMethodPtr, \
                 (const SfxType*) &a##ItemClass##_Impl, \
                 pLinked, pNext, \
                 &a##aShellClass##Args_Impl[nArg0], nArgs, DisableFlags, UnoName \
               }

#define SFX_NEW_SLOT_ENUM( SlaveId, hid, GroupId, pMaster, pNext, MasterId, Value, Flags, DisableFlags, UnoName  ) \
               { SlaveId, GroupId, hid, Flags,   \
                 MasterId,  Value, \
                 0, \
                 0, \
                 (const SfxType*) &aSfxBoolItem_Impl, \
                 pMaster, \
                 pNext, \
                 0, 0, DisableFlags, UnoName \
               }

class SfxPoolItem;

struct SfxFormalArgument
{
    const SfxType*  pType;    // Type of the parameter (SfxPoolItem subclass)
    const char*     pName;    // Name of the sParameters
    sal_uInt16      nSlotId;  // Slot-Id for identification of the Parameters

    const TypeId&           Type() const
                            { return pType->aTypeId; }
    SfxPoolItem*            CreateItem() const
                            { return static_cast<SfxPoolItem*>(pType->aTypeId()); }
};



class SfxSlot
{
public:
    sal_uInt16    nSlotId;   // Unique slot-ID in Shell
    sal_uInt16    nGroupId;  // for configuration region
    sal_uIntPtr   nHelpId;   // Usually == nSlotId
    SfxSlotMode   nFlags;    // arithmetic ordered Flags

    sal_uInt16    nMasterSlotId;  // Enum-Slot for example Which-Id
    sal_uInt16    nValue;         // Value, in case of Enum-Slot

    SfxExecFunc   fnExec;   // Function to be executed
    SfxStateFunc  fnState;  // Function for Status

    const SfxType*  pType;       // SfxPoolItem-Type (Status)

    const SfxSlot*  pLinkedSlot; // Master-Slot for Enum value
    const SfxSlot*  pNextSlot;   // with the same Status-Method

    const SfxFormalArgument*  pFirstArgDef;  // first formal Argument-Definition
    sal_uInt16                nArgDefCount;  // Number of formal Arguments
    long                      nDisableFlags; // DisableFlags that need to be
                                             // present, so that the Slot
                                             // can be enabled
    const char*     pUnoName;      // UnoName for the Slots

public:

    SfxSlotKind         GetKind() const;
    sal_uInt16          GetSlotId() const;
    SfxSlotMode         GetMode() const;
    bool                IsMode( SfxSlotMode nMode ) const;
    sal_uInt16          GetGroupId() const;
    sal_uInt16          GetMasterSlotId() const { return nMasterSlotId; }
    sal_uInt16          GetWhich( const SfxItemPool &rPool ) const;
    sal_uInt16          GetValue() const { return nValue; }
    const SfxType*  GetType() const { return pType; }
    const char*     GetUnoName() const { return pUnoName; }
    SFX2_DLLPUBLIC OString    GetCommand() const;
    SFX2_DLLPUBLIC OUString    GetCommandString() const;

    sal_uInt16          GetFormalArgumentCount() const { return nArgDefCount; }
    const SfxFormalArgument& GetFormalArgument( sal_uInt16 nNo ) const
                    { return pFirstArgDef[nNo]; }

    SfxExecFunc     GetExecFnc() const { return fnExec; }
    SfxStateFunc    GetStateFnc() const { return fnState; }

    const SfxSlot*  GetLinkedSlot() const { return pLinkedSlot; }
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

inline sal_uInt16 SfxSlot::GetGroupId() const
{
    return nGroupId;

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
