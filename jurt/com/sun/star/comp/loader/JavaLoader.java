/*************************************************************************
 *
 *  $RCSfile: JavaLoader.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kr $ $Date: 2000-11-28 14:47:05 $
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

import com.sun.star.loader.CannotActivateFactoryException;
import com.sun.star.loader.XImplementationLoader;

import com.sun.star.registry.CannotRegisterImplementationException;
import com.sun.star.registry.RegistryKeyType;
import com.sun.star.registry.RegistryValueType;
import com.sun.star.registry.InvalidRegistryException;
import com.sun.star.registry.InvalidValueException;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.registry.XSimpleRegistry;

import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.ServiceNotRegisteredException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XInitialization;

import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;

import java.io.IOException;
import java.net.MalformedURLException;

/**
 * The <code>JavaLoader</code> class provides the functionality of the <code>com.sun.star.loader.Java</code>
 * service. Therefor the <code>JavaLoader</code> activates external UNO components which are implemented in Java.
 * The loader is used by the <code>ServiceManger</code>.
 * <p>
 * @version     $Revision: 1.2 $ $ $Date: 2000-11-28 14:47:05 $
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
            multiServiceFactory = (XMultiServiceFactory) UnoRuntime.queryInterface(XMultiServiceFactory.class, args[0]);
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
        boolean needFactoryWrapper = false;
        Object returnObject  = null;
        Class clazz = null;

        DEBUG("try to get factory for " + implementationName);

        // first we must get the class of the implementation
        // 1. If a location URL is given it is assumed that this points to a JAR file.
        //    The components class name is stored in the manifest file.
        // 2. If only the implementation name is given, the class is loaded with the Class.forName() method
        try {
            if ( locationUrl != null ) {
                RegistrationClassFinder classFinder = new RegistrationClassFinder( locationUrl );
                // 1.
                clazz = classFinder.getRegistrationClass();
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

        Class[] paramTypes = {String.class, XMultiServiceFactory.class, XRegistryKey.class};
        Object[] params = { implementationName, multiServiceFactory, xKey };

        // try to get factory from implemetation class
        // - new style: use the public static method __getServiceFactory
        // - old style: use the public static method getServiceFactory ( DEPRECATED )
        Method method = null;
        try {
             method = clazz.getMethod("__getServiceFactory", paramTypes);
        }
        catch ( NoSuchMethodException noSuchMethodEx) {
            method = null;
        }
        catch ( SecurityException secEx) {
            method = null;
        }

        try {
            if ( method == null ) {
                method = clazz.getMethod("getServiceFactory", paramTypes);
            }

            Object oRet = method.invoke(clazz, params);

            if ( (oRet != null) && (oRet instanceof XSingleServiceFactory) ) {
                returnObject = (XSingleServiceFactory) oRet;
            }
        }
        catch ( NoSuchMethodException noSuchMethodEx) {
            needFactoryWrapper = true;
        }
        catch ( SecurityException secEx) {
            needFactoryWrapper = true;
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
        // if no method is found make a factory wrapper for the implementation and return it
        if ( needFactoryWrapper ) {
            DEBUG ("create factory wrapper for " + implementationName);
            ComponentFactoryWrapper wrapp = new ComponentFactoryWrapper( implementationName, locationUrl );
            returnObject = wrapp.getServiceFactory(implementationName,
                                                   multiServiceFactory,
                                                   xKey);
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
        boolean success = false;

        try {

            RegistrationClassFinder classFinder = new RegistrationClassFinder(locationUrl);
            Class clazz = classFinder.getRegistrationClass();

            Class[] paramTypes = { XRegistryKey.class };
            Object[] params = { regKey };

            Method method  = clazz.getMethod("__writeRegistryServiceInfo", paramTypes);
            Object oRet = method.invoke(clazz, params);

            if ( (oRet != null) && (oRet instanceof Boolean) )
                success = ((Boolean) oRet).booleanValue();
        }
        catch (Exception e) {
            // default registration
            ComponentFactoryWrapper wrapp = new ComponentFactoryWrapper(null, locationUrl);
            success = wrapp.writeRegistryServiceInfo(regKey);
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

//**********************************************************************************************************
//**********************************************************************************************************

/**
 * The <code>ComponentFactoryWrapper</code> class provides methods to create a factory for a component and
 * the registration at a registry in a default manner. The class is used by the <code>JavaLoader</code> if the
 * a component does not comes with its own methods for creating a factory or for the registration.
 * <p>
 * @version     $Revision: 1.2 $ $ $Date: 2000-11-28 14:47:05 $
 * @author      Markus Herzog
 * @since       UDK1.0
 */
