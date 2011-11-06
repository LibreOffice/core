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


package com.sun.star.lib.uno.protocols.urp;

import java.io.IOException;

import java.util.Hashtable;


import com.sun.star.uno.IBridge;
import com.sun.star.uno.IEnvironment;
import com.sun.star.uno.Type;


class TestBridge implements IBridge {
    static public final boolean DEBUG = false;

    Hashtable _hashtable = new Hashtable();

    IEnvironment _source ;//= new com.sun.star.lib.uno.environments.java.java_environment(null);


    class MyEnv implements IEnvironment {
        public Object getContext() {
            return null;
        }

        public String getName() {
            return null;
        }

        public Object registerInterface(Object object, String oId[], Type type) {
            return null;
        }

        public void revokeInterface(String oId, Type type) {
        }

        public Object getRegisteredInterface(String oid, Type type) {
            Object object = _hashtable.get(oid);

            if(DEBUG) System.err.println("##### " + getClass().getName() + ".getRegisteredInterface:" + oid + " " + object);

            return object;
        }

        public String getRegisteredObjectIdentifier(Object object) {
            return null;
        }

        public void list() {
        }
    }

    TestBridge() {
        _source = new MyEnv();
    }

    public Object mapInterfaceTo(Object object, Type type) {
        if (object == null) {
            return null;
        } else {
            String oid = ">" + object.toString() + type.toString() + "<";
            _hashtable.put(oid, object);
            return oid;
        }
    }

    public Object mapInterfaceFrom(Object object, Type type) {
        String oid = (String)object;

        return _hashtable.get(oid);
    }

    public IEnvironment getSourceEnvironment() {
        return _source;
    }

    public IEnvironment getTargetEnvironment() {
        return null;
    }

    public void acquire() {}

    public void release() {}

    public void reset() throws IOException {}

    public void dispose() throws InterruptedException, IOException {}
}

