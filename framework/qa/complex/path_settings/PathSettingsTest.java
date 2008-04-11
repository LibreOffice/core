/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PathSettingsTest.java,v $
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
package complex.path_settings;

import com.sun.star.beans.Property;
import com.sun.star.beans.XFastPropertySet;
import com.sun.star.beans.XMultiPropertySet;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertiesChangeListener;
import com.sun.star.beans.XPropertyChangeListener;
import com.sun.star.beans.XVetoableChangeListener;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.AnyConverter;
import complexlib.ComplexTestCase;

public class PathSettingsTest extends ComplexTestCase {

    private static XMultiServiceFactory xMSF;

    // the test object: an instance of the tested service
    private static Object oObj = null;
    // the properties of the service
    private static Property[] props = null;
    private static String[] propNames = null;
    private static String[] availablePropNames = new String[]{
    "Addin",
    "AutoCorrect",
    "Autotext",
    "Backup",
    "Basic",
    "Bitmap",
    "Config",
    "Dictionary",
    "Favorite",
    "Filter",
    "Gallery",
    "Help",
    "Linguistic",
    "Module",
    "Palette",
    "Plugin",
    "Temp",
    "Template",
    "UIConfig",
    "UserConfig",
    "UserDictionary",
    "Work",
};
    private static String[] propVals = null;

    /**
     * A function to tell the framework, which test functions are available.
     * Right now, it's only 'checkComplexTemplateState'.
     * @return All test methods.
     */
    public String[] getTestMethodNames() {
        return new String[]{"checkXFastPropertySet",
                            "checkXMultiPropertySet",
                            "checkXPropertySet"
                        };
    }

    /**
     * Initialize before the tests start: this has to be done only once.
     * This methods sets the 'oObj' and 'props' variables.
     */
    public void before() {
        try {
            xMSF = (XMultiServiceFactory)param.getMSF();
//            oObj = xMSF.createInstance("com.sun.star.util.PathSettings");
            oObj = xMSF.createInstance("com.sun.star.comp.framework.PathSettings");
            System.out.println("Implementation: " + util.utils.getImplName(oObj));
            System.out.println("Service:        ");
            util.dbg.getSuppServices(oObj);
            if (oObj == null) throw new com.sun.star.uno.Exception();
            XPropertySet xProp = (XPropertySet)
                        UnoRuntime.queryInterface(XPropertySet.class, oObj);

            props = xProp.getPropertySetInfo().getProperties();
            propNames = new String[props.length];
            propVals = new String[props.length];

            // get intitial values and create new ones
            log.println("\n---- All properties ----");
            for (int i = 1; i < props.length; i++) {
                propNames[i] = props[i].Name;
                Object o = xProp.getPropertyValue(propNames[i]);
                System.out.println("#### Object: " + o.getClass().getName() + "  -  " + o.toString());
                propVals[i] = AnyConverter.toString(o);
                System.out.println("#### String " + propVals[i]);
                log.println("Property Name: " + propNames[i]);
                log.println("Property Value: " + propVals[i]);
            }
            log.println("---- Finish showing properties ----\n");
        }
        catch(com.sun.star.uno.Exception e) {
            e.printStackTrace();
            log.println(e.getClass().getName());
            log.println("Message: " + e.getMessage());
            failed("Could not create an instance of the test object.");
        }
        catch(Exception e) {
            e.printStackTrace();
            failed("What exception?");
        }
    }

    /**
     * This tests the XFastPropertySet interface implementation.
     */
    public void checkXFastPropertySet()
    {
        log.println("---- Testing the XFastPropertySet interface ----");

        // creating instances
        XFastPropertySet xFPS = (XFastPropertySet)
                UnoRuntime.queryInterface(XFastPropertySet.class, oObj);

        String name = null;
        // do for all properties
        for (int i = 0; i < props.length; i++) {
            try {
                Property property = props[i];
                name = property.Name;
                int handle = property.Handle;

                // get property name and initial value
                log.println("Test property with name: " + name);
                String val = (String)xFPS.getFastPropertyValue(handle);
                log.println("Property has initial value: '" + val + "'");

                // set to a new correct value
                String newVal = changeToCorrectValue(val);
                log.println("Try to change to correct value '" + newVal + "'");
                xFPS.setFastPropertyValue(handle, newVal);

                // check the change
                String checkVal = (String)xFPS.getFastPropertyValue(handle);
                assure("Did not change value on property " + name + ".", checkVal.equals(newVal));

                newVal = changeToIncorrectValue(val);
                log.println("Try to change to incorrect value '" + newVal + "'");
                try {
                    xFPS.setFastPropertyValue(handle, newVal);
                }
                catch(com.sun.star.lang.IllegalArgumentException e) {
                    log.println("Correctly thrown Exception caught.");
                }

                // check if changed
                checkVal = (String)xFPS.getFastPropertyValue(handle);
                assure("Value did change on property " + name + " though it should not have.",
                                                                            !checkVal.equals(newVal));

                // set back to initial setting
                xFPS.setFastPropertyValue(handle, val);

                // check if changed
                checkVal = (String)xFPS.getFastPropertyValue(handle);
                assure("Did not change value back to original on property "
                                            + name, checkVal.equals(val));
                log.println("Test of property " + name + " finished\n");
            }
            catch(com.sun.star.uno.Exception e) {
//                e.printStackTrace();
                log.println(e.getClass().getName());
                log.println("Message: " + e.getMessage());
                failed("Unexpected exception on property " + name + ".");
                continue;
            }
        }
        log.println("---- Test of XFastPropertySet finished ----\n");
    }


