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

#include <memory>
#include <mutex>

#include <vcl/svapp.hxx>
#include <tools/debug.hxx>
#include <vcl/weld.hxx>

#include <basic/sbstar.hxx>
#include <basic/basrdll.hxx>
#include <strings.hrc>
#include <sbxbase.hxx>
#include <config_features.h>

namespace
{
struct BasicDLLImpl : public SvRefBase
{
    bool        bDebugMode;
    bool        bBreakEnabled;

    std::unique_ptr<SbxAppData> xSbxAppData;

    BasicDLLImpl()
        : bDebugMode(false)
        , bBreakEnabled(true)
        , xSbxAppData(new SbxAppData)
    { }

    static BasicDLLImpl* BASIC_DLL;
    static std::mutex& getMutex()
    {
        static std::mutex aMutex;
        return aMutex;
    }
};

BasicDLLImpl* BasicDLLImpl::BASIC_DLL = nullptr;
}

BasicDLL::BasicDLL()
{
    std::scoped_lock aGuard(BasicDLLImpl::getMutex());
    if (!BasicDLLImpl::BASIC_DLL)
        BasicDLLImpl::BASIC_DLL = new BasicDLLImpl;
    m_xImpl = BasicDLLImpl::BASIC_DLL;
}

BasicDLL::~BasicDLL()
{
    std::scoped_lock aGuard(BasicDLLImpl::getMutex());
    const bool bLastRef = m_xImpl->GetRefCount() == 1;
    if (bLastRef) {
        BasicDLLImpl::BASIC_DLL->xSbxAppData->m_aGlobErr.clear();
    }
    m_xImpl.clear();
    // only reset BASIC_DLL after the object had been destroyed
    if (bLastRef)
        BasicDLLImpl::BASIC_DLL = nullptr;
}

void BasicDLL::EnableBreak( bool bEnable )
{
    DBG_ASSERT( BasicDLLImpl::BASIC_DLL, "BasicDLL::EnableBreak: No instance yet!" );
    if (BasicDLLImpl::BASIC_DLL)
    {
        BasicDLLImpl::BASIC_DLL->bBreakEnabled = bEnable;
    }
}

void BasicDLL::SetDebugMode( bool bDebugMode )
{
    DBG_ASSERT( BasicDLLImpl::BASIC_DLL, "BasicDLL::EnableBreak: No instance yet!" );
    if (BasicDLLImpl::BASIC_DLL)
    {
        BasicDLLImpl::BASIC_DLL->bDebugMode = bDebugMode;
    }
}


void BasicDLL::BasicBreak()
{
    DBG_ASSERT( BasicDLLImpl::BASIC_DLL, "BasicDLL::EnableBreak: No instance yet!" );
#if HAVE_FEATURE_SCRIPTING
    if (!BasicDLLImpl::BASIC_DLL)
        return;

    // bJustStopping: if there's someone pressing STOP like crazy umpteen times,
    // but the Basic doesn't stop early enough, the box might appear more often...
    static bool bJustStopping = false;
    if (StarBASIC::IsRunning() && !bJustStopping
        && (BasicDLLImpl::BASIC_DLL->bBreakEnabled || BasicDLLImpl::BASIC_DLL->bDebugMode))
    {
        bJustStopping = true;
        StarBASIC::Stop();
        std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(nullptr,
                                                      VclMessageType::Info, VclButtonsType::Ok,
                                                      BasResId(IDS_SBERR_TERMINATED)));
        xInfoBox->run();
        bJustStopping = false;
    }
#endif
}

SbxAppData& GetSbxData_Impl()
{
    return *BasicDLLImpl::BASIC_DLL->xSbxAppData;
}

bool IsSbxData_Impl()
{
    return BasicDLLImpl::BASIC_DLL && BasicDLLImpl::BASIC_DLL->xSbxAppData;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
