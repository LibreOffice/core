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

package test.cppuhelper.propertysetmixin.comp;

import com.sun.star.beans.Ambiguous;
import com.sun.star.beans.Defaulted;
import com.sun.star.beans.Optional;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.PropertyVetoException;
import com.sun.star.beans.XFastPropertySet;
import com.sun.star.beans.XPropertyAccess;
import com.sun.star.beans.XPropertyChangeListener;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.beans.XVetoableChangeListener;
import com.sun.star.comp.loader.FactoryHelper;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XEventListener;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lib.uno.helper.WeakBase;
import com.sun.star.lib.uno.helper.PropertySetMixin;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.uno.Any;
import com.sun.star.uno.IQueryInterface;
import com.sun.star.uno.Type;
import com.sun.star.uno.XComponentContext;
import test.cppuhelper.propertysetmixin.XSupplier;
import test.cppuhelper.propertysetmixin.XTest3;

public final class JavaSupplier extends WeakBase implements XSupplier {
    public JavaSupplier(XComponentContext context) {
        this.context = context;
    }

    public XComponent getEmpty1() { return new Empty1(); }

    public XComponent getEmpty2() { return new Empty2(); }

    public XTest3 getFull() { return new Full(); }

    public static XSingleServiceFactory __getServiceFactory(
        String implName, XMultiServiceFactory multiFactory, XRegistryKey regKey)
    {
        return implName.equals(implementationName)
            ? FactoryHelper.getServiceFactory(
                JavaSupplier.class, serviceName, multiFactory, regKey)
            : null;
    }

    private static final String implementationName
    = JavaSupplier.class.getName();
    private static final String serviceName
    = "test.cppuhelper.propertysetmixin.JavaSupplier";

    private final class Empty1 extends WeakBase implements XComponent {
        public Empty1() {}

        public void dispose() {
            prop.dispose();
        }

        public void addEventListener(XEventListener listener) {}

        public void removeEventListener(XEventListener listener) {}

        private final PropertySetMixin prop = new PropertySetMixin(
            context, this, new Type(XComponent.class), null);
    }

    private final class Empty2 extends WeakBase
        implements XComponent, XPropertySet, XFastPropertySet, XPropertyAccess
    {
        public Empty2() {}

        public void dispose() {
            prop.dispose();
        }

        public void addEventListener(XEventListener listener) {}

        public void removeEventListener(XEventListener listener) {}

        public com.sun.star.beans.XPropertySetInfo getPropertySetInfo() {
            return prop.getPropertySetInfo();
        }

        public void setPropertyValue(String propertyName, Object value)
            throws UnknownPropertyException, PropertyVetoException,
            com.sun.star.lang.IllegalArgumentException, WrappedTargetException
        {
            prop.setPropertyValue(propertyName, value);
        }

        public Object getPropertyValue(String propertyName)
            throws UnknownPropertyException, WrappedTargetException
        {
            return prop.getPropertyValue(propertyName);
        }

        public void addPropertyChangeListener(
            String propertyName, XPropertyChangeListener listener)
            throws UnknownPropertyException, WrappedTargetException
        {
            prop.addPropertyChangeListener(propertyName, listener);
        }

        public void removePropertyChangeListener(
            String propertyName, XPropertyChangeListener listener)
            throws UnknownPropertyException, WrappedTargetException
        {
            prop.removePropertyChangeListener(propertyName, listener);
        }

        public void addVetoableChangeListener(
            String propertyName, XVetoableChangeListener listener)
            throws UnknownPropertyException, WrappedTargetException
        {
            prop.addVetoableChangeListener(propertyName, listener);
        }

        public void removeVetoableChangeListener(
            String propertyName, XVetoableChangeListener listener)
            throws UnknownPropertyException, WrappedTargetException
        {
            prop.removeVetoableChangeListener(propertyName, listener);
        }

        public void setFastPropertyValue(int handle, Object value)
            throws UnknownPropertyException, PropertyVetoException,
            com.sun.star.lang.IllegalArgumentException, WrappedTargetException
        {
            prop.setFastPropertyValue(handle, value);
        }

        public Object getFastPropertyValue(int handle)
            throws UnknownPropertyException, WrappedTargetException
        {
            return prop.getFastPropertyValue(handle);
        }

        public PropertyValue[] getPropertyValues() {
            return prop.getPropertyValues();
        }

        public void setPropertyValues(PropertyValue[] props)
            throws UnknownPropertyException, PropertyVetoException,
            com.sun.star.lang.IllegalArgumentException, WrappedTargetException
        {
            prop.setPropertyValues(props);
        }

        private final PropertySetMixin prop = new PropertySetMixin(
            context, this, new Type(XComponent.class), null);
    }

