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

package com.sun.star.lib.uno.bridges.java_remote;

import com.sun.star.bridge.XBridge;
import com.sun.star.lib.util.AsynchronousFinalizer;
import com.sun.star.uno.IQueryInterface;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;
import java.lang.reflect.Proxy;

/**
 * A factory for proxies specific to the <code>java_remote_bridge</code>.
 *
 * <p>Eventually, this class should be united with all other proxy classes
 * specific to certain bridges (for example, the JNI bridge), resulting in a
 * generic proxy class.</p>
 */
final class ProxyFactory {
    public ProxyFactory(RequestHandler requestHandler, XBridge bridge) {
        this.requestHandler = requestHandler;
        this.bridge = bridge;
    }

    public Object create(String oid, Type type) {
        return Proxy.newProxyInstance(
            getClass().getClassLoader(),
            new Class[] { com.sun.star.lib.uno.Proxy.class,
                          IQueryInterface.class, type.getZClass() },
            new Handler(oid, type));
    }

    public boolean isProxy(Object obj) {
        if (Proxy.isProxyClass(obj.getClass())) {
            InvocationHandler h = Proxy.getInvocationHandler(obj);
            return h instanceof Handler && ((Handler) h).matches(this);
        } else {
            return false;
        }
    }

    public static XBridge getBridge(Object obj) {
        if (Proxy.isProxyClass(obj.getClass())) {
            InvocationHandler h = Proxy.getInvocationHandler(obj);
            if (h instanceof Handler) {
                return ((Handler) h).getBridge();
            }
        }
        return null;
    }

    static int getDebugCount() {
        synchronized (debugCountLock) {
            return debugCount;
        }
    }

    private static void incrementDebugCount() {
        synchronized (debugCountLock) {
            ++debugCount;
        }
    }

    private static void decrementDebugCount() {
        synchronized (debugCountLock) {
            --debugCount;
        }
    }

    private final class Handler implements InvocationHandler {
        public Handler(String oid, Type type) {
            this.oid = oid;
            this.type = type;
            incrementDebugCount();
        }

        public boolean matches(ProxyFactory factory) {
            return ProxyFactory.this == factory;
        }

        public XBridge getBridge() {
            return bridge;
        }

        public Object invoke(Object proxy, Method method, Object[] args)
            throws Throwable
        {
            if (method.equals(METHOD_EQUALS) || method.equals(METHOD_IS_SAME)) {
                return new Boolean(
                    args[0] != null
                    && oid.equals(UnoRuntime.generateOid(args[0])));
            } else if (method.equals(METHOD_HASH_CODE)) {
                return new Integer(oid.hashCode());
            } else if (method.equals(METHOD_TO_STRING)) {
                return "[Proxy:" + System.identityHashCode(proxy) + "," + oid
                    + "," + type + "]";
            } else if (method.equals(METHOD_QUERY_INTERFACE)) {
                // See the comment in java_remote_bridge.mapInterfaceTo for one
                // reason why this implementation must not satisfy a request for
                // a super-interface with a proxy itself:
                return args[0].equals(type) ? proxy
                    : request("queryInterface", args);
            } else if (method.equals(METHOD_GET_OID)) {
                return oid;
            } else {
                return request(method.getName(), args);
            }
        }

        protected void finalize() {
            AsynchronousFinalizer.add(new AsynchronousFinalizer.Job() {
                    public void run() throws Throwable {
                        try {
                            request("release", null);
                        } finally {
                            decrementDebugCount();
                        }
                    }
                });
        }

        private Object request(String operation, Object[] args) throws Throwable
        {
            Object res = requestHandler.sendRequest(oid, type, operation, args);
            // Avoid early finalization of this object, while an invoke ->
            // request call is still ongoing; as finalize also calls request,
            // this should fulfil the condition from The Java Language
            // Specification, 3rd ed., that "if an object's finalizer can result
            // in synchronization on that object, then that object must be alive
            // and considered reachable whenever a lock is held on it:"
            synchronized (this) {
                ++dummy;
            }
            return res;
        }

        private final String oid;
        private final Type type;
        private int dummy = 0;
    }

    private static final Method METHOD_EQUALS;
    private static final Method METHOD_HASH_CODE;
    private static final Method METHOD_TO_STRING;
    private static final Method METHOD_QUERY_INTERFACE;
    private static final Method METHOD_IS_SAME;
    private static final Method METHOD_GET_OID;
    static {
        try {
            METHOD_EQUALS = Object.class.getMethod(
                "equals", new Class[] { Object.class });
            METHOD_HASH_CODE = Object.class.getMethod("hashCode", null);
            METHOD_TO_STRING = Object.class.getMethod("toString", null);
            METHOD_QUERY_INTERFACE = IQueryInterface.class.getMethod(
                "queryInterface", new Class[] { Type.class });
            METHOD_IS_SAME = IQueryInterface.class.getMethod(
                "isSame", new Class[] { Object.class });
            METHOD_GET_OID = IQueryInterface.class.getMethod("getOid", null);
        } catch (NoSuchMethodException e) {
            throw new ExceptionInInitializerError(e);
        }
    }

    private static final Object debugCountLock = new Object();
    private static int debugCount = 0;

    private final RequestHandler requestHandler;
    private final XBridge bridge;
}
