/*************************************************************************
 *
 *  $RCSfile: ServiceManager.java,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:27:51 $
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

package com.sun.star.comp.servicemanager;

import com.sun.star.uno.IQueryInterface;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.Uik;

import com.sun.star.container.XSet;
import com.sun.star.container.XContentEnumerationAccess;
import com.sun.star.container.XEnumeration;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.container.XElementAccess;

import com.sun.star.lang.XComponent;
import com.sun.star.lang.XEventListener;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XSingleServiceFactory;

import com.sun.star.registry.XRegistryKey;
import com.sun.star.registry.XSimpleRegistry;

import com.sun.star.loader.CannotActivateFactoryException;
import com.sun.star.loader.XImplementationLoader;

import com.sun.star.comp.loader.RegistrationClassFinder;

import com.sun.star.lang.XEventListener;
//import com.sun.star.lang.EventObject;

import com.sun.star.comp.loader.JavaLoader;

import java.lang.reflect.InvocationTargetException;

/**
 * The <code>ServiceManager</code> class is an implmentation of the <code>ServiceManager</code>the central class needed for
 * implementing or using UNO components in Java.
 * <p>
 * The Methods <code>queryInterface</code> and <code>isSame</code> delegate
 * calls to the implementing objects and are used instead of casts
 * and identity comparisons.
 * <p>
 * @version     $Revision: 1.1.1.1 $ $ $Date: 2000-09-18 15:27:51 $
 * @author      Markus Herzog
 * @see         com.sun.star.lang.XMultiServiceFactory
 * @see         com.sun.star.container.XSet
 * @see         com.sun.star.container.XContentEnumerationAccess
 * @see         com.sun.star.lang.XComponent
 * @see         com.sun.star.lang.XServiceInfo
 * @see         com.sun.star.lang.XInitialization
 * @since       UDK1.0
 */