    private final class Full extends WeakBase
        implements XTest3, XPropertySet, XFastPropertySet, XPropertyAccess
    {
        public Full() {}

        public synchronized int getFirst() {
            return a1;
        }

        public void setFirst(int value) {
            prop.prepareSet("First", null);
            synchronized (this) {
                a1 = value;
            }
        }

        public synchronized Ambiguous getSecond()
            throws UnknownPropertyException
        {
            return a2;
        }

        public void setSecond(Ambiguous value)
            throws PropertyVetoException, UnknownPropertyException
        {
            PropertySetMixin.BoundListeners l
                = new PropertySetMixin.BoundListeners();
            prop.prepareSet(
                "Second", Any.VOID,
                (((Optional) ((Defaulted) value.Value).Value).IsPresent
                 ? ((Optional) ((Defaulted) value.Value).Value).Value
                 : Any.VOID),
                l);
            synchronized (this) {
                a2 = value;
            }
            l.notifyListeners();
        }

        public int getThird() throws UnknownPropertyException {
            throw new UnknownPropertyException("Third", this);
        }

        public void setThird(int value) throws UnknownPropertyException {
            throw new UnknownPropertyException("Third", this);
        }

        public int getFourth() throws UnknownPropertyException {
            throw new UnknownPropertyException("Fourth", this);
        }

        public void setFourth(int value) throws UnknownPropertyException {
            throw new UnknownPropertyException("Fourth", this);
        }

        public com.sun.star.beans.XPropertySetInfo getPropertySetInfo() {
            return prop.getPropertySetInfo();
        }

        public void setPropertyValue(String propertyName, Object value)
            throws UnknownPropertyException, PropertyVetoException,
            com.sun.star.lang.IllegalArgumentException, WrappedTargetException
        {
            prop.setPropertyValue(propertyName, value);
        }

        public Object getPropertyValue(String propertyName)
            throws UnknownPropertyException, WrappedTargetException
        {
            return prop.getPropertyValue(propertyName);
        }

        public void addPropertyChangeListener(
            String propertyName, XPropertyChangeListener listener)
            throws UnknownPropertyException, WrappedTargetException
        {
            prop.addPropertyChangeListener(propertyName, listener);
        }

        public void removePropertyChangeListener(
            String propertyName, XPropertyChangeListener listener)
            throws UnknownPropertyException, WrappedTargetException
        {
            prop.removePropertyChangeListener(propertyName, listener);
        }

        public void addVetoableChangeListener(
            String propertyName, XVetoableChangeListener listener)
            throws UnknownPropertyException, WrappedTargetException
        {
            prop.addVetoableChangeListener(propertyName, listener);
        }

        public void removeVetoableChangeListener(
            String propertyName, XVetoableChangeListener listener)
            throws UnknownPropertyException, WrappedTargetException
        {
            prop.removeVetoableChangeListener(propertyName, listener);
        }

        public void setFastPropertyValue(int handle, Object value)
            throws UnknownPropertyException, PropertyVetoException,
            com.sun.star.lang.IllegalArgumentException, WrappedTargetException
        {
            prop.setFastPropertyValue(handle, value);
        }

        public Object getFastPropertyValue(int handle)
            throws UnknownPropertyException, WrappedTargetException
        {
            return prop.getFastPropertyValue(handle);
        }

        public PropertyValue[] getPropertyValues() {
            return prop.getPropertyValues();
        }

        public void setPropertyValues(PropertyValue[] props)
            throws UnknownPropertyException, PropertyVetoException,
            com.sun.star.lang.IllegalArgumentException, WrappedTargetException
        {
            prop.setPropertyValues(props);
        }

        private final PropertySetMixin prop = new PropertySetMixin(
            context, this, new Type(XTest3.class), new String[] { "Third" });

        private int a1 = 0;
        private Ambiguous a2 = new Ambiguous(
            new Defaulted(new Optional(), true), false);
    }

    private final XComponentContext context;
}
