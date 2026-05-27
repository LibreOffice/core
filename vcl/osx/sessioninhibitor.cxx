/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <osx/sessioninhibitor.hxx>

#include <sal/log.hxx>

void SessionManagerInhibitor::inhibit(bool bInhibit, std::u16string_view sReason,
                                      ApplicationInhibitFlags eType)
{
    (void)sReason;
    assert(eType == APPLICATION_INHIBIT_IDLE);

    if (bInhibit)
    {
        IOPMAssertionCreateWithName(kIOPMAssertionTypeNoDisplaySleep, kIOPMAssertionLevelOn,
                                    CFSTR("LibreOffice presentation running"), &mnAssertionID);
    }
    else
    {
        IOPMAssertionRelease(mnAssertionID);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
