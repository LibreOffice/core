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

package ifc.script;

import lib.MultiMethodTest;

import com.sun.star.lang.EventObject;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.script.ScriptEvent;
import com.sun.star.script.ScriptEventDescriptor;
import com.sun.star.script.XEventAttacherManager;
import com.sun.star.script.XScriptListener;

/**
* Testing <code>com.sun.star.script.XEventAttacherManager</code>
* interface methods :
* <ul>
*  <li><code> registerScriptEvent()</code></li>
*  <li><code> registerScriptEvents()</code></li>
*  <li><code> revokeScriptEvent()</code></li>
*  <li><code> revokeScriptEvents()</code></li>
*  <li><code> insertEntry()</code></li>
*  <li><code> removeEntry()</code></li>
*  <li><code> getScriptEvents()</code></li>
*  <li><code> attach()</code></li>
*  <li><code> detach()</code></li>
*  <li><code> addScriptListener()</code></li>
*  <li><code> removeScriptListener()</code></li>
* </ul> <p>
* @see com.sun.star.script.XEventAttacherManager
*/
public class _XEventAttacherManager extends MultiMethodTest {

    /**
     * oObj filled by MultiMethodTest
     */
    public XEventAttacherManager oObj = null;

    private static final int index = 0;

    /**
    * Test calls the method and stores index of new entry. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    */
    public void _insertEntry() {
        try {
            oObj.insertEntry(index);
            tRes.tested("insertEntry()", true);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("insertEntry(" + index
                    + ") throws unexpected exception "
                    + e.getMessage());
            e.printStackTrace(log);
            tRes.tested("insertEntry()", false);
        }
    }

    ScriptEventDescriptor desc;

    /**
    * Test creates <code>ScriptEventDescriptor</code>, registers
    * the script event and stores the descriptor. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> insertEntry() </code> : to have entry's index</li>
    * </ul>
    * @see com.sun.star.script.ScriptEventDescriptor
    */
    public void _registerScriptEvent() {
        requiredMethod("insertEntry()");
        desc = new ScriptEventDescriptor(
                    "XEventListener1",
                    "disposing", "", "Basic", "");

        try {
            oObj.registerScriptEvent(index, desc);
            tRes.tested("registerScriptEvent()", true);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("registerScriptEvent() throws unexpected exception "
                    + e.getMessage());
            e.printStackTrace(log);
            tRes.tested("registerScriptEvent()", false);
        }
    }

    ScriptEventDescriptor descs[];

