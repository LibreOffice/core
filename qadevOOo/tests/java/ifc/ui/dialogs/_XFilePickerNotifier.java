/*************************************************************************
 *
 *  $RCSfile: _XFilePickerNotifier.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 11:27:37 $
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

package ifc.ui.dialogs;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.lang.EventObject;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ui.dialogs.FilePickerEvent;
import com.sun.star.ui.dialogs.XExecutableDialog;
import com.sun.star.ui.dialogs.XFilePicker;
import com.sun.star.ui.dialogs.XFilePickerListener;
import com.sun.star.ui.dialogs.XFilePickerNotifier;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XCancellable;


/**
* Testing <code>com.sun.star.ui.XFilePickerNotifier</code>
* interface methods :
* <ul>
*  <li><code> addFilePickerListener()</code></li>
*  <li><code> removeFilePickerListener()</code></li>
* </ul> <p>
* The object must implement <code>XFilePicker</code>
* interface to check if a listener was called. <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.ui.XFilePickerNotifier
*/
public class _XFilePickerNotifier extends MultiMethodTest {

    public XFilePickerNotifier oObj = null;
    private XFilePicker fps = null ;
    private String dir1 = null,
                   dir2 = null ;
    ExecThread eThread = null;


    public void after() {
        XCancellable canc = (XCancellable) UnoRuntime.queryInterface(
            XCancellable.class, tEnv.getTestObject());
        shortWait();
        if (canc != null) {
            log.println("Cancelling Dialog");
            canc.cancel();
        } else {
            tEnv.dispose();
        }
    }

    /**
    * Listener implementation which sets a flag if some of its
    * methods was called.
    */
    protected class TestListener implements XFilePickerListener {
        public boolean called = false ;

        public void dialogSizeChanged() {
            called = true;
        }

        public void fileSelectionChanged(FilePickerEvent e) {
            called = true;
        }

        public void directoryChanged(FilePickerEvent e) {
            log.println("***** Directory Changed *****");
            called = true;
        }

        public String helpRequested(FilePickerEvent e) {
            called = true;
            return "help";
        }

        public void controlStateChanged(FilePickerEvent e) {
            called = true;
        }

        public void disposing(EventObject e) {}
    }

    TestListener listener = new TestListener() ;

    /**
    * Tries to query object for <code>XFilePicker</code> interface, and
    * initializes two different URLs for changing file picker directory. <p>
    * @throw StatusException If object doesn't support <code>XFilePicker</code>
    * interface.
    */
    public void before() {
        fps = (XFilePicker) UnoRuntime.queryInterface
            (XFilePicker.class, oObj) ;

        if (fps == null) {
            log.println("The object doesnt implement XFilePicker") ;
            throw new StatusException(Status.failed
                ("The object doesnt implement XFilePicker"));
        }

        dir1 = util.utils.getOfficeTemp((XMultiServiceFactory)tParam.getMSF());
        dir2 = util.utils.getFullTestURL("");

        XExecutableDialog exD = (XExecutableDialog) UnoRuntime.queryInterface(
            XExecutableDialog.class, tEnv.getTestObject());
        eThread = new ExecThread(exD);
        log.println("Starting Dialog");
        eThread.start();
    }

    /**
    * Adds a listener, then tries to change display directory and
    * checks if the listener was called. <p>
    * Has <b>OK</b> status if a listener method was called.
    */
    public void _addFilePickerListener() {
        oObj.addFilePickerListener(listener) ;

        try {
            log.println("***** Setting DisplayDirectory to " + dir1);
            fps.setDisplayDirectory(dir1) ;
            log.println("***** Getting: " + fps.getDisplayDirectory());
            log.println("***** Setting DisplayDirectory to " + dir2);
            fps.setDisplayDirectory(dir2) ;
            log.println("***** Getting: " + fps.getDisplayDirectory());

        } catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("!!! Exception changing dir !!!") ;
            e.printStackTrace(log) ;
        }

        try {
            Thread.sleep(5000) ;
        } catch(InterruptedException e) {}

        if (!listener.called) {
            log.println("Listener wasn't called :-(");
        }

        tRes.tested("addFilePickerListener()", listener.called) ;
    }

    /**
    * Removes the listener and changes display directory. <p>
    * Has <b>OK</b> status if the listener wasn't called. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> addFilePickerListener </code> </li>
    * </ul>
    */
    public void _removeFilePickerListener() {
        requiredMethod("addFilePickerListener()") ;

        oObj.removeFilePickerListener(listener) ;

        listener.called = false ;

        try {
            fps.setDisplayDirectory(dir1) ;
            fps.setDisplayDirectory(dir2) ;
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("!!! Exception changing dir !!!") ;
            e.printStackTrace(log) ;
        }

        try {
            Thread.sleep(2000) ;
        } catch(InterruptedException e) {}

        tRes.tested("removeFilePickerListener()", !listener.called) ;
    }

    /**
    * Calls <code>execute()</code> method in a separate thread.
    * Necessary to check if this method works
    */
    protected class ExecThread extends Thread {

        public short execRes = (short) 17 ;
        private XExecutableDialog Diag = null ;

        public ExecThread(XExecutableDialog Diag) {
            this.Diag = Diag ;
        }

        public void run() {
            execRes = Diag.execute();
            System.out.println("HERE: "+execRes);
            shortWait();
        }
    }

    /**
    * Sleeps for 0.5 sec. to allow StarOffice to react on <code>
    * reset</code> call.
    */
    private void shortWait() {
        try {
            Thread.sleep(500) ;
        } catch (InterruptedException e) {
            log.println("While waiting :" + e) ;
        }
    }
}


