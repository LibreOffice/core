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
package com.sun.star.cmp;

import com.sun.star.io.XPersistObject;
import com.sun.star.io.XObjectInputStream;
import com.sun.star.io.XObjectOutputStream;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.beans.Property;
import com.sun.star.beans.XPropertyChangeListener;
import com.sun.star.beans.XVetoableChangeListener;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.uno.XInterface;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.comp.loader.FactoryHelper;
import com.sun.star.uno.Type;

/**
 * Class MyPersistObject implements an XPersistObject, XServiceInfo,
 * XTypeProvider and XPropertySet.
 *
 * Warning: In XPropertySet only the following methods that are
 *          used for testing are really implemented:
 *
 *          - public XPropertySetInfo getPropertySetInfo()
 *          - public void setPropertyValue(String property, Object value)
 *          - public Object getPropertyValue(String property)
 */
public class MyPersistObject implements XPersistObject, XTypeProvider,
                                                XServiceInfo, XPropertySet {

    private class MyPropertySetInfo implements XPropertySetInfo {
        Property[] _props;
        public MyPropertySetInfo(Property[] props) {
            _props = props;
        }
        public Property[] getProperties() {
            return _props;
        }
        public Property getPropertyByName(String name) {
            int i = getPropertyIndexByName(name);
            return i>0?_props[i]:null;
        }
        public int getPropertyIndexByName(String name) {
            for ( int i=0; i<_props.length; i++ )
                if (name.equals(_props[i].Name))
                    return i;
            return -1;
        }
        public boolean hasPropertyByName(String name) {
            int i = getPropertyIndexByName(name);
            return i>0?true:false;
        }
    }

    static private final boolean verbose = false;

    static public final String __serviceName =
                                        "com.sun.star.cmp.PersistObject";
    static public final String __implName =
                                        "com.sun.star.cmp.MyPersistObject";

    // lots of props to write
    Property[] props;
    private byte by;
    private int i;
    private char c;
    private double d;
    private float f;
    private short s;
    private String st;
    // property set info
    XPropertySetInfo xInfo;

    /**
     * Constructor: sets all properties
     **/
    public MyPersistObject() {
        int prop_count = 7;
        props = new Property[prop_count];
        for (int i=0; i<prop_count; i++ ) {
            props[i] = new Property();
        }
        by = 1;
        props[0].Name = "byte";
        i = 3;
        props[1].Name = "int";
        c = 'c';
        props[2].Name = "char";
        d = 3.142;
        props[3].Name = "double";
        f = 2.718f;
        props[4].Name = "float";
        s = 1;
        props[5].Name = "short";
        st = "Though this be madness, yet there is method in 't.";
        props[6].Name = "String";
        xInfo = new MyPropertySetInfo(props);
    }
    /**
     * This function provides the service name
     * @return the service name
     * @see com.sun.star.io.XPersistObject
     */
    public String getServiceName() {
        if ( verbose ) {
            System.out.println("get service name");
        }
        return __serviceName;
    }

    /**
     * Fuction reads properties from this input stream
     * @param inStream the input stream
     * @see com.sun.star.io.XPersistObject
     */
    public void read(XObjectInputStream inStream)
                            throws com.sun.star.io.IOException {
        s = inStream.readShort();
        i = inStream.readLong();
        by = inStream.readByte();
        c = inStream.readChar();
        d = inStream.readDouble();
        f = inStream.readFloat();
        st = inStream.readUTF();
        if ( verbose )
            System.out.println("read called" + s + " " + i + " " + st);
    }

    /**
     * Fuction writes properties on this output stream
     * @param outStream the output stream
     * @see com.sun.star.io.XPersistObject
     */
    public void write(XObjectOutputStream outStream)
                            throws com.sun.star.io.IOException {
        if ( verbose )
            System.out.println("write called");
        outStream.writeShort(s);
        outStream.writeLong(i);
        outStream.writeByte(by);
        outStream.writeChar(c);
        outStream.writeDouble(d);
        outStream.writeFloat(f);
        outStream.writeUTF(st);

    }


    /**
     * Function to get information about the property set.
     * @return The information
     * @see com.sun.star.io.XPropertySet
     */
    public XPropertySetInfo getPropertySetInfo() {
        return xInfo;
    }

    /**
     * Set a property value
     * @param property The name of the property.
     * @param value The new value of the property.
     * @see com.sun.star.io.XPropertySet
     */
    public void setPropertyValue(String property, Object value) {
        if ( property.equals(props[0].Name))
            by = ((Byte)value).byteValue();
        if ( property.equals(props[1].Name))
            i = ((Integer)value).intValue();
        if ( property.equals(props[2].Name))
            c = ((Character)value).charValue();
        if ( property.equals(props[3].Name))
            d = ((Double)value).doubleValue();
        if ( property.equals(props[4].Name))
            f = ((Float)value).floatValue();
        if ( property.equals(props[5].Name))
            s = ((Short)value).shortValue();
        if ( property.equals(props[6].Name))
            st = (String)value;
    }

    /**
     * Get a property value
     * @param property The property name.
     * @return The value of the property.
     * @see com.sun.star.io.XPropertySet
     */
    public Object getPropertyValue(String property) {
        if ( property.equals(props[0].Name))
            return new Byte(by);
        if ( property.equals(props[1].Name))
            return new Integer(i);
        if ( property.equals(props[2].Name))
            return new Character(c);
        if ( property.equals(props[3].Name))
            return new Double(d);
        if ( property.equals(props[4].Name))
            return new Float(f);
        if ( property.equals(props[5].Name))
            return new Short(s);
        if ( property.equals(props[6].Name))
            return st;
        return new Object();
    }

    /**
     * Empty implementation: not needed for tests.
     */
    public void addPropertyChangeListener(String aPropertyName,
             XPropertyChangeListener xListener ) {}

    /**
     * Empty implementation: not needed for tests.
     */
    public void removePropertyChangeListener(String aPropertyName,
             XPropertyChangeListener aListener ) {}

    /**
     * Empty implementation: not needed for tests.
     */
    public void addVetoableChangeListener(String PropertyName,
             XVetoableChangeListener aListener ) {}

    /**
     * Empty implementation: not needed for tests.
     */
    public void removeVetoableChangeListener(String PropertyName,
             XVetoableChangeListener aListener ) {}

    /**
     * Get all implemented types of this class.
     * @return An array of implemented interface types.
     * @see com.sun.star.lang.XTypeProvider
     */
    public Type[] getTypes() {
        Type[] type = new Type[5];
        type[0] = new Type(XInterface.class);
        type[1] = new Type(XTypeProvider.class);
        type[2] = new Type(XPersistObject.class);
        type[3] = new Type(XServiceInfo.class);
        type[4] = new Type(XPropertySet.class);
        return type;
    }

    /**
     * Get the implementation id.
     * @return An empty implementation id.
     * @see com.sun.star.lang.XTypeProvider
     */
    public byte[] getImplementationId() {
        return new byte[0];
    }
    /**
     * Function for reading the implementation name.
     *
     * @return the implementation name
     * @see com.sun.star.lang.XServiceInfo
     */
    public String getImplementationName() {
        return __implName;
    }

    /**
     * Does the implementation support this service?
     *
     * @param serviceName The name of the service in question
     * @return true, if service is supported, false otherwise
     * @see com.sun.star.lang.XServiceInfo
     */
    public boolean supportsService(String serviceName) {
        if(serviceName.equals(__serviceName))
            return true;
        return false;
    }

    /**
     * Function for reading all supported services
     *
     * @return An aaray with all supported service names
     * @see com.sun.star.lang.XServiceInfo
     */
    public String[] getSupportedServiceNames() {
        String[] supServiceNames = {__serviceName};
        return supServiceNames;
    }

   /**
   *
   * Gives a factory for creating the service.
   * This method is called by the <code>JavaLoader</code>
   * <p>
   * @return  returns a <code>XSingleServiceFactory</code> for creating the component
   * @param   implName     the name of the implementation for which a service is desired
   * @param   multiFactory the service manager to be used if needed
   * @param   regKey       the registryKey
   * @see                  com.sun.star.comp.loader.JavaLoader
   */
    public static XSingleServiceFactory __getServiceFactory(String implName,
                    XMultiServiceFactory multiFactory, XRegistryKey regKey)
    {
        XSingleServiceFactory xSingleServiceFactory = null;

        if (implName.equals(MyPersistObject.class.getName()))
            xSingleServiceFactory = FactoryHelper.getServiceFactory(
                MyPersistObject.class, __serviceName, multiFactory, regKey);

        return xSingleServiceFactory;
    }

  /**
   * Writes the service information into the given registry key.
   * This method is called by the <code>JavaLoader</code>
   * <p>
   * @return  returns true if the operation succeeded
   * @param   regKey       the registryKey
   * @see                  com.sun.star.comp.loader.JavaLoader
   */
    public static boolean __writeRegistryServiceInfo(XRegistryKey regKey) {
        return FactoryHelper.writeRegistryServiceInfo(MyPersistObject.class.getName(),
        __serviceName, regKey);
    }




}   // finish class MyPersistObject


