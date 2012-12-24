/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



import com.sun.star.awt.XMessageBox;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.frame.XDesktop;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.*;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XController;

// application specific classes
import com.sun.star.chart.*;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.XInterface;

import com.sun.star.view.XSelectionChangeListener;
import com.sun.star.view.XSelectionSupplier;

import com.sun.star.table.CellRangeAddress;
import com.sun.star.table.XCellRange;
import com.sun.star.sheet.XCellRangeAddressable;

import com.sun.star.awt.Point;
import com.sun.star.awt.Rectangle;
import com.sun.star.awt.Size;
import com.sun.star.awt.XMessageBoxFactory;
import com.sun.star.awt.MessageBoxType;
import com.sun.star.awt.XWindow;

// __________ Implementation __________

/** Create a spreadsheet add some data.
 * Create a presentation and add a chart.
 * Connect the chart to a calc range via a listener
 *
 * Note: This example does not work in StarOffice 6.0.  It will be available
 * in the StarOffice Accessibility release.
 *
 * @author Bj&ouml;rn Milcke
 */
public class SelectionChangeListener implements XSelectionChangeListener {
    public static void main( String args[] ) {
        SelectionChangeListener aMySelf = new SelectionChangeListener( args );

        aMySelf.run();
    }

    public SelectionChangeListener( String args[] ) {
        Helper aHelper = new Helper( args );

        maContext = aHelper.getComponentContext();

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

    public void run() {
        boolean bTrying = true;

        while( bTrying ) {
            // start listening for selection changes
            XSelectionSupplier aSelSupp = (XSelectionSupplier) UnoRuntime.queryInterface(
                XSelectionSupplier.class,
                (((XModel) UnoRuntime.queryInterface(
                XModel.class, maChartDocument )).getCurrentController()) );
            if( aSelSupp != null ) {
                aSelSupp.addSelectionChangeListener( this );
                System.out.println( "Successfully attached as selection change listener" );
                bTrying = false;
            }

            // start listening for death of Controller
            XComponent aComp = (XComponent) UnoRuntime.queryInterface( XComponent.class, aSelSupp );
            if( aComp != null ) {
                aComp.addEventListener( this );
                System.out.println( "Successfully attached as dispose listener" );
            }

            try {
                Thread.currentThread().sleep( 500 );
            } catch( InterruptedException ex ) {
            }
        }
    }

    // ____________________

    // XEventListener (base of XSelectionChangeListener)
    public void disposing( EventObject aSourceObj ) {
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
    public void selectionChanged( EventObject aEvent ) {
        XController aCtrl = (XController) UnoRuntime.queryInterface( XController.class, aEvent.Source );
        if( aCtrl != null ) {
            XMultiComponentFactory mMCF = maContext.getServiceManager();

            MyMessageBox aMsgBox = new MyMessageBox(mMCF);

            aMsgBox.start();

            System.out.println("Listener finished");
        }
    }

    // __________ private __________

    private class MyMessageBox extends Thread{
        private XMultiComponentFactory mMCF;

        public MyMessageBox(XMultiComponentFactory xMCF){
            mMCF = xMCF;
        }

        public void run() {
            XDesktop aDesktop = null;
            XInterface aToolKit = null;
            try {
                Thread.sleep(1000);
            } catch (InterruptedException ex) {
                ex.printStackTrace();
            }
            try {
                Object oDesktop = mMCF.createInstanceWithContext("com.sun.star.frame.Desktop", maContext);
                Object oToolKit = mMCF.createInstanceWithContext("com.sun.star.awt.Toolkit", maContext);

                aDesktop = (XDesktop) UnoRuntime.queryInterface(XDesktop.class, oDesktop);
                aToolKit = (XInterface) UnoRuntime.queryInterface(XInterface.class, oToolKit);
            } catch (Exception ex) {
                ex.printStackTrace();
            }

            XWindow xWin = aDesktop.getCurrentFrame().getContainerWindow();
            XWindowPeer aWinPeer = (XWindowPeer) UnoRuntime.queryInterface(XWindowPeer.class, xWin);

            int button = com.sun.star.awt.MessageBoxButtons.BUTTONS_OK;
            XMessageBoxFactory aMBF = (XMessageBoxFactory) UnoRuntime.queryInterface(XMessageBoxFactory.class, aToolKit);
            XMessageBox xMB = aMBF.createMessageBox(aWinPeer, MessageBoxType.INFOBOX, button, "Event-Notify", "Listener was called, selcetion has changed");
            xMB.execute();
        }
    }

    private XChartDocument            maChartDocument;
    private XComponentContext         maContext;
}
