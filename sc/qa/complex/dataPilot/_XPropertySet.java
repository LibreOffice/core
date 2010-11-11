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

package complex.dataPilot;

import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyAttribute;
import com.sun.star.beans.PropertyChangeEvent;
import com.sun.star.beans.XPropertyChangeListener;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.beans.XVetoableChangeListener;
import com.sun.star.lang.EventObject;
import java.util.Random;
import java.util.StringTokenizer;
import lib.TestParameters;
// import share.LogWriter;
//import lib.MultiMethodTest;
import util.ValueChanger;
import util.utils;

/**
* Testing <code>com.sun.star.beans.XPropertySet</code>
* interface methods :
* <ul>
*  <li><code>getPropertySetInfo()</code></li>
*  <li><code>setPropertyValue()</code></li>
*  <li><code>getPropertyValue()</code></li>
*  <li><code>addPropertyChangeListener()</code></li>
*  <li><code>removePropertyChangeListener()</code></li>
*  <li><code>addVetoableChangeListener()</code></li>
*  <li><code>removeVetoableChangeListener()</code></li>
* </ul>
* @see com.sun.star.beans.XPropertySet
*/
public class _XPropertySet {

    /**
     * The object that is testsed.
     */
    private XPropertySet oObj = null;

    /**
     * The test parameters
     */
    private TestParameters param = null;

    /**
     * The log writer
     */
    // private LogWriter log = null;

    /**
    * Flag that indicates change listener was called.
    */
    boolean propertyChanged = false;


    /**
     * The own property change listener
     */
    XPropertyChangeListener PClistener = new MyChangeListener();

    /**
    * Listener that must be called on bound property changing.
    */
    public class MyChangeListener implements XPropertyChangeListener {
         /**
         * Just set <code>propertyChanged</code> flag to true.
         */
         public void propertyChange(PropertyChangeEvent e) {
            propertyChanged = true;
         }
         public void disposing (EventObject obj) {}
    };


    /**
    * Flag that indicates veto listener was called.
    */
    boolean vetoableChanged = false;

    /**
     * The own vetoable change listener
     */
    XVetoableChangeListener VClistener = new MyVetoListener();

    /**
    * Listener that must be called on constrained property changing.
    */
    public class MyVetoListener implements XVetoableChangeListener {
         /**
         * Just set <code>vetoableChanged</code> flag to true.
         */
         public void vetoableChange(PropertyChangeEvent e) {
            vetoableChanged = true;
         }
         public void disposing (EventObject obj) {}
    };


    /**
     * Properties to test
     */
    PropsToTest PTT = new PropsToTest();

    /**
    * Structure that collects three properties of each type to test :
    * Constrained, Bound and Normal.
    */
    public class PropsToTest {
           String constrained = null;
           String bound = null;
           String normal = null;
    }

    /**
     * Constructor: gets the object to test, a logger and the test parameters
     * @param xObj The test object
     * @param log A log writer
     * @param param The test parameters
     */
    public _XPropertySet(XPropertySet xObj/*, LogWriter log*/, TestParameters param) {
        oObj = xObj;
        // this.log = log;
        this.param = param;
    }

    /**
    * Tests method <code>getPropertySetInfo</code>. After test completed
    * call {@link #getPropsToTest} method to retrieve different kinds
    * of properties to test then. <p>
    * Has OK status if not null <code>XPropertySetInfo</code>
    * object returned.<p>
    * Since <code>getPropertySetInfo</code> is optional, it may return null,
    * if it is not implemented. This method uses then an object relation
    * <code>PTT</code> (Properties To Test) to determine available properties.
    * All tests for services without <code>getPropertySetInfo</code> must
    * provide this object relation.
    */
    public boolean _getPropertySetInfo() {
        XPropertySetInfo propertySetInfo = oObj.getPropertySetInfo();

        if (propertySetInfo == null) {
            System.out.println("getPropertySetInfo() method returned null");
            String[] ptt = (String[]) param.get("PTT");
            PTT.normal=ptt[0];
            PTT.bound=ptt[1];
            PTT.constrained=ptt[2];
        } else {
            getPropsToTest(propertySetInfo);
        }

        return true;

    } // end of getPropertySetInfo()

