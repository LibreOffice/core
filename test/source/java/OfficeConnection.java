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
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import java.io.IOException;
import java.io.InputStream;
import java.io.PrintStream;
import java.util.Map;
import java.util.UUID;
import static org.junit.Assert.*;

public final class OfficeConnection {
    public void setUp() throws Exception {
        description = "pipe,name=oootest" + UUID.randomUUID();
        ProcessBuilder pb = new ProcessBuilder(
            getArgument("path"), "-quickstart=no", "-nofirststartwizard",
            "-accept=" + description + ";urp",
            "-env:UserInstallation=" + getArgument("user"),
            "-env:UNO_JAVA_JFW_ENV_JREHOME=true",
            "-env:UNO_JAVA_JFW_ENV_CLASSPATH=true");
        String envArg = getArgument("env");
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
        XUnoUrlResolver resolver = UnoUrlResolver.create(
            Bootstrap.createInitialComponentContext(null));
        for (int i = 0;; ++i) {
            try {
                factory = UnoRuntime.queryInterface(
                    XMultiServiceFactory.class,
                    resolver.resolve(
                        "uno:" + description +
                        ";urp;StarOffice.ServiceManager"));
                break;
            } catch (NoConnectException e) {
                if (i == 600) { // 600 sec
                    throw e;
                }
            }
            assertNull(waitForProcess(process, 1000)); // 1 sec
        }
    }

    public void tearDown()
        throws InterruptedException, com.sun.star.uno.Exception
    {
        boolean desktopTerminated = true;
        if (factory != null) {
            XDesktop desktop = UnoRuntime.queryInterface(
                XDesktop.class,
                factory.createInstance("com.sun.star.frame.Desktop"));
            factory = null;
            try {
                desktopTerminated = desktop.terminate();
            } catch (DisposedException e) {}
                // it appears that DisposedExceptions can already happen while
                // receiving the response of the terminate call
            desktop = null;
        }
        Integer code = 0;
        if (process != null) {
            code = waitForProcess(process, 600000); // 600 sec
            process.destroy();
        }
        boolean outTerminated = outForward == null || outForward.terminated();
        boolean errTerminated = errForward == null || errForward.terminated();
        assertTrue(desktopTerminated);
        assertEquals(new Integer(0), code);
        assertTrue(outTerminated);
        assertTrue(errTerminated);
    }

    //TODO: get rid of this hack
    public String getDescription() {
        return description;
    }

    public XMultiServiceFactory getFactory() {
        return factory;
    }

    private static String getArgument(String name) {
        return System.getProperty("org.openoffice.test.arg." + name);
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

        InputStream in;
        PrintStream out;
        boolean done = false;
    }

    private String description;
    private Process process = null;
    private Forward outForward = null;
    private Forward errForward = null;
    private XMultiServiceFactory factory = null;
}
