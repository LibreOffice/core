/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CheckIndexedPropertyValues.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-27 09:07:25 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

package complex.writer;

import complexlib.ComplexTestCase;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;
import com.sun.star.beans.PropertyValue;
import com.sun.star.container.XIndexContainer;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.Type;

/**
 * Test the com.sun.star.document.IndexedPropertyValues service
 */
public class CheckIndexedPropertyValues extends ComplexTestCase {

    private final String testedServiceName =
                        "com.sun.star.document.IndexedPropertyValues";
    public String[] getTestMethodNames() {
        return new String[]{"checkIndexedPropertyValues"};
    }

/*    public String getTestObjectName() {
        return testedServiceName;
    }
*/
    public void checkIndexedPropertyValues() {
        Object oObj = null;
        try {
            // print information about the service
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
            System.out.println("Cannot create object.");
            e.printStackTrace();
            failed(e.getMessage());
            return;
        }
        XIndexContainer xCont = (XIndexContainer)UnoRuntime.queryInterface(
                                                XIndexContainer.class, oObj);

        assure("XIndexContainer was queried but returned null.",
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
            assure("Initial container is not empty: " + xCont.getCount(), xCont.getCount()==0);
            log.println("Inserting a PropertyValue.");
            xCont.insertByIndex(0, prop1);
            PropertyValue[]ret = (PropertyValue[])xCont.getByIndex(0);
            assure("Got the wrong PropertyValue: " +
                                    ret[0].Name +  "  " +(String)ret[0].Value,
                                    ret[0].Name.equals(prop1[0].Name) &&
                                    ret[0].Value.equals(prop1[0].Value));
            log.println("Replace the PropertyValue.");
            xCont.replaceByIndex(0, prop2);
            ret = (PropertyValue[])xCont.getByIndex(0);
            assure("Got the wrong PropertyValue: " +
                                    ret[0].Name +  "  " +(String)ret[0].Value,
                                    ret[0].Name.equals(prop2[0].Name) &&
                                    ret[0].Value.equals(prop2[0].Value));
            log.println("Remove the PropertyValue.");
            xCont.removeByIndex(0);
            assure("Could not remove PropertyValue.",
                                !xCont.hasElements() && xCont.getCount()==0);
            log.println("Insert again.");
            xCont.insertByIndex(0, prop1);
            xCont.insertByIndex(1, prop2);
            assure("Did not insert PropertyValue.",
                                xCont.hasElements() && xCont.getCount()==2);

            try {
                log.println("Insert with wrong index.");
                xCont.insertByIndex(25, prop2);
                failed("IllegalArgumentException was not thrown.");
            }
            catch(com.sun.star.lang.IllegalArgumentException e) {
                log.println("Wrong exception thrown.");
                failed(e.getMessage());
                e.printStackTrace();
            }
            catch(com.sun.star.lang.IndexOutOfBoundsException e) {
                log.println("Expected exception thrown: "+e);
            }
            catch(com.sun.star.lang.WrappedTargetException e) {
                log.println("Wrong exception thrown.");
                failed(e.getMessage());
                e.printStackTrace();
            }

            try {
                log.println("Remove non-existing index.");
                xCont.removeByIndex(25);
                failed("IndexOutOfBoundsException was not thrown.");
            }
            catch(com.sun.star.lang.IndexOutOfBoundsException e) {
                log.println("Expected exception thrown: "+e);
            }
            catch(com.sun.star.lang.WrappedTargetException e) {
                log.println("Wrong exception thrown.");
                failed(e.getMessage());
                e.printStackTrace();
            }

            try {
                log.println("Insert wrong argument.");
                xCont.insertByIndex(2, "Example String");
                failed("IllegalArgumentException was not thrown.");
            }
            catch(com.sun.star.lang.IllegalArgumentException e) {
                log.println("Expected exception thrown: " + e);
            }
            catch(com.sun.star.lang.IndexOutOfBoundsException e) {
                log.println("Wrong exception thrown.");
                failed(e.getMessage());
                e.printStackTrace();
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
        catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            failed(e.getMessage());
            e.printStackTrace();
        }
        catch(com.sun.star.lang.WrappedTargetException e) {
            failed(e.getMessage());
            e.printStackTrace();
        }
    }

}
