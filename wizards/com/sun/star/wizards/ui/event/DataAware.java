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
package com.sun.star.wizards.ui.event;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.Arrays;
import java.util.Collection;
import java.util.Iterator;
import com.sun.star.wizards.common.PropertyNames;

/**
 * DataAware objects are used to live-synchronize UI and DataModel/DataObject.
 * It is used as listener on UI events, to keep the DataObject up to date.
 * This class, as a base abstract class, sets a frame of functionality,
 * delegating the data Object get/set methods to a Value object,
 * and leaving the UI get/set methods abstract.
 * Note that event listenning is *not* a part of this model.
 * the updateData() or updateUI() methods should be porogramatically called.
 * in child classes, the updateData() will be binded to UI event calls.
 * <br><br>
 * This class holds references to a Data Object and a Value object.
 * The Value object "knows" how to get and set a value from the 
 * Data Object.
 */
public abstract class DataAware {

    /**
     * this is the data object.
     */
    protected Object dataObject;
    /**
     * A Value Object knows how to get/set a value
     * from/to the data object.
     */
    protected Value value;
    
    /**
     * creates a DataAware object for the given data object and Value object.
     * @param dataObject_
     * @param value_
     */
    protected DataAware(Object dataObject_, Value value_) {
        dataObject = dataObject_;
        value = value_;
    }

    /**
     * returns the data object.
     * @return
     */
    public Object getDataObject() {
        return dataObject;
    }

    /**
     * sets a new data object. Optionally
     * update the UI.
     * @param obj the new data object.
     * @param updateUI if true updateUI() will be called.
     */
    public void setDataObject(Object obj, boolean updateUI) {

        if (obj != null && !value.isAssignable(obj.getClass())) 
            throw new ClassCastException("can not cast new DataObject to original Class");

        dataObject = obj;

        if (updateUI)
            updateUI();

    }

    /**
     * Sets the given value to the data object.
     * this method delegates the job to the
     * Value object, but can be overwritten if
     * another kind of Data is needed.
     * @param newValue the new value to set to the DataObject.
     */
    protected void setToData(Object newValue) {
        value.set(newValue,getDataObject());
    }

    /**
     * gets the current value from the data obejct.
     * this method delegates the job to 
     * the value object.
     * @return the current value of the data object.
     */
    protected Object getFromData() {
        return value.get(getDataObject());
    }
    
    /**
     * sets the given value to the UI control
     * @param newValue the value to set to the ui control.
     */
    protected abstract void setToUI(Object newValue);

    /**
     * gets the current value from the UI control.
     * @return the current value from the UI control.
     */
    protected abstract Object getFromUI();

    /**
     * updates the UI control according to the
     * current state of the data object.
     */
    public void updateUI() {
        Object data = getFromData();
        Object ui = getFromUI();
        if (!equals(data, ui))
            try {
                setToUI(data);
            } catch (Exception ex) {
                ex.printStackTrace();
                //TODO tell user...
            }
        enableControls(data);
    }

    /**
     * enables
     * @param currentValue
     */
    protected void enableControls(Object currentValue) {
    }

    /**
     * updates the DataObject according to
     * the current state of the UI control.
     */
    public void updateData() {
        Object data = getFromData();
        Object ui = getFromUI();
        if (!equals(data, ui))
            setToData(ui);
        enableControls(ui);
    }

    public interface Listener {
        public void eventPerformed(Object event);
    }

    /**
     * compares the two given objects.
     * This method is null safe and returns true also if both are null...
     * If both are arrays, treats them as array of short and compares them.
     * @param a first object to compare
     * @param b second object to compare.
     * @return true if both are null or both are equal.
     */
    protected boolean equals(Object a, Object b) {
        if (a == null && b == null)
            return true;
        if (a == null || b == null)
            return false;
        if (a.getClass().isArray()) {
            if (b.getClass().isArray())
                return Arrays.equals((short[]) a, (short[]) b);
            else
                return false;
        }
        return a.equals(b);
    }

    /**
     * given a collection containing DataAware objects,
     * calls updateUI() on each memebr of the collection.
     * @param dataAwares a collection containing DataAware objects.
     */
    public static void updateUI(Collection<DataAware> dataAwares) {
        for (Iterator<DataAware> i = dataAwares.iterator(); i.hasNext();)
             i.next().updateUI();
    }

    public static void updateData(Collection<DataAware> dataAwares) {
        for (Iterator<DataAware> i = dataAwares.iterator(); i.hasNext();)
             i.next().updateData();
    }

