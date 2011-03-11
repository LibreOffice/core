/*************************************************************************
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
************************************************************************/

package org.openoffice.test;

import com.sun.star.bridge.UnoUrlResolver;
import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.comp.helper.Bootstrap;
import com.sun.star.connection.NoConnectException;
import com.sun.star.frame.XDesktop;
import com.sun.star.lang.DisposedException;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import java.io.IOException;
import java.io.InputStream;
import java.io.PrintStream;
import java.util.Map;
import java.util.UUID;
import static org.junit.Assert.*;

/** Start up and shut down an OOo instance.

    Details about the OOo instance are tunneled in via
    org.openoffice.test.arg.... system properties.
*/
public final class OfficeConnection {
    /** Start up an OOo instance.
    */
    public void setUp() throws Exception {
        String sofficeArg = Argument.get("soffice");
        if (sofficeArg.startsWith("path:")) {
            description = "pipe,name=oootest" + UUID.randomUUID();
            ProcessBuilder pb = new ProcessBuilder(
                sofficeArg.substring("path:".length()), "-quickstart=no",
                "-nofirststartwizard", "-norestore",
                "-accept=" + description + ";urp",
                "-env:UserInstallation=" + Argument.get("user"),
                "-env:UNO_JAVA_JFW_ENV_JREHOME=true");
            String envArg = Argument.get("env");
            if (envArg != null) {
                Map<String, String> env = pb.environment();
                int i = envArg.indexOf("=");
                if (i == -1) {
                    env.remove(envArg);
                } else {
                    env.put(envArg.substring(0, i), envArg.substring(i + 1));
                }
            }
            process = pb.start();
            outForward = new Forward(process.getInputStream(), System.out);
            outForward.start();
            errForward = new Forward(process.getErrorStream(), System.err);
            errForward.start();
        } else if (sofficeArg.startsWith("connect:")) {
            description = sofficeArg.substring("connect:".length());
        } else {
            fail(
                "\"soffice\" argument \"" + sofficeArg +
                " starts with neither \"path:\" nor \"connect:\"");
        }
        XUnoUrlResolver resolver = UnoUrlResolver.create(
            Bootstrap.createInitialComponentContext(null));
        for (;;) {
            try {
                context = UnoRuntime.queryInterface(
                    XComponentContext.class,
                    resolver.resolve(
                        "uno:" + description +
                        ";urp;StarOffice.ComponentContext"));
                break;
            } catch (NoConnectException e) {}
            if (process != null) {
                assertNull(waitForProcess(process, 1000)); // 1 sec
            }
        }
    }

    /** Shut down the OOo instance.
    */
    public void tearDown()
        throws InterruptedException, com.sun.star.uno.Exception
    {
        boolean desktopTerminated = true;
        if (process != null) {
            if (context != null) {
                XMultiComponentFactory factory = context.getServiceManager();
                assertNotNull(factory);
                XDesktop desktop = UnoRuntime.queryInterface(
                    XDesktop.class,
                    factory.createInstanceWithContext(
                        "com.sun.star.frame.Desktop", context));
                context = null;
                try {
                    desktopTerminated = desktop.terminate();
                } catch (DisposedException e) {}
                    // it appears that DisposedExceptions can already happen
                    // while receiving the response of the terminate call
                desktop = null;
            } else {
                process.destroy();
            }
        }
        int code = 0;
        if (process != null) {
            code = process.waitFor();
        }
        boolean outTerminated = outForward == null || outForward.terminated();
        boolean errTerminated = errForward == null || errForward.terminated();
        assertTrue(desktopTerminated);
        assertEquals(0, code);
        assertTrue(outTerminated);
        assertTrue(errTerminated);
    }

    /** Obtain the component context of the running OOo instance.
    */
    public XComponentContext getComponentContext() {
        return context;
    }

    //TODO: get rid of this hack for legacy qa/unoapi tests
    public String getDescription() {
        return description;
    }

    private static Integer waitForProcess(Process process, final long millis)
        throws InterruptedException
    {
        final Thread t1 = Thread.currentThread();
        Thread t2 = new Thread("waitForProcess") {
                public void run() {
                    try {
                        Thread.currentThread().sleep(millis);
                    } catch (InterruptedException e) {}
                    t1.interrupt();
                }
            };
        boolean old = Thread.interrupted();
            // clear interrupted status, get old status
        t2.start();
        int n = 0;
        boolean done = false;
        try {
            n = process.waitFor();
            done = true;
        } catch (InterruptedException e) {}
        t2.interrupt();
        try {
            t2.join();
        } catch (InterruptedException e) {
            t2.join();
        }
        Thread.interrupted(); // clear interrupted status
        if (old) {
            t1.interrupt(); // reset old status
        }
        return done ? new Integer(n) : null;
    }

    private static final class Forward extends Thread {
        public Forward(InputStream in, PrintStream out) {
            super("process output forwarder");
            this.in = in;
            this.out = out;
        }

        public void run() {
            for (;;) {
                byte[] buf = new byte[1024];
                int n;
                try {
                    n = in.read(buf);
                } catch (IOException e) {
                    throw new RuntimeException("wrapping", e);
                }
                if (n == -1) {
                    break;
                }
                out.write(buf, 0, n);
            }
            done = true;
        }

        public boolean terminated() throws InterruptedException {
            join();
            return done;
        }

        private final InputStream in;
        private final PrintStream out;
        private boolean done = false;
    }

    private String description;
    private Process process = null;
    private Forward outForward = null;
    private Forward errForward = null;
    private XComponentContext context = null;
}
