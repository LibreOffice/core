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
import java.io.File;
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
    private final class PostprocessFailedException extends java.lang.RuntimeException {
        PostprocessFailedException() {
            super("This likely means that soffice crashed during the test.");
        }
    };
    /** Start up an OOo instance.
    */
    public void setUp() throws Exception {
        String sofficeArg = Argument.get("soffice");
        if (sofficeArg.startsWith("path:")) {
            description = "pipe,name=oootest" + UUID.randomUUID();
            ProcessBuilder pb = new ProcessBuilder(
                sofficeArg.substring("path:".length()), "--quickstart=no",
                "--nofirststartwizard", "--norestore", "--nologo", "--headless",
                "--accept=" + description + ";urp",
                "-env:UserInstallation=" + Argument.get("user"),
                "-env:UNO_JAVA_JFW_ENV_JREHOME=true");
            String workdirArg = Argument.get("workdir");
            if (workdirArg != null) {
                pb.directory(new File(workdirArg));
            }
            String envArg = Argument.get("env");
            if (envArg != null) {
                Map<String, String> env = pb.environment();
                int i = envArg.indexOf('=');
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
        try {
            boolean desktopTerminated = true;
            if (process != null) {
                if (context != null) {
                    XDesktop desktop = null;
                    try {
                        XMultiComponentFactory factory =
                            context.getServiceManager();
                        assertNotNull(factory);
                        desktop = UnoRuntime.queryInterface(XDesktop.class,
                            factory.createInstanceWithContext(
                                "com.sun.star.frame.Desktop", context));
                    } catch (DisposedException e) {
                        // it can happen that the Java bridge was disposed
                        // already, we want to ensure soffice.bin is killed
                        process.destroy();
                    }
                    context = null;
                    if (desktop != null) {
                        try {
                            desktopTerminated = desktop.terminate();
                            if (!desktopTerminated) {
                                // in case terminate() fails we would wait
                                // forever for the process to die, so kill it
                                process.destroy();
                            }
                            assertTrue(desktopTerminated);
                        } catch (DisposedException e) {}
                        // it appears that DisposedExceptions can already happen
                        // while receiving the response of the terminate call
                    }
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
            assertEquals(0, code);
            assertTrue(outTerminated);
            assertTrue(errTerminated);
        } finally {
            try {
                String sofficeArg = Argument.get("soffice");
                String workdir = Argument.get("workdir");
                String postprocesscommand = Argument.get("postprocesscommand");
                if(sofficeArg.startsWith("path:") && workdir != null && postprocesscommand != null) {
                    ProcessBuilder pb = new ProcessBuilder(
                        postprocesscommand,
                        sofficeArg.substring("path:".length()),
                        workdir);
                    Process postprocess = pb.start();
                    Forward ppoutForward = new Forward(postprocess.getInputStream(), System.out);
                    ppoutForward.start();
                    Forward pperrForward = new Forward(postprocess.getErrorStream(), System.err);
                    pperrForward.start();
                    postprocess.waitFor();
                    if(postprocess.exitValue() != 0)
                    {
                        // no ugly long java stacktrace needed here
                        PostprocessFailedException e = new PostprocessFailedException();
                        StackTraceElement[] newStackTrace = new StackTraceElement[0];
                        e.setStackTrace(newStackTrace);
                        throw e;
                    }
                }
            }
            catch(IOException e) {}
        }
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
                        Thread.sleep(millis);
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
// vim:set et sw=4 sts=4:
