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

import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.*;

// property access
import com.sun.star.beans.*;

// application specific classes
import com.sun.star.chart.*;
import com.sun.star.drawing.*;

import com.sun.star.table.CellRangeAddress;
import com.sun.star.table.XCellRange;
import com.sun.star.sheet.XCellRangeAddressable;

import com.sun.star.frame.XModel;
import com.sun.star.util.XNumberFormatsSupplier;
import com.sun.star.util.XNumberFormats;

// base graphics things
import com.sun.star.awt.Point;
import com.sun.star.awt.Size;
import com.sun.star.awt.FontWeight;
// Exceptions
import com.sun.star.uno.Exception;
import com.sun.star.uno.RuntimeException;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.lang.IndexOutOfBoundsException;
import com.sun.star.util.MalformedNumberFormatException;


// __________ Implementation __________

// Create a spreadsheet add some data and add a chart

public class ChartInCalc
{


    public static void main( String args[] )
    {
        Helper aHelper = new Helper( args );

        CalcHelper aCalcHelper = new CalcHelper( aHelper.createSpreadsheetDocument() );

        // insert a cell range with 4 columns and 24 rows filled with random numbers
        XCellRange aRange = aCalcHelper.insertRandomRange( 4, 24 );
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
        XChartDocument aChartDoc = aCalcHelper.insertChart(
            "ScatterChart",
            aRangeAddress,
            aPos,
            aExtent,
            "com.sun.star.chart.XYDiagram" );

        // instantiate test class with newly created chart
        ChartInCalc aTest   = new ChartInCalc( aChartDoc );

        try
        {
            aTest.lockControllers();

            aTest.testDiagram();
            aTest.testArea();
            aTest.testWall();
            aTest.testTitle();
            aTest.testAxis();
            aTest.testGrid();

            // show an intermediate state, ...
            aTest.unlockControllers();
            aTest.lockControllers();

            // ..., because the following takes a while:
            // an internet URL has to be resolved
            aTest.testDataRowProperties();
            aTest.testDataPointProperties();

            aTest.unlockControllers();
        }
        catch( Exception ex )
        {
            System.out.println( "UNO Exception caught: " + ex );
            System.out.println( "Message: " + ex.getMessage() );
        }

        System.exit( 0 );
    }




    public ChartInCalc( XChartDocument aChartDoc )
    {
        maChartDocument = aChartDoc;
        maDiagram       = maChartDocument.getDiagram();
    }



    public void lockControllers()
        throws RuntimeException
    {
        UnoRuntime.queryInterface( XModel.class, maChartDocument ).lockControllers();
    }



    public void unlockControllers()
        throws RuntimeException
    {
        UnoRuntime.queryInterface( XModel.class, maChartDocument ).unlockControllers();
    }



    public void testDiagram()
        throws RuntimeException, UnknownPropertyException, PropertyVetoException,
               com.sun.star.lang.IllegalArgumentException, WrappedTargetException
    {
        XPropertySet aDiaProp = UnoRuntime.queryInterface( XPropertySet.class, maDiagram );

        if( aDiaProp != null )
        {
            // change chart type
            aDiaProp.setPropertyValue( "Lines", Boolean.TRUE);

            // change attributes for all series
            // set line width to 0.5mm
            aDiaProp.setPropertyValue( "LineWidth",  Integer.valueOf( 50 ));
        }
    }



