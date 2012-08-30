package testcase.gui.sc.cell;

import static org.junit.Assert.*;
import static testlib.gui.AppUtil.*;
import static testlib.gui.UIMap.*;

import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.openoffice.test.common.Logger;

import testlib.gui.CalcUtil;

/**
 * Before running the testing class, you need specify the AOO location firstly
 * with system property openoffice.home.
 *
 *
 */

public class InserCells {

    @Rule
    public Logger log = Logger.getLogger(this);

    @Before
    public void setUp() throws Exception {
        app.start();
        app.dispatch("private:factory/scalc");
        calc.waitForExistence(10, 3);
    }

    @After
    public void tearDown() throws Exception {
        app.close();
    }

    /**
     * Shift row and column, insert entire row and column
     *
     * @throws Exception
     */

    @Test
    public void testShiftRowandColumn() {

        // Input data to cell range A1:B2
        CalcUtil.selectRange("A1");
        typeKeys("1<right>2<down><left>3<right>4");

        // Set expected result after executing shift cell down
        String[][] expectedShiftCellDownResult = new String[][] { { "", "2" }, { "1", "4" }, { "3", "" }, };

        // Select Cell A1
        CalcUtil.selectRange("Sheet1.A1");

        // Launch insert cells dialog via menu
        calc.menuItem("Insert->Cells...").select();

        // Select the first option "shift cells down" from dialog
        typeKeys("<enter>");

        // Verify results after shift one cell down
        assertArrayEquals("Verify results after shift one cell down", expectedShiftCellDownResult, CalcUtil.getCellTexts("A1:B3"));

        // Set expected result after executing shift cell right
        String[][] expectedShiftCellRightResult = new String[][] { { "", "1", "2" }, { "3", "4", "" }, };

        // Undo
        calc.menuItem("Edit->Undo: Insert").select();

        // Select cell B2
        CalcUtil.selectRange("Sheet1.A1");

        // Launch insert cells dialog via menu
        calc.menuItem("Insert->Cells...").select();

        // Select the second option "shift cells right" from dialog
        typeKeys("<down>");
        typeKeys("<enter>");

        // Verify results after shift one cell right
        assertArrayEquals("Verify results after shift one cell right", expectedShiftCellRightResult, CalcUtil.getCellTexts("A1:C2"));

        // Set expected result after executing insert entire row
        String[][] expectedEntireRowResult = new String[][] { { "", "" }, { "1", "2" }, { "3", "4" }, };

        // Undo
        calc.menuItem("Edit->Undo: Insert").select();

        // Select Cell B2
        CalcUtil.selectRange("Sheet1.A1");

        // Launch insert cells dialog via menu
        calc.menuItem("Insert->Cells...").select();

        // Select the third option "Entire row" from dialog
        typeKeys("<down>");
        typeKeys("<enter>");

        // Verify results after insert entire row
        assertArrayEquals("Verify results after insert entire row", expectedEntireRowResult, CalcUtil.getCellTexts("A1:B3"));

        // Set expected result after executing insert entire column
        String[][] expectedEntireColumnResult = new String[][] {

        { "", "1", "2" }, { "", "3", "4" }, };

        // Undo
        calc.menuItem("Edit->Undo: Insert").select();

        // Select Cell A1
        CalcUtil.selectRange("Sheet1.A1");

        // Launch insert cells dialog via menu
        calc.menuItem("Insert->Cells...").select();

        // Select the fourth option "Entire column" from dialog
        typeKeys("<down>");
        typeKeys("<enter>");

        // Verify the results after inserting entire column
        assertArrayEquals("Verify the results after inserting entire column", expectedEntireColumnResult, CalcUtil.getCellTexts("A1:C2"));

    }

}
