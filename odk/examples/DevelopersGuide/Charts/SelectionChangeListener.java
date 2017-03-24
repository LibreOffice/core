/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
        CellRangeAddress aRangeAddress = UnoRuntime.queryInterface(
            XCellRangeAddressable.class, aRange).getRangeAddress();

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



    public void run() {
        boolean bTrying = true;

        while( bTrying ) {
            // start listening for selection changes
            XSelectionSupplier aSelSupp = UnoRuntime.queryInterface(
                XSelectionSupplier.class,
                (UnoRuntime.queryInterface(
                XModel.class, maChartDocument ).getCurrentController()) );
            if( aSelSupp != null ) {
                aSelSupp.addSelectionChangeListener( this );
                System.out.println( "Successfully attached as selection change listener" );
                bTrying = false;
            }

            // start listening for death of Controller
            XComponent aComp = UnoRuntime.queryInterface( XComponent.class, aSelSupp );
            if( aComp != null ) {
                aComp.addEventListener( this );
                System.out.println( "Successfully attached as dispose listener" );
            }

            try {
                Thread.sleep( 500 );
            } catch( InterruptedException ex ) {
            }
        }
    }



    // XEventListener (base of XSelectionChangeListener)
    public void disposing( EventObject aSourceObj ) {
        System.out.println( "disposing called.  detaching as listener" );

        // stop listening for selection changes
        XSelectionSupplier aCtrl = UnoRuntime.queryInterface(
            XSelectionSupplier.class, aSourceObj );
        if( aCtrl != null )
            aCtrl.removeSelectionChangeListener( this );

        // remove as dispose listener
        XComponent aComp = UnoRuntime.queryInterface( XComponent.class, aSourceObj );
        if( aComp != null )
            aComp.removeEventListener( this );

        // bail out
        System.exit( 0 );
    }



    // XSelectionChangeListener
    public void selectionChanged( EventObject aEvent ) {
        XController aCtrl = UnoRuntime.queryInterface( XController.class, aEvent.Source );
        if( aCtrl != null ) {
            XMultiComponentFactory mMCF = maContext.getServiceManager();

            MyMessageBox aMsgBox = new MyMessageBox(mMCF);

            aMsgBox.start();

            System.out.println("Listener finished");
        }
    }

    // __________ private __________

    private class MyMessageBox extends Thread{
        private final XMultiComponentFactory mMCF;

        public MyMessageBox(XMultiComponentFactory xMCF){
            mMCF = xMCF;
        }

        @Override
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

                aDesktop = UnoRuntime.queryInterface(XDesktop.class, oDesktop);
                aToolKit = UnoRuntime.queryInterface(XInterface.class, oToolKit);
            } catch (Exception ex) {
                ex.printStackTrace();
            }

            XWindow xWin = aDesktop.getCurrentFrame().getContainerWindow();
            XWindowPeer aWinPeer = UnoRuntime.queryInterface(XWindowPeer.class, xWin);

            int button = com.sun.star.awt.MessageBoxButtons.BUTTONS_OK;
            XMessageBoxFactory aMBF = UnoRuntime.queryInterface(XMessageBoxFactory.class, aToolKit);
            XMessageBox xMB = aMBF.createMessageBox(aWinPeer, MessageBoxType.INFOBOX, button, "Event-Notify", "Listener was called, selection has changed");
            xMB.execute();
        }
    }

    private final XChartDocument            maChartDocument;
    private final XComponentContext         maContext;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
