/*************************************************************************
 *
 *  $RCSfile: _XMultiPropertySet.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:15:19 $
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

package ifc.beans;

import java.io.PrintWriter;
import java.util.HashSet;
import java.util.Set;
import java.util.StringTokenizer;

import lib.MultiMethodTest;
import lib.Status;
import util.ValueChanger;

import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyAttribute;
import com.sun.star.beans.PropertyChangeEvent;
import com.sun.star.beans.XMultiPropertySet;
import com.sun.star.beans.XPropertiesChangeListener;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.lang.EventObject;


/**
* Testing <code>com.sun.star.beans.XMultiPropertySet</code>
* interface methods :
* <ul>
*  <li><code> getPropertySetInfo()</code></li>
*  <li><code> setPropertyValues()</code></li>
*  <li><code> getPropertyValues()</code></li>
*  <li><code> addPropertiesChangeListener()</code></li>
*  <li><code> removePropertiesChangeListener()</code></li>
*  <li><code> firePropertiesChangeEvent()</code></li>
* </ul> <p>
*
* Required relations :
* <ul>
*  <li> <code>'XMultiPropertySet.ExcludeProps'</code>
*    <b>(optional) </b> : java.util.Set.
*    Has property names which must be skipped from testing in
*    some reasons (for example property accepts restricted set
*    of values).
*  </li>
* <ul> <p>
*
* Test is <b> NOT </b> multithread compilant. <p>
* After test completion object environment has to be recreated.
* @see com.sun.star.beans.XMultiPropertySet
*/
public class _XMultiPropertySet extends MultiMethodTest {

    public XMultiPropertySet oObj = null;

    private boolean propertiesChanged = false;
    private XPropertySetInfo propertySetInfo = null;
    private String [] testPropsNames = null;
    private int testPropsAmount = 0;
    private PrintWriter _log = null;

    private Object[] values = null;

    private Set exclProps = null;

    /**
    * Initializes some fields.
    */
    public void before() {
        _log = log;

        exclProps = (Set) tEnv.getObjRelation("XMultiPropertySet.ExcludeProps");
        if (exclProps == null) exclProps = new HashSet(0);
    }

    /**
    * Listener implementation which sets a flag when
    * listener was called.
    */
    public class MyChangeListener implements XPropertiesChangeListener {
         public void propertiesChange(PropertyChangeEvent[] e) {
             //_log.println("Listener was called");
             propertiesChanged = true;
         }
         public void disposing (EventObject obj) {}
    };

    private XPropertiesChangeListener PClistener =
        new MyChangeListener();

    /**
    * Test calls the method and checks return value.
    * <code>PropertySetInfo</code> object is stored<p>
    * Has <b> OK </b> status if the method returns not null value
    * and no exceptions were thrown. <p>
    */
    public void _getPropertySetInfo() {
        boolean bResult = true;
        propertySetInfo = oObj.getPropertySetInfo();

        if (propertySetInfo == null) {
            log.println("getPropertySetInfo() method returned null");
            bResult = false;
        }

        tRes.tested("getPropertySetInfo()", bResult) ;
    }


    /**
    * Test collects all property names and retrieves their values,
    * then checks the value returned. Finally it also collects
    * bound properties for other methods tests.<p>
    * Has <b> OK </b> status if the method  returns non null value
    * and no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getPropertySetInfo() </code> : to have a list
    *   of properties.</li>
    * </ul>
    */
    public void _getPropertyValues() {
        requiredMethod("getPropertySetInfo()");
        boolean bResult = true;

        Property[] properties = propertySetInfo.getProperties();
        String[] allnames = new String[properties.length];
        for (int i = 0; i < properties.length; i++) {
            allnames[i] = properties[i].Name;
        }

        values = oObj.getPropertyValues(allnames);

        bResult &= values!=null;
        tRes.tested("getPropertyValues()", bResult) ;

        getPropsToTest(properties);
    }

    /**
    * Test adds listener for all bound properties then each property
    * is changed and listener call . <p>
    * Has <b> OK </b> status if on each property change the listener was
    * called and no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getPropertyValues() </code> : to collect bound
    *   properties.</li>
    * </ul>
    */
    public void _addPropertiesChangeListener() {

        requiredMethod("getPropertyValues()");

        boolean result  = true ;
        // Creating listener
        oObj.addPropertiesChangeListener(testPropsNames, PClistener);

        if ((testPropsAmount==1) && (testPropsNames[0].equals("none"))) {
            testPropsAmount = 0;
        }


        // Change one of the property to be sure, that this event was cauched.
        //Random rnd = new Random();
        //int idx = rnd.nextInt(testPropsAmount);
        for (int i=0; i<testPropsAmount;i++) {
            log.print("Trying to change property " + testPropsNames[i]);
            try {
                Object[] gValues = oObj.getPropertyValues(testPropsNames);
                Object newValue = ValueChanger.changePValue(gValues[i]);
                gValues[i] = newValue;
                propertiesChanged = false;
                oObj.setPropertyValues(testPropsNames, gValues);
                waitAMoment() ;
                result &= propertiesChanged ;
                log.println(" ... done");
            } catch (com.sun.star.beans.PropertyVetoException e) {
                log.println("Exception occured while trying to change "+
                    "property '"+testPropsNames[i] + "' :" + e);
                e.printStackTrace(log);
            } catch (com.sun.star.lang.IllegalArgumentException e) {
                log.println("Exception occured while trying to change "+
                    "property '"+testPropsNames[i] + "' :" + e);
                e.printStackTrace(log);
            } catch (com.sun.star.lang.WrappedTargetException e) {
                log.println("Exception occured while trying to change "+
                    "property '"+testPropsNames[i] + "' :" + e);
                e.printStackTrace(log);
            } // end of try-catch
        }
        if (testPropsAmount == 0) {
            log.println("all properties are read only");
            tRes.tested("addPropertiesChangeListener()", Status.skipped(true));
        } else {
            tRes.tested("addPropertiesChangeListener()", propertiesChanged);
        }
    }

