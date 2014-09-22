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

public class SimpleDataAware extends DataAware
{

    protected Object control;
    protected Object[] disableObjects = new Object[0];
    protected Value controlValue;

    public SimpleDataAware(Object dataObject, Value value, Object control_, Value controlValue_)
    {
        super(dataObject, value);
        control = control_;
        controlValue = controlValue_;
    }

    /*
    protected void enableControls(Object value) {
    Boolean b = getBoolean(value);
    for (int i = 0; i<disableObjects.length; i++)
    UIHelper.setEnabled(disableObjects[i],b);
    }
     */
    protected void setToUI(Object value)
    {
        controlValue.set(value, control);
    }

    /**
     * Try to get from an arbitrary object a boolean value.
     * Null returns Boolean.FALSE;
     * A Boolean object returns itself.
     * An Array returns true if it not empty.
     * An Empty String returns Boolean.FALSE.
     * everything else returns a Boolean.TRUE.
     * @param value
     * @return
     */
    /*protected Boolean getBoolean(Object value) {
    if (value==null)
    return Boolean.FALSE;
    if (value instanceof Boolean)
    return (Boolean)value;
    else if (value.getClass().isArray())
    return ((short[])value).length != 0 ? Boolean.TRUE : Boolean.FALSE;
    else if (value.equals(PropertyNames.EMPTY_STRING)) return Boolean.FALSE;
    else return Boolean.TRUE;
    }

    public void disableControls(Object[] controls) {
    disableObjects = controls;
    }
     */
    protected Object getFromUI()
    {
        return controlValue.get(control);
    }
}
