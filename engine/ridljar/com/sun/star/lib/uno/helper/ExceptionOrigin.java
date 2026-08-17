/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package com.sun.star.lib.uno.helper;

public final class ExceptionOrigin {
    private ExceptionOrigin() {}

    public static String of(Throwable throwable) {
        StackTraceElement[] trace = throwable.getStackTrace();
        return trace.length == 0 ? "" : trace[0].toString();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
