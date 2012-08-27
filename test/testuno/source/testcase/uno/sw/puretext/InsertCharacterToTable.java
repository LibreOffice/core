package testcase.uno.sw.puretext;

import static org.junit.Assert.*;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.openoffice.test.common.FileUtil;
import org.openoffice.test.common.Testspace;
import org.openoffice.test.uno.UnoApp;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.text.*;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.beans.PropertyValue;
import com.sun.star.container.XIndexAccess;
import com.sun.star.frame.XStorable;

public class InsertCharacterToTable {

    private static final UnoApp app = new UnoApp();
    private XTextDocument xTextDocument = null;
    private XMultiServiceFactory xWriterFactory = null;
    private XText xText = null;

    @Before
    public void setUp() throws Exception {
        app.start();
    }

    @After
    public void tearDown() throws Exception {
        app.close();
    }

    @Test
    public void testCreateTable() throws Exception {
        xTextDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));
        xText = xTextDocument.getText();
        XTextCursor xTextCursor = xText.createTextCursor();
        // get internal service factory of the document
        xWriterFactory = (XMultiServiceFactory) UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDocument);
        // Create a new table from the document's factory
        XTextTable xTable = (XTextTable) UnoRuntime.queryInterface(XTextTable.class,xWriterFactory.createInstance("com.sun.star.text.TextTable"));
        xTable.initialize(4, 4);
        xText.insertTextContent(xTextCursor, xTable, false);
        //insert text in to table cell
        insertIntoCell( "A1","test", xTable );
        insertIntoCell( "C4","123", xTable );
        insertIntoCell( "D2","fsdf132134", xTable );
        insertIntoCell( "B3","*^$%^$^$", xTable );

        //save to odt
        XStorable xStorable_odt = (XStorable) UnoRuntime.queryInterface(XStorable.class, xTextDocument);
        PropertyValue[] aStoreProperties_odt = new PropertyValue[2];
        aStoreProperties_odt[0] = new PropertyValue();
        aStoreProperties_odt[1] = new PropertyValue();
        aStoreProperties_odt[0].Name = "Override";
        aStoreProperties_odt[0].Value = true;
        aStoreProperties_odt[1].Name = "FilterName";
        aStoreProperties_odt[1].Value = "StarOffice XML (Writer)";
        xStorable_odt.storeToURL(FileUtil.getUrl(Testspace.getPath("output/test.odt")), aStoreProperties_odt);
        //save to doc
        XStorable xStorable_doc = (XStorable) UnoRuntime.queryInterface(XStorable.class, xTextDocument);
        PropertyValue[] aStoreProperties_doc = new PropertyValue[2];
        aStoreProperties_doc[0] = new PropertyValue();
        aStoreProperties_doc[1] = new PropertyValue();
        aStoreProperties_doc[0].Name = "Override";
        aStoreProperties_doc[0].Value = true;
        aStoreProperties_doc[1].Name = "FilterName";
        aStoreProperties_doc[1].Value = "MS Word 97";
        xStorable_doc.storeToURL(FileUtil.getUrl(Testspace.getPath("output/test.doc")), aStoreProperties_doc);
        app.closeDocument(xTextDocument);

        // reopen the document and assert create table successfully
        XTextDocument assertDocument_odt = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class,app.loadDocument(Testspace.getPath("output/test.odt")));
        XTextTablesSupplier xTablesSupplier_odt = (XTextTablesSupplier) UnoRuntime.queryInterface(XTextTablesSupplier.class, assertDocument_odt);
        XIndexAccess xIndexedTables_odt = (XIndexAccess) UnoRuntime.queryInterface(XIndexAccess.class, xTablesSupplier_odt.getTextTables());
        Object xTable_obj_odt = xIndexedTables_odt.getByIndex(0);
        XTextTable xTable_Assert_odt = (XTextTable) UnoRuntime.queryInterface(XTextTable.class, xTable_obj_odt);
        assertEquals("assert table cell text","test",getFromCell("A1", xTable_Assert_odt));
        assertEquals("assert table cell text","*^$%^$^$",getFromCell("B3", xTable_Assert_odt));
        assertEquals("assert table cell text","123",getFromCell("C4", xTable_Assert_odt));
        assertEquals("assert table cell text","fsdf132134",getFromCell("D2", xTable_Assert_odt));

        // reopen the document and assert create table successfully
        XTextDocument assertDocument_doc = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class,app.loadDocument(Testspace.getPath("output/test.doc")));
        XTextTablesSupplier xTablesSupplier_doc = (XTextTablesSupplier) UnoRuntime.queryInterface(XTextTablesSupplier.class, assertDocument_doc);
        XIndexAccess xIndexedTables_doc = (XIndexAccess) UnoRuntime.queryInterface(XIndexAccess.class, xTablesSupplier_doc.getTextTables());
        Object xTable_obj_doc = xIndexedTables_doc.getByIndex(0);
        XTextTable xTable_Assert_doc = (XTextTable) UnoRuntime.queryInterface(XTextTable.class, xTable_obj_doc);
        assertEquals("assert table cell text","test",getFromCell("A1", xTable_Assert_doc));
        assertEquals("assert table cell text","*^$%^$^$",getFromCell("B3", xTable_Assert_doc));
        assertEquals("assert table cell text","123",getFromCell("C4", xTable_Assert_doc));
        assertEquals("assert table cell text","fsdf132134",getFromCell("D2", xTable_Assert_doc));
    }
    // This method is inserts string sText in table cell by sCellName.
    public static void insertIntoCell(String sCellName, String sText, XTextTable xTable) {
        // Access the XText interface of the cell referred to by sCellName
        XText xCellText = (XText) UnoRuntime.queryInterface(XText.class, xTable.getCellByName(sCellName));
        // Set the text in the cell to sText
        xCellText.setString(sText);
    }
    // This method is get string sText in table cell by sCellName.
    public static String getFromCell(String sCellName, XTextTable xTable) {
        // Access the XText interface of the cell referred to by sCellName
        XText xCellText = (XText) UnoRuntime.queryInterface(XText.class, xTable.getCellByName(sCellName));
        // Set the text in the cell to sText
        return xCellText.getString();
    }
}