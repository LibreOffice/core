/*************************************************************************
 *
 *  $RCSfile: JavaLoader.java,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-23 14:43:37 $
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

package com.sun.star.comp.loader;

import java.lang.reflect.Method;
import java.lang.reflect.Constructor;
import java.lang.reflect.Field;

import java.lang.reflect.InvocationTargetException;

import java.net.URLDecoder;

import com.sun.star.loader.CannotActivateFactoryException;
import com.sun.star.loader.XImplementationLoader;

import com.sun.star.registry.CannotRegisterImplementationException;
import com.sun.star.registry.RegistryKeyType;
import com.sun.star.registry.RegistryValueType;
import com.sun.star.registry.InvalidRegistryException;
import com.sun.star.registry.InvalidValueException;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.registry.XSimpleRegistry;

import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.ServiceNotRegisteredException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XInitialization;

import com.sun.star.uno.XComponentContext;
import com.sun.star.beans.XPropertySet;
import com.sun.star.util.XMacroExpander;

import com.sun.star.uno.XInterface;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;

import com.sun.star.lib.util.StringHelper;

import com.sun.star.uno.AnyConverter;

import java.io.IOException;
import java.net.MalformedURLException;

/**
 * The <code>JavaLoader</code> class provides the functionality of the <code>com.sun.star.loader.Java</code>
 * service. Therefor the <code>JavaLoader</code> activates external UNO components which are implemented in Java.
 * The loader is used by the <code>ServiceManger</code>.
 * <p>
 * @version     $Revision: 1.14 $ $ $Date: 2004-07-23 14:43:37 $
 * @author      Markus Herzog
 * @see         com.sun.star.loader.XImplementationLoader
 * @see         com.sun.star.loader.Java
 * @see         com.sun.star.comp.servicemanager.ServiceManager
 * @see         com.sun.star.lang.ServiceManager
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
                    XPropertySet xProps = (XPropertySet)
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
     * @return  new instance
     */
    public JavaLoader() {}

    /**
     * Creates a new <code>JavaLoader</code> object. The specified <code>com.sun.star.lang.XMultiServiceFactory</code>
     * is the <code>ServiceManager</code> service which can be deliviert to all components the <code>JavaLoader</code> is
     * loading.
     * To set the <code>MultiServiceFactory</code> you can use the <code>com.sun.star.lang.XInitialization</code> interface, either.
     * <p>
     * @return  new instance
     * @param   factory     the <code>ServiceManager</code>
     * @see     com.sun.star.lang.ServiceManager
     * @see     com.sun.star.lang.ServiceManager
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
     * @see         com.sun.star.lang.ServiceManager
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
     * @param       xKey
     * @see         com.sun.star.lang.XImplementationLoader
     * @see         com.sun.star.com.loader.RegistrationClassFinder
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
        Class clazz = null;

        DEBUG("try to get factory for " + implementationName);

        // first we must get the class of the implementation
        // 1. If a location URL is given it is assumed that this points to a JAR file.
        //    The components class name is stored in the manifest file.
        // 2. If only the implementation name is given, the class is loaded with the
        //    Class.forName() method. This is a hack to load bootstrap components.
        //    Normally a string must no be null.
        try {
            if ( locationUrl != null ) {
                // 1.
                clazz = RegistrationClassFinder.find( locationUrl );
            }
            else {
                // 2.
                clazz = Class.forName( implementationName );
            }
        }
        catch (java.net.MalformedURLException e) {
            CannotActivateFactoryException cae = new CannotActivateFactoryException(
                    "Can not activate factory because " + e.toString() );
            cae.fillInStackTrace();
            throw cae;
        }
        catch (java.io.IOException e) {
            CannotActivateFactoryException cae = new CannotActivateFactoryException(
                    "Can not activate factory because " + e.toString() );
            cae.fillInStackTrace();
            throw cae;
        }
        catch (java.lang.ClassNotFoundException e) {
            CannotActivateFactoryException cae = new CannotActivateFactoryException(
                    "Can not activate factory because " + e.toString() );
            cae.fillInStackTrace();
            throw cae;
        }

        if (null == clazz)
        {
            CannotActivateFactoryException cae =
                new CannotActivateFactoryException(
                    "Cannot determine activation class!" );
            cae.fillInStackTrace();
            throw cae;
        }

        Class[] paramTypes = {String.class, XMultiServiceFactory.class, XRegistryKey.class};
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
                return (XSingleComponentFactory)ret;
            }
            else
            {
                if ( method == null ) {
                    method = clazz.getMethod("getServiceFactory", paramTypes);
                }

                Object oRet = method.invoke(clazz, params);

                if ( (oRet != null) && (oRet instanceof XSingleServiceFactory) ) {
                    returnObject = (XSingleServiceFactory) oRet;
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

            Class clazz = RegistrationClassFinder.find(locationUrl);
            if (null == clazz)
            {
                throw new CannotRegisterImplementationException(
                    "Cannot determine registration class!" );
            }

            Class[] paramTypes = { XRegistryKey.class };
            Object[] params = { regKey };

            Method method  = clazz.getMethod("__writeRegistryServiceInfo", paramTypes);
            Object oRet = method.invoke(clazz, params);

            if ( (oRet != null) && (oRet instanceof Boolean) )
                success = ((Boolean) oRet).booleanValue();
        }
        catch (Exception e) {
            throw new CannotRegisterImplementationException( e.getMessage());
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

