package complex.sfx2.undo;

import com.sun.star.text.XTextRange;
import com.sun.star.beans.XPropertySet;
import com.sun.star.table.XCell;
import com.sun.star.table.XCellRange;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextTable;
import com.sun.star.text.XText;
import com.sun.star.text.XTextDocument;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import org.openoffice.test.tools.DocumentType;
import static org.junit.Assert.*;

/**
 * implements the {@link DocumentTest} interface on top of a spreadsheet document
 * @author frank.schoenheit@oracle.com
 */
public class WriterDocumentTest extends DocumentTestBase
{
    public WriterDocumentTest( final XMultiServiceFactory i_orb ) throws com.sun.star.uno.Exception
    {
        super( i_orb, DocumentType.WRITER );
    }

    public String getDocumentDescription()
    {
        return "text document";
    }

    public void initializeDocument() throws com.sun.star.uno.Exception
    {
        // TODO?
    }

    public void doSingleModification() throws com.sun.star.uno.Exception
    {
        final XTextDocument textDoc = UnoRuntime.queryInterface( XTextDocument.class, getDocument().getDocument() );
        final XText docText = textDoc.getText();
        docText.setString( s_blindText );
    }

    public void verifyInitialDocumentState() throws com.sun.star.uno.Exception
    {
        final XTextDocument textDoc = UnoRuntime.queryInterface( XTextDocument.class, getDocument().getDocument() );
        final XText docText = textDoc.getText();
        assertEquals( "document should be empty", "", docText.getString() );
    }

    public void verifySingleModificationDocumentState() throws com.sun.star.uno.Exception
    {
        final XTextDocument textDoc = UnoRuntime.queryInterface( XTextDocument.class, getDocument().getDocument() );
        final XText docText = textDoc.getText();
        assertEquals( "blind text not found", s_blindText, docText.getString() );
    }

    public int doMultipleModifications() throws com.sun.star.uno.Exception
    {
        final XTextDocument textDoc = UnoRuntime.queryInterface( XTextDocument.class, getDocument().getDocument() );
        final XText docText = textDoc.getText();

        int expectedUndoActions = 0;

        // create a cursor
        final XTextCursor cursor = docText.createTextCursor();

        // create a table
        final XTextTable textTable = UnoRuntime.queryInterface( XTextTable.class,
            getDocument().createInstance( "com.sun.star.text.TextTable" ) );
        textTable.initialize( 3, 3 );
        final XPropertySet tableProps = UnoRuntime.queryInterface( XPropertySet.class, textTable );
        tableProps.setPropertyValue( "BackColor", 0xCCFF44 );

        // insert the table into the doc
        docText.insertTextContent( cursor, textTable, false );
        ++expectedUndoActions; //FIXME this will create 2 actions! currently the event is sent for every individual action; should it be sent for top-level actions only? how many internal actions are created is an implementation detail!
        ++expectedUndoActions;

        // write some content into the center cell
        final XCellRange cellRange = UnoRuntime.queryInterface( XCellRange.class, textTable );
        final XCell centerCell = cellRange.getCellByPosition( 1, 1 );
        final XTextRange cellText = UnoRuntime.queryInterface( XTextRange.class, centerCell );
        cellText.setString( "Undo Manager API Test" );
        ++expectedUndoActions;

        // give it another color
        final XPropertySet cellProps = UnoRuntime.queryInterface( XPropertySet.class, centerCell );
        cellProps.setPropertyValue( "BackColor", 0x44CCFF );
        ++expectedUndoActions;

        return expectedUndoActions;
    }

    private static final String s_blindText =
        "Lorem ipsum dolor. Sit amet penatibus. A cum turpis. Aenean ac eu. " +
        "Ligula est urna nulla vestibulum ullamcorper. Nec sit in amet tincidunt mus. " +
        "Tellus sagittis mi. Suscipit cursus in vestibulum in eros ipsum felis cursus lectus " +
        "nunc quis condimentum in risus nec wisi aenean luctus hendrerit magna habitasse commodo orci. " +
        "Nisl etiam quis. Vestibulum justo eleifend aliquet luctus sed turpis volutpat ullamcorper " +
        "aliquam penatibus sagittis pede tincidunt egestas. Nibh massa lectus. Sem mattis purus morbi " +
        "scelerisque turpis donec urna phasellus. Quis at lacus. Viverra mauris mollis. " +
        "Dolor tincidunt condimentum.";
}
