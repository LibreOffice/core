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

import complexlib.ComplexTestCase;

public final class Type_Test extends ComplexTestCase {
    public String[] getTestMethodNames() {
        return new String[] { "testZClass", "testIsSupertypeOf" };
    }

    public void testZClass() {
        assure("VOID", new Type("void").getZClass() == void.class);
        assure("BOOLEAN", new Type("boolean").getZClass() == boolean.class);
        assure("BYTE", new Type("byte").getZClass() == byte.class);
        assure("SHORT", new Type("short").getZClass() == short.class);
        assure("UNSIGNED SHORT",
               new Type("unsigned short").getZClass() == short.class);
        assure("LONG", new Type("long").getZClass() == int.class);
        assure("UNSIGNED LONG",
               new Type("unsigned long").getZClass() == int.class);
        assure("HYPER", new Type("hyper").getZClass() == long.class);
        assure("UNSIGNED HYPER",
               new Type("unsigned hyper").getZClass() == long.class);
        assure("FLOAT", new Type("float").getZClass() == float.class);
        assure("DOUBLE", new Type("double").getZClass() == double.class);
        assure("CHAR", new Type("char").getZClass() == char.class);
        assure("STRING", new Type("string").getZClass() == String.class);
        assure("TYPE", new Type("type").getZClass() == Type.class);
        assure("ANY", new Type("any").getZClass() == Object.class);
        assure("sequence of BOOLEAN",
               new Type("[]boolean", TypeClass.SEQUENCE).getZClass()
               == boolean[].class);
        assure("sequence of sequence of XComponentContext",
               new Type("[][]com.sun.star.uno.XComponentContext",
                        TypeClass.SEQUENCE).getZClass()
               == XComponentContext[][].class);
        assure("enum TypeClass",
               new Type("com.sun.star.uno.TypeClass",
                        TypeClass.ENUM).getZClass() == TypeClass.class);
        assure("struct Uik",
               new Type("com.sun.star.uno.Uik", TypeClass.STRUCT).getZClass()
               == Uik.class);
        assure("exception Exception",
               new Type("com.sun.star.uno.Exception",
                        TypeClass.EXCEPTION).getZClass()
               == com.sun.star.uno.Exception.class);
        assure("exception RuntimeException",
               new Type("com.sun.star.uno.RuntimeException",
                        TypeClass.EXCEPTION).getZClass()
               == com.sun.star.uno.RuntimeException.class);
        assure("exception DeploymentException",
               new Type("com.sun.star.uno.DeploymentException",
                        TypeClass.EXCEPTION).getZClass()
               == DeploymentException.class);
        assure("interface XInterface",
               new Type("com.sun.star.uno.XInterface",
                        TypeClass.INTERFACE).getZClass() == XInterface.class);
        assure("interface XComponentContext",
               new Type("com.sun.star.uno.XComponentContext",
                        TypeClass.INTERFACE).getZClass()
               == XComponentContext.class);

        assure(new Type(boolean.class).getZClass() == boolean.class);
        assure(new Type(Boolean.class).getZClass() == boolean.class);
        assure(new Type(boolean[].class).getZClass() == boolean[].class);
        assure(new Type(Boolean[].class).getZClass() == boolean[].class);
    }

    public void testIsSupertypeOf() {
        Type ifc = new Type(com.sun.star.uno.XInterface.class);
        Type ctx = new Type(com.sun.star.uno.XComponentContext.class);
        Type exc = new Type(com.sun.star.uno.RuntimeException.class);
        assure("LONG :> LONG", Type.LONG.isSupertypeOf(Type.LONG));
        assure("not ANY :> XInterface", !Type.ANY.isSupertypeOf(ifc));
        assure("ANY :> ANY", Type.ANY.isSupertypeOf(Type.ANY));
        assure("not ANY :> LONG", !Type.ANY.isSupertypeOf(Type.LONG));
        assure("not XInterface :> ANY", !ifc.isSupertypeOf(Type.ANY));
        assure("XInterface :> XInterface", ifc.isSupertypeOf(ifc));
        assure("XInterface :> XComponentContext", ifc.isSupertypeOf(ctx));
        assure("not XComponentContext :> XInterface", !ctx.isSupertypeOf(ifc));
        assure("XComponentContext :> XComponentContext",
               ctx.isSupertypeOf(ctx));
        assure("not XInterface :> RuntimeException", !ifc.isSupertypeOf(exc));
    }
}
