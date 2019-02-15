/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

package com.sun.star.lib.util;

import java.util.LinkedList;

/**
 * Helper class to asynchronously execute finalize methods.
 *
 * <p>Current JVMs seem not to be robust against long-running finalize methods,
 * in that such long-running finalize methods may lead to OutOfMemoryErrors.
 * This class mitigates the problem by asynchronously shifting the bodies of
 * potentially long-running finalize methods into an extra thread.  Classes that
 * make use of this in their finalize methods are the proxies used in the
 * intra-process JNI UNO bridge and the inter-process Java URP UNO bridge (where
 * in both cases finalizers lead to synchronous UNO release calls).</p>
 *
 * <p>Irrespective whether JVMs are getting more mature and should no longer
 * have problems with long-running finalize methods, at least the JNI UNO bridge
 * needs some way to stop finalization of proxies (to C++ objects) well before
 * process exit, as provided by drain().</p>
 */
public final class AsynchronousFinalizer {
    public AsynchronousFinalizer() {
        thread = new Thread("AsynchronousFinalizer") {
                @Override
                public void run() {
                    for (;;) {
                        Job j;
                        synchronized (queue) {
                            for (;;) {
                                if (done) {
                                    return;
                                }
                                if (!queue.isEmpty()) {
                                    break;
                                }
                                try {
                                    queue.wait();
                                } catch (InterruptedException e) {
                                    return;
                                }
                            }
                            j = queue.remove(0);
                        }
                        try {
                            j.run();
                        } catch (Throwable e) {}
                    }
                }
            };
        thread.start();
    }

    /**
     * Add a job to be executed asynchronously.
     *
     * <p>The run method of the given job is called exactly once. If it terminates
     * abnormally by throwing any Throwable, that is ignored.</p>
     *
     * @param job represents the body of some finalize method; must not be null.
     */
    public void add(Job job) {
        synchronized (queue) {
            boolean first = queue.isEmpty();
            queue.add(job);
            if (first) {
                queue.notify();
            }
        }
    }

    public void drain() throws InterruptedException {
        synchronized (queue) {
            done = true;
            queue.notify();
        }
        // tdf#123481 Only join if we are not in our own thread, else we have a deadlock
        if (Thread.currentThread() != thread)
            thread.join();
    }

    /**
     * An interface to represent bodies of finalize methods.
     *
     * Similar to <code>Runnable</code>, except that the run method may throw any
     * <code>Throwable</code> (which is effectively ignored by
     * <code>AsynchronousFinalizer.add</code>, similar to any <code>Throwables</code>
     * raised by finalize being ignored).
    */
    public interface Job {
        void run() throws Throwable;
    }

    private final LinkedList<Job> queue = new LinkedList<Job>();
    private final Thread thread;
    private boolean done = false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
