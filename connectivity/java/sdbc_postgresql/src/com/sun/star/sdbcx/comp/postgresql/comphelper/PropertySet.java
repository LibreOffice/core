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

import com.sun.star.beans.PropertyVetoException;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XFastPropertySet;
import com.sun.star.beans.XMultiPropertySet;
import com.sun.star.beans.XPropertiesChangeListener;
import com.sun.star.beans.XPropertyChangeListener;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.beans.XVetoableChangeListener;
import com.sun.star.lang.DisposedException;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lib.uno.helper.ComponentBase;
import com.sun.star.sdbcx.comp.postgresql.comphelper.PropertySetAdapter.PropertyGetter;
import com.sun.star.sdbcx.comp.postgresql.comphelper.PropertySetAdapter.PropertySetter;
import com.sun.star.uno.Type;

public class PropertySet extends ComponentBase implements XPropertySet, XFastPropertySet, XMultiPropertySet {
    private final PropertySetAdapter impl;

    protected PropertySet(Object lock) {
        impl = new PropertySetAdapter(lock, this);
    }

    @Override
    protected void postDisposing() {
        impl.dispose();
    }

    public void registerProperty(String propertyName, int handle, Type type, short attributes, PropertyGetter getter, PropertySetter setter) {
        impl.registerProperty(propertyName, handle, type, attributes, getter, setter);
    }

    public void registerProperty(String propertyName, Type type, short attributes, PropertyGetter getter, PropertySetter setter) {
        impl.registerProperty(propertyName, type, attributes, getter, setter);
    }

    public void addPropertyChangeListener(String propertyName, XPropertyChangeListener listener) throws UnknownPropertyException, WrappedTargetException {
        // only add listeners if you are not disposed
        if (!bInDispose && !bDisposed) {
            impl.addPropertyChangeListener(propertyName, listener);
        }
    }

    public void addVetoableChangeListener(String propertyName, XVetoableChangeListener listener) throws UnknownPropertyException, WrappedTargetException {
        // only add listeners if you are not disposed
        if (!bInDispose && !bDisposed) {
            impl.addVetoableChangeListener(propertyName, listener);
        }
    }

    public void addPropertiesChangeListener(String[] propertyNames, XPropertiesChangeListener listener) {
        // only add listeners if you are not disposed
        if (!bInDispose && !bDisposed) {
            impl.addPropertiesChangeListener(propertyNames, listener);
        }
    }

    public XPropertySetInfo getPropertySetInfo() {
        return impl.getPropertySetInfo();
    }

    public Object getPropertyValue(String propertyName) throws UnknownPropertyException, WrappedTargetException {
        if (bInDispose || bDisposed) {
            throw new DisposedException("Component is already disposed");
        }
        return impl.getPropertyValue(propertyName);
    }

    @Override
    public Object getFastPropertyValue(int handle) throws UnknownPropertyException, WrappedTargetException {
        if (bInDispose || bDisposed) {
            throw new DisposedException("Component is already disposed");
        }
        return impl.getFastPropertyValue(handle);
    }

    public Object[] getPropertyValues(String[] propertyNames) {
        if (bInDispose || bDisposed) {
            throw new DisposedException("Component is already disposed");
        }
        return impl.getPropertyValues(propertyNames);
    }

    public void removePropertyChangeListener(String propertyName, XPropertyChangeListener listener) throws UnknownPropertyException, WrappedTargetException {
        // all listeners are automatically released in a dispose call
        if (!bInDispose && !bDisposed) {
            impl.removePropertyChangeListener(propertyName, listener);
        }
    }

    public void removeVetoableChangeListener(String propertyName, XVetoableChangeListener listener) throws UnknownPropertyException, WrappedTargetException {
        // all listeners are automatically released in a dispose call
        if (!bInDispose && !bDisposed) {
            impl.removeVetoableChangeListener(propertyName, listener);
        }
    }

    public void removePropertiesChangeListener(XPropertiesChangeListener listener) {
        // all listeners are automatically released in a dispose call
        if (!bInDispose && !bDisposed) {
            impl.removePropertiesChangeListener(listener);
        }
    }

    public void setPropertyValue(String propertyName, Object value)
            throws UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException {
        if (bInDispose || bDisposed) {
            throw new DisposedException("Component is already disposed");
        }
        impl.setPropertyValue(propertyName, value);
    }

    public void setFastPropertyValue(int handle, Object value)
            throws UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException {
        if (bInDispose || bDisposed) {
            throw new DisposedException("Component is already disposed");
        }
        impl.setFastPropertyValue(handle, value);
    }

    public void setPropertyValues(String[] propertyNames, Object[] values) throws PropertyVetoException, IllegalArgumentException, WrappedTargetException {
        if (bInDispose || bDisposed) {
            throw new DisposedException("Component is already disposed");
        }
        impl.setPropertyValues(propertyNames, values);
    }

    public void firePropertiesChangeEvent(String[] propertyNames, XPropertiesChangeListener listener) {
        if (bInDispose || bDisposed) {
            throw new DisposedException("Component is already disposed");
        }
        impl.firePropertiesChangeEvent(propertyNames, listener);
    }


}
