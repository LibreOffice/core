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
* Test is <b> NOT </b> multithread compliant. <p>
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
    * call successfully returned.
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
    @Override
    public void after() {
        disposeEnvironment() ;
    }
}


