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
* Test is multithread compilant. <p>
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
    public void after() {
        disposeEnvironment() ;
    }

} // finished class _XInitialization