    public void testDataRowProperties()
        throws RuntimeException, UnknownPropertyException, PropertyVetoException,
               com.sun.star.lang.IllegalArgumentException, WrappedTargetException
    {
        // change properties of the data series
        try
        {
            XPropertySet aSeriesProp;
            for( int i = 1; i <= 3; i++ )
            {
                aSeriesProp = maDiagram.getDataRowProperties( i );
                aSeriesProp.setPropertyValue( "LineColor", Integer.valueOf(
                                                  0x400000 * i +
                                                  0x005000 * i +
                                                  0x0000ff - 0x40 * i ));
                if( 1 == i )
                {
                    StringBuffer sUrl = new StringBuffer("file:///");
                    try {
                        /* for use without net it's easier to load a local graphic */
                        java.io.File sourceFile = new java.io.File("bullet.gif");
                        sUrl.append(sourceFile.getCanonicalPath().replace('\\', '/'));
                    } catch (java.io.IOException e) {
                        sUrl = new StringBuffer("http://graphics.openoffice.org/chart/bullet1.gif");
                    }

                    // set a bitmap via URL as symbol for the first series
                    aSeriesProp.setPropertyValue( "SymbolType", Integer.valueOf( ChartSymbolType.BITMAPURL ));
                    aSeriesProp.setPropertyValue( "SymbolBitmapURL", sUrl.toString() );
                }
                else
                {
                    aSeriesProp.setPropertyValue( "SymbolType", Integer.valueOf( ChartSymbolType.SYMBOL1 ));
                    aSeriesProp.setPropertyValue( "SymbolSize", new Size( 250, 250 ));
                }
            }
        }
        catch( IndexOutOfBoundsException ex )
        {
            System.out.println( "Oops, there not enough series for setting properties: " + ex );
        }
    }



    public void testDataPointProperties()
        throws RuntimeException, UnknownPropertyException, PropertyVetoException,
               com.sun.star.lang.IllegalArgumentException, WrappedTargetException
    {
        // set properties for a single data point
        try
        {
            // determine the maximum value of the first series

            XChartDataArray aDataArray = UnoRuntime.queryInterface(
                XChartDataArray.class, maChartDocument.getData());
            double aData[][] = aDataArray.getData();

            int i;
            double fMax = aData[ 0 ][ 1 ];
            for( i = 1; i < aData.length; i++ )
            {
                if( aData[ i ][ 1 ] > fMax )
                {
                    fMax = aData[ i ][ 1 ];
                }
            }

            // first parameter is the index of the point, the second one is the series
            XPropertySet aPointProp = maDiagram.getDataPointProperties( 0, 1 );

            // set a different, larger symbol
            aPointProp.setPropertyValue( "SymbolType", Integer.valueOf( ChartSymbolType.SYMBOL6 ));
            aPointProp.setPropertyValue( "SymbolSize", new Size( 600, 600 ));

            // add a label text with bold font, bordeaux red 14pt
            aPointProp.setPropertyValue( "DataCaption", Integer.valueOf( ChartDataCaption.VALUE ));
            aPointProp.setPropertyValue( "CharHeight",  new Float( 14.0 ));
            aPointProp.setPropertyValue( "CharColor",   Integer.valueOf( 0x993366 ));
            aPointProp.setPropertyValue( "CharWeight",  new Float( FontWeight.BOLD ));
        }
        catch( IndexOutOfBoundsException ex )
        {
            System.out.println( "Oops, there not enough data points or series for setting properties: " + ex );
        }
    }



    public void testArea()
        throws RuntimeException, UnknownPropertyException, PropertyVetoException,
               com.sun.star.lang.IllegalArgumentException, WrappedTargetException
    {
        XPropertySet   aArea = maChartDocument.getArea();

        if( aArea != null )
        {
            // change background color of entire chart
            aArea.setPropertyValue( "FillStyle", FillStyle.SOLID );
            aArea.setPropertyValue( "FillColor", Integer.valueOf( 0xeeeeee ));
        }
    }



    public void testWall()
        throws RuntimeException, UnknownPropertyException, PropertyVetoException,
               com.sun.star.lang.IllegalArgumentException, WrappedTargetException
    {
        XPropertySet aWall = UnoRuntime.queryInterface(
                                  X3DDisplay.class, maDiagram ).getWall();

        // change background color of area
        aWall.setPropertyValue( "FillStyle", FillStyle.SOLID );
        aWall.setPropertyValue( "FillColor", Integer.valueOf( 0xcccccc ));
    }



