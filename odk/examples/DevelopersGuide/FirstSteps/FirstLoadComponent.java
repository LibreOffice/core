import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.beans.XPropertySet;

import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.XComponent;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetView;

import com.sun.star.table.XCell;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XController;
import com.sun.star.frame.XComponentLoader;

import com.sun.star.uno.AnyConverter;

import com.sun.star.sheet.XCellRangesQuery;
import com.sun.star.sheet.XSheetCellRanges;
import com.sun.star.sheet.XCellAddressable;
import com.sun.star.container.XEnumeration;
import com.sun.star.container.XEnumerationAccess;

/*
 * FirstLoadComponent.java
 *
 * Created on 31. März 2002, 18:21
 */

/**
 *
 * @author  dschulten
 */
public class FirstLoadComponent {

    /** Creates a new instance of FirstLoadComponent */
    public FirstLoadComponent() {
    }

    /**
     * @param args the command line arguments
     */
    private XComponentContext xRemoteContext = null;
    private XMultiComponentFactory xRemoteServiceManager = null;

    public static void main(String[] args) {
        FirstLoadComponent firstLoadComponent1 = new FirstLoadComponent();
        try {
            firstLoadComponent1.useConnection();
        }
        catch (java.lang.Exception e){
            System.out.println(e.getMessage());
            e.printStackTrace();
        }
        finally {
            System.exit(0);
        }
    }

    private void useConnection() throws java.lang.Exception {
        try {
            xRemoteServiceManager = this.getRemoteServiceManager(
                    "uno:socket,host=localhost,port=8100;urp;StarOffice.ServiceManager");
            Object desktop = xRemoteServiceManager.createInstanceWithContext(
                "com.sun.star.frame.Desktop", xRemoteContext);
            XComponentLoader xComponentLoader = (XComponentLoader)UnoRuntime.queryInterface(
                XComponentLoader.class, desktop);

            PropertyValue[] loadProps = new PropertyValue[0];
            XComponent xSpreadsheetComponent = xComponentLoader.loadComponentFromURL("private:factory/scalc", "_blank", 0, loadProps);

            XSpreadsheetDocument xSpreadsheetDocument = (XSpreadsheetDocument)UnoRuntime.queryInterface(
                XSpreadsheetDocument.class, xSpreadsheetComponent);

            XSpreadsheets xSpreadsheets = xSpreadsheetDocument.getSheets();
            xSpreadsheets.insertNewByName("MySheet", (short)0);
            com.sun.star.uno.Type elemType = xSpreadsheets.getElementType();

            System.out.println(elemType.getTypeName());
            Object sheet = xSpreadsheets.getByName("MySheet");
            XSpreadsheet xSpreadsheet = (XSpreadsheet)UnoRuntime.queryInterface(
                XSpreadsheet.class, sheet);

            XCell xCell = xSpreadsheet.getCellByPosition(0, 0);
            xCell.setValue(21);
            xCell = xSpreadsheet.getCellByPosition(0, 1);
            xCell.setValue(21);
            xCell = xSpreadsheet.getCellByPosition(0, 2);
            xCell.setFormula("=sum(A1:A2)");

            XPropertySet xCellProps = (XPropertySet)UnoRuntime.queryInterface(
                XPropertySet.class, xCell);
            xCellProps.setPropertyValue("CellStyle", "Result");

            XModel xSpreadsheetModel = (XModel)UnoRuntime.queryInterface(XModel.class, xSpreadsheetComponent);
            XController xSpreadsheetController = xSpreadsheetModel.getCurrentController();
            XSpreadsheetView xSpreadsheetView = (XSpreadsheetView)UnoRuntime.queryInterface(
                XSpreadsheetView.class, xSpreadsheetController);
            xSpreadsheetView.setActiveSheet(xSpreadsheet);

            // *********************************************************
            // example for use of enum types
            xCellProps.setPropertyValue("VertJustify", com.sun.star.table.CellVertJustify.TOP);

            // *********************************************************
            // example for a sequence of PropertyValue structs
            // create an array with one PropertyValue struct, it contains references only
            loadProps = new PropertyValue[1];

            // instantiate PropertyValue struct and set its member fields
            PropertyValue asTemplate = new PropertyValue();
            asTemplate.Name = "AsTemplate";
            asTemplate.Value = new Boolean(true);

            // assign PropertyValue struct to array of references for PropertyValue structs
            loadProps[0] = asTemplate;

            // load calc file as template
            //xSpreadsheetComponent = xComponentLoader.loadComponentFromURL(
            //    "file:///X:/Office60Eng/share/samples/english/spreadsheets/DataAnalysys.sxc", "_blank", 0, loadProps);

            // *********************************************************
            // example for use of XEnumerationAccess
            XCellRangesQuery xCellQuery = (XCellRangesQuery)UnoRuntime.queryInterface(
                XCellRangesQuery.class, sheet);
            XSheetCellRanges xFormulaCells = xCellQuery.queryContentCells(
                (short)com.sun.star.sheet.CellFlags.FORMULA);
            XEnumerationAccess xFormulas = xFormulaCells.getCells();
            XEnumeration xFormulaEnum = xFormulas.createEnumeration();

            while (xFormulaEnum.hasMoreElements()) {
                Object formulaCell = xFormulaEnum.nextElement();
                xCell = (XCell)UnoRuntime.queryInterface(XCell.class, formulaCell);
                XCellAddressable xCellAddress = (XCellAddressable)UnoRuntime.queryInterface(
                    XCellAddressable.class, xCell);
                System.out.println("Formula cell in column " + xCellAddress.getCellAddress().Column
                    + ", row " + xCellAddress.getCellAddress().Row
                    + " contains " + xCell.getFormula());
            }

        }
        catch( com.sun.star.lang.DisposedException e ) { //works from Patch 1
            xRemoteContext = null;
            throw e;
        }
    }

    private XMultiComponentFactory getRemoteServiceManager(String unoUrl) throws java.lang.Exception {
        if (xRemoteContext == null) {
            // First step: create local component context, get local servicemanager and
            // ask it to create a UnoUrlResolver object with an XUnoUrlResolver interface
            XComponentContext xLocalContext =
                com.sun.star.comp.helper.Bootstrap.createInitialComponentContext(null);

            XMultiComponentFactory xLocalServiceManager = xLocalContext.getServiceManager();

            Object urlResolver  = xLocalServiceManager.createInstanceWithContext(
                "com.sun.star.bridge.UnoUrlResolver", xLocalContext );
            // query XUnoUrlResolver interface from urlResolver object
            XUnoUrlResolver xUnoUrlResolver = (XUnoUrlResolver) UnoRuntime.queryInterface(
                XUnoUrlResolver.class, urlResolver );

            // Second step: use xUrlResolver interface to import the remote StarOffice.ServiceManager,
            // retrieve its property DefaultContext and get the remote servicemanager
            Object initialObject = xUnoUrlResolver.resolve( unoUrl );
            XPropertySet xPropertySet = (XPropertySet)UnoRuntime.queryInterface(
                XPropertySet.class, initialObject);
            Object context = xPropertySet.getPropertyValue("DefaultContext");
            xRemoteContext = (XComponentContext)UnoRuntime.queryInterface(
                XComponentContext.class, context);
        }
        return xRemoteContext.getServiceManager();
    }
}
