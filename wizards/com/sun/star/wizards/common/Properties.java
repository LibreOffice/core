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
/*
 * Properties.java
 *
 * Created on 1. Oktober 2003, 17:16
 */
package com.sun.star.wizards.common;

import com.sun.star.beans.PropertyValue;
import java.util.*;

/**
 * Simplifies handling Arrays of PropertyValue.
 * To make a use of this class, instantiate it, and call
 * the put(propName,propValue) method.
 * caution: propName should always be a String.
 * When finished, call the getProperties() method to get an array of the set properties.
 */
public class Properties extends HashMap<String,Object>
{

    public static Object getPropertyValue(PropertyValue[] props, String propName)
    {
        for (int i = 0; i < props.length; i++)
        {
            if (propName.equals(props[i].Name))
            {
                return props[i].Value;
            }
        }
        throw new IllegalArgumentException("Property '" + propName + "' not found.");
    }

    public static boolean hasPropertyValue(PropertyValue[] props, String propName)
    {
        for (int i = 0; i < props.length; i++)
        {
            if (propName.equals(props[i].Name))
            {
                return true;
            }
        }
        return false;
    }

    public PropertyValue[] getProperties()
    {
        return getProperties(this);
    }

    public static PropertyValue[] getProperties(Map<String,Object> map)
    {
        PropertyValue[] pv = new PropertyValue[map.size()];

        Iterator<String> it = map.keySet().iterator();
        for (int i = 0; i < pv.length; i++)
        {
            pv[i] = createProperty(it.next(), map);
        }
        return pv;
    }

    public static PropertyValue createProperty(String name, Map<String,Object> map)
    {
        return createProperty(name, map.get(name));
    }

    public static PropertyValue createProperty(String name, Object value)
    {
        PropertyValue pv = new PropertyValue();
        pv.Name = name;
        pv.Value = value;
        return pv;
    }

    public static PropertyValue createProperty(String name, Object value, int handle)
    {
        PropertyValue pv = createProperty(name, value);
        pv.Handle = handle;
        return pv;
    }

    public static PropertyValue[] convertToPropertyValueArray(Object[] _oObjectArray)
    {
        PropertyValue[] retproperties = null;
        if (_oObjectArray != null)
        {
            if (_oObjectArray.length > 0)
            {
                retproperties = new PropertyValue[_oObjectArray.length];
                for (int i = 0; i < _oObjectArray.length; i++)
                {
                    retproperties[i] = (PropertyValue) _oObjectArray[i];
                }
            }
        }
        return retproperties;
    }
}
