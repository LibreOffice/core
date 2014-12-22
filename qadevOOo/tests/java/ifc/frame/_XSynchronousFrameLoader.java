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
import com.sun.star.frame.XSynchronousFrameLoader;
import com.sun.star.lang.XComponent;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.URL;
import com.sun.star.util.XURLTransformer;

/**
 * Testing <code>com.sun.star.frame.XSynchronousFrameLoader</code>
 * interface methods :
 * <ul>
 *  <li><code> load()</code></li>
 *  <li><code> cancel()</code></li>
 * </ul> <p>
 * This test needs the following object relations :
 * <ul>
 *  <li> <code>'FrameLoader.URL'</code> (of type <code>String</code>):
 *   an url of component to be loaded </li>
 *  <li> <code>'FrameLoader.Frame'</code> <b>(optional)</b>
 *  (of type <code>com.sun.star.frame.XFrame</code>):
 *   a target frame where component to be loaded. If this
 *   relation is omitted then a text document created and its
 *   frame is used. </li>
 * <ul> <p>
 *
 * @see com.sun.star.frame.XSynchronousFrameLoader
 */
public class _XSynchronousFrameLoader extends MultiMethodTest {

    public XSynchronousFrameLoader oObj = null; // oObj filled by MultiMethodTest
    private XFrame frame = null ;
    private XComponent frameSup = null ;
    private PropertyValue[] descr = null;

    /**
     * Retrieves all relations. If optional relation
     * <code>FrameLoader.Frame</code> not found
     * creates a new document and otains its frame for loading. <p>
     *
     * Also <code>MediaDescriptor</code> is created using
     * URL from <code>FrameLoader.URL</code> relation.
     *
     * @throws StatusException If one of required relations not found.
     */
    @Override
    public void before() {
        String url = (String) tEnv.getObjRelation("FrameLoader.URL") ;
        frame = (XFrame) tEnv.getObjRelation("FrameLoader.Frame") ;

        if (url == null) {
            throw new StatusException(Status.failed("Some relations not found")) ;
        }

        SOfficeFactory SOF = SOfficeFactory.getFactory(
                             tParam.getMSF() );

        XURLTransformer xURLTrans = null;

        // if frame is not contained in relations the writer frmame will be used.
        if (frame == null) {
            try {
                log.println( "creating a textdocument" );
                frameSup = SOF.createTextDoc( null );

                Object oDsk = tParam.getMSF().createInstance
                    ("com.sun.star.frame.Desktop") ;
                XDesktop dsk = UnoRuntime.queryInterface
                    (XDesktop.class, oDsk) ;
                frame = dsk.getCurrentFrame() ;

                Object o = tParam.getMSF().createInstance
                    ("com.sun.star.util.URLTransformer") ;
                xURLTrans = UnoRuntime.queryInterface
                    (XURLTransformer.class, o) ;

            } catch ( com.sun.star.uno.Exception e ) {
                // Some exception occurs.FAILED
                e.printStackTrace( log );
                throw new StatusException( "Couldn't create a document.", e );
            }
        }

        URL[] urlS = new URL[1];
        urlS[0] = new URL();
        urlS[0].Complete = url;
        boolean res = xURLTrans.parseStrict(urlS);
        log.println("Parsing URL '" + url + "': " + res);
        descr = new PropertyValue[1] ;
        descr[0] = new PropertyValue();
        descr[0].Name = "URL" ;
        descr[0].Value = urlS[0] ;
    }


    /**
     * Tries to load component to a frame. <p>
     * Has <b> OK </b> status if <code>true</code> is returned.
     */
    public void _load() {
        boolean result = oObj.load(descr, frame) ;

        tRes.tested("load()", result) ;
    }

    /**
     * Tries to load component to a frame in separate thread to
     * avoid blocking of the current thread and immediately
     * cancels loading. <p>
     *
     * Has <b> OK </b> status if <code>flase</code> is returned,
     * i.e. loading was not completed.
     */
    public void _cancel() {
        requiredMethod("load()") ;

        final boolean[] result = new boolean[1] ;

        (new Thread() {
            @Override
            public void run() {
                result[0] = oObj.load(descr, frame);
            }
        }).start();

        oObj.cancel() ;

        util.utils.pause(1000);


        tRes.tested("cancel()", !result[0]) ;
    }

    /**
     * Disposes document if it was created for frame supplying.
     */
    @Override
    protected void after() {
        if (frameSup != null) {
            frameSup.dispose();
        }
    }
}


