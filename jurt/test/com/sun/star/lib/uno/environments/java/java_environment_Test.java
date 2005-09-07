/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: java_environment_Test.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:11:23 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
