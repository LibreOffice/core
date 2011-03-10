/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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
