/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>
#include <vcl/dllapi.h>
#include <vcl/idle.hxx>

//IdleTask class to add a low priority Idle task
class VCL_DLLPUBLIC IdleTask
{
public:
    bool GetFlag() const;
    IdleTask();

    // Launch an Idle at reserved lowest priority of TaskPriority::TOOLKIT_DEBUG
    // and wait until it completes. Can be used to wait until pending Idles at
    // higher TaskPriority have completed.
    static void waitUntilIdleDispatched();

private:
    DECL_LINK(FlipFlag, Timer*, void);
    bool flag;
    Idle maIdle{ "testtool IdleTask" };
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
