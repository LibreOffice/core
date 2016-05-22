/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
import com.sun.star.sheet.XSheetAnnotation;
import com.sun.star.sheet.XSheetAnnotationAnchor;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.CellAddress;
import com.sun.star.table.XCell;
import com.sun.star.table.XCellRange;
import com.sun.star.text.XSimpleText;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;


/**
 *
 * initial description
 * @see com.sun.star.beans.XPropertySet
 * @see com.sun.star.beans.XPropertyState
 * @see com.sun.star.document.XDocumentInsertable
 * @see com.sun.star.style.CharacterProperties
 * @see com.sun.star.style.ParagraphProperties
 * @see com.sun.star.text.XParagraphCursor
 * @see com.sun.star.text.XSentenceCursor
 * @see com.sun.star.text.XTextCursor
 * @see com.sun.star.text.XTextRange
 * @see com.sun.star.text.XWordCursor
 * @see com.sun.star.util.XSortable
 *
 */
public class ScAnnotationTextCursor extends TestCase {

    static XSpreadsheetDocument xSheetDoc = null;

    protected void initialize( TestParameters tParam, PrintWriter log ) {
        SOfficeFactory SOF = SOfficeFactory.getFactory(  (XMultiServiceFactory) tParam.getMSF() );

        try {
            log.println( "creating a Spreadsheet document" );
            xSheetDoc = SOF.createCalcDoc(null);
        } catch ( com.sun.star.uno.Exception e ) {
            // Some exception occured.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }

    }

    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xSheetDoc " );
        XComponent oComp = (XComponent) UnoRuntime.queryInterface
            (XComponent.class, xSheetDoc) ;
        util.DesktopTools.closeDoc(oComp);
    }


    /**
     *    creating a Testenvironment for the interfaces to be tested
     */
    public synchronized TestEnvironment createTestEnvironment
            ( TestParameters Param, PrintWriter log )
            throws StatusException {

        XInterface oObj = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "Creating a test environment" );

        CellAddress cellPos = new CellAddress((short)0, 1, 2);

        try {
            log.println("Getting test object ") ;

            XSpreadsheetDocument xArea = (XSpreadsheetDocument)
                UnoRuntime.queryInterface(XSpreadsheetDocument.class, xSheetDoc);

            XSpreadsheets oSheets = (XSpreadsheets) xArea.getSheets();

            XIndexAccess XAccess = (XIndexAccess)
                        UnoRuntime.queryInterface(XIndexAccess.class, oSheets);

            XSpreadsheet oSheet = (XSpreadsheet)XAccess.getByIndex(cellPos.Sheet);

            XCellRange oCRange = (XCellRange)
                            UnoRuntime.queryInterface(XCellRange.class, oSheet);

            XCell oCell = oCRange.getCellByPosition(cellPos.Column, cellPos.Row);

            XSheetAnnotationAnchor oAnnoA = (XSheetAnnotationAnchor)
                UnoRuntime.queryInterface(XSheetAnnotationAnchor.class, oCell);

            XSheetAnnotation oAnno = oAnnoA.getAnnotation();
            XSimpleText aText = (XSimpleText)
                            UnoRuntime.queryInterface(XSimpleText.class,oAnno);
            aText.setString("A nice little Test");
            oObj = aText.createTextCursor();

        } catch (Exception e) {
            e.printStackTrace(log) ;
            throw new StatusException(
                    "Error getting test object from spreadsheet document",e) ;
        }

        TestEnvironment tEnv = new TestEnvironment( oObj );

        // Other parameters required for interface tests

        return tEnv;
    }

}    // finish class ScAnnotationTextCursor

