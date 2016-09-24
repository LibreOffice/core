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

package ifc.lang;

import lib.MultiMethodTest;

import com.sun.star.lang.XInitialization;

/**
* Testing <code>com.sun.star.lang.XInitialization</code>
* interface methods. <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'XInitialization.args'</code> (of type <code>Object[]</code>):
*   (<b>Optional</b>) : argument for <code>initialize</code>
*   method. If ommitet zero length array is used. </li>
* <ul> <p>
* Test is multithread compliant. <p>
* Till the present time there was no need to recreate environment
* after this test completion.
*/
public class _XInitialization extends MultiMethodTest {

    public static XInitialization oObj = null;

    /**
    * Test calls the method with 0 length array and checks that
    * no exceptions were thrown. <p>
    * Has <b> OK </b> status if no exceptions were thrown. <p>
    */
    public void _initialize() {
        boolean result = true ;

        try {
            XInitialization xInit = (XInitialization) tEnv.getObjRelation("XInitialization.xIni");
            if (xInit == null) xInit = oObj;

            log.println("calling method with valid arguments...");
            Object[] args = (Object[]) tEnv.getObjRelation("XInitialization.args");
            if (args==null) {
                System.out.println("Using new Object[0] as Argument");
                xInit.initialize(new Object[0]);
            } else {
                xInit.initialize(args);
            }

            // try to call the method with invalid parameters
            Object[] ExArgs = (Object[]) tEnv.getObjRelation("XInitialization.ExceptionArgs");
            if (ExArgs !=null) {
                log.println("calling method with in-valid arguments...");
                try{
                    result = false;
                    xInit.initialize(ExArgs);
                } catch (com.sun.star.uno.Exception e) {
                    log.println("Expected Exception 'com.sun.star.uno.Exception' occurred -> OK") ;
                    result = true ;
                } catch (com.sun.star.uno.RuntimeException e) {
                    log.println("Expected Exception 'com.sun.star.uno.RuntimeException' occurred -> OK") ;
                    result = true ;
                } catch (Exception e) {
                    log.println("Un-Expected Exception occurred -> FALSE") ;
                    log.println(e.toString());
                    e.printStackTrace();
                }
            }

        } catch (com.sun.star.uno.Exception e) {
            log.println("Exception occurred while method calling.") ;
            log.println(e) ;
            result = false ;
        }

        tRes.tested("initialize()", result) ;
    } // finished _initialize()

    /**
    * Disposes object environment.
    */
    @Override
    public void after() {
        disposeEnvironment() ;
    }

} // finished class _XInitialization


