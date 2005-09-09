/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SwXTextTables.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 04:03:32 $
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
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextTable;
import com.sun.star.text.XTextTablesSupplier;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 *
 * initial description
 * @see com.sun.star.container.XElementAccess
 * @see com.sun.star.container.XIndexAccess
 * @see com.sun.star.container.XNameAccess
 *
 */
public class SwXTextTables extends TestCase {
    XTextDocument xTextDoc;

    protected void initialize( TestParameters tParam, PrintWriter log ) {
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );

        try {
            log.println( "creating a textdocument" );
            xTextDoc = SOF.createTextDoc( null );
        } catch ( com.sun.star.uno.Exception e ) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }
    }

    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xTextDoc " );
        util.DesktopTools.closeDoc(xTextDoc);
    }


    /**
     *    creating a Testenvironment for the interfaces to be tested
     *
     *  @param tParam    class which contains additional test parameters
     *  @param log        class to log the test state and result
     *
     *  @return    Status class
     *
     *  @see TestParameters
     *    @see PrintWriter
     */
    public synchronized TestEnvironment createTestEnvironment
            (TestParameters tParam, PrintWriter log) {

        XInterface oObj = null;
        int nRow = 4;
        int nCol = 5;


        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "creating a test environment" );

        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );

        // create testobject here
        TestEnvironment tEnv = null;
        XTextTable oTable = null;
        try {
            oTable = SOF.createTextTable( xTextDoc );
            SOF.insertTextContent(xTextDoc, oTable );
        }
        catch( Exception uE ) {
            uE.printStackTrace( log );
            throw new StatusException("Couldn't create TextTable : "
                    + uE.getMessage(), uE);
        }

        // Number two
        XTextTable oTable2 = null;
        try {
            oTable2 = SOF.createTextTable( xTextDoc );
            SOF.insertTextContent(xTextDoc, oTable2 );
        }
        catch( Exception uE ) {
            uE.printStackTrace( log );
            throw new StatusException("Couldn't create TextTable two: "
                    + uE.getMessage(), uE);
        }

        XMultiServiceFactory msf = (XMultiServiceFactory)
            UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDoc);
        XTextTablesSupplier oTTSupp = (XTextTablesSupplier)
            UnoRuntime.queryInterface(XTextTablesSupplier.class, msf);
        oObj = oTTSupp.getTextTables();

        if ( oTable != null ) {
            log.println("Creating instance...");
            tEnv = new TestEnvironment(oObj);
        }

        log.println( "adding TextDocument as mod relation to environment" );
        tEnv.addObjRelation( "TEXTDOC", xTextDoc );
        tEnv.addObjRelation( "ROW", new Integer( nRow ) );
        tEnv.addObjRelation( "COL", new Integer( nCol ) );
        try {
            tEnv.addObjRelation( "INST", SOF.createTextTable( xTextDoc ));
        }
        catch( Exception uE ) {
            uE.printStackTrace( log );
            throw new StatusException("Couldn't create TextTable : "
                    + uE.getMessage(), uE);
        }

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class SwXTextTables

