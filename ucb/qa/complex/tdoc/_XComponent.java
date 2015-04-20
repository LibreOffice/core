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

package complex.tdoc;

import com.sun.star.lang.EventObject;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XEventListener;
import share.LogWriter;

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
    private final LogWriter log = null;

    private boolean listenerDisposed[] = new boolean[2];
    private String[] Loutput = new String[2];

    /**
    * Listener which added but not removed, and its method must be called
    * on <code>dispose</code> call.
    */
    private class MyEventListener implements XEventListener {
        public void disposing ( EventObject oEvent ) {
            Loutput[0] = Thread.currentThread() + " is DISPOSING EV1" + this;
            listenerDisposed[0] = true;
        }
    }

    /**
    * Listener which added and then removed, and its method must <b>not</b>
    * be called on <code>dispose</code> call.
    */
    private class MyEventListener2 implements XEventListener {
        public void disposing ( EventObject oEvent ) {
            Loutput[0] = Thread.currentThread() + " is DISPOSING EV2" + this;
            listenerDisposed[1] = true;
        }
    }

    private final XEventListener listener1 = new MyEventListener();
    private final XEventListener listener2 = new MyEventListener2();

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
        if (disposed) return true;
        // the second listener should not be called
        oObj.removeEventListener( listener2 );
        log.println(Thread.currentThread() + " is removing EL " + listener2);
        return true;
    } // finished _removeEventListener()

    private boolean disposed = false;

} // finished class _XComponent