    // ____________________
    /**
     * This tests the XMultiPropertySet interface implementation.
     */
    public void checkXMultiPropertySet()
    {
        log.println("---- Testing the XMultiPropertySet interface ----");
        XMultiPropertySet xMPS = (XMultiPropertySet)
            UnoRuntime.queryInterface(XMultiPropertySet.class, oObj);

        String[] correctVals = new String[props.length];
        String[] incorrectVals = new String[props.length];

        // get intitial values and create new ones
        for (int i = 0; i < props.length; i++) {
            correctVals[i] = changeToCorrectValue(propVals[i]);
            incorrectVals[i] = changeToIncorrectValue(propVals[i]);
        }

        try {
            // add a change listener
            MyChangeListener mListener = new MyChangeListener();
            xMPS.addPropertiesChangeListener(propNames, mListener);

            // first change props to correct values
            log.println("Change to correct values.");
            xMPS.setPropertyValues(propNames, correctVals);
            assure("Could not change to correct values with XMultiPropoertySet.",
                            verifyPropertySet(xMPS,propNames,correctVals)>0);

            // second, change to incorrect values: expect an exception
            log.println("Try to change to incorrect values.");
            try {
                xMPS.setPropertyValues(propNames, incorrectVals);
            }
            catch(com.sun.star.lang.IllegalArgumentException r) {
                log.println("Correctly thrown Exception caught.");
            }
            assure("Did change to incorrect values with XMultiPropertySet," +
                            " but should not have.",
                            verifyPropertySet(xMPS,propNames,correctVals)>0);

            // third, change back to initial values
            log.println("Change back to initial values.");
            xMPS.setPropertyValues(propNames, propVals);
            assure("Could not change back to initial values with" +
                                " XMultiPropertySet.",
                                verifyPropertySet(xMPS,propNames,propVals)>0);

            // fire the event for the listener
            log.println("Fire event.");
            xMPS.firePropertiesChangeEvent(propNames, mListener);
            assure("Event was not fired on XMultiPropertySet.",
                                        mListener.changePropertiesEventFired());
        }
        catch(com.sun.star.uno.Exception e) {
//            e.printStackTrace();
            log.println(e.getClass().getName());
            log.println("Message: " + e.getMessage());
            failed("Unexpected exception on XMultiPropertySet.");
        }

        // test finished
        log.println("---- Test of XMultiPropertySet finished ----\n");
    }

    /**
     * Verify if the values of xProp are the same as vals.
     * @param xProp A XMultiPropertySet.
     * @param propNames An array with property names.
     * @param vals An array with values of the properties
     * @return -1 if none are equal, 1 if all are equal, 0 if some were equal
     * and some not.
     * @throws com.sun.star.lang.IllegalArgumentException
     */
    private int verifyPropertySet(XMultiPropertySet xProp,
                            String[] propNames, String[] vals)
    {
        int ret=0;
        if (vals.length != propNames.length) {
            log.println("Length of array parameters must be equal.");
            return ret;
        }
        for (int i = 0; i < vals.length; i++) {
            Object[] objs = xProp.getPropertyValues(new String[]{propNames[i]});
            String retVal = (String)objs[0];
            boolean nCheck = retVal.equals(vals[i]);
            if (!nCheck) {
                log.println("Property '" + propNames[i] +
                                        "' was supposed to have value:");
                log.println(vals[i]);
                log.println("but has value:");
                log.println(retVal);
            }
            // initialize
            if (i==0) {
                ret = nCheck?1:-1;
                continue;
            }
            // return 0 if equal state changes compared to initial value
            if ((nCheck && ret<0) || (!nCheck && ret>0)) {
                ret = 0;
            }
        }
        return ret;
    }