    /**
    * Tests change listener which added for bound properties.
    * Adds listener to bound property (if it exists), then changes
    * its value and check if listener was called. <p>
    * Method tests to be successfully completed before :
    * <ul>
    *  <li> <code>getPropertySetInfo</code> : in this method test
    *    one of bound properties is retrieved. </li>
    * </ul> <p>
    * Has OK status if NO bound properties exist or if listener
    * was successfully called.
    */
    public boolean _addPropertyChangeListener() {

        propertyChanged = false;
        boolean result = true;

        if ( PTT.bound.equals("none") ) {
            System.out.println("*** No bound properties found ***");
        } else {
            try {
                oObj.addPropertyChangeListener(PTT.bound,PClistener);
                Object gValue = oObj.getPropertyValue(PTT.bound);
                oObj.setPropertyValue(PTT.bound,
                    ValueChanger.changePValue(gValue));
            } catch (com.sun.star.beans.PropertyVetoException e) {
                System.out.println("Exception occured while trying to change "+
                    "property '"+ PTT.bound+"'");
                e.printStackTrace();
            } catch (com.sun.star.lang.IllegalArgumentException e) {
                System.out.println("Exception occured while trying to change "+
                    "property '"+ PTT.bound+"'");
                e.printStackTrace();
            } catch (com.sun.star.beans.UnknownPropertyException e) {
                System.out.println("Exception occured while trying to change "+
                    "property '"+ PTT.bound+"'");
                e.printStackTrace();
            } catch (com.sun.star.lang.WrappedTargetException e) {
                System.out.println("Exception occured while trying to change "+
                    "property '"+ PTT.bound+"'");
                e.printStackTrace();
            } // end of try-catch
            result = propertyChanged;
            if (!propertyChanged) {
                System.out.println("propertyChangeListener wasn't called for '"+
                    PTT.bound+"'");
            }
        } //endif

        return result;

    } // end of addPropertyChangeListener()

    /**
    * Tests vetoable listener which added for constrained properties.
    * Adds listener to constrained property (if it exists), then changes
    * its value and check if listener was called. <p>
    * Method tests to be successfully completed before :
    * <ul>
    *  <li> <code>getPropertySetInfo</code> : in this method test
    *    one of constrained properties is retrieved. </li>
    * </ul> <p>
    * Has OK status if NO constrained properties exist or if listener
    * was successfully called.
    */
    public boolean _addVetoableChangeListener() {

//        requiredMethod("getPropertySetInfo()");

        vetoableChanged = false;
        boolean result = true;

        if ( PTT.constrained.equals("none") ) {
            System.out.println("*** No constrained properties found ***");
        } else {
            try {
                oObj.addVetoableChangeListener(PTT.constrained,VClistener);
                Object gValue = oObj.getPropertyValue(PTT.constrained);
                oObj.setPropertyValue(PTT.constrained,
                    ValueChanger.changePValue(gValue));
            } catch (com.sun.star.beans.PropertyVetoException e) {
                System.out.println("Exception occured while trying to change "+
                    "property '"+ PTT.constrained+"'");
                e.printStackTrace();
            } catch (com.sun.star.lang.IllegalArgumentException e) {
                System.out.println("Exception occured while trying to change "+
                    "property '"+ PTT.constrained+"'");
                e.printStackTrace();
            } catch (com.sun.star.beans.UnknownPropertyException e) {
                System.out.println("Exception occured while trying to change "+
                    "property '"+ PTT.constrained+"'");
                e.printStackTrace();
            } catch (com.sun.star.lang.WrappedTargetException e) {
                System.out.println("Exception occured while trying to change "+
                    "property '"+ PTT.constrained+"'");
                e.printStackTrace();
            } // end of try-catch
            result = vetoableChanged;
            if (!vetoableChanged) {
                System.out.println("vetoableChangeListener wasn't called for '"+
                    PTT.constrained+"'");
            }
        } //endif

        return result;

    } // end of addVetoableChangeListener()


