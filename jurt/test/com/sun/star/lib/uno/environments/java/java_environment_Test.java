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



package com.sun.star.lib.uno.environments.java;

import com.sun.star.uno.Type;
import com.sun.star.uno.XInterface;
import complexlib.ComplexTestCase;

public final class java_environment_Test extends ComplexTestCase {
    public String getTestObjectName() {
        return getClass().getName();
    }

    public String[] getTestMethodNames() {
        return new String[] { "test" };
    }

    public void test() {
        java_environment env = new java_environment(null);

        Object obj = new Integer(3);
        String[] oid = new String[1];

        Object obj2 = env.registerInterface(obj, oid,
                                            new Type(XInterface.class));
        Object obj3 = env.registerInterface(obj, oid,
                                            new Type(XInterface.class));
        assure("register ordinary interface twice",
               obj2 == obj && obj3 == obj);

        assure("ask for registered interface",
               env.getRegisteredInterface(oid[0], new Type(XInterface.class))
               == obj);

        env.revokeInterface(oid[0], new Type(XInterface.class));
        env.revokeInterface(oid[0], new Type(XInterface.class));
        assure("revoke interface",
               env.getRegisteredInterface(oid[0], new Type(XInterface.class))
               == null);
    }
}
