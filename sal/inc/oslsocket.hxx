/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <rtl/ustring.h>
#include <osl/socket.h>

/** Retrieve this machines hostname as fully qualified domain name (FQDN).
    Note that this might be slower than calling osl_getLocalHostname
    since DNS needs to be queried to determine the FQDN.
    @param  strLocalHostname out-parameter. The string that receives the local host name.
    @retval sal_True upon success
    @retval sal_False
*/
oslSocketResult osl_getLocalHostnameFQDN(rtl_uString** strLocalHostname);

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */