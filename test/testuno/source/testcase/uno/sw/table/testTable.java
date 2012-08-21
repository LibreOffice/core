package testcase.uno.sw.table;

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
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexAccess;
import com.sun.star.table.*;
import com.sun.star.frame.XStorable;


public class testTable {

    private static final UnoApp app = new UnoApp();
    private XTextDocument xTextDocument=null;
    private XMultiServiceFactory xWriterFactory=null;
    private XText xText=null;
    @Before
    public void setUp() throws Exception {
        app.start();
    }

    @After
    public void tearDown() throws Exception {
        app.close();
    }
  /*
   * test create table
   * 1.new a text document and create a table,5 rows,4 columns
   * 2.save to odt,close it and reopen new saved document
   * 3.check the table column count and row count
   */
    @Test
    public void testCreateTable() throws Exception {
        xTextDocument =(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));
        xText=xTextDocument.getText();
        XTextCursor xTextCursor = xText.createTextCursor();
        // get internal service factory of the document
        xWriterFactory =(XMultiServiceFactory)UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDocument);
        // Create a new table from the document's factory
        XTextTable xTable = (XTextTable)UnoRuntime.queryInterface(XTextTable.class, xWriterFactory.createInstance("com.sun.star.text.TextTable"));
        xTable.initialize(5, 4);
        xText.insertTextContent(xTextCursor,xTable,false);

        //save and reload text document
        XStorable xStorable = (XStorable) UnoRuntime.queryInterface(XStorable.class, xTextDocument);
        PropertyValue[] aStoreProperties = new PropertyValue[2];
        aStoreProperties[0] = new PropertyValue();
        aStoreProperties[1] = new PropertyValue();
        aStoreProperties[0].Name = "Override";
        aStoreProperties[0].Value = true;
        aStoreProperties[1].Name = "FilterName";
        aStoreProperties[1].Value = "StarOffice XML (Writer)";
        xStorable.storeToURL(FileUtil.getUrl(Testspace.getPath("output/test.odt")), aStoreProperties);
        app.closeDocument(xTextDocument);

        //reopen the document and assert create table successfully
        XTextDocument assertDocument=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.odt")));
        XTextTablesSupplier xTablesSupplier = (XTextTablesSupplier) UnoRuntime.queryInterface(XTextTablesSupplier.class, assertDocument );
        XIndexAccess xIndexedTables = (XIndexAccess) UnoRuntime.queryInterface(XIndexAccess.class, xTablesSupplier.getTextTables());
        Object xTable_obj=xIndexedTables.getByIndex(0);
        XTextTable xTable_Assert=(XTextTable) UnoRuntime.queryInterface(XTextTable.class, xTable_obj);
        XTableRows xRows=xTable_Assert.getRows();
        assertEquals("assert inserted table has 5 rows",5, xRows.getCount());
        XTableColumns xColumns=xTable_Assert.getColumns();
        assertEquals("assert inserted table has 4 columns",4, xColumns.getCount());
    }
    /*
     * test insert/delete row/column
     * 1.new a text document and new a table 5x4
     * 2.insert 2 rows,4 columns
     * 3.check the total row count and column count
     * 4.delete 3 row,2 column
     * 5.check the total row count and column count
     */
    @Test
    public void testInsert_Delete_Rows_Columns() throws Exception {
        xTextDocument =(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));
        xText=xTextDocument.getText();
        XTextCursor xTextCursor = xText.createTextCursor();
        // get internal service factory of the document
        xWriterFactory =(XMultiServiceFactory)UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDocument);
        // Create a new table from the document's factory
        XTextTable xTable = (XTextTable)UnoRuntime.queryInterface(XTextTable.class, xWriterFactory.createInstance("com.sun.star.text.TextTable"));
        xTable.initialize(5, 4);
        xText.insertTextContent(xTextCursor,xTable,false);
        XTableRows xRows=xTable.getRows();
        XTableColumns xColumns=xTable.getColumns();
        xRows.insertByIndex(0, 2);
        xColumns.insertByIndex(3, 4);
        assertEquals("assert inserted 2 rows",7, xRows.getCount());
        assertEquals("assert inserted 2 columns",8, xColumns.getCount());
        xRows.removeByIndex(0, 3);
        xColumns.removeByIndex(3, 2);
        assertEquals("assert delete 3 rows",4, xRows.getCount());
        assertEquals("assert delete 2 columns",6, xColumns.getCount());
        app.closeDocument(xTextDocument);
    }
    /*
     * test table tow height
     * 1.new a text document and new a table
     * 2.set the first row height and not auto fit
     * 3.save to odt,close it and reopen new saved document
     * 4.check the table first row height setting
     */
    @Test
    public void testSetRowHeight() throws Exception {
        xTextDocument =(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));
        xText=xTextDocument.getText();
        XTextCursor xTextCursor = xText.createTextCursor();
        // get internal service factory of the document
        xWriterFactory =(XMultiServiceFactory)UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDocument);
        // Create a new table from the document's factory
        XTextTable xTable = (XTextTable)UnoRuntime.queryInterface(XTextTable.class, xWriterFactory.createInstance("com.sun.star.text.TextTable"));
        xText.insertTextContent(xTextCursor,xTable,false);
        XTableRows xRows=xTable.getRows();
        //set first row not auto fit and user-defined height
        XPropertySet xRowsProps = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xRows.getByIndex(0));
        xRowsProps.setPropertyValue("IsAutoHeight",false);
        xRowsProps.setPropertyValue("Height",5001);
        //save and reload text document
        XStorable xStorable = (XStorable) UnoRuntime.queryInterface(XStorable.class, xTextDocument);
        PropertyValue[] aStoreProperties = new PropertyValue[2];
        aStoreProperties[0] = new PropertyValue();
        aStoreProperties[1] = new PropertyValue();
        aStoreProperties[0].Name = "Override";
        aStoreProperties[0].Value = true;
        aStoreProperties[1].Name = "FilterName";
        aStoreProperties[1].Value = "StarOffice XML (Writer)";
        xStorable.storeToURL(FileUtil.getUrl(Testspace.getPath("output/test.odt")), aStoreProperties);
        app.closeDocument(xTextDocument);

        //reopen the document and assert row height setting
        XTextDocument assertDocument=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.odt")));
        XTextTablesSupplier xTablesSupplier = (XTextTablesSupplier) UnoRuntime.queryInterface(XTextTablesSupplier.class, assertDocument );
        XIndexAccess xIndexedTables = (XIndexAccess) UnoRuntime.queryInterface(XIndexAccess.class, xTablesSupplier.getTextTables());
        Object xTable_obj=xIndexedTables.getByIndex(0);
        XTextTable xTable_Assert=(XTextTable) UnoRuntime.queryInterface(XTextTable.class, xTable_obj);
        XTableRows xRows_Assert=xTable_Assert.getRows();
        XPropertySet xRowsProps_assert = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xRows_Assert.getByIndex(0));
        assertEquals("assert the second row height is 5001",5001,xRowsProps_assert.getPropertyValue("Height"));
        assertEquals("assert the second row height is not autofitable",false, xRowsProps_assert.getPropertyValue("IsAutoHeight"));
    }
    /*
     * test table border setting
     * 1.new a text document and create a table
     * 2.set table border line color and style
     * 3.save to odt,close it and reopen new saved document
     * 4.check the table border setting
     */
    @Test
    public void testSetTableBorder() throws Exception {
        xTextDocument =(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));
        xText=xTextDocument.getText();
        XTextCursor xTextCursor = xText.createTextCursor();
        // get internal service factory of the document
        xWriterFactory =(XMultiServiceFactory)UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDocument);
        // Create a new table from the document's factory
        XTextTable xTable = (XTextTable)UnoRuntime.queryInterface(XTextTable.class, xWriterFactory.createInstance("com.sun.star.text.TextTable"));
        xText.insertTextContent(xTextCursor,xTable,false);
        XPropertySet xTableProps = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xTable);
        //set table border
        TableBorder tableBorder = new TableBorder();
        BorderLine[]borderLine=new BorderLine[] {new BorderLine(),new BorderLine(),new BorderLine(),new BorderLine(),new BorderLine(),new BorderLine()};
        borderLine[0].Color=0x00FF0000;
        borderLine[0].InnerLineWidth=101;
        borderLine[0].OuterLineWidth=19;
        borderLine[0].LineDistance=100;
        borderLine[1].Color =0x00FFFF00;
        borderLine[1].InnerLineWidth=101;
        borderLine[1].OuterLineWidth=19;
        borderLine[1].LineDistance=101;
        borderLine[2].Color =0x0000FF00;
        borderLine[2].InnerLineWidth=150;
        borderLine[2].OuterLineWidth=19;
        borderLine[2].LineDistance=101;
        borderLine[3].Color =0x0000FF00;
        borderLine[3].InnerLineWidth=150;
        borderLine[3].OuterLineWidth=19;
        borderLine[3].LineDistance=101;
        borderLine[4].Color =0x0000FF00;
        borderLine[4].InnerLineWidth=150;
        borderLine[4].OuterLineWidth=19;
        borderLine[4].LineDistance=101;
        borderLine[5].Color =0x0000FF00;
        borderLine[5].InnerLineWidth=150;
        borderLine[5].OuterLineWidth=19;
        borderLine[5].LineDistance=101;
        tableBorder.TopLine =borderLine[0];
        tableBorder.BottomLine =borderLine[1];
        tableBorder.LeftLine =borderLine[2];
        tableBorder.RightLine =borderLine[3];
        tableBorder.HorizontalLine =borderLine[4];
        tableBorder.VerticalLine =borderLine[5];
        tableBorder.IsBottomLineValid = true;
        tableBorder.IsLeftLineValid = true;
        tableBorder.IsRightLineValid = true;
        tableBorder.IsTopLineValid = true;
        tableBorder.IsHorizontalLineValid = true;
        tableBorder.IsVerticalLineValid = true;
        xTableProps.setPropertyValue("TableBorder", tableBorder);
        //save and reload text document
        XStorable xStorable = (XStorable) UnoRuntime.queryInterface(XStorable.class, xTextDocument);
        PropertyValue[] aStoreProperties = new PropertyValue[2];
        aStoreProperties[0] = new PropertyValue();
        aStoreProperties[1] = new PropertyValue();
        aStoreProperties[0].Name = "Override";
        aStoreProperties[0].Value = true;
        aStoreProperties[1].Name = "FilterName";
        aStoreProperties[1].Value = "StarOffice XML (Writer)";
        xStorable.storeToURL(FileUtil.getUrl(Testspace.getPath("output/test.odt")), aStoreProperties);
        app.closeDocument(xTextDocument);

        //reopen the document and assert table border setting
        XTextDocument assertDocument=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.odt")));
        XTextTablesSupplier xTablesSupplier = (XTextTablesSupplier) UnoRuntime.queryInterface(XTextTablesSupplier.class, assertDocument );
        XIndexAccess xIndexedTables = (XIndexAccess) UnoRuntime.queryInterface(XIndexAccess.class, xTablesSupplier.getTextTables());
        Object xTable_obj=xIndexedTables.getByIndex(0);
        XTextTable xTable_Assert=(XTextTable) UnoRuntime.queryInterface(XTextTable.class, xTable_obj);
        XPropertySet xTableProps_Assert = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xTable_Assert);
        TableBorder tableBorder_Assert=(TableBorder) UnoRuntime.queryInterface(TableBorder.class, xTableProps_Assert.getPropertyValue("TableBorder"));
        assertEquals("assert topline color as setting",0x00FF0000,tableBorder_Assert.TopLine.Color);
        assertEquals("assert topline innerline width as setting",101,tableBorder_Assert.TopLine.InnerLineWidth);
        assertEquals("assert topline outerlinewidth as setting",19,tableBorder_Assert.TopLine.OuterLineWidth);
        assertEquals("assert topline linedistance as setting",101,tableBorder_Assert.TopLine.LineDistance);
        assertEquals("assert bottomline color as setting",0x00FFFF00,tableBorder_Assert.BottomLine.Color);
        assertEquals("assert bottomline innerline width as setting",101,tableBorder_Assert.BottomLine.InnerLineWidth);
        assertEquals("assert bottomline outerlinewidth as setting",19,tableBorder_Assert.BottomLine.OuterLineWidth);
        assertEquals("assert bottomline linedistance as setting",101,tableBorder_Assert.BottomLine.LineDistance);
        assertEquals("assert leftline color as setting",0x0000FF00,tableBorder_Assert.LeftLine.Color);
        assertEquals("assert leftline innerline width as setting",150,tableBorder_Assert.LeftLine.InnerLineWidth);
        assertEquals("assert leftline outerlinewidth as setting",19,tableBorder_Assert.LeftLine.OuterLineWidth);
        assertEquals("assert leftline linedistance as setting",101,tableBorder_Assert.LeftLine.LineDistance);
        assertEquals("assert rightline color as setting",0x0000FF00,tableBorder_Assert.RightLine.Color);
        assertEquals("assert rightline linedistance as setting",101,tableBorder_Assert.RightLine.LineDistance);
        assertEquals("assert rightline innerline width as setting",150,tableBorder_Assert.RightLine.InnerLineWidth);
        assertEquals("assert rightline outerlinewidth as setting",19,tableBorder_Assert.RightLine.OuterLineWidth);
        assertEquals("assert HorizontalLine color as setting",0x0000FF00,tableBorder_Assert.HorizontalLine.Color);
        assertEquals("assert HorizontalLine innerline width as setting",150,tableBorder_Assert.HorizontalLine.InnerLineWidth);
        assertEquals("assert HorizontalLine outerlinewidth as setting",19,tableBorder_Assert.HorizontalLine.OuterLineWidth);
        assertEquals("assert HorizontalLine linedistance as setting",101,tableBorder_Assert.HorizontalLine.LineDistance);
        assertEquals("assert VerticalLine color as setting",0x0000FF00,tableBorder_Assert.VerticalLine.Color);
        assertEquals("assert VerticalLine innerline width as setting",150,tableBorder_Assert.VerticalLine.InnerLineWidth);
        assertEquals("assert VerticalLine outerlinewidth as setting",19,tableBorder_Assert.VerticalLine.OuterLineWidth);
        assertEquals("assert VerticalLine linedistance as setting",101,tableBorder_Assert.VerticalLine.LineDistance);
    }
    /*
     * test table spacing to page and alignment
     * 1.new a text document
     * 2.create a table
     * 3.set the table alignment to automatic,and spacing to margin
     * 4.repeat step2 5 times,and set second table alignment to manual/center/left/from left/right,and spacing to margin
     * 5.save to odt,close it and reopen the new saved document
     * 6.reopen and check the every table alignment and spacing to margin
     */
    @Test
    public void testSetTable_AlignmentAndMarginToPage() throws Exception {
        xTextDocument =(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));
        xText=xTextDocument.getText();
        XTextCursor xTextCursor = xText.createTextCursor();
        // get internal service factory of the document
        xWriterFactory =(XMultiServiceFactory)UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDocument);
        // Create  new table from the document's factory
        XTextTable xTable1 = (XTextTable)UnoRuntime.queryInterface(XTextTable.class, xWriterFactory.createInstance("com.sun.star.text.TextTable"));
        xText.insertTextContent(xTextCursor,xTable1,false);
        XPropertySet xTableProps1 = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xTable1);
        xTableProps1.setPropertyValue("HoriOrient",com.sun.star.text.HoriOrientation.FULL);
        xTableProps1.setPropertyValue("LeftMargin",2591);
        xTableProps1.setPropertyValue("RightMargin",3000);
        xTableProps1.setPropertyValue("TopMargin",2000);
        xTableProps1.setPropertyValue("BottomMargin",2000);
        XTextTable xTable2 = (XTextTable)UnoRuntime.queryInterface(XTextTable.class, xWriterFactory.createInstance("com.sun.star.text.TextTable"));
        xText.insertTextContent(xTextCursor,xTable2,false);
        XPropertySet xTableProps2 = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xTable2);
        xTableProps2.setPropertyValue("HoriOrient",com.sun.star.text.HoriOrientation.NONE);
        xTableProps2.setPropertyValue("LeftMargin",2591);
        xTableProps2.setPropertyValue("RightMargin",3000);
        xTableProps2.setPropertyValue("TopMargin",2000);
        xTableProps2.setPropertyValue("BottomMargin",2000);
        XTextTable xTable3 = (XTextTable)UnoRuntime.queryInterface(XTextTable.class, xWriterFactory.createInstance("com.sun.star.text.TextTable"));
        xText.insertTextContent(xTextCursor,xTable3,false);
        XPropertySet xTableProps3 = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xTable3);
        xTableProps3.setPropertyValue("HoriOrient",com.sun.star.text.HoriOrientation.CENTER);
        xTableProps3.setPropertyValue("LeftMargin",2000);
        xTableProps3.setPropertyValue("RightMargin",3000);
        xTableProps3.setPropertyValue("TopMargin",2000);
        xTableProps3.setPropertyValue("BottomMargin",2000);
        XTextTable xTable4 = (XTextTable)UnoRuntime.queryInterface(XTextTable.class, xWriterFactory.createInstance("com.sun.star.text.TextTable"));
        xText.insertTextContent(xTextCursor,xTable4,false);
        XPropertySet xTableProps4 = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xTable4);
        xTableProps4.setPropertyValue("HoriOrient",com.sun.star.text.HoriOrientation.LEFT);
        xTableProps4.setPropertyValue("KeepTogether",true);
        xTableProps4.setPropertyValue("RightMargin",2000);
        xTableProps4.setPropertyValue("TopMargin",2000);
        xTableProps4.setPropertyValue("BottomMargin",2000);
        XTextTable xTable5 = (XTextTable)UnoRuntime.queryInterface(XTextTable.class, xWriterFactory.createInstance("com.sun.star.text.TextTable"));
        xText.insertTextContent(xTextCursor,xTable5,false);
        XPropertySet xTableProps5 = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xTable5);
        xTableProps5.setPropertyValue("HoriOrient",com.sun.star.text.HoriOrientation.LEFT_AND_WIDTH);
        xTableProps5.setPropertyValue("TopMargin",2000);
        xTableProps5.setPropertyValue("BottomMargin",2000);
        XTextTable xTable6 = (XTextTable)UnoRuntime.queryInterface(XTextTable.class, xWriterFactory.createInstance("com.sun.star.text.TextTable"));
        xText.insertTextContent(xTextCursor,xTable6,false);
        XPropertySet xTableProps6 = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xTable6);
        xTableProps6.setPropertyValue("HoriOrient",com.sun.star.text.HoriOrientation.RIGHT);
        xTableProps6.setPropertyValue("TopMargin",2000);
        xTableProps6.setPropertyValue("BottomMargin",2000);
        //save and reload text document
        XStorable xStorable = (XStorable) UnoRuntime.queryInterface(XStorable.class, xTextDocument);
        PropertyValue[] aStoreProperties = new PropertyValue[2];
        aStoreProperties[0] = new PropertyValue();
        aStoreProperties[1] = new PropertyValue();
        aStoreProperties[0].Name = "Override";
        aStoreProperties[0].Value = true;
        aStoreProperties[1].Name = "FilterName";
        aStoreProperties[1].Value = "StarOffice XML (Writer)";
        xStorable.storeToURL(FileUtil.getUrl(Testspace.getPath("output/test.odt")), aStoreProperties);
        app.closeDocument(xTextDocument);

        //reopen the document and assert table margin to page setting
        XTextDocument assertDocument=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.odt")));
        XTextTablesSupplier xTablesSupplier = (XTextTablesSupplier) UnoRuntime.queryInterface(XTextTablesSupplier.class, assertDocument );
        XIndexAccess xIndexedTables = (XIndexAccess) UnoRuntime.queryInterface(XIndexAccess.class, xTablesSupplier.getTextTables());
        Object xTable_obj1=xIndexedTables.getByIndex(0);
        XTextTable xTable_Assert1=(XTextTable) UnoRuntime.queryInterface(XTextTable.class, xTable_obj1);
        XPropertySet xTableProps_assert1 = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTable_Assert1);
        assertEquals("assert table alignment as automatic",com.sun.star.text.HoriOrientation.FULL,xTableProps_assert1.getPropertyValue("HoriOrient"));
        assertEquals("assert left margin to page",0,xTableProps_assert1.getPropertyValue("LeftMargin"));
        assertEquals("assert right margin to page",0,xTableProps_assert1.getPropertyValue("RightMargin"));
        assertEquals("assert top margin to page",2000,xTableProps_assert1.getPropertyValue("TopMargin"));
        assertEquals("assert bottom margin to page",2000,xTableProps_assert1.getPropertyValue("BottomMargin"));
        Object xTable_obj2=xIndexedTables.getByIndex(1);
        XTextTable xTable_Assert=(XTextTable) UnoRuntime.queryInterface(XTextTable.class, xTable_obj2);
        XPropertySet xTableProps_assert2 = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTable_Assert);
        assertEquals("assert table alignment as manual",com.sun.star.text.HoriOrientation.NONE,xTableProps_assert2.getPropertyValue("HoriOrient"));
        assertEquals("assert left margin to page",2591,xTableProps_assert2.getPropertyValue("LeftMargin"));
        assertEquals("assert right margin to page",3000,xTableProps_assert2.getPropertyValue("RightMargin"));
        assertEquals("assert top margin to page",2000,xTableProps_assert2.getPropertyValue("TopMargin"));
        assertEquals("assert bottom margin to page",2000,xTableProps_assert2.getPropertyValue("BottomMargin"));
        Object xTable_obj3=xIndexedTables.getByIndex(2);
        XTextTable xTable_Assert3=(XTextTable) UnoRuntime.queryInterface(XTextTable.class, xTable_obj3);
        XPropertySet xTableProps_assert3 = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTable_Assert3);
        assertEquals("assert table alignment as center",com.sun.star.text.HoriOrientation.CENTER,xTableProps_assert3.getPropertyValue("HoriOrient"));
        assertEquals("assert top margin to page",2000,xTableProps_assert3.getPropertyValue("TopMargin"));
        assertEquals("assert bottom margin to page",2000,xTableProps_assert3.getPropertyValue("BottomMargin"));
        Object xTable_obj4=xIndexedTables.getByIndex(3);
        XTextTable xTable_Assert4=(XTextTable) UnoRuntime.queryInterface(XTextTable.class, xTable_obj4);
        XPropertySet xTableProps_assert4 = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTable_Assert4);
        assertEquals("assert table alignment as left",com.sun.star.text.HoriOrientation.LEFT,xTableProps_assert4.getPropertyValue("HoriOrient"));
        assertEquals("assert top margin to page",2000,xTableProps_assert4.getPropertyValue("TopMargin"));
        assertEquals("assert bottom margin to page",2000,xTableProps_assert4.getPropertyValue("BottomMargin"));
        Object xTable_obj5=xIndexedTables.getByIndex(4);
        XTextTable xTable_Assert5=(XTextTable) UnoRuntime.queryInterface(XTextTable.class, xTable_obj5);
        XPropertySet xTableProps_assert5 = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTable_Assert5);
        assertEquals("assert table alignment as from left",com.sun.star.text.HoriOrientation.LEFT,xTableProps_assert5.getPropertyValue("HoriOrient"));
        assertEquals("assert top margin to page",2000,xTableProps_assert5.getPropertyValue("TopMargin"));
        assertEquals("assert bottom margin to page",2000,xTableProps_assert5.getPropertyValue("BottomMargin"));
        Object xTable_obj6=xIndexedTables.getByIndex(5);
        XTextTable xTable_Assert6=(XTextTable) UnoRuntime.queryInterface(XTextTable.class, xTable_obj6);
        XPropertySet xTableProps_assert6 = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTable_Assert6);
        assertEquals("assert table alignment as right",com.sun.star.text.HoriOrientation.RIGHT,xTableProps_assert6.getPropertyValue("HoriOrient"));
        assertEquals("assert top margin to page",2000,xTableProps_assert5.getPropertyValue("TopMargin"));
        assertEquals("assert bottom margin to page",2000,xTableProps_assert5.getPropertyValue("BottomMargin"));
    }
    /*
     * test set table background with color
     * 1.new a text document and new a table
     * 2.set table background with color
     * 3.save to odt and close it,then reopen the new saved document
     * 4.check the table background color
     */
    @Test
    public void testSetTableBackColor() throws Exception {
        xTextDocument =(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));
        xText=xTextDocument.getText();
        XTextCursor xTextCursor = xText.createTextCursor();
        // get internal service factory of the document
        xWriterFactory =(XMultiServiceFactory)UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDocument);
        // Create a new table from the document's factory
        XTextTable xTable = (XTextTable)UnoRuntime.queryInterface(XTextTable.class, xWriterFactory.createInstance("com.sun.star.text.TextTable"));
        xText.insertTextContent(xTextCursor,xTable,false);
        XPropertySet xTableProps = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xTable);
        xTableProps.setPropertyValue("BackColor",0x0000FF00);

        //save and reload text document
        XStorable xStorable = (XStorable) UnoRuntime.queryInterface(XStorable.class, xTextDocument);
        PropertyValue[] aStoreProperties = new PropertyValue[2];
        aStoreProperties[0] = new PropertyValue();
        aStoreProperties[1] = new PropertyValue();
        aStoreProperties[0].Name = "Override";
        aStoreProperties[0].Value = true;
        aStoreProperties[1].Name = "FilterName";
        aStoreProperties[1].Value = "StarOffice XML (Writer)";
        xStorable.storeToURL(FileUtil.getUrl(Testspace.getPath("output/test.odt")), aStoreProperties);
        app.closeDocument(xTextDocument);

        //reopen the document and assert table margin to page setting
        XTextDocument assertDocument=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.odt")));
        XTextTablesSupplier xTablesSupplier = (XTextTablesSupplier) UnoRuntime.queryInterface(XTextTablesSupplier.class, assertDocument );
        XIndexAccess xIndexedTables = (XIndexAccess) UnoRuntime.queryInterface(XIndexAccess.class, xTablesSupplier.getTextTables());
        Object xTable_obj=xIndexedTables.getByIndex(0);
        XTextTable xTable_Assert=(XTextTable) UnoRuntime.queryInterface(XTextTable.class, xTable_obj);
        XPropertySet xTableProps_assert = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTable_Assert);
        assertEquals("verify table background color",0x0000FF00,xTableProps_assert.getPropertyValue("BackColor"));
    }
    /*test table repeat heading setting
     * 1.new a text document and create a table
     * 2.set table first row as repeat heading
     * 3.save to odt and close it,then reopen the document
     * 4.check the table first row as repeat heading
     */

    @Test
    public void testSetTableRepeatHeading() throws Exception {
        xTextDocument =(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));
        xText=xTextDocument.getText();
        XTextCursor xTextCursor = xText.createTextCursor();
        // get internal service factory of the document
        xWriterFactory =(XMultiServiceFactory)UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDocument);
        // Create a new table from the document's factory
        XTextTable xTable = (XTextTable)UnoRuntime.queryInterface(XTextTable.class, xWriterFactory.createInstance("com.sun.star.text.TextTable"));
        xText.insertTextContent(xTextCursor,xTable,false);
        XPropertySet xTableProps = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xTable);
        //set table first one row as table heading
        xTableProps.setPropertyValue("RepeatHeadline",true);
        xTableProps.setPropertyValue("HeaderRowCount",1);

        //save and reload text document
        XStorable xStorable = (XStorable) UnoRuntime.queryInterface(XStorable.class, xTextDocument);
        PropertyValue[] aStoreProperties = new PropertyValue[2];
        aStoreProperties[0] = new PropertyValue();
        aStoreProperties[1] = new PropertyValue();
        aStoreProperties[0].Name = "Override";
        aStoreProperties[0].Value = true;
        aStoreProperties[1].Name = "FilterName";
        aStoreProperties[1].Value = "StarOffice XML (Writer)";
        xStorable.storeToURL(FileUtil.getUrl(Testspace.getPath("output/test.odt")), aStoreProperties);
        app.closeDocument(xTextDocument);

        //reopen the document and assert table repeat heading setting
        XTextDocument assertDocument=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.odt")));
        XTextTablesSupplier xTablesSupplier = (XTextTablesSupplier) UnoRuntime.queryInterface(XTextTablesSupplier.class, assertDocument );
        XIndexAccess xIndexedTables = (XIndexAccess) UnoRuntime.queryInterface(XIndexAccess.class, xTablesSupplier.getTextTables());
        Object xTable_obj=xIndexedTables.getByIndex(0);
        XTextTable xTable_Assert=(XTextTable) UnoRuntime.queryInterface(XTextTable.class, xTable_obj);
        XPropertySet xTableProps_assert = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTable_Assert);
        assertEquals("verify table heading row number",1,xTableProps_assert.getPropertyValue("HeaderRowCount"));
        assertEquals("verify table heading repeat",true,xTableProps_assert.getPropertyValue("RepeatHeadline"));
    }
    /*
     * test table shadow setting
     * 1.new a text document
     * 2.create 5 tables
     * 3.set the 5 table shadow location to bottom_right,bottom_left,none,top_left,top_right,and shadow width
     * 4.save to odt and close it,then reopen the new saved document
     * 5.check the 5 table shadow location and width
     */
    @Test
    public void testSetTableShadow() throws Exception {
        xTextDocument =(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));
        xText=xTextDocument.getText();
        XTextCursor xTextCursor = xText.createTextCursor();
        // get internal service factory of the document
        xWriterFactory =(XMultiServiceFactory)UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDocument);
        // Create new table from the document's factory
        XTextTable xTable1 = (XTextTable)UnoRuntime.queryInterface(XTextTable.class, xWriterFactory.createInstance("com.sun.star.text.TextTable"));
        xText.insertTextContent(xTextCursor,xTable1,false);
        XPropertySet xTableProps1 = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xTable1);
        XTextTable xTable2 = (XTextTable)UnoRuntime.queryInterface(XTextTable.class, xWriterFactory.createInstance("com.sun.star.text.TextTable"));
        xText.insertTextContent(xTextCursor,xTable2,false);
        XPropertySet xTableProps2 = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xTable2);
        XTextTable xTable3 = (XTextTable)UnoRuntime.queryInterface(XTextTable.class, xWriterFactory.createInstance("com.sun.star.text.TextTable"));
        xText.insertTextContent(xTextCursor,xTable3,false);
        XPropertySet xTableProps3 = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xTable3);
        XTextTable xTable4 = (XTextTable)UnoRuntime.queryInterface(XTextTable.class, xWriterFactory.createInstance("com.sun.star.text.TextTable"));
        xText.insertTextContent(xTextCursor,xTable4,false);
        XPropertySet xTableProps4 = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xTable4);
        XTextTable xTable5 = (XTextTable)UnoRuntime.queryInterface(XTextTable.class, xWriterFactory.createInstance("com.sun.star.text.TextTable"));
        xText.insertTextContent(xTextCursor,xTable5,false);
        XPropertySet xTableProps5 = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xTable5);
        //set table shadow
        ShadowFormat[] shadowFormat=new ShadowFormat[] {new ShadowFormat(),new ShadowFormat(),new ShadowFormat(),new ShadowFormat(),new ShadowFormat()};
        shadowFormat[0].Location=ShadowLocation.BOTTOM_RIGHT;
        shadowFormat[0].ShadowWidth=100;
        shadowFormat[0].Color=0x00FF00FF;
        shadowFormat[1].Location=ShadowLocation.BOTTOM_LEFT;
        shadowFormat[1].ShadowWidth=100;
        shadowFormat[1].Color=0x00FF00FF;
        shadowFormat[2].Location=ShadowLocation.NONE;
        shadowFormat[3].Location=ShadowLocation.TOP_LEFT;
        shadowFormat[3].ShadowWidth=100;
        shadowFormat[3].Color=0x00FF00FF;
        shadowFormat[4].Location=ShadowLocation.TOP_RIGHT;
        shadowFormat[4].ShadowWidth=100;
        shadowFormat[4].Color=0x00FF00FF;
        xTableProps1.setPropertyValue("ShadowFormat",shadowFormat[0]);
        xTableProps2.setPropertyValue("ShadowFormat",shadowFormat[1]);
        xTableProps3.setPropertyValue("ShadowFormat",shadowFormat[2]);
        xTableProps4.setPropertyValue("ShadowFormat",shadowFormat[3]);
        xTableProps5.setPropertyValue("ShadowFormat",shadowFormat[4]);

        //save and reload text document
        XStorable xStorable = (XStorable) UnoRuntime.queryInterface(XStorable.class, xTextDocument);
        PropertyValue[] aStoreProperties = new PropertyValue[2];
        aStoreProperties[0] = new PropertyValue();
        aStoreProperties[1] = new PropertyValue();
        aStoreProperties[0].Name = "Override";
        aStoreProperties[0].Value = true;
        aStoreProperties[1].Name = "FilterName";
        aStoreProperties[1].Value = "StarOffice XML (Writer)";
        xStorable.storeToURL(FileUtil.getUrl(Testspace.getPath("output/test.odt")), aStoreProperties);
        app.closeDocument(xTextDocument);

        //reopen the document and assert table shadow setting
        XTextDocument assertDocument=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.odt")));
        XTextTablesSupplier xTablesSupplier = (XTextTablesSupplier) UnoRuntime.queryInterface(XTextTablesSupplier.class, assertDocument );
        XIndexAccess xIndexedTables = (XIndexAccess) UnoRuntime.queryInterface(XIndexAccess.class, xTablesSupplier.getTextTables());
        Object xTable_obj1=xIndexedTables.getByIndex(0);
        XTextTable xTable_Assert1=(XTextTable) UnoRuntime.queryInterface(XTextTable.class, xTable_obj1);
        XPropertySet xTableProps_assert1 = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTable_Assert1);
        ShadowFormat shadowFormat_Assert1=(ShadowFormat) UnoRuntime.queryInterface(ShadowFormat.class, xTableProps_assert1.getPropertyValue("ShadowFormat"));
        assertEquals("assert shadow location",ShadowLocation.BOTTOM_RIGHT,shadowFormat_Assert1.Location);
        assertEquals("assert shadow width",100,shadowFormat_Assert1.ShadowWidth);
        assertEquals("assert shadow color",0x00FF00FF,shadowFormat_Assert1.Color);

        Object xTable_obj2=xIndexedTables.getByIndex(1);
        XTextTable xTable_Assert2=(XTextTable) UnoRuntime.queryInterface(XTextTable.class, xTable_obj2);
        XPropertySet xTableProps_assert2 = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTable_Assert2);
        ShadowFormat shadowFormat_Assert2=(ShadowFormat) UnoRuntime.queryInterface(ShadowFormat.class, xTableProps_assert2.getPropertyValue("ShadowFormat"));
        assertEquals("assert shadow location",ShadowLocation.BOTTOM_LEFT,shadowFormat_Assert2.Location);
        assertEquals("assert shadow width",100,shadowFormat_Assert2.ShadowWidth);
        assertEquals("assert shadow color",0x00FF00FF,shadowFormat_Assert2.Color);

        Object xTable_obj3=xIndexedTables.getByIndex(2);
        XTextTable xTable_Assert3=(XTextTable) UnoRuntime.queryInterface(XTextTable.class, xTable_obj3);
        XPropertySet xTableProps_assert3 = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTable_Assert3);
        ShadowFormat shadowFormat_Assert3=(ShadowFormat) UnoRuntime.queryInterface(ShadowFormat.class, xTableProps_assert3.getPropertyValue("ShadowFormat"));
        assertEquals("assert shadow location",ShadowLocation.NONE,shadowFormat_Assert3.Location);

        Object xTable_obj4=xIndexedTables.getByIndex(3);
        XTextTable xTable_Assert4=(XTextTable) UnoRuntime.queryInterface(XTextTable.class, xTable_obj4);
        XPropertySet xTableProps_assert4 = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTable_Assert4);
        ShadowFormat shadowFormat_Assert4=(ShadowFormat) UnoRuntime.queryInterface(ShadowFormat.class, xTableProps_assert4.getPropertyValue("ShadowFormat"));
        assertEquals("assert shadow location",ShadowLocation.TOP_LEFT,shadowFormat_Assert4.Location);
        assertEquals("assert shadow width",100,shadowFormat_Assert4.ShadowWidth);
        assertEquals("assert shadow color",0x00FF00FF,shadowFormat_Assert4.Color);

        Object xTable_obj5=xIndexedTables.getByIndex(4);
        XTextTable xTable_Assert5=(XTextTable) UnoRuntime.queryInterface(XTextTable.class, xTable_obj5);
        XPropertySet xTableProps_assert5 = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTable_Assert5);
        ShadowFormat shadowFormat_Assert5=(ShadowFormat) UnoRuntime.queryInterface(ShadowFormat.class, xTableProps_assert5.getPropertyValue("ShadowFormat"));
        assertEquals("assert shadow location",ShadowLocation.TOP_RIGHT,shadowFormat_Assert5.Location);
        assertEquals("assert shadow width",100,shadowFormat_Assert5.ShadowWidth);
        assertEquals("assert shadow color",0x00FF00FF,shadowFormat_Assert5.Color);
    }
    /*
     * test set table background with graphic
     * 1.new a text document and create a table
     * 2.set table background with a picture
     * 3.save to odt and closet it,then reopen the new saved document
     * 4.check the table background
     */
    @Test
    public void testSetTableBackGraphic() throws Exception {
        xTextDocument =(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));
        xText=xTextDocument.getText();
        XTextCursor xTextCursor = xText.createTextCursor();
        // get internal service factory of the document
        xWriterFactory =(XMultiServiceFactory)UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDocument);
        // Create a new table from the document's factory
        XTextTable xTable1 = (XTextTable)UnoRuntime.queryInterface(XTextTable.class, xWriterFactory.createInstance("com.sun.star.text.TextTable"));
        xText.insertTextContent(xTextCursor,xTable1,false);
        XPropertySet xTableProps1 = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xTable1);
        xTableProps1.setPropertyValue("BackGraphicURL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/table/Desert.jpg")));
        xTableProps1.setPropertyValue("BackGraphicFilter","draw_jpg_Export");
        xTableProps1.setPropertyValue("BackGraphicLocation",com.sun.star.style.GraphicLocation.LEFT_BOTTOM);

        XTextTable xTable2 = (XTextTable)UnoRuntime.queryInterface(XTextTable.class, xWriterFactory.createInstance("com.sun.star.text.TextTable"));
        xText.insertTextContent(xTextCursor,xTable2,false);
        XPropertySet xTableProps2 = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xTable2);
        xTableProps2.setPropertyValue("BackGraphicURL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/table/Desert.jpg")));
        xTableProps2.setPropertyValue("BackGraphicFilter","draw_jpg_Export");
        xTableProps2.setPropertyValue("BackGraphicLocation",com.sun.star.style.GraphicLocation.LEFT_MIDDLE);

        XTextTable xTable3 = (XTextTable)UnoRuntime.queryInterface(XTextTable.class, xWriterFactory.createInstance("com.sun.star.text.TextTable"));
        xText.insertTextContent(xTextCursor,xTable3,false);
        XPropertySet xTableProps3 = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xTable3);
        xTableProps3.setPropertyValue("BackGraphicURL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/table/Desert.jpg")));
        xTableProps3.setPropertyValue("BackGraphicFilter","draw_jpg_Export");
        xTableProps3.setPropertyValue("BackGraphicLocation",com.sun.star.style.GraphicLocation.LEFT_TOP);

        XTextTable xTable4 = (XTextTable)UnoRuntime.queryInterface(XTextTable.class, xWriterFactory.createInstance("com.sun.star.text.TextTable"));
        xText.insertTextContent(xTextCursor,xTable4,false);
        XPropertySet xTableProps4 = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xTable4);
        xTableProps4.setPropertyValue("BackGraphicURL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/table/Desert.jpg")));
        xTableProps4.setPropertyValue("BackGraphicFilter","draw_jpg_Export");
        xTableProps4.setPropertyValue("BackGraphicLocation",com.sun.star.style.GraphicLocation.MIDDLE_BOTTOM);

        XTextTable xTable5 = (XTextTable)UnoRuntime.queryInterface(XTextTable.class, xWriterFactory.createInstance("com.sun.star.text.TextTable"));
        xText.insertTextContent(xTextCursor,xTable5,false);
        XPropertySet xTableProps5 = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xTable5);
        xTableProps5.setPropertyValue("BackGraphicURL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/table/Desert.jpg")));
        xTableProps5.setPropertyValue("BackGraphicFilter","draw_jpg_Export");
        xTableProps5.setPropertyValue("BackGraphicLocation",com.sun.star.style.GraphicLocation.MIDDLE_MIDDLE);

        XTextTable xTable6 = (XTextTable)UnoRuntime.queryInterface(XTextTable.class, xWriterFactory.createInstance("com.sun.star.text.TextTable"));
        xText.insertTextContent(xTextCursor,xTable6,false);
        XPropertySet xTableProps6 = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xTable6);
        xTableProps6.setPropertyValue("BackGraphicURL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/table/Desert.jpg")));
        xTableProps6.setPropertyValue("BackGraphicFilter","draw_jpg_Export");
        xTableProps6.setPropertyValue("BackGraphicLocation",com.sun.star.style.GraphicLocation.MIDDLE_TOP);

        XTextTable xTable7 = (XTextTable)UnoRuntime.queryInterface(XTextTable.class, xWriterFactory.createInstance("com.sun.star.text.TextTable"));
        xText.insertTextContent(xTextCursor,xTable7,false);
        XPropertySet xTableProps7 = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xTable7);
        xTableProps7.setPropertyValue("BackGraphicURL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/table/Desert.jpg")));
        xTableProps7.setPropertyValue("BackGraphicFilter","draw_jpg_Export");
        xTableProps7.setPropertyValue("BackGraphicLocation",com.sun.star.style.GraphicLocation.NONE);

        XTextTable xTable8 = (XTextTable)UnoRuntime.queryInterface(XTextTable.class, xWriterFactory.createInstance("com.sun.star.text.TextTable"));
        xText.insertTextContent(xTextCursor,xTable8,false);
        XPropertySet xTableProps8 = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xTable8);
        xTableProps8.setPropertyValue("BackGraphicURL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/table/Desert.jpg")));
        xTableProps8.setPropertyValue("BackGraphicFilter","draw_jpg_Export");
        xTableProps8.setPropertyValue("BackGraphicLocation",com.sun.star.style.GraphicLocation.RIGHT_BOTTOM);

        XTextTable xTable9 = (XTextTable)UnoRuntime.queryInterface(XTextTable.class, xWriterFactory.createInstance("com.sun.star.text.TextTable"));
        xText.insertTextContent(xTextCursor,xTable9,false);
        XPropertySet xTableProps9 = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xTable9);
        xTableProps9.setPropertyValue("BackGraphicURL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/table/Desert.jpg")));
        xTableProps9.setPropertyValue("BackGraphicFilter","draw_jpg_Export");
        xTableProps9.setPropertyValue("BackGraphicLocation",com.sun.star.style.GraphicLocation.RIGHT_MIDDLE);

        XTextTable xTable10 = (XTextTable)UnoRuntime.queryInterface(XTextTable.class, xWriterFactory.createInstance("com.sun.star.text.TextTable"));
        xText.insertTextContent(xTextCursor,xTable10,false);
        XPropertySet xTableProps10 = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xTable10);
        xTableProps10.setPropertyValue("BackGraphicURL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/table/Desert.jpg")));
        xTableProps10.setPropertyValue("BackGraphicFilter","draw_jpg_Export");
        xTableProps10.setPropertyValue("BackGraphicLocation",com.sun.star.style.GraphicLocation.RIGHT_TOP);

        XTextTable xTable11 = (XTextTable)UnoRuntime.queryInterface(XTextTable.class, xWriterFactory.createInstance("com.sun.star.text.TextTable"));
        xText.insertTextContent(xTextCursor,xTable11,false);
        XPropertySet xTableProps11 = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xTable11);
        xTableProps11.setPropertyValue("BackGraphicURL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/table/Desert.jpg")));
        xTableProps11.setPropertyValue("BackGraphicFilter","draw_jpg_Export");
        xTableProps11.setPropertyValue("BackGraphicLocation",com.sun.star.style.GraphicLocation.AREA);
        //save and reload text document
        XStorable xStorable = (XStorable) UnoRuntime.queryInterface(XStorable.class, xTextDocument);
        PropertyValue[] aStoreProperties = new PropertyValue[2];
        aStoreProperties[0] = new PropertyValue();
        aStoreProperties[1] = new PropertyValue();
        aStoreProperties[0].Name = "Override";
        aStoreProperties[0].Value = true;
        aStoreProperties[1].Name = "FilterName";
        aStoreProperties[1].Value = "StarOffice XML (Writer)";
        xStorable.storeToURL(FileUtil.getUrl(Testspace.getPath("output/test.odt")), aStoreProperties);
        app.closeDocument(xTextDocument);

        //reopen the document and assert table margin to page setting
        XTextDocument assertDocument=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.odt")));
        XTextTablesSupplier xTablesSupplier = (XTextTablesSupplier) UnoRuntime.queryInterface(XTextTablesSupplier.class, assertDocument );
        XIndexAccess xIndexedTables = (XIndexAccess) UnoRuntime.queryInterface(XIndexAccess.class, xTablesSupplier.getTextTables());
        Object xTable_obj1=xIndexedTables.getByIndex(0);
        XTextTable xTable_Assert1=(XTextTable) UnoRuntime.queryInterface(XTextTable.class, xTable_obj1);
        XPropertySet xTableProps1_assert = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTable_Assert1);
        assertEquals("verify table backgraphic location",com.sun.star.style.GraphicLocation.LEFT_BOTTOM,xTableProps1_assert.getPropertyValue("BackGraphicLocation"));
        assertEquals("verify table backgraphic fileter","draw_jpg_Export",xTableProps1_assert.getPropertyValue("BackGraphicFilter"));
        assertEquals("verify table backgraphic URL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/table/Desert.jpg")),xTableProps1_assert.getPropertyValue("BackGraphicURL"));

        Object xTable_obj2=xIndexedTables.getByIndex(1);
        XTextTable xTable_Assert2=(XTextTable) UnoRuntime.queryInterface(XTextTable.class, xTable_obj2);
        XPropertySet xTableProps2_assert = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTable_Assert2);
        assertEquals("verify table backgraphic location",com.sun.star.style.GraphicLocation.LEFT_MIDDLE,xTableProps2_assert.getPropertyValue("BackGraphicLocation"));
        assertEquals("verify table backgraphic fileter","draw_jpg_Export",xTableProps2_assert.getPropertyValue("BackGraphicFilter"));
        assertEquals("verify table backgraphic URL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/table/Desert.jpg")),xTableProps2_assert.getPropertyValue("BackGraphicURL"));

        Object xTable_obj3=xIndexedTables.getByIndex(2);
        XTextTable xTable_Assert3=(XTextTable) UnoRuntime.queryInterface(XTextTable.class, xTable_obj3);
        XPropertySet xTableProps3_assert = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTable_Assert3);
        assertEquals("verify table backgraphic location",com.sun.star.style.GraphicLocation.LEFT_TOP,xTableProps3_assert.getPropertyValue("BackGraphicLocation"));
        assertEquals("verify table backgraphic fileter","draw_jpg_Export",xTableProps3_assert.getPropertyValue("BackGraphicFilter"));
        assertEquals("verify table backgraphic URL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/table/Desert.jpg")),xTableProps3_assert.getPropertyValue("BackGraphicURL"));

        Object xTable_obj4=xIndexedTables.getByIndex(3);
        XTextTable xTable_Assert4=(XTextTable) UnoRuntime.queryInterface(XTextTable.class, xTable_obj4);
        XPropertySet xTableProps4_assert = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTable_Assert4);
        assertEquals("verify table backgraphic location",com.sun.star.style.GraphicLocation.MIDDLE_BOTTOM,xTableProps4_assert.getPropertyValue("BackGraphicLocation"));
        assertEquals("verify table backgraphic fileter","draw_jpg_Export",xTableProps4_assert.getPropertyValue("BackGraphicFilter"));
        assertEquals("verify table backgraphic URL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/table/Desert.jpg")),xTableProps4_assert.getPropertyValue("BackGraphicURL"));

        Object xTable_obj5=xIndexedTables.getByIndex(4);
        XTextTable xTable_Assert5=(XTextTable) UnoRuntime.queryInterface(XTextTable.class, xTable_obj5);
        XPropertySet xTableProps5_assert = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTable_Assert5);
        assertEquals("verify table backgraphic location",com.sun.star.style.GraphicLocation.MIDDLE_MIDDLE,xTableProps5_assert.getPropertyValue("BackGraphicLocation"));
        assertEquals("verify table backgraphic fileter","draw_jpg_Export",xTableProps5_assert.getPropertyValue("BackGraphicFilter"));
        assertEquals("verify table backgraphic URL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/table/Desert.jpg")),xTableProps5_assert.getPropertyValue("BackGraphicURL"));

        Object xTable_obj6=xIndexedTables.getByIndex(5);
        XTextTable xTable_Assert6=(XTextTable) UnoRuntime.queryInterface(XTextTable.class, xTable_obj6);
        XPropertySet xTableProps6_assert = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTable_Assert6);
        assertEquals("verify table backgraphic location",com.sun.star.style.GraphicLocation.MIDDLE_TOP,xTableProps6_assert.getPropertyValue("BackGraphicLocation"));
        assertEquals("verify table backgraphic fileter","draw_jpg_Export",xTableProps6_assert.getPropertyValue("BackGraphicFilter"));
        assertEquals("verify table backgraphic URL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/table/Desert.jpg")),xTableProps6_assert.getPropertyValue("BackGraphicURL"));

        Object xTable_obj7=xIndexedTables.getByIndex(6);
        XTextTable xTable_Assert7=(XTextTable) UnoRuntime.queryInterface(XTextTable.class, xTable_obj7);
        XPropertySet xTableProps7_assert = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTable_Assert7);
        assertEquals("verify table backgraphic location is title",com.sun.star.style.GraphicLocation.NONE,xTableProps7_assert.getPropertyValue("BackGraphicLocation"));

        Object xTable_obj8=xIndexedTables.getByIndex(7);
        XTextTable xTable_Assert8=(XTextTable) UnoRuntime.queryInterface(XTextTable.class, xTable_obj8);
        XPropertySet xTableProps8_assert = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTable_Assert8);
        assertEquals("verify table backgraphic location",com.sun.star.style.GraphicLocation.RIGHT_BOTTOM,xTableProps8_assert.getPropertyValue("BackGraphicLocation"));
        assertEquals("verify table backgraphic fileter","draw_jpg_Export",xTableProps8_assert.getPropertyValue("BackGraphicFilter"));
        assertEquals("verify table backgraphic URL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/table/Desert.jpg")),xTableProps8_assert.getPropertyValue("BackGraphicURL"));

        Object xTable_obj9=xIndexedTables.getByIndex(8);
        XTextTable xTable_Assert9=(XTextTable) UnoRuntime.queryInterface(XTextTable.class, xTable_obj9);
        XPropertySet xTableProps9_assert = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTable_Assert9);
        assertEquals("verify table backgraphic location",com.sun.star.style.GraphicLocation.RIGHT_MIDDLE,xTableProps9_assert.getPropertyValue("BackGraphicLocation"));
        assertEquals("verify table backgraphic fileter","draw_jpg_Export",xTableProps9_assert.getPropertyValue("BackGraphicFilter"));
        assertEquals("verify table backgraphic URL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/table/Desert.jpg")),xTableProps9_assert.getPropertyValue("BackGraphicURL"));

        Object xTable_obj10=xIndexedTables.getByIndex(9);
        XTextTable xTable_Assert10=(XTextTable) UnoRuntime.queryInterface(XTextTable.class, xTable_obj10);
        XPropertySet xTableProps10_assert = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTable_Assert10);
        assertEquals("verify table backgraphic location",com.sun.star.style.GraphicLocation.RIGHT_TOP,xTableProps10_assert.getPropertyValue("BackGraphicLocation"));
        assertEquals("verify table backgraphic fileter","draw_jpg_Export",xTableProps10_assert.getPropertyValue("BackGraphicFilter"));
        assertEquals("verify table backgraphic URL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/table/Desert.jpg")),xTableProps10_assert.getPropertyValue("BackGraphicURL"));

        Object xTable_obj11=xIndexedTables.getByIndex(10);
        XTextTable xTable_Assert11=(XTextTable) UnoRuntime.queryInterface(XTextTable.class, xTable_obj11);
        XPropertySet xTableProps11_assert = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTable_Assert11);
        assertEquals("verify table backgraphic location",com.sun.star.style.GraphicLocation.AREA,xTableProps11_assert.getPropertyValue("BackGraphicLocation"));
        assertEquals("verify table backgraphic fileter","draw_jpg_Export",xTableProps11_assert.getPropertyValue("BackGraphicFilter"));
        assertEquals("verify table backgraphic URL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/table/Desert.jpg")),xTableProps11_assert.getPropertyValue("BackGraphicURL"));
    }
}

