/*************************************************************************
 *
 *  $RCSfile: _XComponent.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2004-07-23 10:54:38 $
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

package imageManager.interfaces;

import com.sun.star.container.XNameContainer;
import share.LogWriter;

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
    private XNameContainer xContainer = null;
    private XComponent altDispose = null;
    TestParameters tEnv = null;
    LogWriter log = null;
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

    public _XComponent(LogWriter log, TestParameters tEnv, XComponent oObj) {
        this.log = log;
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
        if (disposed) return false;
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

        log.println( "begin dispose" + Thread.currentThread());
        XDesktop oDesk = (XDesktop) tEnv.get("Desktop");
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
        return  listenerDisposed[0] && !listenerDisposed[1];

    } // finished _dispose()

    /**
    * Forces object recreation.
    */
    protected void after() {
//        disposeEnvironment();
    }

} // finished class _XComponent


