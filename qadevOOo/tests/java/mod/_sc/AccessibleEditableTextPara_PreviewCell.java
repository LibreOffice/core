/*************************************************************************
 *
 *  $RCSfile: AccessibleEditableTextPara_PreviewCell.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 12:01:04 $
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

import lib.Status;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.AccessibilityTools;
import util.SOfficeFactory;
import util.utils;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.awt.XWindow;
import com.sun.star.container.XIndexAccess;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
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
import com.sun.star.util.URL;
import com.sun.star.util.XURLTransformer;


public class AccessibleEditableTextPara_PreviewCell extends TestCase {
    XSpreadsheetDocument xSheetDoc = null;

    /**
     * Creates a spreadsheet document.
     */
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory) tParam.getMSF() );
        try {
            log.println( "creating a Spreadsheet document" );
            xSheetDoc = SOF.createCalcDoc(null);
        } catch ( com.sun.star.uno.Exception e ) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }
    }

    /**
     * Disposes a spreadsheet document.
     */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xSheetDoc " );
        XComponent oComp = (XComponent)UnoRuntime.queryInterface
            (XComponent.class, xSheetDoc);
        oComp.dispose();
    }


    /**
     * Creating a Testenvironment for the interfaces to be tested.
     * Obtains the accessible object for a one of cell in preview mode.
     */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XAccessibleContext oObj = null;
        XCell xCell = null;

        try {
            log.println("Getting spreadsheet") ;
            XSpreadsheets oSheets = xSheetDoc.getSheets() ;
            XIndexAccess oIndexSheets = (XIndexAccess)
            UnoRuntime.queryInterface(XIndexAccess.class, oSheets);
            XSpreadsheet oSheet = (XSpreadsheet) AnyConverter.toObject(
                    new Type(XSpreadsheet.class),oIndexSheets.getByIndex(0));

            log.println("Getting a cell from sheet") ;
            xCell = oSheet.getCellByPosition(0, 0);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException(
                "Error getting cell object from spreadsheet document", e);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            throw new StatusException(
                "Error getting cell object from spreadsheet document", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException(
                "Error getting cell object from spreadsheet document", e);
        }

        xCell.setFormula("Value");

        XModel xModel = (XModel)
            UnoRuntime.queryInterface(XModel.class, xSheetDoc);

        XController xController = xModel.getCurrentController();

        //switch to 'Print Preview' mode
        try {
            XDispatchProvider xDispProv = (XDispatchProvider)
                UnoRuntime.queryInterface(XDispatchProvider.class, xController);
            XURLTransformer xParser = (com.sun.star.util.XURLTransformer)
                UnoRuntime.queryInterface(XURLTransformer.class,
            ( (XMultiServiceFactory) Param.getMSF()).createInstance("com.sun.star.util.URLTransformer"));
            URL[] aParseURL = new URL[1];
            aParseURL[0] = new URL();
            aParseURL[0].Complete = ".uno:PrintPreview";
            xParser.parseStrict(aParseURL);
            URL aURL = aParseURL[0];
            XDispatch xDispatcher = xDispProv.queryDispatch(aURL, "", 0);
            if(xDispatcher != null)
                xDispatcher.dispatch( aURL, null );
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't change mode");
            throw new StatusException(Status.failed("Couldn't change mode"));
        }

        shortWait();


        XWindow xWindow = AccessibilityTools.getCurrentWindow( (XMultiServiceFactory) Param.getMSF(), xModel);
        XAccessible xRoot = AccessibilityTools.getAccessibleObject(xWindow);

        AccessibilityTools.getAccessibleObjectForRole(xRoot, AccessibleRole.TABLE_CELL);

        xRoot = AccessibilityTools.SearchedAccessible;

        AccessibilityTools.printAccessibleTree(log,xRoot);

        oObj = AccessibilityTools.getAccessibleObjectForRole(xRoot,AccessibleRole.PARAGRAPH);

        log.println("ImplementationName " + utils.getImplName(oObj));
        log.println("AccessibleName " + oObj.getAccessibleName());
        log.println("Parent " + utils.getImplName(oObj.getAccessibleParent()));

        TestEnvironment tEnv = new TestEnvironment( oObj );

        tEnv.addObjRelation("EditOnly", "AccessibleEditableTextPara_PreviewCell");
        tEnv.addObjRelation("Destroy", "AccessibleEditableTextPara_PreviewCell");

        final XCell cell_to_change = xCell;
        tEnv.addObjRelation("EventProducer",
            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer(){
                public void fireEvent() {
                    cell_to_change.setFormula("NewString");
                }
            });

        return tEnv;
    }

    protected void shortWait() {
        try {
            Thread.sleep(1000) ;
        } catch (InterruptedException e) {
            System.out.println("While waiting :" + e);
        }
    }
}