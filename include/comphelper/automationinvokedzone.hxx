/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_COMPHELPER_AUTOMATIONINVOKEDZONE_HXX
#define INCLUDED_COMPHELPER_AUTOMATIONINVOKEDZONE_HXX

#include <comphelper/comphelperdllapi.h>

// Helpers for use when calling into LO from an Automation (OLE
// Automation) client.

namespace comphelper::Automation
{
// Create an object of this class at the start of a function directly invoked from an Automation
// client.

class COMPHELPER_DLLPUBLIC AutomationInvokedZone
{
public:
    AutomationInvokedZone();

    ~AutomationInvokedZone();

    static bool isActive();
};
}

#endif // INCLUDED_COMPHELPER_AUTOMATIONINVOKEDZONE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
