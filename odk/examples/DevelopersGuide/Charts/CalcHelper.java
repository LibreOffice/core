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

// __________ Imports __________

import java.util.Random;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XMultiServiceFactory;
// container access
import com.sun.star.container.*;

// application specific classes
import com.sun.star.sheet.*;
import com.sun.star.table.*;
import com.sun.star.chart.*;
import com.sun.star.text.XText;

import com.sun.star.document.XEmbeddedObjectSupplier;
import com.sun.star.frame.XModel;
// base graphics things
import com.sun.star.awt.Point;
import com.sun.star.awt.Size;
import com.sun.star.awt.Rectangle;

// Exceptions
import com.sun.star.uno.RuntimeException;
import com.sun.star.container.NoSuchElementException;

// __________ Implementation __________

// Helper for accessing a calc document

public class CalcHelper
{
    public CalcHelper( XSpreadsheetDocument aDoc )
    {
        maSpreadSheetDoc = aDoc;
        initSpreadSheet();
    }



    public XSpreadsheet getChartSheet() throws RuntimeException
    {
        XNameAccess aSheetsNA = UnoRuntime.queryInterface(
            XNameAccess.class, maSpreadSheetDoc.getSheets() );

        XSpreadsheet aSheet = null;
        try
        {
            aSheet = UnoRuntime.queryInterface(
                XSpreadsheet.class, aSheetsNA.getByName( msChartSheetName ) );
        }
        catch( NoSuchElementException ex )
        {
            System.out.println( "Couldn't find sheet with name " + msChartSheetName + ": " + ex );
        }
        catch( Exception ex )
        {}

        return aSheet;
    }



    public XSpreadsheet getDataSheet() throws RuntimeException
    {
        XNameAccess aSheetsNA = UnoRuntime.queryInterface(
            XNameAccess.class, maSpreadSheetDoc.getSheets() );

        XSpreadsheet aSheet = null;
        if( aSheetsNA != null )
        {
            try
            {
                aSheet = UnoRuntime.queryInterface(
                    XSpreadsheet.class, aSheetsNA.getByName( msDataSheetName ) );
            }
            catch( NoSuchElementException ex )
            {
                System.out.println( "Couldn't find sheet with name " + msDataSheetName + ": " + ex );
            }
            catch( Exception ex )
            {}
        }

        return aSheet;
    }



    /** Insert a chart using the given name as name of the OLE object and the range as corresponding
        range of data to be used for rendering.  The chart is placed in the sheet for charts at
        position aUpperLeft extending as large as given in aExtent.

        The service name must be the name of a diagram service that can be instantiated via the
        factory of the chart document
     */
    public XChartDocument insertChart(
        String               sChartName,
        CellRangeAddress     aRange,
        Point                aUpperLeft,
        Size                 aExtent,
        String               sChartServiceName )
    {
        XChartDocument aResult = null;
        XTableChartsSupplier aSheet;

        // get the sheet to insert the chart
        try
        {
            aSheet = UnoRuntime.queryInterface(
                XTableChartsSupplier.class, getChartSheet() );
        }
        catch( Exception ex )
        {
            System.out.println( "Sheet not found" + ex );
            return aResult;
        }

        XTableCharts aChartCollection = aSheet.getCharts();
        XNameAccess  aChartCollectionNA = UnoRuntime.queryInterface(
            XNameAccess.class, aChartCollection );

        if( aChartCollectionNA != null &&
            ! aChartCollectionNA.hasByName( sChartName ) )
        {
            Rectangle aRect = new Rectangle( aUpperLeft.X, aUpperLeft.Y, aExtent.Width, aExtent.Height );

            CellRangeAddress[] aAddresses = new CellRangeAddress[ 1 ];
            aAddresses[ 0 ] = aRange;

            // first bool: ColumnHeaders
            // second bool: RowHeaders
            aChartCollection.addNewByName( sChartName, aRect, aAddresses, true, false );

            try
            {
                XTableChart aTableChart = UnoRuntime.queryInterface(
                    XTableChart.class, aChartCollectionNA.getByName( sChartName ));

                // the table chart is an embedded object which contains the chart document
                aResult = UnoRuntime.queryInterface(
                    XChartDocument.class,
                    UnoRuntime.queryInterface(
                        XEmbeddedObjectSupplier.class,
                        aTableChart ).getEmbeddedObject());

                // create a diagram via the factory and set this as new diagram
                aResult.setDiagram(
                    UnoRuntime.queryInterface(
                        XDiagram.class,
                        UnoRuntime.queryInterface(
                            XMultiServiceFactory.class,
                            aResult ).createInstance( sChartServiceName )));
            }
            catch( NoSuchElementException ex )
            {
                System.out.println( "Couldn't find chart with name " + sChartName + ": " + ex );
            }
            catch( Exception ex )
            {}
        }

        return aResult;
    }



