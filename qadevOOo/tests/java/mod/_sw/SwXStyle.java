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
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.style.Style</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::container::XNamed</code></li>
 *  <li> <code>com::sun::star::style::Style</code></li>
 *  <li> <code>com::sun::star::style::XStyle</code></li>
 * </ul> <p>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurrently.
 * @see com.sun.star.container.XNamed
 * @see com.sun.star.style.Style
 * @see com.sun.star.style.XStyle
 * @see ifc.container._XNamed
 * @see ifc.style._Style
 * @see ifc.style._XStyle
 */
public class SwXStyle extends TestCase {
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
        util.DesktopTools.closeDoc(xTextDoc);
    }

    /**
    * Creating a TestEnvironment for the interfaces to be tested.
    * At first style families are gotten from a text document using
    * <code>XStyleFamiliesSupplier</code> interface, then family indexed '0' is
    * gotten from this style family using <code>XIndexAccess</code> interface.
    * Next, method creates an instance of the service
    * <code>com.sun.star.style.CharacterStyle</code> and inserts it to a
    * previously obtained style family using <code>XNameContainer</code>
    * interface. Finally, method creates a cursor of a major text of text
    * document and sets it's property 'CharStyleName' value to the name of
    * previously created style's name.
    *     Object relations created :
    * <ul>
    *  <li> <code>'PoolStyle'</code> for
    *      {@link ifc.style._XStyle} : style indexed '10' obtained from
    *  StyleFamily indexed '0' from text document using
    *  <code>XIndexAccess</code> interface.</li>
    * </ul>
    */
    @Override
    protected TestEnvironment createTestEnvironment
            (TestParameters Param, PrintWriter log) throws Exception {

        TestEnvironment tEnv = null;
        XNameAccess oSFNA = null;
        XStyle oStyle = null;
        XStyle oMyStyle = null;

        log.println("creating a test environment");

        log.println("getting style");
        XStyleFamiliesSupplier oSFS = UnoRuntime.queryInterface(XStyleFamiliesSupplier.class,
        xTextDoc);
        XNameAccess oSF = oSFS.getStyleFamilies();
        XIndexAccess oSFsIA = UnoRuntime.queryInterface(XIndexAccess.class, oSF);
        oSFNA = (XNameAccess) AnyConverter.toObject(
                    new Type(XNameAccess.class),oSFsIA.getByIndex(0));
        XIndexAccess oSFIA = UnoRuntime.queryInterface(XIndexAccess.class, oSFNA);
        oStyle = (XStyle) AnyConverter.toObject(
                new Type(XStyle.class),oSFIA.getByIndex(10));

        log.print("Creating a user-defined style... ");
        XMultiServiceFactory oMSF = UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDoc);
        XInterface oInt = (XInterface)
            oMSF.createInstance("com.sun.star.style.CharacterStyle");
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

        xProp.setPropertyValue("CharStyleName", oMyStyle.getName());

        log.println("creating a new environment for object");
        tEnv = new TestEnvironment(oMyStyle);
        tEnv.addObjRelation("PoolStyle", oStyle);

        XPropertySet xStyleProp = UnoRuntime.queryInterface(XPropertySet.class, oMyStyle);
        tEnv.addObjRelation("PropertyNames",getPropertyNames(xStyleProp));

        return tEnv;
    }

    public String[] getPropertyNames(XPropertySet props) {
        Property[] the_props = props.getPropertySetInfo().getProperties();
        String[] names = new String[the_props.length];
        String placebo = "";
        for (int i=0;i<the_props.length;i++) {
            boolean isWritable =
                ((the_props[i].Attributes & PropertyAttribute.READONLY) == 0);
            if (isWritable) placebo=the_props[i].Name;
        }
        for (int i=0;i<the_props.length;i++) {
            boolean isWritable =
                ((the_props[i].Attributes & PropertyAttribute.READONLY) == 0);
            if (isWritable) {
                names[i]=the_props[i].Name;
            } else {
                names[i] = placebo;
            }
        }
        return names;
    }

}    // finish class SwXStyle
