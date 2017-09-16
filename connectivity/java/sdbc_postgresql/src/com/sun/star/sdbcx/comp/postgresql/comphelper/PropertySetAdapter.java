/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

package com.sun.star.sdbcx.comp.postgresql.comphelper;

import java.util.Arrays;
import java.util.Comparator;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.concurrent.atomic.AtomicInteger;

import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyAttribute;
import com.sun.star.beans.PropertyChangeEvent;
import com.sun.star.beans.PropertyVetoException;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XFastPropertySet;
import com.sun.star.beans.XMultiPropertySet;
import com.sun.star.beans.XPropertiesChangeListener;
import com.sun.star.beans.XPropertyChangeListener;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.beans.XVetoableChangeListener;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lib.uno.helper.InterfaceContainer;
import com.sun.star.lib.uno.helper.MultiTypeInterfaceContainer;
import com.sun.star.uno.Any;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.TypeClass;
import com.sun.star.uno.XInterface;

public class PropertySetAdapter implements XPropertySet, XFastPropertySet, XMultiPropertySet {
    private final Object lock;
    private final Object eventSource;
    // after registerListeners(), these are read-only:
    private final Map<String,PropertyData> propertiesByName = new HashMap<String,PropertyData>();
    private final Map<Integer,PropertyData> propertiesByHandle = new HashMap<Integer,PropertyData>();
    private AtomicInteger nextHandle = new AtomicInteger(1);
    // interface containers are locked internally:
    protected final MultiTypeInterfaceContainer boundListeners = new MultiTypeInterfaceContainer();
    protected final MultiTypeInterfaceContainer vetoableListeners = new MultiTypeInterfaceContainer();
    protected final InterfaceContainer propertiesChangeListeners = new InterfaceContainer();
    private final PropertySetInfo propertySetInfo = new PropertySetInfo();

    public static interface PropertyGetter {
        Object getValue();
    }

    public static interface PropertySetter {
        void setValue(Object value);
    }

    private static class PropertyData {
        Property property;
        PropertyGetter getter;
        PropertySetter setter;

        PropertyData(Property property, PropertyGetter getter, PropertySetter setter) {
            this.property = property;
            this.getter = getter;
            this.setter = setter;
        }
    }

    private static final Comparator<Property> propertyNameComparator = new Comparator<Property>() {
        @Override
        public int compare(Property first, Property second) {
            return first.Name.compareTo(second.Name);
        }
    };

    private class PropertySetInfo implements XPropertySetInfo {
        @Override
        public Property[] getProperties() {
            Property[] properties = new Property[propertiesByName.size()];
            int next = 0;
            for (Map.Entry<String,PropertyData> entry : propertiesByName.entrySet()) {
                properties[next++] = entry.getValue().property;
            }
            Arrays.sort(properties, propertyNameComparator);
            return properties;
        }

        @Override
        public Property getPropertyByName(String propertyName) throws UnknownPropertyException {
            PropertyData propertyData = getPropertyData(propertyName);
            return propertyData.property;
        }

        @Override
        public boolean hasPropertyByName(String propertyName) {
            return propertiesByName.containsKey(propertyName);
        }
    }

    /**
     * Creates a new instance.
     * @param lock the lock that will be held while calling the getters and setters
     * @param eventSource the com.sun.star.lang.EventObject Source field, to use in events sent to listeners
     */
    public PropertySetAdapter(Object lock, Object eventSource) {
        this.lock = lock;
        this.eventSource = eventSource;
    }

    public void dispose() {
        // Create an event with this as sender
        EventObject event = new EventObject(eventSource);

        // inform all listeners to release this object
        boundListeners.disposeAndClear(event);
        vetoableListeners.disposeAndClear(event);
    }

    public void registerProperty(String propertyName, int handle, Type type, short attributes,
            PropertyGetter getter, PropertySetter setter) {
        Property property = new Property(propertyName, handle, type, attributes);
        PropertyData propertyData = new PropertyData(property, getter, setter);
        propertiesByName.put(propertyName, propertyData);
        propertiesByHandle.put(property.Handle, propertyData);
    }

    public void registerProperty(String propertyName, Type type, short attributes,
            PropertyGetter getter, PropertySetter setter) {
        int handle;
        // registerProperty() should only be called from one thread, but just in case:
        handle = nextHandle.getAndIncrement();
        registerProperty(propertyName, handle, type, attributes, getter, setter);
    }