    /** Fill a rectangular range with random numbers.
        The first column has increasing values
     */
    public XCellRange insertRandomRange( int nColumnCount, int nRowCount )
    {
        XCellRange aRange = null;

        // get the sheet to insert the chart
        try
        {
            XSpreadsheet aSheet = getDataSheet();
            XCellRange aSheetRange = UnoRuntime.queryInterface( XCellRange.class, aSheet );

            aRange = aSheetRange.getCellRangeByPosition(
                0, 0,
                nColumnCount - 1, nRowCount - 1 );

            int nCol, nRow;
            double fBase  = 0.0;
            double fRange = 10.0;
            double fValue;
            Random aGenerator = new Random();


            for( nCol = 0; nCol < nColumnCount; nCol++ )
            {
                if( 0 == nCol )
                {
                    (aSheet.getCellByPosition( nCol, 0 )).setFormula( "X" );
                }
                else
                {
                    (aSheet.getCellByPosition( nCol, 0 )).setFormula( "Random " + nCol );
                }

                for( nRow = 1; nRow < nRowCount; nRow++ )
                {
                    if( 0 == nCol )
                    {
                        // x values: ascending numbers
                        fValue = nRow + aGenerator.nextDouble();
                    }
                    else
                    {
                        fValue = fBase + ( aGenerator.nextGaussian() * fRange );
                    }

                    // put value into cell

                    // note: getCellByPosition is a method at ...table.XCellRange which
                    //       the XSpreadsheet inherits via ...sheet.XSheetCellRange
                    (aSheet.getCellByPosition( nCol, nRow )).setValue( fValue );
                }
            }

        }
        catch( Exception ex )
        {
            System.out.println( "Sheet not found" + ex );
        }

        return aRange;
    }



    public XCellRange insertFormulaRange( int nColumnCount, int nRowCount )
    {
        XCellRange aRange = null;

        // get the sheet to insert the chart
        try
        {
            XSpreadsheet aSheet = getDataSheet();
            XCellRange aSheetRange = UnoRuntime.queryInterface( XCellRange.class, aSheet );

            aRange = aSheetRange.getCellRangeByPosition(
                0, 0,
                nColumnCount - 1, nRowCount - 1 );

            int nCol, nRow;
            double fValue;
            double fFactor = 2.0 * Math.PI / (nRowCount - 1);
            String aFormula;

            // set variable factor for cos formula
            int nFactorCol = nColumnCount + 2;
            (aSheet.getCellByPosition( nFactorCol - 1, 0 )).setValue( 0.2 );

            XText xCellText = UnoRuntime.queryInterface( XText.class, aSheet.getCellByPosition( nFactorCol - 1, 1 ) );
            xCellText.setString( "Change the factor above and\nwatch the changes in the chart" );

            for( nCol = 0; nCol < nColumnCount; nCol++ )
            {
                for( nRow = 0; nRow < nRowCount; nRow++ )
                {
                    if( 0 == nCol )
                    {
                        // x values: ascending numbers
                        fValue = nRow * fFactor;
                        (aSheet.getCellByPosition( nCol, nRow )).setValue( fValue );
                    }
                    else
                    {
                        aFormula = "=";
                        if( nCol % 2 == 0 )
                            aFormula += "SIN";
                        else
                            aFormula += "COS";
                        aFormula += "(INDIRECT(ADDRESS(" + (nRow + 1) + ";1)))+RAND()*INDIRECT(ADDRESS(1;" + nFactorCol + "))";
                        (aSheet.getCellByPosition( nCol, nRow )).setFormula( aFormula );
                    }
                }
            }

        }
        catch( Exception ex )
        {
            System.out.println( "Sheet not found" + ex );
        }

        return aRange;
    }



    /** Bring the sheet containing charts visually to the foreground
     */
    public void raiseChartSheet()
    {
        UnoRuntime.queryInterface(
            XSpreadsheetView.class,
            UnoRuntime.queryInterface(
                XModel.class,
                maSpreadSheetDoc ).getCurrentController()).setActiveSheet( getChartSheet() );
    }


    // __________ private members and methods __________

    private static final String  msDataSheetName  = "Data";
    private static final String  msChartSheetName = "Chart";

    private final XSpreadsheetDocument   maSpreadSheetDoc;




    /** create two sheets, one for data and one for charts in the document
     */
    private void initSpreadSheet()
    {
        if( maSpreadSheetDoc != null )
        {
            XSpreadsheets  aSheets    = maSpreadSheetDoc.getSheets();
            XNameContainer aSheetsNC  = UnoRuntime.queryInterface(
                XNameContainer.class, aSheets );
            XIndexAccess   aSheetsIA  = UnoRuntime.queryInterface(
                XIndexAccess.class, aSheets );

            if( aSheets   != null &&
                aSheetsNC != null &&
                aSheetsIA != null )
            {
                try
                {
                    // remove all sheets except one
                    for( int i = aSheetsIA.getCount() - 1; i > 0; i-- )
                    {
                        aSheetsNC.removeByName(
                            UnoRuntime.queryInterface(
                                XNamed.class, aSheetsIA.getByIndex( i ) ).getName() );
                    }

                    XNamed aFirstSheet = UnoRuntime.queryInterface(
                        XNamed.class,
                        aSheetsIA.getByIndex( 0 ));

                    // first sheet becomes data sheet
                    aFirstSheet.setName( msDataSheetName );

                    // second sheet becomes chart sheet
                    aSheets.insertNewByName( msChartSheetName, (short)1 );
                }
                catch( Exception ex )
                {
                    System.out.println( "Couldn't initialize Spreadsheet Document: " + ex );
                }
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
