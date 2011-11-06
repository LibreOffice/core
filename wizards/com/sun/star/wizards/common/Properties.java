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
 * @author  rp
 */
public class Properties extends Hashtable
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

    public static PropertyValue[] getProperties(Map map)
    {
        PropertyValue[] pv = new PropertyValue[map.size()];

        Iterator it = map.keySet().iterator();
        for (int i = 0; i < pv.length; i++)
        {
            pv[i] = createProperty((String) it.next(), map);
        }
        return pv;
    }

    public static PropertyValue createProperty(String name, Map map)
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
