
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

#include <osl/thread.h>
#include <sal/config.h>
#include <salframe.hxx>

#include <comphelper/solarmutex.hxx>

typedef void(^RuninmainBlock)(void);

union RuninmainResult
{
    void*                            pointer;
    bool                             boolean;
    struct SalFrame::SalPointerState state;

    RuninmainResult() {}
};

class AquaSalYieldMutex : public comphelper::SolarMutex
{
public:
    std::mutex              m_runInMainMutex;
    std::condition_variable m_aInMainCondition;
    std::condition_variable m_aResultCondition;
    bool                    m_wakeUpMain = false;
    bool                    m_resultReady = false;
    RuninmainBlock          m_aCodeBlock;
    RuninmainResult         m_aResult;

protected:
    virtual void            doAcquire( sal_uInt32 nLockCount ) override;
    virtual sal_uInt32      doRelease( bool bUnlockAll ) override;

public:
    AquaSalYieldMutex();
    virtual ~AquaSalYieldMutex() override;

    virtual bool IsCurrentThread() const override;
    bool         IsMainThread() const;

private:
    oslThreadIdentifier maMainThread;
    bool                mbNoYieldLock;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
