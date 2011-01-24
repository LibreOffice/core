/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _FOREIGN_WINDOW_SYSTEM_HXX
#define _FOREIGN_WINDOW_SYSTEM_HXX

#include <X11/Xlib.h>

#if defined(__cplusplus)
extern "C" {
#endif

/* Initialize our atoms and determine if the current window manager is
 * providing FWS extension support.
 */

Bool
WMSupportsFWS (Display *display, int screen);

/* Send a client message to the FWS_COMM_WINDOW indicating the existance
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

#endif // _FOREIGN_WINDOW_SYSTEM_HXX

