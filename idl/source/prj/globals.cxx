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

#include <sal/config.h>
#include <rtl/instance.hxx>

#include <globals.hxx>
#include <database.hxx>

namespace {

struct TheIdlDll: public rtl::Static<IdlDll, TheIdlDll> {};

}

IdlDll & GetIdlApp()
{
    return TheIdlDll::get();
}

IdlDll::IdlDll()
{}

IdlDll::~IdlDll()
{
}

static SvStringHashEntry * INS( const OString& rName )
{
    sal_uInt32  nIdx;
    GetIdlApp().pHashTable->Insert( rName, &nIdx );
    return GetIdlApp().pHashTable->Get( nIdx );
}
#define A_ENTRY( Name ) , MM_##Name( INS( #Name ) )

SvGlobalHashNames::SvGlobalHashNames()
    : MM_module( INS( "module" ) )
    , MM_interface( INS( "interface" ) )
    , MM_shell( INS( "shell" ) )
    , MM_Toggle( INS( "Toggle" ) )
    , MM_AutoUpdate( INS( "AutoUpdate" ) )
    , MM_Asynchron( INS( "Asynchron" ) )
    A_ENTRY(RecordPerSet)
    A_ENTRY(RecordPerItem)
    A_ENTRY(NoRecord)
    A_ENTRY(RecordAbsolute)
    A_ENTRY(enum)
    A_ENTRY(UINT16)
    A_ENTRY(INT16)
    A_ENTRY(UINT32)
    A_ENTRY(INT32)
    A_ENTRY(BOOL)
    A_ENTRY(BYTE)
    A_ENTRY(float)
    A_ENTRY(double)
    A_ENTRY(item)
    A_ENTRY(import)
    A_ENTRY(SlotIdFile)
    A_ENTRY(include)
    A_ENTRY(ExecMethod)
    A_ENTRY(StateMethod)
    A_ENTRY(GroupId)
    A_ENTRY(define)
    A_ENTRY(MenuConfig)
    A_ENTRY(ToolBoxConfig)
    A_ENTRY(AccelConfig)
    A_ENTRY(FastCall)
    A_ENTRY(SbxObject)
    A_ENTRY(Container)
    A_ENTRY(ReadOnlyDoc)
    A_ENTRY(struct)
    A_ENTRY(DisableFlags)
{}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
