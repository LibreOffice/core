/*************************************************************************
 *
 *  $RCSfile: SwXHeadFootText.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change:$Date: 2003-09-08 12:46:29 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
            throw new StatusException( "Couldn³t create document", e );
        }
    }

    /**
    * Disposes text document.
    */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xTextDoc " );
        xTextDoc.dispose();
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