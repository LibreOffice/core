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
import util.InstCreator;
import util.SOfficeFactory;
import util.TableDsc;

import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNamed;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XText;
import com.sun.star.text.XTextColumns;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextSection;
import com.sun.star.text.XTextSectionsSupplier;
import com.sun.star.text.XWordCursor;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 *
 * initial description
 * @see com.sun.star.text.XText
 *
 */

public class SwXTextSection extends TestCase {
    XTextDocument xTextDoc;

    @Override
    protected void initialize( TestParameters tParam, PrintWriter log ) throws Exception {
        SOfficeFactory SOF = SOfficeFactory.getFactory( tParam.getMSF() );
        log.println( "creating a textdocument" );
        xTextDoc = SOF.createTextDoc( null );
    }

    @Override
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
    log.println( "    disposing xTextDoc " );
    util.DesktopTools.closeDoc(xTextDoc);
    }

    /**
     *    creating a TestEnvironment for the interfaces to be tested
     */
    @Override
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) throws Exception {

        XInterface oObj = null;
        XInterface oTS = null;
        XTextSection xTS = null;
        XText oText = null;
        XTextColumns TC = null;
        Object instance = null;

        log.println( "creating a test environment" );

        oText = xTextDoc.getText();
            XTextCursor oCursor = oText.createTextCursor();

        XMultiServiceFactory oDocMSF = UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDoc);

        try {
            XTextSectionsSupplier oTSSupp = UnoRuntime.queryInterface
                    ( XTextSectionsSupplier.class, xTextDoc );
            XNameAccess oTSSuppName = oTSSupp.getTextSections();

            //cleanup if necessary
            if (oTSSuppName.hasByName("SwXTextSection")) {
                XTextSection old = (XTextSection) AnyConverter.toObject(
                    new Type(XTextSection.class),
                        oTSSuppName.getByName("SwXTextSection"));
                XComponent oldC = UnoRuntime.queryInterface(XComponent.class,old);
                oldC.dispose();
                oText.setString("");
            }

            //insert two sections parent and child
            oTS = (XInterface) oDocMSF.createInstance
                    ("com.sun.star.text.TextSection");
            instance = oDocMSF.createInstance("com.sun.star.text.TextSection");
            XTextContent oTSC = UnoRuntime.queryInterface(XTextContent.class, oTS);
            oText.insertTextContent(oCursor, oTSC, false);
            XWordCursor oWordC = UnoRuntime.queryInterface(XWordCursor.class, oCursor);
            oCursor.setString("End of TextSection");
            oCursor.gotoStart(false);
            oCursor.setString("Start of TextSection ");
            oWordC.gotoEndOfWord(false);
            XInterface oTS2 = (XInterface) oDocMSF.createInstance
                ("com.sun.star.text.TextSection");
            oTSC = UnoRuntime.queryInterface(XTextContent.class, oTS2);
            oText.insertTextContent(oCursor, oTSC, false);

            XIndexAccess oTSSuppIndex = UnoRuntime.queryInterface(XIndexAccess.class, oTSSuppName);

            log.println( "getting a TextSection with the XTextSectionSupplier()" );
            xTS = (XTextSection) AnyConverter.toObject(
                        new Type(XTextSection.class),oTSSuppIndex.getByIndex(0));
            XNamed xTSName = UnoRuntime.queryInterface( XNamed.class, xTS);
            xTSName.setName("SwXTextSection");
        }
        catch(Exception e){
            System.out.println("Couldn't get Textsection " + e);
        }

        oObj = xTS;

        log.println( "creating a new environment for TextSection object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        log.println( "adding InstDescriptor object" );
        TableDsc tDsc = new TableDsc( 6, 4 );

        log.println( "adding InstCreator object" );
        tEnv.addObjRelation( "XTEXTINFO", new InstCreator( xTextDoc, tDsc ) );

        tEnv.addObjRelation("TRO",Boolean.TRUE);

        TC = UnoRuntime.queryInterface(XTextColumns.class,
                oDocMSF.createInstance("com.sun.star.text.TextColumns"));
        tEnv.addObjRelation("TC",TC);

        tEnv.addObjRelation("CONTENT", UnoRuntime.queryInterface(XTextContent.class,instance));
        tEnv.addObjRelation("RANGE", xTextDoc.getText().createTextCursor());

        return tEnv;
    } // finish method getTestEnvironment

}// finish class SwXTextSection


