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

#ifndef INCLUDED_IDL_INC_GLOBALS_HXX
#define INCLUDED_IDL_INC_GLOBALS_HXX

#include <hash.hxx>

typedef tools::SvRef<SvStringHashEntry> SvStringHashEntryRef;

class SvClassManager;
struct SvGlobalHashNames
{
    SvStringHashEntryRef MM_module;
    SvStringHashEntryRef MM_interface;
    SvStringHashEntryRef MM_shell;
    SvStringHashEntryRef MM_Toggle;
    SvStringHashEntryRef MM_AutoUpdate;
    SvStringHashEntryRef MM_Asynchron;
    SvStringHashEntryRef MM_RecordPerSet;
    SvStringHashEntryRef MM_RecordPerItem;
    SvStringHashEntryRef MM_NoRecord;
    SvStringHashEntryRef MM_RecordAbsolute;
    SvStringHashEntryRef MM_enum;
    SvStringHashEntryRef MM_UINT16;
    SvStringHashEntryRef MM_INT16;
    SvStringHashEntryRef MM_UINT32;
    SvStringHashEntryRef MM_INT32;
    SvStringHashEntryRef MM_BOOL;
    SvStringHashEntryRef MM_BYTE;
    SvStringHashEntryRef MM_float;
    SvStringHashEntryRef MM_double;
    SvStringHashEntryRef MM_item;
    SvStringHashEntryRef MM_PseudoSlots;
    SvStringHashEntryRef MM_import;
    SvStringHashEntryRef MM_SlotIdFile;
    SvStringHashEntryRef MM_include;
    SvStringHashEntryRef MM_ExecMethod;
    SvStringHashEntryRef MM_StateMethod;
    SvStringHashEntryRef MM_GroupId;
    SvStringHashEntryRef MM_Export;
    SvStringHashEntryRef MM_PseudoPrefix;
    SvStringHashEntryRef MM_define;
    SvStringHashEntryRef MM_MenuConfig;
    SvStringHashEntryRef MM_ToolBoxConfig;
    SvStringHashEntryRef MM_AccelConfig;
    SvStringHashEntryRef MM_FastCall;
    SvStringHashEntryRef MM_SbxObject;
    SvStringHashEntryRef MM_Container;
    SvStringHashEntryRef MM_ImageRotation;
    SvStringHashEntryRef MM_ImageReflection;
    SvStringHashEntryRef MM_ReadOnlyDoc;
    SvStringHashEntryRef MM_struct;
    SvStringHashEntryRef MM_SlotType;
    SvStringHashEntryRef MM_DisableFlags;

    SvGlobalHashNames();
};

class IdlDll
{
public:
    SvStringHashTable * pHashTable;
    SvGlobalHashNames * pGlobalNames;

                IdlDll();
                ~IdlDll();
};

IdlDll & GetIdlApp();

#define HASH_INLINE( Name )                                  \
inline SvStringHashEntry * SvHash_##Name()                   \
{                                                            \
    if( !GetIdlApp().pGlobalNames )                          \
        GetIdlApp().pGlobalNames = new SvGlobalHashNames();  \
    return GetIdlApp().pGlobalNames->MM_##Name;              \
}

HASH_INLINE(module)
HASH_INLINE(interface)
HASH_INLINE(shell)
HASH_INLINE(Toggle)
HASH_INLINE(AutoUpdate)
HASH_INLINE(Asynchron)
HASH_INLINE(RecordPerItem)
HASH_INLINE(RecordPerSet)
HASH_INLINE(NoRecord)
HASH_INLINE(RecordAbsolute)
HASH_INLINE(enum)
HASH_INLINE(UINT16)
HASH_INLINE(INT16)
HASH_INLINE(UINT32)
HASH_INLINE(INT32)
HASH_INLINE(BOOL)
HASH_INLINE(BYTE)
HASH_INLINE(item)
HASH_INLINE(PseudoSlots)
HASH_INLINE(import)
HASH_INLINE(SlotIdFile)
HASH_INLINE(include)
HASH_INLINE(ExecMethod)
HASH_INLINE(StateMethod)
HASH_INLINE(GroupId)
HASH_INLINE(float)
HASH_INLINE(double)
HASH_INLINE(Export)
HASH_INLINE(PseudoPrefix)
HASH_INLINE(define)
HASH_INLINE(MenuConfig)
HASH_INLINE(ToolBoxConfig)
HASH_INLINE(AccelConfig)
HASH_INLINE(FastCall)
HASH_INLINE(SbxObject)
HASH_INLINE(Container)
HASH_INLINE(ImageRotation)
HASH_INLINE(ImageReflection)
HASH_INLINE(ReadOnlyDoc)
HASH_INLINE(struct)
HASH_INLINE(SlotType)
HASH_INLINE(DisableFlags)


#endif // INCLUDED_IDL_INC_GLOBALS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
