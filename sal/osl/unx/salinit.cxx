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

#include "sal/config.h"

#if defined MACOSX
#include <cassert>
#include <limits>
#include <unistd.h>
#include <sys/stat.h>
#endif

#include "osl/process.h"
#include "sal/main.h"
#include "sal/types.h"

#if HAVE_SYSLOG_H
#include <string.h>
#include <syslog.h>
// from sal/osl/all/log.cxx:
extern bool sal_use_syslog;
#endif

extern "C" {

//From time.c
void sal_initGlobalTimer();

void sal_detail_initialize(int argc, char ** argv) {
#if defined MACOSX
    // On Mac OS X, soffice can restart itself via exec (see restartOnMac in
    // desktop/source/app/app.cxx), which leaves all file descriptors open,
    // which in turn can have unwanted effects (see
    // <https://bugs.freedesktop.org/show_bug.cgi?id=50603> "Unable to update
    // LibreOffice without resetting user profile").  But closing fds in
    // restartOnMac before calling exec does not work, as additional threads
    // might still be running then, which can still use those fds and cause
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
        struct stat s;
        if (fstat(fd, &s) != -1 && S_ISREG(s.st_mode))
            close(fd);
    }
#endif
    sal_initGlobalTimer();
#if HAVE_SYSLOG_H
    const char *use_syslog = getenv("SAL_LOG_SYSLOG");
    sal_use_syslog = use_syslog != NULL && !strcmp(use_syslog, "1");
    if (sal_use_syslog)
        openlog("libreoffice", 0, LOG_USER);
#endif

    osl_setCommandArgs(argc, argv);
}

void sal_detail_deinitialize() {}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
