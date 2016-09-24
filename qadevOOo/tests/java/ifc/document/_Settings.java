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
package ifc.document;

import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyAttribute;
import com.sun.star.i18n.XForbiddenCharacters;
import com.sun.star.uno.UnoRuntime;
import java.lang.reflect.Method;

import lib.MultiPropertyTest;
import lib.Status;
import lib.StatusException;


/*
 * Generic test for all properties contained in this service
 */
public class _Settings extends MultiPropertyTest {

    /**
     * This property accepts only values in a range of 1-3
     * @see com.sun.star.document.PrinterIndependentLayout
     */
    public void _PrinterIndependentLayout() {
        try{
            Short oldVal = (Short) oObj.getPropertyValue("PrinterIndependentLayout");
            Short newVal = oldVal.intValue() == 1 ?  Short.valueOf("3") : Short.valueOf("1");


            testProperty("PrinterIndependentLayout", oldVal, newVal);

        } catch (com.sun.star.beans.UnknownPropertyException e) {
            throw new StatusException(e, Status.failed("the property 'PrinterIndependentLayout' is unknown."));
        } catch (com.sun.star.lang.WrappedTargetException e) {
            throw new StatusException(e, Status.failed("the property 'PrinterIndependentLayout' could not be tested."));
        }
    }

    public void _PrinterName() {
        Object[] oServices = null;
        Exception ex = null;

        try {
            Class<?> cPrinterJob = Class.forName("java.awt.print.PrinterJob");
            Method lookupMethod = cPrinterJob.getDeclaredMethod("lookupPrintServices", new Class[0]);
            Object retValue = lookupMethod.invoke(cPrinterJob, new Object[0]);
            oServices = (Object[])retValue;
        }
        catch(java.lang.ClassNotFoundException e) {
            ex = e;
        }
        catch(java.lang.NoSuchMethodException e) {
            ex = e;
        }
        catch(java.lang.IllegalAccessException e) {
            ex = e;
        }
        catch(java.lang.reflect.InvocationTargetException e) {
            ex = e;
        }

        if (ex != null) {
            // get Java version:
            String javaVersion = System.getProperty("java.version");
            throw new StatusException(Status.failed(
                "Cannot execute test with current Java version (Java 1.4 required) " +
                javaVersion + ": " + ex.getMessage()));
        }

        if (oServices.length > 1) {
            testProperty("PrinterName", getPrinterNameWithReflection(oServices[0]),
                            getPrinterNameWithReflection(oServices[1]));
        } else {
            log.println(
                    "checking this property needs at least two printers to be installed on your system");
            throw new StatusException(Status.failed(
                                              "only one printer installed so I can't change it"));
        }
    }

    public void _ForbiddenCharacters() {
        boolean res = true;

        try {
            //check if it is read only as specified
            res &= isReadOnly("ForbiddenCharacters");

            if (!isReadOnly("ForbiddenCharacters")) {
                log.println(
                        "The Property 'ForbiddenCharacters' isn't readOnly as specified");
            }

            //check if the property has the right type
            Object pValue = oObj.getPropertyValue("ForbiddenCharacters");
            XForbiddenCharacters fc = UnoRuntime.queryInterface(
                                              XForbiddenCharacters.class,
                                              pValue);
            res &= (fc != null);
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            log.println(
                    "Exception while checking property 'ForbiddenCharacters' " +
                    e.getMessage());
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println(
                    "Exception while checking property 'ForbiddenCharacters' " +
                    e.getMessage());
        }

        tRes.tested("ForbiddenCharacters", res);
    }

    protected boolean isReadOnly(String PropertyName) {
        boolean res = false;
        Property[] props = oObj.getPropertySetInfo().getProperties();

        for (int i = 0; i < props.length; i++) {
            if (props[i].Name.equals(PropertyName)) {
                res = ((props[i].Attributes & PropertyAttribute.READONLY) != 0);
            }
        }

        return res;
    }

    private String getPrinterNameWithReflection(Object pService) {
        String pName = null;
        try {
            Class<?> cPrintService = Class.forName("javax.print.PrintService");
            Method getNameMethod = cPrintService.getDeclaredMethod("getName", new Class[0]);
            Object retValue = getNameMethod.invoke(pService, new Object[0]);
            pName = (String)retValue;
        }
        // ignore all exceptions: we already ran into one of these if Java is too old
        catch(java.lang.ClassNotFoundException e) {
        }
        catch(java.lang.NoSuchMethodException e) {
        }
        catch(java.lang.IllegalAccessException e) {
        }
        catch(java.lang.reflect.InvocationTargetException e) {
        }
        return pName;
    }
}
