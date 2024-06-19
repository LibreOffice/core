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

package complex.dataPilot;

import com.sun.star.container.XNamed;
import lib.TestParameters;

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
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.container.XNamed
*/
public class _XNamed {

    /**
     * The object that is tested.
     */
    private final XNamed oObj;

    /**
     * Constructor: gets the object to test, a logger and the test parameters
     * @param xObj The test object
     * @param param The test parameters
     */
    public _XNamed(XNamed xObj/*, LogWriter log*/, TestParameters param) {
        oObj = xObj;
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


