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
            throw new StatusException(Status.failed("the property 'PrinterIndependentLayout' is unknown."));
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
