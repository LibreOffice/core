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

package complex.imageManager;

import com.sun.star.container.XNameContainer;
import com.sun.star.frame.XDesktop;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XEventListener;
import lib.TestParameters;

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
public class _XComponent {

    public static XComponent oObj = null;
    private XComponent altDispose = null;
    TestParameters tEnv = null;
    boolean listenerDisposed[] = new boolean[2];
    String[] Loutput = new String[2];

    /**
    * Listener which added but not removed, and its method must be called
    * on <code>dispose</code> call.
    */
    public class MyEventListener implements XEventListener {
        int number = 0;
        String message = null;
        public MyEventListener(int number, String message) {
            this.message = message;
            this.number = number;
        }
        public void disposing ( EventObject oEvent ) {
            Loutput[number] = Thread.currentThread() + " is DISPOSING " + message + this;
            listenerDisposed[number] = true;
        }
    };

    XEventListener listener1 = new MyEventListener(0, "EV1");
    XEventListener listener2 = new MyEventListener(1, "EV2");

    public _XComponent(TestParameters tEnv, XComponent oObj) {
        this.tEnv = tEnv;
        this.oObj = oObj;
    }

    /**
     * For the cfgmgr2.OSetElement tests: dispose the owner element.
     */
    public void before() {
        // do not dispose this component, but parent instead
        altDispose = (XComponent)tEnv.get("XComponent.DisposeThis");

    }

    /**
    * Adds two listeners. <p>
    * Has OK status if then the first listener will receive an event
    * on <code>dispose</code> method call.
    */
    public boolean _addEventListener() {

        listenerDisposed[0] = false;
        listenerDisposed[1] = false;

        oObj.addEventListener( listener1 );
        oObj.addEventListener( listener2 );

        return true;
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
    public boolean _removeEventListener() {
        if (disposed)
        {
            System.out.println("Hint: already disposed.");
            return false;
        }
        // the second listener should not be called
        oObj.removeEventListener( listener2 );
        System.out.println(Thread.currentThread() + " is removing EL " + listener2);
        return true;
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
    public boolean _dispose() {
        disposed = false;

        System.out.println( "begin dispose" + Thread.currentThread());
        XDesktop oDesk = (XDesktop) tEnv.get("Desktop");
        if (oDesk !=null) {
            oDesk.terminate();
        }
        else {
            if (altDispose == null)
            {
                oObj.dispose();
            }
            else
            {
                altDispose.dispose();
            }
        }

        try {
            Thread.sleep(500) ;
        } catch (InterruptedException e) {}
        if (Loutput[0]!=null){
            System.out.println(Loutput[0]);
        }
        if (Loutput[1]!=null) {
            System.out.println(Loutput[1]);
        }
        System.out.println( "end dispose" + Thread.currentThread());
        disposed = true;

        // check that dispose() works OK.
        return  listenerDisposed[0] && !listenerDisposed[1];

    } // finished _dispose()

    /**
    * Forces object recreation.
    */
    protected void after() {
    }

} // finished class _XComponent


