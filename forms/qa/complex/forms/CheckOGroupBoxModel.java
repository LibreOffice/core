/*************************************************************************
 *
 *  $RCSfile: CheckOGroupBoxModel.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Date: 2007-07-24 13:13:16 $
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
package complex.forms;

import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyAttribute;
import com.sun.star.beans.PropertyChangeEvent;
import com.sun.star.beans.XMultiPropertySet;
import com.sun.star.beans.XPropertiesChangeListener;
import com.sun.star.lang.EventObject;
import com.sun.star.drawing.XControlShape;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import complexlib.ComplexTestCase;
import java.io.PrintWriter;
import java.util.Vector;
import util.FormTools;
import util.SOfficeFactory;
import util.ValueChanger;

/**
 */
public class CheckOGroupBoxModel extends ComplexTestCase {

    private XMultiPropertySet xPropSet = null;

    public String[] getTestMethodNames() {
        return new String[] {"setPropertyValues"};
    }

    public void before() {
        XComponent xDrawDoc = null;
        SOfficeFactory SOF = SOfficeFactory.getFactory(((XMultiServiceFactory) param.getMSF()));

        try {
            log.println("creating a draw document");
            xDrawDoc = SOF.createDrawDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace((PrintWriter)log);
            failed("Couldn't create document.");
        }

        String objName = "GroupBox";
        XControlShape shape = FormTools.insertControlShape(xDrawDoc, 5000, 7000, 2000, 2000, objName);
        xPropSet = (XMultiPropertySet)UnoRuntime.queryInterface(XMultiPropertySet.class, shape.getControl());
    }


    public void setPropertyValues() {
        String[] boundPropsToTest = getBoundPropsToTest();

        MyChangeListener ml = new MyChangeListener();
        xPropSet.addPropertiesChangeListener(boundPropsToTest, ml);

        Object[] gValues = xPropSet.getPropertyValues(boundPropsToTest);
        Object[] newValue = new Object[gValues.length];
        log.println("Trying to change all properties.");
        for (int i=0; i<boundPropsToTest.length; i++) {
            newValue[i] = ValueChanger.changePValue(gValues[i]);
        }
        try {
            xPropSet.setPropertyValues(boundPropsToTest, newValue);
        } catch (com.sun.star.beans.PropertyVetoException e) {
            e.printStackTrace((PrintWriter)log);
            failed("Exception occured while trying to change the properties.");
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace((PrintWriter)log);
            failed("Exception occured while trying to change the properties.");
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace((PrintWriter)log);
            failed("Exception occured while trying to change the properties.");
        } // end of try-catch

        assure("Listener was not called.", ml.wasListenerCalled());
        xPropSet.removePropertiesChangeListener(ml);
    }

    private String[] getBoundPropsToTest() {
        Property[] properties = xPropSet.getPropertySetInfo().getProperties();
        String[] testPropsNames = null;

        Vector tNames = new Vector();

        for (int i = 0; i < properties.length; i++) {

            Property property = properties[i];
            String name = property.Name;
            boolean isWritable = ((property.Attributes &
                PropertyAttribute.READONLY) == 0);
            boolean isNotNull = ((property.Attributes &
                PropertyAttribute.MAYBEVOID) == 0);
            boolean isBound = ((property.Attributes &
                PropertyAttribute.BOUND) != 0);

            if ( isWritable && isNotNull && isBound) {
                tNames.add(name);
            }

        } // endfor

        //get a array of bound properties
        testPropsNames = new String[tNames.size()];
        testPropsNames = (String[])tNames.toArray(testPropsNames);
        return testPropsNames;
    }

    /**
    * Listener implementation which sets a flag when
    * listener was called.
    */
    public class MyChangeListener implements XPropertiesChangeListener {
        boolean propertiesChanged = false;
        public void propertiesChange(PropertyChangeEvent[] e) {
             propertiesChanged = true;
        }
        public void disposing (EventObject obj) {}
        public boolean wasListenerCalled() { return propertiesChanged; }
        public void reset() { propertiesChanged = false; }
    };

}
