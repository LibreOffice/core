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

package ifc.task;



/**
* <code>com.sun.star.task.XInteractionHandler</code> interface testing.
* This test needs the following object relations :
* <ul>
*  <li> <code>'XInteractionHandler.Request'</code>
*    (of type <code>com.sun.star.task.XInteractionRequest</code>):
*    this interface implementation is handler specific and is
*    passed as argument to method <code>handle</code>. </li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.task.XInteractionHandler
*/
import lib.MultiMethodTest;
import lib.Status;

import com.sun.star.task.XInteractionHandler;
import com.sun.star.task.XInteractionRequest;

/**
* <code>com.sun.star.task.XInteractionHandler</code> interface testing.
* This test needs the following object relations :
* <ul>
*  <li> <code>'XInteractionHandler.Request'</code>
*    (of type <code>com.sun.star.task.XInteractionRequest</code>):
*    this interface implementation is handler specific and is
*    passed as argument to method <code>handle</code>. </li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.task.XInteractionHandler
*/
public class _XInteractionHandler extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XInteractionHandler oObj = null ;

    private XInteractionRequest request = null ;

    /**
    * Retrieves an object relation. <p>
    * @throws StatusException If the relation is not found.
    */
    public void before() {
        request = (XInteractionRequest)
            tEnv.getObjRelation("XInteractionHandler.Request") ;

        //if (request == null)
        //    throw new StatusException(Status.failed("Reelation not found")) ;
    }

    /**
    * Sinse this test is interactive (dialog window can't be
    * disposed using API) it is skipped. <p>
    * Always has <b>SKIPPED.OK</b> status .
    */
    public void _handle() {

        /*

        final XMultiServiceFactory msf = (XMultiServiceFactory)tParam.getMSF() ;
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );
        XComponent textDoc = null ;
        try {
            textDoc = SOF.createTextDoc( null );
        } catch (com.sun.star.uno.Exception e) {
            log.println("Can't create Document") ;
            tRes.tested("handle()", false) ;
            return ;
        }
        final XComponent fTextDoc = textDoc ;
        final XModel xModelDoc = (XModel)
            UnoRuntime.queryInterface(XModel.class, textDoc);

        Thread thr = new Thread( new Runnable() {
            public void run() {
                try {
                    Thread.sleep(1000) ;
                } catch (InterruptedException e ) {}

                //fTextDoc.dispose() ;

                XFrame docFr = xModelDoc.getCurrentController().getFrame() ;
                docFr.dispose() ;

                /
                try {

                    Object dsk = msf.createInstance
                        ("com.sun.star.frame.Desktop");

                    XFrame xDsk = (XFrame)
                        UnoRuntime.queryInterface(XFrame.class, dsk) ;

                    XFrame fr = xDsk.findFrame("_top", 55) ;

                    XWindow win = fr.getContainerWindow() ;

                    String name = null ;
                    if (fr != null) {
                        name = fr.getName() ;
                        docFr.dispose() ;
                    }

                } catch (com.sun.star.uno.Exception e) {
                    e.printStackTrace();
                }
            }
        }) ;
        thr.start() ;

        oObj.handle(request) ;
        try {
            thr.join(500) ;
        } catch (InterruptedException e ) {}
        */

        tRes.tested("handle()", Status.skipped(true)) ;
    }
}


