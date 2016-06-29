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

import com.sun.star.uno.Type;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.uno.TypeClass;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.Any;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.beans.XPropertyChangeListener;
import com.sun.star.beans.XVetoableChangeListener;
import com.sun.star.beans.PropertyChangeEvent;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyAttribute;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XPropertiesChangeListener;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.beans.XFastPropertySet;
import com.sun.star.beans.PropertyVetoException;
import com.sun.star.beans.XMultiPropertySet;
import java.util.Iterator;
import java.util.Collection;
import java.util.HashMap;
import java.lang.reflect.Field;
import com.sun.star.lang.DisposedException;


/** This class is an implementation of the interfaces com.sun.star.beans.XPropertySet,
 *  com.sun.star.beans.XFastPropertySet and com.sun.star.beans.XMultiPropertySet. This
 *  class has to be inherited to be used. The values of properties are stored in member
 *  variables of the inheriting class. By overriding the methods
 *  {@link #convertPropertyValue convertPropertyValue},
 *  {@link #setPropertyValueNoBroadcast setPropertyValueNoBroadcast} and
 *  {@link #getPropertyValue(Property)} one can determine how
 *  property values are stored.
 *  When using the supplied implementations of this class then the member variables which
 *  hold property values have to be declared in the class which inherits last in the inheriting
 *  chain and they have to be public<p>
 *  Properties have to be registered by one of the registerProperty methods. They take among other
 *  arguments an Object named <em>id</em> which has to be a String that represents the name of
 *  the member variable. The registering has to occur in the constructor of the inheriting class.
 *  It is no allowed to add or change properties later on.<p>
 *  Example:
 *  <pre>
 *  public class Foo extends PropertySet
 *  {
 *      protected int intProp;
 *
 *      public Foo()
 *      {
 *          registerProperty("PropertyA", 0, new Type(int.class), (short)0, "intProp");
 *      }
 *  }
 *
 *  </pre>
 */
