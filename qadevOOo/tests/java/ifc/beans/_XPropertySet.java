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

package ifc.beans;

import java.util.Random;
import java.util.StringTokenizer;

import lib.MultiMethodTest;
import util.ValueChanger;
import util.utils;

import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyAttribute;
import com.sun.star.beans.PropertyChangeEvent;
import com.sun.star.beans.XPropertyChangeListener;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.beans.XVetoableChangeListener;
import com.sun.star.lang.EventObject;

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
public class _XPropertySet extends MultiMethodTest {

    public XPropertySet oObj = null;

    /**
    * Flag that indicates change listener was called.
    */
    private boolean propertyChanged = false;

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

    private final XPropertyChangeListener PClistener = new MyChangeListener();

    /**
    * Flag that indicates veto listener was called.
    */
    private boolean vetoableChanged = false;

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

    private final XVetoableChangeListener VClistener = new MyVetoListener();

    /**
    * Structure that collects three properties of each type to test :
    * Constrained, Bound and Normal.
    */
    private final class PropsToTest {
           String constrained = null;
           String bound = null;
           String normal = null;
    }

    private final PropsToTest PTT = new PropsToTest();

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
    public void _getPropertySetInfo() {

        XPropertySetInfo propertySetInfo = oObj.getPropertySetInfo();

        if (propertySetInfo == null) {
            log.println("getPropertySetInfo() method returned null");
            tRes.tested("getPropertySetInfo()", true) ;
            String[] ptt = (String[]) tEnv.getObjRelation("PTT");
            PTT.normal=ptt[0];
            PTT.bound=ptt[1];
            PTT.constrained=ptt[2];
        } else {
            tRes.tested("getPropertySetInfo()", true );
            getPropsToTest(propertySetInfo);
        }

        return;

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
    public void _addPropertyChangeListener() {

        requiredMethod("getPropertySetInfo()");

        propertyChanged = false;


        if ( PTT.bound.equals("none") ) {
            log.println("*** No bound properties found ***");
            tRes.tested("addPropertyChangeListener()", true) ;
        } else {
            try {
                oObj.addPropertyChangeListener(PTT.bound,PClistener);
                Object gValue = oObj.getPropertyValue(PTT.bound);
                oObj.setPropertyValue(PTT.bound,
                    ValueChanger.changePValue(gValue));
            } catch (com.sun.star.beans.PropertyVetoException e) {
                log.println("Exception occured while trying to change "+
                    "property '"+ PTT.bound+"'");
                e.printStackTrace(log);
            } catch (com.sun.star.lang.IllegalArgumentException e) {
                log.println("Exception occured while trying to change "+
                    "property '"+ PTT.bound+"'");
                e.printStackTrace(log);
            } catch (com.sun.star.beans.UnknownPropertyException e) {
                log.println("Exception occured while trying to change "+
                    "property '"+ PTT.bound+"'");
                e.printStackTrace(log);
            } catch (com.sun.star.lang.WrappedTargetException e) {
                log.println("Exception occured while trying to change "+
                    "property '"+ PTT.bound+"'");
                e.printStackTrace(log);
            } // end of try-catch
            tRes.tested("addPropertyChangeListener()", propertyChanged);
            if (!propertyChanged) {
                log.println("propertyChangeListener wasn't called for '"+
                    PTT.bound+"'");
            }
        } //endif

        return;

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
    public void _addVetoableChangeListener() {

        requiredMethod("getPropertySetInfo()");

        vetoableChanged = false;

        if ( PTT.constrained.equals("none") ) {
            log.println("*** No constrained properties found ***");
            tRes.tested("addVetoableChangeListener()", true) ;
        } else {
            try {
                oObj.addVetoableChangeListener(PTT.constrained,VClistener);
                Object gValue = oObj.getPropertyValue(PTT.constrained);
                oObj.setPropertyValue(PTT.constrained,
                    ValueChanger.changePValue(gValue));
            } catch (com.sun.star.beans.PropertyVetoException e) {
                log.println("Exception occured while trying to change "+
                    "property '"+ PTT.constrained+"'");
                e.printStackTrace(log);
            } catch (com.sun.star.lang.IllegalArgumentException e) {
                log.println("Exception occured while trying to change "+
                    "property '"+ PTT.constrained+"'");
                e.printStackTrace(log);
            } catch (com.sun.star.beans.UnknownPropertyException e) {
                log.println("Exception occured while trying to change "+
                    "property '"+ PTT.constrained+"'");
                e.printStackTrace(log);
            } catch (com.sun.star.lang.WrappedTargetException e) {
                log.println("Exception occured while trying to change "+
                    "property '"+ PTT.constrained+"'");
                e.printStackTrace(log);
            } // end of try-catch
            tRes.tested("addVetoableChangeListener()",vetoableChanged);
            if (!vetoableChanged) {
                log.println("vetoableChangeListener wasn't called for '"+
                    PTT.constrained+"'");
            }
        } //endif

        return;

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
    public void _setPropertyValue() {

        requiredMethod("getPropertySetInfo()");

        Object gValue = null;
        Object sValue = null;

        if ( PTT.normal.equals("none") ) {
            log.println("*** No changeable properties found ***");
            tRes.tested("setPropertyValue()", true) ;
        } else {
            try {
                log.println("try to cheange value of property '" + PTT.normal + "'" );
                gValue = oObj.getPropertyValue(PTT.normal);
                sValue = ValueChanger.changePValue(gValue);
                oObj.setPropertyValue(PTT.normal, sValue);
                sValue = oObj.getPropertyValue(PTT.normal);
            } catch (com.sun.star.beans.PropertyVetoException e) {
                log.println("Exception occured while trying to change "+
                    "property '"+ PTT.normal+"'");
                e.printStackTrace(log);
            } catch (com.sun.star.lang.IllegalArgumentException e) {
                log.println("Exception occured while trying to change "+
                    "property '"+ PTT.normal+"'");
                e.printStackTrace(log);
            } catch (com.sun.star.beans.UnknownPropertyException e) {
                log.println("Exception occured while trying to change "+
                    "property '"+ PTT.normal+"'");
                e.printStackTrace(log);
            } catch (com.sun.star.lang.WrappedTargetException e) {
                log.println("Exception occured while trying to change "+
                    "property '"+ PTT.normal+"'");
                e.printStackTrace(log);
            } // end of try-catch
            tRes.tested("setPropertyValue()",(! gValue.equals(sValue)));
        } //endif

        return;

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
    public void _getPropertyValue() {

        requiredMethod("getPropertySetInfo()");

        String toCheck = PTT.normal;

        if ( PTT.normal.equals("none") ) {
            toCheck = oObj.getPropertySetInfo().getProperties()[0].Name;
            log.println("All properties are Read Only");
            log.println("Using: "+toCheck);
        }

        try {
            oObj.getPropertyValue(toCheck);
            tRes.tested("getPropertyValue()",true);
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            log.println("Exception occured while trying to get property '"+
                 PTT.normal+"'");
            e.printStackTrace(log);
            tRes.tested("getPropertyValue()",false);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Exception occured while trying to get property '"+
                PTT.normal+"'");
            e.printStackTrace(log);
            tRes.tested("getPropertyValue()",false);
        } // end of try-catch

        return;
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
    public void _removePropertyChangeListener() {

        requiredMethod("addPropertyChangeListener()");

        propertyChanged = false;

        if ( PTT.bound.equals("none") ) {
            log.println("*** No bound properties found ***");
            tRes.tested("removePropertyChangeListener()", true) ;
        } else {
            try {
                propertyChanged = false;
                oObj.removePropertyChangeListener(PTT.bound,PClistener);
                Object gValue = oObj.getPropertyValue(PTT.bound);
                oObj.setPropertyValue(PTT.bound,
                    ValueChanger.changePValue(gValue));
            } catch (com.sun.star.beans.PropertyVetoException e) {
                log.println("Exception occured while trying to change "+
                    "property '"+ PTT.bound+"'");
                e.printStackTrace(log);
            } catch (com.sun.star.lang.IllegalArgumentException e) {
                log.println("Exception occured while trying to change "+
                    "property '"+ PTT.bound+"'");
                e.printStackTrace(log);
            } catch (com.sun.star.beans.UnknownPropertyException e) {
                log.println("Exception occured while trying to change "+
                    "property '"+ PTT.bound+"'");
                e.printStackTrace(log);
            } catch (com.sun.star.lang.WrappedTargetException e) {
                log.println("Exception occured while trying to change "+
                    "property '"+ PTT.bound+"'");
                e.printStackTrace(log);
            } // end of try-catch

            tRes.tested("removePropertyChangeListener()",!propertyChanged);
            if (propertyChanged) {
                log.println("propertyChangeListener was called after removing"+
                    " for '"+PTT.bound+"'");
            }
        } //endif

        return;

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
    public void _removeVetoableChangeListener() {

        requiredMethod("addVetoableChangeListener()");

        vetoableChanged = false;

        if ( PTT.constrained.equals("none") ) {
            log.println("*** No constrained properties found ***");
            tRes.tested("removeVetoableChangeListener()", true) ;
        } else {
            try {
                oObj.removeVetoableChangeListener(PTT.constrained,VClistener);
                Object gValue = oObj.getPropertyValue(PTT.constrained);
                oObj.setPropertyValue(PTT.constrained,
                    ValueChanger.changePValue(gValue));
            } catch (com.sun.star.beans.PropertyVetoException e) {
                log.println("Exception occured while trying to change "+
                    "property '"+ PTT.constrained+"'");
                e.printStackTrace(log);
            } catch (com.sun.star.lang.IllegalArgumentException e) {
                log.println("Exception occured while trying to change "+
                    "property '"+ PTT.constrained+"'");
                e.printStackTrace(log);
            } catch (com.sun.star.beans.UnknownPropertyException e) {
                log.println("Exception occured while trying to change "+
                    "property '"+ PTT.constrained+"'");
                e.printStackTrace(log);
            } catch (com.sun.star.lang.WrappedTargetException e) {
                log.println("Exception occured while trying to change "+
                    "property '"+ PTT.constrained+"'");
                e.printStackTrace(log);
            } // end of try-catch

            tRes.tested("removeVetoableChangeListener()",!vetoableChanged);
            if (vetoableChanged) {
                log.println("vetoableChangeListener was called after "+
                    "removing for '"+PTT.constrained+"'");
            }
        } //endif

        return;

    } // end of removeVetoableChangeListener()


    /**
    * Gets the properties being tested. Searches and stores by one
    * property of each kind (Bound, Vetoable, Normal).
    */
    public void getPropsToTest(XPropertySetInfo xPSI) {

        Property[] properties = xPSI.getProperties();
        String bound = "";
        String constrained = "";
        String normal = "";
        // some properties should not be changed in a unspecific way
        String[] skip = {"PrinterName", "CharRelief", "IsLayerMode"};

        for (int i = 0; i < properties.length; i++) {

            Property property = properties[i];
            String name = property.Name;

            boolean cont = false;
            for (int j = 0; j < skip.length; j++) {
                if (name.equals(skip[j])){
                    log.println("skipping '" + name + "'");
                    cont = true;
                }
            }

            if (cont) continue;

            if (name.equals(oObj))
            log.println("Checking '"+name+"'");
            boolean isWritable = ((property.Attributes &
                PropertyAttribute.READONLY) == 0);
            boolean isNotNull = ((property.Attributes &
                PropertyAttribute.MAYBEVOID) == 0);
            boolean isBound = ((property.Attributes &
                PropertyAttribute.BOUND) != 0);
            boolean isConstr = ((property.Attributes &
                PropertyAttribute.CONSTRAINED) != 0);
            boolean canChange = false;

            if ( !isWritable ) log.println("Property '"+name+"' is READONLY");

            if (name.endsWith("URL")) isWritable = false;
            if (name.startsWith("Fill")) isWritable = false;
            if (name.startsWith("Font")) isWritable = false;
            if (name.startsWith("IsNumbering")) isWritable = false;
            if (name.startsWith("LayerName")) isWritable = false;
            if (name.startsWith("Line")) isWritable = false;
            if (name.startsWith("TextWriting")) isWritable = false;

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
        log.println("Bound: "+PTT.bound);

        //get a random constrained property
        PTT.constrained=getRandomString(constrained);
        log.println("Constrained: "+PTT.constrained);

        //get a random normal property
        PTT.normal=getRandomString(normal);
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
            log.println("Getting: "+getProp);
            if (name.equals("xinterfaceA")) {
                System.out.println("drin");
            }

            Object setValue = null;
            if (getProp != null) {
                if (!utils.isVoid(getProp))
                    setValue = ValueChanger.changePValue(getProp);
                else log.println("Property '"+name+
                    "' is void but MAYBEVOID isn't set");
            } else log.println("Property '"+name+"'is null and can't be changed");
            if (name.equals("LineStyle")) setValue = null;
            if (setValue != null) {
                log.println("Setting to :"+setValue);
                oObj.setPropertyValue(name, setValue);
                hasChanged = (! getProp.equals(oObj.getPropertyValue(name)));
            } else log.println("Couldn't change Property '"+name+"'");
        } catch (com.sun.star.beans.PropertyVetoException e) {
            log.println("'" + name + "' throws exception '" + e + "'");
            e.printStackTrace(log);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("'" + name + "' throws exception '" + e + "'");
            e.printStackTrace(log);
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            log.println("'" + name + "' throws exception '" + e + "'");
            e.printStackTrace(log);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("'" + name + "' throws exception '" + e + "'");
            e.printStackTrace(log);
        } catch (com.sun.star.uno.RuntimeException e) {
            log.println("'" + name + "' throws exception '" + e + "'");
            e.printStackTrace(log);
        } catch (java.lang.ArrayIndexOutOfBoundsException e) {
            log.println("'" + name + "' throws exception '" + e + "'");
            e.printStackTrace(log);
        }

        return hasChanged;
    }

    /**
    * Forces environment recreation.
    */
    protected void after() {
        disposeEnvironment();
    }


}  // finish class _XPropertySet

