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
package com.sun.star.lib.uno.helper;

import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XInitialization;
import com.sun.star.registry.XRegistryKey;

import com.sun.star.uno.UnoRuntime;


/** Factory helper class supporting com.sun.star.lang.XServiceInfo and
    com.sun.star.lang.XSingleComponentFactory.

    This factory implementation does not support lang.XSingleServiceFactory.
*/
public class Factory
    extends ComponentBase
    implements XSingleComponentFactory, XServiceInfo
{
    private static final boolean DEBUG = false;

    /** Creates an object factory supporting interfaces
        com.sun.star.lang.XSingleComponentFactory and
        com.sun.star.lang.XServiceInfo

        @param impl_class
               implementation class
        @param impl_name
               implementation name
        @param supported_services
               services implemented
        @return
               object factory

        @since UDK 3.2.13
    */
    public static XSingleComponentFactory createComponentFactory(
        Class impl_class, String impl_name, String supported_services [] )
        throws com.sun.star.uno.RuntimeException
    {
        return new Factory( impl_class, impl_name, supported_services );
    }

    /** Creates an object factory supporting interfaces
        com.sun.star.lang.XSingleComponentFactory and
        com.sun.star.lang.XServiceInfo

        The implementation name is the name of the implementation class.

        @param impl_class
               implementation class
        @param supported_services
               services implemented
        @return
                object factory
    */
    public static XSingleComponentFactory createComponentFactory(
        Class impl_class, String supported_services [] )
        throws com.sun.star.uno.RuntimeException
    {
        return createComponentFactory(
            impl_class, impl_class.getName(), supported_services );
    }
    /** Writes component's implementation info to given registry key.

        @param impl_name
               name of implementation
        @param supported_services
               supported services of implementation
        @param xKey
               registry key to write to
        @return
                success
    */
    public static boolean writeRegistryServiceInfo(
        String impl_name, String supported_services [], XRegistryKey xKey )
    {
          try
        {
            XRegistryKey xNewKey = xKey.createKey( "/" + impl_name + "/UNO/SERVICES" );
            for ( int nPos = 0; nPos < supported_services.length; ++nPos )
            {
                xNewKey.createKey( supported_services[ nPos ] );
            }
            return true;
          }
          catch (com.sun.star.registry.InvalidRegistryException exc)
        {
            if (DEBUG)
            {
                System.err.println(
                    "##### " + Factory.class.getName() + ".writeRegistryServiceInfo -- exc: " +
                    exc.toString() );
            }
          }
        return false;
    }

    //==============================================================================================
    private String m_impl_name;
    private String [] m_supported_services;
    private Class<?> m_impl_class;
    private java.lang.reflect.Method m_method;
    private java.lang.reflect.Constructor m_ctor;

    private Factory(
        Class impl_class, String impl_name, String supported_services [] )
    {
        m_impl_name = impl_name;
        m_supported_services = supported_services;
        m_impl_class = impl_class;
        m_method = null;
        m_ctor = null;

        Class params [] = new Class [] { XComponentContext.class };

        try
        {
            // seeking for "public static Object __create( XComponentContext )"
            m_method = m_impl_class.getMethod( "__create", params );
            int mod = m_method.getModifiers();
            if (!m_method.getReturnType().equals( Object.class ) ||
                !java.lang.reflect.Modifier.isStatic( mod ) ||
                !java.lang.reflect.Modifier.isPublic( mod ))
            {
                m_method = null;
            }
        }
        catch (Exception exc)
        {
        }

        if (null == m_method)
        {
            try
            {
                // ctor with context
                m_ctor = m_impl_class.getConstructor( params );
            }
            catch (Exception exc)
            {
                // else take default ctor
            }
        }
    }

    //______________________________________________________________________________________________
    private final Object instantiate( XComponentContext xContext )
        throws com.sun.star.uno.Exception
    {
        try
        {
            if (DEBUG)
                System.out.print( "instantiating " + m_impl_class.toString() + " using " );
            if (null != m_method)
            {
                if (DEBUG)
                    System.out.println( "__create( XComponentContext )..." );
                return m_method.invoke( null, new Object [] { xContext } );
            }
            if (null != m_ctor)
            {
                if (DEBUG)
                    System.out.println( "ctor( XComponentContext )..." );
                return m_ctor.newInstance( new Object [] { xContext } );
            }
            if (DEBUG)
                System.out.println( "default ctor..." );
            return m_impl_class.newInstance(); // default ctor
        }
        catch (java.lang.reflect.InvocationTargetException exc)
        {
            Throwable targetException = exc.getTargetException();
            if (targetException instanceof java.lang.RuntimeException)
                throw (java.lang.RuntimeException)targetException;
            else if (targetException instanceof com.sun.star.uno.RuntimeException)
                throw (com.sun.star.uno.RuntimeException)targetException;
            else if (targetException instanceof com.sun.star.uno.Exception)
                throw (com.sun.star.uno.Exception)targetException;
            else
                throw new com.sun.star.uno.Exception( targetException.toString(), this );
        }
        catch (IllegalAccessException exc)
        {
            throw new com.sun.star.uno.RuntimeException( exc.toString(), this );
        }
        catch (InstantiationException exc)
        {
            throw new com.sun.star.uno.RuntimeException( exc.toString(), this );
        }
    }
    // XSingleComponentFactory impl
    //______________________________________________________________________________________________
    public final Object createInstanceWithContext(
        XComponentContext xContext )
        throws com.sun.star.uno.Exception
    {
        return instantiate( xContext );
    }
    //______________________________________________________________________________________________
    public final Object createInstanceWithArgumentsAndContext(
        Object arguments [], XComponentContext xContext )
        throws com.sun.star.uno.Exception
    {
        Object inst = instantiate( xContext );
        XInitialization xInit = UnoRuntime.queryInterface(
            XInitialization.class, inst );
        if (null == xInit)
        {
            throw new com.sun.star.lang.IllegalArgumentException(
                "cannot pass arguments to component; no XInitialization implemented!", this,
                (short)0 );
        }
        xInit.initialize( arguments );
        return inst;
    }

    // XServiceInfo impl
    //______________________________________________________________________________________________
    public final String getImplementationName()
    {
        return m_impl_name;
    }
    //______________________________________________________________________________________________
    public final boolean supportsService( String service_name )
    {
        for ( int nPos = 0; nPos < m_supported_services.length; ++nPos )
        {
            if (m_supported_services[ nPos ].equals( service_name ))
                return true;
        }
        return false;
    }
    //______________________________________________________________________________________________
    public final String [] getSupportedServiceNames()
    {
        return m_supported_services;
    }
}

