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

#ifndef INCLUDED_VCL_UNX_GENERIC_WINDOW_FWS_HXX
#define INCLUDED_VCL_UNX_GENERIC_WINDOW_FWS_HXX

#include <X11/Xlib.h>

#if defined(__cplusplus)
extern "C" {
#endif

/* Initialize our atoms and determine if the current window manager is
 * providing FWS extension support.
 */

Bool
WMSupportsFWS (Display *display, int screen);

/* Send a client message to the FWS_COMM_WINDOW indicating the existence
 * of a new FWS client window.  Be careful to avoid BadWindow errors on
 * the XSendEvent in case the FWS_COMM_WINDOW root window property had
 * old/obsolete junk in it.
 */

Bool
RegisterFwsWindow (Display *display, Window window);

/* Add the FWS protocol atoms to the WMProtocols property for the window.
 */

void
AddFwsProtocols (Display *display, Window window);

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif // INCLUDED_VCL_UNX_GENERIC_WINDOW_FWS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
