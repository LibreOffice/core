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

#pragma once

#include "hash.hxx"
#include <memory>

struct SvGlobalHashNames
{
    SvStringHashEntry* MM_module;
    SvStringHashEntry* MM_interface;
    SvStringHashEntry* MM_shell;
    SvStringHashEntry* MM_Toggle;
    SvStringHashEntry* MM_AutoUpdate;
    SvStringHashEntry* MM_Asynchron;
    SvStringHashEntry* MM_RecordPerSet;
    SvStringHashEntry* MM_RecordPerItem;
    SvStringHashEntry* MM_NoRecord;
    SvStringHashEntry* MM_RecordAbsolute;
    SvStringHashEntry* MM_enum;
    SvStringHashEntry* MM_UINT16;
    SvStringHashEntry* MM_INT16;
    SvStringHashEntry* MM_UINT32;
    SvStringHashEntry* MM_INT32;
    SvStringHashEntry* MM_BOOL;
    SvStringHashEntry* MM_BYTE;
    SvStringHashEntry* MM_float;
    SvStringHashEntry* MM_double;
    SvStringHashEntry* MM_item;
    SvStringHashEntry* MM_import;
    SvStringHashEntry* MM_SlotIdFile;
    SvStringHashEntry* MM_include;
    SvStringHashEntry* MM_ExecMethod;
    SvStringHashEntry* MM_StateMethod;
    SvStringHashEntry* MM_GroupId;
    SvStringHashEntry* MM_define;
    SvStringHashEntry* MM_MenuConfig;
    SvStringHashEntry* MM_ToolBoxConfig;
    SvStringHashEntry* MM_AccelConfig;
    SvStringHashEntry* MM_FastCall;
    SvStringHashEntry* MM_SbxObject;
    SvStringHashEntry* MM_Container;
    SvStringHashEntry* MM_ReadOnlyDoc;
    SvStringHashEntry* MM_struct;
    SvStringHashEntry* MM_DisableFlags;

    SvGlobalHashNames();
};

class IdlDll
{
public:
    std::unique_ptr<SvStringHashTable> pHashTable;
    std::unique_ptr<SvGlobalHashNames> pGlobalNames;

                IdlDll();
                ~IdlDll();
};

IdlDll & GetIdlApp();

#define HASH_INLINE( Name )                                  \
inline SvStringHashEntry * SvHash_##Name()                   \
{                                                            \
    if( !GetIdlApp().pGlobalNames )                          \
        GetIdlApp().pGlobalNames.reset( new SvGlobalHashNames() );  \
    return GetIdlApp().pGlobalNames->MM_##Name;      \
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
HASH_INLINE(import)
HASH_INLINE(SlotIdFile)
HASH_INLINE(include)
HASH_INLINE(ExecMethod)
HASH_INLINE(StateMethod)
HASH_INLINE(GroupId)
HASH_INLINE(float)
HASH_INLINE(double)
HASH_INLINE(define)
HASH_INLINE(MenuConfig)
HASH_INLINE(ToolBoxConfig)
HASH_INLINE(AccelConfig)
HASH_INLINE(FastCall)
HASH_INLINE(SbxObject)
HASH_INLINE(Container)
HASH_INLINE(ReadOnlyDoc)
HASH_INLINE(struct)
HASH_INLINE(DisableFlags)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
