/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ScTableSheetsObj.java,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 18:10:49 $
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

package mod._sc;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.sheet.Spreadsheets</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::sheet::XSpreadsheets</code></li>
*  <li> <code>com::sun::star::container::XNameAccess</code></li>
*  <li> <code>com::sun::star::container::XIndexAccess</code></li>
*  <li> <code>com::sun::star::container::XElementAccess</code></li>
*  <li> <code>com::sun::star::container::XNameReplace</code></li>
*  <li> <code>com::sun::star::container::XNameContainer</code></li>
* </ul>
* @see com.sun.star.sheet.Spreadsheets
* @see com.sun.star.sheet.XSpreadsheets
* @see com.sun.star.container.XNameAccess
* @see com.sun.star.container.XIndexAccess
* @see com.sun.star.container.XElementAccess
* @see com.sun.star.container.XNameReplace
* @see com.sun.star.container.XNameContainer
* @see ifc.sheet._XSpreadsheets
* @see ifc.container._XNameAccess
* @see ifc.container._XIndexAccess
* @see ifc.container._XElementAccess
* @see ifc.container._XNameReplace
* @see ifc.container._XNameContainer
*/
public class ScTableSheetsObj extends TestCase {
    private static XSpreadsheetDocument xSpreadsheetDoc = null;

    /**
    * Creates Spreadsheet document.
    */
    public void initialize( TestParameters Param, PrintWriter log ) {
        // get a soffice factory object

        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)Param.getMSF());
        try {
            log.println("creating a spreadsheetdocument");
            xSpreadsheetDoc = SOF.createCalcDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document ", e );
        }
    }

    /**
    * Disposes Spreadsheet document.
    */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println("disposing xSpreadsheetDocument");
        XComponent oComp = (XComponent)
            UnoRuntime.queryInterface(XComponent.class, xSpreadsheetDoc);
        util.DesktopTools.closeDoc(oComp);
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Retrieves the collection of the spreadsheets using the interface
    * <code>XSpreadsheetDocument</code>. The retrieved collection is the
    * instance of the service <code>com.sun.star.sheet.Spreadsheets</code>.
    * Creates instances of the service <code>com.sun.star.sheet.Spreadsheet</code>.
    * Object relations created :
    * <ul>
    *  <li> <code>'SecondInstance'</code> for
    *      {@link ifc.container._XNameContainer} (the created instance of the
    *       service <code>com.sun.star.sheet.Spreadsheet</code>) </li>
    *  <li> <code>'INSTANCE1', ..., 'INSTANCEN'</code> for
    *      {@link ifc.container._XNameContainer} (the created instances of the
    *       service <code>com.sun.star.sheet.Spreadsheet</code>) </li>
    * </ul>
    * @see com.sun.star.sheet.Spreadsheets
    * @see com.sun.star.sheet.XSpreadsheetDocument
    * @see com.sun.star.sheet.Spreadsheet
    */
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)Param.getMSF());

        log.println("getting sheets");
        XSpreadsheets xSpreadsheets = (XSpreadsheets)xSpreadsheetDoc.getSheets();

        XInterface oObj = (XInterface)
            UnoRuntime.queryInterface(XInterface.class, xSpreadsheets);

        log.println("creating a new environment for object");
        TestEnvironment tEnv = new TestEnvironment(oObj);

        log.println("creating instance of the sheet object to use it in tests");
        XSpreadsheet oSecondSheet = null;

        try {
            oSecondSheet = SOF.createSpreadsheet(xSpreadsheetDoc);
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create instances", e);
        }

        //adding Instance for XNameContainer
        tEnv.addObjRelation("SecondInstance",oSecondSheet);

        // INSTANCEn : _XNameContainer; _XNameReplace
        log.println( "adding INSTANCEn as mod relation to environment" );

        int THRCNT = 1;
        if ((String)Param.get("THRCNT") != null) {
            THRCNT = Integer.parseInt((String)Param.get("THRCNT"));
        }
        try {
            for (int n = 1; n < (THRCNT+1) ;n++ ) {
                log.println(
                    "adding INSTANCE" + n +" as mod relation to environment" );
                tEnv.addObjRelation(
                    "INSTANCE" + n,
                    SOF.createSpreadsheet(xSpreadsheetDoc) );
                }
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException(
                "Couldn't create instances for object relations", e);
        }

        return tEnv;
    }
}


