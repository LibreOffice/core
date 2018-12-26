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

#ifndef INCLUDED_VCL_INC_OSX_RUNINMAIN_HXX
#define INCLUDED_VCL_INC_OSX_RUNINMAIN_HXX

/**
 * Runs a command in the main thread.
 *
 * These macros are always used like a recursive calls, so they work like
 * a closure.
 *
 * Uses two conditionals for a two way communication.
 * The data (code block + result) is protected by the SolarMutex.
 *
 * There are three main macros, which act as function initializers:
 *  - OSX_RUNINMAIN - for all functions without return values
 *  - OSX_RUNINMAIN_POINTER - for all functions returning a pointer
 *  - OSX_RUNINMAIN_UNION - for all other return types
 *
 * All types used via OSX_RUNINMAIN_UNION must implement a move constructor,
 * so there is no memory leak!
 */

#include <unordered_map>

#include <Block.h>

#include <osl/thread.h>

#include "saltimer.h"
#include <salframe.hxx>
#include <tools/debug.hxx>

union RuninmainResult
{
    void*                            pointer;
    bool                             boolean;
    struct SalFrame::SalPointerState state;

    RuninmainResult() {}
};

#define OSX_RUNINMAIN_MEMBERS \
    std::mutex              m_runInMainMutex; \
    std::condition_variable m_aInMainCondition; \
    std::condition_variable m_aResultCondition; \
    bool                    m_wakeUpMain = false; \
    bool                    m_resultReady = false; \
    RuninmainBlock          m_aCodeBlock; \
    RuninmainResult         m_aResult;

#define OSX_RUNINMAIN( instance, command ) \
    if ( !instance->IsMainThread() ) \
    { \
        DBG_TESTSOLARMUTEX(); \
        SalYieldMutex *aMutex = static_cast<SalYieldMutex*>(instance->GetYieldMutex()); \
        { \
            std::unique_lock<std::mutex> g(aMutex->m_runInMainMutex); \
            assert( !aMutex->m_aCodeBlock ); \
            aMutex->m_aCodeBlock = Block_copy(^{ \
                command; \
            }); \
            aMutex->m_wakeUpMain = true; \
            aMutex->m_aInMainCondition.notify_all(); \
        } \
        dispatch_async(dispatch_get_main_queue(),^{ \
            ImplNSAppPostEvent( AquaSalInstance::YieldWakeupEvent, NO ); \
            }); \
        { \
            std::unique_lock<std::mutex> g(aMutex->m_runInMainMutex); \
            aMutex->m_aResultCondition.wait( \
                g, [&aMutex]() { return aMutex->m_resultReady; }); \
            aMutex->m_resultReady = false; \
        } \
        return; \
    }

#define OSX_RUNINMAIN_POINTER( instance, command, type ) \
    if ( !instance->IsMainThread() ) \
    { \
        DBG_TESTSOLARMUTEX(); \
        SalYieldMutex *aMutex = static_cast<SalYieldMutex*>(instance->GetYieldMutex()); \
        { \
            std::unique_lock<std::mutex> g(aMutex->m_runInMainMutex); \
            assert( !aMutex->m_aCodeBlock ); \
            aMutex->m_aCodeBlock = Block_copy(^{ \
                aMutex->m_aResult.pointer = static_cast<void*>( command ); \
            }); \
            aMutex->m_wakeUpMain = true; \
            aMutex->m_aInMainCondition.notify_all(); \
        } \
        dispatch_async(dispatch_get_main_queue(),^{ \
            ImplNSAppPostEvent( AquaSalInstance::YieldWakeupEvent, NO ); \
            }); \
        { \
            std::unique_lock<std::mutex> g(aMutex->m_runInMainMutex); \
            aMutex->m_aResultCondition.wait( \
                g, [&aMutex]() { return aMutex->m_resultReady; }); \
            aMutex->m_resultReady = false; \
        } \
        return static_cast<type>( aMutex->m_aResult.pointer ); \
    }

#define OSX_RUNINMAIN_UNION( instance, command, member ) \
    if ( !instance->IsMainThread() ) \
    { \
        DBG_TESTSOLARMUTEX(); \
        SalYieldMutex *aMutex = static_cast<SalYieldMutex*>(instance->GetYieldMutex()); \
        { \
            std::unique_lock<std::mutex> g(aMutex->m_runInMainMutex); \
            assert( !aMutex->m_aCodeBlock ); \
            aMutex->m_aCodeBlock = Block_copy(^{ \
                aMutex->m_aResult.member = command; \
            }); \
            aMutex->m_wakeUpMain = true; \
            aMutex->m_aInMainCondition.notify_all(); \
        } \
        dispatch_async(dispatch_get_main_queue(),^{ \
            ImplNSAppPostEvent( AquaSalInstance::YieldWakeupEvent, NO ); \
            }); \
        { \
            std::unique_lock<std::mutex> g(aMutex->m_runInMainMutex); \
            aMutex->m_aResultCondition.wait( \
                g, [&aMutex]() { return aMutex->m_resultReady; }); \
            aMutex->m_resultReady = false; \
        } \
        return std::move( aMutex->m_aResult.member ); \
    }

/**
 * convenience macros used from SalInstance
 */

#define OSX_INST_RUNINMAIN( command ) \
    OSX_RUNINMAIN( this, command )

#define OSX_INST_RUNINMAIN_POINTER( command, type ) \
    OSX_RUNINMAIN_POINTER( this, command, type )

#define OSX_INST_RUNINMAIN_UNION( command, member ) \
    OSX_RUNINMAIN_UNION( this, command, member )

/**
 * convenience macros using global SalData
 */

#define OSX_SALDATA_RUNINMAIN( command ) \
    OSX_RUNINMAIN( GetSalData()->mpInstance, command )

#define OSX_SALDATA_RUNINMAIN_POINTER( command, type ) \
    OSX_RUNINMAIN_POINTER( GetSalData()->mpInstance, command, type )

#define OSX_SALDATA_RUNINMAIN_UNION( command, member ) \
    OSX_RUNINMAIN_UNION( GetSalData()->mpInstance, command, member )

#endif // INCLUDED_VCL_INC_OSX_RUNINMAIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
