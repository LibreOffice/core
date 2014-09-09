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

package lib;

import java.util.Iterator;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import com.sun.star.beans.Property;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.beans.XPropertyChangeListener;
import com.sun.star.beans.XVetoableChangeListener;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.uno.Type;

/**
 * Parameters is a container of String parameters.
 */
public class Parameters implements XPropertySet {
    final private Map<String, Object> parameters;
    private final Parameters defaults;
    private Property[] props;

    public Parameters(Map<String, Object> params) {
        this (params, null);
    }

    private Parameters(Map<String, Object> params, Parameters defaultParams) {
        parameters = params;
        defaults = defaultParams;
        checkParameters(parameters);

        Set<String> paramSet = new HashSet<String>(parameters.keySet());

        if (defaults != null) {
            Set<String> defSet = defaults.toMap().keySet();
            paramSet.addAll(defSet);
        }

        props = new Property[paramSet.size()];

        int num = 0;

        for (Iterator<String> i = paramSet.iterator(); i.hasNext(); num++) {
            String name = i.next();

            props[num] = new Property(name, num, new Type(String.class), (short)0);
        }
    }


    public String get(String paramName) {
        Object res = parameters.get(paramName);

        if (res != null && res instanceof String)
            return (String)res;

        if (defaults != null)
            return defaults.get(paramName);

        return null;
    }

    public Object getPropertyValue(String name) {
        Object erg = parameters.get(name);
        if (erg ==  null && defaults != null)
            return defaults.getPropertyValue(name);
        return erg;
    }

    public void setPropertyValue(String name, Object value) {
        parameters.put(name, value);
        int size = props.length;
        Property[] addProps = new Property[size+1];
        System.arraycopy(props, 0, addProps, 0, size);
        addProps[size] = new Property(name, size, new Type(value.getClass()), (short)0);
        props = addProps;
    }

    public void addVetoableChangeListener(String name, XVetoableChangeListener l) {
    }

    public void removeVetoableChangeListener(String name, XVetoableChangeListener l) {
    }

    public void addPropertyChangeListener(String name, XPropertyChangeListener l) {
    }

    public void removePropertyChangeListener(String name, XPropertyChangeListener l) {
    }

    public XPropertySetInfo getPropertySetInfo() {
        return new XPropertySetInfo() {
            public Property[] getProperties() {
                return props;
            }

            public boolean hasPropertyByName(String name) {
                for (int i = 0; i < props.length; i++) {
                    Property prop = props[i];

                    if (prop.Name.equals(name)) {
                        return true;
                    }
                }

                return false;
            }

            public Property getPropertyByName(String name) throws UnknownPropertyException {
                for (int i = 0; i < props.length; i++) {
                    Property prop = props[i];

                    if (prop.Name.equals(name)) {
                        return prop;
                    }
                }

                throw new UnknownPropertyException(name);
            }
        };
    }

    private Map<String,Object> toMap() {
        return new HashMap<String,Object>(parameters) {
        };
    }

    private static void checkParameters(Map<String, Object> params) {
        for (Iterator<String> i = params.keySet().iterator(); i.hasNext();) {
            Object key = i.next();

            if (!(key instanceof String)) {
                throw new IllegalArgumentException(
                        "Wrong key " + key + ", it should be of String type");
            }
        }
    }






}
