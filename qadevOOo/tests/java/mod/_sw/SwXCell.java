/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SwXCell.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 03:44:38 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

package mod._sw;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is a cell of the table in a text document, and
 * represented by service <code>com.sun.star.table.Cell</code><p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::text::CellProperties</code></li>
 *  <li> <code>com::sun::star::beans::XPropertySet</code></li>
 * </ul> <p>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurently.
 * @see com.sun.star.table.Cell
 * @see com.sun.star.text.CellProperties
 * @see com.sun.star.beans.XPropertySet
 * @see ifc.text._CellProperties
 * @see ifc.beans._XPropertySet
 */
public class SwXCell extends TestCase {
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
    * Creating a Testenvironment for the interfaces to be tested. After creating
    * a text table, it is inserted to the text document. Finally, first cell of
    * this table is gotten.
    *     Object relations created :
    * <ul>
    *  <li> <code>'CellProperties.TextSection'</code> for
    *    {@link ifc.text._CellProperties} : range of complete paragraphs
    *  within a text</li>
    * </ul>
    */
    public TestEnvironment createTestEnvironment(
            TestParameters tParam, PrintWriter log ) throws StatusException {
        XInterface oObj = null;
        XTextContent oTable = null;

        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );
        log.println( "creating a test environment" );
        try {
            oTable = SOF.createTextTable(xTextDoc, 3, 4);
        } catch ( com.sun.star.uno.Exception e ) {
            log.println("Unable to create TextTable...");
            e.printStackTrace(log);
        }
        try {
            SOF.insertTextContent( xTextDoc, oTable );
        } catch ( com.sun.star.lang.IllegalArgumentException e ) {
            log.println("Unable to insert TextContent...");
            e.printStackTrace(log);
        }
        oObj = SOF.getFirstTableCell( oTable );

        log.println( "    creating a new environment for bodytext object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        XMultiServiceFactory oDocMSF = (XMultiServiceFactory)
                UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDoc);
        try {
            XInterface oTS = (XInterface) oDocMSF.createInstance
                ("com.sun.star.text.TextSection");
            log.println("  adding TextSection object");
            tEnv.addObjRelation("CellProperties.TextSection", oTS);
        } catch (com.sun.star.uno.Exception e) {
            log.println("Could not get instance of TextSection");
            e.printStackTrace(log);
        }

        return tEnv;
    } // finish method getTestEnvironment
}    // finish class SwXCell

