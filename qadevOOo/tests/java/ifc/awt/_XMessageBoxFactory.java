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

package ifc.awt;

import com.sun.star.awt.Rectangle;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.lang.XMultiServiceFactory;
import lib.MultiMethodTest;

import com.sun.star.awt.XMessageBoxFactory;
import com.sun.star.awt.XWindow;
import com.sun.star.frame.XModel;
import java.io.PrintWriter;
import lib.Status;
import lib.StatusException;
import util.UITools;

/**
 * Testing <code>com.sun.star.awt.XMessageBoxFactory</code>
 * interface methods :
 * <ul>
 *  <li><code> </code>createMessageBox()</li>
 * </ul> <p>
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.awt.XMessageBoxFactory
 */
public class _XMessageBoxFactory extends MultiMethodTest {

    public XMessageBoxFactory oObj = null;
    private XWindowPeer the_win = null;
    private XModel xModel = null;

    /**
     * Retrieves object relation.
     * @throws StatusException If the relation not found.
     */
    public void before() {
        the_win = (XWindowPeer) tEnv.getObjRelation("WINPEER");
        if (the_win == null)
            throw new StatusException(Status.failed("Relation 'WINPEER' not found")) ;
        xModel = (XModel) tEnv.getObjRelation("XModel");
        if (xModel == null)
            throw new StatusException(Status.failed("Relation 'xModel' not found")) ;

    }


    /**
     * As <code>execute()</code> method is a blocking call,
     * then it must be executed in a separate thread. This
     * thread class just call <code>execute</code> method
     * of tested object.
     */
    protected  Thread execThread = new Thread(
            new Runnable() {
        public void run() {
            Rectangle rect = new Rectangle(0,0,100,100);
            oObj.createMessageBox(the_win, rect, "errorbox", 1, "The Title", "The Message") ;
        }
    }) ;


    /**
     * Starts the execution of MessageBox in a separate thread.
     * As this call is blocking then the thread execution
     * must not be finished. <p>
     * Has <b>OK</b> status if thread wasn't finished and
     * no exceptions occured.
     */
    public void _createMessageBox() {
        boolean result = true ;

        log.println("Starting createMessageBox() thread ...") ;
        execThread.start() ;

        try {
            execThread.join(200) ;
        } catch (InterruptedException e) {
            log.println("createMessageBox() thread was interrupted") ;
            result = false ;
        }
        result &= execThread.isAlive() ;

        UITools oUITools = new UITools((XMultiServiceFactory) tParam.getMSF(), xModel);

        XWindow xWindow = null;
        try{
            xWindow = oUITools.getActiveTopWindow();

            oUITools.printAccessibleTree(log, tParam.getBool("DebugIsActive"));

            oUITools.clickButton("OK");

        } catch (java.lang.Exception e) {
            e.printStackTrace((PrintWriter) log);
            log.println("Could not cklick 'OK' on messagebox: " + e.toString());
            result = false;
        }

        tRes.tested("createMessageBox()", result) ;
    }

}


