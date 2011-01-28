package complex.sfx2.undo;

import org.openoffice.test.tools.SpreadsheetDocument;
import com.sun.star.table.XCellRange;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.table.XCell;
import com.sun.star.uno.UnoRuntime;
import org.openoffice.test.tools.DocumentType;
import static org.junit.Assert.*;

/**
 * implements the {@link DocumentTest} interface on top of a spreadsheet document
 * @author frank.schoenheit@oracle.com
 */
public class CalcDocumentTest extends DocumentTestBase
{
    public CalcDocumentTest( final XMultiServiceFactory i_orb ) throws Exception
    {
        super( i_orb, DocumentType.CALC );
    }

    public String getDocumentDescription()
    {
        return "spreadsheet document";
    }

    public void initializeDocument() throws com.sun.star.uno.Exception
    {
        final XCell cellA1 = getCellA1();
        cellA1.setValue( INIT_VALUE );
        assertEquals( "initializing the cell value didn't work", cellA1.getValue(), INIT_VALUE, 0 );

        XCellRange range = UnoRuntime.queryInterface( XCellRange.class,
            ((SpreadsheetDocument)m_document).getSheet(0) );

        for ( int i=0; i<12; ++i )
        {
            XCell cell = range.getCellByPosition( 1, i );
            cell.setFormula( "" );
        }
    }

    public void doSingleModification() throws com.sun.star.uno.Exception
    {
        final XCell cellA1 = getCellA1();
        assertEquals( "initial cell value not as expected", INIT_VALUE, cellA1.getValue(), 0 );
        cellA1.setValue( MODIFIED_VALUE );
        assertEquals( "modified cell value not as expected", MODIFIED_VALUE, cellA1.getValue(), 0 );
    }

    public void verifyInitialDocumentState() throws com.sun.star.uno.Exception
    {
        final XCell cellA1 = getCellA1();
        assertEquals( "cell A1 doesn't have its initial value", INIT_VALUE, cellA1.getValue(), 0 );

        XCellRange range = UnoRuntime.queryInterface( XCellRange.class,
            ((SpreadsheetDocument)m_document).getSheet(0) );
        for ( int i=0; i<12; ++i )
        {
            final XCell cell = range.getCellByPosition( 1, i );
            assertEquals( "Cell B" + (i+1) + " not having its initial value (an empty string)", "", cell.getFormula() );
        }
    }

    public void verifySingleModificationDocumentState() throws com.sun.star.uno.Exception
    {
        final XCell cellA1 = getCellA1();
        assertEquals( "cell A1 doesn't have the value which we gave it", MODIFIED_VALUE, cellA1.getValue(), 0 );
    }

    public int doMultipleModifications() throws com.sun.star.uno.Exception
    {
        XCellRange range = UnoRuntime.queryInterface( XCellRange.class,
            ((SpreadsheetDocument)m_document).getSheet(0) );

        final String[] months = new String[] {
            "January", "February", "March", "April", "May", "June", "July", "August",
            "September", "October", "November", "December" };
        for ( int i=0; i<12; ++i )
        {
            final XCell cell = range.getCellByPosition( 1, i );
            cell.setFormula( months[i] );
        }
        return 12;
    }

    private XCell getCellA1() throws com.sun.star.uno.Exception
    {
        XCellRange range = UnoRuntime.queryInterface( XCellRange.class,
            ((SpreadsheetDocument)m_document).getSheet(0) );
        return range.getCellByPosition( 0, 0 );
    }

    private static final double INIT_VALUE = 100.0;
    private static final double MODIFIED_VALUE = 200.0;
}