    /**
    * Tests <code>setPropertyValue</code> method.
    * Stores value before call, and compares it with value after
    * call. <p>
    * Method tests to be successfully completed before :
    * <ul>
    *  <li> <code>getPropertySetInfo</code> : in this method test
    *    one of normal properties is retrieved. </li>
    * </ul> <p>
    * Has OK status if NO normal properties exist or if value before
    * method call is not equal to value after.
    */
    public boolean _setPropertyValue() {

//        requiredMethod("getPropertySetInfo()");

        Object gValue = null;
        Object sValue = null;

        boolean result = true;

        if ( PTT.normal.equals("none") ) {
            System.out.println("*** No changeable properties found ***");
        } else {
            try {
                gValue = oObj.getPropertyValue(PTT.normal);
                sValue = ValueChanger.changePValue(gValue);
                oObj.setPropertyValue(PTT.normal, sValue);
                sValue = oObj.getPropertyValue(PTT.normal);
            } catch (com.sun.star.beans.PropertyVetoException e) {
                System.out.println("Exception occured while trying to change "+
                    "property '"+ PTT.normal+"'");
                e.printStackTrace();
            } catch (com.sun.star.lang.IllegalArgumentException e) {
                System.out.println("Exception occured while trying to change "+
                    "property '"+ PTT.normal+"'");
                e.printStackTrace();
            } catch (com.sun.star.beans.UnknownPropertyException e) {
                System.out.println("Exception occured while trying to change "+
                    "property '"+ PTT.normal+"'");
                e.printStackTrace();
            } catch (com.sun.star.lang.WrappedTargetException e) {
                System.out.println("Exception occured while trying to change "+
                    "property '"+ PTT.normal+"'");
                e.printStackTrace();
            } // end of try-catch
            result = !gValue.equals(sValue);
        } //endif

        return result;

    } // end of setPropertyValue()

    /**
    * Tests <code>getPropertyValue</code> method.
    * Just call this method and checks for no exceptions <p>
    * Method tests to be successfully completed before :
    * <ul>
    *  <li> <code>getPropertySetInfo</code> : in this method test
    *    one of normal properties is retrieved. </li>
    * </ul> <p>
    * Has OK status if NO normal properties exist or if no
    * exceptions were thrown.
    */
    public boolean _getPropertyValue() {

//        requiredMethod("getPropertySetInfo()");

        boolean result = true;
        String toCheck = PTT.normal;

        if ( PTT.normal.equals("none") ) {
            toCheck = oObj.getPropertySetInfo().getProperties()[0].Name;
            System.out.println("All properties are Read Only");
            System.out.println("Using: "+toCheck);
        }

        try {
            Object gValue = oObj.getPropertyValue(toCheck);
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            System.out.println("Exception occured while trying to get property '"+
                 PTT.normal+"'");
            e.printStackTrace();
            result = false;
        } catch (com.sun.star.lang.WrappedTargetException e) {
            System.out.println("Exception occured while trying to get property '"+
                PTT.normal+"'");
            e.printStackTrace();
            result = false;
        } // end of try-catch

        return result;
    }

