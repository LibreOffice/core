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
    * no exceptions occurred.
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


