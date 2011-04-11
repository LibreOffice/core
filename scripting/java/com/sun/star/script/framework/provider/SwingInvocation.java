/*************************************************************************
*
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2000, 2011 Oracle and/or its affiliates.
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

package com.sun.star.script.framework.provider;

import javax.swing.SwingUtilities;

// On Mac OS X, AWT/Swing must not be accessed from the AppKit thread, so call
// SwingUtilities.invokeLater always on a fresh thread to avoid that problem
// (also, the current thread must not wait for that fresh thread to terminate,
// as that would cause a deadlock if this thread is the AppKit thread):
public final class SwingInvocation {
    public static void invoke(final Runnable doRun) {
        new Thread("SwingInvocation") {
            public void run() { SwingUtilities.invokeLater(doRun); }
        }.start();
    }

    private SwingInvocation() {}
}
