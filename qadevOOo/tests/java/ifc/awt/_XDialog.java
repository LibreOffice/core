/*************************************************************************
 *
 *  $RCSfile: _XDialog.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:11:11 $
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

package ifc.awt;

import lib.MultiMethodTest;

import com.sun.star.awt.XDialog;

/**
* Testing <code>com.sun.star.awt.XDialog</code>
* interface methods :
* <ul>
*  <li><code> setTitle()</code></li>
*  <li><code> getTitle()</code></li>
*  <li><code> execute()</code></li>
*  <li><code> endExecute()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* After test completion object environment has to be recreated.
* @see com.sun.star.awt.XDialog
*/
public class _XDialog extends MultiMethodTest {

    public XDialog oObj = null;

    /**
    * As <code>execute()</code> method is a blocking call,
    * then it must be executed in a separate thread. This
    * thread class just call <code>execute</code> method
    * of tested object.
    */
    protected  Thread execThread = new Thread(
        new Runnable() {
            public void run() {
                oObj.execute() ;
            }
        }) ;

    /**
    * Sets the title to some string. <p>
    * Has <b>OK</b> status if no runtime exceptions occurs.
    */
    public void _setTitle() {

        oObj.setTitle("XDialog test") ;

        tRes.tested("setTitle()", true) ;
    }

    /**
    * Gets the title and compares it to the value set in
    * <code>setTitle</code> method test. <p>
    * Has <b>OK</b> status is set/get values are equal.
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setTitle </code>  </li>
    * </ul>
    */
    public void _getTitle() {
        requiredMethod("setTitle()") ;

        tRes.tested("getTitle()",
            "XDialog test".equals(oObj.getTitle())) ;
    }

    /**
    * Starts the execution of dialog in a separate thread.
    * As this call is blocking then the thread execution
    * must not be finished. <p>
    * Has <b>OK</b> status if thread wasn't finished and
    * no exceptions occured.
    */
    public void _execute() {
        boolean result = true ;

        log.println("Starting execute() thread ...") ;
        execThread.start() ;

        try {
            execThread.join(200) ;
        } catch (InterruptedException e) {
            log.println("execute() thread was interrupted") ;
            result = false ;
        }
        result &= execThread.isAlive() ;

        tRes.tested("execute()", result) ;
    }

    /**
    * Calls the method and checks if the execution thread
    * where <code>execute()</code> method is running was
    * finished. If <code>execute</code> method didn't return
    * and still running then thread interrupted. <p>
    * Has <b>OK</b> status if <code>execute</code> method
    * call successfully retured.
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> execute </code> </li>
    * </ul>
    */
    public void _endExecute() {
        requiredMethod("execute()") ;

        boolean result = true ;

        oObj.endExecute() ;

        try {
            execThread.join(200) ;
        } catch (InterruptedException e) {
            log.println("execute() thread was interrupted") ;
            result = false ;
        }

        if (execThread.isAlive()) {
            execThread.interrupt() ;
        }

        result &= !execThread.isAlive() ;

        tRes.tested("endExecute()", result) ;
    }

    /**
    * Disposes object environment.
    */
    public void after() {
        disposeEnvironment() ;
    }
}


