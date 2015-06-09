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

package mod._sc;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNamed;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.sheet.TableAutoFormat</code>. <p>
* In StarCalc application there is a collection of autoformats
* for tables (you can select a predefined format for a
* table or create your own). This object represents
* one of these autoformats. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::container::XNamed</code></li>
*  <li> <code>com::sun::star::container::XIndexAccess</code></li>
*  <li> <code>com::sun::star::container::XElementAccess</code></li>
*  <li> <code>com::sun::star::sheet::TableAutoFormat</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
* </ul>
* This object test <b> is NOT </b> designed to be run in several
* threads concurrently.
* @see com.sun.star.container.XNamed
* @see com.sun.star.container.XIndexAccess
* @see com.sun.star.container.XElementAccess
* @see com.sun.star.sheet.TableAutoFormat
* @see com.sun.star.beans.XPropertySet
* @see ifc.container._XNamed
* @see ifc.container._XIndexAccess
* @see ifc.container._XElementAccess
* @see ifc.sheet._TableAutoFormat
* @see ifc.beans._XPropertySet
*/
public class ScAutoFormatObj extends TestCase {

    /**
    * Creating a TestEnvironment for the interfaces to be tested.
    * Using SOffice ServiceManager an instance of
    * <code>com.sun.star.sheet.TableAutoFormatField</code> service
    * is created. From this collection one Format is retrieved as
    * object tested.
    */
    @Override
    public TestEnvironment createTestEnvironment(TestParameters tParam,
            PrintWriter log) throws Exception {

        XInterface oObj = null;
        XMultiServiceFactory oMSF = tParam.getMSF();
        XInterface formats = (XInterface)oMSF.createInstance
                ("com.sun.star.sheet.TableAutoFormats");
        XIndexAccess formatsIndex = UnoRuntime.queryInterface(XIndexAccess.class, formats);
        oObj = (XInterface) AnyConverter.toObject(
                new Type(XInterface.class),formatsIndex.getByIndex
                                        (formatsIndex.getCount() - 1));

        XNamed objNamed = UnoRuntime.queryInterface(XNamed.class, oObj) ;
        log.println("AutoFormat name is '" + objNamed.getName() + "'") ;

        TestEnvironment tEnv = new TestEnvironment(oObj);

        return tEnv;
    }
}

