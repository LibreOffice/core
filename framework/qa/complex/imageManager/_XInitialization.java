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

package complex.imageManager;




import com.sun.star.lang.XInitialization;
import lib.TestParameters;

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
public class _XInitialization {


    TestParameters tEnv = null;
    public static XInitialization oObj = null;

    public _XInitialization(TestParameters tEnv, XInitialization oObj) {

        this.tEnv = tEnv;
        this.oObj = oObj;
    }

    /**
    * Test calls the method with 0 length array and checks that
    * no exceptions were thrown. <p>
    * Has <b> OK </b> status if no exceptions were thrown. <p>
    */
    public boolean _initialize() {
        boolean result = true ;

        try {
            Object[] args = (Object[]) tEnv.get("XInitialization.args");
            if (args==null) {
                oObj.initialize(new Object[0]);
            } else {
                oObj.initialize(args);
            }

        } catch (com.sun.star.uno.Exception e) {
            System.out.println("Exception occurred while method calling.") ;
            result = false ;
        }

        return  result ;
    } // finished _initialize()

    /**
    * Disposes object environment.
    */
    public void after() {
//        disposeEnvironment() ;
    }

} // finished class _XInitialization


