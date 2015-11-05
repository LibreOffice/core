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

package com.sun.star.uno;

import org.junit.Test;
import static org.junit.Assert.*;

public final class Type_Test {
    @Test public void testZClass() {
        assertSame("VOID", void.class, new Type("void").getZClass());
        assertSame("BOOLEAN", boolean.class, new Type("boolean").getZClass());
        assertSame("BYTE", byte.class, new Type("byte").getZClass());
        assertSame("SHORT", short.class, new Type("short").getZClass());
        assertSame(
            "UNSIGNED SHORT", short.class,
            new Type("unsigned short").getZClass());
        assertSame("LONG", int.class, new Type("long").getZClass());
        assertSame(
            "UNSIGNED LONG", int.class, new Type("unsigned long").getZClass());
        assertSame("HYPER", long.class, new Type("hyper").getZClass());
        assertSame(
            "UNSIGNED HYPER", long.class,
            new Type("unsigned hyper").getZClass());
        assertSame("FLOAT", float.class, new Type("float").getZClass());
        assertSame("DOUBLE", double.class, new Type("double").getZClass());
        assertSame("CHAR", char.class, new Type("char").getZClass());
        assertSame("STRING", String.class, new Type("string").getZClass());
        assertSame("TYPE", Type.class, new Type("type").getZClass());
        assertSame("ANY", Object.class, new Type("any").getZClass());
        assertSame(
            "sequence of BOOLEAN", boolean[].class,
            new Type("[]boolean", TypeClass.SEQUENCE).getZClass());
        assertSame(
            "sequence of sequence of XComponentContext",
            XComponentContext[][].class,
            new Type("[][]com.sun.star.uno.XComponentContext", TypeClass.SEQUENCE).getZClass());
        assertSame(
            "enum TypeClass", TypeClass.class,
            new Type("com.sun.star.uno.TypeClass", TypeClass.ENUM).getZClass());
        assertSame(
            "struct Uik", Uik.class,
            new Type("com.sun.star.uno.Uik", TypeClass.STRUCT).getZClass());
        assertSame(
            "exception Exception", com.sun.star.uno.Exception.class,
            new Type("com.sun.star.uno.Exception", TypeClass.EXCEPTION).getZClass());
        assertSame(
            "exception RuntimeException",
            com.sun.star.uno.RuntimeException.class,
            new Type("com.sun.star.uno.RuntimeException", TypeClass.EXCEPTION).getZClass());
        assertSame(
            "exception DeploymentException", DeploymentException.class,
            new Type("com.sun.star.uno.DeploymentException", TypeClass.EXCEPTION).getZClass());
        assertSame(
            "interface XInterface", XInterface.class,
            new Type("com.sun.star.uno.XInterface", TypeClass.INTERFACE).getZClass());
        assertSame(
            "interface XComponentContext", XComponentContext.class,
            new Type("com.sun.star.uno.XComponentContext", TypeClass.INTERFACE). getZClass());

        assertSame(boolean.class, new Type(boolean.class).getZClass());
        assertSame(boolean.class, new Type(Boolean.class).getZClass());
        assertSame(boolean[].class, new Type(boolean[].class).getZClass());
        assertSame(boolean[].class, new Type(Boolean[].class).getZClass());
    }

    @Test public void testIsSupertypeOf() {
        Type ifc = new Type(com.sun.star.uno.XInterface.class);
        Type ctx = new Type(com.sun.star.uno.XComponentContext.class);
        Type exc = new Type(com.sun.star.uno.RuntimeException.class);
        assertTrue("LONG :> LONG", Type.LONG.isSupertypeOf(Type.LONG));
        assertFalse("not ANY :> XInterface", Type.ANY.isSupertypeOf(ifc));
        assertTrue("ANY :> ANY", Type.ANY.isSupertypeOf(Type.ANY));
        assertFalse("not ANY :> LONG", Type.ANY.isSupertypeOf(Type.LONG));
        assertFalse("not XInterface :> ANY", ifc.isSupertypeOf(Type.ANY));
        assertTrue("XInterface :> XInterface", ifc.isSupertypeOf(ifc));
        assertTrue("XInterface :> XComponentContext", ifc.isSupertypeOf(ctx));
        assertFalse(
            "not XComponentContext :> XInterface", ctx.isSupertypeOf(ifc));
        assertTrue(
            "XComponentContext :> XComponentContext", ctx.isSupertypeOf(ctx));
        assertFalse(
            "not XInterface :> RuntimeException", ifc.isSupertypeOf(exc));
    }
}
