



package testcase.uno.sc.rowcolumn;

import static org.junit.Assert.*;


import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import org.openoffice.test.uno.UnoApp;


import com.sun.star.lang.XComponent;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.XCell;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.table.XTableColumns;
import com.sun.star.table.XTableRows;
import com.sun.star.table.XColumnRowRange;

/**
 * Test insert or delete rows and columns
 * @author test
 *
 */

public class InsertDeleteRowAndColumn {

    UnoApp unoApp = new UnoApp();
    XSpreadsheetDocument scDocument = null;
    XComponent scComponent = null;

    @Before
    public void setUp() throws Exception {
        unoApp.start();
    }

    @After
    public void tearDown() throws Exception {
        unoApp.closeDocument(scComponent);
        unoApp.close();
        }

    @Test
    public void testInsertDeleteRows() throws Exception {

        String sheetname = "sheet1";
        scComponent = unoApp.newDocument("scalc");
        scDocument = (XSpreadsheetDocument) UnoRuntime.queryInterface(XSpreadsheetDocument.class, scComponent);
        XSpreadsheets spreadsheets = scDocument.getSheets();
        Object sheetObj = spreadsheets.getByName(sheetname);


        XSpreadsheet sheet = (XSpreadsheet) UnoRuntime.queryInterface(XSpreadsheet.class, sheetObj);
        XColumnRowRange xCRRange = (XColumnRowRange) UnoRuntime.queryInterface( XColumnRowRange.class, sheet );
        XTableRows xRows = xCRRange.getRows();

        // Create a cell series "A2:A8" with the values 1 ... 7.
        int nRow = 1;
        for (int i = 1; i < 8; ++i) {
            sheet.getCellByPosition( 0, nRow ).setValue( nRow );
            nRow += 1;
        }

        //Insert a row between row 2 and row 3
        xRows.insertByIndex( 2, 1 );

        //Get value of cell A3
        XCell cell = sheet.getCellByPosition(0, 2);
        double checkvalue = 0.0;

        //Verify after insert row
        assertEquals("Verify one new row inserted after Row 2",checkvalue, cell.getValue(),0);

        //Delete the row 3 and row 4
        xRows.removeByIndex( 2, 2 );

        //Get value of cell A3 and A4
        XCell cellA3 = sheet.getCellByPosition(0, 2);
        XCell cellA4 = sheet.getCellByPosition(0, 3);
        double checkvalueA3 = 3.0;
        double checkvalueA4 = 4.0;

        //Verify after delete row3 and row4
        assertEquals("Verify tow rows deleted the value of row 3",checkvalueA3, cellA3.getValue(),0);
        assertEquals("Verify tow rows deleted the value of row 4",checkvalueA4, cellA4.getValue(),0);

}

@Test
public void testInsertDeleteColumns() throws Exception {

    String sheetname = "sheet1";
    scComponent = unoApp.newDocument("scalc");
    scDocument = (XSpreadsheetDocument) UnoRuntime.queryInterface(XSpreadsheetDocument.class, scComponent);
    XSpreadsheets spreadsheets = scDocument.getSheets();
    Object sheetObj = spreadsheets.getByName(sheetname);


    XSpreadsheet sheet = (XSpreadsheet) UnoRuntime.queryInterface(XSpreadsheet.class, sheetObj);
    XColumnRowRange xCRRange = (XColumnRowRange) UnoRuntime.queryInterface( XColumnRowRange.class, sheet );
    XTableColumns xColumns = xCRRange.getColumns();

    // Create a cell series "A2:A8" with the values 1 ... 7.
    int nRow = 1;
    for (int i = 1; i < 8; ++i) {
        sheet.getCellByPosition( 1, nRow ).setValue( nRow );
        nRow += 1;
    }

    //Insert a row between row 2 and row 3
    xColumns.insertByIndex( 0, 1 );

    //Get value of cell C2
    XCell cell = sheet.getCellByPosition(2, 1);
    double checkvalue = 1.0;

    //Verify after insert row
    assertEquals("Verify if one new column inserted after Column A",checkvalue, cell.getValue(),0);

    //Delete the row 3 and row 4
    xColumns.removeByIndex( 0, 1 );

    //Get value of cell A3 and A4
    XCell cellA3 = sheet.getCellByPosition(1, 2);
    XCell cellA4 = sheet.getCellByPosition(1, 3);
    double checkvalueA3 = 2.0;
    double checkvalueA4 = 3.0;

    //Verify after delete row3 and row4
    assertEquals("Verify after tow rows deleted, the value of A3",checkvalueA3, cellA3.getValue(),0);
    assertEquals("Verify after tow rows deleted, the value of A4",checkvalueA4, cellA4.getValue(),0);

  }

}

