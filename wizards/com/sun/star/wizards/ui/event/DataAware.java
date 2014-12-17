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

import java.util.Arrays;

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
    private Object dataObject;
    /**
     * A Value Object knows how to get/set a value
     * from/to the data object.
     */
    private Value value;

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
     */
    private Object getDataObject() {
        return dataObject;
    }

    /**
     * Sets the given value to the data object.
     * this method delegates the job to the
     * Value object, but can be overwritten if
     * another kind of Data is needed.
     * @param newValue the new value to set to the DataObject.
     */
    private void setToData(Object newValue) {
        value.set(newValue,getDataObject());
    }

    /**
     * gets the current value from the data object.
     * this method delegates the job to
     * the value object.
     * @return the current value of the data object.
     */
    private Object getFromData() {
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
     * enables
     * @param currentValue
     */
    private void enableControls(Object currentValue) {
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

    /**
     * compares the two given objects.
     * This method is null safe and returns true also if both are null...
     * If both are arrays, treats them as array of short and compares them.
     * @param a first object to compare
     * @param b second object to compare.
     * @return true if both are null or both are equal.
     */
    private boolean equals(Object a, Object b) {
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
     * Value objects read and write a value from and
     * to an object. Typically using reflection and JavaBeans properties
     * or directly using member reflection API.
     * DataAware delegates the handling of the DataObject
     * to a Value object.
     * 2 implementations currently exist: PropertyValue,
     * using JavaBeans properties reflection, and DataAwareFields classes
     * which implement different member types.
     */
    interface Value {
        /**
         * gets a value from the given object.
         * @param target the object to get the value from.
         * @return the value from the given object.
         */
        Object get(Object target);
        /**
         * sets a value to the given object.
         * @param value the value to set to the object.
         * @param target the object to set the value to.
         */
        void set(Object value, Object target);
    }

}
