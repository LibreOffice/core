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

import java.io.PrintWriter;

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
 * threads concurrently.
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
    @Override
    protected void initialize( TestParameters tParam, PrintWriter log ) throws Exception {
        XMultiServiceFactory msf = tParam.getMSF();
        SOfficeFactory SOF = SOfficeFactory.getFactory( msf );
        log.println( "creating a textdocument" );
        xTextDoc = SOF.createTextDoc( null );
    }

    /**
     *  Removes added element from AutoTextGroup
     */
    @Override
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
     * Creating a TestEnvironment for the interfaces to be tested.
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
    @Override
    protected TestEnvironment createTestEnvironment
            (TestParameters Param, PrintWriter log) throws Exception {

        XAutoTextEntry oEntry = null;
        XAutoTextContainer oContainer;
        XInterface oObj = null;
        log.println( "creating a test environment" );
        XMultiServiceFactory myMSF = Param.getMSF();
        Object oInst = myMSF.createInstance
                ("com.sun.star.text.AutoTextContainer");
        oContainer = UnoRuntime.queryInterface(XAutoTextContainer.class,oInst);

        XNameAccess oContNames = UnoRuntime.queryInterface(XNameAccess.class, oContainer);

        String contNames[] = oContNames.getElementNames();
        for (int i =0; i < contNames.length; i++){
            log.println("ContainerNames[ "+ i + "]: " + contNames[i]);
        }

        oObj = (XInterface) AnyConverter.toObject(new Type(XInterface.class),oContNames.getByName("mytexts"));

        oGroup = UnoRuntime.queryInterface
                (XAutoTextGroup.class, oObj);
        String[] oENames = oGroup.getElementNames();
        for (int i=0; i<oENames.length; i++) {
            log.println("AutoTextEntryNames[" + i + "]: " + oENames[i]);
        }

        XText oText = xTextDoc.getText();
        oText.insertString(oText.getStart(), "New AutoText", true);
        XTextRange oTextRange = oText;
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
