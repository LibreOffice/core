/*************************************************************************
 *
 *  $RCSfile: ScModelObj.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change:$Date: 2003-09-08 12:14:48 $
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

package mod._sc;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.container.XIndexAccess;
import com.sun.star.frame.XController;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.XCell;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.view.XSelectionSupplier;

/**
* Test for object which is represented by service
* <code>com.sun.star.sheet.SpreadsheetDocument</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::sheet::XSpreadsheetDocument</code></li>
*  <li> <code>com::sun::star::sheet::SpreadsheetDocumentSettings</code></li>
*  <li> <code>com::sun::star::lang::XComponent</code></li>
*  <li> <code>com::sun::star::frame::XModel</code></li>
*  <li> <code>com::sun::star::sheet::SpreadsheetDocument</code></li>
*  <li> <code>com::sun::star::util::XNumberFormatsSupplier</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
* </ul>
* @see com.sun.star.sheet.SpreadsheetDocument
* @see com.sun.star.sheet.XSpreadsheetDocument
* @see com.sun.star.sheet.SpreadsheetDocumentSettings
* @see com.sun.star.lang.XComponent
* @see com.sun.star.frame.XModel
* @see com.sun.star.sheet.SpreadsheetDocument
* @see com.sun.star.util.XNumberFormatsSupplier
* @see com.sun.star.beans.XPropertySet
* @see ifc.sheet._XSpreadsheetDocument
* @see ifc.sheet._SpreadsheetDocumentSettings
* @see ifc.lang._XComponent
* @see ifc.frame._XModel
* @see ifc.sheet._SpreadsheetDocument
* @see ifc.util._XNumberFormatsSupplier
* @see ifc.beans._XPropertySet
*/
public class ScModelObj extends TestCase {
    public XSpreadsheetDocument xSpreadsheetDoc;
    public XSpreadsheetDocument xSecondsheetDoc;

    /**
    * Disposes Spreadsheet documents.
    */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xSheetDoc " );

        XComponent oComp = (XComponent)
            UnoRuntime.queryInterface (XComponent.class, xSpreadsheetDoc);
        oComp.dispose();

        oComp = (XComponent)
            UnoRuntime.queryInterface (XComponent.class, xSecondsheetDoc);
        oComp.dispose();
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Disposes the spreadsheet documents if they was created already.
    * Creates two spreadsheet documents. Rertieves the current controller for
    * each of them. Obtains the collection of spreadsheets, takes one of them
    * and takes some cell from the spreadsheet. The created documents are the instances
    * of the service <code>com.sun.star.sheet.SpreadsheetDocument</code>.
    * Object relations created :
    * <ul>
    *  <li> <code>'SELSUPP'</code> for
    *      {@link ifc.frame._XModel}( the interface
    *      <code>XSelectionSupplier</code> that was queried from the current
    *      controller of the spreadsheet)</li>
    *  <li> <code>'TOSELECT'</code> for
    *      {@link ifc.frame._XModel}( the cell that was retrieved from the
    *      spreadsheet)</li>
    *  <li> <code>'CONT2'</code> for
    *      {@link ifc.frame._XModel}( the current controller of the second
    *      spreadsheet)</li>
    * </ul>
    * @see
    */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        // creation of the testobject here
        // first we write what we are intend to do to log file

        log.println("craeting a test environment");

        // get a soffice factory object

        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)Param.getMSF());

        if (xSpreadsheetDoc != null) {
            XComponent oComp = (XComponent)
                UnoRuntime.queryInterface (XComponent.class, xSpreadsheetDoc);
            oComp.dispose();
        }
        if (xSecondsheetDoc != null) {
            XComponent oComp = (XComponent)
                UnoRuntime.queryInterface (XComponent.class, xSecondsheetDoc);
            oComp.dispose();
        }

        try {
            log.println("creating a spreadsheetdocument");
            xSpreadsheetDoc = SOF.createCalcDoc(null);
            xSecondsheetDoc = SOF.createCalcDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document ", e);
        }

        XModel model1 = (XModel)
            UnoRuntime.queryInterface(XModel.class, xSpreadsheetDoc);
        XModel model2 = (XModel)
            UnoRuntime.queryInterface(XModel.class, xSecondsheetDoc);
        XInterface oObj = model1;

        TestEnvironment tEnv = new TestEnvironment(oObj);

        XController cont1 = model1.getCurrentController();
        XController cont2 = model2.getCurrentController();

        XSelectionSupplier sel = (XSelectionSupplier)
            UnoRuntime.queryInterface(XSelectionSupplier.class, cont1);

        XCell toSel = null;

        try {
            log.println("Getting spreadsheet") ;
            XSpreadsheets oSheets = xSpreadsheetDoc.getSheets() ;
            XIndexAccess oIndexSheets = (XIndexAccess)
                UnoRuntime.queryInterface(XIndexAccess.class, oSheets);
            XSpreadsheet oSheet = (XSpreadsheet) AnyConverter.toObject(
                        new Type(XSpreadsheet.class),oIndexSheets.getByIndex(0));

            log.println("Getting a cell from sheet") ;
            toSel = oSheet.getCellByPosition(2, 3) ;
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log) ;
            throw new StatusException(
                "Error getting cell object from spreadsheet document", e);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log) ;
            throw new StatusException(
                "Error getting cell object from spreadsheet document", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log) ;
            throw new StatusException(
                "Error getting cell object from spreadsheet document", e);
        }

        log.println( "Adding SelectionSupplier and Shape to select for XModel");
        tEnv.addObjRelation("SELSUPP", sel);
        tEnv.addObjRelation("TOSELECT", toSel);

        log.println( "adding Controller as ObjRelation for XModel");
        tEnv.addObjRelation("CONT2", cont2);

        return tEnv;
    }


}    // finish class ScModelObj
