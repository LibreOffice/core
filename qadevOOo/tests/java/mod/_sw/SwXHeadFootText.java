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
import util.ParagraphDsc;
import util.SOfficeFactory;
import util.TableDsc;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.style.XStyle;
import com.sun.star.style.XStyleFamiliesSupplier;
import com.sun.star.text.XText;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;


/**
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::text::XTextRangeMover</code></li>
 *  <li> <code>com::sun::star::text::XSimpleText</code></li>
 *  <li> <code>com::sun::star::text::XTextRange</code></li>
 *  <li> <code>com::sun::star::text::XRelativeTextContentInsert</code></li>
 *  <li> <code>com::sun::star::text::XTextRangeCompare</code></li>
 *  <li> <code>com::sun::star::container::XElementAccess</code></li>
 *  <li> <code>com::sun::star::container::XEnumerationAccess</code></li>
 *  <li> <code>com::sun::star::text::XText</code></li>
 * </ul> <p>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurently.
 * @see com.sun.star.text.XTextRangeMover
 * @see com.sun.star.text.XSimpleText
 * @see com.sun.star.text.XTextRange
 * @see com.sun.star.text.XRelativeTextContentInsert
 * @see com.sun.star.text.XTextRangeCompare
 * @see com.sun.star.container.XElementAccess
 * @see com.sun.star.container.XEnumerationAccess
 * @see com.sun.star.text.XText
 * @see ifc.text._XTextRangeMover
 * @see ifc.text._XSimpleText
 * @see ifc.text._XTextRange
 * @see ifc.text._XRelativeTextContentInsert
 * @see ifc.text._XTextRangeCompare
 * @see ifc.container._XElementAccess
 * @see ifc.container._XEnumerationAccess
 * @see ifc.text._XText
 */
public class SwXHeadFootText extends TestCase {
    XTextDocument xTextDoc;

    /**
    * Creates text document.
    */
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        SOfficeFactory SOF = SOfficeFactory.getFactory(  (XMultiServiceFactory) tParam.getMSF() );
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
    * Creating a Testenvironment for the interfaces to be tested. At first
    * style families are obtained from text document, then style 'Standard' has
    * gotten from style family 'PageStyles'. At the end, document header and
    * footer are switched on and document text is obtained.
    *     Object relations created :
    * <ul>
    *  <li> <code>'XTEXTINFO'</code> for
    *    {@link ifc.text._XRelativeTextContentInsert},
    *    {@link ifc.text._XText} : creates tables 6x4</li>
    *  <li> <code>'TEXTDOC'</code> for
    *    {@link ifc.text._XTextRangeCompare} : text document</li>
    *  <li> <code>'PARA'</code> for
    *    {@link ifc.text._XRelativeTextContentInsert} : paragraph creator</li>
    * </ul>
    */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {
        XInterface oObj = null;
        XPropertySet PropSet;
        XNameAccess PageStyles = null;
        XStyle StdStyle = null;

        log.println( "creating a test environment" );
        XStyleFamiliesSupplier StyleFam = (XStyleFamiliesSupplier)
            UnoRuntime.queryInterface(XStyleFamiliesSupplier.class, xTextDoc);
        XNameAccess StyleFamNames = StyleFam.getStyleFamilies();

        // obtains style 'Standatd' from style family 'PageStyles'
        try {
            PageStyles = (XNameAccess) AnyConverter.toObject(
                new Type(XNameAccess.class),StyleFamNames.getByName("PageStyles"));
            StdStyle = (XStyle) AnyConverter.toObject(
                    new Type(XStyle.class),PageStyles.getByName("Standard"));
        } catch ( com.sun.star.lang.WrappedTargetException e ) {
            e.printStackTrace(log);
            throw new StatusException("Error getting style by name!", e);
        } catch ( com.sun.star.container.NoSuchElementException e ) {
            e.printStackTrace(log);
            throw new StatusException("Error, no such style name! ", e);
        } catch ( com.sun.star.lang.IllegalArgumentException e ) {
            e.printStackTrace(log);
            throw new StatusException("Error getting style by name!", e);
        }

        PropSet = (XPropertySet)
            UnoRuntime.queryInterface( XPropertySet.class, StdStyle);

        // changing/getting some properties
        try {
            log.println( "Switching on header" );
            PropSet.setPropertyValue("HeaderIsOn", new Boolean(true));
            log.println( "Switching on footer" );
            PropSet.setPropertyValue("FooterIsOn", new Boolean(true));
            log.println( "Get header text" );
            oObj = (XText) UnoRuntime.queryInterface(
                        XText.class, PropSet.getPropertyValue("HeaderText"));
        } catch ( com.sun.star.lang.WrappedTargetException e ) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't set/get propertyValue...", e);
        } catch ( com.sun.star.lang.IllegalArgumentException e ) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't set/get propertyValue...", e);
        } catch ( com.sun.star.beans.PropertyVetoException e ) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't set/get propertyValue...", e);
        } catch ( com.sun.star.beans.UnknownPropertyException e ) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't set/get propertyValue...", e);
        }

        log.println( "creating a new environment for bodytext object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        log.println( "    adding Paragraph" );
        ParagraphDsc pDsc = new ParagraphDsc();
        tEnv.addObjRelation( "PARA", new InstCreator( xTextDoc, pDsc ) );

        log.println( "adding InstDescriptor object" );
        TableDsc tDsc = new TableDsc( 6, 4 );

        log.println( "adding InstCreator object" );
        tEnv.addObjRelation( "XTEXTINFO", new InstCreator( xTextDoc, tDsc ) );

        tEnv.addObjRelation( "TEXT", oObj);

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class SwXHeadFootText
