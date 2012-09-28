/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "sal/config.h"

#if defined MACOSX
#include <cassert>
#include <limits>
#include <unistd.h>
#endif

#include "osl/process.h"
#include "sal/main.h"
#include "sal/types.h"

extern "C" {

void sal_detail_initialize(int argc, char ** argv) {
#if defined MACOSX
    // On Mac OS X, soffice can restart itself via exec (see restartOnMac in
    // desktop/source/app/app.cxx), which leaves all file descriptors open,
    // which in turn can have unwanted effects (see
    // <https://bugs.freedesktop.org/show_bug.cgi?id=50603> "Unable to update
    // LibreOffice without resetting user profile").  But closing fds in
    // restartOnMac before calling exec does not work, as additional threads
    // might still be running then, wich can still use those fds and cause
    // crashes.  Therefore, the simples solution is to close fds at process
    // start (as early as possible, so that no other threads have been created
    // yet that might already have opened some fds); this is done for all kinds
    // of processes here, not just soffice, but hopefully none of our processes
    // rely on being spawned with certain fds already open.  Unfortunately, Mac
    // OS X appears to have no better interface to close all fds (like
    // closefrom):
    long openMax = sysconf(_SC_OPEN_MAX);
    if (openMax == -1) {
        // Some random value, but hopefully sysconf never returns -1 anyway:
        openMax = 1024;
    }
    assert(openMax >= 0 && openMax <= std::numeric_limits< int >::max());
    for (int fd = 3; fd < openMax; ++fd) {
        close(fd);
    }
#endif

    osl_setCommandArgs(argc, argv);
}

void sal_detail_deinitialize() {}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
