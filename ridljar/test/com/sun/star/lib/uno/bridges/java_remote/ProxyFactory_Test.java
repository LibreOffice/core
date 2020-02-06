/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

package com.sun.star.lib.uno.bridges.java_remote;

import com.sun.star.uno.IQueryInterface;
import com.sun.star.uno.MappingException;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.XNamingService;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.lang.reflect.UndeclaredThrowableException;
import org.junit.Test;
import static org.junit.Assert.*;

public final class ProxyFactory_Test {
    @Test public void testQueryInterface() {
        TestRequestHandler handler = new TestRequestHandler();
        Type type = new Type(XNamingService.class);
        Object proxy = new ProxyFactory(handler, null).create("TestOID", type);
        assertSame(proxy, ((IQueryInterface) proxy).queryInterface(type));
        assertSame(proxy, UnoRuntime.queryInterface(type, proxy));
    }

    @Test public void testExceptionHandling() throws Exception {
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
                                Object obj, Object[] args, Class<?>[] exceptions)
        throws Exception
    {
        for (int i = 0; i < exceptions.length; ++i) {
            handler.setModus(i);
            testExceptionType(method, obj, args, exceptions[i]);
        }
    }

    private void testExceptionType(Method method, Object obj, Object[] args,
            Class<?> exception) throws Exception {
        try {
            method.invoke(obj, args);
            assertNull(exception);
        } catch (InvocationTargetException e) {
            assertNotNull(exception);
            assertTrue(exception.isInstance(e.getTargetException()));
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
