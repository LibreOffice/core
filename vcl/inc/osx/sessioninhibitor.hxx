/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vclpluginapi.h>
#include <string_view>
#include <assert.h>

#include <premac.h>
#include <IOKit/pwr_mgt/IOPMLib.h>
#include <postmac.h>

enum ApplicationInhibitFlags
{
    APPLICATION_INHIBIT_IDLE = (1 << 3) // Inhibit the session being marked as idle
};

class VCLPLUG_OSX_PUBLIC SessionManagerInhibitor
{
public:
    void inhibit(bool bInhibit, std::u16string_view sReason = u"",
                 ApplicationInhibitFlags eType = APPLICATION_INHIBIT_IDLE);

private:
    IOPMAssertionID mnAssertionID;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
