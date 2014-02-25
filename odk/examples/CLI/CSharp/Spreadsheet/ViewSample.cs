/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

using System;
using System.Threading;

// __________  implementation  ____________________________________

/** Create and modify a spreadsheet view.
 */
public class ViewSample : SpreadsheetDocHelper
{

    public static void Main( String [] args )
    {
        try
        {
            using ( ViewSample aSample = new ViewSample( args ) )
            {
                aSample.doSampleFunction();
            }
            Console.WriteLine( "\nSamples done." );
        }
        catch (Exception ex)
        {
            Console.WriteLine( "Sample caught exception! " + ex );
        }
    }



    public ViewSample( String[] args )
        : base( args )
    {
    }
    


    /** This sample function performs all changes on the view. */
    public void doSampleFunction()
    {
        unoidl.com.sun.star.sheet.XSpreadsheetDocument xDoc = getDocument();
        unoidl.com.sun.star.frame.XModel xModel =
            (unoidl.com.sun.star.frame.XModel) xDoc;
        unoidl.com.sun.star.frame.XController xController =
            xModel.getCurrentController();
 
        // --- Spreadsheet view ---
        // freeze the first column and first two rows
        unoidl.com.sun.star.sheet.XViewFreezable xFreeze =
            (unoidl.com.sun.star.sheet.XViewFreezable) xController;
        if ( null != xFreeze )
            Console.WriteLine( "got xFreeze" );
        xFreeze.freezeAtPosition( 1, 2 );

        // --- View pane ---
        // get the cell range shown in the second pane and assign
        // a cell background to them
        unoidl.com.sun.star.container.XIndexAccess xIndex =
            (unoidl.com.sun.star.container.XIndexAccess) xController;
        uno.Any aPane = xIndex.getByIndex(1);
        unoidl.com.sun.star.sheet.XCellRangeReferrer xRefer =
            (unoidl.com.sun.star.sheet.XCellRangeReferrer) aPane.Value;
        unoidl.com.sun.star.table.XCellRange xRange = xRefer.getReferredCells();
        unoidl.com.sun.star.beans.XPropertySet xRangeProp =
            (unoidl.com.sun.star.beans.XPropertySet) xRange;
        xRangeProp.setPropertyValue(
            "IsCellBackgroundTransparent", new uno.Any( false ) );
        xRangeProp.setPropertyValue(
            "CellBackColor", new uno.Any( (Int32) 0xFFFFCC ) );
 
        // --- View settings ---
        // change the view to display green grid lines
        unoidl.com.sun.star.beans.XPropertySet xProp =
            (unoidl.com.sun.star.beans.XPropertySet) xController;
        xProp.setPropertyValue(
            "ShowGrid", new uno.Any( true ) );
        xProp.setPropertyValue(
            "GridColor", new uno.Any( (Int32) 0x00CC00 ) );

        // --- Range selection ---
        // let the user select a range and use it as the view's selection
        unoidl.com.sun.star.sheet.XRangeSelection xRngSel =
            (unoidl.com.sun.star.sheet.XRangeSelection) xController;
        ExampleRangeListener aListener = new ExampleRangeListener();
        xRngSel.addRangeSelectionListener( aListener );
        unoidl.com.sun.star.beans.PropertyValue[] aArguments =
            new unoidl.com.sun.star.beans.PropertyValue[2];
        aArguments[0] = new unoidl.com.sun.star.beans.PropertyValue();
        aArguments[0].Name   = "Title";
        aArguments[0].Value  = new uno.Any( "Please select a range" );
        aArguments[1] = new unoidl.com.sun.star.beans.PropertyValue();
        aArguments[1].Name   = "CloseOnMouseRelease";
        aArguments[1].Value  = new uno.Any( true );
        xRngSel.startRangeSelection( aArguments );
        Monitor.Enter( aListener );
        try
        {
            Monitor.Wait( aListener );       // wait until the selection is done
        }
        finally
        {
            Monitor.Exit( aListener );
        }
        xRngSel.removeRangeSelectionListener( aListener );
        if ( aListener.aResult != null && aListener.aResult.Length != 0 )
        {
            unoidl.com.sun.star.view.XSelectionSupplier xSel =
                (unoidl.com.sun.star.view.XSelectionSupplier) xController;
            unoidl.com.sun.star.sheet.XSpreadsheetView xView =
                (unoidl.com.sun.star.sheet.XSpreadsheetView) xController;
            unoidl.com.sun.star.sheet.XSpreadsheet xSheet =
                xView.getActiveSheet();
            unoidl.com.sun.star.table.XCellRange xResultRange =
                xSheet.getCellRangeByName( aListener.aResult );
            xSel.select(
                new uno.Any(
                    typeof (unoidl.com.sun.star.table.XCellRange),
                    xResultRange ) );
        }
    }



    //  listener to react on finished selection

    private class ExampleRangeListener
        : unoidl.com.sun.star.sheet.XRangeSelectionListener
    {
        public String aResult;

        public void done( unoidl.com.sun.star.sheet.RangeSelectionEvent aEvent )
        {
            aResult = aEvent.RangeDescriptor;
            Monitor.Enter( this );
            try
            {
                Monitor.Pulse( this );
            }
            finally
            {
                Monitor.Exit( this );
            }
        }

        public void aborted(
            unoidl.com.sun.star.sheet.RangeSelectionEvent aEvent )
        {
            Monitor.Enter( this );
            try
            {
                Monitor.Pulse( this );
            }
            finally
            {
                Monitor.Exit( this );
            }
        }

        public void disposing( unoidl.com.sun.star.lang.EventObject aObj )
        {
        }
    }



}
