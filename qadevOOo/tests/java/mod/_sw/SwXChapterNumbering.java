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

import com.sun.star.beans.PropertyValue;
import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XChapterNumberingSupplier;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Test for object, which is a collection of chapter numbering rules of document.
 * Object implements the following interfaces:
 * <ul>
 *  <li> <code>com::sun::star::beans::XPropertySet</code></li>
 *  <li> <code>com::sun::star::container::XIndexAccess</code></li>
 *  <li> <code>com::sun::star::container::XElementAccess</code></li>
 *  <li> <code>com::sun::star::text::NumberingRules</code></li>
 * </ul> <p>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurently.
 * @see com.sun.star.beans.XPropertySet
 * @see com.sun.star.container.XIndexAccess
 * @see com.sun.star.container.XElementAccess
 * @see com.sun.star.text.NumberingRules
 * @see ifc.beans._XPropertySet
 * @see ifc.container._XIndexAccess
 * @see ifc.container._XElementAccess
 * @see ifc.text._NumberingRules
 */
public class SwXChapterNumbering extends TestCase {
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
    * Creating a Testenvironment for the interfaces to be tested. Chapter
    * numbering rules are gotten from text document using interface
    * <code>XChapterNumberingSupplier</code>.
    */
    public TestEnvironment createTestEnvironment(
            TestParameters tParam, PrintWriter log ) throws StatusException {
        XInterface oObj = null;

        XChapterNumberingSupplier oCNSupp = (XChapterNumberingSupplier)
            UnoRuntime.queryInterface(XChapterNumberingSupplier.class,xTextDoc);
        oObj = oCNSupp.getChapterNumberingRules();

        PropertyValue[] instance1 = null;
        try {
            instance1 = (PropertyValue[]) oCNSupp.getChapterNumberingRules().getByIndex(1);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace();
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace();
        }

        //instance1[0].Value = new Short((short)5);

        TestEnvironment tEnv = new TestEnvironment( oObj );

        tEnv.addObjRelation("INSTANCE1", instance1);

        return tEnv;

    } // finish method getTestEnvironment

}    // finish class SwXChapterNumbering

