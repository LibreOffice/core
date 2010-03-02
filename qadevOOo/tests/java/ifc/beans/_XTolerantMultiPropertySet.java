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
package ifc.beans;

import com.sun.star.beans.GetDirectPropertyTolerantResult;
import com.sun.star.beans.GetPropertyTolerantResult;
import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyAttribute;
import com.sun.star.beans.PropertyState;
import com.sun.star.beans.SetPropertyTolerantFailed;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertyState;
import com.sun.star.beans.XTolerantMultiPropertySet;
import com.sun.star.uno.UnoRuntime;

import java.util.ArrayList;
import java.util.Collections;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import util.ValueChanger;
import util.ValueComparer;


public class _XTolerantMultiPropertySet extends MultiMethodTest {
    public XTolerantMultiPropertySet oObj;
    protected String[] namesOfDirectProperties = null;
    protected String[] namesOfProperties = null;
    protected Object[] valuesOfProperties = null;
    protected Property[] properties = null;
    protected XPropertyState pState = null;
    protected XPropertySet PS = null;


    /*
     * Queries XPropertySet from the given Component and gets XPropertySetInfo
     * from it to get the PropertyNames available and their Values<br>
     * Then queries XPropertyState from the given Component
     * to get the direct properties<br>
     * Throws a lib StatusException if the Component doesn't support XPropertySet or XPropertyState
     */
    public void before() {
        PS = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class,
                                                      tEnv.getTestObject());

        if (PS == null) {
            throw new StatusException(Status.failed(
                                              "Component doesn't provide the needed XPropertySet"));
        }

        pState = (XPropertyState) UnoRuntime.queryInterface(
                         XPropertyState.class, tEnv.getTestObject());

        if (pState == null) {
            throw new StatusException(Status.failed(
                                              "Component doesn't provide the needed XPropertyState"));
        }

