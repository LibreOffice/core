/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ScScenariosObj.java,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 18:07:23 $
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

import com.sun.star.container.XIndexAccess;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XCellRangeAddressable;
import com.sun.star.sheet.XScenariosSupplier;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.table.XCellRange;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.sheet.Scenarios</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::container::XNameAccess</code></li>
*  <li> <code>com::sun::star::container::XElementAccess</code></li>
*  <li> <code>com::sun::star::sheet::XScenarios</code></li>
* </ul>
* @see com.sun.star.sheet.Scenarios
* @see com.sun.star.container.XNameAccess
* @see com.sun.star.container.XElementAccess
* @see com.sun.star.sheet.XScenarios
* @see ifc.container._XNameAccess
* @see ifc.container._XElementAccess
* @see ifc.sheet._XScenarios
*/
public class ScScenariosObj extends TestCase {
    public static XSpreadsheetDocument xSpreadsheetDoc;

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
        log.println( "    disposing xSheetDoc " );
        XComponent oComp = (XComponent)
            UnoRuntime.queryInterface (XComponent.class, xSpreadsheetDoc) ;
        util.DesktopTools.closeDoc(oComp);
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Retrieves a collection of spreadsheets from a document
    * and takes one of them. Fills some cells of the spreadsheet.
    * Retrieves the collection of all scenarios using the interface
    * <code>XScenariosSupplier</code>. Creates a new scenario and adds it to the
    * collection. This collection is the instance of the service
    * <code>com.sun.star.sheet.Scenarios</code>.
    * Object relations created :
    * <ul>
    *  <li> <code>'ADDR'</code> for
    *      {@link ifc.sheet._XScenarios}(the array of cell range addresses
    *      of the created scenario)</li>
    * </ul>
    * @see com.sun.star.sheet.Scenarios
    * @see com.sun.star.sheet.XScenariosSupplier
    */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        log.println("getting sheets");
        XSpreadsheets xSpreadsheets = (XSpreadsheets)xSpreadsheetDoc.getSheets();
        log.println("getting a sheet");
        XSpreadsheet oSheet = null;
        XIndexAccess oIndexAccess = (XIndexAccess)
            UnoRuntime.queryInterface(XIndexAccess.class, xSpreadsheets);
        try {
            oSheet = (XSpreadsheet)AnyConverter.toObject(
                    new Type(XSpreadsheet.class),oIndexAccess.getByIndex(0));
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException( "Couldn't get a spreadsheet", e);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            throw new StatusException( "Couldn't get a spreadsheet", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException( "Couldn't get a spreadsheet", e);
        }

        log.println("filling some cells");
        try {
            oSheet.getCellByPosition(5, 5).setValue(15);
            oSheet.getCellByPosition(1, 4).setValue(10);
            oSheet.getCellByPosition(2, 0).setValue(-5.15);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't fill some cell", e);
        }

        XScenariosSupplier xSupp = (XScenariosSupplier)
            UnoRuntime.queryInterface(XScenariosSupplier.class, oSheet);
        XCellRange oRange = (XCellRange)
            UnoRuntime.queryInterface(XCellRange.class, oSheet);
        XCellRange myRange = oRange.getCellRangeByName("A1:N4");
        XCellRangeAddressable oRangeAddr = (XCellRangeAddressable)
            UnoRuntime.queryInterface(XCellRangeAddressable.class, myRange);
        CellRangeAddress myAddr = oRangeAddr.getRangeAddress();

        CellRangeAddress[] oAddr = new CellRangeAddress[1];
        oAddr[0] = myAddr;

        xSupp.getScenarios().addNewByName("ScScenarios", oAddr, "Range");

        XInterface oObj = xSupp.getScenarios();

        TestEnvironment tEnv = new TestEnvironment(oObj);

        log.println("adding ObjectRelation for XScenarios");
        tEnv.addObjRelation("ADDR", oAddr);

        return tEnv;
    }


}    // finish class ScScenariosObj

