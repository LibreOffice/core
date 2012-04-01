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
            (new Type(
                "[][]com.sun.star.uno.XComponentContext", TypeClass.SEQUENCE).
             getZClass()));
        assertSame(
            "enum TypeClass", TypeClass.class,
            new Type("com.sun.star.uno.TypeClass", TypeClass.ENUM).getZClass());
        assertSame(
            "struct Uik", Uik.class,
            new Type("com.sun.star.uno.Uik", TypeClass.STRUCT).getZClass());
        assertSame(
            "exception Exception", com.sun.star.uno.Exception.class,
            (new Type("com.sun.star.uno.Exception", TypeClass.EXCEPTION).
             getZClass()));
        assertSame(
            "exception RuntimeException",
            com.sun.star.uno.RuntimeException.class,
            (new Type("com.sun.star.uno.RuntimeException", TypeClass.EXCEPTION).
             getZClass()));
        assertSame(
            "exception DeploymentException", DeploymentException.class,
            (new Type(
                "com.sun.star.uno.DeploymentException", TypeClass.EXCEPTION).
             getZClass()));
        assertSame(
            "interface XInterface", XInterface.class,
            (new Type("com.sun.star.uno.XInterface", TypeClass.INTERFACE).
             getZClass()));
        assertSame(
            "interface XComponentContext", XComponentContext.class,
            (new Type(
                "com.sun.star.uno.XComponentContext", TypeClass.INTERFACE).
             getZClass()));

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
