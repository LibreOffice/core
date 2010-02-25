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

package mod._sw;

import com.sun.star.container.ElementExistException;
import com.sun.star.container.XNameAccess;
import java.io.PrintWriter;

import lib.StatusException;
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
 * threads concurently.
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
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );
        try {
            log.println( "creating a textdocument" );
            xTextDoc = SOF.createTextDoc( null );
        } catch ( com.sun.star.uno.Exception e ) {
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }
    }

    /**
     * Disposes text document.
     */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xTextDoc " );
        util.DesktopTools.closeDoc(xTextDoc);
    }

    /**
     * Creating a Testenvironment for the interfaces to be tested.
     * Creates an instance of the service
     * <code>com.sun.star.text.AutoTextContainer</code>, then creates a new
     * group into the container.<p>
     *     Object relations created :
     * <ul>
     *  <li> <code>'TextRange'</code> for
     *      {@link ifc.text._XAutoTextGroup} range of text</li>
     * </ul>
     */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XInterface oObj = null;
        XAutoTextContainer oContainer;

        log.println( "creating a test environment" );
        try {
            XMultiServiceFactory myMSF = (XMultiServiceFactory)Param.getMSF();
            Object oInst = myMSF.createInstance("com.sun.star.text.AutoTextContainer");
            oContainer = (XAutoTextContainer) UnoRuntime.queryInterface(XAutoTextContainer.class,oInst);
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create AutoTextContainer", e);
        }
        String myGroupName="myNewGroup2*1";

        XAutoTextContainer xATC = (XAutoTextContainer) UnoRuntime.queryInterface(XAutoTextContainer.class, oContainer);

        try {
            log.println("removing element with name '" + myGroupName + "'");
            xATC.removeByName(myGroupName);
        } catch (com.sun.star.container.NoSuchElementException e) {
        }

        try {
            log.println("adding element with name '" + myGroupName + "'");
            xATC.insertNewByName(myGroupName);
        } catch (ElementExistException ex) {
            ex.printStackTrace(log);
            throw new StatusException("could not insert '"+myGroupName+"' into container",ex);
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            ex.printStackTrace(log);
            throw new StatusException("could not insert '"+myGroupName+"' into container",ex);
        }


        XNameAccess oContNames = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, oContainer);

        if (Param.getBool(util.PropertyName.DEBUG_IS_ACTIVE)){
            String contNames[] = oContNames.getElementNames();
            for (int i =0; i < contNames.length; i++){
                log.println("ContainerNames[ "+ i + "]: " + contNames[i]);
            }
        }

        try{
            oObj = (XInterface) AnyConverter.toObject(new Type(XInterface.class),oContNames.getByName(myGroupName));
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get AutoTextGroup '"+myGroupName + "'", e);
        }

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
