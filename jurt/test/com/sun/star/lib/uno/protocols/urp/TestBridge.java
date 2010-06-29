/*************************************************************************
 *
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
 *
 ************************************************************************/
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