    /**
     * /**
     * Given a collection containing DataAware objects,
     * sets the given DataObject to each DataAware object
     * in the given collection
     * @param dataAwares a collection of DataAware objects.
     * @param dataObject new data object to set to the DataAware objects in the given collection.
     * @param updateUI if true, calls updateUI() on each DataAware object.
     */public static void setDataObject(Collection<DataAware> dataAwares, Object dataObject, boolean updateUI) {
        for (Iterator<DataAware> i = dataAwares.iterator(); i.hasNext();)
             i.next().setDataObject(dataObject, updateUI);
    }
    
    /**
     * Value objects read and write a value from and
     * to an object. Typically using reflection and JavaBeans properties 
     * or directly using memeber reflection API.
     * DataAware delegates the handling of the DataObject
     * to a Value object.
     * 2 implementations currently exist: PropertyValue,
     * using JavaBeans properties reflection, and DataAwareFields classes
     * which implement different memeber types.
     */
    public interface Value {
        /**
         * gets a value from the given object.
         * @param target the object to get the value from.
         * @return the value from the given object.
         */
        public Object get(Object target);
        /**
         * sets a value to the given object.
         * @param value the value to set to the object.
         * @param target the object to set the value to.
         */
        public void set(Object value, Object target);
        /**
         * checks if this Value object can handle
         * the given object type as a target.
         * @param type the type of a target to check
         * @return true if the given class is acceptible for
         * the Value object. False if not.
         */
        public boolean isAssignable(Class<?> type);
    }
    
    /**
     * implementation of Value, handling JavaBeans properties through
     * reflection.
     * This Object gets and sets a value a specific
     * (JavaBean-style) property on a given object.
     * @author rp143992
     */
    public static class PropertyValue implements Value {
        /**
         * the get method of the JavaBean-style property
         */
        private Method getMethod;
        /**
         * the set method of the JavaBean-style property
         */
        private Method setMethod;

        /**
         * creates a PropertyValue for the property with
         * the given name, of the given JavaBean object.
         * @param propertyName the property to access. Must be a Cup letter (e.g. PropertyNames.PROPERTY_NAME for getName() and setName("..."). )
         * @param propertyOwner the object which "own" or "contains" the property.
         */
        public PropertyValue(String propertyName, Object propertyOwner) {
            getMethod = createGetMethod(propertyName, propertyOwner);
            setMethod = createSetMethod(propertyName, propertyOwner, getMethod.getReturnType());
        }
        
        /**
         * called from the constructor, and creates a get method reflection object
         * for the given property and object.
         * @param propName the property name0
         * @param obj the object which contains the property.
         * @return the get method reflection object.
         */
        protected Method createGetMethod(String propName, Object obj)
        {
            Method m = null;
            try
            { //try to get a "get" method.

                m = obj.getClass().getMethod("get" + propName);
            }
            catch (NoSuchMethodException ex1)
            {
                throw new IllegalArgumentException("get" + propName + "() method does not exist on " + obj.getClass().getName());
            }
            return m;
        }

        /* (non-Javadoc)
         * @see com.sun.star.wizards.ui.event.DataAware.Value#get(java.lang.Object)
         */
        public Object get(Object target) {
            try {
                return getMethod.invoke(target);
            } catch (IllegalAccessException ex1) {
                ex1.printStackTrace();
            } catch (InvocationTargetException ex2) {
                ex2.printStackTrace();
            } catch (NullPointerException npe) {
                if (getMethod.getReturnType().equals(String.class))
                    return PropertyNames.EMPTY_STRING;
                if (getMethod.getReturnType().equals(Short.class))
                    return new Short((short) 0);
                if (getMethod.getReturnType().equals(Integer.class))
                    return 0;
                if (getMethod.getReturnType().equals(short[].class))
                    return new short[0];
            }
            return null;
        }

        protected Method createSetMethod(String propName, Object obj, Class<?> paramClass) {
            Method m = null;
            try {
                m = obj.getClass().getMethod("set" + propName, paramClass);
            } catch (NoSuchMethodException ex1) {
                throw new IllegalArgumentException("set" + propName + "(" + getMethod.getReturnType().getName() + ") method does not exist on " + obj.getClass().getName());
            }
            return m;
        }

        /* (non-Javadoc)
         * @see com.sun.star.wizards.ui.event.DataAware.Value#set(java.lang.Object, java.lang.Object)
         */
        public void set(Object value, Object target) {
            try {
                setMethod.invoke(target, value);
            } catch (IllegalAccessException ex1) {
                ex1.printStackTrace();
            } catch (InvocationTargetException ex2) {
                ex2.printStackTrace();
            }
        }

        /* (non-Javadoc)
         * @see com.sun.star.wizards.ui.event.DataAware.Value#isAssignable(java.lang.Class)
         */
        public boolean isAssignable(Class<?> type) {
            return getMethod.getDeclaringClass().isAssignableFrom(type) &&
                setMethod.getDeclaringClass().isAssignableFrom(type);
        }
    }
}