    @Override
    public void addPropertyChangeListener(
            String propertyName, XPropertyChangeListener listener) throws UnknownPropertyException, WrappedTargetException {
        PropertyData propertyData = getPropertyData(propertyName);
        if ((propertyData.property.Attributes & PropertyAttribute.BOUND) != 0) {
            boundListeners.addInterface(propertyName, listener);
        } // else ignore silently
    }

    @Override
    public void addVetoableChangeListener(
            String propertyName, XVetoableChangeListener listener) throws UnknownPropertyException, WrappedTargetException {
        PropertyData propertyData = getPropertyData(propertyName);
        if ((propertyData.property.Attributes & PropertyAttribute.CONSTRAINED) != 0) {
            vetoableListeners.addInterface(propertyName, listener);
        } // else ignore silently
    }

    @Override
    public void addPropertiesChangeListener(String[] propertyNames, XPropertiesChangeListener listener) {
        propertiesChangeListeners.add(listener);
    }

    @Override
    public XPropertySetInfo getPropertySetInfo() {
        return propertySetInfo;
    }

    private PropertyData getPropertyData(String propertyName) throws UnknownPropertyException {
        PropertyData propertyData = propertiesByName.get(propertyName);
        if (propertyData == null) {
            throw new UnknownPropertyException(propertyName);
        }
        return propertyData;
    }

    private PropertyData getPropertyData(int handle) throws UnknownPropertyException {
        PropertyData propertyData = propertiesByHandle.get(handle);
        if (propertyData == null) {
            throw new UnknownPropertyException(Integer.toString(handle));
        }
        return propertyData;
    }

    private Object getPropertyValue(PropertyData propertyData) {
        Object ret;
        synchronized (lock) {
            ret = propertyData.getter.getValue();
        }

        // null must not be returned. Either a void any is returned or an any containing
        // an interface type and a null reference.
        if (ret == null) {
            if (propertyData.property.Type.getTypeClass() == TypeClass.INTERFACE) {
                ret = new Any(propertyData.property.Type, null);
            } else {
                ret = new Any(new Type(void.class), null);
            }
        }
        return ret;
    }

    @Override
    public Object getPropertyValue(String propertyName) throws UnknownPropertyException, WrappedTargetException {
        PropertyData propertyData = getPropertyData(propertyName);
        return getPropertyValue(propertyData);
    }

    @Override
    public Object getFastPropertyValue(int handle) throws UnknownPropertyException, WrappedTargetException {
        PropertyData propertyData = getPropertyData(handle);
        return getPropertyValue(propertyData);
    }

    @Override
    public Object[] getPropertyValues(String[] propertyNames) {
        Object[] values = new Object[propertyNames.length];
        for (int i = 0; i < propertyNames.length; i++) {
            Object value = null;
            try {
                value = getPropertyValue(propertyNames[i]);
            } catch (UnknownPropertyException unknownPropertyException) {
            } catch (WrappedTargetException wrappedTargetException) {
            }
            values[i] = value;
        }
        return values;
    }

    @Override
    public void removePropertyChangeListener(
            String propertyName, XPropertyChangeListener listener) throws UnknownPropertyException, WrappedTargetException {
        // check existence:
        getPropertyData(propertyName);
        boundListeners.removeInterface(propertyName, listener);
    }

    @Override
    public synchronized void removeVetoableChangeListener(
            String propertyName, XVetoableChangeListener listener) throws UnknownPropertyException, WrappedTargetException {
        // check existence:
        getPropertyData(propertyName);
        vetoableListeners.removeInterface(propertyName, listener);
    }

    @Override
    public void removePropertiesChangeListener(XPropertiesChangeListener listener) {
        propertiesChangeListeners.remove(listener);
    }

    @Override
    public void setPropertyValue(String propertyName, Object value)
            throws UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException {
        PropertyData propertyData = getPropertyData(propertyName);
        setPropertyValue(propertyData, value);
    }

    @Override
    public void setFastPropertyValue(int handle, Object value)
            throws UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException {
        PropertyData propertyData = getPropertyData(handle);
        setPropertyValue(propertyData, value);
    }