public class PropertySet extends ComponentBase implements XPropertySet, XFastPropertySet,
XMultiPropertySet
{
    private HashMap<String,Property> _nameToPropertyMap;
    private HashMap<Integer,Property> _handleToPropertyMap;
    private HashMap<Property,Object> _propertyToIdMap;
    private Property[] arProperties;

    private int lastHandle= 1;

    protected XPropertySetInfo propertySetInfo;
    protected MultiTypeInterfaceContainer aBoundLC= new MultiTypeInterfaceContainer();
    protected MultiTypeInterfaceContainer aVetoableLC= new MultiTypeInterfaceContainer();
    public PropertySet()
    {
        super();
        initMappings();
    }

    /** Registers a property with this helper class and associates the argument <em>id</em> with it.
     *  <em>id</em> is used to identify the storage of the property value. How property values are stored
     *  and retrieved is determined by the methods {@link #convertPropertyValue convertPropertyValue},
     *  {@link #setPropertyValueNoBroadcast setPropertyValueNoBroadcast} and {@link #getPropertyValue(Property) getPropertyValue}
     *  These methods expect <em>id</em> to be a java.lang.String which represents the name of a member variable
     *  which holds the property value.
     *  Only properties which are registered can be accessed. Registration has to occur during
     *  initialization of the inheriting class (i.e. within the contructor).
     *  @param prop The property to be registered.
     *  @param id Identifies the properties storage.
     *  @see #getPropertyId
     */
    protected void registerProperty(Property prop, Object id)
    {
        putProperty(prop);
        assignPropertyId(prop, id);
    }

    /** Registers a property with this helper class and associates the argument id with it.
     *  It does the same as {@link #registerProperty(Property, Object)}. The first four
     *  arguments are used to construct a Property object.
     *  Registration has to occur during
     *  initialization of the inheriting class (i.e. within the contructor)
     *  @param name The property's name (Property.Name).
     *  @param handle The property's handle (Property.Handle).
     *  @param type The property's type (Property.Type).
     *  @param attributes The property's attributes (Property.Attributes).
     *  @param id Identifies the property's storage.
     */
    protected void registerProperty(String name, int handle, Type type, short attributes, Object id)
    {
        Property p= new Property(name, handle, type, attributes);
        registerProperty(p, id);
    }

    /** Registers a property with this  class and associates the argument id with it.
     *  It does the same as {@link #registerProperty(Property, Object)}. The first three
     *  arguments are used to construct a Property object. The value for the Property.Handle
     *  is generated and does not have to be specified here. Use this method for registering
     *  a property if you do not care about the Property's handles.
     *  Registration has to occur during
     *  initialization of the inheriting class (i.e. within the contructor).
     *  @param name The property's name (Property.Name).
     *  @param type The property's type (Property.Type).
     *  @param attributes The property's attributes (Property.Attributes).
     *  @param id Identifies the property's storage.
     */
    protected void registerProperty(String name, Type type, short attributes, Object id)
    {
        Property p= new Property(name, lastHandle++, type, attributes);
        registerProperty(p, id);
    }

    /** Registers a property with this class. This method expects that property values
     *  are stored in member variables as is the case if the methods convertPropertyValue,
     *  setPropertyValueNoBroadcast and getPropertyValue(Property) are not overridden.
     *  It is presumed that the type of the member variable
     *  corresponds Property.Type. For example, if the TypeClass of Property.Type is to be
     *  a TypeClass.SHORT then the member must be a short or java.lang.Short.
     *  The handle for the property is generated.<br>
     *  If there is no member with the specified name or if the member has an incompatible type
     *  then a com.sun.star.uno.RuntimeException is thrown.
     *  @param propertyName The name of the property.
     *  @param memberName The name of the member variable that holds the value of the property.
     *  @param attributes The property attributes.
     */
    protected void registerProperty(String propertyName, String memberName, short attributes)
    {
        Field propField= null;
        try
        {
            propField= getClass().getDeclaredField(memberName);
        }
        catch (NoSuchFieldException e)
        {
            throw new com.sun.star.uno.RuntimeException("there is no member variable: " + memberName);
        }
        Class cl= propField.getType();
        Type t= new Type(cl);
        if (t.getTypeClass() != TypeClass.UNKNOWN)
        {
            Property p= new Property(propertyName, lastHandle++,  t, attributes);
            registerProperty(p,memberName);
        }
        else
            throw new com.sun.star.uno.RuntimeException("the member has an unknown type: " + memberName);
    }

    /** Registers a property with this class.
     *  It is presumed that the name of property is equal to the name of the member variable
     *  that holds the property value.
     *  @param propertyName The name of the property and the member variable that holds the property's value.
     *  @param attributes The property attributes.
     *  @see #registerProperty(String, String, short)
     */
    protected void registerProperty(String propertyName, short attributes)
    {
        registerProperty(propertyName, propertyName, attributes);
    }



    /** Returns the Property object for a given property name or null if that property does
     *  not exists (i.e. it has not been registered). Override this method
     *  if you want to implement your own mapping from property names to Property objects.
     *  Then you also have to override {@link #initMappings}, {@link #getProperties()} and
     *  {@link #putProperty(Property)}.
     *  @param propertyName The name of the property (Property.Name)
     *  @return The Property object with the name <em>propertyName</em>.
     */
    protected Property getProperty(String propertyName)
    {
        return _nameToPropertyMap.get(propertyName);
    }

    /** Returns the Property object with a handle (Property.Handle) as specified by the argument
     *  <em>nHandle</em>. The method returns null if there is no such property (i.e. it has not
     *  been registered). Override this method if you want to implement your own mapping from handles
     *  to Property objects. Then you also have to override {@link #initMappings}, {@link #putProperty(Property)}.
     *  @param nHandle The handle of the property (Property.Handle).
     *  @return The Property object with the handle <em>nHandle</em>
     */
    protected Property getPropertyByHandle(int nHandle)
    {
        return _handleToPropertyMap.get(new Integer(nHandle));
    }

    /** Returns an array of all Property objects or an array of length null if there
     *  are no properties. Override this method if you want to implement your own mapping from names
     *  to Property objects. Then you also have to override {@link #initMappings}, {@link #getProperty(String)} and
     *  {@link #putProperty}.
     *  @return Array of all Property objects.
     */
    protected Property[] getProperties()
    {
        if (arProperties == null)
        {
            Collection<Property> values= _nameToPropertyMap.values();
            arProperties= values.toArray(new Property[_nameToPropertyMap.size()]);
        }
        return arProperties;
    }

    /** Stores a Property object so that it can be retrieved subsequently by
     *  {@link #getProperty(String)},{@link #getProperties()},{@link #getPropertyByHandle(int)}.
     *  Override this method if you want to implement your own mapping from handles
     *  to Property objects and names to Property objects. Then you also need to override {@link #initMappings},
     *  {@link #getProperty(String)},{@link #getProperties()},{@link #getPropertyByHandle(int)}.
     *  @param prop The Property object that is to be stored.
     */
    protected void putProperty(Property prop)
    {
        _nameToPropertyMap.put(prop.Name, prop);
        if (prop.Handle != -1)
            _handleToPropertyMap.put(new Integer(prop.Handle), prop);
    }

    /** Assigns an identifyer object to a Property object so that the identifyer
     *  can be obtained by {@link #getPropertyId getPropertyId} later on. The identifyer
     *  is used to specify a certain storage for the property's value. If you do not
     *  override {@link #setPropertyValueNoBroadcast setPropertyValueNoBroadcast} or {@link #getPropertyValue(Property)}
     *  then the argument <em>id</em> has to be a java.lang.String that equals the name of
     *  the member variable that holds the Property's value.
     *  Override this method if you want to implement your own mapping from Property objects  to ids or
     *  if you need ids of a type other then java.lang.String.
     *  Then you also need to override {@link #initMappings initMappings} and {@link #getPropertyId getPropertyId}.
     *  @param prop The Property object that is being assigned an id.
     *  @param id The object which identifies the storage used for the property's value.
     *  @see #registerProperty(Property, Object)
     */
    protected void assignPropertyId(Property prop, Object id)
    {
       if (id instanceof String && ((String) id).equals("") == false)
            _propertyToIdMap.put(prop, id);
    }

    /** Returns the identifyer object for a certain Property. The object must have been
     *  previously assigned to the Property object by {@link #assignPropertyId assignPropertyId}.
     *  Override this method if you want to implement your own mapping from Property objects to ids.
     *  Then you also need to override {@link #initMappings initMappings} and {@link #assignPropertyId assignPropertyId}.
     *  @param prop The property for which the id is to be retrieved.
     *  @return The id object that identifies the storage used for the property's value.
     *  @see #registerProperty(Property, Object)
     */
    protected Object getPropertyId(Property prop)
    {
        return _propertyToIdMap.get(prop);
    }

    /** Initializes data structures used for mappings of property names to property object,
     *  property handles to property objects and property objects to id objects.
     *  Override this method if you want to implement your own mappings. Then you also need to
     *  override {@link #putProperty putProperty},{@link #getProperty getProperty}, {@link #getPropertyByHandle},
     *  {@link #assignPropertyId assignPropertyId} and {@link #getPropertyId getPropertyId}.
     */
    protected void initMappings()
    {
       _nameToPropertyMap= new HashMap<String,Property>();
       _handleToPropertyMap= new HashMap<Integer,Property>();
       _propertyToIdMap= new HashMap<Property,Object>();
    }

    /** Makes sure that listeners which are kept in aBoundLC (XPropertyChangeListener) and aVetoableLC
     *  (XVetoableChangeListener) receive a disposing call. Also those listeners are relesased.
     */
    protected void postDisposing()
    {
        // Create an event with this as sender
        EventObject aEvt= new EventObject(this);

        // inform all listeners to reelease this object
        aBoundLC.disposeAndClear(aEvt);
        aVetoableLC.disposeAndClear(aEvt);
    }

    //XPropertySet ----------------------------------------------------
    synchronized public void addPropertyChangeListener(String str, XPropertyChangeListener xPropertyChangeListener)
    throws UnknownPropertyException, WrappedTargetException
    {
          // only add listeners if you are not disposed
        if (! bInDispose && ! bDisposed)
        {
            if (str.length() > 0)
            {
                Property prop= getProperty(str);
                if (prop == null)
                    throw new UnknownPropertyException("Property " + str + " is unknown");

                // Add listener for a certain property
                if ((prop.Attributes & PropertyAttribute.BOUND) > 0)
                    aBoundLC.addInterface(str, xPropertyChangeListener);
                else
                    //ignore silently
                    return;
            }
            else
                // Add listener for all properties
                listenerContainer.addInterface(XPropertyChangeListener.class, xPropertyChangeListener);
        }
    }
    //XPropertySet ----------------------------------------------------
    synchronized public void addVetoableChangeListener(String str, com.sun.star.beans.XVetoableChangeListener xVetoableChangeListener) throws com.sun.star.beans.UnknownPropertyException, com.sun.star.lang.WrappedTargetException
    {
         // only add listeners if you are not disposed
        if (! bInDispose && ! bDisposed)
        {
            if (str.length() > 0)
            {
                Property prop= getProperty(str);
                if (prop == null)
                    throw new UnknownPropertyException("Property " + str + " is unknown");

                // Add listener for a certain property
                if ((prop.Attributes & PropertyAttribute.CONSTRAINED) > 0)
                    aVetoableLC.addInterface(str, xVetoableChangeListener);
                else
                    //ignore silently
                    return;
            }
            else
                // Add listener for all properties
                listenerContainer.addInterface(XVetoableChangeListener.class, xVetoableChangeListener);
        }
    }
    //XPropertySet ----------------------------------------------------
    public com.sun.star.beans.XPropertySetInfo getPropertySetInfo()
    {
        if (propertySetInfo == null)
        {
            synchronized (this)
            {
                if (propertySetInfo == null)
                    propertySetInfo= new PropertySetInfo();
            }
        }
        return propertySetInfo;
    }
    //XPropertySet ----------------------------------------------------
    public Object getPropertyValue(String name) throws UnknownPropertyException, WrappedTargetException
    {
        Object ret= null;
        if (bInDispose || bDisposed)
            throw new com.sun.star.lang.DisposedException("The component has been disposed already");

        Property prop= getProperty(name);
        if (prop == null)
            throw new UnknownPropertyException("The property " + name + " is unknown");

        synchronized (this)
        {
            ret= getPropertyValue(prop);
        }
        // null must not be returned. Either a void any is returned or an any containing
        // an interface type and a null reference.
        if (ret == null)
        {
            if (prop.Type.getTypeClass() == TypeClass.INTERFACE)
                ret= new Any(prop.Type, null);
            else
                ret= new Any(new Type(void.class), null);
        }
        return ret;
    }

    //XPropertySet ----------------------------------------------------
    synchronized public void removePropertyChangeListener(String propName, XPropertyChangeListener listener) throws UnknownPropertyException, WrappedTargetException
    {   // all listeners are automaticly released in a dispose call
        if (!bInDispose && !bDisposed)
        {
            if (propName.length() > 0)
            {
                Property prop = getProperty(propName);
                if (prop == null)
                    throw new UnknownPropertyException("Property " + propName + " is unknown");
                aBoundLC.removeInterface(propName, listener);
            }
            else
                listenerContainer.removeInterface(XPropertyChangeListener.class, listener);
        }
    }

    //XPropertySet ----------------------------------------------------
    synchronized public void removeVetoableChangeListener(String propName, XVetoableChangeListener listener) throws UnknownPropertyException, WrappedTargetException
    {// all listeners are automaticly released in a dispose call
        if (!bInDispose && !bDisposed)
        {
            if (propName.length() > 0)
            {
                Property prop = getProperty(propName);
                if (prop == null)
                    throw new UnknownPropertyException("Property " + propName + " is unknown");
                aVetoableLC.removeInterface(propName, listener);
            }
            else
                listenerContainer.removeInterface(XVetoableChangeListener.class, listener);
        }
    }

    //XPropertySet ----------------------------------------------------
    /** Sets the value of a property.
     *  The idl description for this interfaces, stipulates that the argument value is an Any. Since a java.lang.Object
     *  reference has the same meaning as an Any this function accepts
     *  java anys (com.sun.star.uno.Any) and all other appropriate objects as arguments. The value argument can be one
     *  of these:
     *  <ul>
     *  <li>java.lang.Boolean</li>
     *  <li>java.lang.Character</li>
     *  <li>java.lang.Byte</li>
     *  <li>java.lang.Short</li>
     *  <li>java.lang.Integer</li>
     *  <li>java.lang.Long</li>
     *  <li>java.lang.Float</li>
     *  <li>java.lang.Double</li>
     *  <li>java.lang.String</li>
     *  <li>com.sun.star.uno.Type</li>
     *  <li><em>objects which implement UNO interfaces</em></li>
     *  <li><em>arrays which contain elements of the types above</em></li>
     *  <li>com.sun.star.uno.Any containing an instance of one of the above types</li>
     *  </ul>
     *
     *  Properties can have the attribute com.sun.star.beans.PropertyAttribute.MAYBEVOID, which means that the value
     *  (not the type) can be void. In order to assign a void value to a property one can either pass an Any which
     *  contains a null reference or pass null directly. In bothe cases the null reference is only accepted if
     *  the PropertyAttribute.MAYBEVOID attribute is set for the property.
     *
     *  Properties which have the attribute MAYBEVOID set (Property.Attributes) can have a void value. The following
     *  considerations presume that the Property has that attribute set. Further, when mentioning an Any's value we
     *  actually refer to the object returned by Any.getObject.
     *  If the argument <em>value</em> is null, or it is an Any whose value is null (but with a valid Type)
     *  then the member variable used for storing the property's value is set to null.
     *  Therefore those properties can only be stored in objects
     *  and primitive types are not allowed (one can use the wrapper classes instead,e.g. java.lang.Byte) .
     *  If a property's value is kept in a member variable of type Any and that reference is still null
     *  then when setPropertyValue is called with
     *  <em>value</em> = null then the member variable is assigned an Any with type void and a null value.
     *  Or if the argument is an Any with a null value then it is assigned to the member variable.
     *  Further, if the variable already
     *  references an Any and setPropertyValue is called with <em>value</em> = null, then the variable is assigned
     *  a new Any with the same type as the previously referenced Any and with a null value.
     *  @param name The name of the property.
     *  @param value The new value of the property.
     *     *     */
    public void setPropertyValue(String name, Object value) throws UnknownPropertyException,
    PropertyVetoException, com.sun.star.lang.IllegalArgumentException,  WrappedTargetException
    {
        Property prop= getProperty(name);
        if (prop == null)
            throw new UnknownPropertyException("Property " + name + " is unknown");
        setPropertyValue(prop, value);
    }

    /** Sets the value of a property. It checks if the property's attributes (READONLY,MAYBEVOID), allow that the
     *  new value can be set. It also causes the notification of listeners.
     *  @param prop The property whose value is to be set.
     *  @param value The new value for the property.
     */
    protected void setPropertyValue(Property prop, Object value) throws UnknownPropertyException,
    PropertyVetoException, com.sun.star.lang.IllegalArgumentException, WrappedTargetException
    {
        if ((prop.Attributes & PropertyAttribute.READONLY) == PropertyAttribute.READONLY)
            throw new com.sun.star.beans.PropertyVetoException();
        // The value may be null only if MAYBEVOID attribute is set
        boolean bVoidValue= false;
        if (value instanceof Any)
            bVoidValue= ((Any) value).getObject() == null;
        else
            bVoidValue= value == null;
        if (bVoidValue && (prop.Attributes & PropertyAttribute.MAYBEVOID) == 0)
            throw new com.sun.star.lang.IllegalArgumentException("The property must have a value; the MAYBEVOID attribute is not set!");
        if (bInDispose || bDisposed)
            throw new DisposedException("Component is already disposed");

        //Check if the argument is allowed
        boolean bValueOk= false;
        if (value instanceof Any)
            bValueOk= checkType(((Any) value).getObject());
        else
            bValueOk= checkType(value);
        if (! bValueOk)
            throw new com.sun.star.lang.IllegalArgumentException("No valid UNO type");


        boolean bConversionOk= false;
        Object[] outConvertedVal= new Object[1];
        Object[] outOldValue= new Object[1];
        synchronized (this)
        {
            bConversionOk= convertPropertyValue(prop, outConvertedVal, outOldValue, value);
        }

        //The next step following the conversion is to set the new value of the property. Prior to this
        // the XVetoableChangeListener s have to be notified.
        if (bConversionOk)
        {
            // If the property is CONSTRAINED, then we must notify XVetoableChangeListener. The listener can throw a com.sun.star.lang.beans.PropertyVetoException which
            // will cause this method to return (the exception is not caught here).
            fire( new Property[]{prop}, outConvertedVal, outOldValue, true);

            synchronized (this)
            {
                setPropertyValueNoBroadcast(prop, outConvertedVal[0]);
            }
            // fire a change event (XPropertyChangeListener, PropertyAttribute.BOUND
            fire( new Property[]{prop}, outConvertedVal, outOldValue, false);
        }
    }

    /** Converts a value in a way so that it is appropriate for storing as a property value, that is
     *  {@link #setPropertyValueNoBroadcast setPropertyValueNoBroadcast} can process the value without any further
     *  conversion. This implementation presumes that
     *  the values are stored in member variables of the furthest inheriting class. For example,
     *  class A inherits this class then members of class A
     *  can hold property values. If there is a class B which inherits A then only members of B can hold
     *  property values. The variables must be public. A property must have been registered (e.g. by
     *  {@link #registerProperty(Property, Object)} in order for this method to work. The identifyer argument (type Object)
     *  used in the registerProperty methods must
     *  be a java.lang.String, which is, the name of the member variable that holds the property value.
     *  If one opts to store values differently then one may override
     *  this method, as well as {@link #setPropertyValueNoBroadcast setPropertyValueNoBroadcast} and
     *  {@link #getPropertyValue(Property) getPropertyValue(Property)}.
     *  This method is always called as a result of a call to one of the setter methods, such as
     *  {@link #setPropertyValue(String,Object) XPropertySet.setPropertyValue},
     *  {@link #setFastPropertyValue XFastPropertySet.setFastPropertyValue}
     *  and {@link #setPropertyValues XMultiPropertySet.setPropertyValues}.
     *  If this method fails, that is, it returns false or throws an exception, then no listeners are notified and the
     *  property value, that was intended to be changed, remains untouched.<p> This method does not have to deal with property attributes, such as
     *  PropertyAttribute.READONLY or PropertyAttribute.MAYBEVOID. The processing of these attributes occurs
     *  in the calling methods.<p>
     *  Only if this method returns successfully further processing, such
     *  as listener notification and finally the modifiction of the property's value, will occur.<p>
     *
     *  The actual modification of a property's value is done by {@link #setPropertyValueNoBroadcast setPropertyValueNoBroadcast}
     *  which is called subsequent to convertPropertyValue.
     *<p>
     *  This method converts values by help of the com.sun.star.uno.AnyConverter which only does a few widening
     *  conversions on integer types and floating point types. For example, there is the property PropA with a Type equivalent
     *  to int.class and the
     *  value of the property is to be stored in a member variable of type int with name intProp. Then setPropertyValue is
     *  called:
     *  <pre>
     *  set.setPropertyValue( "PropA", new Byte( (byte)111));
     *  </pre>
     *  At some point setPropertyValue will call convertPropertyValue and pass in the Byte object. Since we allow
     *  that Byte values can be used with the property and know that the value is to be stored in intProp (type int)
     *  we convert the Byte object into an Integer object which is then returned in the out-parameter <em>newVal</em>. This
     *  conversion is actually performed by the AnyConverter. Later
     *  the setPropertyValueNoBroadcast is called with that Integer object and the int value can be easily extracted
     *  from the object and be assigned to the member intProp.
     *  <p>
     *  The method handles Any arguments the same as Object arguments. That is, the <em>setVal</em> argument can
     *  be a java.lang.Boolean or a com.sun.star.uno.Any containing a java.lang.Boolean. Likewise, a member
     *  containing a property value can be a com.sun.star.uno.Any or an java.lang.Object.
     *  Then, no conversion is necessary, since they can hold all possible values. However, if
     *  the member is an Object and <em>setVal</em> is an Any then the object contained in the any is assigned to
     *  the member. The extra type information which exists as Type object in the Any will get lost. If this is not
     *  intended then use an Any variable rather then an Object.<p>
     *  If a member is an Object or Any and the argument <em>setVal</em> is an Object, other than String or array,
     *  then it is presumed to be an UNO object and queried for XInterface. If successful, the out-param <em>newVal</em>
     *  returns the XInterface.<p>
     *  If a member is an UNO interface, then <em>setVal</em> is queried for this interface and the result is returned.
     *  If <em>setVal</em> is null then <em>newVal</em> will be null too after return.
     *  <p>
     *  If a property value is stored using a primitive type the the out-parameters
     *  <em>curVal</em> and <em>newVal</em> contain the respective wrapper class (e.g.java.lang.Byte, etc.).
     *  curVal is used in calls to the XVetoableChangeListener and XPropertyChangeListener.
     *
     * @param property - in-param property for which the data is to be converted.
     * @param newVal - out-param which contains the converted value on return.
     * @param curVal - out-param the current value of the property. It is used in calls to the
     *                   XVetoableChangeListener and XPropertyChangeListener.
     *  @param setVal - in-param. The value that is to be converted so that it matches Property and the internally used
     *  dataformat for that property.
     *  @return true - Conversion was successful. <em>newVal</em> contains a valid value for the property. false -
     *  conversion failed for some reason.
     *  @throws com.sun.star.lang.IllegalArgumentException The value provided is unfit for the property.
     *  @throws com.sun.star.lang.WrappedTargetException - An exception occurred during the conversion, that is to be made known
     *  to the caller.
     */
    protected boolean convertPropertyValue(Property property, Object[] newVal, Object[]curVal,  Object setVal)
        throws com.sun.star.lang.IllegalArgumentException, WrappedTargetException, UnknownPropertyException
    {
        boolean ret= true;
        try
        {
            // get the member name
            String sMember= (String) getPropertyId(property);
            if (sMember != null)
            {
                // use reflection to obtain the field that holds the property value
                // Class.getDeclaredFields does not return inherited fields. One could use Class.getFields to
                // also get inherited fields, but only those which are public.
                Field propField= getClass().getDeclaredField(sMember);
                if (propField != null)
                {
                    curVal[0]= propField.get(this);
                    Class memberClass= propField.getType();

                    // MAYBEVOID: if setVal == null or it is an Any and getObject returns null, then a void value is to be set
                    // This works only if there are no primitive types. For those we use the respective wrapper classes.
                    // In this implementation, a null reference means void value.
                    boolean bVoidValue= false;
                    boolean bAnyVal= setVal instanceof Any;
                    if (bAnyVal)
                        bVoidValue= ((Any) setVal).getObject() == null;
                    else
                        bVoidValue= setVal == null;
                    if (bVoidValue && memberClass.isPrimitive())
                        throw new com.sun.star.lang.IllegalArgumentException("The implementation does not support the MAYBEVOID attribute for this property");

                    Object convObj= null;
                    //The member that keeps the value of the Property is an Any. It can contain all possible
                    //types, therefore a conversion is not necessary.
                    if (memberClass.equals(Any.class))
                    {
                        if (bAnyVal)
                            //parameter setVal is also an Any and can be used without further processing
                            convObj= setVal;
                        else
                        {
                            // Parameter setVal is not an Any. We need to construct an Any that contains
                            // the argument setVal.
                            // If setVal is an interface implementation then, we cannot constuct the
                            // Any with setVal.getClass(), because the Any.Type._typeClass would be TypeClass.UNKNOWN.
                            // We try to get an XInterface of setVal and set an XInterface type.
                            if (setVal instanceof XInterface)
                            {
                                XInterface xint= UnoRuntime.queryInterface(XInterface.class, setVal);
                                if (xint != null)
                                    convObj= new Any(new Type(XInterface.class), xint);
                            }
                            // The member is an any, and the past in argument was null reference (MAYBEVOID is set)
                            else if (setVal == null)
                            {
                                // if the any member is still null we create a void any
                                if (curVal[0] == null)
                                    convObj= new Any(new Type(), null);
                                else
                                {
                                    //otherwise we create an Any with the same type as a value of null;
                                    convObj= new Any( ((Any)curVal[0]).getType(), null);
                                }
                            }
                            else
                                convObj= new Any(new Type(setVal.getClass()), setVal);
                        }
                    }
                    else
                        convObj= convert(memberClass, setVal);
                    newVal[0]= convObj;
                }
            }
            else
                throw new UnknownPropertyException("Property " + property.Name + " is unknown");
        }
        catch (java.lang.NoSuchFieldException e)
        {
            throw new WrappedTargetException("Field does not exist", this, e);
        }
        catch (java.lang.IllegalAccessException e)
        {
            throw new WrappedTargetException("", this ,e);
        }
        return ret;
    }

    private boolean checkType(Object obj)
    {
        if (obj == null
        || obj instanceof Boolean
        || obj instanceof Character
        || obj instanceof Number
        || obj instanceof String
        || obj instanceof XInterface
        || obj instanceof Type
        || obj instanceof com.sun.star.uno.Enum
        || obj.getClass().isArray())
            return true;
        return false;
    }

    // Param object can be an Any or other object. If obj is null then the return value is null
    private Object convert( Class cl, Object obj) throws com.sun.star.lang.IllegalArgumentException
    {
        Object retVal= null;
       //The member that keeps the value of the Property is an Object.Objects are similar to Anys in that they can
       // hold all types.
        if (obj == null || (obj instanceof Any && ((Any) obj).getObject() == null))
            retVal= null;
        else if(cl.equals(Object.class))
        {
            if (obj instanceof Any)
                obj= ((Any) obj).getObject();
            retVal= obj;
        }
        else if(cl.equals(boolean.class))
            retVal= new Boolean(AnyConverter.toBoolean(obj));
        else if (cl.equals(char.class))
            retVal= new Character(AnyConverter.toChar(obj));
        else if (cl.equals(byte.class))
            retVal= new Byte(AnyConverter.toByte(obj));
        else if (cl.equals(short.class))
            retVal= new Short(AnyConverter.toShort(obj));
        else if (cl.equals(int.class))
            retVal= new Integer(AnyConverter.toInt(obj));
        else if (cl.equals(long.class))
            retVal= new Long(AnyConverter.toLong(obj));
        else if (cl.equals(float.class))
            retVal= new Float(AnyConverter.toFloat(obj));
        else if (cl.equals(double.class))
            retVal= new Double(AnyConverter.toDouble(obj));
        else if (cl.equals(String.class))
            retVal= AnyConverter.toString(obj);
        else if (cl.isArray())
            retVal= AnyConverter.toArray(obj);
        else if (cl.equals(Type.class))
            retVal= AnyConverter.toType(obj);
        else if (cl.equals(Boolean.class))
            retVal= new Boolean(AnyConverter.toBoolean(obj));
        else if (cl.equals(Character.class))
            retVal= new Character(AnyConverter.toChar(obj));
        else if (cl.equals(Byte.class))
            retVal= new Byte(AnyConverter.toByte(obj));
        else if (cl.equals(Short.class))
            retVal= new Short(AnyConverter.toShort(obj));
        else if (cl.equals(Integer.class))
            retVal= new Integer(AnyConverter.toInt(obj));
        else if (cl.equals(Long.class))
            retVal= new Long(AnyConverter.toLong(obj));
        else if (cl.equals(Float.class))
            retVal= new Float(AnyConverter.toFloat(obj));
        else if (cl.equals(Double.class))
            retVal= new Double(AnyConverter.toDouble(obj));
        else if (XInterface.class.isAssignableFrom(cl))
            retVal= AnyConverter.toObject(new Type(cl), obj);
        else if (com.sun.star.uno.Enum.class.isAssignableFrom(cl))
            retVal= AnyConverter.toObject(new Type(cl), obj);
        else
            throw new com.sun.star.lang.IllegalArgumentException("Could not convert the argument");
        return retVal;
    }

    /**  Sets the value of a property. In this implementation property values are stored in member variables
     *  (see {@link #convertPropertyValue convertPropertyValue} Notification of property listeners
     *  does not occur in this method. By overriding this method one can take full control about how property values
     *  are stored. But then, the {@link #convertPropertyValue convertPropertyValue} and
     *  {@link #getPropertyValue(Property)} must be overridden too.
     *
     *  A Property with the MAYBEVOID attribute set, is stored as null value. Therefore the member variable must be
     *  an Object in order to make use of the property attribute. An exception is Any. The Any variable can be initially null, but
     *  once it is set the reference will not become null again. If the value is to be set to
     *  void then a new Any will be stored
     *  with a valid type but without a value (i.e. Any.getObject returns null).
     *  If a property has the READONLY attribute set, and one of the setter methods, such as setPropertyValue, has been
     *  called, then this method is not going to be called.
     *  @param property the property for which the new value is set
     *  @param newVal the new value for the property.
     *  @throws com.sun.star.lang.WrappedTargetException An exception, which has to be made known to the caller,
     *  occurred during the setting of the value.
     */
    protected void setPropertyValueNoBroadcast(Property property, Object newVal)
    throws WrappedTargetException
    {
        try
        {
            // get the member name
            String sMember= (String) getPropertyId(property);
            if (sMember != null)
            {
                // use reflection to obtain the field that holds the property value
                // Class.getDeclaredFields does not return inherited fields. One could use Class.getFields to
                // also get inherited fields, but only those which are public.
                Field propField= getClass().getDeclaredField(sMember);
                if (propField != null)
                    propField.set(this, newVal);
            }
        }
        catch(java.lang.Exception e)
        {
            throw new WrappedTargetException("PropertySet.setPropertyValueNoBroadcast", this, e);
        }
    }
    /** Retrieves the value of a property. This implementation presumes that the values are stored in member variables
     *  of the furthest inheriting class (see {@link #convertPropertyValue convertPropertyValue}) and that the
     *  variables are public. The property must have
     *  been registered, for example by {@link #registerProperty(Property, Object)}. The identifyer Object argument
     *  must have been a java.lang.String which was the name of the member variable holding the property value.
     *  When properties are to be stored differently one has to override this method as well as
     *  {@link #convertPropertyValue} and {@link #setPropertyValueNoBroadcast}. <br>
     *  If a value is stored in a variable of a primitive type then this method returns an instance of the respective
     *  wrapper class (e.g. java.lang.Boolean).
     *  @param property The property for which the value is to be retrieved.
     *  @return The value of the property.
     */
    protected Object getPropertyValue(Property property)
    {
        Object ret= null;
        try
        {
            // get the member name
            String sMember= (String) getPropertyId(property);
            if (sMember != null)
            {
                // use reflection to obtain the field that holds the property value
                // Class.getDeclaredFields does not return inherited fields. One could use Class.getFields to
                // also get inherited fields, but only those which are public.
                Field propField= getClass().getDeclaredField(sMember);
                if (propField != null)
                    ret= propField.get(this);
            }
        }
        catch(java.lang.NoSuchFieldException e)
        {
            throw new java.lang.RuntimeException(e);
        }
        catch(java.lang.IllegalAccessException e)
        {
            throw new java.lang.RuntimeException(e);
        }
        return ret;
    }

    /**
     *  This method fires events to XPropertyChangeListener,XVetoableChangeListener and
     *  XPropertiesChangeListener event sinks.
     *  To distinguish what listeners are to be called the argument <em>bVetoable</em> is to be set to true if
     *  a XVetoableChangeListener is meant. For XPropertyChangeListener and XPropertiesChangeListener
     *  it is to be set to false.
     *
     * @param properties    Properties wich will be or have been affected.
     * @param newValues the new values of the properties.
     * @param oldValues the old values of the properties.
     * @param bVetoable true means fire to VetoableChangeListener, false means fire to
     * XPropertyChangedListener and XMultiPropertyChangedListener.
     */
    protected void  fire(
    Property[]  properties,
    Object[] newValues,
    Object[] oldValues,
    boolean bVetoable ) throws PropertyVetoException
    {
        // Only fire, if one or more properties changed
        int nNumProps= properties.length;
        if (nNumProps > 0)
        {
            PropertyChangeEvent[] arEvts= new PropertyChangeEvent[nNumProps];
            int nAffectedProps= 0;
            // Loop over all changed properties to fill the event struct
            for (int i= 0; i < nNumProps; i++)
            {
                if ((bVetoable && (properties[i].Attributes & PropertyAttribute.CONSTRAINED) > 0)
                    || (!bVetoable && (properties[i].Attributes & PropertyAttribute.BOUND) > 0))
                {
                    arEvts[i]= new PropertyChangeEvent(this, properties[i].Name, false,
                                        properties[i].Handle, oldValues[i], newValues[i]);
                    nAffectedProps++;
                }
            }
            // fire the events for all changed properties
            for (int i= 0; i < nAffectedProps; i++)
            {
                // get the listener container for the property name
                InterfaceContainer lc= null;
                if (bVetoable)
                    lc= aVetoableLC.getContainer(arEvts[i].PropertyName);
                else
                    lc= aBoundLC.getContainer(arEvts[i].PropertyName);
                if (lc != null)
                {
                    Iterator it= lc.iterator();
                    while( it.hasNext())
                    {
                        Object listener= it.next();
                        if (bVetoable)
                            ((XVetoableChangeListener) listener).vetoableChange(arEvts[i]);
                        else
                            ((XPropertyChangeListener) listener).propertyChange(arEvts[i]);
                    }
                }
                   // broadcast to all listeners with "" property name
                if(bVetoable)
                    lc= listenerContainer.getContainer(XVetoableChangeListener.class);
                else
                    lc= listenerContainer.getContainer(XPropertyChangeListener.class);
                if(lc != null)
                {
                    Iterator it= lc.iterator();
                    while(it.hasNext() )
                    {
                        Object listener= it.next();
                        if( bVetoable ) // fire change Events?
                            ((XVetoableChangeListener) listener).vetoableChange(arEvts[i]);
                        else
                            ((XPropertyChangeListener) listener).propertyChange(arEvts[i]);
                    }
                }
            }
            // fire at XPropertiesChangeListeners
            // if nAffectedProps == 0 then there are no BOUND properties
            if (!bVetoable && nAffectedProps > 0)
            {

                PropertyChangeEvent[] arReduced= new PropertyChangeEvent[nAffectedProps];
                System.arraycopy(arEvts, 0, arReduced, 0, nAffectedProps);
                InterfaceContainer lc= listenerContainer.getContainer(XPropertiesChangeListener.class);
                if (lc != null)
                {
                    Iterator it= lc.iterator();
                    while (it.hasNext())
                    {
                        XPropertiesChangeListener listener = (XPropertiesChangeListener) it.next();
                        // fire the hole event sequence to the XPropertiesChangeListener's
                        listener.propertiesChange( arEvts );
                    }
                }
            }
        }
    }
    // XFastPropertySet--------------------------------------------------------------------------------
    public void setFastPropertyValue(int nHandle, Object aValue ) throws UnknownPropertyException,
    PropertyVetoException, com.sun.star.lang.IllegalArgumentException, WrappedTargetException
    {
        Property prop= getPropertyByHandle(nHandle);
        if (prop == null)
            throw new UnknownPropertyException(" The property with handle : " + nHandle +" is unknown");
        setPropertyValue(prop, aValue);
    }

    // XFastPropertySet --------------------------------------------------------------------------------
    public Object getFastPropertyValue(int nHandle ) throws UnknownPropertyException,
    WrappedTargetException
    {
        Property prop= getPropertyByHandle(nHandle);
        if (prop == null)
            throw new UnknownPropertyException("The property with handle : " + nHandle + " is unknown");
        return getPropertyValue(prop);
    }

    // XMultiPropertySet -----------------------------------------------------------------------------------
    public void addPropertiesChangeListener(String[] propNames, XPropertiesChangeListener listener)
    {
        listenerContainer.addInterface(XPropertiesChangeListener.class, listener);
    }

    // XMultiPropertySet -----------------------------------------------------------------------------------
    public void firePropertiesChangeEvent(String[] propNames, XPropertiesChangeListener listener)
    {
        // Build the events.
        PropertyChangeEvent[] arEvents= new PropertyChangeEvent[propNames.length];
        int eventCount= 0;
        // get a snapshot of the current property values
        synchronized (this)
        {
            for (int i= 0; i < propNames.length; i++)
            {
                Property prop= getProperty(propNames[i]);
                if (prop != null)
                {
                    Object value= null;
                    try
                    {
                       value= getPropertyValue(prop);
                    }
                    catch(Exception e)
                    {
                        continue;
                    }
                    arEvents[eventCount]= new PropertyChangeEvent(this, prop.Name,
                                        false, prop.Handle, value, value);
                    eventCount++;
                }
            }
        }

        // fire events from unsynchronized section so as to prevent deadlocks
        if (eventCount > 0)
        {
            // Reallocate the array of the events if necessary
            if (arEvents.length != eventCount)
            {
                PropertyChangeEvent[] arPropsTmp= new PropertyChangeEvent[eventCount];
                System.arraycopy(arEvents, 0, arPropsTmp, 0, eventCount);
                arEvents= arPropsTmp;
            }
            listener.propertiesChange(arEvents);
        }
    }
    // XMultiPropertySet -----------------------------------------------------------------------------------
    /** If a value for a property could not be retrieved then the respective element in the returned
     *  array has the value null.
     */
    public Object[] getPropertyValues(String[] propNames)
    {
        Object[] arValues= new Object[propNames.length];
        synchronized (this)
        {
            for (int i= 0; i < propNames.length; i++)
            {
                Object value= null;
                try
                {
                    value= getPropertyValue(propNames[i]);
                }
                catch (Exception e)
                {
                }
                arValues[i]= value;
            }
        }
        return arValues;
    }
    // XMultiPropertySet -----------------------------------------------------------------------------------
    public void removePropertiesChangeListener(XPropertiesChangeListener xPropertiesChangeListener)
    {
        listenerContainer.removeInterface(XPropertiesChangeListener.class, xPropertiesChangeListener);
    }
    // XMultiPropertySet -----------------------------------------------------------------------------------
    /** If the array of property names containes an unknown property then it will be ignored.
     */
    public void setPropertyValues(String[] propNames, Object[] values) throws PropertyVetoException, com.sun.star.lang.IllegalArgumentException, com.sun.star.lang.WrappedTargetException
    {
        for (int i= 0; i < propNames.length; i++)
        {
            try
            {
                setPropertyValue(propNames[i], values[i]);
            }
            catch (UnknownPropertyException e)
            {
                continue;
            }

        }
    }

    private class PropertySetInfo implements XPropertySetInfo
    {
        public com.sun.star.beans.Property[] getProperties()
        {
            return PropertySet.this.getProperties();
        }

        public com.sun.star.beans.Property getPropertyByName(String name) throws UnknownPropertyException
        {
            return getProperty(name);
        }

        public boolean hasPropertyByName(String name)
        {
            return getProperty(name) != null;
        }

    }
}





