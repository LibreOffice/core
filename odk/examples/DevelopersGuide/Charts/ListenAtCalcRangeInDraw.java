/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

// __________ Imports __________

import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.*;

// property access
import com.sun.star.beans.*;

// application specific classes
import com.sun.star.chart.*;
import com.sun.star.table.XCellRange;
import com.sun.star.sheet.XSpreadsheetDocument;

import com.sun.star.frame.XModel;
// base graphics things
import com.sun.star.awt.Point;
import com.sun.star.awt.Size;
// Exceptions
import com.sun.star.uno.Exception;


// __________ Implementation __________

/** Create a spreadsheet add some data.
    Create a presentation and add a chart.
    Connect the chart to a calc range via a listener
    @author Bj&ouml;rn Milcke
 */
public class ListenAtCalcRangeInDraw implements XChartDataChangeEventListener
{
    public static void main( String args[] )
    {
        ListenAtCalcRangeInDraw aMySelf = new ListenAtCalcRangeInDraw( args );

        aMySelf.run();
    }

    public ListenAtCalcRangeInDraw( String args[] )
    {
        Helper aHelper = new Helper( args );

        maSheetDoc = aHelper.createSpreadsheetDocument();
        maDrawDoc  = aHelper.createDrawingDocument();
        CalcHelper aCalcHelper   = new CalcHelper(  maSheetDoc );
        ChartHelper aChartHelper = new ChartHelper( maDrawDoc );

        XCellRange aRange = aCalcHelper.insertFormulaRange( 3, 30 );

        // the unit for measures is 1/100th of a millimeter
        // position at (1cm, 1cm)
        Point aPos    = new Point( 1000, 1000 );

        // size of the chart is 15cm x 9.271cm
        Size  aExtent = new Size( 15000, 9271 );

        // insert a new chart into the "Chart" sheet of the
        // spreadsheet document
        maChartDocument = aChartHelper.insertOLEChartInDraw(
            "ChartWithCalcData",
            aPos,
            aExtent,
            "com.sun.star.chart.XYDiagram" );

        // attach the data coming from the cell range to the chart
        maChartData = (XChartData) UnoRuntime.queryInterface( XChartData.class, aRange );
        maChartDocument.attachData( maChartData );
    }

    // ____________________

    public void run()
    {
        try
        {
            ((XPropertySet) UnoRuntime.queryInterface(
                XPropertySet.class, maChartDocument )).setPropertyValue(
                "HasSubTitle", new Boolean( true ));

            // start listening for death of spreadsheet
            ((XComponent) UnoRuntime.queryInterface(
                XComponent.class, maSheetDoc )).addEventListener( this );

            // start listening for death of chart
            ((XComponent) UnoRuntime.queryInterface(
                XComponent.class, maChartDocument )).addEventListener( this );

            //start listening for change of data
            maChartData.addChartDataChangeEventListener( this );
        }
        catch( Exception ex )
        {
            System.out.println( "Oops: " + ex );
        }

        // call listener
        ChartDataChangeEvent aEvent = new ChartDataChangeEvent();
        aEvent.Type = ChartDataChangeType.ALL;
        chartDataChanged( aEvent );
    }

    // ____________________

    // XEventListener (base of XChartDataChangeEventListener)
    public void disposing( EventObject aSourceObj )
    {
        if( UnoRuntime.queryInterface( XChartDocument.class, aSourceObj.Source ) != null )
            System.out.println( "Disconnecting Listener because Chart was shut down" );

        if( UnoRuntime.queryInterface( XSpreadsheetDocument.class, aSourceObj.Source ) != null )
            System.out.println( "Disconnecting Listener because Spreadsheet was shut down" );

        // remove data change listener
        maChartData.removeChartDataChangeEventListener( this );

        // remove dispose listeners
        ((XComponent) UnoRuntime.queryInterface(
            XComponent.class, maSheetDoc )).removeEventListener( this );
        ((XComponent) UnoRuntime.queryInterface(
            XComponent.class, maChartDocument )).removeEventListener( this );

        System.exit( 0 );
    }

    // ____________________

    // XChartDataChangeEventListener
    public void chartDataChanged( ChartDataChangeEvent aEvent )
    {
        // update subtitle
        String aTitle = new String( "Last Update: " + new java.util.Date( System.currentTimeMillis() ));

        try
        {
            XPropertySet aDocProp = (XPropertySet) UnoRuntime.queryInterface(
                XPropertySet.class, maChartDocument );
            aDocProp.setPropertyValue( "HasMainTitle", new Boolean( true ));

            ((XPropertySet) UnoRuntime.queryInterface(
                XPropertySet.class, maChartDocument.getSubTitle())).setPropertyValue(
                "String", aTitle );

            maChartDocument.attachData( maChartData );
        }
        catch( Exception ex )
        {
            System.out.println( "Oops: " + ex );
        }

        System.out.println( "Data has changed" );
    }


    // __________ private __________

    private XSpreadsheetDocument      maSheetDoc;
    private XModel                    maDrawDoc;
    private XChartDocument            maChartDocument;
    private XChartData                maChartData;
}
