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

#include <sot/object.hxx>

SotObject::SotObject()
    : nOwnerLockCount(0)
    , bInClose(false)
{
}

SotObject::~SotObject() = default;

/** When the OwnerLock is decremented to zero, the DoClose method is called.
 *  This happens independently of the lock or RefCount. If the OwnerLock
 *  counter != zero, no DoClose is called by <SotObject::FuzzyLock>.
 *
 *  bLock - true, lock. false, unlock.
 */
void SotObject::OwnerLock(bool bLock)
{
    if (bLock)
    {
        nOwnerLockCount++;
        AddFirstRef();
    }
    else if (nOwnerLockCount)
    {
        if (0 == --nOwnerLockCount)
            DoClose();
        ReleaseRef();
    }
}

bool SotObject::DoClose()
{
    bool bRet = false;
    if (!bInClose)
    {
        tools::SvRef<SotObject> xHoldAlive(this);
        bInClose = true;
        bRet = Close();
        bInClose = false;
    }
    return bRet;
}

bool SotObject::Close() { return true; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
