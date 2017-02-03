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

package ifc.frame;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;
import util.SOfficeFactory;

import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XFrameLoader;
import com.sun.star.frame.XLoadEventListener;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.XComponent;
import com.sun.star.uno.UnoRuntime;


/**
* Testing <code>com.sun.star.frame.XFrameLoader</code>
* interface methods :
* <ul>
*  <li><code> load()</code></li>
*  <li><code> cancel()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'FrameLoader.URL'</code> (of type <code>String</code>):
*   an url or component to be loaded </li>
*  <li> <code>'FrameLoader.Frame'</code> <b>(optional)</b>
*  (of type <code>com.sun.star.frame.XFrame</code>):
*   a target frame where component to be loaded. If this
*   relation is omitted then a text document created and its
*   frame is used. </li>
*  <li> <code>'FrameLoader.args'</code> <b>(optional)</b>
*   (of type <code>Object[]</code>):
*   necessary arguments for loading a component. If omitted
*   then zero length array is passed as parameter</li>
* <ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.frame.XFrameLoader
*/
public class _XFrameLoader extends MultiMethodTest {

    public XFrameLoader oObj = null; // oObj filled by MultiMethodTest
    private String url = null ;
    private XFrame frame = null ;
    private PropertyValue[] args = new PropertyValue[0] ;

    /**
    * Implementation of load listener which geristers all its calls.
    */
    protected static class TestListener implements XLoadEventListener {
        public boolean finished = false ;
        public boolean cancelled = false ;

        public void loadFinished(XFrameLoader l) {
            finished = true ;
        }
        public void loadCancelled(XFrameLoader l) {
            cancelled = true ;
        }
        public void disposing(EventObject e) {}
    }

    TestListener listener = new TestListener() ;
    XComponent frameSup = null ;

    /**
    * Retrieves all relations. If optional  ones are not found
    * creates their default values. <p>
    * @throws StatusException If one of required relations not found.
    */
    @Override
    public void before() {
        url = (String) tEnv.getObjRelation("FrameLoader.URL") ;
        frame = (XFrame) tEnv.getObjRelation("FrameLoader.Frame") ;

        if (frame == null) {
            SOfficeFactory SOF = SOfficeFactory.getFactory( tParam.getMSF() );

            try {
                log.println( "creating a textdocument" );
                frameSup = SOF.createTextDoc( null );

                Object oDsk = tParam.getMSF()
                        .createInstance("com.sun.star.frame.Desktop") ;
                XDesktop dsk = UnoRuntime.queryInterface(XDesktop.class, oDsk) ;

                util.utils.shortWait();
                frame = dsk.getCurrentFrame() ;
            } catch ( com.sun.star.uno.Exception e ) {
                // Some exception occurs.FAILED
                e.printStackTrace( log );
                throw new StatusException( "Couldn't create a frame.", e );
            }
        }

        Object args = tEnv.getObjRelation("FrameLoader.args") ;
        if (args != null) this.args = (PropertyValue[]) args ;

        if (url == null /*|| frame == null*/) {
            throw new StatusException
                (Status.failed("Some relations not found")) ;
        }
    }

    private boolean loaded = false ;

    /**
    * First <code>cancel</code> method test is called.
    * If in that test loading process was interrupted by
    * <code>cancel</code> call then <code>load</code> test
    * executes. It loads a component, waits some moment to
    * listener have a chance to be called  and then checks
    * if the load listener was called.<p>
    * Has <b>OK</b> status if <code>cancel</code> method test
    * didn't interrupt loading and it was successful, or
    * if in this method it loads successful and listener's
    * <code>finished</code> method was called.
    * The following method tests are to be executed before:
    * <ul>
    *  <li> <code> cancel() </code> </li>
    * </ul>
    */
    public void _load() throws Exception {
        executeMethod("cancel()") ;

        if (!loaded) {
            oObj.load(frame, url, args, listener) ;

            waitForEventIdle();

            loaded = listener.finished ;
        }

        tRes.tested("load()", loaded) ;
    }

    /**
    * Starts to load a component and then immediately tries to
    * cancel the process. <p>
    * Has <b>OK</b> status if the process was cancelled or
    * finished (appropriate listener methods were called).
    */
    public void _cancel() throws Exception {
        boolean result = true ;

        oObj.load(frame, url, args, listener) ;
        oObj.cancel() ;

        waitForEventIdle();

        if (listener.cancelled) {
            log.println("Loading was canceled.") ;
        }
        if (listener.finished) {
            log.println("Loading was finished.") ;
            loaded = true ;
        }
        if (!listener.cancelled && !listener.finished) {
            log.println("Loading was not canceled and not finished") ;
            result = false ;
        }

        tRes.tested("cancel()", result) ;
    }

    @Override
    public void after() {
        if (frameSup != null) frameSup.dispose() ;
        frame.dispose();
    }
}

