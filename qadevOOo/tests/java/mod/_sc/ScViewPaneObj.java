/*************************************************************************
 *
 *  $RCSfile: ScViewPaneObj.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change:$Date: 2003-09-08 12:18:43 $
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
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XViewPane;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.sheet.SpreadsheetViewPane</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::sheet::XViewPane</code></li>
*  <li> <code>com::sun::star::sheet::XCellRangeReferrer</code></li>
* </ul>
* @see com.sun.star.sheet.SpreadsheetViewPane
* @see com.sun.star.sheet.XViewPane
* @see com.sun.star.sheet.XCellRangeReferrer
* @see ifc.sheet._XViewPane
* @see ifc.sheet._XCellRangeReferrer
*/
public class ScViewPaneObj extends TestCase {
    private XSpreadsheetDocument xSpreadsheetDoc;
    private SOfficeFactory SOF;
    private XInterface oObj;

    /**
    * Creates Spreadsheet document.
    */
    public void initialize( TestParameters Param, PrintWriter log ) {
        // get a soffice factory object
        SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)Param.getMSF());

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
        oComp.dispose();
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Retieves the current controller of the spreadsheet document using the
    * interface <code>XModel</code>. The controller contains the collection
    * of the view panes so retrieves the view pane with index 0 from
    * the collection. The retrived view pane is the instance of the service
    * <code>com.sun.star.sheet.SpreadsheetViewPane</code>. Retrieves the address
    * of the cell range that consists of the cells which are visible in the pane.
    * Object relations created :
    * <ul>
    *  <li> <code>'DATAAREA'</code> for
    *      {@link ifc.sheet._XViewPane}(the retrieved address)</li>
    * </ul>
    * @see com.sun.star.frame.XModel
    */
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XModel xm = (XModel)
            UnoRuntime.queryInterface(XModel.class, xSpreadsheetDoc);
        XController xc = xm.getCurrentController();
        XIndexAccess xIA = (XIndexAccess)
            UnoRuntime.queryInterface(XIndexAccess.class, xc);
        try {
            oObj = (XInterface) AnyConverter.toObject(
                    new Type(XInterface.class),xIA.getByIndex(0));
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get by index", e);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get by index", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get by index", e);
        }

        TestEnvironment tEnv = new TestEnvironment(oObj);

        XViewPane VP = (XViewPane)
            UnoRuntime.queryInterface(XViewPane.class, oObj);
        CellRangeAddress dataArea = VP.getVisibleRange();
        tEnv.addObjRelation("DATAAREA", dataArea);

        return tEnv;
    }
}