    /**
    * Test creates array of <code>ScriptEventDescriptor</code>, registers
    * this script events and stores the descriptors. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> insertEntry() </code> : to have entry's index</li>
    * </ul>
    * @see com.sun.star.script.ScriptEventDescriptor
    */
    public void _registerScriptEvents() {
        requiredMethod("insertEntry()");
        descs = new ScriptEventDescriptor[] {
            new ScriptEventDescriptor(
                    "XEventListener2",
                    "disposing", "", "Basic", ""),
            new ScriptEventDescriptor(
                    "XEventListener3",
                    "disposing", "", "Basic", "")
        };

        try {
            oObj.registerScriptEvents(index, descs);
            tRes.tested("registerScriptEvents()", true);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("registerScriptEvents() throws unexpected exception "
                    + e.getMessage());
            e.printStackTrace(log);
            tRes.tested("registerScriptEvents()", false);
        }
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned array of descriptors contains
    * array of descriptors registered by methods <code>registerScriptEvents</code>
    * and <code>registerScriptEvent</code> and no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> registerScriptEvent() </code> :
    *   to have registered descriptor </li>
    *  <li> <code> registerScriptEvents() </code> :
    *   to have registered descriptors </li>
    * </ul>
    */
    public void _getScriptEvents() {
        requiredMethod("registerScriptEvent()");
        requiredMethod("registerScriptEvents()");

        ScriptEventDescriptor[] res;

        try {
            res = oObj.getScriptEvents(index);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("registerScriptEvents() throws unexpected exception "
                    + e.getMessage());
            e.printStackTrace(log);
            tRes.tested("registerScriptEvents()", false);
            return;
        }

        // checking the desc and descs are in script events
        tRes.tested("getScriptEvents()",
                contains(res, desc) && containsArray(res, descs));

        log.println("Script events :") ;
        printEvents(res) ;
    }

    /**
     * Method checks that array of descriptors contains the concrete desciptor.
     * @param container the array of descriptors
     * @param evt the descriptor which presence in the array is checked
     * @return true if the descriptor presence in the array
     */
    boolean contains(ScriptEventDescriptor[] container,
            ScriptEventDescriptor evt) {
        for (int i = 0; i < container.length; i++) {
            if (equal(container[i], evt)) {
                return true;
            }
        }

        return false;
    }

    /**
     * Method checks that one array of descriptors contains
     * another array of descriptors.
     * @param container the array of descriptors
     * @param events the array of descriptors which presence
     * in array <code>container</code> is checked
     * @return true if the array <code>events</code> contains in the array
     * <code>container</code>
     */
    boolean containsArray(ScriptEventDescriptor[] container,
            ScriptEventDescriptor[] events) {
        for (int i = 0; i < events.length; i++) {
            if (!contains(container, events[i])) {
                return false;
            }
        }

        return true;
    }

    /**
     * Compares descriptor <code>evt1</code> to descriptor <code>evt2</code>.
     * Two descriptors are considered equal if all their fields are equal.
     * @return true if the argument is not <code>null</code> and
     * the descriptors are equal; false otherwise
     */
    boolean equal(ScriptEventDescriptor evt1,
            ScriptEventDescriptor evt2) {
        return evt1.ListenerType.equals(evt2.ListenerType)
            && evt1.EventMethod.equals(evt2.EventMethod)
            && evt1.ScriptType.equals(evt2.ScriptType)
            && evt1.ScriptCode.equals(evt2.ScriptCode)
            && evt1.AddListenerParam.equals(evt2.AddListenerParam);
    }

    /**
     * Prints fields of descriptor <code>evt</code> to log.
     * @param evt the descriptor that needs to be printed to log
     */
    void printEvent(ScriptEventDescriptor evt) {
        if (evt == null) {
            log.println("null");
        } else {
            log.println("\"" + evt.ListenerType + "\",\""
                     + evt.EventMethod + "\",\""
                     + evt.ScriptType + "\",\""
                     + evt.ScriptCode + "\",\""
                     + evt.AddListenerParam + "\"");
        }
    }

    /**
     * Prints the descriptors to log.
     * @param events the array of descriptors that need to be printed to log
     */
    void printEvents(ScriptEventDescriptor events[]) {
        if (events == null) {
            log.println("null");
        } else {
            for (int i = 0; i < events.length; i++) {
                printEvent(events[i]);
            }
        }
    }

    Object attachedObject;

    /**
    * Test creates instance of <code>NamingService</code> (arbitrarily),
    * stores it and attaches it to the entry with index stored in the method
    * <code>insertEntry()</code>. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> insertEntry() </code> : to have entry's index for attach</li>
    */
    public void _attach() {
        requiredMethod("insertEntry()");

        try {
            XMultiServiceFactory xMSF = tParam.getMSF();
            attachedObject = xMSF.createInstance
                ( "com.sun.star.uno.NamingService" );
        } catch( com.sun.star.uno.Exception e ) {
            log.println("com.sun.star.uno.NamingService not available" );
            e.printStackTrace(log);
            tRes.tested("attach()", false);
            return;
        }
        if (attachedObject == null) {
            log.println("com.sun.star.uno.NamingService not available" );
            tRes.tested("attach()", false);
            return;
        }

        try {
            oObj.attach(index, attachedObject, "param");
            tRes.tested("attach()", true);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("attach() throws exception "
                    + e.getMessage());
            e.printStackTrace(log);
            tRes.tested("attach()", false);
        } catch (com.sun.star.lang.ServiceNotRegisteredException e) {
            log.println("attach() throws exception "
                    + e.getMessage());
            e.printStackTrace(log);
            tRes.tested("attach()", false);
        }
    }

    /**
    * Test calls the method for the object that was stored in the method
    * <code>attach()</code>. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> attach() </code> : to have attached object </li>
    * </ul>
    */
    public void _detach() {
        requiredMethod("attach()");

        try {
            oObj.detach(index, attachedObject);
            tRes.tested("detach()", true);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("detach() throws unexpected exception "
                    + e.getMessage());
            e.printStackTrace(log);
            tRes.tested("detach()", false);
        }
    }

    /**
    * Test revokes script event that was registered by method
    * <code>registerScriptEvent()</code> and checks that the description
    * was removed. <p>
    * Has <b> OK </b> status if description was successfully removed. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> registerScriptEvent() </code> :
    *   to have registered descriptor </li>
    * </ul>
    * The following method tests are to be executed before :
    * <ul>
    *  <li> <code> getScriptEvents() </code> :
    *   this method must be executed first </li>
    * </ul>
    */
    public void _revokeScriptEvent() {
        requiredMethod("registerScriptEvent()");
        executeMethod("getScriptEvents()") ;

        try {
            oObj.revokeScriptEvent(index, desc.ListenerType,
                    desc.EventMethod, "");

            ScriptEventDescriptor[] res = oObj.getScriptEvents(index);
            // checking that the desc has been removed
            tRes.tested("revokeScriptEvent()", !contains(res, desc));
            printEvents(res) ;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("revokeScriptEvent() throws unexpected exception "
                    + e.getMessage());
            e.printStackTrace(log);
            tRes.tested("revokeScriptEvent()", false);
        }
    }

    /**
    * Test revokes script events that was registered by method
    * <code>registerScriptEvents()</code> and checks that the descriptions
    * were removed. <p>
    * Has <b> OK </b> status if descriptions were successfully removed. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> revokeScriptEvent() </code> :
    *   this method must be executed first </li>
    * </ul>
    * The following method tests are to be executed before :
    * <ul>
    *  <li> <code> getScriptEvents() </code> :
    *   this method must be executed first </li>
    * </ul>
    */
    public void _revokeScriptEvents() {
        requiredMethod("revokeScriptEvent()");
        executeMethod("getScriptEvents()") ;

        try {
            oObj.revokeScriptEvents(index);

            ScriptEventDescriptor[] res = oObj.getScriptEvents(index);
            // checking that all events have been removed
            tRes.tested("revokeScriptEvents()",
                    res == null || res.length == 0);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("revokeScriptEvents() throws unexpected exception "
                    + e.getMessage());
            e.printStackTrace(log);
            tRes.tested("revokeScriptEvents()", false);
        }
    }

    /**
    * Test calls the method with entry's index that was stored in method
    * <code>insertEntry()</code>. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> insertEntry() </code> :
    *  to have entry's index </li>
    */
    public void _removeEntry() {
        requiredMethod("insertEntry()");
        try {
            oObj.removeEntry(index);
            tRes.tested("removeEntry()", true);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("removeEntry(" + index
                    + ") throws unexpected exception "
                    + e.getMessage());
            e.printStackTrace(log);
            tRes.tested("removeEntry()", false);
        }
    }

    XScriptListener listener;

    /**
    * Test creates object that supports interface <code>XScriptListener</code>,
    * stores it and adds this scripts listener. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    * @see com.sun.star.script.XScriptListener
    */
    public void _addScriptListener() {
        listener = new MyScriptListener();

        try {
            oObj.addScriptListener(listener);
            tRes.tested("addScriptListener()", true);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("addScriptListener() throws unexpected exception "
                    + e.getMessage());
            e.printStackTrace(log);
            tRes.tested("addScriptListener()", false);
        }
    }

    /**
    * Test removes script listener that was stored in method
    * <code>addScriptListener()</code>. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> addScriptListener() </code> :
    *  to have script listener </li>
    * </ul>
    */
    public void _removeScriptListener() {
        requiredMethod("addScriptListener()");

        try {
            oObj.removeScriptListener(listener);
            tRes.tested("removeScriptListener()", true);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("removeScriptListener() throws unexpected exception "
                    + e.getMessage());
            e.printStackTrace(log);
            tRes.tested("removeScriptListener()", false);
        }
    }

    /**
     * Class implement interface <code>XScriptListener</code>
     * for test of the method <code>addScriptListener()</code>.
     * No functionality implemented.
     * @see com.sun.star.script.XScriptListener
     */
    static class MyScriptListener implements XScriptListener {
        public void firing(ScriptEvent evt) {
        }

        public Object approveFiring(ScriptEvent evt) {
            return evt.Helper;
        }

        public void disposing(EventObject evt) {
        }
    }

}

