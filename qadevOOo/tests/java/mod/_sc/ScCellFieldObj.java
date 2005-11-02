/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ScCellFieldObj.java,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 17:54:45 $
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
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.XCell;
import com.sun.star.text.XText;
import com.sun.star.text.XTextContent;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object that represents a text field (implements
* <code>com.sun.star.text.TextField</code>) which inserted in a cell of
* the spreadsheet. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::lang::XComponent</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
*  <li> <code>com::sun::star::text::XTextField</code></li>
*  <li> <code>com::sun::star::text::XTextContent</code></li>
*  <li> <code>com::sun::star::text::TextContent</code></li>
* </ul>
* @see com.sun.star.text.TextField
* @see com.sun.star.lang.XComponent
* @see com.sun.star.beans.XPropertySet
* @see com.sun.star.text.XTextField
* @see com.sun.star.text.XTextContent
* @see com.sun.star.text.TextContent
* @see ifc.lang._XComponent
* @see ifc.beans._XPropertySet
* @see ifc.text._XTextField
* @see ifc.text._XTextContent
* @see ifc.text._TextContent
*/
public class ScCellFieldObj extends TestCase {
    static XSpreadsheetDocument xSheetDoc = null;

    /**
    * Creates Spreadsheet document.
    */
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );

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
    * Disposes Spreadsheet document.
    */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xSheetDoc " );
        XComponent oComp = (XComponent)
            UnoRuntime.queryInterface (XComponent.class, xSheetDoc);
        util.DesktopTools.closeDoc(oComp);
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.text.TextField.URL</code>, inserts it to the content
    * of the cell in the spreadsheet, retrieves a text content
    * <code>com.sun.star.text.XTextContent</code> from the cell.<p>
    * Object relations created :
    * <ul>
    *  <li> <code>'TRO'</code> for
    *      {@link ifc.text._TextContent} </li>
    *  <li> <code>'CONTENT'</code> for
    *      {@link ifc.text._XTextContent} (type of
    *      <code>com.sun.star.text.XTextContent</code> that was queried from
    *      the newly created service <code>com.sun.star.text.TextField.URL</code>)</li>
    *  <li> <code>'TEXT'</code> for
    *      {@link ifc.text._XTextContent} (the text of the cell)</li>
    * </ul>
    */
    protected synchronized TestEnvironment createTestEnvironment(
        TestParameters Param, PrintWriter log) {

        XInterface oObj = null;
        XText oText = null;
        XTextContent oContent = null;
        XInterface aField = null;

        try {
            // we want to create an instance of ScCellFieldObj.
            // to do this we must get an MultiServiceFactory.

            XMultiServiceFactory _oMSF = (XMultiServiceFactory)
                UnoRuntime.queryInterface(XMultiServiceFactory.class, xSheetDoc);

            // Now create the instance of com.sun.star.text.TextField.
            // This object has type ScCellFieldObj.

            oObj = (XInterface)
                _oMSF.createInstance("com.sun.star.text.TextField.URL");

            aField = (XInterface)
                _oMSF.createInstance("com.sun.star.text.TextField.URL");
            oContent = (XTextContent)
                UnoRuntime.queryInterface(XTextContent.class, aField);

            XSpreadsheets oSheets = xSheetDoc.getSheets() ;
            XIndexAccess oIndexSheets = (XIndexAccess)
                UnoRuntime.queryInterface(XIndexAccess.class, oSheets);
            XSpreadsheet oSheet = (XSpreadsheet) AnyConverter.toObject(
                    new Type(XSpreadsheet.class),oIndexSheets.getByIndex(0));

            XCell oCell = oSheet.getCellByPosition(2,3);
            oText = (XText)UnoRuntime.queryInterface(XText.class, oCell);

            XTextContent oTextContent = (XTextContent)
                UnoRuntime.queryInterface(XTextContent.class, oObj);

            oText.insertTextContent(
                oText.createTextCursor(), oTextContent, true);

            oCell = oSheet.getCellByPosition(1,4);
            oText = (XText)UnoRuntime.queryInterface(XText.class, oCell);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Exception occured while creating test Object.");
            e.printStackTrace(log);
            throw new StatusException("Couldn't create test object", e);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Exception occured while creating test Object.");
            e.printStackTrace(log);
            throw new StatusException("Couldn't create test object", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Exception occured while creating test Object.");
            e.printStackTrace(log);
            throw new StatusException("Couldn't create test object", e);
        } catch (com.sun.star.uno.Exception e) {
            log.println("Exception occured while creating test Object.");
            e.printStackTrace(log);
            throw new StatusException("Couldn't create test object", e);
        }

        TestEnvironment tEnv = new TestEnvironment(oObj) ;

        log.println ("Object created.") ;
        tEnv.addObjRelation("TRO", new Boolean(true));

        tEnv.addObjRelation("CONTENT",oContent);
        tEnv.addObjRelation("TEXT",oText);

        return tEnv;
    }

}    // finish class ScCellFieldObj

