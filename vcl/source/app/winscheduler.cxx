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

#ifdef _WIN32

#include <sal/config.h>

#include <sal/log.hxx>
#include <vcl/winscheduler.hxx>

#include <svsys.h>
#include <win/saldata.hxx>
#include <win/salinst.h>

namespace
{
void PostMessageToComWnd(UINT nMsg)
{
    bool const ret = PostMessageW(GetSalData()->mpInstance->mhComWnd, nMsg, 0, 0);
    SAL_WARN_IF(!ret, "vcl.schedule", "ERROR: PostMessage() failed!");
}
}

void WinScheduler::SetForceRealTimer() { PostMessageToComWnd(SAL_MSG_FORCE_REAL_TIMER); }

void WinScheduler::PostDummyMessage() { PostMessageToComWnd(SAL_MSG_DUMMY); }

#endif // _WIN32

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
