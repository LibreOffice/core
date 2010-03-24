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

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.DefaultDsc;
import util.InstCreator;
import util.SOfficeFactory;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XAutoTextContainer;
import com.sun.star.text.XAutoTextEntry;
import com.sun.star.text.XAutoTextGroup;
import com.sun.star.text.XText;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextRange;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;


/**
 * Test for object which is represented by service
 * <code>com.sun.star.text.AutoTextEntry</code>.<p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::text::XSimpleText</code></li>
 *  <li> <code>com::sun::star::text::XAutoTextEntry</code></li>
 *  <li> <code>com::sun::star::text::XTextRange</code></li>
 *  <li> <code>com::sun::star::text::XText</code></li>
 * </ul> <p>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurently.
 * @see com.sun.star.text.AutoTextEntry
 * @see com.sun.star.text.AutoTextContainer
 * @see com.sun.star.text.XSimpleText
 * @see com.sun.star.text.XAutoTextEntry
 * @see com.sun.star.text.XTextRange
 * @see com.sun.star.text.XText
 * @see ifc.text._XSimpleText
 * @see ifc.text._XAutoTextEntry
 * @see ifc.text._XTextRange
 * @see ifc.text._XText
 */
public class SwXAutoTextEntry extends TestCase {
    XTextDocument xTextDoc;
    XAutoTextGroup oGroup;

    /**
     * Creates text document.
     */
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        XMultiServiceFactory msf = (XMultiServiceFactory) tParam.getMSF();
        SOfficeFactory SOF = SOfficeFactory.getFactory( msf );
        try {
            log.println( "creating a textdocument" );
            xTextDoc = SOF.createTextDoc( null );
        } catch ( com.sun.star.uno.Exception e ) {
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }
    }

    /**
     *  Removes added element from AutoTextGroup
     */
    protected void cleanup( TestParameters Param, PrintWriter log) {
        try {
            if ( oGroup.hasByName("NewEntryName") ) {
                log.println("Removing 'NewEntryName' element");
                oGroup.removeByName("NewEntryName");
            }
        } catch ( com.sun.star.container.NoSuchElementException e ) {
            log.println("Cannot remove TextEntry from group...");
            e.printStackTrace(log);
        }
        log.println( "disposing xTextDoc " );
        util.DesktopTools.closeDoc(xTextDoc);
    }


    /**
     * Creating a Testenvironment for the interfaces to be tested.
     * Creates an instance of the service
     * <code>com.sun.star.text.AutoTextContainer</code>, then selects the 'mytexts'
     * group from the given container using <code>XNameAccess</code> interface,
     * and inserts some text entry to this group. Then entry passed as test
     * component.<p>
     *     Object relations created :
     * <ul>
     *  <li><code>'XTEXTINFO'</code> for
     *    {@link ifc.text._XText} : creates tables 6x4</li>
     *  <li><code>'TEXTDOC'</code> for
     *    {@link ifc.text._XAutoTextEntry} : text document</li>
     * </ul>
     */
    protected synchronized TestEnvironment createTestEnvironment
            (TestParameters Param, PrintWriter log) {

        XAutoTextEntry oEntry = null;
        XAutoTextContainer oContainer;
        XInterface oObj = null;
        int n = 0;
        int nCount = 0;

        log.println( "creating a test environment" );
        try {
            XMultiServiceFactory myMSF = (XMultiServiceFactory)Param.getMSF();
            Object oInst = myMSF.createInstance
                    ("com.sun.star.text.AutoTextContainer");
            oContainer = (XAutoTextContainer)
            UnoRuntime.queryInterface(XAutoTextContainer.class,oInst);
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create AutoTextContainer", e);
        }

        XNameAccess oContNames = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, oContainer);

        String contNames[] = oContNames.getElementNames();
        for (int i =0; i < contNames.length; i++){
            log.println("ContainerNames[ "+ i + "]: " + contNames[i]);
        }

        try{
            oObj = (XInterface) AnyConverter.toObject(new Type(XInterface.class),oContNames.getByName("mytexts"));
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get AutoTextGroup", e);
        }

        oGroup = (XAutoTextGroup) UnoRuntime.queryInterface
                (XAutoTextGroup.class, oObj);
        String[] oENames = oGroup.getElementNames();
        for (int i=0; i<oENames.length; i++) {
            log.println("AutoTextEntryNames[" + i + "]: " + oENames[i]);
        }

        XText oText = xTextDoc.getText();
        oText.insertString(oText.getStart(), "New AutoText", true);
        XTextRange oTextRange = (XTextRange) oText;
        try {
            if ( oGroup.hasByName("NewEntryName") ) {
                oGroup.removeByName("NewEntryName");
                log.println("Element 'NewEntryName' exists, removing...");
            }
            log.println("Adding new element 'NewEntryName' to group...");
            oGroup.insertNewByName("NewEntryName", "NewEntryTitle", oTextRange);
            oEntry = (XAutoTextEntry) AnyConverter.toObject(
                    new Type(XAutoTextEntry.class),oGroup.getByName("NewEntryName"));
        } catch ( com.sun.star.container.ElementExistException e ) {
            e.printStackTrace(log);
        } catch ( com.sun.star.container.NoSuchElementException e ) {
            e.printStackTrace(log);
        } catch ( com.sun.star.lang.WrappedTargetException e ) {
            e.printStackTrace(log);
        } catch ( com.sun.star.lang.IllegalArgumentException e ) {
            e.printStackTrace(log);
        }

        oObj = oEntry;

        log.println("Trying to use XText as TextRange in the method applyTo");
        oEntry.applyTo(oTextRange);

        oTextRange = oText.createTextCursor();
        log.println("Trying to use XTextCursor as TextRange in the method applyTo");
        oEntry.applyTo(oTextRange);

        log.println( "creating a new environment for AutoTextEntry object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        // adding relation for XText
        DefaultDsc tDsc = new DefaultDsc("com.sun.star.text.XTextContent",
                "com.sun.star.text.TextField.DateTime");
        log.println( "    adding InstCreator object" );
        tEnv.addObjRelation( "XTEXTINFO", new InstCreator( xTextDoc, tDsc ) );

        log.println( "adding TextDocument as mod relation to environment" );
        tEnv.addObjRelation("TEXTDOC", xTextDoc);

        return tEnv;
    } // finish method getTestEnvironment


}    // finish class SwXAutoTextEntry
