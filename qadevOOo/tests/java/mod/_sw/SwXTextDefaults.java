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

//API Interfaces
import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.text.Defaults</code> created by a writer
 * document's <code>XMultiServiceFactory</code>. <p>
 *
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::style::ParagraphProperties</code></li>
 *  <li> <code>com::sun::star::style::CharacterPropertiesComplex</code></li>
 *  <li> <code>com::sun::star::style::CharacterPropertiesAsian</code></li>
 *  <li> <code>com::sun::star::text::Defaults</code></li>
 *  <li> <code>com::sun::star::style::CharacterProperties</code></li>
 *  <li> <code>com::sun::star::beans::XPropertySet</code></li>
 *  <li> <code>com::sun::star::style::ParagraphPropertiesComplex</code></li>
 * </ul> <p>
 *
 * @see com.sun.star.style.ParagraphProperties
 * @see com.sun.star.style.CharacterPropertiesComplex
 * @see com.sun.star.style.CharacterPropertiesAsian
 * @see com.sun.star.text.Defaults
 * @see com.sun.star.style.CharacterProperties
 * @see com.sun.star.beans.XPropertySet
 * @see com.sun.star.style.ParagraphPropertiesComplex
 * @see ifc.style._ParagraphProperties
 * @see ifc.style._CharacterPropertiesComplex
 * @see ifc.style._CharacterPropertiesAsian
 * @see ifc.text._Defaults
 * @see ifc.style._CharacterProperties
 * @see ifc.beans._XPropertySet
 * @see ifc.style._ParagraphPropertiesComplex
 */
public class SwXTextDefaults extends TestCase {

    XTextDocument xTextDoc;

    /**
     * Creates the service <code>com.sun.star.text.Defaults</code>
     */
    protected TestEnvironment createTestEnvironment(
            TestParameters tParam, PrintWriter log) {

        XInterface oObj = null;

        XMultiServiceFactory docMSF = (XMultiServiceFactory)
            UnoRuntime.queryInterface(XMultiServiceFactory.class,xTextDoc);

        try {
            oObj = (XInterface)
                docMSF.createInstance("com.sun.star.text.Defaults");
        } catch (com.sun.star.uno.Exception e) {
            throw new StatusException("Couldn't create Object",e);
        }

        TestEnvironment tEnv = new TestEnvironment(oObj);

        return tEnv;
    }

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
}
