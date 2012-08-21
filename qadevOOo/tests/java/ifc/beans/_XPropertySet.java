/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package ifc.beans;

import java.util.Vector;

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
    }

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
    }

    private final XVetoableChangeListener VClistener = new MyVetoListener();

    /**
    * Structure that collects the properties of different types to test :
    * Constrained, Bound and Normal.
    */
    private final class PropsToTest {
        Vector< String > constrained = new Vector< String >();
        Vector< String > bound = new Vector< String >();
        Vector< String > normal = new Vector< String >();
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
            PTT.normal.clear();
            PTT.bound.clear();
            PTT.constrained.clear();
            PTT.normal.add( ptt[0] );
            PTT.bound.add( ptt[1] );
            PTT.constrained.add( ptt[2] );
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

        int count = PTT.bound.size();
        if ( count==0 || PTT.bound.get(0).equals("none") ) {
            log.println("*** No bound properties found ***");
            tRes.tested("addPropertyChangeListener()", true) ;
        } else {
            boolean error = false;
            for (int i = 0; i < count; i++) {
                String propertyName = PTT.bound.get(i);
                propertyChanged = false;
                try {
                    oObj.addPropertyChangeListener(propertyName,PClistener);
                    Object gValue = oObj.getPropertyValue(propertyName);
                    log.println("Check bound property: " + propertyName );
                    oObj.setPropertyValue(propertyName,
                        ValueChanger.changePValue(gValue));
                } catch (com.sun.star.beans.PropertyVetoException e) {
                    log.println("Exception occurred while trying to change "+
                        "property '"+ propertyName+"'");
                    e.printStackTrace(log);
                } catch (com.sun.star.lang.IllegalArgumentException e) {
                    log.println("Exception occurred while trying to change "+
                        "property '"+ propertyName+"'");
                    e.printStackTrace(log);
                } catch (com.sun.star.beans.UnknownPropertyException e) {
                    log.println("Exception occurred while trying to change "+
                        "property '"+ propertyName+"'");
                    e.printStackTrace(log);
                } catch (com.sun.star.lang.WrappedTargetException e) {
                    log.println("Exception occurred while trying to change "+
                        "property '"+ propertyName+"'");
                    e.printStackTrace(log);
                } // end of try-catch
                error = error || !propertyChanged;
                if (!propertyChanged) {
                    log.println("propertyChangeListener wasn't called for '"+
                        propertyName+"'");
                }
            }
            tRes.tested("addPropertyChangeListener()", !error);
        }

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

        int count = PTT.constrained.size();
        if ( count==0 || PTT.constrained.get(0).equals("none") ) {
            log.println("*** No constrained properties found ***");
            tRes.tested("addVetoableChangeListener()", true) ;
        } else {
            boolean error = false;
            for (int i = 0; i < count; i++) {
                String propertyName = PTT.constrained.get(i);
                vetoableChanged = false;
                try {
                    oObj.addVetoableChangeListener(propertyName,VClistener);
                    Object gValue = oObj.getPropertyValue(propertyName);
                    oObj.setPropertyValue(propertyName,
                        ValueChanger.changePValue(gValue));
                } catch (com.sun.star.beans.PropertyVetoException e) {
                    log.println("Exception occurred while trying to change "+
                        "property '"+ propertyName+"'");
                    e.printStackTrace(log);
                } catch (com.sun.star.lang.IllegalArgumentException e) {
                    log.println("Exception occurred while trying to change "+
                        "property '"+ propertyName+"'");
                    e.printStackTrace(log);
                } catch (com.sun.star.beans.UnknownPropertyException e) {
                    log.println("Exception occurred while trying to change "+
                        "property '"+ propertyName+"'");
                    e.printStackTrace(log);
                } catch (com.sun.star.lang.WrappedTargetException e) {
                    log.println("Exception occurred while trying to change "+
                        "property '"+ propertyName+"'");
                    e.printStackTrace(log);
                } // end of try-catch
                error = error || !vetoableChanged;
                if (!vetoableChanged) {
                    log.println("vetoableChangeListener wasn't called for '"+
                        propertyName+"'");
                }
            }
            tRes.tested("addVetoableChangeListener()",!error);
        }

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

        int count = PTT.normal.size();
        if ( count==0 || PTT.normal.get(0).equals("none") ) {
            log.println("*** No changeable properties found ***");
            tRes.tested("setPropertyValue()", true) ;
        } else {
            boolean error = false;
            for (int i = 0; i < count; i++) {
                String propertyName = PTT.normal.get(i);
                try {
                    log.println("try to change value of property '" + propertyName + "'" );
                    gValue = oObj.getPropertyValue(propertyName);
                    sValue = ValueChanger.changePValue(gValue);
                    oObj.setPropertyValue(propertyName, sValue);
                    sValue = oObj.getPropertyValue(propertyName);
                } catch (com.sun.star.beans.PropertyVetoException e) {
                    log.println("Exception occurred while trying to change "+
                        "property '"+ propertyName+"'");
                    e.printStackTrace(log);
                } catch (com.sun.star.lang.IllegalArgumentException e) {
                    log.println("Exception occurred while trying to change "+
                        "property '"+ propertyName+"'");
                    e.printStackTrace(log);
                } catch (com.sun.star.beans.UnknownPropertyException e) {
                    log.println("Exception occurred while trying to change "+
                        "property '"+ propertyName+"'");
                    e.printStackTrace(log);
                } catch (com.sun.star.lang.WrappedTargetException e) {
                    log.println("Exception occurred while trying to change "+
                        "property '"+ propertyName+"'");
                    e.printStackTrace(log);
                } // end of try-catch
                /*  this is stupid: we can't set properties whose semantics we
                 *  don't know to random values in an arbitrary order and
                 *  expect that to actually work.
                if( gValue.equals(sValue) )
                {
                    log.println("setting property '"+ propertyName+"' failed");
                    error = true;
                }
                */
            }
            tRes.tested("setPropertyValue()",!error);
        } //endif

        return;

    } // end of setPropertyValue()

    /**
    * Tests <code>getPropertyValue</code> method for the given property.
    * Returns true if no exceptions occurred
    */
    private boolean getSinglePropertyValue( String propertyName )
    {
        boolean runOk = false;
        try {
            oObj.getPropertyValue(propertyName);
            runOk = true;
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            log.println("Exception occurred while trying to get property '"+
                 propertyName+"'");
            e.printStackTrace(log);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Exception occurred while trying to get property '"+
                propertyName+"'");
            e.printStackTrace(log);
        }
        return runOk;
    }

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

        int count = PTT.normal.size();
        if ( count==0 || PTT.normal.get(0).equals("none") ) {
            Property[] properties = oObj.getPropertySetInfo().getProperties();
            if( properties.length > 0 ) {
                String propertyName = properties[0].Name;
                log.println("All properties are Read Only");
                log.println("Using: "+propertyName);
                tRes.tested("getPropertyValue()", getSinglePropertyValue( propertyName ) );
            }
            else {
                log.println("*** No properties found ***");
                tRes.tested("getPropertyValue()", true) ;
            }
        } else {
            boolean error = false;
            for (int i = 0; i < count; i++) {
                String propertyName = PTT.normal.get(i);
                boolean runOk = getSinglePropertyValue( propertyName );
                if( !runOk )
                {
                    error = true;
                    log.println("getPropertyValue() failed for property '"+propertyName+"'");
                }
            }
            tRes.tested("getPropertyValue()", !error) ;
        }

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

        int count = PTT.bound.size();
        if ( count==0 || PTT.bound.get(0).equals("none") ) {
            log.println("*** No bound properties found ***");
            tRes.tested("removePropertyChangeListener()", true) ;
        } else {

            //remove all listeners first
            for (int i = 0; i < count; i++) {
                String propertyName = PTT.bound.get(i);
                try {
                    oObj.removePropertyChangeListener(propertyName,PClistener);
                } catch (Exception e) {
                    log.println("Exception occurred while removing change listener from"+
                        "property '"+ propertyName+"'");
                    e.printStackTrace(log);
                }
            }

            boolean error = false;
            for (int i = 0; i < count; i++) {
                String propertyName = PTT.bound.get(i);
                try {
                    propertyChanged = false;
                    oObj.addPropertyChangeListener(propertyName,PClistener);
                    oObj.removePropertyChangeListener(propertyName,PClistener);
                    Object gValue = oObj.getPropertyValue(propertyName);
                    oObj.setPropertyValue(propertyName,
                        ValueChanger.changePValue(gValue));
                } catch (com.sun.star.beans.PropertyVetoException e) {
                    log.println("Exception occurred while trying to change "+
                        "property '"+ propertyName+"'");
                    e.printStackTrace(log);
                } catch (com.sun.star.lang.IllegalArgumentException e) {
                    log.println("Exception occurred while trying to change "+
                        "property '"+ propertyName+"'");
                    e.printStackTrace(log);
                } catch (com.sun.star.beans.UnknownPropertyException e) {
                    log.println("Exception occurred while trying to change "+
                        "property '"+ propertyName+"'");
                    e.printStackTrace(log);
                } catch (com.sun.star.lang.WrappedTargetException e) {
                    log.println("Exception occurred while trying to change "+
                        "property '"+ propertyName+"'");
                    e.printStackTrace(log);
                } // end of try-catch

                error = error || propertyChanged;
                if (propertyChanged) {
                    log.println("propertyChangeListener was called after removing"+
                        " for '"+propertyName+"'");
                }
            }
            tRes.tested("removePropertyChangeListener()",!error);
        }

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

        int count = PTT.constrained.size();
        if ( count==0 || PTT.constrained.get(0).equals("none") ) {
            log.println("*** No constrained properties found ***");
            tRes.tested("removeVetoableChangeListener()", true) ;
        } else {

            //remove all listeners first
            for (int i = 0; i < count; i++) {
                String propertyName = PTT.constrained.get(i);
                try {
                    oObj.removeVetoableChangeListener(propertyName,VClistener);
                } catch (Exception e) {
                    log.println("Exception occurred while removing veto listener from"+
                        "property '"+ propertyName+"'");
                    e.printStackTrace(log);
                }
            }

            boolean error = false;
            for (int i = 0; i < count; i++) {
                String propertyName = PTT.constrained.get(i);
                vetoableChanged = false;
                try {
                    oObj.addVetoableChangeListener(propertyName,VClistener);
                    oObj.removeVetoableChangeListener(propertyName,VClistener);
                    Object gValue = oObj.getPropertyValue(propertyName);
                    oObj.setPropertyValue(propertyName,
                        ValueChanger.changePValue(gValue));
                } catch (com.sun.star.beans.PropertyVetoException e) {
                    log.println("Exception occurred while trying to change "+
                        "property '"+ propertyName+"'");
                    e.printStackTrace(log);
                } catch (com.sun.star.lang.IllegalArgumentException e) {
                    log.println("Exception occurred while trying to change "+
                        "property '"+ propertyName+"'");
                    e.printStackTrace(log);
                } catch (com.sun.star.beans.UnknownPropertyException e) {
                    log.println("Exception occurred while trying to change "+
                        "property '"+ propertyName+"'");
                    e.printStackTrace(log);
                } catch (com.sun.star.lang.WrappedTargetException e) {
                    log.println("Exception occurred while trying to change "+
                        "property '"+ propertyName+"'");
                    e.printStackTrace(log);
                } // end of try-catch
                error = error || vetoableChanged;
                if (vetoableChanged) {
                    log.println("vetoableChangeListener was called after "+
                        "removing for '"+propertyName+"'");
                }
            }
            tRes.tested("removeVetoableChangeListener()",!error);
        }

        return;

    } // end of removeVetoableChangeListener()

    /**
    * Gets the properties being tested. Searches and stores by one
    * property of each kind (Bound, Vetoable, Normal).
    */
    public void getPropsToTest(XPropertySetInfo xPSI) {

        Property[] properties = xPSI.getProperties();
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
                PTT.bound.add(name);
            }

            if ( isWritable && isNotNull && isConstr && canChange) {
                PTT.constrained.add(name);
            }

            if ( isWritable && isNotNull && canChange) {
                PTT.normal.add(name);
            }


        } // endfor
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

