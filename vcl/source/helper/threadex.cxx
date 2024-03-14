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

#include <vcl/threadex.hxx>
#include <vcl/svapp.hxx>

using namespace vcl;

SolarThreadExecutor::SolarThreadExecutor()
    : m_bTimeout(false)
{
}

SolarThreadExecutor::~SolarThreadExecutor() {}

IMPL_LINK_NOARG(SolarThreadExecutor, worker, void*, void)
{
    if (!m_bTimeout)
    {
        m_aStart.set();
        doIt();
        m_aFinish.set();
    }
}

void SolarThreadExecutor::execute()
{
    if (Application::IsMainThread())
    {
        m_aStart.set();
        doIt();
        m_aFinish.set();
    }
    else
    {
        m_aStart.reset();
        m_aFinish.reset();
        ImplSVEvent* nEvent = Application::PostUserEvent(LINK(this, SolarThreadExecutor, worker));
        SolarMutexGuard aGuard;
        SolarMutexReleaser aReleaser;
        if (m_aStart.wait() == osl::Condition::result_timeout)
        {
            m_bTimeout = true;
            Application::RemoveUserEvent(nEvent);
        }
        else
        {
            m_aFinish.wait();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
