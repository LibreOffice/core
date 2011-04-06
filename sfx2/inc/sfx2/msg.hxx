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
#ifndef _SFXMSG_HXX
#define _SFXMSG_HXX

#include <tools/rtti.hxx>
#include <sfx2/shell.hxx>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <sfx2/dllapi.h>

//--------------------------------------------------------------------

#define SFX_SLOT_CACHABLE      0x0001L // exclusiv to VOLATILE, default
#define SFX_SLOT_VOLATILE      0x0002L // per Timer every 2s get new,
                                       // exclusiv to CACHABLE
#define SFX_SLOT_TOGGLE        0x0004L // inverted for Execute old value
#define SFX_SLOT_AUTOUPDATE    0x0008L // invalidated the status automatically
                                       // after execute

#define SFX_SLOT_SYNCHRON      0x0010L // exclusiv to ASYNCHRON, default
#define SFX_SLOT_ASYNCHRON     0x0020L // via Post-Message, exclusiv
                                       // to SYNCHRON

#define SFX_SLOT_HASCOREID     0x0040L // Slot-ID/Which-ID execute mapping
#define SFX_SLOT_HASDIALOG     0x0080L // Coordinates for dialogue after recofig

#define SFX_SLOT_NORECORD      0x0100L // no recording
#define SFX_SLOT_RECORDPERITEM 0x0200L // each item, one statement
#define SFX_SLOT_RECORDPERSET  0x0400L // The whole Set is a Statement, default
#define SFX_SLOT_RECORDMANUAL  0x0800L // Recording by the application
                                       // developer is default

#define SFX_SLOT_RECORDABSOLUTE 0x1000000L // Recording with absolute Target
#define SFX_SLOT_STANDARD       ( SFX_SLOT_CACHABLE | \
                                  SFX_SLOT_SYNCHRON | \
                                  SFX_SLOT_RECORDPERSET )

#define SFX_SLOT_PROPGET       0x1000L  // get property
#define SFX_SLOT_PROPSET       0x2000L  // set property, exclusiv to
                                        // SFX_SLOT_METHOD
#define SFX_SLOT_METHOD        0x4000L  // Method, exclusiv to SFX_SLOT_PROPSET

#define SFX_SLOT_FASTCALL      0x8000L  // No test if disabled before Execute

#define SFX_SLOT_STATUSBARCONFIG 0x10000L  // configurable status row
#define SFX_SLOT_MENUCONFIG      0x20000L  // configurable Menu
#define SFX_SLOT_TOOLBOXCONFIG   0x40000L  // configurable Toolboxen
#define SFX_SLOT_ACCELCONFIG     0x80000L  // configurable keys

#define SFX_SLOT_CONTAINER      0x100000L  // Operated by the container at
                                           // InPlace
#define SFX_SLOT_READONLYDOC    0x200000L  // also available for
                                           // read-only Documents
#define SFX_SLOT_IMAGEROTATION  0x400000L  // Rotate image on Vertical/
                                           // Bi-directional writing
#define SFX_SLOT_IMAGEREFLECTION  0x800000L // Mirror image on Vertical/
                                           // Bi-directional writing

//--------------------------------------------------------------------

class SfxRequest;
class SfxItemSet;

#define SFX_EXEC_STUB( aShellClass, aExecMethod) \
 void SfxStub##aShellClass##aExecMethod( \
   SfxShell *pShell, SfxRequest& rReq) \
  { \
    (( aShellClass* ) pShell )->aExecMethod( rReq ); \
  }

#define SFX_STATE_STUB( aShellClass, aStateMethod) \
 void  SfxStub##aShellClass##aStateMethod( \
   SfxShell *pShell, SfxItemSet& rSet) \
  { \
    (( aShellClass* ) pShell )->aStateMethod( rSet ); \
  }

#define SFX_STUB_PTR( aShellClass, aMethod ) \
   &SfxStub##aShellClass##aMethod

#define SFX_STUB_PTR_EXEC_NONE &SfxShell::EmptyExecStub

#define SFX_STUB_PTR_STATE_NONE &SfxShell::EmptyStateStub

