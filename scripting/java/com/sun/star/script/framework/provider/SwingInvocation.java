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

package com.sun.star.script.framework.provider;

import javax.swing.SwingUtilities;

// On Mac OS X, AWT/Swing must not be accessed from the AppKit thread, so call
// SwingUtilities.invokeLater always on a fresh thread to avoid that problem
// (also, the current thread must not wait for that fresh thread to terminate,
// as that would cause a deadlock if this thread is the AppKit thread):
public final class SwingInvocation {

    public static void invoke(final Runnable doRun) {
        new Thread("SwingInvocation") {
            @Override
            public void run() {
                SwingUtilities.invokeLater(doRun);
            }
        } .start();
    }

    private SwingInvocation() {}
}