/*************************************************************************
 *
 *  $RCSfile: Type_Test.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-08-13 17:23:54 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
               new Type("unsigned short").getZClass() == null);
        assure("LONG", new Type("long").getZClass() == int.class);
        assure("UNSIGNED LONG", new Type("unsigned long").getZClass() == null);
        assure("HYPER", new Type("hyper").getZClass() == long.class);
        assure("UNSIGNED HYPER",
               new Type("unsigned hyper").getZClass() == null);
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
    }

    public void testIsSupertypeOf() {
        Type ifc = new Type(com.sun.star.uno.XInterface.class);
        Type ctx = new Type(com.sun.star.uno.XComponentContext.class);
        Type exc = new Type(com.sun.star.uno.RuntimeException.class);
        assure("", !Type.ANY.isSupertypeOf(ifc));
        assure("", !ifc.isSupertypeOf(Type.ANY));
        assure("", ifc.isSupertypeOf(ctx));
        assure("", !ctx.isSupertypeOf(ifc));
        assure("", ctx.isSupertypeOf(ctx));
        assure("", !ifc.isSupertypeOf(exc));
    }
}
