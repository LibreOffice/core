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

package ifc.container;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.awt.XControl;
import com.sun.star.awt.XControlContainer;
import com.sun.star.container.ContainerEvent;
import com.sun.star.container.XContainer;
import com.sun.star.container.XContainerListener;
import com.sun.star.container.XNameContainer;
import com.sun.star.container.XNameReplace;
import com.sun.star.lang.EventObject;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XNamingService;


/**
* Testing <code>com.sun.star.container.XContainer</code>
* interface methods :
* <ul>
*  <li><code> addContainerListener()</code></li>
*  <li><code> removeContainerListener()</code></li>
* </ul>
* This test needs the following object relations :
* <ul>
*  <li> <code>'INSTANCE'</code> : Object which can be inserted into
*    container.</li>
*  <li> <code>'INSTANCE2'</code> : <b>(optional)</b>
*     Object which can be inserted into container. The relation
*     must be specified when container cann't contain two
*     identical objects. Replaces the first instance.</li>
*  <li> <code>'XContainer.Container'</code> (of type
*  <code>com.sun.star.container.XNameContainer</code>)
*  <b>optional</b> : is required when the tested component
*  doesn't implement <code>XNameContainer</code> and is used
*  for adding and removing elements.</li>
* <ul> <p>
* Object <b>must implement</b>
* <code>com.sun.star.container.XNameContainer</code>.
* <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.container.XContainer
*/
public class _XContainer extends MultiMethodTest {

    public XContainer oObj = null;
    private boolean bElementInserted = false;
    private boolean bElementRemoved  = false;
    private boolean bElementReplaced = false;
    private XNameContainer NC = null ;
    private XControlContainer CC = null ;
    private XNamingService NV = null ;
    private XNameReplace NR = null ;
    private Object inst = null ;
    private Object inst2 = null ;

    /**
    * Retrieves object relations, and tries to query object for
    * <code>XNameContainer</code> interface.
    * @throws StatusException If one of relations not found or
    * object doesn't implement <code>XNameContainer</code> interface.
    */
    public void before() throws StatusException {
        // do this test with a different object
        Object altObj = tEnv.getObjRelation("XContainer.AlternateObject");
        if (altObj != null) {
            oObj = UnoRuntime.queryInterface(XContainer.class, altObj);
        }

        NC = UnoRuntime.queryInterface
            (XNameContainer.class, oObj) ;

        Object container = null;
        if (NC == null) {
            container = tEnv.getObjRelation("XContainer.Container") ;
        }

        if (container != null) {
            if (container instanceof com.sun.star.awt.XControlContainer) {
                CC = (XControlContainer) container;
            } else if (container instanceof com.sun.star.uno.XNamingService) {
                NV = (XNamingService) container;
            } else if (container instanceof com.sun.star.container.XNameReplace) {
                NR = (XNameReplace) container;
                inst2 = tEnv.getObjRelation("XContainer.NewValue");
                inst = tEnv.getObjRelation("XContainer.ElementName");
            } else if (container instanceof com.sun.star.container.XNameContainer) {
                NC = (XNameContainer) container;
            }
        }

        if (NC == null && CC == null && NV == null && NR == null)
            throw new StatusException(
                Status.failed("Neither object implements XNameContainer" +
                    " nor relation 'XContainer.Container' found.")) ;

        if (inst == null)
            inst = tEnv.getObjRelation("INSTANCE");
        if (inst == null) {
            log.println("No INSTANCE ObjRelation!!! ");
            throw new StatusException(Status.failed("No INSTANCE ObjRelation!!!")) ;
        }
        if (inst2 == null)
            inst2 = tEnv.getObjRelation("INSTANCE2");
    }

    /**
    * Listener implementation which just set flags on appropriate
    * events.
    */
    public class MyListener implements XContainerListener {
         public void elementInserted(ContainerEvent e) {
            //_log.println("Element was inserted");
            bElementInserted = true;
         }
         public void elementRemoved(ContainerEvent e) {
            //_log.println("Element was removed");
            bElementRemoved = true;
         }
         public void elementReplaced(ContainerEvent e) {
            //_log.println("Element was replaced");
            bElementReplaced = true;
         }
         public void disposing (EventObject obj) {}
    }

    MyListener listener = new MyListener();

    /**
    * Adds <code>MyListener</code> and performs all possible changes
    * (insert, replace, remove) with container. The checks which
    * events were called. <p>
    * Has <b>OK</b> status if all methods of the listener were called.
    */
    public void _addContainerListener() {
        boolean bResult = true;

        oObj.addContainerListener(listener);
        bResult &= performChanges();
        //we can't replace if the container is XControlContainer
        if (NC != null) bResult &= bElementReplaced;
        // we do not remove and insert if the listener is triggered by XNameReplace
        if (NR == null) bResult &= bElementRemoved;
        if (NR == null) bResult &= bElementInserted;

        if (!bResult) {
            log.println("inserted was " + (bElementInserted ? "" : "NOT")
                                                                + " called.");

            if (NC != null) {
                log.println("replaced was " + (bElementReplaced ? "" : "NOT")
                                                                    + " called.");
            }
            log.println("removed was " + (bElementRemoved ? "" : "NOT")
                                                                + " called.");
        }

        tRes.tested("addContainerListener()", bResult);
    }

