
import com.sun.star.uno.UnoRuntime;

// __________  implementation  ____________________________________

/** Create and modify a spreadsheet view.
 */
public class ViewSample extends SpreadsheetDocHelper
{

// ________________________________________________________________

    public static void main( String args[] )
    {
        try
        {
            ViewSample aSample = new ViewSample( args );
            aSample.doSampleFunction();
        }
        catch (Exception ex)
        {
            System.out.println( "Sample caught exception! " + ex );
            System.exit( 1 );
        }
        System.out.println( "\nSamples done." );
        System.exit( 0 );
    }

// ________________________________________________________________

    public ViewSample( String[] args )
    {
        super( args );
    }

// ________________________________________________________________

    /** This sample function performs all changes on the view. */
    public void doSampleFunction() throws Exception
    {
        com.sun.star.sheet.XSpreadsheetDocument xDoc = getDocument();
        com.sun.star.frame.XModel xModel = (com.sun.star.frame.XModel)
            UnoRuntime.queryInterface( com.sun.star.frame.XModel.class, xDoc);
        com.sun.star.frame.XController xController = xModel.getCurrentController();

        // --- Spreadsheet view ---
        // freeze the first column and first two rows
        com.sun.star.sheet.XViewFreezable xFreeze = (com.sun.star.sheet.XViewFreezable)
            UnoRuntime.queryInterface( com.sun.star.sheet.XViewFreezable.class, xController );
        if ( null != xFreeze )
            System.out.println( "got xFreeze" );
        xFreeze.freezeAtPosition( 1, 2 );

        // --- View pane ---
        // get the cell range shown in the second pane and assign a cell background to them
        com.sun.star.container.XIndexAccess xIndex = (com.sun.star.container.XIndexAccess)
            UnoRuntime.queryInterface( com.sun.star.container.XIndexAccess.class, xController );
        Object aPane = xIndex.getByIndex(1);
        com.sun.star.sheet.XCellRangeReferrer xRefer = (com.sun.star.sheet.XCellRangeReferrer)
            UnoRuntime.queryInterface( com.sun.star.sheet.XCellRangeReferrer.class, aPane );
        com.sun.star.table.XCellRange xRange = xRefer.getReferredCells();
        com.sun.star.beans.XPropertySet xRangeProp = (com.sun.star.beans.XPropertySet)
            UnoRuntime.queryInterface( com.sun.star.beans.XPropertySet.class, xRange );
        xRangeProp.setPropertyValue( "IsCellBackgroundTransparent", new Boolean( false ) );
        xRangeProp.setPropertyValue( "CellBackColor", new Integer( 0xFFFFCC ) );

        // --- View settings ---
        // change the view to display green grid lines
        com.sun.star.beans.XPropertySet xProp = (com.sun.star.beans.XPropertySet)
            UnoRuntime.queryInterface( com.sun.star.beans.XPropertySet.class, xController );
        xProp.setPropertyValue( "ShowGrid", new Boolean(true) );
        xProp.setPropertyValue( "GridColor", new Integer(0x00CC00) );

        // --- Range selection ---
        // let the user select a range and use it as the view's selection
        com.sun.star.sheet.XRangeSelection xRngSel = (com.sun.star.sheet.XRangeSelection)
            UnoRuntime.queryInterface( com.sun.star.sheet.XRangeSelection.class, xController );
        ExampleRangeListener aListener = new ExampleRangeListener();
        xRngSel.addRangeSelectionListener( aListener );
        com.sun.star.beans.PropertyValue[] aArguments = new com.sun.star.beans.PropertyValue[2];
        aArguments[0] = new com.sun.star.beans.PropertyValue();
        aArguments[0].Name   = "Title";
        aArguments[0].Value  = "Please select a cell range (e.g. C4:E6)";
        aArguments[1] = new com.sun.star.beans.PropertyValue();
        aArguments[1].Name   = "CloseOnMouseRelease";
        aArguments[1].Value  = new Boolean(true);
        xRngSel.startRangeSelection( aArguments );
        synchronized (aListener)
        {
            aListener.wait();       // wait until the selection is done
        }
        xRngSel.removeRangeSelectionListener( aListener );
        if ( aListener.aResult != null && aListener.aResult.length() != 0 )
        {
            com.sun.star.view.XSelectionSupplier xSel = (com.sun.star.view.XSelectionSupplier)
                UnoRuntime.queryInterface( com.sun.star.view.XSelectionSupplier.class, xController );
            com.sun.star.sheet.XSpreadsheetView xView = (com.sun.star.sheet.XSpreadsheetView)
                UnoRuntime.queryInterface( com.sun.star.sheet.XSpreadsheetView.class, xController );
            com.sun.star.sheet.XSpreadsheet xSheet = xView.getActiveSheet();
            com.sun.star.table.XCellRange xResultRange = xSheet.getCellRangeByName( aListener.aResult );
            xSel.select( xResultRange );
        }
    }

// ________________________________________________________________

    //  listener to react on finished selection

    private class ExampleRangeListener implements com.sun.star.sheet.XRangeSelectionListener
    {
        public String aResult;

        public void done( com.sun.star.sheet.RangeSelectionEvent aEvent )
        {
            aResult = aEvent.RangeDescriptor;
            synchronized (this)
            {
                notify();
            }
        }

        public void aborted( com.sun.star.sheet.RangeSelectionEvent aEvent )
        {
            synchronized (this)
            {
                notify();
            }
        }

        public void disposing( com.sun.star.lang.EventObject aObj )
        {
        }
    }

// ________________________________________________________________

}