    private void setPropertyValue(PropertyData propertyData, Object value)
            throws UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException {
        if ((propertyData.property.Attributes & PropertyAttribute.READONLY) != 0) {
            throw new PropertyVetoException();
        }
        // The value may be null only if MAYBEVOID attribute is set
        boolean isVoid = false;
        if (value instanceof Any) {
            isVoid = ((Any) value).getObject() == null;
        } else {
            isVoid = value == null;
        }
        if (isVoid && (propertyData.property.Attributes & PropertyAttribute.MAYBEVOID) == 0) {
            throw new IllegalArgumentException("The property must have a value; the MAYBEVOID attribute is not set!");
        }

        // Check if the argument is allowed
        boolean isValueOk = false;
        if (value instanceof Any) {
            isValueOk = checkType(((Any) value).getObject());
        } else {
            isValueOk = checkType(value);
        }
        if (!isValueOk) {
            throw new IllegalArgumentException("No valid UNO type");
        }

        Object[] futureValue = new Object[] { AnyConverter.toObject(propertyData.property.Type, value) };
        Object[] currentValue = new Object[] { getPropertyValue(propertyData.property.Name) };
        Property[] properties = new Property[] { propertyData.property };

        fire(properties, currentValue, futureValue, false);
        synchronized (lock) {
            propertyData.setter.setValue(futureValue[0]);
        }
        fire(properties, currentValue, futureValue, true);
    }

    @Override
    public void setPropertyValues(String[] propertyNames, Object[] values) throws PropertyVetoException, IllegalArgumentException, WrappedTargetException {
        for (int i = 0; i < propertyNames.length; i++) {
            try {
                setPropertyValue(propertyNames[i], values[i]);
            } catch (UnknownPropertyException e) {
                continue;
            }
        }
    }

    private boolean checkType(Object obj) {
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

    @Override
    public void firePropertiesChangeEvent(String[] propertyNames, XPropertiesChangeListener listener) {
        PropertyChangeEvent[] events = new PropertyChangeEvent[propertyNames.length];
        int eventCount = 0;
        for (int i = 0; i < propertyNames.length; i++) {
            try {
                PropertyData propertyData = getPropertyData(propertyNames[i]);
                Object value = getPropertyValue(propertyNames[i]);
                events[eventCount++] = new PropertyChangeEvent(eventSource, propertyNames[i],
                        false, propertyData.property.Handle, value, value);
            } catch (UnknownPropertyException unknownPropertyException) {
            } catch (WrappedTargetException wrappedTargetException) {
            }
        }
        if (eventCount > 0) {
            if (events.length != eventCount) {
                PropertyChangeEvent[] tmp = new PropertyChangeEvent[eventCount];
                System.arraycopy(events, 0, tmp, 0, eventCount);
                events = tmp;
            }
            listener.propertiesChange(events);
        }
    }

    private void fire(Property[] properties, Object[] oldValues, Object[] newValues, boolean hasChanged) throws PropertyVetoException {
        PropertyChangeEvent[] events = new PropertyChangeEvent[properties.length];
        int eventCount = 0;
        for (int i = 0; i < properties.length; i++) {
            if ((!hasChanged && (properties[i].Attributes & PropertyAttribute.CONSTRAINED) != 0) ||
                    (hasChanged && (properties[i].Attributes & PropertyAttribute.BOUND) != 0)) {
                events[eventCount++] = new PropertyChangeEvent(
                        eventSource, properties[i].Name, false, properties[i].Handle, oldValues[i], newValues[i]);
            }
        }
        for (int i = 0; i < eventCount; i++) {
            fireListeners(hasChanged, events[i].PropertyName, events[i]);
            fireListeners(hasChanged, "", events[i]);
        }
        if (hasChanged && eventCount > 0) {
            if (eventCount != events.length) {
                PropertyChangeEvent[] tmp = new PropertyChangeEvent[eventCount];
                System.arraycopy(events, 0, tmp, 0, eventCount);
                events = tmp;
            }
            for (Iterator<?> it = propertiesChangeListeners.iterator(); it.hasNext();) {
                XPropertiesChangeListener listener = (XPropertiesChangeListener) it.next();
                listener.propertiesChange(events);
            }
        }
    }

    private void fireListeners(boolean hasChanged, String key, PropertyChangeEvent event) throws PropertyVetoException {
        InterfaceContainer listeners;
        if (hasChanged) {
            listeners = boundListeners.getContainer(key);
        } else {
            listeners = vetoableListeners.getContainer(key);
        }
        if (listeners != null) {
            Iterator<?> it = listeners.iterator();
            while (it.hasNext()) {
                Object listener = it.next();
                if (hasChanged) {
                    ((XPropertyChangeListener)listener).propertyChange(event);
                } else {
                    ((XVetoableChangeListener)listener).vetoableChange(event);
                }
            }
        }
    }
}
