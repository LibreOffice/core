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