    /**
    * Removes listener added before and performs all possible changes
    * (insert, replace, remove) with container. The checks which
    * events were called. <p>
    * Has <b>OK</b> status if no methods of the listener were called. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> addContainerListener() </code> : to remove it now. </li>
    * </ul>
    */
    public void _removeContainerListener() {
        requiredMethod("addContainerListener()") ;

        boolean bResult = true;
        bElementReplaced = bElementRemoved = bElementInserted = false;

        oObj.removeContainerListener(listener);
        bResult &= performChanges();
        bResult &= !bElementReplaced;
        bResult &= !bElementRemoved;
        bResult &= !bElementInserted;

        tRes.tested("removeContainerListener()", bResult);
    }

    /**
    * Inserts, replaces and finally removes object from container.
    * Object is gotten from <code>'INSTANCE'</code> relation. If
    * the relation <code>'INSTANCE2'</code> exists then the first
    * instance is replaced with second.
    */
    protected boolean performChanges() {
        if (CC != null) return performChanges2();
        if (NV != null) return performChanges3();
        if (NR != null) return performChanges4();
        boolean bResult = true;
        try {
            String[] names = NC.getElementNames();
            log.println("Elements count = " + names.length);
            NC.insertByName("XContainer_dummy", inst);
            names = NC.getElementNames();
            log.println("Elements count = " + names.length);
            if (inst2 == null) {
                NC.replaceByName("XContainer_dummy", inst);
            } else {
                NC.replaceByName("XContainer_dummy", inst2);
            }
            NC.removeByName("XContainer_dummy");
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("Exception occurred ");
            ex.printStackTrace(log);
            bResult = false;
        } catch (com.sun.star.lang.WrappedTargetException ex) {
            log.println("Exception occurred ");
            ex.printStackTrace(log);
            bResult = false;
        } catch (com.sun.star.container.NoSuchElementException ex) {
            log.println("Exception occurred ");
            ex.printStackTrace(log);
            bResult = false;
        } catch (com.sun.star.container.ElementExistException ex) {
            log.println("Exception occurred ");
            ex.printStackTrace(log);
            bResult = false;
        }

        return bResult;
    }

    /**
    * In case no XNameContainer is available, but a XControlContainer
    * instead.
    * the XControl instance is inserted
    * Method returns true if the count of Controls is changed afterwards
    */
    protected boolean performChanges2() {
        int precount = CC.getControls().length;
        CC.addControl("NewControl",(XControl) inst);
        shortWait();
        int count = CC.getControls().length;
        CC.removeControl(CC.getControl("NewControl"));
        shortWait();
        return count>precount;
    }

    /**
    * In case no XNameContainer is available, but a XNamingService
    * instead.
    * the instance is registered and revoked again
    * Method return true if getRegisteredObject() works after
    * registering and doesn't after revoke
    */
    protected boolean performChanges3() {
        boolean res = true;
        Object reg = null;

        try {
            reg = NV.getRegisteredObject("MyFactory");
            NV.revokeObject("MyFactory");
        } catch (Exception e) {

        }

        try {
            NV.registerObject("MyFactory", inst);
            reg = NV.getRegisteredObject("MyFactory");
            res &= (reg != null);
        } catch (Exception e) {
            e.printStackTrace(log);
            log.println("registerObject failed");
            res &= false;
        }

        try {
            NV.revokeObject("MyFactory");
            reg = NV.getRegisteredObject("MyFactory");
            log.println("revokeObject failed");
            res &= false;
        } catch (Exception e) {
            res &= true;
        }

        return res;
    }

    /**
    * In case no XNameContainer is available, but a XNamingReplace
    * instead.
    */
    protected boolean performChanges4() {
        boolean res = true;
        Object newValue = inst2;
        Object originalValue = null;
        String name = null;

        try {
            name = (String)inst;
        }
        catch(java.lang.ClassCastException e) {
            log.write("Expected String as object relations 'XContainer.ElementName'.");
            e.printStackTrace(log);
            return false;
        }

        try {
            originalValue = NR.getByName(name);
            NR.replaceByName(name, newValue);
        } catch (Exception e) {
            e.printStackTrace(log);
            res = false;
        }

        try {
            NR.replaceByName(name, originalValue);
        } catch (Exception e) {
            e.printStackTrace(log);
            res = false;
        }

        return res;
    }

    /**
    * Sleeps for 0.5 sec. to allow StarOffice to react on <code>
    * reset</code> call.
    */
    private void shortWait() {
        try {
            Thread.sleep(1000) ;
        } catch (InterruptedException e) {
            log.println("While waiting :" + e) ;
        }
    }
}


