/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cassert>

#include <comphelper/automationinvokedzone.hxx>

namespace comphelper
{
namespace Automation
{
thread_local static int nActiveount = 0;

bool AutomationInvokedZone::isActive() { return nActiveount > 0; }

AutomationInvokedZone::AutomationInvokedZone()
{
    assert(nActiveount < 1000);
    nActiveount++;
}

AutomationInvokedZone::~AutomationInvokedZone()
{
    assert(nActiveount > 0);
    nActiveount--;
}
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
