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

package com.sun.star.comp.servicemanager;

import java.util.ArrayList;
import java.util.Collections;

import com.sun.star.container.XContentEnumerationAccess;
import com.sun.star.container.XEnumeration;
import com.sun.star.container.XSet;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XEventListener;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;

/**
 * The <code>ServiceManager</code> class is an implmentation of the
 * <code>ServiceManager</code>the central class needed for implementing or using
 * UNO components in Java.
 *
 * <p>The Methods <code>queryInterface</code> and <code>isSame</code> delegate
 * calls to the implementing objects and are used instead of casts and identity
 * comparisons.</p>
 *
 * @see         com.sun.star.lang.XMultiServiceFactory
 * @see         com.sun.star.container.XSet
 * @see         com.sun.star.container.XContentEnumerationAccess
 * @see         com.sun.star.lang.XComponent
 * @see         com.sun.star.lang.XServiceInfo
 * @since       UDK1.0
 */
public class ServiceManager implements XMultiServiceFactory,
                                       XMultiComponentFactory,
                                       XSet,
                                       XContentEnumerationAccess,
                                       XComponent,
                                       XServiceInfo
{
    private static final boolean DEBUG = false;

    private static final void DEBUG (String dbg) {
        if (DEBUG) System.err.println( dbg );
    }

    private static com.sun.star.uno.Type UNO_TYPE = null;

    static String[] supportedServiceNames = {
            "com.sun.star.lang.MultiServiceFactory",
            "com.sun.star.lang.ServiceManager"
    };

    ArrayList<XEventListener>    eventListener;
    java.util.HashMap<String, Object> factoriesByImplNames;
    java.util.HashMap<String, ArrayList<Object>> factoriesByServiceNames;  // keys:

    private com.sun.star.uno.XComponentContext m_xDefaultContext;

    /**
     * Creates a new instance of the <code>ServiceManager</code>.
     */
    public ServiceManager() {
        eventListener           = new ArrayList<XEventListener>();
        factoriesByImplNames    = new java.util.HashMap<String, Object>();
        factoriesByServiceNames = new java.util.HashMap<String, ArrayList<Object>>();
        m_xDefaultContext = null;
    }

    public void setDefaultContext(XComponentContext context) {
        m_xDefaultContext = context;
    }

    /**
     * Creates a new instance of a specified service.
     *
     * <p>Therefore the associated factory of the service is looked up and used
     * to instanciate a new component. </p>
     *
     * @param   serviceSpecifier    indicates the service or component name.
     * @return  newly created component.
     *
     * @see     com.sun.star.lang.XMultiServiceFactory
     */
    public java.lang.Object createInstance( String serviceSpecifier )
            throws com.sun.star.uno.Exception,
                   com.sun.star.uno.RuntimeException
    {
        return createInstanceWithContext( serviceSpecifier, m_xDefaultContext );
    }

    /**
     * Creates a new instance of a specified service with the given parameters.
     *
     * <p>Therefore the associated factory of the service is  looked up and used
     * to instanciate a new component.</p>
     *
     * @return  newly created component.
     * @param   serviceSpecifier    indicates the service or component name.
     * @see     com.sun.star.lang.XMultiServiceFactory
     */
    public java.lang.Object createInstanceWithArguments(
        String serviceSpecifier, Object[] args )
        throws com.sun.star.uno.Exception, com.sun.star.uno.RuntimeException
    {
        if (DEBUG) {
            System.err.println("createInstanceWithArguments:" );

            for (Object arg : args) {
                System.err.print(" " + arg);
            }

            System.err.println();
        }

        return createInstanceWithArgumentsAndContext( serviceSpecifier, args, m_xDefaultContext );
    }

    /**
     * Look up the factory for a given service or implementation name.
     *
     * <p>First the requested service name is search in the list of available
     * services. If it can not be found the name is looked up in the implementation
     * list.</p>
     *
     * @param   serviceName    indicates the service or implementation name.
     * @return  the factory of the service / implementation.
     *
     * @see     com.sun.star.lang.XMultiServiceFactory
     */
    private Object queryServiceFactory(String serviceName)
            throws com.sun.star.uno.Exception,
                   com.sun.star.uno.RuntimeException
    {
        DEBUG("queryServiceFactory for name " + serviceName );
        Object factory = null;

        if ( factoriesByServiceNames.containsKey( serviceName ) ) {
            ArrayList<Object> availableFact = factoriesByServiceNames.get( serviceName );

            DEBUG("");
            DEBUG("aviable factories for " + serviceName +" "+ availableFact);
            DEBUG("");

            if ( !availableFact.isEmpty() )
                factory = availableFact.get(availableFact.size()-1);

        } else // not found in list of services - now try the implementations
            factory = factoriesByImplNames.get( serviceName ); // return null if none is aviable

        if (DEBUG) {
            if (factory == null) System.err.println("service not registered");
            else
                System.err.println("service found:" + factory + " " + UnoRuntime.queryInterface(XSingleServiceFactory.class, factory));
        }

        if (factory == null)
            throw new com.sun.star.uno.Exception("Query for service factory for " + serviceName + " failed.");

        return factory;
    }

    /**
     * Supplies a list of all avialable services names.
     *
     * @return  list of Strings of all service names.
     * @see     com.sun.star.container.XContentEnumerationAccess
     */
    public String[] getAvailableServiceNames()
            throws com.sun.star.uno.RuntimeException
    {
        try{
            return factoriesByServiceNames.keySet().toArray(
                new String[ factoriesByServiceNames.size() ] );
        } catch(Exception ex) {
            throw new com.sun.star.uno.RuntimeException(ex);
        }
    }

    // XMultiComponentFactory implementation

    /** Create a service instance with given context.
     *
     * @param rServiceSpecifier service name.
     * @param xContext context.
     * @return service instance.
     */
    public java.lang.Object createInstanceWithContext(
        String rServiceSpecifier,
        com.sun.star.uno.XComponentContext xContext )
        throws com.sun.star.uno.Exception
    {
        Object fac = queryServiceFactory( rServiceSpecifier );
        if (fac != null)
        {
            XSingleComponentFactory xCompFac = UnoRuntime.queryInterface(
                XSingleComponentFactory.class, fac );
            if (xCompFac != null)
            {
                return xCompFac.createInstanceWithContext( xContext );
            }
            else
            {
                XSingleServiceFactory xServiceFac = UnoRuntime.queryInterface(
                    XSingleServiceFactory.class, fac );
                if (xServiceFac != null)
                {
                    if (DEBUG)
                        System.err.println( "### ignoring context raising service \"" + rServiceSpecifier + "\"!" );
                    return xServiceFac.createInstance();
                }
                else
                {
                    throw new com.sun.star.uno.Exception(
                        "retrieved service factory object for \"" + rServiceSpecifier +
                        "\" does not export XSingleComponentFactory nor XSingleServiceFactory!" );
                }
            }
        }
        return null;
    }
    /**
     * Create a service instance with given context and arguments.
     *
     * @param rServiceSpecifier service name.
     * @param rArguments arguments.
     * @param xContext context.
     * @return service instance.
     */
    public java.lang.Object createInstanceWithArgumentsAndContext(
        String rServiceSpecifier,
        java.lang.Object[] rArguments,
        com.sun.star.uno.XComponentContext xContext )
        throws com.sun.star.uno.Exception
    {
        Object fac = queryServiceFactory( rServiceSpecifier );
        if (fac != null)
        {
            XSingleComponentFactory xCompFac = UnoRuntime.queryInterface(
                XSingleComponentFactory.class, fac );
            if (xCompFac != null)
            {
                return xCompFac.createInstanceWithArgumentsAndContext( rArguments, xContext );
            }
            else
            {
                XSingleServiceFactory xServiceFac = UnoRuntime.queryInterface(
                    XSingleServiceFactory.class, fac );
                if (xServiceFac != null)
                {
                    if (DEBUG)
                        System.err.println( "### ignoring context raising service \"" + rServiceSpecifier + "\"!" );
                    return xServiceFac.createInstanceWithArguments( rArguments );
                }
                else
                {
                    throw new com.sun.star.uno.Exception(
                        "retrieved service factory object for \"" + rServiceSpecifier +
                        "\" does not export XSingleComponentFactory nor XSingleServiceFactory!" );
                }
            }
        }
        return null;
    }

    /**
     * Removes all listeners from the <code>ServiceManager</code> and clears the
     * list of the services.
     *
     * @see com.sun.star.lang.XComponent
     */
    public void dispose()
        throws com.sun.star.uno.RuntimeException
    {
        if (eventListener != null) {
            for (XEventListener listener : eventListener) {
                listener.disposing(new com.sun.star.lang.EventObject(this));
            }
            eventListener.clear();
        }

        factoriesByServiceNames.clear();
        factoriesByImplNames.clear();
    }

    /**
     * Adds a new <code>EventListener</code>.
     *
     * <p>The listener is notified when a service is added (removed) to (from)
     * the <code>ServiceManager</code>.</p>
     *
     * <p>If the listener is already registred a
     * <code>com.sun.star.uno.RuntimeException</code> will be thrown.</p>
     *
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

           eventListener.add(xListener);
    }

    /**
     * Removes a <code>EventListener</code> from the <code>ServiceManager</code>.
     *
     * <p>If the listener is not registered a <code>com.sun.star.uno.RuntimeException</code>
     * will be thrown.</p>
     *
     * @param   xListener   the new listener which should been removed.
     * @see     com.sun.star.lang.XComponent
     */
    public void removeEventListener( XEventListener xListener )
            throws com.sun.star.uno.RuntimeException
    {
        if (xListener == null)
            throw new com.sun.star.uno.RuntimeException("Listener must not be null");

          if ( !eventListener.contains(xListener) )
              throw new com.sun.star.uno.RuntimeException("Listener is not registered.");

        eventListener.remove(xListener);
    }

    /**
     * Checks if a component is registered at the <code>ServiceManager</code>.
     *
     * <p>The given object argument must provide a <code>XServiceInfo</code>
     * interface.</p>
     *
     * @param   object object which provides a <code>XServiceInfo</code> interface.
     * @return  true if the component is registred otherwise false.
     *
     * @see     com.sun.star.container.XSet
     * @see     com.sun.star.lang.XServiceInfo
     */
    public boolean has( Object object )
        throws com.sun.star.uno.RuntimeException
    {
        if (object == null)
            throw new com.sun.star.uno.RuntimeException("The parameter must not been null");

        XServiceInfo xServiceInfo = UnoRuntime.queryInterface(XServiceInfo.class, object);

        return xServiceInfo != null && UnoRuntime.areSame(factoriesByImplNames.get(xServiceInfo.getImplementationName()), object);
    }

    /**
     * Adds a <code>SingleServiceFactory</code> to the <code>ServiceManager</code>.
     *
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

        XServiceInfo xServiceInfo =
                UnoRuntime.queryInterface(XServiceInfo.class, object);

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
        ArrayList<Object> vec  ;

        for (String serviceName : serviceNames) {
            if (!factoriesByServiceNames.containsKey(serviceName)) {
                DEBUG("> no registered services found under " + serviceName + ": adding...");
                factoriesByServiceNames.put(serviceName, new ArrayList<Object>());
            }
            vec = factoriesByServiceNames.get(serviceName);
            if (vec.contains( object )) {
                System.err.println("The implementation " + xServiceInfo.getImplementationName() +
                        " already registered for the service " + serviceName + " - ignoring!");
            } else {
                vec.add(object);
            }
        }
    }

    /**
     * Removes a <code>SingleServiceFactory</code> from the <code>ServiceManager</code>.
     *
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
            UnoRuntime.queryInterface(XServiceInfo.class, object);

        if (xServiceInfo == null)
            throw new com.sun.star.lang.IllegalArgumentException(
                    "The given object does not implement the XServiceInfo interface."
            );

        XSingleServiceFactory xSingleServiceFactory =
            UnoRuntime.queryInterface(XSingleServiceFactory.class, object);

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

        for (String serviceName : serviceNames) {
            if (factoriesByServiceNames.containsKey(serviceName)) {
                ArrayList<Object> vec = factoriesByServiceNames.get(serviceName);
                if (!vec.remove(object)) {
                    System.err.println("The implementation " + xServiceInfo.getImplementationName() +
                            " is not registered for the service " + serviceName + " - ignoring!");
                }
                // remove the vector if no implementations aviable for the service
                if (vec.isEmpty()) {
                    factoriesByServiceNames.remove(serviceName);
                }
            }
        }
    }

    /**
     * Provides an enumeration of all registered services.
     *
     * @return  an enumeration of all available services.
     * @see     com.sun.star.container.XEnumerationAccess
     */
    public XEnumeration createEnumeration()
            throws com.sun.star.uno.RuntimeException
    {
        return new ServiceEnumerationImpl( factoriesByImplNames.values().iterator() );
    }

    /**
     * Provides the UNO type of the <code>ServiceManager</code>
     *
     * @return  the UNO type of the <code>ServiceManager</code>.
     * @see     com.sun.star.container.XElementAccess
     * @see     com.sun.star.uno.TypeClass
     */
    public com.sun.star.uno.Type getElementType()
            throws com.sun.star.uno.RuntimeException
    {
        if ( UNO_TYPE == null )
            UNO_TYPE = new com.sun.star.uno.Type(ServiceManager.class);

        return UNO_TYPE;
    }

    /**
     * Checks if the any componets are registered.
     *
     * @return  true - if the list of the registred components is not empty - otherwise false.
     * @see     com.sun.star.container.XElementAccess
     */
    public boolean hasElements() {
        return ! factoriesByImplNames.isEmpty();
    }

    /**
     * Provides an enumeration of all factories for a specified service.
     *
     * @param   serviceName     name of the requested service.
     * @return  an enumeration for service name.
     * @see     com.sun.star.container.XContentEnumerationAccess
     */
    public XEnumeration createContentEnumeration( String serviceName )
                throws com.sun.star.uno.RuntimeException
    {
        XEnumeration enumer  ;

        ArrayList<Object> serviceList = factoriesByServiceNames.get(serviceName);

        if (serviceList != null)
            enumer = new ServiceEnumerationImpl( serviceList.iterator() );
        else
            enumer = new ServiceEnumerationImpl();

        return enumer;
    }

    /**
     * Returns the implementation name of the <code>ServiceManager</code> component.
     *
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
     *
     * @param   serviceName service name which should be checked.
     * @return  true if the service is supported - otherwise false.
     *
     * @see     com.sun.star.lang.XServiceInfo
     */
    public boolean supportsService( String serviceName )
            throws com.sun.star.uno.RuntimeException
    {
        for (String supportedServiceName : supportedServiceNames) {
            if (supportedServiceName.equals(serviceName)) {
                return true;
            }
        }

        return getImplementationName().equals(serviceName);
    }

    /**
     * Supplies list of all supported services.
     *
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
     *
     * <p>It is a inner wrapper for a java.util.Enumeration object.</p>
     *
     * @see         com.sun.star.lang.XSingleServiceFactory
     * @see         com.sun.star.lang.XServiceInfo
     * @since       UDK1.0
     */
    static class ServiceEnumerationImpl implements XEnumeration {
        java.util.Iterator<Object> enumeration = null;

        /**
         * Constructs a new empty instance.
         */
        public ServiceEnumerationImpl() {
        }

        /**
         * Constructs a new instance with a given enumeration.
         *
         * @param   enumer  is the enumeration which should been wrapped.
         * @see     com.sun.star.container.XEnumeration
         */
        public ServiceEnumerationImpl(java.util.Enumeration<Object> enumer) {
            enumeration = Collections.list(enumer).iterator();
        }

        /**
         * Constructs a new instance with a given enumeration.
         *
         * @param   enumer  is the enumeration which should been wrapped.
         * @see     com.sun.star.container.XEnumeration
         */
        public ServiceEnumerationImpl(java.util.Iterator<Object> enumer) {
            enumeration = enumer;
        }

        /**
         * Checks if the enumeration contains more elements.
         *
         * @return  true if more elements are available - otherwise false.
         * @see     com.sun.star.container.XEnumeration
         */
        public boolean hasMoreElements()
                throws com.sun.star.uno.RuntimeException
        {
            return enumeration != null && enumeration.hasNext();

            }

        /**
         * Returns the next element of the enumeration.
         *
         * <p>If no further elements available a com.sun.star.container.NoSuchElementException
         * exception will be thrown.</p>
         *
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
                return enumeration.next();
            } catch (java.util.NoSuchElementException e) {
                throw new com.sun.star.container.NoSuchElementException(e, e.toString());
            }
        }
    }
}
