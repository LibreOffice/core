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

    protected void initialize( TestParameters tParam, PrintWriter log ) {
    SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );

    try {
        log.println( "creating a textdocument" );
        xTextDoc = SOF.createTextDoc( null );
    } catch ( com.sun.star.uno.Exception e ) {
        // Some exception occures.FAILED
        e.printStackTrace( log );
        throw new StatusException( "Couldn't create document", e );
    }
    }

    protected void cleanup( TestParameters tParam, PrintWriter log ) {
    log.println( "    disposing xTextDoc " );
    util.DesktopTools.closeDoc(xTextDoc);
    }

    /**
     *    creating a Testenvironment for the interfaces to be tested
     */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XInterface oObj = null;
        XInterface oTS = null;
        XTextSection xTS = null;
        XText oText = null;
        XTextColumns TC = null;
        Object instance = null;

        log.println( "creating a test environment" );

        oText = xTextDoc.getText();
            XTextCursor oCursor = oText.createTextCursor();

        XMultiServiceFactory oDocMSF = (XMultiServiceFactory)
            UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDoc);

        try {
            XTextSectionsSupplier oTSSupp = (XTextSectionsSupplier)
            UnoRuntime.queryInterface
                    ( XTextSectionsSupplier.class, xTextDoc );
            XNameAccess oTSSuppName = oTSSupp.getTextSections();

            //cleanup if necessary
            if (oTSSuppName.hasByName("SwXTextSection")) {
                XTextSection old = (XTextSection) AnyConverter.toObject(
                    new Type(XTextSection.class),
                        oTSSuppName.getByName("SwXTextSection"));
                XComponent oldC = (XComponent)
                        UnoRuntime.queryInterface(XComponent.class,old);
                oldC.dispose();
                oText.setString("");
            }

            //insert two sections parent and child
            oTS = (XInterface) oDocMSF.createInstance
                    ("com.sun.star.text.TextSection");
            instance = oDocMSF.createInstance("com.sun.star.text.TextSection");
            XTextContent oTSC = (XTextContent)
                    UnoRuntime.queryInterface(XTextContent.class, oTS);
            oText.insertTextContent(oCursor, oTSC, false);
            XWordCursor oWordC = (XWordCursor)
                UnoRuntime.queryInterface(XWordCursor.class, oCursor);
            oCursor.setString("End of TextSection");
            oCursor.gotoStart(false);
            oCursor.setString("Start of TextSection ");
            oWordC.gotoEndOfWord(false);
            XInterface oTS2 = (XInterface) oDocMSF.createInstance
                ("com.sun.star.text.TextSection");
            oTSC = (XTextContent)UnoRuntime.queryInterface(XTextContent.class, oTS2);
            oText.insertTextContent(oCursor, oTSC, false);

            XIndexAccess oTSSuppIndex = (XIndexAccess)
            UnoRuntime.queryInterface(XIndexAccess.class, oTSSuppName);

            log.println( "getting a TextSection with the XTextSectionSupplier()" );
            xTS = (XTextSection) AnyConverter.toObject(
                        new Type(XTextSection.class),oTSSuppIndex.getByIndex(0));
            XNamed xTSName = (XNamed)
                    UnoRuntime.queryInterface( XNamed.class, xTS);
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

        tEnv.addObjRelation("TRO",new Boolean(true));

        try {
            TC = (XTextColumns) UnoRuntime.queryInterface(XTextColumns.class,
                    oDocMSF.createInstance("com.sun.star.text.TextColumns"));
        } catch ( com.sun.star.uno.Exception e ) {
            e.printStackTrace(log);
            throw new StatusException
                    ("Couldn't create instance of service TextColumns", e );
        }
        tEnv.addObjRelation("TC",TC);

        tEnv.addObjRelation("CONTENT", (XTextContent)
                        UnoRuntime.queryInterface(XTextContent.class,instance));
        tEnv.addObjRelation("RANGE", xTextDoc.getText().createTextCursor());

        return tEnv;
    } // finish method getTestEnvironment

}// finish class SwXTextSection