    // ____________________
    /**
     * This tests the XPropertySet interface implementation.
     */
    public void checkXPropertySet()
    {
        log.println("---- Testing the XPropertySet interface ----");

        XPropertySet xPS = (XPropertySet)
                            UnoRuntime.queryInterface(XPropertySet.class, oObj);

        MyChangeListener mListener1 = new MyChangeListener();
        MyChangeListener mListener2 = new MyChangeListener();

        for (int i=0; i<props.length; i++) {
            // adding listeners
            String name = propNames[i];
            log.println("Testing property '" + name + "'");
            try {
                log.println("Add 2 Listeners.");
                xPS.addPropertyChangeListener(name, mListener1);
                xPS.addVetoableChangeListener(name, mListener1);
                xPS.addPropertyChangeListener(name, mListener2);
                xPS.addVetoableChangeListener(name, mListener2);

                // change the property
                log.println("Change value.");
                String changeVal = changeToCorrectValue(propVals[i]);
                xPS.setPropertyValue(name, changeVal);
                String newVal = (String)xPS.getPropertyValue(name);

                assure("Value did not change on property " + name + ".",
                                                    newVal.equals(changeVal));

                assure("Listener 1 was not called.", checkListener(mListener1), true);
                assure("Listener 2 was not called.", checkListener(mListener2), true);

                mListener1.resetListener();
                mListener2.resetListener();

                log.println("Remove Listener 1.");

                xPS.removePropertyChangeListener(name, mListener1);
                xPS.removeVetoableChangeListener(name, mListener1);

                // change the property
                log.println("Change value back.");
                xPS.setPropertyValue(name, propVals[i]);
                newVal = (String)xPS.getPropertyValue(name);
                assure("Value did not change on property " + name,
                                                    newVal.equals(propVals[i]));

                assure("Listener was called, although it was removed on" +
                        " property " + name + ".", !checkListener(mListener1), true);
                assure("Listener 2 was not called on property " + name + ".",
                                                    checkListener(mListener2), true);
            }
            catch(com.sun.star.uno.Exception e) {
                e.printStackTrace();
                log.println(e.getClass().getName());
                log.println("Message: " + e.getMessage());
                failed("Unexpcted exception on property " + name);
                continue;
            }
            log.println("Finish testing property '" + propNames[i] + "'\n");
        }
        log.println("---- Test of XPropertySet finished ----\n");

    }

    private boolean checkListener(MyChangeListener ml) {
        return  ml.changePropertyEventFired() ||
                ml.changePropertiesEventFired() ||
                ml.vetoableChangeEventFired();
    }

    // ____________________
    /**
     * Change the given String to a correct path URL.
     * @return The changed path URL.
     */
    private String changeToCorrectValue(String path) {
        // the simplest possiblity
        if ( path == null || path.equals("") ) {
            return "file:///tmp";
        }
        return path + "/tmp";
    }


    /**
     * Change the given String to an incorrect path URL.
     * @return The changed path URL.
     */
    private String changeToIncorrectValue(String path) {
        // the simplest possiblity
        return "fileblablabla";
    }


    /**
    * Listener implementation which sets a flag when
    * listener was called.
    */
    public class MyChangeListener implements XPropertiesChangeListener,
                                                XPropertyChangeListener,
                                                XVetoableChangeListener {

         private boolean propChanged = false;
         private boolean propertiesChanged = false;
         private boolean disposeCalled = false;
         private boolean vetoableChanged = false;

         public void propertiesChange(
                    com.sun.star.beans.PropertyChangeEvent[] e) {
             propertiesChanged = true;
         }

         public void vetoableChange(com.sun.star.beans.PropertyChangeEvent pE)
                            throws com.sun.star.beans.PropertyVetoException {
             vetoableChanged = true;
         }

         public void propertyChange(com.sun.star.beans.PropertyChangeEvent pE) {
             propChanged = true;
         }

         public void disposing(com.sun.star.lang.EventObject eventObject) {
             disposeCalled = true;
         }

         public void resetListener() {
             propChanged = false;
             propertiesChanged = false;
             disposeCalled = false;
             vetoableChanged = false;
         }

         public boolean changePropertyEventFired() {
             return propChanged;
         }
         public boolean changePropertiesEventFired() {
             return propertiesChanged;
         }
         public boolean vetoableChangeEventFired() {
             return vetoableChanged;
         }

    };
}