//--------------------------------------------------------------------

enum SfxSlotKind
{
    SFX_KIND_STANDARD,
    SFX_KIND_ENUM,
    SFX_KIND_ATTR
};

//=========================================================================

struct SfxTypeAttrib
{
    sal_uInt16                  nAID;
    const char* pName;
};

struct SfxType
{
    TypeId          aTypeId;
    sal_uInt16          nAttribs;
    SfxTypeAttrib   aAttrib[16];

    const TypeId&   Type() const
                    { return aTypeId; }
    SfxPoolItem*    CreateItem() const
                    { return (SfxPoolItem*) aTypeId(); }
};

struct SfxType0
{
    TypeId          aTypeId;
    sal_uInt16          nAttribs;

    const TypeId&   Type() const
                    { return aTypeId; }
    SfxPoolItem*    CreateItem() const
                    { return (SfxPoolItem*) aTypeId(); }
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
SFX_DECL_TYPE(18); // for SvxSearchItem

// all SfxTypes must be in this header
#undef SFX_DECL_TYPE

#define SFX_SLOT_ARG( aShellClass, id, GroupId, ExecMethodPtr, StateMethodPtr, Flags, ItemClass, nArg0, nArgs, Name, Prop ) \
               { id, GroupId, id, Flags | Prop, \
                 USHRT_MAX, 0, \
                 ExecMethodPtr, \
                 StateMethodPtr, \
                 (const SfxType*) &a##ItemClass##_Impl, \
                 Name, Name, 0, 0, \
                 &a##aShellClass##Args_Impl[nArg0], nArgs, 0, Name \
               }

#define SFX_SLOT( aShellClass, id, GroupId, ExecMethodPtr, StateMethodPtr, Flags, ItemClass ) \
               { id, GroupId, id, Flags, \
                 0, 0, \
                 ExecMethodPtr, \
                 StateMethodPtr, \
                 (const SfxType*) &a##ItemClass##_Impl, \
                 0, 0, 0, 0, 0, 0, 0 \
               }

#define SFX_SLOT_ATTR( id, GroupId, Dummy, ExecMethodPtr, StateMethodPtr, Flags, ItemClass ) \
               { id, GroupId, id, Flags, \
                 USHRT_MAX, 0, \
                 ExecMethodPtr, \
                 StateMethodPtr, \
                 (const SfxType*) &a##ItemClass##_Impl, \
                 0, 0, 0, 0, 0, 0, 0 \
               }

#define SFX_SLOT_ENUM( SlaveId, GroupId, MasterId, Value, Flags ) \
               { SlaveId, GroupId, id, Flags,    \
                 MasterId,  Value, 0, 0, \
                 (const SfxType*) &aSfxBoolItem_Impl, \
                 0, 0, 0, 0, 0, 0, 0 \
               }
#define SFX_NEW_SLOT_ARG( aShellClass, id, hid, GroupId, pLinked, pNext, ExecMethodPtr, StateMethodPtr, Flags, DisableFlags, ItemClass, nArg0, nArgs, Name, Prop, UnoName ) \
               { id, GroupId, hid, Flags | Prop, \
                 USHRT_MAX, 0, \
                 ExecMethodPtr, \
                 StateMethodPtr, \
                 (const SfxType*) &a##ItemClass##_Impl, \
                 Name, Name, \
                 pLinked, pNext, \
                 &a##aShellClass##Args_Impl[nArg0], nArgs, DisableFlags, UnoName \
               }

#define SFX_NEW_SLOT_ENUM( SlaveId, hid, GroupId, pMaster, pNext, MasterId, Value, Flags, DisableFlags, UnoName  ) \
               { SlaveId, GroupId, hid, Flags,   \
                 MasterId,  Value, \
                 0, \
                 0, \
                 (const SfxType*) &aSfxBoolItem_Impl, \
                 0, 0, \
                 pMaster, \
                 pNext, \
                 0, 0, DisableFlags, UnoName \
               }

#define SFX_SLOTMAP_NONE(ShellClass) \
            static SfxFormalArgument a##ShellClass##Args_Impl[1]; \
            static SfxSlot a##ShellClass##Slots_Impl[] = \
            { { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } }

#define SFX_ARGUMENT( ArgSlotId, ArgName, ArgTypeId ) \
            { (const SfxType*) &a##ArgTypeId##_Impl, ArgName, ArgSlotId }

//--------------------------------------------------------------------

class SfxPoolItem;

struct SfxFormalArgument
{
    const SfxType*  pType;    // Type of the parameter (SfxPoolItem subclass)
    const char*     pName;    // Name of the sParameters
    sal_uInt16      nSlotId;  // Slot-Id for identification of the Parameters

    const TypeId&           Type() const
                            { return pType->aTypeId; }
    SfxPoolItem*            CreateItem() const
                            { return (SfxPoolItem*) pType->aTypeId(); }
};

//--------------------------------------------------------------------

class SfxSlot
{
public:
    sal_uInt16    nSlotId;   // Unique slot-ID in Shell
    sal_uInt16    nGroupId;  // for configuration region
    sal_uIntPtr   nHelpId;   // Usually == nSlotId
    sal_uIntPtr   nFlags;    // artihmetic ordered Flags

    sal_uInt16    nMasterSlotId;  // Enum-Slot for example Which-Id
    sal_uInt16    nValue;         // Value, in case of Enum-Slot

    SfxExecFunc   fnExec;   // Function to be excecuted
    SfxStateFunc  fnState;  // Function for Status

    const SfxType*  pType;        // SfxPoolItem-Type (Status)
    const char*     pName;        // Name of the Slots
    const char*     pMethodName;  // Name of the Method if different

    const SfxSlot*  pLinkedSlot; // Master-Slot for Enum value
    const SfxSlot*  pNextSlot;   // with the same Status-Method

    const SfxFormalArgument*  pFirstArgDef;  // first formal Argument-Definition
    sal_uInt16                nArgDefCount;  // Number of formal Argumentents
    long                      nDisableFlags; // DisableFlags that need to be
                                             // present, so that the Slot
                                             // can be enabled
    const char*     pUnoName;      // UnoName for the Slots

public:

    SfxSlotKind     GetKind() const;
    sal_uInt16          GetSlotId() const;
    sal_uIntPtr         GetHelpId() const;
    sal_uIntPtr         GetMode() const;
    sal_Bool            IsMode( sal_uIntPtr nMode ) const;
    sal_uInt16          GetGroupId() const;
    sal_uInt16          GetMasterSlotId() const { return nMasterSlotId; }
    sal_uInt16          GetWhich( const SfxItemPool &rPool ) const;
    sal_uInt16          GetValue() const { return nValue; }
    const SfxType*  GetType() const { return pType; }
    const char*     GetUnoName() const { return pUnoName; }

    sal_uInt16          GetFormalArgumentCount() const { return nArgDefCount; }
    const SfxFormalArgument& GetFormalArgument( sal_uInt16 nNo ) const
                    { return pFirstArgDef[nNo]; }

    SfxExecFunc     GetExecFnc() const { return fnExec; }
    SfxStateFunc    GetStateFnc() const { return fnState; }

    const SfxSlot*  GetLinkedSlot() const { return pLinkedSlot; }
    const SfxSlot*  GetNextSlot() const { return pNextSlot; }
};

//=========================================================================

// returns the id of the function

inline sal_uInt16 SfxSlot::GetSlotId() const
{
    return nSlotId;
}
//--------------------------------------------------------------------
// returns the help-id of the slot

inline sal_uIntPtr SfxSlot::GetHelpId() const
{
    return nHelpId;
}

//--------------------------------------------------------------------

// returns  a bitfield with flags

inline sal_uIntPtr SfxSlot::GetMode() const
{
    return nFlags;
}
//--------------------------------------------------------------------

// determines if the specified mode is assigned

inline sal_Bool SfxSlot::IsMode( sal_uIntPtr nMode ) const
{
    return (nFlags & nMode) != 0;
}
//--------------------------------------------------------------------

// returns the id of the associated group

inline sal_uInt16 SfxSlot::GetGroupId() const
{
    return nGroupId;

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