    /**
    * Calls method and check if listener was called. <p>
    * Has <b> OK </b> status if the listener was
    * called and no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> addPropertiesChangeListener() </code> : listener to
    *    be added.</li>
    * </ul>
    */
    public void _firePropertiesChangeEvent() {
        requiredMethod("addPropertiesChangeListener()");
        propertiesChanged = false ;

        oObj.firePropertiesChangeEvent(testPropsNames, PClistener);
        waitAMoment() ;

        tRes.tested("firePropertiesChangeEvent()", propertiesChanged);
    }


    /**
    * Removes listener added before. <p>
    * Has <b> OK </b> status no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> addPropertiesChangeListener() </code> : listener to
    *    be added.</li>
    * </ul>
    */
    public void _removePropertiesChangeListener() {
        requiredMethod("firePropertiesChangeEvent()");
        boolean bResult = true;

        oObj.removePropertiesChangeListener(PClistener);

        tRes.tested("removePropertiesChangeListener()", bResult);
    }


    /**
    * Changes all properties, then set them to new values, get them
    * and checks if their values were changed properly. <p>
    * Has <b> OK </b> status if all properties properly changed
    * and no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getPropertyValues() </code> : to collect bound
    *   properties.</li>
    * </ul>
    */
    public void _setPropertyValues() {
        requiredMethod("getPropertyValues()");
        boolean bResult = true;

        if ((testPropsNames.length==1)&&(testPropsNames[0].equals("none"))) {
            log.println("all properties are readOnly");
            tRes.tested("setPropertyValues()",Status.skipped(true));
            return;
        }

        log.println("Changing all properties");
        Object[] gValues = oObj.getPropertyValues(testPropsNames);
        for (int i=0; i<testPropsAmount;i++) {
            Object oldValue = gValues[i];
            Object newValue = ValueChanger.changePValue(oldValue);
            gValues[i] = newValue;
        }

        try {
            oObj.setPropertyValues(testPropsNames, gValues);
            Object[] newValues = oObj.getPropertyValues(testPropsNames);
            for (int i=0; i<testPropsAmount;i++) {
                if (newValues[i].equals(gValues[i])) {
                    bResult = true;
                }
            }
        } catch (com.sun.star.beans.PropertyVetoException e) {
            log.println("Exception occured while setting properties");
            e.printStackTrace(log);
            bResult = false;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Exception occured while setting properties");
            e.printStackTrace(log);
            bResult = false;
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Exception occured while setting properties");
            e.printStackTrace(log);
            bResult = false;
        } // end of try-catch

        tRes.tested("setPropertyValues()", bResult);
    }

    //Get the properties being tested
    private void getPropsToTest(Property[] properties) {

        String bound = "";

        for (int i = 0; i < properties.length; i++) {

            Property property = properties[i];
            String name = property.Name;
            boolean isWritable = ((property.Attributes &
                PropertyAttribute.READONLY) == 0);
            boolean isNotNull = ((property.Attributes &
                PropertyAttribute.MAYBEVOID) == 0);
            boolean isBound = ((property.Attributes &
                PropertyAttribute.BOUND) != 0);
            boolean isExcluded = exclProps.contains(name);

            if ( isWritable && isNotNull && isBound && !isExcluded) {
                bound+=name+";";
            }

        } // endfor

        //get a array of bound properties
        if (bound.equals("")) bound = "none";
        StringTokenizer ST=new StringTokenizer(bound,";");
        int nr = ST.countTokens();
        testPropsNames = new String[nr];
        for (int i=0; i<nr; i++) testPropsNames[i] = ST.nextToken();
        testPropsAmount = nr;
        return;

    }

    /**
    * Waits some time for listener to be called.
    */
    private void waitAMoment() {
        try {
            Thread.sleep(200) ;
        } catch (java.lang.InterruptedException e) {
            log.println("!!! Exception while waiting !!!") ;
        }
    }

    /*
    * Does nothing.
    */
    protected void after() {
        disposeEnvironment();
    }
}


