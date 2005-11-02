/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ScSubTotalFieldObj.java,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 18:09:19 $
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
import com.sun.star.sheet.GeneralFunction;
import com.sun.star.sheet.SubTotalColumn;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.sheet.XSubTotalCalculatable;
import com.sun.star.sheet.XSubTotalDescriptor;
import com.sun.star.sheet.XSubTotalField;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.sheet.SubTotalField</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::sheet::XSubTotalField</code></li>
* </ul>
* @see com.sun.star.sheet.SubTotalField
* @see com.sun.star.sheet.XSubTotalField
* @see ifc.sheet._XSubTotalField
*/
public class ScSubTotalFieldObj extends TestCase {
    public static XSpreadsheetDocument xSpreadsheetDoc;

    /**
    * Creates Spreadsheet document.
    */
    public void initialize( TestParameters Param, PrintWriter log ) {
        // creation of the testobject here
        // first we write what we are intend to do to log file
        log.println("creating a test environment");

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
    * Retrieves a collection of spreadsheets from the document and takes one of
    * them. Creates a subtotal descriptor using the interface
    * <code>XSubTotalCalculatable</code>. Adds a subtotal field definition to
    * the descriptor. Obtains the subtotal field with index 0 from the
    * collection. The obtained subtotal field is the instance of the service
    * <code>com.sun.star.sheet.SubTotalField</code>.
    * @see com.sun.star.sheet.XSubTotalCalculatable
    * @see com.sun.star.sheet.SubTotalField
    */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        log.println("getting sheets");
        XSpreadsheets xSpreadsheets = (XSpreadsheets)xSpreadsheetDoc.getSheets();

        log.println("getting a sheet");
        XSpreadsheet oSheet = null;
        XIndexAccess oIndexAccess = (XIndexAccess)
            UnoRuntime.queryInterface(XIndexAccess.class, xSpreadsheets);
        try {
            oSheet = (XSpreadsheet) AnyConverter.toObject(
                    new Type (XSpreadsheet.class),oIndexAccess.getByIndex(0));
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

        XSubTotalCalculatable xSTC = (XSubTotalCalculatable)
            UnoRuntime.queryInterface(XSubTotalCalculatable.class, oSheet);

        XSubTotalDescriptor xSTD = xSTC.createSubTotalDescriptor(true);

        SubTotalColumn[] columns = new SubTotalColumn[1];
        SubTotalColumn column = new SubTotalColumn();
        column.Column = 5;
        column.Function = GeneralFunction.SUM;
        columns[0] = column;
        xSTD.addNew(columns, 1);

        XIndexAccess oDescIndex = (XIndexAccess)
            UnoRuntime.queryInterface(XIndexAccess.class, xSTD);

        XInterface oObj = null;

        try {
            oObj = ( XSubTotalField ) AnyConverter.toObject(
                    new Type(XSubTotalField.class),oDescIndex.getByIndex(0));
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get XSubTotalField", e);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get XSubTotalField", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get XSubTotalField", e);
        }

        TestEnvironment tEnv = new TestEnvironment(oObj);
        return tEnv;

    } // finish method getTestEnvironment

}    // finish class ScSubTotalFieldObj

