/*************************************************************************
 *
 *  $RCSfile: java_environment.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kr $ $Date: 2001-02-26 18:26:36 $
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

package com.sun.star.lib.uno.environments.java;


import java.lang.reflect.Array;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

import java.util.Hashtable;
import java.util.Enumeration;


import com.sun.star.lib.sandbox.Disposable;

import com.sun.star.lib.sandbox.generic.Dispatcher;
import com.sun.star.lib.sandbox.generic.DispatcherAdapterBase;
import com.sun.star.lib.sandbox.generic.DispatcherAdapterFactory;

import com.sun.star.lib.uno.typedesc.TypeDescription;

import com.sun.star.uno.IEnvironment;
import com.sun.star.uno.IQueryInterface;
import com.sun.star.uno.MappingException;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * The java_environment is the environment where objects and
 * interfaces are registered, which are mapped out of java or
 * into java. The java_environment implements the <code>IEnvironment</code>
 * interface defined in the uno runtime.
 * <p>
 * <p>
 * @version     $Revision: 1.4 $ $ $Date: 2001-02-26 18:26:36 $
 * @author      Kay Ramme
 * @see         com.sun.star.uno.UnoRuntime
 * @see         com.sun.star.uno.IEnvironment
 * @since       UDK1.0
 */
public class java_environment implements IEnvironment, Disposable {
    /**
     * When set to true, enables various debugging output.
     */
    static public final boolean DEBUG = false;

    /*
    ** This is the holder proxy, which one gets while trying to get a registered object
    */
    static public class HolderProxy extends DispatcherAdapterBase implements Dispatcher, XInterface, IQueryInterface {
        static private Hashtable __methodss = new Hashtable();

        /**
         * Gets the methods of a class as a hashtable.
         * Uses reflection for first time and stores the methods
         * array in hashtable for later use.
         * <p>
         * This method does not belong to the provided <code>api</code>
         * <p>
         * @return the methods
         * @param  zClass   the class
         * @see    java.lang.Class#getMethods
         */
        static public Hashtable __getMethodsAsTable(Class zClass) {
            Hashtable methods_tab = (Hashtable)__methodss.get(zClass);

            if(methods_tab == null) {
                methods_tab = new Hashtable();

                Method methods[] = zClass.getMethods();

                for(int i = 0; i < methods.length; ++ i) {
                    methods_tab.put(methods[i].getName(), methods[i]);
                }

                __methodss.put(zClass, methods_tab);
            }

            return methods_tab;
        }

        Holder _holder;
        Type _type;

        Hashtable _methods;

        void setHolder(Holder holder) {
            _holder = holder;
        }

        public Object invoke(Object object, String name, Object params[]) throws Exception  {
            Method method = (Method)_methods.get(name);

            Object result = null;

            if(method == null) {
                System.err.println(getClass().getName() + ".invoke - method not found:" + object + " " + name + " " + params);
            }
            else {
                try {
                    result = method.invoke(object, params);
                }
                // don't hide exceptions
                catch(InvocationTargetException invocationTargetException) {
                    throw (Exception)invocationTargetException.getTargetException();
                }
            }

            return result;
        }

        public HolderProxy() {
            super();
        }

        public void setInterface(Type type) {
            _type = type;
            _methods = __getMethodsAsTable(object.getClass());
        }

        public Type getInterface() {
            return _type;
        }

        // IQueryInterface - delegate calls through this proxy
        public Object queryInterface(Type type) throws MappingException {
            return UnoRuntime.queryInterface(type, object);
        }

        public boolean isSame(Object object) throws MappingException {
            return UnoRuntime.areSame(this.object, object);
        }

        public String getOid() {
            return UnoRuntime.generateOid(object);
        }

        public void finalize() {
              if(java_environment.DEBUG) System.err.println("###################### Proxy Proxy is dying");
            _holder.decRefCount();
        }
    }

    /*
    ** This is the holder class, whichs instances are put into the hashtable
    */
    class Holder {
        int    _refCount;
        String _oId;
        Object _object;

        Holder(String oId, Object object) {
            _oId    = oId;
            _object = object;
            _refCount = 1;
        }

        synchronized void incRefCount() {
              if(DEBUG) System.err.println("##### " + getClass().getName() + ".incRefCount:" + _refCount);

            ++ _refCount;
        }

        synchronized void decRefCount() {
              if(DEBUG) System.err.println("##### " + getClass().getName() + ".decRefCount:" + _refCount);

            -- _refCount;

            if(_refCount == 0)
                _objects.remove(_oId);
        }

