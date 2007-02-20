/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XMessageBoxFactory.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-02-20 14:21:56 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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