        properties = PS.getPropertySetInfo().getProperties();
        namesOfProperties = getProperties();
        valuesOfProperties = getPropertyValues(namesOfProperties);
    }

    /*
     * Calls the method getDirectPropertyValuesTolerant() and compares the resulting
     * sequence with the one gained as direct values in the before() method.<br>
     * Has OK state if both sequences equal.
     */
    public void _getDirectPropertyValuesTolerant() {
        namesOfDirectProperties = getDirectProperties(properties);

        GetDirectPropertyTolerantResult[] GDPR = oObj.getDirectPropertyValuesTolerant(
                                                         namesOfProperties);

        boolean res = (GDPR.length == namesOfDirectProperties.length);

        if (!res) {
            log.println("Found: ");

            for (int i = 0; i < GDPR.length; i++) {
                log.println("\t" + GDPR[i].Name);
            }

            log.println("Expected: ");

            for (int i = 0; i < namesOfDirectProperties.length; i++) {
                log.println("\t" + namesOfDirectProperties[i]);
            }
        } else {
            for (int i = 0; i < GDPR.length; i++) {
                boolean localres = GDPR[i].Name.equals(
                                           namesOfDirectProperties[i]);

                if (!localres) {
                    log.println("Found: ");
                    log.println("\t" + GDPR[i].Name);
                    log.println("Expected: ");
                    log.println("\t" + namesOfDirectProperties[i]);
                }

                res &= localres;
            }
        }

        tRes.tested("getDirectPropertyValuesTolerant()", res);
    }

    public void _getPropertyValuesTolerant() {
        requiredMethod("getDirectPropertyValuesTolerant()");
        GetPropertyTolerantResult[] GPR = oObj.getPropertyValuesTolerant(
                                                  namesOfProperties);

        boolean res = (GPR.length == namesOfProperties.length);

        if (!res) {
            log.println("Length of sequences differs");
            log.println("Found: " + GPR.length);
            log.println("Expected: " + namesOfProperties.length);
        } else {
            for (int i = 0; i < GPR.length; i++) {
                boolean localres = true;

                if (!(GPR[i].Value instanceof com.sun.star.uno.Any)) {
                    localres = ValueComparer.equalValue(GPR[i].Value,
                                                        valuesOfProperties[i]);

                }

                if (!localres) {
                    log.println("Values differ for : " +
                                namesOfProperties[i]);
                    log.println("\t" + GPR[i].Value);
                    log.println("Expected: ");
                    log.println("\t" + valuesOfProperties[i]);
                }

                res &= localres;
            }
        }

        tRes.tested("getPropertyValuesTolerant()", res);
    }

    public void _setPropertyValuesTolerant() {
        requiredMethod("getPropertyValuesTolerant()");

        SetPropertyTolerantFailed[] SPTF = null;

        try {
            SPTF = oObj.setPropertyValuesTolerant(namesOfProperties,
                                                  getNewValues(
                                                          valuesOfProperties));
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
        }

        //read only properties will throw a PropertyVetoExeption if they are set
        int failures = 0;

        for (int k = 0; k < SPTF.length; k++) {
            if (SPTF[k].Result == com.sun.star.beans.TolerantPropertySetResultType.PROPERTY_VETO) {
                failures++;
            }
        }

        int roProps = getCountOfReadOnlyProperties();

        boolean res = (failures == roProps);

        if (!res) {
            log.println("Failures: " + failures);
            log.println("Count of R/O properties: " + roProps);

            for (int i = 0; i < SPTF.length; i++) {
                if (SPTF[i].Result == com.sun.star.beans.TolerantPropertySetResultType.PROPERTY_VETO) {
                    failures++;
                    log.println("Failed for " + SPTF[i].Name);
                    log.println("\t Result: " + SPTF[i].Result);
                }
            }
        } else {
            for (int i = 0; i < SPTF.length; i++) {
                boolean localres = true;
                GetPropertyTolerantResult[] GPR = oObj.getPropertyValuesTolerant(
                                                          namesOfProperties);

                if ((!(GPR[i].Value instanceof com.sun.star.uno.Any)) &&
                        (SPTF[i].Result == com.sun.star.beans.TolerantPropertySetResultType.SUCCESS)) {
                    localres = ValueComparer.equalValue(GPR[i].Value,
                                                        valuesOfProperties[i]);
                }

                if (!localres) {
                    log.println("Values differ for : " +
                                namesOfProperties[i]);
                    log.println("\t" + GPR[i].Value);
                    log.println("Expected: ");
                    log.println("\t" + valuesOfProperties[i]);
                }

                res &= localres;
            }
        }

        tRes.tested("setPropertyValuesTolerant()", res);
    }

    /*
     * This method returns a sorted list of property names
     * contained in a given sequence of properties that additionally
     * have the state DIRECT_VALUE
     */
    protected String[] getDirectProperties(Property[] props) {
        ArrayList direct = new ArrayList();

        for (int i = 0; i < props.length; i++) {
            String pName = props[i].Name;

            try {
                PropertyState state = pState.getPropertyState(pName);

                if (state.equals(PropertyState.DIRECT_VALUE)) {
                    if (isUsable(pName)) direct.add(pName);
                }
            } catch (com.sun.star.beans.UnknownPropertyException e) {
                log.println("Property '" + pName + "'");
            }
        }

        Collections.sort(direct);

        Object[] obj = direct.toArray();
        String[] ret = new String[obj.length];

        for (int i = 0; i < obj.length; i++) {
            ret[i] = (String) obj[i];
        }

        return ret;
    }

    private boolean isUsable(String name) {
        boolean isUsable=true;
        if (name.startsWith("TextWriting")) isUsable = false;
        if (name.startsWith("MetaFile")) isUsable = false;
        return isUsable;
    }

    /*
     * This method returns a sorted list of property names
     * contained in a given sequence of properties
     */
    protected String[] getProperties() {
        ArrayList names = new ArrayList();

        for (int i = 0; i < properties.length; i++) {
            String pName = properties[i].Name;
            if (isUsable(pName)) names.add(pName);
        }

        Collections.sort(names);

        Object[] obj = names.toArray();
        String[] ret = new String[obj.length];

        for (int i = 0; i < obj.length; i++) {
            ret[i] = (String) obj[i];
        }

        return ret;
    }

    /*
     * Returns the values of a given array of properties in an Object array
     */
    protected Object[] getPropertyValues(String[] propertyNames) {
        Object[] values = new Object[propertyNames.length];

        for (int i = 0; i < propertyNames.length; i++) {
            try {
                values[i] = PS.getPropertyValue(propertyNames[i]);
            } catch (com.sun.star.beans.UnknownPropertyException e) {
                e.printStackTrace(log);
            } catch (com.sun.star.lang.WrappedTargetException e) {
                e.printStackTrace(log);
            }
        }

        return values;
    }

    protected int getCountOfReadOnlyProperties() {
        int ro = 0;

        for (int i = 0; i < properties.length; i++) {
            Property property = properties[i];
            boolean isWritable = ((property.Attributes & PropertyAttribute.READONLY) == 0);

            if (!isWritable) {
                ro++;
            }
        }

        return ro;
    }

    protected Object[] getNewValues(Object[] oldValues) {
        Object[] newValues = new Object[oldValues.length];

        for (int i = 0; i < oldValues.length; i++) {
            if (oldValues[i] instanceof com.sun.star.uno.Any) {
                newValues[i] = oldValues[i];
            } else {
                newValues[i] = ValueChanger.changePValue(oldValues[i]);
            }
        }

        return newValues;
    }
}