    public void testTitle()
        throws RuntimeException, UnknownPropertyException, PropertyVetoException,
               com.sun.star.lang.IllegalArgumentException, WrappedTargetException
    {
        // change main title
        XPropertySet aDocProp = UnoRuntime.queryInterface(
            XPropertySet.class, maChartDocument );
        aDocProp.setPropertyValue( "HasMainTitle", Boolean.TRUE);

        XShape aTitle = maChartDocument.getTitle();
        XPropertySet aTitleProp = UnoRuntime.queryInterface( XPropertySet.class, aTitle );

        // set new text
        if( aTitleProp != null )
        {
            aTitleProp.setPropertyValue( "String", "Random Scatter Chart" );
            aTitleProp.setPropertyValue( "CharHeight", new Float(14.0) );
        }

        // align title with y axis
        XShape aAxis = UnoRuntime.queryInterface(
            XShape.class, UnoRuntime.queryInterface(
                XAxisYSupplier.class, maDiagram ).getYAxis() );

        if( aAxis != null &&
            aTitle != null )
        {
            Point aPos = aTitle.getPosition();
            aPos.X = ( aAxis.getPosition() ).X;
            aTitle.setPosition( aPos );
        }
    }



    public void testAxis()
        throws RuntimeException, UnknownPropertyException, PropertyVetoException,
               com.sun.star.lang.IllegalArgumentException, WrappedTargetException,
               MalformedNumberFormatException
    {
        // x axis
        XPropertySet aAxisProp = UnoRuntime.queryInterface(
                                      XAxisXSupplier.class, maDiagram ).getXAxis();
        if( aAxisProp != null )
        {
            aAxisProp.setPropertyValue( "Max",      Integer.valueOf( 24 ));
            aAxisProp.setPropertyValue( "StepMain", Integer.valueOf( 3 ));
        }

        // change number format for y axis
        aAxisProp = UnoRuntime.queryInterface(
                         XAxisYSupplier.class, maDiagram ).getYAxis();

        // add a new custom number format and get the new key
        int nNewNumberFormat = 0;
        XNumberFormatsSupplier aNumFmtSupp = UnoRuntime.queryInterface(
            XNumberFormatsSupplier.class, maChartDocument );

        if( aNumFmtSupp != null )
        {
            XNumberFormats aFormats = aNumFmtSupp.getNumberFormats();
            Locale aLocale = new Locale( "de", "DE", "de" );

            String aFormatStr = aFormats.generateFormat( nNewNumberFormat, aLocale, true, true, (short)3, (short)1 );
            nNewNumberFormat = aFormats.addNew( aFormatStr, aLocale );
        }

        if( aAxisProp != null )
        {
            aAxisProp.setPropertyValue( "NumberFormat", Integer.valueOf( nNewNumberFormat ));
        }
    }



    public void testGrid()
        throws RuntimeException, UnknownPropertyException, PropertyVetoException,
               com.sun.star.lang.IllegalArgumentException, WrappedTargetException
    {
        // y major grid
        XPropertySet aGridProp = UnoRuntime.queryInterface(
            XPropertySet.class,
            UnoRuntime.queryInterface(
                XAxisYSupplier.class, maDiagram ).getYMainGrid());

        if( aGridProp != null )
        {
            LineDash aDash = new LineDash();
            aDash.Style    = DashStyle.ROUND;
            aDash.Dots     = 2;
            aDash.DotLen   = 10;
            aDash.Dashes   = 1;
            aDash.DashLen  = 200;
            aDash.Distance = 100;

            aGridProp.setPropertyValue( "LineColor", Integer.valueOf( 0x999999 ));
            aGridProp.setPropertyValue( "LineStyle", LineStyle.DASH );
            aGridProp.setPropertyValue( "LineDash", aDash );
            aGridProp.setPropertyValue( "LineWidth", Integer.valueOf( 30 ));
        }
    }




    // private members


    private final XChartDocument maChartDocument;
    private final XDiagram       maDiagram;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
