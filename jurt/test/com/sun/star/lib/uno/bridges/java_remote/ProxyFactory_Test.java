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
        Object proxy = new ProxyFactory(handler, null).create("TestOID", type);
        assure("", proxy == ((IQueryInterface) proxy).queryInterface(type));
        assure("", proxy == UnoRuntime.queryInterface(type, proxy));
    }

    public void testExceptionHandling() throws Exception {
        TestRequestHandler handler = new TestRequestHandler();
        Object proxy = new ProxyFactory(handler, null).create(
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