public class ServiceManager implements XMultiServiceFactory,
                                       XSet,
                                       XContentEnumerationAccess,
                                       XComponent,
                                       XServiceInfo,
                                       XInitialization
{
    private static final boolean DEBUG = false;

    private static final void DEBUG (String dbg) {
        if (DEBUG) System.err.println( dbg );
    }

    private static com.sun.star.uno.Type UNO_TYPE = null;

    XImplementationLoader loader = null;

    static String[] supportedServiceNames = {
            "com.sun.star.lang.MultiServiceFactory",
            "com.sun.star.lang.ServiceManager"
    };

    java.util.Vector    eventListener;
    java.util.Hashtable factoriesByImplNames;
    java.util.Hashtable factoriesByServiceNames;  // keys:

    /**
     * Creates a new instance of the <code>ServiceManager</code>.
     */
    public ServiceManager() {
        eventListener           = new java.util.Vector();
        factoriesByImplNames    = new java.util.Hashtable();
        factoriesByServiceNames = new java.util.Hashtable();
    }

    /**
     * Returns the service factory for the <code>ServiceManager</code>. If the given implementation name
     * does not equal to the <code>ServiceManagers</code> class name null will be returned.
     * <p>
     * @return     the factory for the <code>ServiceManager</code>.
     * @param      implName     the implementation name of the of the service.
     *                          Must be equal to <code>com.sun.star.comp.servicemanager.ServicManager</code>
     * @param      multiFactory refernce of the <code>MultiServiceFactory</code>. This parameter will be ignored.
     * @param      regKey       the root key of the registry. This parameter will be ignored.
     */
      public static XSingleServiceFactory getServiceFactory( String implName,
                                                           XMultiServiceFactory multiFactory,
                                                           XRegistryKey regKey)
    {
        if ( implName.equals(ServiceManager.class.getName()) )
            return new ServiceManagerFactory();

        return null;
    }


    /**
     * Supplies a Java component loader. The loader component must be enlisted at the <code>ServiceManager</code> before.
     * <p>
     * @return      a new instance of the Java component loader
     * @see         com.sun.star.loader.Java
     */
    private XImplementationLoader getLoader()
                throws  com.sun.star.uno.Exception,
                          com.sun.star.uno.RuntimeException
    {
        Object[] param = { this };
        DEBUG("make loader");
        Object loaderObj = createInstanceWithArguments( "com.sun.star.loader.Java", param );

        if (loaderObj == null)
            throw new com.sun.star.uno.Exception("Can get an instance of com.sun.star.loader.Java");

        return (XImplementationLoader) UnoRuntime.queryInterface( XImplementationLoader.class, loaderObj );
    }

    /**
     * Registers a list of components given by their class names.
     * <p>
     * @param   newImpls    list of the components that should be registered, given by their class names.
     *                      If any exception occured during the registration, the process will be canceled.
     * @see     com.sun.star.container.XSet
     */
    public void addFactories( String[] newImpls )
                    throws com.sun.star.uno.Exception
    {
        for (int i=0; i<newImpls.length; i++) {
            DEBUG ("try to add " + newImpls[i] );
            Object newFactory = null;

            try {
                if (loader == null)
                    loader = getLoader();

                newFactory = loader.activate( newImpls[i], null, null, null );
            }
            catch (com.sun.star.uno.Exception e) {

//****************************** BEGIN DEPRECATED ******************************************

                try {
                    // try to get the class of the implementation
                    Class clazz = Class.forName( newImpls[i] );

                    Class[] methodClassParam = { String.class, XMultiServiceFactory.class, XRegistryKey.class };
                    java.lang.reflect.Method getFactoryMeth = null;
                    try {
                        getFactoryMeth = clazz.getMethod("__getServiceFactory", methodClassParam);
                    }
                    catch (NoSuchMethodException noSuchMethodEx) {
                        getFactoryMeth = null;
                    }
                    catch (SecurityException securityExc) {
                        getFactoryMeth = null;
                    }

                    if (getFactoryMeth == null)
                        getFactoryMeth = clazz.getMethod("getServiceFactory", methodClassParam);

                    Object[] methodParams = { newImpls[i], this, null };
                    newFactory = getFactoryMeth.invoke( clazz, methodParams );
                }
                catch (NoSuchMethodException ex) {}
                catch (SecurityException ex) {}
                catch (ClassNotFoundException ex) {}
                catch (IllegalAccessException ex) {}
                catch (IllegalArgumentException ex) {}
                catch (InvocationTargetException ex) {}

//****************************** END DEPRECATED ******************************************
            }

            if ( newFactory == null )
                throw new com.sun.star.loader.CannotActivateFactoryException("Can not get factory for " +  newImpls[i]);

            insert( newFactory );
        } // end of for ...
    }

    /**
     * The method is used to add components to the <code>ServiceManager</code>. The first argument indicates a <code>SimpleRegistry</code>.
     * The components which should be added will be searched under the <i>Implementations</i> key in the registry.
     * <p>
     * @param   args    the first argument ( args[0] ) specifices the SimpleRegistry object
     * @see     com.sun.star.lang.XInitialization
     * @see     com.sun.star.lang.RegistryServiceManager
     * @see     com.sun.star.lang.XSimpleRegistry
     */
    public void initialize( Object args[] )
                throws  com.sun.star.uno.Exception,
                        com.sun.star.uno.RuntimeException {
        XSimpleRegistry xSimpleRegistry = null;
        try {
            xSimpleRegistry = (XSimpleRegistry) args[0];
        }
        catch (ArrayIndexOutOfBoundsException e) {
            throw new com.sun.star.lang.IllegalArgumentException("Argument must not be null.");
        }

        XRegistryKey rootkey = xSimpleRegistry.getRootKey();

        XRegistryKey implkey_xRegistryKey = rootkey.openKey("Implementations");
        if(implkey_xRegistryKey != null) {
            XRegistryKey xRegistryKeys[] = implkey_xRegistryKey.openKeys();

            for(int i = 0; i < xRegistryKeys.length; ++ i) {
                addFactories(new String[]{xRegistryKeys[i].getStringValue()});
            }
        }
    }

    /**
     * Creates a new instance of a specified service. Therefor the associated factory of the service is
     * looked up and used to instanciate a new component.
     * <p>
     * @return  newly created component
     * @param   serviceSpecifier    indicates the service or component name
     * @see     com.sun.star.lang.XMultiServiceFactory
     */
    public java.lang.Object createInstance( String serviceSpecifier )
            throws com.sun.star.uno.Exception,
                   com.sun.star.uno.RuntimeException
    {
        return queryServiceFactory(serviceSpecifier).createInstance();
    }

    /**
     * Creates a new instance of a specified service with the given parameters.
     * Therefor the associated factory of the service is  looked up and used to instanciate a new component.
     * <p>
     * @return  newly created component
     * @param   serviceSpecifier    indicates the service or component name
     * @see     com.sun.star.lang.XMultiServiceFactory
     */
    public java.lang.Object createInstanceWithArguments ( String serviceSpecifier,
                                                Object[] args )
            throws com.sun.star.uno.Exception,
                   com.sun.star.uno.RuntimeException
    {
        if (DEBUG) {
            System.err.println("createInstanceWithArguments:" );

            for (int i=0; i<args.length; i++)
                System.err.print(" "+ args[i]);

            System.err.println();
        }

        return queryServiceFactory(serviceSpecifier).createInstanceWithArguments( args );
    }

    /**
     * Look up the factory for a given service or implementation name.
     * First the requested service name is search in the list of avaible services. If it can not be found
     * the name is looked up in the the implementation list.
     * <p>
     * @return  the factory of the service / implementation
     * @param   serviceSpecifier    indicates the service or implementation name
     * @see     com.sun.star.lang.XMultiServiceFactory
     */
    private XSingleServiceFactory queryServiceFactory(String serviceName)
            throws com.sun.star.uno.Exception,
                   com.sun.star.uno.RuntimeException
    {
        DEBUG("queryServiceFactory for name " + serviceName );
        Object factory = null;

        if ( factoriesByServiceNames.containsKey( serviceName ) ) {
            java.util.Vector aviableFact = (java.util.Vector) factoriesByServiceNames.get( serviceName );

            DEBUG("");
            DEBUG("aviable factories for " + serviceName +" "+ aviableFact);
            DEBUG("");

            if ( !aviableFact.isEmpty() )
                factory = aviableFact.firstElement();

        } else // not found in list of services - now try the implementations
            factory = factoriesByImplNames.get( serviceName ); // return null if none is aviable

        if (DEBUG) {
            if (factory == null) System.err.println("service not registered");
            else
                System.err.println("service found:" + factory + " " + UnoRuntime.queryInterface(XSingleServiceFactory.class, factory));
        }

        if (factory == null)
            throw new com.sun.star.uno.Exception("Query for service factory for " + serviceName + " failed.");

        return (XSingleServiceFactory) UnoRuntime.queryInterface(XSingleServiceFactory.class, factory);
    }

    /**
     * Supplies a list of all avialable services names.
     * <p>
     * @return  list of Strings of all service names
     * @see     com.sun.star.container.XContentEnumerationAccess
     */
    public String[] getAvailableServiceNames()
            throws com.sun.star.uno.RuntimeException
    {
        int i = 0;
        String[] availableServiceNames = new String[factoriesByServiceNames.size()];

        java.util.Enumeration keys = factoriesByServiceNames.keys();

        while (keys.hasMoreElements())
            availableServiceNames[i++] = (String) keys.nextElement();

        return availableServiceNames;
    }

    /**
     * Removes all listeners from the <code>ServiceManager</code> and clears the list of the services.
     * <p>
     * @see com.sun.star.lang.XComponent
     */
    public void dispose()
        throws com.sun.star.uno.RuntimeException
    {
        if (eventListener != null) {
            java.util.Enumeration enum = eventListener.elements();

            while (enum.hasMoreElements()) {
                XEventListener listener = (XEventListener) enum.nextElement();
                listener.disposing(new com.sun.star.lang.EventObject(this));
            }
        }

        eventListener.removeAllElements();
        factoriesByServiceNames.clear();
        factoriesByImplNames.clear();
    }

    /**
     * Adds a new <code>EventListener</code>. The listener is notified when a
     * service is added (removed) to (from) the <code>ServiceManager</code>.
     * If the listener is already registred a
     * <code>com.sun.star.uno.RuntimeException</code> will be thrown.
     * <p>
     * @param   xListener   the new listener which should been added.
     * @see     com.sun.star.lang.XComponent
     */
    public void addEventListener( XEventListener xListener )
            throws com.sun.star.uno.RuntimeException
    {
        if (xListener == null)
            throw new com.sun.star.uno.RuntimeException("Listener must not be null");

          if ( eventListener.contains(xListener) )
              throw new com.sun.star.uno.RuntimeException("Listener already registred.");

           eventListener.addElement(xListener);
    }

    /**
     * Removes a <code>EventListener</code> from the <code>ServiceManager</code>.
     * If the listener is not registered a <code>com.sun.star.uno.RuntimeException</code>
     * will be thrown.
     * <p>
     * @param   xListener   the new listener which should been removed.
     * @see com.sun.star.lang.XComponent
     */
    public void removeEventListener( XEventListener xListener )
            throws com.sun.star.uno.RuntimeException
    {
        if (xListener == null)
            throw new com.sun.star.uno.RuntimeException("Listener must not be null");

          if ( !eventListener.contains(xListener) )
              throw new com.sun.star.uno.RuntimeException("Listener is not registered.");

        eventListener.removeElement(xListener);
    }

    /**
     * Checks if a component is registered at the <code>ServiceManager</code>. The given object argument must
     * provide a <code>XServiceInfo</code> interface.
     * <p>
     * @return  true if the component is registred otherwise false.
     * @param   object object which provides a <code>XServiceInfo</code> interface.
     * @see     com.sun.star.container.XSet
     * @see     com.sun.star.lang.XServiceInfo
     */
    public boolean has( Object object )
        throws com.sun.star.uno.RuntimeException
    {
            if (object == null)
                throw new com.sun.star.uno.RuntimeException("The parameter must not been null");

            XServiceInfo xServiceInfo = (XServiceInfo) UnoRuntime.queryInterface(XServiceInfo.class, object);

            if (xServiceInfo != null) {
                return UnoRuntime.areSame(factoriesByImplNames.get(xServiceInfo.getImplementationName()), object);
            }

        return false;
    }

    /**
     * Adds a <code>SingleServiceFactory</code> to the <code>ServiceManager</code>.
     * <p>
     * @param   object  factory which should be added.
     * @see com.sun.star.container.XSet
     * @see com.sun.star.lang.XSingleServiceFactory
     */
    public void insert( Object object )
        throws com.sun.star.lang.IllegalArgumentException,
               com.sun.star.container.ElementExistException,
               com.sun.star.uno.RuntimeException
    {
        if (object == null) throw new com.sun.star.lang.IllegalArgumentException();

        XSingleServiceFactory newFactory =
                (XSingleServiceFactory) UnoRuntime.queryInterface(XSingleServiceFactory.class, object);

        if ( newFactory == null )
            throw new com.sun.star.lang.IllegalArgumentException(
                "The given object does not implement the XSingleServiceFactory interface."
            );

        XServiceInfo xServiceInfo =
                (XServiceInfo) UnoRuntime.queryInterface(XServiceInfo.class, object);

        if (xServiceInfo == null)
            throw new com.sun.star.lang.IllegalArgumentException(
                "The given object does not implement the XServiceInfo interface."
            );

        if ( factoriesByImplNames.containsKey( xServiceInfo.getImplementationName() ) ) {
            throw new com.sun.star.container.ElementExistException(
                xServiceInfo.getImplementationName() + " already registred"
            );
        }

        DEBUG("add factory " + object.toString() + " for " + xServiceInfo.getImplementationName());
        factoriesByImplNames.put( xServiceInfo.getImplementationName(), object );


        String[] serviceNames = xServiceInfo.getSupportedServiceNames();
        java.util.Vector vec = null;

        for (int i=0; i<serviceNames.length; i++) {
            if ( !factoriesByServiceNames.containsKey( serviceNames[i] ) ) {
                DEBUG("no registered services found under " + serviceNames[i] );
                factoriesByServiceNames.put(serviceNames[i], new java.util.Vector());
            }

            vec = (java.util.Vector) factoriesByServiceNames.get( serviceNames[i] );

            if ( vec.contains( object ) )
                System.err.println("The implementation " + xServiceInfo.getImplementationName() +
                    " already registered for the service " + serviceNames[i] + " - ignoring!");
            else
                vec.addElement(object);
        }
    }

    /**
     * Removes a <code>SingleServiceFactory</code> from the <code>ServiceManager</code>.
     * <p>
     * @param   object  factory which should be removed.
     * @see com.sun.star.container.XSet
     * @see com.sun.star.lang.XSingleServiceFactory
     */
    public void remove( Object object )
        throws com.sun.star.lang.IllegalArgumentException,
               com.sun.star.container.NoSuchElementException,
               com.sun.star.uno.RuntimeException
    {
        if (object == null)
            throw new com.sun.star.lang.IllegalArgumentException(
                    "The given object must not be null."
            );

        XServiceInfo xServiceInfo =
            (XServiceInfo) UnoRuntime.queryInterface(XServiceInfo.class, object);

        if (xServiceInfo == null)
            throw new com.sun.star.lang.IllegalArgumentException(
                    "The given object does not implement the XServiceInfo interface."
            );

        XSingleServiceFactory xSingleServiceFactory =
            (XSingleServiceFactory) UnoRuntime.queryInterface(XSingleServiceFactory.class, object);

        if (xSingleServiceFactory == null)
            throw new com.sun.star.lang.IllegalArgumentException(
                    "The given object does not implement the XSingleServiceFactory interface."
            );

        if ( factoriesByImplNames.remove( xServiceInfo.getImplementationName() ) == null )
            throw new com.sun.star.container.NoSuchElementException(
                    xServiceInfo.getImplementationName() +
                    " is not registered as an implementation."
            );

        String[] serviceNames = xServiceInfo.getSupportedServiceNames();

        for ( int i=0; i<serviceNames.length; i++ ) {
            if ( factoriesByServiceNames.containsKey( serviceNames[i] ) ) {
                java.util.Vector vec = (java.util.Vector) factoriesByServiceNames.get(serviceNames[i]);

                if ( !vec.removeElement(object) )
                    System.err.println("The implementation " + xServiceInfo.getImplementationName() +
                        " is not registered for the service " + serviceNames[i] + " - ignoring!");

                if ( vec.isEmpty() ) // remove the vector if no implementations aviable for the service
                    factoriesByServiceNames.remove( serviceNames[i] );
            }
        }
    }

    /**
     * Provides an enumeration of all registred services.
     * <p>
     * @return  an enumeration of all avialable services.
     * @see     com.sun.star.conatiner.XEnumerationAccess
     */
    public XEnumeration createEnumeration()
            throws com.sun.star.uno.RuntimeException
    {
        return new ServiceEnumerationImpl( factoriesByImplNames.elements() );
    }

    /**
     * Provides the UNO type of the <code>ServiceManager</code>
     * <p>
     * @return  the UNO type of the <code>ServiceManager</code>.
     * @see     com.sun.star.container.XElementAccess
     * @see     com.sun.star.uno.TypeClass
     */
    public com.sun.star.uno.Type getElementType()
            throws com.sun.star.uno.RuntimeException
    {
        if ( UNO_TYPE == null )
        try {
            UNO_TYPE = new com.sun.star.uno.Type(   com.sun.star.uno.TypeClass.SERVICE,
                                                      ServiceManager.class.getName(),
                                                      ServiceManager[].class.getName(),
                                                      ServiceManager.class );
        }
        catch (com.sun.star.uno.Exception e) {
            com.sun.star.uno.RuntimeException rte = new com.sun.star.uno.RuntimeException();
            rte.fillInStackTrace();
            throw rte;
        }

        return UNO_TYPE;
    }

    /**
     * Checks if the any componets are registered.
     * <p>
     * @return  true - if the list of the registred components is not empty - otherwise false.
     * @see     com.sun.star.container.XElementAccess
     */
    public boolean hasElements() {
        return ! factoriesByImplNames.isEmpty();
    }

    /**
     * Provides an enumeration of of all factorys for a specified service.
     * <p>
     * @return  an enumeration for service name.
     * @param   serviceName     name of the requested service
     * @see     com.sun.star.container.XContentEnumerationAccess
     */
    public XEnumeration createContentEnumeration( String serviceName )
                throws com.sun.star.uno.RuntimeException
    {
        XEnumeration enum = null;

        java.util.Vector serviceList = (java.util.Vector) factoriesByServiceNames.get(serviceName);

        if (serviceList != null)
            enum = new ServiceEnumerationImpl( serviceList.elements() );
        else
            enum = new ServiceEnumerationImpl();

        return enum;
    }

    /**
     * Returns the implementation name of the <code>ServiceManager</code> component.
     * <p>
     * @return  the class name of the <code>ServiceManager</code>.
     * @see     com.sun.star.lang.XServiceInfo
     */
    public String getImplementationName()
            throws com.sun.star.uno.RuntimeException
    {
        return getClass().getName();
    }

    /**
     * Checks if the <code>ServiceManager</code> supports a service.
     * <p>
     * @return  true if the service is supported - otherwise false.
     * @param   serviceName service name which should be checked.
     * @see     com.sun.star.lang.XServiceInfo
     */
    public boolean supportsService( String serviceName )
            throws com.sun.star.uno.RuntimeException
    {
        for (int i=0; i<supportedServiceNames.length; i++)
            if (supportedServiceNames[i].equals( serviceName )) return true;

        if (getImplementationName().equals( serviceName )) return true;

        return false;
    }

    /**
     * Supplies list of all supported services.
     * <p>
     * @return  a list of all supported service names.
     * @see     com.sun.star.lang.XServiceInfo
     */
    public String[] getSupportedServiceNames()
            throws com.sun.star.uno.RuntimeException
    {
        return supportedServiceNames;
    }

    /**
     * The <code>ServiceEnumerationImpl</code> class provides an
     * implementation of the @see com.sun.star.container.XEnumeration interface.
     * It is a inner wrapper for a java.util.Enumeration object.
     * <p>
     * @version     $Revision: 1.1.1.1 $ $ $Date: 2000-09-18 15:27:51 $
     * @author      Markus Herzog
     * @see         com.sun.star.lang.XSingleServiceFactory
     * @see         com.sun.star.lang.XServiceInfo
     * @since       UDK1.0
     */
    class ServiceEnumerationImpl implements XEnumeration {
        java.util.Enumeration enumeration = null;

        /**
         * Constructs a new empty instance.
         */
        public ServiceEnumerationImpl() {
        }

        /**
         * Constructs a new instance with a given enumeration.
         * <p>
         * @param   enum    is the enumeration which should been wrapped.
         * @see     com.sun.star.container.XEnumeration
         */
        public ServiceEnumerationImpl(java.util.Enumeration enum) {
            enumeration = enum;
        }

        /**
         * Checks if the enumeration contains more elements.
         * <p>
         * @return  true if more elements are available - otherwise false.
         * @see     com.sun.star.container.XEnumeration
         */
        public boolean hasMoreElements()
                throws com.sun.star.uno.RuntimeException
        {
            if (enumeration != null)
                return enumeration.hasMoreElements();

            return false;
        }

        /**
         * Returns the next element of the enumeration. If no further elements
         * available a com.sun.star.container.NoSuchElementException exception will be thrown.
         * <p>
         * @return  the next element.
         * @see     com.sun.star.container.XEnumeration
         */
        public Object nextElement()
                throws com.sun.star.container.NoSuchElementException,
                       com.sun.star.lang.WrappedTargetException,
                       com.sun.star.uno.RuntimeException
        {
            if (enumeration == null)
                throw new com.sun.star.container.NoSuchElementException();

            try {
                return enumeration.nextElement();
            } catch (java.util.NoSuchElementException e) {
                com.sun.star.container.NoSuchElementException ex =
                        new com.sun.star.container.NoSuchElementException();
                ex.fillInStackTrace();

                throw ex;
            }
        }
    }
}
/**
 * The <code>ServiceManagerFactory</code> is the factory class for the
 * <code>ServiceManager</code>. As all factories it implments the
 * com.sun.star.lang.XSingleServiceFactory and the com.sun.star.lang.XServiceInfo
 * interfaces.
 * <p>
 * @version     $Revision: 1.1.1.1 $ $ $Date: 2000-09-18 15:27:51 $
 * @author      Markus Herzog
 * @see         com.sun.star.lang.XSingleServiceFactory
 * @see         com.sun.star.lang.XServiceInfo
 * @since       UDK1.0
*/
class ServiceManagerFactory
            implements  XServiceInfo,
                        XSingleServiceFactory
{
    /**
     * Creates a new instance of the <code>ServiceManagerFactory</code>.
     */
    public ServiceManagerFactory() {
    }

    /**
     * Supplies the implementation name of the <code>ServiceManager</code>.
     * <p>
     * @return      <code>ServiceManager</code> class name.
      * @see         com.sun.star.lang.XServiceInfo
     */
    public String getImplementationName()
            throws com.sun.star.uno.RuntimeException
    {
        return ServiceManager.class.getName();
    }

    /**
     * Checks wether or not a service is supported.
     * <p>
     * @return      true - if the service is supported, otherwise false.
     * @param       serviceName     the name of the service that should be checked.
      * @see         com.sun.star.lang.XServiceInfo
     */
    public boolean supportsService( String serviceName )
            throws com.sun.star.uno.RuntimeException
    {
        for ( int i=0; i<ServiceManager.supportedServiceNames.length; i++ )
            if ( ServiceManager.supportedServiceNames[i].equals(serviceName) ) return true;

        if ( getImplementationName().equals(serviceName) ) return true;

        return false;
    }

    /**
     * Returns all service names which are supported by <code>ServiceManager</code>.
     * <p>
     * @return      a list aof all supported service names.
      * @see         com.sun.star.lang.XServiceInfo
     */
    public String[] getSupportedServiceNames()
            throws com.sun.star.uno.RuntimeException
    {
        return ServiceManager.supportedServiceNames;
    }

    /**
     * Creates a new instance of the <code>ServiceManager</code>.
     * <p>
     * @return      newly created <code>ServiceManager</code> object.
      * @see         com.sun.star.lang.XSingleServiceFactory
     */
    public java.lang.Object createInstance()
            throws com.sun.star.uno.Exception,
                   com.sun.star.uno.RuntimeException
    {
        return new ServiceManager();
    }

    /**
     * Creates a new instance of the <code>ServiceManager</code> with arguments.
     * At this time it always throws a com.sun.star.lang.NoSuchMethodException
     * because there is no the <code>ServiceManager</code> has no constructor with
     * arguments.
     * <p>
     * @return      null - allways throws an exception
     * @param       aArguments arguments for new instance.
      * @see         com.sun.star.lang.XSingleServiceFactory
     */
    public java.lang.Object createInstanceWithArguments( java.lang.Object[] aArguments )
            throws com.sun.star.uno.Exception,
                   com.sun.star.uno.RuntimeException
    {
        throw new com.sun.star.lang.NoSuchMethodException("Constructor with arguments is not supported.");
    }
}


