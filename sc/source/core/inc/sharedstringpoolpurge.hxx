/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <memory>

#include <vcl/timer.hxx>

namespace sc
{
/*
Calls svl::SharedStringPool::purge() after a delay when idle. Can be
used to compress repeated calls, as purge() may be somewhat expensive
with large documents. And since vcl links to svl, it's not possible
to use VCL timers in svl, so a separate class is needed.
*/
class SharedStringPoolPurge
{
public:
    SharedStringPoolPurge();
    ~SharedStringPoolPurge();
    void delayedPurge(const std::shared_ptr<svl::SharedStringPool>& pool);

private:
    void cleanup();
    std::vector<std::shared_ptr<svl::SharedStringPool>> mPoolsToPurge;
    Timer mTimer;
    static SharedStringPoolPurge* self;
    DECL_DLLPRIVATE_LINK(timerHandler, Timer*, void);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