        Object xxgetObject(Type type) {
            Object result = _object;

            if(_object instanceof Proxy) {
                if(DEBUG) System.err.println("##### " + getClass().getName() + " -  creating new Proxy Proxy");
                Class holderProxyClass = DispatcherAdapterFactory.createDispatcherAdapter(((TypeDescription)type.getTypeDescription()).getZClass(),
                                                                                          HolderProxy.class.getName().replace('.', '/'));

                try {
                    HolderProxy holderProxy = (HolderProxy)holderProxyClass.newInstance();
                    holderProxy.setObject(holderProxy, _object);
                    holderProxy.setHolder(this);
                    holderProxy.setInterface(type);

                    result = holderProxy;
                }
                catch(Exception exception) {
                    System.err.println("##### " + getClass().getName() + ".xxgetObject - exception occurred:" + exception);
                    exception.printStackTrace();

                    result = null;
                }
            }

            return result;
        }

        public String toString() {
            return "holder:" + _refCount + " " + _oId + " " + _object;
        }
    }

    protected Hashtable _objects;
    protected    String _name;
    // free context pointer, that can be used for specific classes of environments,
    protected    Object _context;

    public java_environment(Object context) {
        _name    = "java";
        _context = context;
        _objects = new Hashtable();

        if(DEBUG) System.err.println("##### " + getClass().getName()  + " - instantiated ");
    }

        /**
     * the context
     */
    public Object getContext() {
        return _context;
    }

    /**
     * a name for this environment
     */
    public String getName() {
        return _name;
    }

    /**
     * Tests if two environments are equal.
     *<BR>
     * @param environment       one environment
     */
    public boolean equals(Object object) {
        return false;
    }

    /**
     * You register internal and external interfaces via this method. Internal interfaces are
     * proxies that are used in an environment. External interfaces are interfaces that are
     * exported to another environment, thus providing an object identifier for this task.
     * This can be called an external reference.
     * Interfaces are held weakly at an environment; they demand a final revokeInterface()
     * call for each interface that has been registered.
     * <p>
     * @return  a proxy to registered interface if necessare, otherwise the registered object itself
     * @param object      the interface to register
     * @param oId[]       inout parameter for the corresponding object id
     * @param type  the type description of the given interface
     * @see               com.sun.star.uno.IEnvironment#registerInterface
     */
    public Object registerInterface(Object object, String oId[], Type type) {
        if(oId[0] == null)
            oId[0] = UnoRuntime.generateOid(object);

        String keyName = oId[0] + type;

        synchronized(_objects) {
            // get the holder
            Holder holder = (Holder)_objects.get(keyName);

            if(DEBUG)
                System.err.println("##### " + getClass().getName() + ".registerInterface:" + object + " " + oId[0] + " " + type);

            if(holder == null) {
                holder = new Holder(keyName, object);

                _objects.put(keyName, holder);
            }
            else
                holder.incRefCount();

            object = holder.xxgetObject(type);
        }

        return object;
    }

    /**
     * You have to revoke ANY interface that has been registered via this method.
     * <p>
     * @param oId         object id of interface to be revoked
     * @param type  the type description of the interface
     * @see               com.sun.star.uno.IEnvironment#revokeInterface
     */
    public void revokeInterface(String oId, Type type) {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".revokeInterface:" + oId + " " + type);
        synchronized(_objects) {
            Holder holder = (Holder)_objects.get(oId + type);
            if(holder != null)
                holder.decRefCount();
            else
                System.err.println("java_environment.revokeInterface - unknown oid:" + oId + " " + type);
        }
    }

    /**
     * Retrieves an interface identified by its object id and type from this environment.
     * <p>
     * @param oId        object id of interface to be retrieved
     * @param type the type description of the interface to be retrieved
     * @see               com.sun.star.uno.IEnvironment#getRegisteredInterface
     */
    public Object getRegisteredInterface(String oId, Type type)     {
        Object result = null;

        Holder holder = (Holder)_objects.get(oId + type);

        if(holder != null) {
            result = holder.xxgetObject(type);
        }

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".getRegisteredInterface:>" + oId + "< " + type +" " + result);

        return result;
    }

    /**
     * Retrieves the object identifier for a registered interface from this environment.
     * <p>
     * @param object      a registered interface
     * @see               com.sun.star.uno.IEnvironment#getRegisteredObjectIdentifier
     */
    public String getRegisteredObjectIdentifier(Object object) {
        return UnoRuntime.generateOid(object);
    }

    /**
     * List the registered interfaces.
     * <p>
     * @see               com.sun.star.uno.IEnvironment#list
     */
    public void list() {
        System.err.println("##### " + getClass().getName() + ".list(" + getName() + " " + getContext() + "):");

        Enumeration elements = _objects.elements();
        while(elements.hasMoreElements()) {
            System.err.println("#### key:" + elements.nextElement());
        }
    }

    /**
     * Dispose this environment
     * <p>
     * @see               com.sun.star.uno.IEnvironment#dispose
     */
    public void dispose() {
        if(_objects.size() > 0)
            list();

        _objects = new Hashtable();
    }

}

