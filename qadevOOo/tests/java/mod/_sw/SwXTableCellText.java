/*************************************************************************
 *
 *  $RCSfile: SwXTableCellText.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 12:49:32 $
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
import util.TextSectionDsc;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.table.XCell;
import com.sun.star.text.XSimpleText;
import com.sun.star.text.XText;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextTable;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by cell text of the text table.<p>
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
public class SwXTableCellText extends TestCase {
    XTextDocument xTextDoc;
    SOfficeFactory SOF;

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
    * Creating a Testenvironment for the interfaces to be tested. After creation
    * of text table, it is inserted to a text document. Then cell named 'A1' is
    * obtained from table, string 'SwXTableCellText' is set to this cell, then
    * cell text is gotten from cell obtained before and returned as a test
    * component.
    *     Object relations created :
    * <ul>
    *  <li> <code>'TEXT'</code> for
    *      {@link ifc.text._XTextRangeCompare} : table cell text</li>
    *  <li> <code>'PARA'</code> for
    *      {@link ifc.text._XRelativeTextContentInsert} : paragraph creator</li>
    *  <li> <code>'XTEXTINFO'</code> for
    *      {@link ifc.text._XRelativeTextContentInsert},
    *      {@link ifc.text._XText} : creates text sections</li>
    * </ul>
    */
    public synchronized TestEnvironment createTestEnvironment(
            TestParameters Param, PrintWriter log ) throws StatusException {
        XInterface oObj = null;
        XTextTable oTable = null;

        log.println( "Creating a test environment" );
        try {
            oTable = SOF.createTextTable( xTextDoc );
        } catch ( com.sun.star.uno.Exception e ) {
            e.printStackTrace( log );
            throw new StatusException("Couldn't create TextTable : "
                +e.getMessage(),e);
        }

        try {
            SOF.insertTextContent(xTextDoc, oTable );
        } catch ( com.sun.star.lang.IllegalArgumentException e ) {
            e.printStackTrace( log );
            throw new StatusException("Couldn't insert text content : "
                +e.getMessage(),e);
        }

        XCell oCell = oTable.getCellByName("A1");
        XSimpleText oCellText = (XSimpleText)
            UnoRuntime.queryInterface(XSimpleText.class, oCell);
        oCellText.setString("SwXTableCellText");
        oObj = oCellText.getText();

        log.println( "creating a new environment for Cell object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        log.println( "adding TextDocument as mod relation to environment" );
        tEnv.addObjRelation("TEXT", (XText) oObj);

        log.println( "    adding InstCreator object" );
        TextSectionDsc tDsc = new TextSectionDsc();
        tEnv.addObjRelation( "XTEXTINFO", new InstCreator( xTextDoc, tDsc ) );

        log.println( "    adding Paragraph" );
        ParagraphDsc pDsc = new ParagraphDsc();
        tEnv.addObjRelation( "PARA", new InstCreator( xTextDoc, pDsc ) );

        return tEnv;
    } // finish method getTestEnvironment


}    // finish class SwXTableCellText