    /**
    * Tests <code>removePropertyChangeListener</code> method.
    * Removes change listener, then changes bound property value
    * and checks if the listener was NOT called.
    * Method tests to be successfully completed before :
    * <ul>
    *  <li> <code>addPropertyChangeListener</code> : here listener
    *   was added. </li>
    * </ul> <p>
    * Has OK status if NO bound properties exist or if listener
    * was not called and no exceptions arose.
    */
    public boolean _removePropertyChangeListener() {

//        requiredMethod("addPropertyChangeListener()");

        propertyChanged = false;
        boolean result = true;

        if ( PTT.bound.equals("none") ) {
            System.out.println("*** No bound properties found ***");
        } else {
            try {
                propertyChanged = false;
                oObj.removePropertyChangeListener(PTT.bound,PClistener);
                Object gValue = oObj.getPropertyValue(PTT.bound);
                oObj.setPropertyValue(PTT.bound,
                    ValueChanger.changePValue(gValue));
            } catch (com.sun.star.beans.PropertyVetoException e) {
                System.out.println("Exception occured while trying to change "+
                    "property '"+ PTT.bound+"'");
                e.printStackTrace();
            } catch (com.sun.star.lang.IllegalArgumentException e) {
                System.out.println("Exception occured while trying to change "+
                    "property '"+ PTT.bound+"'");
                e.printStackTrace();
            } catch (com.sun.star.beans.UnknownPropertyException e) {
                System.out.println("Exception occured while trying to change "+
                    "property '"+ PTT.bound+"'");
                e.printStackTrace();
            } catch (com.sun.star.lang.WrappedTargetException e) {
                System.out.println("Exception occured while trying to change "+
                    "property '"+ PTT.bound+"'");
                e.printStackTrace();
            } // end of try-catch

             result = !propertyChanged;
            if (propertyChanged) {
                System.out.println("propertyChangeListener was called after removing"+
                    " for '"+PTT.bound+"'");
            }
        } //endif

        return result;

    } // end of removePropertyChangeListener()


    /**
    * Tests <code>removeVetoableChangeListener</code> method.
    * Removes vetoable listener, then changes constrained property value
    * and checks if the listener was NOT called.
    * Method tests to be successfully completed before :
    * <ul>
    *  <li> <code>addPropertyChangeListener</code> : here vetoable listener
    *   was added. </li>
    * </ul> <p>
    * Has OK status if NO constrained properties exist or if listener
    * was NOT called and no exceptions arose.
    */
    public boolean _removeVetoableChangeListener() {

//        requiredMethod("addVetoableChangeListener()");

        vetoableChanged = false;
        boolean result = true;

        if ( PTT.constrained.equals("none") ) {
            System.out.println("*** No constrained properties found ***");
        } else {
            try {
                oObj.removeVetoableChangeListener(PTT.constrained,VClistener);
                Object gValue = oObj.getPropertyValue(PTT.constrained);
                oObj.setPropertyValue(PTT.constrained,
                    ValueChanger.changePValue(gValue));
            } catch (com.sun.star.beans.PropertyVetoException e) {
                System.out.println("Exception occured while trying to change "+
                    "property '"+ PTT.constrained+"'");
                e.printStackTrace();
            } catch (com.sun.star.lang.IllegalArgumentException e) {
                System.out.println("Exception occured while trying to change "+
                    "property '"+ PTT.constrained+"'");
                e.printStackTrace();
            } catch (com.sun.star.beans.UnknownPropertyException e) {
                System.out.println("Exception occured while trying to change "+
                    "property '"+ PTT.constrained+"'");
                e.printStackTrace();
            } catch (com.sun.star.lang.WrappedTargetException e) {
                System.out.println("Exception occured while trying to change "+
                    "property '"+ PTT.constrained+"'");
                e.printStackTrace();
            } // end of try-catch

            result = !vetoableChanged;
            if (vetoableChanged) {
                System.out.println("vetoableChangeListener was called after "+
                    "removing for '"+PTT.constrained+"'");
            }
        } //endif

        return result;

    } // end of removeVetoableChangeListener()


