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

package mod._ucprmt;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ucb.XContentIdentifierFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.ucb.RemoteAccessContentProvider</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::ucb::XContentProvider</code></li>
*  <li> <code>com::sun::star::ucb::XFileIdentifierConverter</code></li>
*  <li> <code>com::sun::star::ucb::XContentIdentifierFactory</code></li>
*  <li> <code>com::sun::star::ucb::XParameterizedContentProvider</code></li>
* </ul>
* This object test <b> is NOT </b> designed to be run in several
* threads concurrently.
* @see com.sun.star.ucb.XContentProvider
* @see com.sun.star.ucb.XFileIdentifierConverter
* @see com.sun.star.ucb.XContentIdentifierFactory
* @see com.sun.star.ucb.XParameterizedContentProvider
* @see ifc.ucb._XContentProvider
* @see ifc.ucb._XFileIdentifierConverter
* @see ifc.ucb._XContentIdentifierFactory
* @see ifc.ucb._XParameterizedContentProvider
*/
public class ContentProvider extends TestCase {

    /**
    * Creating a TestEnvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.ucprmt.ContentProvider</code>. <p>
    *     Object relations created :
    * <ul>
    *  <li> <code>'FACTORY'</code> for
    *      {@link ifc.ucb._XContentProvider} factory for
    *   content identifiers. Here it is
    *   <code>UniversalContentBroker</code> service.</li>
    *  <li> <code>'CONTENT1'</code> for
    *     {@link ifc.ucb._XContentProvider}: suitable
    *   content URL for this provider </li>
    *  <li> <code>'CONTENT2'</code> for
    *     {@link ifc.ucb._XContentProvider}: another suitable
    *   content URL for this provider </li>
    * </ul>
    */
    @Override
    public TestEnvironment createTestEnvironment
            (TestParameters Param, PrintWriter log ) throws Exception {
        XInterface oObj = null;
        XMultiServiceFactory xMSF = Param.getMSF();
        XContentIdentifierFactory cntIDFac = null ;

        oObj = (XInterface) xMSF.createInstance
            ("com.sun.star.ucb.RemoteAccessContentProvider");
        cntIDFac = UnoRuntime.queryInterface
                (XContentIdentifierFactory.class, oObj) ;

        TestEnvironment tEnv = new TestEnvironment( oObj );

        // adding relation for XContentProvider
        tEnv.addObjRelation("FACTORY", cntIDFac) ;
        tEnv.addObjRelation("CONTENT1","vnd.sun.star.wfs:///user/work");
        tEnv.addObjRelation("CONTENT2","vnd.sun.star.wfs:///user/config");

        return tEnv;
    } // finish method getTestEnvironment

}