class ComponentFactoryWrapper
        implements  XServiceInfo,
                    XSingleServiceFactory
{
    private static final boolean DEBUG = false;
    private String serviceName = null;
    private String implName = null;
    private String locationUrl = null;
    private Class serviceClass = null;
    private XMultiServiceFactory aServiceManager = null;
    private boolean bServiceManagerAlreadySet = false;

    /**
     * Constructor for a <code>ComponentFactoryWrapper</code> object.
     * <p>
     * @param      name     specifies the name of the implementation. If no loaction URL is given
     *                      the implementation name must be the class name of the component.
     * @param      lUrl     points to an archive (JAR file) which contains a component.
     */
    public ComponentFactoryWrapper( String name, String lUrl ) {
        implName = name;
        locationUrl = lUrl;
    }

    private static final void DEBUG( String dbg ) {
        if (DEBUG) System.err.println(">>>ComponentFactoryWrapper - " + dbg);
    }

    /**
     * Registers the component at the registry. First it is verified if component includes the optional
     * method __writeRegistryServiceInfo ( writeRegistryServiceInfo - DEPRECATED ). If so the call is delegated
     * to this method. Otherwise the component will be registered under its implementation name.
     * <p>
     * @return     true if registration succseeded - otherwise false
     * @param      regKey   root key under which the <code>JavaLoader</code> should be regidstered
     */
    public boolean writeRegistryServiceInfo( XRegistryKey regKey )
            throws CannotRegisterImplementationException,
                   com.sun.star.uno.RuntimeException
    {
        boolean success = false;
        boolean defaultRegistration = false;

        Class clazz = getServiceClass();
        if ( clazz != null ) {
            try {
                Class[] paramTypes = { XRegistryKey.class };
                Object[] params = { regKey };

                Method method = null;

                try {
                    method = clazz.getMethod("__writeRegistryServiceInfo", paramTypes);
                }
                catch (NoSuchMethodException noSuchMethodEx) {
                    method = null;
                }
                catch (SecurityException securityEx) {
                    method = null;
                }

                if (method == null) {
                    method = clazz.getMethod("writeRegistryServiceInfo", paramTypes);
                }

                Object oRet = method.invoke(clazz, params);

                if ( (oRet != null) && (oRet instanceof Boolean) )
                    success = ((Boolean) oRet).booleanValue();
            }
            catch (NoSuchMethodException noSuchMethodEx) {
                defaultRegistration = true;
            }
            catch (SecurityException securityEx) {
                defaultRegistration = true;
            }
            catch (IllegalAccessException e) {
                throw new CannotRegisterImplementationException("Can not register " + implName + " because " + e.toString() );
            }
            catch (IllegalArgumentException e) {
                throw new CannotRegisterImplementationException("Can not register " + implName + " because " + e.toString() );
            }
            catch (InvocationTargetException e) {
                throw new CannotRegisterImplementationException("Can not register " + implName + " because " + e.toString() );
            }


            if (defaultRegistration) {
                try {
                    XRegistryKey newKey = regKey.createKey("/" + implName + "/UNO/SERVICES");
                    String names[] = getServiceNames();

                    for (int i=0; i<names.length; i++)
                        newKey.createKey( names[i] );

                     success = true;
                }
                catch (com.sun.star.registry.InvalidRegistryException e) {
                    throw new CannotRegisterImplementationException("Can not register " + implName + " because " + e.toString() );
                }

            }
        }
        return success;
    }

    /**
     * Supplies the factory for the component.  First it is verified if component includes the optional
     * method __getServiceFactory ( getServiceFactory - DEPRECATED ). If so the call is delegated
     * to this method. Otherwise the ComponentFactoryWrapper is returned.
     * <p>
     * @return  the factory for the component
     * @param   impName     the components implementation name
     * @param   multiFac    specifices the ServiceManager
     * @param   regKey      only used if the call is delegated
     */
    public Object getServiceFactory( String impName,
                                     XMultiServiceFactory multiFac,
                                     XRegistryKey regKey)
    {
        aServiceManager = multiFac;

        try {
            Class clazz = getServiceClass();
            Class[] paramTypes = { String.class, XMultiServiceFactory.class, XRegistryKey.class};
            Object[] params = { impName, multiFac, regKey };

            // try to get factory form implemetation class
            Method method = null;

            try {
                method = clazz.getMethod("__getServiceFactory", paramTypes);
            }
            catch (NoSuchMethodException noSuchMethodEx) {
                method = null;
            }
            catch (SecurityException securityEx) {
                method = null;
            }

            if (method == null) {
                method = clazz.getMethod("getServiceFactory", paramTypes);
            }

            return UnoRuntime.queryInterface( XSingleServiceFactory.class,method.invoke(clazz, params) );

        }
        catch (NoSuchMethodException noSuchMethodEx) {
        }
        catch (SecurityException securityEx) {
        }
        catch (IllegalAccessException e) {
        }
        catch (IllegalArgumentException e) {
        }
        catch (InvocationTargetException e) {
        }

        // if any execption occurred this will be returned
        return this;
    }

    /**
     * set the ServiceManager at the component,
     * if the method __setSericeFactory( XMultiServiceFactor ) is found
     */
    /**
     * Set the <code>ServiceManager</code> at the component. For that the component must support
     * the <code>__setServiceManager</code> methode. The method is called after a new instance of the
     * component is created.
     * <p>
     * @param      obj  the newly created component
     * @see        createInstanceWithArguments
     * @see        createInstance
     */
    private void setServiceManager( Object obj ) {
            Class clazz = getServiceClass();
            Class paramTypes[] = { XMultiServiceFactory.class };
            Object[] args = { aServiceManager };
        try {
            clazz.getDeclaredMethod( "__setServiceManager", paramTypes).invoke(obj, args);
        }
        catch (NoSuchMethodException e) {}
        catch (SecurityException e) {}
        catch (IllegalAccessException e) {}
        catch (IllegalArgumentException e) {}
        catch (InvocationTargetException e) {}
    }

    /**
     * Tries to instantiate a component by its implementation name. For it the Beans.instantiate
     * method is called. If any exception occured a com.sun.star.uno.Exception will be thrown.
     * <p>
     * @return
     * @see     java.beans.Beans
     */
    private Object createInstanceWithDefaultConstructor()
                    throws  com.sun.star.uno.Exception,
                               com.sun.star.uno.RuntimeException
    {
        Object resObj = null;
        try {
            DEBUG ("try to instantiate " + implName );

            resObj = java.beans.Beans.instantiate(getClass().getClassLoader(), implName);
        }
        catch (IOException e) {
            if (DEBUG) e.printStackTrace();
            throw new com.sun.star.uno.Exception("Can not create an instance of " + implName + "\n" +
                                                  e.toString());
        }
        catch (ClassNotFoundException e) {
            if (DEBUG) e.printStackTrace();
            throw new com.sun.star.uno.Exception("Can not create an instance of " + implName + "\n" +
                                                  e.toString());
        }
        catch (NoSuchMethodError e) {
            if (DEBUG) e.printStackTrace();
            throw new com.sun.star.uno.Exception("Can not create an instance of " + implName + "\n" +
                                                  e.toString());
        }

        return resObj;
    }


    /**
     * Creates a new instance of the component.
     * <p>
     * @return  newly instanciated component
     * @see     com.sun.star.lang.XSimpleServiceFactory
     */
    public Object createInstance()
            throws com.sun.star.uno.Exception,
                   com.sun.star.uno.RuntimeException
    {
        Object resObj = createInstanceWithDefaultConstructor();

        setServiceManager( resObj );

        return resObj;
    }

    /**
     * Creates a new component with arguments. How the arguments are commited depends on the component.
     * For that the following order is used:
     * 1.   The component supports the XInitialization interface: An object is created using Beans.instantiate.
     *      The arguments are set with the <code>initialize</code> method.
     * 2.   The component has a constructor with an array of objects as a argument.
     * 3.   The component has any other constructor which can take the arguments.
     * <p>
     * @return     newly created component
     * @param      args the arguments which should be used
     */
    public Object createInstanceWithArguments( Object[] args )
            throws com.sun.star.uno.Exception,
                   com.sun.star.uno.RuntimeException
    {
        java.lang.Object resObj = null;

        boolean useDefaultCtor = false;
        // first we try if the component supports the XInitialization interface
        Class clazz = getServiceClass();

        if ( clazz.isAssignableFrom( XInitialization.class ) )
            useDefaultCtor = true;
        else {
            try {
                Class parameterTypes[] = { Class.class, Object.class };
                Method queryMeth = clazz.getMethod("queryInterface", parameterTypes );
                useDefaultCtor = true;
            }
            catch (NoSuchMethodException e) {}
            catch (SecurityException e) {}
        }

        if (useDefaultCtor) {
            resObj = createInstanceWithDefaultConstructor();
            XInitialization iniObj = (XInitialization) UnoRuntime.queryInterface( XInitialization.class, resObj );
            iniObj.initialize( args );
        }
        else {
            // try the constructor A(java.lang.Object[])
            Class[] clazzParams = { Object[].class };
            java.lang.reflect.Constructor ctor = null;

            try {
                 ctor = clazz.getConstructor( clazzParams );
            }
            catch (NoSuchMethodException noSuchMethodEx) {
                ctor = null;
            }
            catch (SecurityException securityEx) {
                ctor = null;
            }

            if (ctor == null) {
                // look for the first matching constructor
                java.lang.reflect.Constructor ctors[] = clazz.getDeclaredConstructors();

                int i=0;
                while ( (i<ctors.length) && (resObj==null) ) {
                    try {
                        resObj = ctors[i++].newInstance( args );
                    }
                    catch (InstantiationException instantiationEx) {}
                    catch (IllegalAccessException illegalAccessEx) {}
                    catch (IllegalArgumentException illegalArgumentEx) {}
                    catch (InvocationTargetException invocationTargetEx) {}
                }

                if (resObj == null)
                    throw new com.sun.star.uno.Exception( "Can not create instance with arguments for " + implName );
            }
            else
            {
                try {
                    resObj = ctor.newInstance( args );
                }
                catch (InstantiationException e) {
                    throw new com.sun.star.uno.Exception("Can not create instance with arguments for "
                                                         + implName + " because " + e.toString() );
                }
                catch (IllegalAccessException e) {
                    throw new com.sun.star.uno.Exception("Can not create instance with arguments for "
                                                         + implName + " because " + e.toString() );
                }
                catch (IllegalArgumentException e) {
                    throw new com.sun.star.uno.Exception("Can not create instance with arguments for "
                                                         + implName + " because " + e.toString() );
                }
                catch (InvocationTargetException e) {
                    throw new com.sun.star.uno.Exception("Can not create instance with arguments for "
                                                         + implName + " because " + e.toString() );
                }
            }

            setServiceManager( resObj );

        }

        return resObj;
    }

    /**
     * Supplies the implementation name of the component.
     * <p>
     * @return      the name of the implementation
     * @see         com.sun.star.lang.XServiceInfo
     */
    public String getImplementationName()
            throws com.sun.star.uno.RuntimeException
    {
        return implName;
    }

    /**
     * Verifies whether or not a service is supportet by the component.
     * <p>
     * @return      true if the service is supported - otherwise false
     * @param       requestedService    name of the requested service
     */
    public boolean supportsService(String requestedService)
            throws com.sun.star.uno.RuntimeException
    {
        boolean found = false;
        int i = 0;

        if (requestedService == null)
            throw new com.sun.star.uno.RuntimeException("no service requested");

        String names[] = getServiceNames();

        if (names.length == 0)
            throw new com.sun.star.uno.RuntimeException("no service name found");

        while (i<names.length && !found)
            found = names[i++].equals(requestedService);

        return found;
    }

    /**
     * Provides a list of all supported services by the component.
     * <p>
     * @return      list of the service names which are supported by the component
     * @see         com.sun.star.lang.XServiceInfo
     */
    public String[] getSupportedServiceNames()
            throws com.sun.star.uno.RuntimeException
    {
        return getServiceNames();
    }

    /**
     * Supplies the class of the component. If the location URL is specified the
     * <code>com.sun.star.comp.loader.RegistrationClassFinder</code> is used to obtain the class.
     * Otherwise the <code>java.lang.Class.forName</code> is called with implementation name.
     * <p>
     * @return      the class of the component
     * @see         com.sun.star.comp.loader.RegistrationClassFinder
     */
    Class getServiceClass()
            throws com.sun.star.uno.RuntimeException
    {
        if (serviceClass == null) {
            try {
                if ( locationUrl != null ) {
                    RegistrationClassFinder classFinder = new RegistrationClassFinder( locationUrl );
                    serviceClass = classFinder.getRegistrationClass();
                }
                else {
                    serviceClass = Class.forName( implName );
                }
            }
            catch ( java.net.MalformedURLException e ) {
                throw new com.sun.star.uno.RuntimeException( "can't get the class " + implName + ".\n" + e );
            }
            catch ( java.io.IOException e ) {
                throw new com.sun.star.uno.RuntimeException( "can't get the class " + implName + ".\n" + e );
            }
            catch ( java.lang.ClassNotFoundException e ) {
                throw new com.sun.star.uno.RuntimeException( "can't get the class " + implName + ".\n" + e );
            }
        }

        return serviceClass;
    }

    /**
     * Extract the service names supported by the component. The optional static member
     * <code>__serviceName</code> ( <code>serviceName</code> DEPRECATED ) is used to specify the supported
     * service names. If no member can be found the implementation name is return.
     * <p>
     * @return      a list with the supported service names
     */
    public String[] getServiceNames()
            throws com.sun.star.uno.RuntimeException
    {
        String result[]   = null;
        try {
            Class clazz = getServiceClass();
            Object attrib = null;
            try {
                attrib = clazz.getDeclaredField("__serviceName").get(clazz);
            } catch (NoSuchFieldException e) {
//********************* DEPRECATED ******************************************
                attrib = clazz.getDeclaredField("serviceName").get(clazz);
//***************************************************************************
            }

            if ( attrib instanceof String ) {
                String str = (String) attrib;
                result = new String[1];
                result[0] = str;
            } else
                result = (String[]) attrib; // expecting an array of Strings - otherwise an exception will be thrown
        }
        catch (NoSuchFieldException e) {}
        catch (SecurityException e) {}
        catch (IllegalAccessException e) {}

        if (result == null) {
            result = new String[1];
            result[0] = implName;
        }

        return result;
    }

    public String toString() {
      String result = super.toString();
      result += " implementation name: " +getImplementationName();
      result += " supported services:";

      String[] services = getSupportedServiceNames();
      for (int i=0; i<services.length; i++)
        result += " " + services[i];

      return result;
    }
}

