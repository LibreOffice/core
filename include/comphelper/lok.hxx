/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_COMPHELPER_LOK_HXX
#define INCLUDED_COMPHELPER_LOK_HXX

#include <comphelper/comphelperdllapi.h>

// Interface between the LibreOfficeKit implementation called by LibreOfficeKit clients and other
// LibreOffice code.

namespace comphelper
{

namespace LibreOfficeKit
{

// Functions to be called only from the LibreOfficeKit implementation in desktop, not from other
// places in LibreOffice code.

COMPHELPER_DLLPUBLIC void setActive(bool bActive = true);

enum class statusIndicatorCallbackType { Start, SetValue, Finish };

COMPHELPER_DLLPUBLIC void setStatusIndicatorCallback(void (*callback)(void *data, statusIndicatorCallbackType type, int percent), void *data);


// Functions that can be called from arbitrary places in LibreOffice.

// Check whether the code is running as invoked through LibreOfficeKit.
COMPHELPER_DLLPUBLIC bool isActive();

// Status indicator handling. Even if in theory there could be several status indicators active at
// the same time, in practice there is only one at a time, so we don't handle any identification of
// status indicator in this API.
COMPHELPER_DLLPUBLIC void statusIndicatorStart();
COMPHELPER_DLLPUBLIC void statusIndicatorSetValue(int percent);
COMPHELPER_DLLPUBLIC void statusIndicatorFinish();

}
}

#endif // INCLUDED_COMPHELPER_LOK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
