/*************************************************************************
 *
 *  $RCSfile: _XExecutableDialog.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:27:01 $
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

import com.sun.star.ui.dialogs.XExecutableDialog;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XCancellable;

/**
 * Testing <code>com.sun.star.ui.dialogs.XExecutableDialog</code>
 * interface methods :
 * <ul>
 *  <li><code> setTitle()</code></li>
 *  <li><code> execute()</code></li>
 * </ul> <p>
 *
 * This interface methods cann't be checked, thereby methods
 * are just called. <code>execute</code> method is not called
 * at all as the dialog shown cann't be disposed. <p>
 *
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.ui.dialogs.XExecutableDialog
 */
public class _XExecutableDialog extends MultiMethodTest {

    public XExecutableDialog oObj = null;

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method successfully returns
     * and no exceptions were thrown. <p>
     */
    public void _setTitle() {
        oObj.setTitle("The Title");
        tRes.tested("setTitle()",true);
    }

    /**
     * This method is excluded from automated test since
     * we can't close the dialog. <p>
     * Always has <b>OK</b> status.
     */
    public void _execute() {
        String aName = tEnv.getTestCase().getObjectName();
        boolean result = false;
        if (aName.startsWith("OData") || aName.startsWith("OSQL")) {
            log.println("dbaccess dialogs can't be closed via API");
            log.println("therefore they aren't executed");
            log.println("and the result is set to true");
            result = true;
        } else {
            ExecThread eThread = new ExecThread(oObj);
            log.println("Starting Dialog");
            eThread.start();
            XCancellable canc = (XCancellable)UnoRuntime.queryInterface
                (XCancellable.class, tEnv.getTestObject());
            shortWait();
            if (canc != null) {
                log.println("Cancelling Dialog");
                canc.cancel();
                shortWait();
                short res = eThread.execRes;
                log.println("result: "+res);
                result = (res == 0);
            } else {
                tEnv.dispose();
                result=true;
                log.println("XCancellable isn't supported and the "+
                    "environment is killed hard");
            }


        }
        tRes.tested("execute()",result);
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


