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

import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNameContainer;
import com.sun.star.lang.XComponent;
import com.sun.star.style.XStyleFamiliesSupplier;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.style.StyleFamily</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::container::XNameContainer</code></li>
 *  <li> <code>com::sun::star::container::XNameAccess</code></li>
 *  <li> <code>com::sun::star::container::XElementAccess</code></li>
 *  <li> <code>com::sun::star::container::XIndexAccess</code></li>
 *  <li> <code>com::sun::star::container::XNameReplace</code></li>
 * </ul> <p>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurrently.
 * @see com.sun.star.container.XNameContainer
 * @see com.sun.star.container.XNameAccess
 * @see com.sun.star.container.XElementAccess
 * @see com.sun.star.container.XIndexAccess
 * @see com.sun.star.container.XNameReplace
 * @see ifc.container._XNameContainer
 * @see ifc.container._XNameAccess
 * @see ifc.container._XElementAccess
 * @see ifc.container._XIndexAccess
 * @see ifc.container._XNameReplace
 */
public class SwXStyleFamily extends TestCase {
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
    * Creating a TestEnvironment for the interfaces to be tested. At first,
    * style families of text document are gotten using
    * <code>XStyleFamiliesSupplier</code> interface, then family indexed '0' is
    * gotten from previously obtained style families collection using
    * <code>XIndexAccess</code> interface. Finally, method creates an instance
    * of the service <code>com.sun.star.style.CharacterStyle</code> and inserts
    * it to a gotten style family as 'SwXStyleFamily' using
    * <code>XNameContainer</code> interface.
    *     Object relations created :
    * <ul>
    *  <li> <code>'XNameReplaceINDEX'</code> for
    *      {@link ifc.container._XNameReplace} : number of last taken instance
    *      of <code>com.sun.star.style.CharacterStyle</code>, when multithread
    *      testing is going.</li>
    *  <li> <code>'NAMEREPLACE'</code> for
    *      {@link ifc.container._XNameReplace} : name of style family, inserted
    *      to style families of a text document.</li>
    *  <li> <code>'INSTANCEn'</code> for
    *      {@link ifc.container._XIndexContainer},
    *      {@link ifc.container._XIndexReplace},
    *      {@link ifc.container._XNameContainer},
    *      {@link ifc.container._XNameReplace} : several relations, which are
    *      represented by instances of service
    *      <code>com.sun.star.style.CharacterStyle</code>.</li>
    * </ul>
    */
    @Override
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) throws Exception {

        XNameAccess oSFNA = null;

        log.println( "Creating Test Environment..." );

        SOfficeFactory SOF = SOfficeFactory.getFactory( Param.getMSF());
        XComponent xComp = UnoRuntime.queryInterface(XComponent.class, xTextDoc);
        XInterface oInstance = (XInterface)
            SOF.createInstance(xComp, "com.sun.star.style.CharacterStyle");
        XStyleFamiliesSupplier oSFsS = UnoRuntime.queryInterface(XStyleFamiliesSupplier.class, xTextDoc);
        XNameAccess oSF = oSFsS.getStyleFamilies();
        XIndexAccess oSFIA = UnoRuntime.queryInterface(XIndexAccess.class, oSF);

        oSFNA = (XNameAccess) AnyConverter.toObject(
                    new Type(XNameAccess.class),oSFIA.getByIndex(0));

        XNameContainer oContainer = UnoRuntime.queryInterface(XNameContainer.class, oSFNA);

        // insert a Style which can be replaced by name
        try {
            oContainer.insertByName("SwXStyleFamily",oInstance);
        } catch ( com.sun.star.lang.WrappedTargetException e ) {
            log.println("Could not insert style family.");
            e.printStackTrace(log);
        } catch ( com.sun.star.lang.IllegalArgumentException e ) {
            log.println("Could not insert style family.");
            e.printStackTrace(log);
        } catch ( com.sun.star.container.ElementExistException e ) {
            log.println("Could not insert style family.");
            e.printStackTrace(log);
        }

        TestEnvironment tEnv = new TestEnvironment(oSFNA);

        int THRCNT = 1;
        if ((String)Param.get("THRCNT") != null) {
            THRCNT = Integer.parseInt((String)Param.get("THRCNT"));
        }
        String nr = Integer.toString(THRCNT+1);

        log.println( "adding NameReplaceIndex as mod relation to environment" );
        tEnv.addObjRelation("XNameReplaceINDEX", nr);

        for (int n=1; n<(THRCNT+3); n++ ) {
            log.println( "adding INSTANCE"+n+" as mod relation to environment");
            tEnv.addObjRelation("INSTANCE"+n,
                SOF.createInstance(xComp,"com.sun.star.style.CharacterStyle"));
        }

        log.println("adding NAMEREPLACE as mod relation to environment");
        tEnv.addObjRelation("NAMEREPLACE", "SwXStyleFamily");

        return tEnv;
    }

}    // finish class SwXStyleFamily
