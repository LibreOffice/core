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

import com.sun.star.beans.PropertyAttribute;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNameContainer;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.style.XStyle;
import com.sun.star.style.XStyleFamiliesSupplier;
import com.sun.star.text.XText;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import java.io.PrintWriter;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.DesktopTools;
import util.SOfficeFactory;
import util.utils;


/**
 * Test for object which is represented by service
 * <code>com.sun.star.style.ParagraphStyle</code>. <p>
 * @see com.sun.star.style.ParagraphStyle
 */
public class ParagraphStyle extends TestCase  {

    XTextDocument xTextDoc;
    SOfficeFactory SOF = null;

    /**
    * Creates text document.
    */
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );
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
        DesktopTools.closeDoc(xTextDoc);
    }

    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) {
        TestEnvironment tEnv = null;
        XNameAccess oSFNA = null;
        XStyle oStyle = null;
        XStyle oMyStyle = null;

        log.println("creating a test environment");

        try {
            log.println("getting style");
            XStyleFamiliesSupplier oSFS = (XStyleFamiliesSupplier)
                UnoRuntime.queryInterface(XStyleFamiliesSupplier.class,
                xTextDoc);
            XNameAccess oSF = oSFS.getStyleFamilies();
            oSFNA = (XNameAccess) UnoRuntime.queryInterface(
                            XNameAccess.class,oSF.getByName("ParagraphStyles"));
            XIndexAccess oSFIA = (XIndexAccess)
                UnoRuntime.queryInterface(XIndexAccess.class, oSFNA);
            String[] els = oSFNA.getElementNames();
            oStyle = (XStyle) UnoRuntime.queryInterface(
                           XStyle.class,oSFIA.getByIndex(1));
        } catch ( com.sun.star.lang.WrappedTargetException e ) {
            log.println("Error: exception occurred.");
            e.printStackTrace(log);
            throw new StatusException( "Couldn't create environment ", e );
        } catch ( com.sun.star.lang.IndexOutOfBoundsException e ) {
            log.println("Error: exception occurred.");
            e.printStackTrace(log);
            throw new StatusException( "Couldn't create environment ", e );
        } catch ( com.sun.star.container.NoSuchElementException e ) {
            log.println("Error: exception occurred.");
            e.printStackTrace(log);
            throw new StatusException( "Couldn't create environment ", e );
        }

        try {
            log.print("Creating a user-defined style... ");
            XMultiServiceFactory oMSF = (XMultiServiceFactory)
                UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDoc);
            XInterface oInt = (XInterface)
                oMSF.createInstance("com.sun.star.style.ParagraphStyle");
            oMyStyle = (XStyle) UnoRuntime.queryInterface(XStyle.class, oInt);
        } catch ( com.sun.star.uno.Exception e ) {
            log.println("Error: exception occurred.");
            e.printStackTrace(log);
            throw new StatusException( "Couldn't create environment ", e );
        }


        if (oMyStyle == null)
            log.println("FAILED");
        else
            log.println("OK");
            XNameContainer oSFNC = (XNameContainer)
            UnoRuntime.queryInterface(XNameContainer.class, oSFNA);

        try {
            if ( oSFNC.hasByName("My Style") )
                oSFNC.removeByName("My Style");
            oSFNC.insertByName("My Style", oMyStyle);
        } catch ( com.sun.star.lang.WrappedTargetException e ) {
            e.printStackTrace(log);
            throw new StatusException( "Couldn't create environment ", e );
        } catch     ( com.sun.star.lang.IllegalArgumentException e ) {
            e.printStackTrace(log);
            throw new StatusException( "Couldn't create environment ", e );
        } catch ( com.sun.star.container.NoSuchElementException e ) {
            e.printStackTrace(log);
            throw new StatusException( "Couldn't create environment ", e );
        } catch ( com.sun.star.container.ElementExistException e ) {
            e.printStackTrace(log);
            throw new StatusException( "Couldn't create environment ", e );
        }

        XText oText = xTextDoc.getText();
        XTextCursor oCursor = oText.createTextCursor();
        XPropertySet xProp = (XPropertySet)
            UnoRuntime.queryInterface(XPropertySet.class, oCursor);

        try {
            xProp.setPropertyValue("ParaStyleName", oMyStyle.getName());
        } catch ( com.sun.star.lang.WrappedTargetException e ) {
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create environment ", e );
        } catch ( com.sun.star.lang.IllegalArgumentException e ) {
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create environment ", e );
        } catch ( com.sun.star.beans.PropertyVetoException e ) {
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create environment ", e );
        } catch ( com.sun.star.beans.UnknownPropertyException e ) {
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create environment ", e );
        }

        log.println("creating a new environment for object");
        tEnv = new TestEnvironment(oMyStyle);
        tEnv.addObjRelation("PoolStyle", oStyle);

        XPropertySet xStyleProp = (XPropertySet)
            UnoRuntime.queryInterface(XPropertySet.class, oMyStyle);
        short exclude = PropertyAttribute.MAYBEVOID + PropertyAttribute.READONLY;
        String[] names = utils.getFilteredPropertyNames(xStyleProp, (short)0, exclude);
        tEnv.addObjRelation("PropertyNames", names);

        return tEnv;
    }

}
