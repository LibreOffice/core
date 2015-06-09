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

import com.sun.star.beans.Property;
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

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.DesktopTools;
import util.SOfficeFactory;
import util.utils;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.style.PageStyle</code>. <p>
 * @see com.sun.star.style.PageStyle
 */
public class PageStyle extends TestCase {

    XTextDocument xTextDoc;
    SOfficeFactory SOF = null;

    /**
    * Creates text document.
    */
    @Override
    protected void initialize( TestParameters tParam, PrintWriter log ) throws Exception {
        SOF = SOfficeFactory.getFactory( tParam.getMSF() );
        log.println( "creating a textdocument" );
        xTextDoc = SOF.createTextDoc( null );
    }

    /**
    * Disposes text document.
    */
    @Override
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xTextDoc " );
        DesktopTools.closeDoc(xTextDoc);
    }

    @Override
    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) throws Exception {
        TestEnvironment tEnv = null;
        XNameAccess oSFNA = null;
        XStyle oStyle = null;
        XStyle oMyStyle = null;

        log.println("creating a test environment");

        log.println("getting style");
        XStyleFamiliesSupplier oSFS = UnoRuntime.queryInterface(XStyleFamiliesSupplier.class,
        xTextDoc);
        XNameAccess oSF = oSFS.getStyleFamilies();
        oSFNA = UnoRuntime.queryInterface(
                    XNameAccess.class,oSF.getByName("PageStyles"));    // get the page style
        XIndexAccess oSFIA = UnoRuntime.queryInterface(XIndexAccess.class, oSFNA);
        oStyle = UnoRuntime.queryInterface(
                    XStyle.class,oSFIA.getByIndex(0));
        log.println("Chosen pool style: "+oStyle.getName());

        log.print("Creating a user-defined style... ");
        XMultiServiceFactory oMSF = UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDoc);
        XInterface oInt = (XInterface)
            oMSF.createInstance("com.sun.star.style.PageStyle");
        oMyStyle = UnoRuntime.queryInterface(XStyle.class, oInt);


        if (oMyStyle == null)
            log.println("FAILED");
        else
            log.println("OK");

        XNameContainer oSFNC = UnoRuntime.queryInterface(XNameContainer.class, oSFNA);


        if ( oSFNC.hasByName("My Style") )
            oSFNC.removeByName("My Style");
        oSFNC.insertByName("My Style", oMyStyle);

        XText oText = xTextDoc.getText();
        XTextCursor oCursor = oText.createTextCursor();
        XPropertySet xProp = UnoRuntime.queryInterface(XPropertySet.class, oCursor);
        Property[] props = xProp.getPropertySetInfo().getProperties();
        for (int i=0; i<props.length; i++)
            System.out.println("# Property: " + props[i].Name + "    val: " + props[i].Type.toString() + "   attr: " + props[i].Attributes);
        xProp.setPropertyValue("PageDescName", oMyStyle.getName());

//        oMyStyle = oStyle;
        log.println("creating a new environment for object");
        tEnv = new TestEnvironment(oMyStyle);
        tEnv.addObjRelation("PoolStyle", oStyle);

        tEnv.addObjRelation("FollowStyle", "Envelope");
        XPropertySet xStyleProp = UnoRuntime.queryInterface(XPropertySet.class, oMyStyle);

        short exclude = PropertyAttribute.READONLY;
        String[] names = utils.getFilteredPropertyNames(xStyleProp, (short)0, exclude);
        tEnv.addObjRelation("PropertyNames", names);

        return tEnv;
    }

}
