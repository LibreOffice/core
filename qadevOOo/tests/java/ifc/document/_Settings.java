/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: _Settings.java,v $
 * $Revision: 1.6 $
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
package ifc.document;

import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyAttribute;
import com.sun.star.i18n.XForbiddenCharacters;
import com.sun.star.uno.UnoRuntime;
import java.lang.reflect.Method;

//import java.awt.print.PrinterJob;

//import javax.print.PrintService;

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
            Short newVal = oldVal.intValue() == 1 ?  new Short("3") : new Short("1");


            testProperty("PrinterIndependentLayout", oldVal, newVal);

        } catch (com.sun.star.beans.UnknownPropertyException e) {
            throw new StatusException(Status.failed("the property 'PrinterIndependentLayout' is unkown."));
        } catch (com.sun.star.lang.WrappedTargetException e) {
            throw new StatusException(Status.failed("the property 'PrinterIndependentLayout' could not be tested."));
        }
    }

    public void _PrinterName() {
        Object[] oServices = null;
        Exception ex = null;

        try {
            Class cPrinterJob = Class.forName("java.awt.print.PrinterJob");
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
//        PrintService[] services = PrinterJob.lookupPrintServices();

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
            XForbiddenCharacters fc = (XForbiddenCharacters) UnoRuntime.queryInterface(
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
            Class cPrintService = Class.forName("javax.print.PrintService");
            Method getNameMethod = cPrintService.getDeclaredMethod("getName", new Class[0]);
            Object retValue = getNameMethod.invoke(pService, new Object[0]);
            pName = (String)retValue;
        }
        // ignore all excptions: we already ran into one of these if Java is too old
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
