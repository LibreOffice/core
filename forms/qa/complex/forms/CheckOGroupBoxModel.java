/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: CheckOGroupBoxModel.java,v $
 * $Revision: 1.4 $
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
