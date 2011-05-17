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

package complex.tdoc;

import com.sun.star.container.XNameContainer;
import lib.MultiMethodTest;

import com.sun.star.frame.XDesktop;
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
    private XNameContainer xContainer = null;
    private XComponent altDispose = null;
    public LogWriter log = null;

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
    };

    /**
    * Listener which added and then removed, and its method must <b>not</b>
    * be called on <code>dispose</code> call.
    */
    public class MyEventListener2 implements XEventListener {
        public void disposing ( EventObject oEvent ) {
            Loutput[0] = Thread.currentThread() + " is DISPOSING EV2" + this;
            listenerDisposed[1] = true;
        }
    };

    XEventListener listener1 = new MyEventListener();
    XEventListener listener2 = new MyEventListener2();

    /**
     * For the cfgmgr2.OSetElement tests: dispose the owner element.
     */
    protected void before() {
        // do not dispose this component, but parent instead
//        altDispose = (XComponent)tEnv.getObjRelation("XComponent.DisposeThis");

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
//        executeMethod("addEventListener()");
        if (disposed) return true;
        // the second listener should not be called
        oObj.removeEventListener( listener2 );
        log.println(Thread.currentThread() + " is removing EL " + listener2);
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
//        executeMethod("removeEventListener()");

        log.println( "begin dispose" + Thread.currentThread());
        oObj.dispose();

        try {
            Thread.sleep(500) ;
        } catch (InterruptedException e) {}
        if (Loutput[0]!=null) log.println(Loutput[0]);
        if (Loutput[1]!=null) log.println(Loutput[1]);
        log.println( "end dispose" + Thread.currentThread());
        disposed = true;

        // check that dispose() works OK.
        return  listenerDisposed[0] && !listenerDisposed[1];

    } // finished _dispose()

    /**
    * Forces object recreation.
    */
    protected void after() {
//        disposeEnvironment();
    }

} // finished class _XComponent


