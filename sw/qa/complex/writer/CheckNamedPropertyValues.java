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

package complex.writer;


import complexlib.ComplexTestCase;
import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;
import com.sun.star.container.XNameContainer;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.Type;

/**
 *
 */
public class CheckNamedPropertyValues extends ComplexTestCase {

    private final String testedServiceName =
                    "com.sun.star.document.NamedPropertyValues";

    public String[] getTestMethodNames() {
        return new String[]{"checkNamedPropertyValues"};
    }

/*    public String getTestObjectName() {
        return "complex.writer.CheckNamedPropertyValues";
    } */

    public void checkNamedPropertyValues() {
        Object oObj = null;
        try {
            XMultiServiceFactory xMSF = (XMultiServiceFactory)param.getMSF();
            oObj = xMSF.createInstance(testedServiceName);
            System.out.println("****************");
            System.out.println("Service Name:");
            util.dbg.getSuppServices(oObj);
            System.out.println("****************");
            System.out.println("Interfaces:");
            util.dbg.printInterfaces((XInterface)oObj, true);
        }
        catch(com.sun.star.uno.Exception e) {
            e.printStackTrace();
            failed(e.getMessage());
            return;
        }
        XNameContainer xCont = (XNameContainer)UnoRuntime.queryInterface(
                                                XNameContainer.class, oObj);

        assure("XNameContainer was queried but returned null.",
                                                        (xCont != null));
        PropertyValue[] prop1 = new PropertyValue[1];
        prop1[0] = new PropertyValue();
        prop1[0].Name  = "Jupp";
        prop1[0].Value = "GoodGuy";

        PropertyValue[] prop2 = new PropertyValue[1];
        prop2[0] = new PropertyValue();
        prop2[0].Name  = "Horst";
        prop2[0].Value = "BadGuy";

        try {
            Type t = xCont.getElementType();
            log.println("Insertable Type: " + t.getTypeName());
            assure("Initial container is not empty.", !xCont.hasElements());

            log.println("Insert a PropertyValue.");
            xCont.insertByName("prop1", prop1);
            PropertyValue[]ret = (PropertyValue[])xCont.getByName("prop1");
            assure("Got the wrong PropertyValue: " +
                                    ret[0].Name +  "  " +(String)ret[0].Value,
                                    ret[0].Name.equals(prop1[0].Name) &&
                                    ret[0].Value.equals(prop1[0].Value));
            log.println("Replace the PropertyValue.");
            xCont.replaceByName("prop1", prop2);
            ret = (PropertyValue[])xCont.getByName("prop1");
            assure("Got the wrong PropertyValue: " +
                                    ret[0].Name +  "  " +(String)ret[0].Value,
                                    ret[0].Name.equals(prop2[0].Name) &&
                                    ret[0].Value.equals(prop2[0].Value));
            log.println("Remove the PropertyValue.");
            xCont.removeByName("prop1");
            assure("Could not remove PropertyValue.", !xCont.hasElements());
            log.println("Insert again.");
            xCont.insertByName("prop1", prop1);
            xCont.insertByName("prop2", prop2);
            assure("Did not insert PropertyValue.", xCont.hasElements());
            String[] names = xCont.getElementNames();
            int count = 0;
            for (int i=0; i<names.length; i++) {
                if (names[i].equals("prop1") || names[i].equals("prop2"))
                    count++;
                else
                    failed("Got a wrong element name: "+names[i]);
            }
            if (count != 2)
                failed("Not all element names were returned.");

            try {
                log.println("Insert PropertyValue with an existing name.");
                xCont.insertByName("prop2", prop1);
                failed("ElementExistException was not thrown.");
            }
            catch(com.sun.star.lang.IllegalArgumentException e) {
                log.println("Wrong exception thrown.");
                failed(e.getMessage());
                e.printStackTrace();
            }
            catch(com.sun.star.container.ElementExistException e) {
                log.println("Expected exception thrown: "+e);
            }
            catch(com.sun.star.lang.WrappedTargetException e) {
                log.println("Wrong exception thrown.");
                failed(e.getMessage());
                e.printStackTrace();
            }

            try {
                log.println("Inserting a wrong argument.");
                xCont.insertByName("prop3", "Example String");
                failed("IllegalArgumentException was not thrown.");
            }
            catch(com.sun.star.lang.IllegalArgumentException e) {
                log.println("Expected exception thrown: "+e);
            }
            catch(com.sun.star.container.ElementExistException e) {
                log.println("Wrong exception thrown.");
                failed(e.getMessage());
                e.printStackTrace();
            }
            catch(com.sun.star.lang.WrappedTargetException e) {
                log.println("Wrong exception thrown.");
                failed(e.getMessage());
                e.printStackTrace();
            }

            try {
                log.println("Remove a non-existing element.");
                xCont.removeByName("prop3");
                failed("NoSuchElementException was not thrown.");
            }
            catch(com.sun.star.container.NoSuchElementException e) {
                log.println("Expected exception thrown: "+e);
            }
            catch(com.sun.star.lang.WrappedTargetException e) {
                log.println("Wrong exception thrown.");
                failed(e.getMessage());
                e.printStackTrace();
            }

        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            failed(e.getMessage());
            e.printStackTrace();
        }
        catch(com.sun.star.container.ElementExistException e) {
            failed(e.getMessage());
            e.printStackTrace();
        }
        catch(com.sun.star.container.NoSuchElementException e) {
            failed(e.getMessage());
            e.printStackTrace();
        }
        catch(com.sun.star.lang.WrappedTargetException e) {
            failed(e.getMessage());
            e.printStackTrace();
        }
    }


}
