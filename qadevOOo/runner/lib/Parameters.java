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



package lib;

import java.util.Iterator;
import java.util.Hashtable;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import com.sun.star.beans.Property;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.beans.XPropertyChangeListener;
import com.sun.star.beans.XVetoableChangeListener;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.uno.Type;

/**
 * Parameters is a container of String parameters.
 * @deprecated
 */

public class Parameters implements XPropertySet {
/*    final protected Map parameters;
    final Parameters defaults; */
    final protected Map parameters;
    final Parameters defaults;
    Property[] props;

    public Parameters(Map params) {
        this (params, null);
    }

    public Parameters(Map params, Parameters defaultParams) {
        parameters = params;
        defaults = defaultParams;
        checkParameters(parameters);

        Set paramSet = new HashSet(parameters.keySet());

        if (defaults != null) {
            Set defSet = defaults.toMap().keySet();
            paramSet.addAll(defSet);
        }

        props = new Property[paramSet.size()];

        int num = 0;

        for (Iterator i = paramSet.iterator(); i.hasNext(); num++) {
            String name = (String)i.next();

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
        for (int i=0; i<size; i++)
        {
            addProps[i] = props[i];
        }
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

    public Map toMap() {
        return new Hashtable(parameters) {
            public Object get(Object obj) {
                if (obj instanceof String) {
                    return Parameters.this.get((String) obj);
                } else {
                    return null;
                }
            }
        };
    }

    private static void checkParameters(Map params) {
        for (Iterator i = params.keySet().iterator(); i.hasNext();) {
            Object key = i.next();

            if (!(key instanceof String)) {
                throw new IllegalArgumentException(
                        "Wrong key " + key + ", it should be of String type");
            }

/*            Object value = params.get(key);

            if (!(value instanceof String)) {
                throw new IllegalArgumentException(
                        "Wrong value " + value + ", it should be of String type");
            } */
        }
    }

    public static String getString(XPropertySet props, String name) {
        try {
            return (String)props.getPropertyValue(name);
        } catch (UnknownPropertyException e) {
            return null;
        } catch (WrappedTargetException e) {
            return null;
        }
    }

    public static Object get(XPropertySet props, String name) {
        try {
            return props.getPropertyValue(name);
        } catch (UnknownPropertyException e) {
            return null;
        } catch (WrappedTargetException e) {
            return null;
        }
    }

    public static Map toMap(XPropertySet props) {
        Hashtable result = new Hashtable(10);

        XPropertySetInfo setInfo = props.getPropertySetInfo();
        Property[] properties = setInfo.getProperties();

        for (int i = 0; i < properties.length; i++) {
            String name = properties[i].Name;
            Object value;

            try {
                value = props.getPropertyValue(name);
            } catch (WrappedTargetException e) {
                continue;
            } catch (UnknownPropertyException e) {
                continue;
            }

            result.put(name, value);
        }

        return result;
    }
}
