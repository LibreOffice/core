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

package mod._sw;

import com.sun.star.container.XNameAccess;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XAutoTextContainer;
import com.sun.star.text.XText;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

import util.utils;


/**
 * Test for object which is represented by service
 * <code>com.sun.star.text.AutoTextGroup</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::container::XNamed</code></li>
 *  <li> <code>com::sun::star::container::XNameAccess</code></li>
 *  <li> <code>com::sun::star::container::XIndexAccess</code></li>
 *  <li> <code>com::sun::star::container::XElementAccess</code></li>
 *  <li> <code>com::sun::star::text::XAutoTextGroup</code></li>
 * </ul> <p>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurrently.
 * @see com.sun.star.container.XNamed
 * @see com.sun.star.container.XNameAccess
 * @see com.sun.star.container.XIndexAccess
 * @see com.sun.star.container.XElementAccess
 * @see com.sun.star.text.XAutoTextGroup
 * @see com.sun.star.text.AutoTextContainer
 * @see ifc.container._XNamed
 * @see ifc.container._XNameAccess
 * @see ifc.container._XIndexAccess
 * @see ifc.container._XElementAccess
 * @see ifc.text._XAutoTextGroup
 */
public class SwXAutoTextGroup extends TestCase {
    XTextDocument xTextDoc;

    /**
     * Creates text document.
     */
    @Override
    protected void initialize( TestParameters tParam, PrintWriter log ) throws Exception {
        SOfficeFactory SOF = SOfficeFactory.getFactory( tParam.getMSF() );
        log.println( "creating a textdocument" );
        xTextDoc = SOF.createTextDoc( null );
    }

    /**
     * Disposes text document.
     */
    @Override
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xTextDoc " );
        util.DesktopTools.closeDoc(xTextDoc);
    }

    /**
     * Creating a TestEnvironment for the interfaces to be tested.
     * Creates an instance of the service
     * <code>com.sun.star.text.AutoTextContainer</code>, then creates a new
     * group into the container.<p>
     *     Object relations created :
     * <ul>
     *  <li> <code>'TextRange'</code> for
     *      {@link ifc.text._XAutoTextGroup} range of text</li>
     * </ul>
     */
    @Override
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) throws Exception {

        XInterface oObj = null;
        XAutoTextContainer oContainer;

        log.println( "creating a test environment" );
        XMultiServiceFactory myMSF = Param.getMSF();
        Object oInst = myMSF.createInstance("com.sun.star.text.AutoTextContainer");
        oContainer = UnoRuntime.queryInterface(XAutoTextContainer.class,oInst);
        String myGroupName="myNewGroup2*1";

        XAutoTextContainer xATC = UnoRuntime.queryInterface(XAutoTextContainer.class, oContainer);

        try {
            log.println("removing element with name '" + myGroupName + "'");
            xATC.removeByName(myGroupName);
        } catch (com.sun.star.container.NoSuchElementException e) {
        }

        log.println("adding element with name '" + myGroupName + "'");
        xATC.insertNewByName(myGroupName);

        XNameAccess oContNames = UnoRuntime.queryInterface(XNameAccess.class, oContainer);

        if (Param.getBool(util.PropertyName.DEBUG_IS_ACTIVE)){
            String contNames[] = oContNames.getElementNames();
            for (int i =0; i < contNames.length; i++){
                log.println("ContainerNames[ "+ i + "]: " + contNames[i]);
            }
        }

        oObj = (XInterface) AnyConverter.toObject(new Type(XInterface.class),oContNames.getByName(myGroupName));

        log.println("ImplementationName " + utils.getImplName(oObj));

        log.println( "creating a new environment for AutoTextGroup object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        XText oText = xTextDoc.getText();
        oText.insertString(oText.getStart(), "New AutoText", true);

        log.println( "adding TextRange as mod relation to environment" );
        tEnv.addObjRelation("TextRange", oText);

        return tEnv;
    } // finish method getTestEnvironment


}    // finish class SwXAutoTextGroup
