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

#include <helper/shareablemutex.hxx>

namespace framework
{
ShareableMutex::ShareableMutex()
{
    m_pMutexRef = new MutexRef;
    m_pMutexRef->acquire();
}

ShareableMutex::ShareableMutex(const ShareableMutex& rShareableMutex)
{
    m_pMutexRef = rShareableMutex.m_pMutexRef;
    m_pMutexRef->acquire();
}

ShareableMutex& ShareableMutex::operator=(const ShareableMutex& rShareableMutex)
{
    rShareableMutex.m_pMutexRef->acquire();
    m_pMutexRef->release();
    m_pMutexRef = rShareableMutex.m_pMutexRef;
    return *this;
}

void ShareableMutex::acquire() { m_pMutexRef->m_oslMutex.acquire(); }

void ShareableMutex::release() { m_pMutexRef->m_oslMutex.release(); }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
