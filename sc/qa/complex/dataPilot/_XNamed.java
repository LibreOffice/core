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

package complex.dataPilot;

import com.sun.star.container.XNamed;
import lib.TestParameters;
// import share.LogWriter;
// import util.utils;

/**
* Testing <code>com.sun.star.container.XNamed</code>
* interface methods :
* <ul>
*  <li><code> getName()</code></li>
*  <li><code> setName()</code></li>
* </ul>
* This test need the following object relations :
* <ul>
*  <li> <code>'setName'</code> : of <code>Boolean</code>
*    type. If it exists then <code>setName</code> method
*    isn't to be tested and result of this test will be
*    equal to relation value.</li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.container.XNamed
*/
public class _XNamed {

    /**
     * The object that is testsed.
     */
    public XNamed oObj = null;

    /**
     * The test parameters
     */
    private TestParameters param = null;

    /**
     * The log writer
     */
    // private LogWriter log = null;

    /**
     * Constructor: gets the object to test, a logger and the test parameters
     * @param xObj The test object
     * @param log A log writer
     * @param param The test parameters
     */
    public _XNamed(XNamed xObj/*, LogWriter log*/, TestParameters param) {
        oObj = xObj;
        // this.log = log;
        this.param = param;
    }

    /**
    * Test calls the method and checks return value and that
    * no exceptions were thrown. <p>
    * Has <b> OK </b> status if the method returns non null value
    * and no exceptions were thrown. <p>
    */
    public boolean _getName() {

        // write to log what we try next
        System.out.println( "test for getName()" );

        boolean result = true;
        boolean loc_result = true;
        String name = null;
        String NewName = null;

        loc_result = ((name = oObj.getName()) != null);
        System.out.println("getting the name \"" + name + "\"");

        if (loc_result)
        {
            System.out.println("... getName() - OK");
        }
        else
        {
            System.out.println("... getName() - FAILED");
        }
        result &= loc_result;
        return result;
    }

    /**
    * Sets a new name for object and checks if it was properly
    * set. Special cases for the following objects :
    * <ul>
    *  <li><code>ScSheetLinkObj</code> : name must be in form of URL.</li>
    *  <li><code>ScDDELinkObj</code> : name must contain link to cell in
    *     some external Sheet.</li>
    * </ul>
    * Has <b> OK </b> status if new name was successfully set, or if
    * object environment contains relation <code>'setName'</code> with
    * value <code>true</code>. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getName() </code> : to be sure the method works</li>
    * </ul>
    */
    public boolean _setName(){
//        requiredMethod("getName()");
        System.out.println("testing setName() ... ");

        String oldName = oObj.getName();
        String NewName = oldName == null ? "XNamed" : oldName + "X" ;

        boolean result = true;
        boolean loc_result = true;
        System.out.println("set the name of object to \"" + NewName + "\"");
        oObj.setName(NewName);
        System.out.println("check that container has element with this name");

        String name = oObj.getName();
        System.out.println("getting the name \"" + name + "\"");
        loc_result = name.equals(NewName);

        if (loc_result)
        {
            System.out.println("... setName() - OK");
        }
        else
        {
            System.out.println("... setName() - FAILED");
        }
        result &= loc_result;
        oObj.setName(oldName);
        return result;
    }
}


