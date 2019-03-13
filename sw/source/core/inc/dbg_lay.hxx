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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_DBG_LAY_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_DBG_LAY_HXX

#include <o3tl/typed_flags_set.hxx>
#include <memory>

enum class PROT {
    FileInit   = 0x00000000,
    Init       = 0x00000001,
    MakeAll    = 0x00000002,
    MoveFwd    = 0x00000004,
    MoveBack   = 0x00000008,
    Grow       = 0x00000010,
    Shrink     = 0x00000020,
    GrowTest   = 0x00000040,
    ShrinkTest = 0x00000080,
    Size       = 0x00000100,
    PrintArea  = 0x00000200,
    AdjustN    = 0x00000800,
    Section    = 0x00001000,
    Cut        = 0x00002000,
    Paste      = 0x00004000,
    Leaf       = 0x00008000,
    TestFormat = 0x00010000,
    FrmChanges = 0x00020000,
};
namespace o3tl {
    template<> struct typed_flags<PROT> : is_typed_flags<PROT, 0x0003fbff> {};
}

enum class DbgAction {
    NONE,
    Start, End,
    CreateMaster, CreateFollow,
    DelMaster, DelFollow,
    Merge,
    NextSect, PrevSect
};

#ifdef DBG_UTIL

class SwImplProtocol;
class SwFrame;
class SwImplEnterLeave;

class SwProtocol
{
    static PROT nRecord;
    static SwImplProtocol* pImpl;
    static bool Start() { return bool( PROT::Init & nRecord ); }

public:
    static PROT Record() { return nRecord; }
    static void SetRecord( PROT nNew ) { nRecord = nNew; }
    static bool Record( PROT nFunc ) { return bool(( nFunc | PROT::Init ) & nRecord); }
    static void Record( const SwFrame* pFrame, PROT nFunction, DbgAction nAction, void* pParam );
    static void Init();
    static void Stop();
};

class SwEnterLeave
{
    std::unique_ptr<SwImplEnterLeave> pImpl;
public:
    SwEnterLeave( const SwFrame* pFrame, PROT nFunc, DbgAction nAct, void* pPar );
    ~SwEnterLeave();
};

#define PROTOCOL( pFrame, nFunc, nAct, pPar ) {   if( SwProtocol::Record( nFunc ) )\
                                                    SwProtocol::Record( pFrame, nFunc, nAct, pPar ); }
#define PROTOCOL_INIT SwProtocol::Init();
#define PROTOCOL_STOP SwProtocol::Stop();
#define PROTOCOL_ENTER( pFrame, nFunc, nAct, pPar ) SwEnterLeave aEnter( pFrame, nFunc, nAct, pPar );

#else

#define PROTOCOL( pFrame, nFunc, nAct, pPar )
#define PROTOCOL_INIT
#define PROTOCOL_STOP
#define PROTOCOL_ENTER( pFrame, nFunc, nAct, pPar )

#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
