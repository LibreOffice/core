/*************************************************************************
 *
 *  $RCSfile: ProxyFactory_Test.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-05-22 09:10:42 $
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
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package com.sun.star.lib.uno.bridges.java_remote;

import com.sun.star.uno.IQueryInterface;
import com.sun.star.uno.MappingException;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.XNamingService;
import complexlib.ComplexTestCase;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.lang.reflect.UndeclaredThrowableException;

public final class ProxyFactory_Test extends ComplexTestCase {
    public String getTestObjectName() {
        return getClass().getName();
    }

    public String[] getTestMethodNames() {
        return new String[] { "testQueryInterface", "testExceptionHandling" };
    }

    public void testQueryInterface() {
        TestRequestHandler handler = new TestRequestHandler();
        Type type = new Type(XNamingService.class);
        Object proxy = new ProxyFactory(handler).create("TestOID", type);
        assure("", proxy == ((IQueryInterface) proxy).queryInterface(type));
        assure("", proxy == UnoRuntime.queryInterface(type, proxy));
    }

    public void testExceptionHandling() throws Exception {
        TestRequestHandler handler = new TestRequestHandler();
        Object proxy = new ProxyFactory(handler).create(
            "TestOID", new Type(XNamingService.class));
        testExceptions(
            handler,
            proxy.getClass().getMethod("queryInterface",
                                       new Class[] { Type.class }),
            proxy, new Object[] { new Type(XInterface.class) },
            new Class[] { null, MappingException.class,
                          com.sun.star.uno.RuntimeException.class,
                          UndeclaredThrowableException.class,
                          NullPointerException.class,
                          UndeclaredThrowableException.class });
        testExceptions(
            handler,
            proxy.getClass().getMethod("getRegisteredObject",
                                       new Class[] { String.class }),
            proxy, new Object[] { "TestName" },
            new Class[] { null, MappingException.class,
                          com.sun.star.uno.RuntimeException.class,
                          com.sun.star.uno.Exception.class,
                          NullPointerException.class, Exception.class });
    }

    private void testExceptions(TestRequestHandler handler, Method method,
                                Object obj, Object[] args, Class[] exceptions)
        throws Exception
    {
        for (int i = 0; i < exceptions.length; ++i) {
            handler.setModus(i);
            testExceptionType(method, obj, args, exceptions[i]);
        }
    }

    private void testExceptionType(Method method, Object obj, Object[] args,
                                   Class exception) throws Exception {
        try {
            method.invoke(obj, args);
            assure("expected exception: " + exception, exception == null);
        } catch (InvocationTargetException e) {
            assure("unexpected exception: " + e.getTargetException(),
                   exception != null
                   && exception.isInstance(e.getTargetException()));
            // TODO  check stack trace
        }
    }

    private static final class TestRequestHandler implements RequestHandler {
        public Object sendRequest(String oid, Type type, String operation,
                                  Object[] args)
            throws Throwable
        {
            if (operation.equals("release")) {
                return null;
            }
            int m;
            synchronized (lock) {
                m = modus;
            }
            switch (m) {
            case 0:
                return operation.equals("getInstance") ? "TestResult" : null;
            case 1:
                // TODO  What is this test, with an obviously obsoleted
                // MappingException, good for?
                throw new MappingException();
            case 2:
                throw new com.sun.star.uno.RuntimeException();
            case 3:
                throw new com.sun.star.uno.Exception();
            case 4:
                throw new NullPointerException();
            default:
                throw new Throwable();
            }
        }

        public void setModus(int modus) {
            synchronized (lock) {
                this.modus = modus;
            }
        }

        private final Object lock = new Object();
        private int modus = 0;
    }
}
