// -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
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

package com.sun.star.bridges.jni_uno;

import com.sun.star.lib.util.AsynchronousFinalizer;
import com.sun.star.lib.util.NativeLibraryLoader;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.IEnvironment;
import com.sun.star.uno.IQueryInterface;


//==============================================================================
public final class JNI_proxy implements java.lang.reflect.InvocationHandler
{
    static {
        if (System.getProperty("java.vendor") == "The Android Project") {
            // See corresponding code in
            // javaunohelper/com/sun/star/comp/helper/Bootstrap.java for more
            // comments.

            boolean disable_dynloading = false;
            try {
                System.loadLibrary("lo-bootstrap");
            } catch (UnsatisfiedLinkError e) {
                disable_dynloading = true;
            }

            if (!disable_dynloading)
                NativeLibraryLoader.loadLibrary(JNI_info_holder.class.getClassLoader(),
                                                "java_uno");
        } else
            NativeLibraryLoader.loadLibrary(JNI_proxy.class.getClassLoader(),
                                            "java_uno");
    }
    protected static ClassLoader s_classloader =
        JNI_proxy.class.getClassLoader();
    protected static Class s_InvocationHandler [] =
        new Class [] { java.lang.reflect.InvocationHandler.class };

    protected long m_bridge_handle;
    protected IEnvironment m_java_env;
    protected long m_receiver_handle;
    protected long m_td_handle;
    protected Type m_type;
    protected String m_oid;
    protected Class m_class;

    //__________________________________________________________________________
    public static String get_stack_trace( Throwable throwable )
        throws Throwable
    {
        boolean current_trace = false;
        if (null == throwable)
        {
            throwable = new Throwable();
            current_trace = true;
        }
        java.io.StringWriter string_writer =
            new java.io.StringWriter();
        java.io.PrintWriter print_writer =
            new java.io.PrintWriter( string_writer, true );
        throwable.printStackTrace( print_writer );
        print_writer.flush();
        print_writer.close();
        string_writer.flush();
        String trace = string_writer.toString();
        if (current_trace)
        {
            // cut out first two lines
            int n = trace.indexOf( '\n' );
            n = trace.indexOf( '\n', n +1 );
            trace = trace.substring( n +1 );
        }
        return "\njava stack trace:\n" + trace;
    }

    //__________________________________________________________________________
    private native void finalize( long bridge_handle );

    //__________________________________________________________________________
    public void finalize()
    {
        AsynchronousFinalizer.add(new AsynchronousFinalizer.Job() {
                public void run() throws Throwable {
                    JNI_proxy.this.finalize( m_bridge_handle );
                }
            });
    }

    //__________________________________________________________________________
    private JNI_proxy(
        long bridge_handle, IEnvironment java_env,
        long receiver_handle, long td_handle, Type type, String oid )
    {
        m_bridge_handle = bridge_handle;
        m_java_env = java_env;
        m_receiver_handle = receiver_handle;
        m_td_handle = td_handle;
        m_type = type;
        m_oid = oid;
        m_class = m_type.getZClass();
    }

    //__________________________________________________________________________
    public static Object create(
        long bridge_handle, IEnvironment java_env,
        long receiver_handle, long td_handle, Type type, String oid,
        java.lang.reflect.Constructor proxy_ctor )
        throws Throwable
    {
        JNI_proxy handler = new JNI_proxy(
            bridge_handle, java_env, receiver_handle, td_handle, type, oid );
        Object proxy = proxy_ctor.newInstance( new Object [] { handler } );
        return java_env.registerInterface( proxy, new String [] { oid }, type );
    }

    //__________________________________________________________________________
    public static java.lang.reflect.Constructor get_proxy_ctor( Class clazz )
        throws Throwable
    {
        Class proxy_class = java.lang.reflect.Proxy.getProxyClass(
            s_classloader,
            new Class [] { clazz, IQueryInterface.class,
                           com.sun.star.lib.uno.Proxy.class } );
        return proxy_class.getConstructor( s_InvocationHandler );
    }

    //__________________________________________________________________________
    private native Object dispatch_call(
        long bridge_handle, String method, Object args [] )
        throws Throwable;

    // InvocationHandler impl
    //__________________________________________________________________________
    public Object invoke(
        Object proxy, java.lang.reflect.Method method, Object args [] )
        throws Throwable
    {
        Class decl_class = method.getDeclaringClass();
        String method_name = method.getName();

        if (Object.class.equals( decl_class ))
        {
            if (method_name.equals( "hashCode" ))
            {
                // int hashCode()
                return new Integer( m_oid.hashCode() );
            }
            else if (method_name.equals( "equals" ))
            {
                // boolean equals( Object obj )
                return isSame(args[0]);
            }
            else if (method_name.equals( "toString" ))
            {
                // String toString()
                return this.toString() + " [oid=" + m_oid +
                    ", type=" + m_type.getTypeName() + "]";
            }
        }
        // UNO interface call
        else if (decl_class.isAssignableFrom( m_class ))
        {
            // dispatch interface call
            return dispatch_call( m_bridge_handle, method_name, args );
        }
        // IQueryInterface impl
        else if (IQueryInterface.class.equals( decl_class ))
        {
            if (method_name.equals( "queryInterface" ))
            {
                // Object queryInterface( Type type )
                Object registered_proxy =
                    m_java_env.getRegisteredInterface( m_oid, (Type)args[ 0 ] );
                if (null == registered_proxy)
                {
                    return dispatch_call( m_bridge_handle, method_name, args );
                }
                else
                {
                    return registered_proxy;
                }
            }
            else if (method_name.equals( "isSame" ))
            {
                // boolean isSame( Object object )
                return isSame(args[0]);
            }
            else if (method_name.equals( "getOid" ))
            {
                // String getOid()
                return m_oid;
            }
        }

        throw new com.sun.star.uno.RuntimeException(
            "[jni_uno bridge error] unexpected call on proxy " +
            proxy.toString() + ": " + method.toString() );
    }

    private Boolean isSame(Object obj) {
        return new Boolean(obj != null
                           && m_oid.equals(UnoRuntime.generateOid(obj)));
    }
}

// vim:set shiftwidth=4 softtabstop=4 expandtab:
