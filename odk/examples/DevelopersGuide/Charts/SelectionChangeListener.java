// __________ Imports __________

// base classes
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.*;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XController;

// application specific classes
import com.sun.star.chart.*;

import com.sun.star.view.XSelectionChangeListener;
import com.sun.star.view.XSelectionSupplier;

import com.sun.star.table.CellRangeAddress;
import com.sun.star.table.XCellRange;
import com.sun.star.sheet.XCellRangeAddressable;

import com.sun.star.awt.Point;
import com.sun.star.awt.Size;

// Exceptions
import com.sun.star.uno.Exception;
import com.sun.star.uno.RuntimeException;

// for output messages
import javax.swing.JOptionPane;

// __________ Implementation __________

/** Create a spreadsheet add some data.
    Create a presentation and add a chart.
    Connect the chart to a calc range via a listener

    Note: This example does not work in StarOffice 6.0.  It will be available
          in the StarOffice Accessibility release.

    @author Bj&ouml;rn Milcke
 */
public class SelectionChangeListener implements XSelectionChangeListener
{
    public static void main( String args[] )
    {
        SelectionChangeListener aMySelf = new SelectionChangeListener( args );

        aMySelf.run();
    }

    public SelectionChangeListener( String args[] )
    {
        Helper aHelper = new Helper( args );

        CalcHelper aCalcHelper = new CalcHelper( aHelper.createSpreadsheetDocument() );

        // insert a cell range with 4 columns and 12 rows filled with random numbers
        XCellRange aRange = aCalcHelper.insertRandomRange( 4, 12 );
        CellRangeAddress aRangeAddress = ((XCellRangeAddressable) UnoRuntime.queryInterface(
            XCellRangeAddressable.class, aRange)).getRangeAddress();

        // change view to sheet containing the chart
        aCalcHelper.raiseChartSheet();

        // the unit for measures is 1/100th of a millimeter
        // position at (1cm, 1cm)
        Point aPos    = new Point( 1000, 1000 );

        // size of the chart is 15cm x 9.271cm
        Size  aExtent = new Size( 15000, 9271 );

        // insert a new chart into the "Chart" sheet of the
        // spreadsheet document
        maChartDocument = aCalcHelper.insertChart(
            "SampleChart",
            aRangeAddress,
            aPos,
            aExtent,
            "com.sun.star.chart.XYDiagram" );
    }

    // ____________________

    public void run()
    {
        boolean bTrying = true;

        while( bTrying )
        {
            // start listening for selection changes
            XSelectionSupplier aSelSupp = (XSelectionSupplier) UnoRuntime.queryInterface(
                XSelectionSupplier.class,
                (((XModel) UnoRuntime.queryInterface(
                      XModel.class, maChartDocument )).getCurrentController()) );
            if( aSelSupp != null )
            {
                aSelSupp.addSelectionChangeListener( this );
                System.out.println( "Successfully attached as selection change listener" );
                bTrying = false;
            }

            // start listening for death of Controller
            XComponent aComp = (XComponent) UnoRuntime.queryInterface( XComponent.class, aSelSupp );
            if( aComp != null )
            {
                aComp.addEventListener( this );
                System.out.println( "Successfully attached as dispose listener" );
            }

            try
            {
                Thread.currentThread().sleep( 500 );
            }
            catch( InterruptedException ex )
            {
            }
        }
    }

    // ____________________

    // XEventListener (base of XSelectionChangeListener)
    public void disposing( EventObject aSourceObj )
    {
        System.out.println( "disposing called.  detaching as listener" );

        // stop listening for selection changes
        XSelectionSupplier aCtrl = (XSelectionSupplier) UnoRuntime.queryInterface(
            XSelectionSupplier.class, aSourceObj );
        if( aCtrl != null )
            aCtrl.removeSelectionChangeListener( this );

        // remove as dispose listener
        XComponent aComp = (XComponent) UnoRuntime.queryInterface( XComponent.class, aSourceObj );
        if( aComp != null )
            aComp.removeEventListener( this );

        // bail out
        System.exit( 0 );
    }

    // ____________________

    // XSelectionChangeListener
    public void selectionChanged( EventObject aEvent )
    {
        XController aCtrl = (XController) UnoRuntime.queryInterface( XController.class, aEvent.Source );
        if( aCtrl != null )
        {
            JOptionPane.showMessageDialog( null, "Selection has changed", "Got new Event",
                                           JOptionPane.INFORMATION_MESSAGE );
        }
    }

    // __________ private __________

    private XChartDocument            maChartDocument;
}
