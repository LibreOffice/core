/*************************************************************************
 *
 *  $RCSfile: _Settings.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2004-12-10 17:02:36 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
package ifc.document;

import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyAttribute;
import com.sun.star.i18n.ForbiddenCharacters;
import com.sun.star.i18n.XForbiddenCharacters;
import com.sun.star.i18n.XLocaleData;
import com.sun.star.lang.Locale;
import com.sun.star.lang.XMultiServiceFactory;
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