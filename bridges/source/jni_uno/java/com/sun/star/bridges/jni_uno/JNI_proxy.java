/*************************************************************************
 *
 *  $RCSfile: JNI_proxy.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-18 19:07:04 $
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
package com.sun.star.bridges.jni_uno;

import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.IEnvironment;
import com.sun.star.uno.IQueryInterface;


//==================================================================================================
public final class JNI_proxy implements java.lang.reflect.InvocationHandler
{
    static { System.loadLibrary( "java_uno" ); }
    protected static ClassLoader s_system_classloader =
        ClassLoader.getSystemClassLoader();
    protected static Class s_InvocationHandler [] =
        new Class [] { java.lang.reflect.InvocationHandler.class };

    protected long m_bridge_handle;
    protected IEnvironment m_java_env;
    protected long m_receiver_handle;
    protected long m_td_handle;
    protected Type m_type;
    protected String m_oid;
    protected Class m_class;

    //______________________________________________________________________________________________
    public static String get_stack_trace( Throwable throwable )
        throws Throwable
    {
        boolean current_trace = false;
        if (null == throwable)
        {
            throwable = new Throwable();
            current_trace = true;
        }
        java.io.StringWriter string_writer = new java.io.StringWriter();
        java.io.PrintWriter print_writer = new java.io.PrintWriter( string_writer, true );
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

    //______________________________________________________________________________________________
    private native void finalize( long bridge_handle );
    //______________________________________________________________________________________________
    public void finalize()
    {
        finalize( m_bridge_handle );
    }
    //______________________________________________________________________________________________
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
    //______________________________________________________________________________________________
    public static Object create(
        long bridge_handle, IEnvironment java_env,
        long receiver_handle, long td_handle, Type type, String oid,
        java.lang.reflect.Constructor proxy_ctor )
        throws Throwable
    {
        JNI_proxy handler =
            new JNI_proxy( bridge_handle, java_env, receiver_handle, td_handle, type, oid );
        Object proxy = proxy_ctor.newInstance( new Object [] { handler } );
        return java_env.registerInterface( proxy, new String [] { oid }, type );
    }
    //______________________________________________________________________________________________
    public static java.lang.reflect.Constructor get_proxy_ctor( Class clazz )
        throws Throwable
    {
        Class proxy_class = java.lang.reflect.Proxy.getProxyClass(
            s_system_classloader,
            new Class [] { clazz, IQueryInterface.class, com.sun.star.lib.uno.Proxy.class } );
        return proxy_class.getConstructor( s_InvocationHandler );
    }

    //______________________________________________________________________________________________
    private native Object dispatch_call(
        long bridge_handle, String decl_class, String method, Object args [] )
        throws Throwable;
    // InvocationHandler impl
    //______________________________________________________________________________________________
    public Object invoke(
        Object proxy, java.lang.reflect.Method method, Object args [] )
        throws Throwable
    {
        Class decl_class = method.getDeclaringClass();
        String method_name = method.getName();

        if (Object.class.equals( decl_class ))
        {
            if (method_name.equals( "hashCode" )) // int hashCode()
            {
                return new Integer( System.identityHashCode( args[ 0 ] ) );
            }
            else if (method_name.equals( "equals" )) // boolean equals( Object obj )
            {
                return (proxy == args[ 0 ] ? Boolean.TRUE : Boolean.FALSE);
            }
            else if (method_name.equals( "toString" )) // String toString()
            {
                return this.toString() + " [oid=" + m_oid + ", type=" + m_type.getTypeName() + "]";
            }
        }
        // UNO interface call
        else if (decl_class.isAssignableFrom( m_class ))
        {
            // dispatch interface call
            return dispatch_call( m_bridge_handle, decl_class.getName(), method_name, args );
        }
        // IQueryInterface impl
        else if (IQueryInterface.class.equals( decl_class ))
        {
            if (method_name.equals( "queryInterface" )) // Object queryInterface( Type type )
            {
                // opt
                Object registered_proxy =
                    m_java_env.getRegisteredInterface( m_oid, (Type)args[ 0 ] );
                if (null == registered_proxy)
                {
                    return dispatch_call(
                        m_bridge_handle, "com.sun.star.uno.XInterface", method_name, args );
                }
                else
                {
                    return registered_proxy;
                }
            }
            else if (method_name.equals( "isSame" )) // boolean isSame( Object object )
            {
                Object right = args[ 0 ];
                if (proxy == right)
                    return Boolean.TRUE;
                if (m_oid.equals( UnoRuntime.generateOid( right ) ))
                    return Boolean.TRUE;
                return Boolean.FALSE;
            }
            else if (method_name.equals( "getOid" )) // String getOid()
            {
                return m_oid;
            }
        }

        throw new com.sun.star.uno.RuntimeException(
            "[jni_uno bridge error] unexpected call on proxy " + proxy.toString() + ": " +
            method.toString() );
    }
}