    /**
    * Gets the properties being tested. Searches and stores by one
    * property of each kind (Bound, Vetoable, Normal).
    */
    public PropsToTest getPropsToTest(XPropertySetInfo xPSI) {

        Property[] properties = xPSI.getProperties();
        String bound = "";
        String constrained = "";
        String normal = "";

        for (int i = 0; i < properties.length; i++) {

            Property property = properties[i];
            String name = property.Name;
            System.out.println("Checking '"+name+"'");
            boolean isWritable = ((property.Attributes &
                PropertyAttribute.READONLY) == 0);
            boolean isNotNull = ((property.Attributes &
                PropertyAttribute.MAYBEVOID) == 0);
            boolean isBound = ((property.Attributes &
                PropertyAttribute.BOUND) != 0);
            boolean isConstr = ((property.Attributes &
                PropertyAttribute.CONSTRAINED) != 0);
            boolean canChange = false;

            if ( !isWritable ) System.out.println("Property '"+name+"' is READONLY");

            if (name.endsWith("URL")) isWritable = false;
            if (name.startsWith("Fill")) isWritable = false;
            if (name.startsWith("Font")) isWritable = false;
            if (name.startsWith("IsNumbering")) isWritable = false;
            if (name.startsWith("LayerName")) isWritable = false;
            if (name.startsWith("Line")) isWritable = false;

            //if (name.equals("xinterfaceA") || name.equals("xtypeproviderA")
            //|| name.equals("arAnyA")) isWritable=false;

            if ( isWritable && isNotNull ) canChange = isChangeable(name);

            if ( isWritable && isNotNull && isBound && canChange) {
                bound+=name+";";
            }

            if ( isWritable && isNotNull && isConstr && canChange) {
                constrained+=name+";";
            }

            if ( isWritable && isNotNull && canChange) normal+=name+";";


        } // endfor

        //get a random bound property
        PTT.bound=getRandomString(bound);
        System.out.println("Bound: "+PTT.bound);

        //get a random constrained property
        PTT.constrained=getRandomString(constrained);
        System.out.println("Constrained: "+PTT.constrained);

        //get a random normal property
        PTT.normal=getRandomString(normal);

        return PTT;

    }

    /**
    * Retrieves one random property name from list (property names separated
    * by ';') of property names.
    */
    public String getRandomString(String str) {

        String gRS = "none";
        Random rnd = new Random();

        if (str.equals("")) str = "none";
        StringTokenizer ST=new StringTokenizer(str,";");
        int nr = rnd.nextInt(ST.countTokens());
        if (nr < 1) nr+=1;
        for (int i=1; i<nr+1; i++) gRS = ST.nextToken();

        return gRS;

    }

    public boolean isChangeable(String name) {
        boolean hasChanged = false;
        try {
            Object getProp = oObj.getPropertyValue(name);
            System.out.println("Getting: "+getProp);

            Object setValue = null;
            if (getProp != null) {
                if (!utils.isVoid(getProp))
                    setValue = ValueChanger.changePValue(getProp);
                else System.out.println("Property '"+name+
                    "' is void but MAYBEVOID isn't set");
            } else System.out.println("Property '"+name+"'is null and can't be changed");
            if (name.equals("LineStyle")) setValue = null;
            if (setValue != null) {
                oObj.setPropertyValue(name, setValue);
                System.out.println("Setting to :"+setValue);
                hasChanged = (! getProp.equals(oObj.getPropertyValue(name)));
            } else System.out.println("Couldn't change Property '"+name+"'");
        } catch (com.sun.star.beans.PropertyVetoException e) {
            System.out.println("'" + name + "' throws exception '" + e + "'");
            e.printStackTrace();
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            System.out.println("'" + name + "' throws exception '" + e + "'");
            e.printStackTrace();
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            System.out.println("'" + name + "' throws exception '" + e + "'");
            e.printStackTrace();
        } catch (com.sun.star.lang.WrappedTargetException e) {
            System.out.println("'" + name + "' throws exception '" + e + "'");
            e.printStackTrace();
        } catch (com.sun.star.uno.RuntimeException e) {
            System.out.println("'" + name + "' throws exception '" + e + "'");
            e.printStackTrace();
        } catch (java.lang.ArrayIndexOutOfBoundsException e) {
            System.out.println("'" + name + "' throws exception '" + e + "'");
            e.printStackTrace();
        }

        return hasChanged;
    }


}  // finish class _XPropertySet

