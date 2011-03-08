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
