/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



package com.sun.star.lib.uno.environments.remote;

import complexlib.ComplexTestCase;

public final class JavaThreadPoolFactory_Test extends ComplexTestCase {
    public String[] getTestMethodNames() {
        return new String[] { "test" };
    }

    public void test() throws InterruptedException {
        ThreadId i1 = JavaThreadPoolFactory.getThreadId();
        assure(i1.equals(JavaThreadPoolFactory.getThreadId()));
        final ThreadId[] i2 = new ThreadId[1];
        new Thread() {
            public void run() {
                synchronized (i2) {
                    i2[0] = JavaThreadPoolFactory.getThreadId();
                    i2.notify();
                }
            }
        }.start();
        synchronized (i2) {
            while (i2[0] == null) {
                i2.wait();
            }
        }
        assure(!i1.equals(i2[0]));
    }
}
