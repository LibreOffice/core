/*
 ************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Properties.java,v $
 *
 * $Revision: 1.4.192.1 $
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
