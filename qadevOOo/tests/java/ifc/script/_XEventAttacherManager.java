/*************************************************************************
 *
 *  $RCSfile: _XEventAttacherManager.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 10:49:57 $
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

    int index;

    /**
    * Test calls the method and stores index of new entry. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    */
    public void _insertEntry() {
        index = 0;
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
    * Test creates instance of <code>TypeDescriptionProvider</code>,
    * stores it and attaches it to the entry with index stored in the method
    * <code>insertEntry()</code>. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> insertEntry() </code> : to have entry's index for attach</li>
    * @see com.sun.star.reflection.TypeDescriptionProvider
    */
    public void _attach() {
        requiredMethod("insertEntry()");

        try {
            XMultiServiceFactory xMSF = (XMultiServiceFactory)tParam.getMSF();
            attachedObject = xMSF.createInstance
                ( "com.sun.star.reflection.TypeDescriptionProvider" );
        } catch( com.sun.star.uno.Exception e ) {
            log.println("Service not available" );
            e.printStackTrace(log);
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
    * stores it and addes this scripts listener. <p>
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
    class MyScriptListener implements XScriptListener {
        public void firing(ScriptEvent evt) {
        }

        public Object approveFiring(ScriptEvent evt) {
            return evt.Helper;
        }

        public void disposing(EventObject evt) {
        }
    }

}

