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
import util.SOfficeFactory;

import com.sun.star.container.XNameAccess;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XText;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextSectionsSupplier;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 *
 * initial description
 * @see com.sun.star.text.XText
 *
 */
public class SwXTextSections extends TestCase {
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
    public TestEnvironment createTestEnvironment
        (TestParameters Param, PrintWriter log ) throws Exception {

        XInterface oObj = null;
        XInterface oTS = null;
        XNameAccess oTSSuppName = null;

        XText oText = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "creating a test environment" );


        oText = xTextDoc.getText();
        XTextCursor oCursor = oText.createTextCursor();


        log.println( "inserting TextSections" );

        XMultiServiceFactory oDocMSF = UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDoc);

        // First TextSection
        oTS = (XInterface) oDocMSF.createInstance
            ("com.sun.star.text.TextSection");
        XTextContent oTSC = UnoRuntime.queryInterface(XTextContent.class, oTS);
        oText.insertTextContent(oCursor, oTSC, false);

        // Second TextSection
        oTS = (XInterface) oDocMSF.createInstance
            ("com.sun.star.text.TextSection");
        XTextContent oTSC2 = UnoRuntime.queryInterface(XTextContent.class, oTS);
        oText.insertTextContent(oCursor, oTSC2, false);


        log.println( "try to get a TextSection with the XTextSectionSupplier()" );

        try{
            XTextSectionsSupplier oTSSupp = UnoRuntime.queryInterface( XTextSectionsSupplier.class,
            xTextDoc );
            oTSSuppName = oTSSupp.getTextSections();
        }
        catch(Exception e){
            System.out.println("Couldn't get Textsection " + e);
        }


        oObj = oTSSuppName;

        log.println( "creating a new environment for TextSections object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        log.println( "adding TextDocument as mod relation to environment" );
        tEnv.addObjRelation("TEXTDOC", xTextDoc);

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class SwXTextSection
