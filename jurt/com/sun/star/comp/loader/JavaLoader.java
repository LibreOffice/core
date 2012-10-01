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

package com.sun.star.comp.loader;

import java.lang.reflect.Method;

import java.lang.reflect.InvocationTargetException;

import java.net.URLDecoder;

import com.sun.star.loader.CannotActivateFactoryException;
import com.sun.star.loader.XImplementationLoader;

import com.sun.star.registry.CannotRegisterImplementationException;
import com.sun.star.registry.XRegistryKey;

import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XInitialization;

import com.sun.star.uno.XComponentContext;
import com.sun.star.beans.XPropertySet;
import com.sun.star.util.XMacroExpander;

import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;

import com.sun.star.lib.util.StringHelper;

import com.sun.star.uno.AnyConverter;


/**
 * The <code>JavaLoader</code> class provides the functionality of the <code>com.sun.star.loader.Java</code>
 * service. Therefor the <code>JavaLoader</code> activates external UNO components which are implemented in Java.
 * The loader is used by the <code>ServiceManger</code>.
 * <p>
 * @see         com.sun.star.loader.XImplementationLoader
 * @see         com.sun.star.loader.Java
 * @see         com.sun.star.comp.servicemanager.ServiceManager
 * @since       UDK1.0
 */
public class JavaLoader implements XImplementationLoader,
                                   XServiceInfo,
                                   XInitialization
{
    private static final boolean DEBUG = false;

    private static final void DEBUG(String dbg) {
        if (DEBUG) System.err.println( dbg );
    }

    private static String[] supportedServices = {
        "com.sun.star.loader.Java"
    };

    protected XMultiServiceFactory multiServiceFactory = null;

    private XMacroExpander m_xMacroExpander = null;
    private static final String EXPAND_PROTOCOL_PREFIX = "vnd.sun.star.expand:";

    /** Expands macrofied url using the macro expander singleton.
     */
    private String expand_url( String url ) throws RuntimeException
    {
        if (url != null && url.startsWith( EXPAND_PROTOCOL_PREFIX ))
        {
            try
            {
                if (m_xMacroExpander == null)
                {
                    XPropertySet xProps =
                        UnoRuntime.queryInterface(
                            XPropertySet.class, multiServiceFactory );
                    if (xProps == null)
                    {
                        throw new com.sun.star.uno.RuntimeException(
                            "service manager does not support XPropertySet!",
                            this );
                    }
                    XComponentContext xContext = (XComponentContext)
                        AnyConverter.toObject(
                            new Type( XComponentContext.class ),
                            xProps.getPropertyValue( "DefaultContext" ) );
                    m_xMacroExpander = (XMacroExpander)AnyConverter.toObject(
                        new Type( XMacroExpander.class ),
                        xContext.getValueByName(
                            "/singletons/com.sun.star.util.theMacroExpander" )
                        );
                }
                // decode uric class chars
                String macro = URLDecoder.decode(
                    StringHelper.replace(
                        url.substring( EXPAND_PROTOCOL_PREFIX.length() ),
                        '+', "%2B" ) );
                // expand macro string
                String ret = m_xMacroExpander.expandMacros( macro );
                if (DEBUG)
                {
                    System.err.println(
                        "JavaLoader.expand_url(): " + url + " => " +
                        macro + " => " + ret );
                }
                return ret;
            }
            catch (com.sun.star.uno.Exception exc)
            {
                throw new com.sun.star.uno.RuntimeException(
                    exc.getMessage(), this );
            }
            catch (java.lang.Exception exc)
            {
                throw new com.sun.star.uno.RuntimeException(
                    exc.getMessage(), this );
            }
        }
        return url;
    }

    /** default constructor
     */

    /**
     * Creates a new instance of the <code>JavaLoader</code> class.
     * <p>
     */
    public JavaLoader() {}

    /**
     * Creates a new <code>JavaLoader</code> object. The specified <code>com.sun.star.lang.XMultiServiceFactory</code>
     * is the <code>ServiceManager</code> service which can be deliviert to all components the <code>JavaLoader</code> is
     * loading.
     * To set the <code>MultiServiceFactory</code> you can use the <code>com.sun.star.lang.XInitialization</code> interface, either.
     * <p>
     * @param   factory     the <code>ServiceManager</code>
     * @see     com.sun.star.comp.servicemanager.ServiceManager
     * @see     com.sun.star.lang.XInitialization
     */
    public JavaLoader(XMultiServiceFactory factory) {
        multiServiceFactory = factory;
    }

    /**
     * Unlike the original intention, the method could be called every time a new
     * <code>com.sun.star.lang.XMultiServiceFactory</code> should be set at the loader.
     * <p>
     * @param       args - the first parameter (args[0]) specifices the <code>ServiceManager</code>
     * @see         com.sun.star.lang.XInitialization
     * @see         com.sun.star.comp.servicemanager.ServiceManager
     */
    public void initialize( java.lang.Object[] args )
            throws com.sun.star.uno.Exception,
                   com.sun.star.uno.RuntimeException
    {
        if (args.length == 0) throw new com.sun.star.lang.IllegalArgumentException("No arguments specified");

        try {
            multiServiceFactory = (XMultiServiceFactory) AnyConverter.toObject(
                new Type(XMultiServiceFactory.class), args[0]);
        }
        catch (ClassCastException castEx) {
            throw new com.sun.star.lang.IllegalArgumentException(
                "The argument must be an instance of XMultiServiceFactory");
        }
    }

    /**
     * Supplies the implementation name of the component.
     * <p>
     * @return      the implementation name - here the class name
     * @see         com.sun.star.lang.XServiceInfo
     */
    public String getImplementationName()
            throws com.sun.star.uno.RuntimeException
    {
        return getClass().getName();
    }

    /**
     * Verifies if a given service is supported by the component.
     * <p>
     * @return      true,if service is suported - otherwise false
     * @param       serviceName     the name of the service that should be checked
     * @see         com.sun.star.lang.XServiceInfo
     */
    public boolean supportsService(String serviceName)
            throws com.sun.star.uno.RuntimeException
    {
        for ( int i = 0; i < supportedServices.length; i++ ) {
            if ( supportedServices[i].equals(serviceName) )
                return true;
        }
        return false;
    }

    /**
     * Supplies a list of all service names supported by the component
     * <p>
     * @return      a String array with all supported services
     * @see         com.sun.star.lang.XServiceInfo
     */
    public String[] getSupportedServiceNames()
            throws com.sun.star.uno.RuntimeException
    {
        return supportedServices;
    }

    /**
     * Provides a components factory.
     * The <code>JavaLoader</code> tries to load the class first. If a loacation URL is given the
     * RegistrationClassFinder is used to load the class. Otherwise the class is loaded thru the Class.forName
     * method.
     * To get the factory the inspects the class for the optional static member functions __getServiceFactory resp.
     * getServiceFactory (DEPRECATED).
     * If the function can not be found a default factory @see ComponentFactoryWrapper will be created.
     * <p>
     * @return      the factory for the component (@see com.sun.star.lang.XSingleServiceFactory)
     * @param       implementationName          the implementation (class) name of the component
     * @param       implementationLoaderUrl     the URL of the implementation loader. Not used.
     * @param       locationUrl                 points to an archive (JAR file) which contains a component
     * @param       xKey                        registry key
     * @see         com.sun.star.loader.XImplementationLoader
     * @see         com.sun.star.comp.loader.RegistrationClassFinder
     */
    public java.lang.Object activate( String implementationName,
                                      String implementationLoaderUrl,
                                      String locationUrl,
                                      XRegistryKey xKey )
        throws CannotActivateFactoryException,
               com.sun.star.uno.RuntimeException
    {
        locationUrl = expand_url( locationUrl );

        Object returnObject  = null;
        Class<?> clazz  ;

        DEBUG("try to get factory for " + implementationName);

        // first we must get the class of the implementation
        // 1. If a location URL is given it is assumed that this points to a JAR file.
        //    The components class name is stored in the manifest file.
        // 2. If only the implementation name is given, the class is loaded with the
        //    Class.forName() method. This is a hack to load bootstrap components.
        //    Normally a string must no be null.
        try {
            if ( locationUrl != null ) {
                clazz = RegistrationClassFinder.find( locationUrl );
                if (clazz == null) {
                    throw new CannotActivateFactoryException(
                        "Cannot activate jar " + locationUrl);
                }
            } else {
                clazz = Class.forName( implementationName );
                if (clazz == null) {
                    throw new CannotActivateFactoryException(
                        "Cannot find class " + implementationName);
                }
            }
        }
        catch (java.net.MalformedURLException e) {
            CannotActivateFactoryException cae = new CannotActivateFactoryException(
                    "Can not activate factory because " + e );
            cae.initCause(e);
            throw cae;
        }
        catch (java.io.IOException e) {
            CannotActivateFactoryException cae = new CannotActivateFactoryException(
                    "Can not activate factory because " + e );
            cae.initCause(e);
            throw cae;
        }
        catch (java.lang.ClassNotFoundException e) {
            CannotActivateFactoryException cae = new CannotActivateFactoryException(
                    "Can not activate factory because " + e );
            cae.initCause(e);
            throw cae;
        }

        Class<?>[] paramTypes = {String.class, XMultiServiceFactory.class, XRegistryKey.class};
        Object[] params = { implementationName, multiServiceFactory, xKey };

        // try to get factory from implemetation class
        // latest style: use the public static method __getComponentFactory
        // - new style: use the public static method __getServiceFactory
        // - old style: use the public static method getServiceFactory ( DEPRECATED )

        Method compfac_method = null;
        try
        {
             compfac_method = clazz.getMethod(
                "__getComponentFactory", new Class [] { String.class } );
        }
        catch ( NoSuchMethodException noSuchMethodEx) {}
        catch ( SecurityException secEx) {}

        Method method = null;
        if (null == compfac_method)
        {
            try {
                method = clazz.getMethod("__getServiceFactory", paramTypes);
            }
            catch ( NoSuchMethodException noSuchMethodEx) {
                method = null;
            }
            catch ( SecurityException secEx) {
                method = null;
            }
        }

        try {
            if (null != compfac_method)
            {
                Object ret = compfac_method.invoke( clazz, new Object [] { implementationName } );
                if (null == ret || !(ret instanceof XSingleComponentFactory))
                {
                    throw new CannotActivateFactoryException(
                        "No factory object for " + implementationName );
                }
                return ret;
            }
            else
            {
                if ( method == null ) {
                    method = clazz.getMethod("getServiceFactory", paramTypes);
                }

                Object oRet = method.invoke(clazz, params);

                if ( (oRet != null) && (oRet instanceof XSingleServiceFactory) ) {
                    returnObject = oRet;
                }
            }
        }
        catch ( NoSuchMethodException e) {
            throw new CannotActivateFactoryException("Can not activate the factory for "
                        + implementationName + " because " + e.toString() );
        }
        catch ( SecurityException e) {
            throw new CannotActivateFactoryException("Can not activate the factory for "
                        + implementationName + " because " + e.toString() );
        }
        catch ( IllegalAccessException e ) {
            throw new CannotActivateFactoryException("Can not activate the factory for "
                        + implementationName + " because " + e.toString() );
        }
        catch ( IllegalArgumentException e ) {
            throw new CannotActivateFactoryException("Can not activate the factory for "
                        + implementationName + " because " + e.toString() );
        }
        catch ( InvocationTargetException e ) {
            throw new CannotActivateFactoryException("Can not activate the factory for "
                        + implementationName + " because " + e.getTargetException().toString() );
        }

        return returnObject;
    }

    /**
     * Registers the component in a registry under a given root key. If the component supports the optional
     * methods __writeRegistryServiceInfo, writeRegistryServiceInfo (DEPRECATED), the call is delegated to that
     * method. Otherwise a default registration will be accomplished.
     * <p>
     * @return      true if registration is successfully - otherwise false
     * @param       regKey                  the root key under that the component should be registred.
     * @param       implementationLoaderUrl specifies the loader, the component is loaded by.
     * @param       locationUrl             points to an archive (JAR file) which contains a component
     * @see         ComponentFactoryWrapper
     */
    public boolean writeRegistryInfo( XRegistryKey regKey,
                                      String implementationLoaderUrl,
                                      String locationUrl )
            throws CannotRegisterImplementationException,
                   com.sun.star.uno.RuntimeException
    {
        locationUrl = expand_url( locationUrl );

        boolean success = false;

        try {

            Class<?> clazz = RegistrationClassFinder.find(locationUrl);
            if (null == clazz)
            {
                throw new CannotRegisterImplementationException(
                    "Cannot determine registration class!" );
            }

            Class<?>[] paramTypes = { XRegistryKey.class };
            Object[] params = { regKey };

            Method method  = clazz.getMethod("__writeRegistryServiceInfo", paramTypes);
            Object oRet = method.invoke(clazz, params);

            if ( (oRet != null) && (oRet instanceof Boolean) )
                success = ((Boolean) oRet).booleanValue();
        }
        catch (Exception e) {
            CannotRegisterImplementationException e2 =
                new CannotRegisterImplementationException(e.toString());
            e2.initCause(e);
            throw e2;
         }

        return success;
    }

    /**
     * Supplies the factory for the <code>JavaLoader</code>
     * <p>
     * @return  the factory for the <code>JavaLoader</code>
     * @param   implName        the name of the desired component
     * @param   multiFactory    the <code>ServiceManager</code> is delivered to the factory
     * @param   regKey          not used - can be null
     */
    public static XSingleServiceFactory getServiceFactory( String implName,
                                                           XMultiServiceFactory multiFactory,
                                                           XRegistryKey regKey)
    {
        if ( implName.equals(JavaLoader.class.getName()) )
            return new JavaLoaderFactory( multiFactory );

        return null;
    }

    /**
     * Registers the <code>JavaLoader</code> at the registry.
     * <p>
     * @return     true if registration succseeded - otherwise false
     * @param      regKey   root key under which the <code>JavaLoader</code> should be regidstered
     */
    public static boolean writeRegistryServiceInfo(XRegistryKey regKey) {
        boolean result = false;

        try {
            XRegistryKey newKey = regKey.createKey("/" + JavaLoader.class.getName() + "/UNO/SERVICE");

            for (int i=0; i<supportedServices.length; i++)
                newKey.createKey(supportedServices[i]);

            result = true;
        }
        catch (Exception ex) {
            if (DEBUG) System.err.println(">>>JavaLoader.writeRegistryServiceInfo " + ex);
        }

        return result;
    }
}

