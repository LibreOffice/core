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

package ifc.lang;

import lib.MultiMethodTest;

import com.sun.star.frame.XDesktop;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XEventListener;

/**
* Testing <code>com.sun.star.lang.XComponent</code>
* interface methods :
* <ul>
*  <li><code> dispose()</code></li>
*  <li><code> addEventListener()</code></li>
*  <li><code> removeEventListener()</code></li>
* </ul>
* After this interface test object <b>must be recreated</b>. <p>
* Multithreaded test ability <b>not implemented</b> yet.
* @see com.sun.star.lang.XComponent
*/
public class _XComponent extends MultiMethodTest {

    public static XComponent oObj = null;
    private XComponent altDispose = null;

    boolean listenerDisposed[] = new boolean[2];
    String[] Loutput = new String[2];

    /**
    * Listener which added but not removed, and its method must be called
    * on <code>dispose</code> call.
    */
    public class MyEventListener implements XEventListener {
        public void disposing ( EventObject oEvent ) {
            Loutput[0] = Thread.currentThread() + " is DISPOSING EV1" + this;
            listenerDisposed[0] = true;
        }
    }

    /**
    * Listener which added and then removed, and its method must <b>not</b>
    * be called on <code>dispose</code> call.
    */
    public class MyEventListener2 implements XEventListener {
        public void disposing ( EventObject oEvent ) {
            Loutput[0] = Thread.currentThread() + " is DISPOSING EV2" + this;
            listenerDisposed[1] = true;
        }
    }

    XEventListener listener1 = new MyEventListener();
    XEventListener listener2 = new MyEventListener2();

    /**
     * For the (no longer existing) cfgmgr2.OSetElement tests: dispose the owner
     * element.  TODO: Is this needed for anything else, too, or should it be
     * removed?
     */
    protected void before() {
        // do not dispose this component, but parent instead
        altDispose = (XComponent)tEnv.getObjRelation("XComponent.DisposeThis");

    }

    /**
    * Adds two listeners. <p>
    * Has OK status if then the first listener will receive an event
    * on <code>dispose</code> method call.
    */
    public void _addEventListener() {

        listenerDisposed[0] = false;
        listenerDisposed[1] = false;

        oObj.addEventListener( listener1 );
        oObj.addEventListener( listener2 );

        return;
    } // finished _addEventListener()

    /**
    * Removes the second of two added listeners. <p>
    * Method tests to be completed successfully :
    * <ul>
    * <li> <code>addEventListener</code> : method must add two listeners. </li>
    * </ul> <p>
    * Has OK status if no events will be sent to the second listener on
    * <code>dispose</code> method call.
    */
    public void _removeEventListener() {
        executeMethod("addEventListener()");
        if (disposed) return;
        // the second listener should not be called
        oObj.removeEventListener( listener2 );
        log.println(Thread.currentThread() + " is removing EL " + listener2);
    } // finished _removeEventListener()

    static boolean disposed = false;

    /**
    * Disposes the object and then check appropriate listeners were
    * called or not. <p>
    * Method tests to be completed successfully :
    * <ul>
    * <li> <code>removeEventListener</code> : method must remove one of two
    *    listeners. </li>
    * </ul> <p>
    * Has OK status if liseter removed wasn't called and other listener
    * was.
    */
    public void _dispose() {
        disposed = false;
        executeMethod("removeEventListener()");

        log.println( "begin dispose in thread " + Thread.currentThread());
        XDesktop oDesk = (XDesktop) tEnv.getObjRelation("Desktop");
        if (oDesk !=null) {
            oDesk.terminate();
        }
        else {
            if (altDispose == null)
                oObj.dispose();
            else
                altDispose.dispose();
        }

        try {
            Thread.sleep(500) ;
        } catch (InterruptedException e) {}
        if (Loutput[0]!=null) log.println(Loutput[0]);
        if (Loutput[1]!=null) log.println(Loutput[1]);
        log.println( "end dispose" + Thread.currentThread());
        disposed = true;

        // check that dispose() works OK.
        tRes.tested("addEventListener()", listenerDisposed[0]);
        tRes.tested("removeEventListener()", !listenerDisposed[1]);
        tRes.tested("dispose()", listenerDisposed[0] && !listenerDisposed[1]);

    } // finished _dispose()

    /**
    * Forces object recreation.
    */
    protected void after() {
        disposeEnvironment();
    }

} // finished class _XComponent


