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

// package name: as default, start with complex
package qa;

import complexlib.ComplexTestCase;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.Type;
import java.io.PrintWriter;
import com.sun.star.lang.*;
import com.sun.star.beans.*;
import com.sun.star.frame.*;
import com.sun.star.chart.*;
import com.sun.star.drawing.*;
import com.sun.star.awt.*;
import com.sun.star.container.*;
import com.sun.star.util.XCloseable;
import com.sun.star.util.CloseVetoException;
import com.sun.star.uno.AnyConverter;
import util.utils;

/**
 * The following Complex Test will test the
 * com.sun.star.document.IndexedPropertyValues
 * service
 */

public class TestCaseOldAPI extends ComplexTestCase {

    // The name of the tested service
    private static final String testedServiceName =
          "com.sun.star.chart.ChartDocument";

    // The first of the mandatory functions:
    /**
     * Return the name of the test.
     * In this case it is the actual name of the service.
     * @return The tested service.
     */
    @Override
    public String getTestObjectName() {
        return testedServiceName;
    }

    // The second of the mandatory functions: return all test methods as an
    // array. There is only one test function in this example.
    /**
     * Return all test methods.
     * @return The test methods.
     */
    @Override
    public String[] getTestMethodNames() {
        // For some tests a view needs to be created. Accessing the model via
        // this program and the view may lead to problems
        boolean bAvoidViewCreation = false;

        if( bAvoidViewCreation )
            return new String[] {
                "testData",
                "testChartType",
                "testArea",
                "testAggregation",
                "testFactory",
                "testDataSeriesAndPoints",
                "testStatistics",
                "testStockProperties"
            };

        return new String[] {
            "testData",
            "testChartType",
            "testTitle",
            "testSubTitle",
            "testDiagram",
            "testAxis",
            "testLegend",
            "testArea",
            "testAggregation",
            "testFactory",
            "testDataSeriesAndPoints",
            "testStatistics",
            "testStockProperties"
        };
    }



    public void before()
    {
        // set to "true" to get a view
        mbCreateView = true;

        if( mbCreateView )
            mxChartModel = createDocument( "schart" );
        else
            mxChartModel = createChartModel();

        mxOldDoc = UnoRuntime.queryInterface(
            XChartDocument.class, mxChartModel );
    }



    public void after()
    {
        XCloseable xCloseable = UnoRuntime.queryInterface(
            XCloseable.class, mxChartModel );
        assure( "document is no XCloseable", xCloseable != null );

        // do not close document if there exists a view
        if( ! mbCreateView )
        {
            try
            {
                xCloseable.close( true );
            }
            catch( CloseVetoException ex )
            {
                failed( ex.getMessage() );
                ex.printStackTrace( (PrintWriter)log );
            }
        }
    }



    public void testTitle()
    {
        try
        {
            XPropertySet xDocProp = UnoRuntime.queryInterface(
                XPropertySet.class, mxOldDoc );
            assure( "Chart Document is no XPropertySet", xDocProp != null );
            xDocProp.setPropertyValue( "HasMainTitle", Boolean.TRUE);
            assure( "Property HasMainTitle", AnyConverter.toBoolean(
                        xDocProp.getPropertyValue( "HasMainTitle" )));

            XShape xTitleShape = mxOldDoc.getTitle();
            XPropertySet xTitleProp = UnoRuntime.queryInterface(
                XPropertySet.class, xTitleShape );

            // set property via old API
            if( xTitleProp != null )
            {
                String aTitle = " Overwritten by Old API ";
                float fHeight = (float)17.0;

                xTitleProp.setPropertyValue( "String", aTitle );
                xTitleProp.setPropertyValue( "CharHeight", new Float( fHeight ) );

                float fNewHeight = AnyConverter.toFloat( xTitleProp.getPropertyValue( "CharHeight" ) );
                assure( "Changing CharHeight via old API failed", fNewHeight == fHeight );

                String aNewTitle = AnyConverter.toString( xTitleProp.getPropertyValue( "String" ) );
                assure( "Property \"String\" failed", aNewTitle.equals( aTitle ));
            }

            // move title
            Point aSetPos = new Point();
            aSetPos.X = 1000;
            aSetPos.Y = 200;
            xTitleShape.setPosition( aSetPos );

            Point aNewPos = xTitleShape.getPosition();
            assure( "Title Position X", approxEqual( aNewPos.X, aSetPos.X, 1 ));
            assure( "Title Position Y", approxEqual( aNewPos.Y, aSetPos.Y, 1 ));
        }
        catch( Exception ex )
        {
            failed( ex.getMessage() );
            ex.printStackTrace( (PrintWriter)log );
        }
    }



    public void testSubTitle()
    {
        try
        {
            XPropertySet xDocProp = UnoRuntime.queryInterface(
                XPropertySet.class, mxOldDoc );
            assure( "Chart Document is no XPropertySet", xDocProp != null );
            xDocProp.setPropertyValue( "HasSubTitle", Boolean.TRUE);
            assure( "Property HasSubTitle", AnyConverter.toBoolean(
                        xDocProp.getPropertyValue( "HasSubTitle" )));

            XShape xTitleShape = mxOldDoc.getSubTitle();
            XPropertySet xTitleProp = UnoRuntime.queryInterface(
                XPropertySet.class, xTitleShape );

            // set Property via old API
            if( xTitleProp != null )
            {
                int nColor = 0x009acd; // DeepSkyBlue3
                float fWeight = FontWeight.BOLD;
                float fHeight = (float)14.0;

                xTitleProp.setPropertyValue( "CharColor", Integer.valueOf( nColor ) );
                xTitleProp.setPropertyValue( "CharWeight", new Float( fWeight ));
                xTitleProp.setPropertyValue( "CharHeight", new Float( fHeight ) );

                int nNewColor = AnyConverter.toInt( xTitleProp.getPropertyValue( "CharColor" ) );
                assure( "Changing CharColor via old API failed", nNewColor == nColor );

                float fNewWeight = AnyConverter.toFloat( xTitleProp.getPropertyValue( "CharWeight" ) );
                assure( "Changing CharWeight via old API failed", fNewWeight == fWeight );

                float fNewHeight = AnyConverter.toFloat( xTitleProp.getPropertyValue( "CharHeight" ) );
                assure( "Changing CharHeight via old API failed", fNewHeight == fHeight );
            }
        }
        catch( Exception ex )
        {
            failed( ex.getMessage() );
            ex.printStackTrace( (PrintWriter)log );
        }
    }



    public void testDiagram()
    {
        try
        {
            // testing wall
            XDiagram xDia = mxOldDoc.getDiagram();
            if( xDia != null )
            {
                X3DDisplay xDisp = UnoRuntime.queryInterface(
                    X3DDisplay.class, xDia );
                assure( "X3DDisplay not supported", xDisp != null );

                // Wall
                XPropertySet xProp = xDisp.getWall();
                if( xProp != null )
                {
                    int nColor = 0xffe1ff; // thistle1
                    xProp.setPropertyValue( "FillColor", Integer.valueOf( nColor ) );
                    int nNewColor = AnyConverter.toInt( xProp.getPropertyValue( "FillColor" ) );
                    assure( "Changing FillColor via old API failed", nNewColor == nColor );
                }

                assure( "Wrong Diagram Type", xDia.getDiagramType().equals(
                            "com.sun.star.chart.BarDiagram" ));

                // Diagram properties
                xProp = UnoRuntime.queryInterface( XPropertySet.class, xDia );
                assure( "Diagram is no property set", xProp != null );

                // y-axis
                boolean bFirstYAxisText = false;
                xProp.setPropertyValue( "HasYAxisDescription", Boolean.valueOf( bFirstYAxisText ));
                boolean bNewFirstYAxisText = AnyConverter.toBoolean(
                    xProp.getPropertyValue( "HasYAxisDescription" ));
                assure( "Removing description of first y-axis", bNewFirstYAxisText == bFirstYAxisText );

                // second y-axis
                boolean bSecondaryYAxis = true;
                xProp.setPropertyValue( "HasSecondaryYAxis", Boolean.valueOf( bSecondaryYAxis ));
                boolean bNewSecYAxisValue = AnyConverter.toBoolean(
                    xProp.getPropertyValue( "HasSecondaryYAxis" ));
                assure( "Adding a second y-axis does not work", bNewSecYAxisValue == bSecondaryYAxis );

                XTwoAxisYSupplier xSecYAxisSuppl = UnoRuntime.queryInterface(
                    XTwoAxisYSupplier.class, xDia );
                assure( "XTwoAxisYSupplier not implemented", xSecYAxisSuppl != null );
                assure( "No second y-axis found", xSecYAxisSuppl.getSecondaryYAxis() != null );
            }

            // move diagram
            {
                XShape xDiagramShape = UnoRuntime.queryInterface(
                    XShape.class, xDia );

                Point aOldPos = xDiagramShape.getPosition();
                int xDiff = 20;
                int yDiff = 20;
                Point aSetPos = new Point();
                aSetPos.X = aOldPos.X + xDiff;
                aSetPos.Y = aOldPos.Y + yDiff;
                xDiagramShape.setPosition( aSetPos );

                Point aNewPos = xDiagramShape.getPosition();
                assure( "Diagram Position X", approxEqual( aNewPos.X, aSetPos.X, 1 ));
                assure( "Diagram Position Y", approxEqual( aNewPos.Y, aSetPos.Y, 1 ));
            }

            // size diagram
            {
                XShape xDiagramShape = UnoRuntime.queryInterface(
                    XShape.class, xDia );

                Size aOldSize = xDiagramShape.getSize();
                int xDiff = aOldSize.Width/2+2;
                int yDiff = aOldSize.Height/2+2;
                Size aSetSize = new Size();
                aSetSize.Width = aOldSize.Width - xDiff;
                aSetSize.Height = aOldSize.Height - yDiff;
                xDiagramShape.setSize( aSetSize );

                Size aNewSize = xDiagramShape.getSize();
                assure( "Diagram Width", approxEqual( aNewSize.Width, aSetSize.Width, 2 ));
                assure( "Diagram Height", approxEqual( aNewSize.Height, aSetSize.Height, 2 ));
            }
        }
        catch( Exception ex )
        {
            failed( ex.getMessage() );
            ex.printStackTrace( (PrintWriter)log );
        }
    }



    public void testAxis()
    {
        try
        {
            XAxisYSupplier xYAxisSuppl = UnoRuntime.queryInterface(
                XAxisYSupplier.class, mxOldDoc.getDiagram() );
            assure( "Diagram is no y-axis supplier", xYAxisSuppl != null );

            XPropertySet xProp = xYAxisSuppl.getYAxis();
            assure( "No y-axis found", xProp != null );

            double fMax1, fMax2;
            Object oMax = xProp.getPropertyValue( "Max" );
            assure( "No Maximum set", AnyConverter.isDouble( oMax ));
            fMax1 = AnyConverter.toDouble( oMax );
            log.println( "Maximum retrieved: " + fMax1 );
            //todo: the view has to be built before there is an explicit value
            xProp.setPropertyValue( "AutoMax", Boolean.FALSE);
            oMax = xProp.getPropertyValue( "Max" );
            assure( "No Maximum set", AnyConverter.isDouble( oMax ));
            fMax2 = AnyConverter.toDouble( oMax );
            log.println( "Maximum with AutoMax off: " + fMax2 );
            assure( "maxima differ", fMax1 == fMax2 );

            double nNewMax = 12.3;
            double nNewOrigin = 2.7;

            xProp.setPropertyValue( "Max", new Double( nNewMax ));
            assure( "AutoMax is on", ! AnyConverter.toBoolean( xProp.getPropertyValue( "AutoMax" )) );

            assure( "Maximum value invalid",
                    utils.approxEqual(
                        AnyConverter.toDouble( xProp.getPropertyValue( "Max" )),
                        nNewMax ));

            xProp.setPropertyValue( "AutoMin", Boolean.TRUE);
            assure( "AutoMin is off", AnyConverter.toBoolean( xProp.getPropertyValue( "AutoMin" )) );

            xProp.setPropertyValue( "Origin", new Double( nNewOrigin ));
            assure( "Origin invalid",
                    utils.approxEqual(
                        AnyConverter.toDouble( xProp.getPropertyValue( "Origin" )),
                        nNewOrigin ));
            xProp.setPropertyValue( "AutoOrigin", Boolean.TRUE);
            assure( "AutoOrigin is off", AnyConverter.toBoolean( xProp.getPropertyValue( "AutoOrigin" )) );
            Object oOrigin = xProp.getPropertyValue( "Origin" );
            assure( "No Origin set", AnyConverter.isDouble( oOrigin ));
            log.println( "Origin retrieved: " + AnyConverter.toDouble( oOrigin ));

            xProp.setPropertyValue( "Logarithmic", Boolean.TRUE);
            assure( "Scaling is not logarithmic",
                    AnyConverter.toBoolean( xProp.getPropertyValue( "Logarithmic" )) );
            xProp.setPropertyValue( "Logarithmic", Boolean.FALSE);
            assure( "Scaling is not logarithmic",
                    ! AnyConverter.toBoolean( xProp.getPropertyValue( "Logarithmic" )) );

            int nNewColor =  0xcd853f; // peru
            xProp.setPropertyValue( "LineColor", Integer.valueOf( nNewColor ));
            assure( "Property LineColor",
                    AnyConverter.toInt( xProp.getPropertyValue( "LineColor" )) == nNewColor );
            float fNewCharHeight = (float)(16.0);
            xProp.setPropertyValue( "CharHeight", new Float( fNewCharHeight ));
            assure( "Property CharHeight",
                    AnyConverter.toFloat( xProp.getPropertyValue( "CharHeight" )) == fNewCharHeight );

            int nNewTextRotation = 700; // in 1/100 degrees
            xProp.setPropertyValue( "TextRotation", Integer.valueOf( nNewTextRotation ));
            assure( "Property TextRotation",
                    AnyConverter.toInt( xProp.getPropertyValue( "TextRotation" )) == nNewTextRotation );

            double fStepMain = 10.0;
            xProp.setPropertyValue( "StepMain", new Double( fStepMain ));
            assure( "Property StepMain",
                    AnyConverter.toDouble( xProp.getPropertyValue( "StepMain" )) == fStepMain );

            // note: fStepHelp must be a divider of fStepMain, because
            // internally, the help-step is stored as an integer number of
            // substeps
            double fStepHelp = 5.0;
            xProp.setPropertyValue( "StepHelp", new Double( fStepHelp ));
            assure( "Property StepHelp",
                    AnyConverter.toDouble( xProp.getPropertyValue( "StepHelp" )) == fStepHelp );

            xProp.setPropertyValue( "DisplayLabels", Boolean.FALSE);
            assure( "Property DisplayLabels", ! AnyConverter.toBoolean(
                        xProp.getPropertyValue( "DisplayLabels" )));
        }
        catch( Exception ex )
        {
            failed( ex.getMessage() );
            ex.printStackTrace( (PrintWriter)log );
        }
    }



    public void testLegend()
    {
        XShape xLegend = mxOldDoc.getLegend();
        assure( "No Legend returned", xLegend != null );

        XPropertySet xLegendProp = UnoRuntime.queryInterface(
            XPropertySet.class, xLegend );
        assure( "Legend is no property set", xLegendProp != null );

        try
        {
            ChartLegendPosition eNewPos = ChartLegendPosition.BOTTOM;
            xLegendProp.setPropertyValue( "Alignment", eNewPos );
            assure( "Property Alignment",
                    AnyConverter.toObject(
                        new Type( ChartLegendPosition.class ),
                        xLegendProp.getPropertyValue( "Alignment" )) == eNewPos );

            float fNewCharHeight = (float)(11.0);
            xLegendProp.setPropertyValue( "CharHeight", new Float( fNewCharHeight ));
            assure( "Property CharHeight",
                    AnyConverter.toFloat( xLegendProp.getPropertyValue( "CharHeight" )) == fNewCharHeight );

            // move legend
            {
                Point aOldPos = xLegend.getPosition();
                int xDiff = 20;
                int yDiff = 20;
                Point aSetPos = new Point();
                aSetPos.X = aOldPos.X + xDiff;
                aSetPos.Y = aOldPos.Y + yDiff;
                xLegend.setPosition( aSetPos );

                Point aNewPos = xLegend.getPosition();
                assure( "Legend Position X", approxEqual( aNewPos.X, aSetPos.X, 1 ));
                assure( "Legend Position Y", approxEqual( aNewPos.Y, aSetPos.Y, 1 ));
            }
        }
        catch( Exception ex )
        {
            failed( ex.getMessage() );
            ex.printStackTrace( (PrintWriter)log );
        }
    }



    public void testArea()
    {
        XPropertySet xArea = mxOldDoc.getArea();
        assure( "No Area", xArea != null );

        try
        {
            int nColor = 0xf5fffa; // mint cream
            xArea.setPropertyValue( "FillColor", Integer.valueOf( nColor ) );
            xArea.setPropertyValue( "FillStyle", FillStyle.SOLID );

            int nNewColor = AnyConverter.toInt( xArea.getPropertyValue( "FillColor" ) );
            assure( "Changing FillColor of Area failed", nNewColor == nColor );
        }
        catch( Exception ex )
        {
            failed( ex.getMessage() );
            ex.printStackTrace( (PrintWriter)log );
        }
    }



    public void testChartType()
    {
        XMultiServiceFactory xFact = UnoRuntime.queryInterface(
            XMultiServiceFactory.class, mxOldDoc );
        assure( "document is no factory", xFact != null );

        try
        {
            String aMyServiceName = "com.sun.star.chart.BarDiagram";
            String aServices[] = xFact.getAvailableServiceNames();
            boolean bServiceFound = false;
            for( int i = 0; i < aServices.length; ++i )
            {
                if( aServices[ i ].equals( aMyServiceName ))
                {
                    bServiceFound = true;
                    break;
                }
            }
            assure( "getAvailableServiceNames did not return " + aMyServiceName, bServiceFound );

            if( bServiceFound )
            {
                XDiagram xDia = UnoRuntime.queryInterface(
                    XDiagram.class, xFact.createInstance( aMyServiceName ));
                assure( aMyServiceName + " could not be created", xDia != null );

                mxOldDoc.setDiagram( xDia );

                XPropertySet xDiaProp = UnoRuntime.queryInterface(
                    XPropertySet.class, xDia );
                assure( "Diagram is no XPropertySet", xDiaProp != null );

                xDiaProp.setPropertyValue( "Stacked", Boolean.TRUE);
                assure( "StackMode could not be set correctly",
                        AnyConverter.toBoolean(
                            xDiaProp.getPropertyValue( "Stacked" )));

                xDiaProp.setPropertyValue( "Dim3D", Boolean.FALSE);
                assure( "Dim3D could not be set correctly",
                        ! AnyConverter.toBoolean(
                            xDiaProp.getPropertyValue( "Dim3D" )));

                xDiaProp.setPropertyValue( "Vertical", Boolean.TRUE);
                assure( "Vertical could not be set correctly",
                        AnyConverter.toBoolean(
                            xDiaProp.getPropertyValue( "Vertical" )));
            }

        }
        catch( Exception ex )
        {
            failed( ex.getMessage() );
            ex.printStackTrace( (PrintWriter)log );
        }
    }



    public void testAggregation()
    {
        // query to new type
        XChartDocument xDiaProv = UnoRuntime.queryInterface(
            XChartDocument.class, mxOldDoc );
        assure( "query to new interface failed", xDiaProv != null );

        com.sun.star.chart.XChartDocument xDoc = UnoRuntime.queryInterface(
            com.sun.star.chart.XChartDocument.class, xDiaProv );
        assure( "querying back to old interface failed", xDoc != null );
    }



    public void testDataSeriesAndPoints()
    {
        try
        {
            XDiagram xDia = mxOldDoc.getDiagram();
            assure( "Invalid Diagram", xDia != null );
            XMultiServiceFactory xFact = UnoRuntime.queryInterface(
                XMultiServiceFactory.class, mxOldDoc );
            assure( "document is no factory", xFact != null );

            // FillColor
            XPropertySet xProp = xDia.getDataRowProperties( 0 );
            int nColor = 0xffd700; // gold
            xProp.setPropertyValue( "FillColor", Integer.valueOf( nColor ));
            int nNewColor = AnyConverter.toInt( xProp.getPropertyValue( "FillColor" ) );
            assure( "Changing FillColor of Data Series failed", nNewColor == nColor );

            // Gradient

            // note: the FillGradient property is optional, however it was
            // supported in the old chart's API
            XNameContainer xGradientTable = UnoRuntime.queryInterface(
                XNameContainer.class,
                xFact.createInstance( "com.sun.star.drawing.GradientTable" ));
            assure( "no gradient table", xGradientTable != null );
            String aGradientName = "NewAPITestGradient";
            Gradient aGradient = new Gradient();
            aGradient.Style = GradientStyle.LINEAR;
            aGradient.StartColor = 0xe0ffff; // light cyan
            aGradient.EndColor  = 0xff8c00; // dark orange
            aGradient.Angle = 300;          // 30 degrees
            aGradient.Border = 15;
            aGradient.XOffset = 0;
            aGradient.YOffset = 0;
            aGradient.StartIntensity = 100;
            aGradient.EndIntensity = 80;
            aGradient.StepCount = 23;

            xGradientTable.insertByName( aGradientName, aGradient );
            xProp.setPropertyValue( "FillStyle", FillStyle.GRADIENT );
            xProp.setPropertyValue( "FillGradientName", aGradientName );
            String aNewGradientName = AnyConverter.toString( xProp.getPropertyValue( "FillGradientName" ));
            assure( "GradientName", aNewGradientName.equals( aGradientName ));
            Gradient aNewGradient = (Gradient) AnyConverter.toObject(
                new Type( Gradient.class ),
                xGradientTable.getByName( aNewGradientName ));
            assure( "Gradient Style", aNewGradient.Style == aGradient.Style );
            assure( "Gradient StartColor", aNewGradient.StartColor == aGradient.StartColor );
            assure( "Gradient EndColor", aNewGradient.EndColor == aGradient.EndColor );
            assure( "Gradient Angle", aNewGradient.Angle == aGradient.Angle );
            assure( "Gradient Border", aNewGradient.Border == aGradient.Border );
            assure( "Gradient XOffset", aNewGradient.XOffset == aGradient.XOffset );
            assure( "Gradient YOffset", aNewGradient.YOffset == aGradient.YOffset );
            assure( "Gradient StartIntensity", aNewGradient.StartIntensity == aGradient.StartIntensity );
            assure( "Gradient EndIntensity", aNewGradient.EndIntensity == aGradient.EndIntensity );
            assure( "Gradient StepCount", aNewGradient.StepCount == aGradient.StepCount );

            // Hatch
            xProp = xDia.getDataPointProperties( 1, 0 );
            assure( "No DataPointProperties for (1,0)", xProp != null );

            // note: the FillHatch property is optional, however it was
            // supported in the old chart's API
            XNameContainer xHatchTable = UnoRuntime.queryInterface(
                XNameContainer.class,
                xFact.createInstance( "com.sun.star.drawing.HatchTable" ));
            assure( "no hatch table", xHatchTable != null );
            String aHatchName = "NewAPITestHatch";
            Hatch aHatch = new Hatch();
            aHatch.Style = HatchStyle.DOUBLE;
            aHatch.Color = 0xd2691e; // chocolate
            aHatch.Distance = 200;   // 2 mm (?)
            aHatch.Angle = 230;      // 23 degrees

            xHatchTable.insertByName( aHatchName, aHatch );
            xProp.setPropertyValue( "FillHatchName", aHatchName );
            xProp.setPropertyValue( "FillStyle", FillStyle.HATCH );
            xProp.setPropertyValue( "FillBackground", Boolean.TRUE);
            String aNewHatchName = AnyConverter.toString( xProp.getPropertyValue( "FillHatchName" ));
            assure( "HatchName", aNewHatchName.equals( aHatchName ));
            Hatch aNewHatch = (Hatch) AnyConverter.toObject(
                new Type( Hatch.class ),
                xHatchTable.getByName( aNewHatchName ));
            assure( "Hatch Style", aNewHatch.Style == aHatch.Style );
            assure( "Hatch Color", aNewHatch.Color == aHatch.Color );
            assure( "Hatch Distance", aNewHatch.Distance == aHatch.Distance );
            assure( "Hatch Angle", aNewHatch.Angle == aHatch.Angle );
            assure( "FillBackground", AnyConverter.toBoolean( xProp.getPropertyValue( "FillBackground" )) );
        }
        catch( Exception ex )
        {
            failed( ex.getMessage() );
            ex.printStackTrace( (PrintWriter)log );
        }
    }



    public void testStatistics()
    {
        try
        {
            XDiagram xDia = mxOldDoc.getDiagram();
            assure( "Invalid Diagram", xDia != null );

            XPropertySet xProp = xDia.getDataRowProperties( 0 );
            assure( "No DataRowProperties for first series", xProp != null );

            xProp.setPropertyValue( "MeanValue", Boolean.TRUE);
            assure( "No MeanValue", AnyConverter.toBoolean( xProp.getPropertyValue( "MeanValue" )) );
        }
        catch( Exception ex )
        {
            failed( ex.getMessage() );
            ex.printStackTrace( (PrintWriter)log );
        }
    }



    public void setStockData_Type4()
    {
        try
        {
            XPropertySet xDiaProp = UnoRuntime.queryInterface(
                XPropertySet.class, mxOldDoc.getDiagram() );

            ChartDataRowSource eNewSource = ChartDataRowSource.ROWS;
            xDiaProp.setPropertyValue( "DataRowSource", eNewSource );
            assure( "Couldn't set \"DataRowSource\" property at Diagram",
                    AnyConverter.toObject(
                        new Type( ChartDataRowSource.class ),
                        xDiaProp.getPropertyValue( "DataRowSource" )) == eNewSource );

            double aData[][] =
                {
                    { 100.0, 200.0, 300.0, 250.0, 300.0 },
                    { 6.5, 4.5, 6.0, 5.5, 3.5 },
                    { 1.0, 1.5, 2.0, 2.5, 3.0 },
                    { 6.0, 6.5, 7.0, 6.5, 5.0 },
                    { 6.0, 5.5, 4.0, 4.5, 4.0 }
                };

            String[] aRowDescriptions =
                {
                    "Volume", "Open", "Min", "Max", "Close"
                };

             String[] aColumnDescriptions =
             {
                 "First Row", "Second Row", "Third Row", "Fourth Row", "Fifth Row"
             };


            XChartData xData = mxOldDoc.getData();
            XChartDataArray xDataArray = UnoRuntime.queryInterface(
                XChartDataArray.class, xData );
            assure( "document has no XChartDataArray", xDataArray != null );

            xDataArray.setData( aData );
            xDataArray.setRowDescriptions( aRowDescriptions );
            xDataArray.setColumnDescriptions( aColumnDescriptions );

            mxOldDoc.attachData( xData );
        }
        catch( Exception ex )
        {
            failed( ex.getMessage() );
            ex.printStackTrace( (PrintWriter)log );
        }
    }



    public void testStockProperties()
    {
        try
        {
            setStockData_Type4();

            XMultiServiceFactory xFact = UnoRuntime.queryInterface(
                XMultiServiceFactory.class, mxOldDoc );
            assure( "document is no factory", xFact != null );

            String aMyServiceName = "com.sun.star.chart.StockDiagram";
            XDiagram xDia = UnoRuntime.queryInterface(
                XDiagram.class, xFact.createInstance( aMyServiceName ));
            assure( aMyServiceName + " could not be created", xDia != null );

            mxOldDoc.setDiagram( xDia );

            XPropertySet xDiaProp = UnoRuntime.queryInterface(
                XPropertySet.class, xDia );
            assure( "Diagram is no XPropertySet", xDiaProp != null );

            xDiaProp.setPropertyValue( "Volume", Boolean.TRUE);
            assure( "Has Volume", AnyConverter.toBoolean( xDiaProp.getPropertyValue( "Volume" )));

            xDiaProp.setPropertyValue( "UpDown", Boolean.TRUE);
            assure( "Has UpDown", AnyConverter.toBoolean( xDiaProp.getPropertyValue( "UpDown" )));

            // MinMaxLine
            XStatisticDisplay xMinMaxProvider = UnoRuntime.queryInterface(
                XStatisticDisplay.class, xDia );
            assure( "Diagram is no XStatisticDisplay", xMinMaxProvider != null );
            XPropertySet xMinMaxProp = xMinMaxProvider.getMinMaxLine();
            assure( "No MinMaxLine", xMinMaxProp != null );

            int nLineColor = 0x458b00; // chartreuse4
            xMinMaxProp.setPropertyValue( "LineColor", Integer.valueOf( nLineColor ));
            int nNewColor = AnyConverter.toInt( xMinMaxProp.getPropertyValue( "LineColor" ) );
            assure( "Changing LineColor of MinMax Line", nNewColor == nLineColor );
        }
        catch( Exception ex )
        {
            failed( ex.getMessage() );
            ex.printStackTrace( (PrintWriter)log );
        }
    }



    public void testFactory()
    {
        try
        {
            XMultiServiceFactory xFact = UnoRuntime.queryInterface(
                XMultiServiceFactory.class, mxOldDoc );
            assure( "document is no factory", xFact != null );

            Object aTestTable = xFact.createInstance( "com.sun.star.drawing.GradientTable" );
            assure( "Couldn't create gradient table via factory", aTestTable != null );
        }
        catch( Exception ex )
        {
            failed( ex.getMessage() );
            ex.printStackTrace( (PrintWriter)log );
        }
    }



    public void testData()
    {
        try
        {
            // set data
            double aData[][] = {
                { 1.0, 1.5, 2.0, 2.5, 3.0 },
                { 2.0, 2.5, 3.0, 3.5, 4.0 },
                { 3.0, 3.5, 4.0, 4.5, 5.0 }
            };

            String[] aColumnDescriptions = {
                "First Column", "Second Column", "Third Column",
                "Fourth Column", "Fifth Column"
            };

            String[] aRowDescriptions = {
                "First Row", "Second Row", "Third Row"
            };

            XPropertySet xDiaProp = UnoRuntime.queryInterface(
                    XPropertySet.class, mxOldDoc.getDiagram() );
            ChartDataRowSource eNewSource = ChartDataRowSource.ROWS;
            xDiaProp.setPropertyValue( "DataRowSource", eNewSource );
            assure( "Couldn't set \"DataRowSource\" property at Diagram",
                    AnyConverter.toObject(
                        new Type( ChartDataRowSource.class ),
                        xDiaProp.getPropertyValue( "DataRowSource" )) == eNewSource );

            XChartData xData = mxOldDoc.getData();
            XChartDataArray xDataArray = UnoRuntime.queryInterface(
                XChartDataArray.class, xData );
            assure( "document has no XChartDataArray", xDataArray != null );

            xDataArray.setData( aData );
            xDataArray.setRowDescriptions( aRowDescriptions );
            xDataArray.setColumnDescriptions( aColumnDescriptions );

            mxOldDoc.attachData( xData );

            // get data
            double aReadData[][];
            String[] aReadColumnDescriptions;
            String[] aReadRowDescriptions;

            // refetch data
            xData = mxOldDoc.getData();
            xDataArray = UnoRuntime.queryInterface(
                XChartDataArray.class, xData );
            assure( "document has no XChartDataArray", xDataArray != null );

            aReadData = xDataArray.getData();
            aReadRowDescriptions = xDataArray.getRowDescriptions();
            aReadColumnDescriptions = xDataArray.getColumnDescriptions();

            // compare to values set before
            assure( "Data size differs", aData.length == aReadData.length );
            for( int i=0; i<aReadData.length; ++i )
            {
                assure( "Data size differs", aData[i].length == aReadData[i].length );
                for( int j=0; j<aReadData[i].length; ++j )
                    assure( "Data differs", aData[i][j] == aReadData[i][j] );
            }

            assure( "Column Description size differs", aColumnDescriptions.length == aReadColumnDescriptions.length );
            for( int i=0; i<aReadColumnDescriptions.length; ++i )
                assure( "Column Descriptions differ", aColumnDescriptions[i].equals( aReadColumnDescriptions[i] ));

            assure( "Row Description size differs", aRowDescriptions.length == aReadRowDescriptions.length );
            for( int i=0; i<aReadRowDescriptions.length; ++i )
                assure( "Row Descriptions differ", aRowDescriptions[i].equals( aReadRowDescriptions[i] ));
        }
        catch( Exception ex )
        {
            failed( ex.getMessage() );
            ex.printStackTrace( (PrintWriter)log );
        }
    }



    private XModel                    mxChartModel;
    private XChartDocument            mxOldDoc;
    private boolean                   mbCreateView;



    private XModel createDocument( String sDocType )
    {
        XModel aResult = null;
        try
        {
            XComponentLoader aLoader = UnoRuntime.queryInterface(
                XComponentLoader.class,
                param.getMSF().createInstance( "com.sun.star.frame.Desktop" ) );

            aResult = UnoRuntime.queryInterface(
                XModel.class,
                aLoader.loadComponentFromURL( "private:factory/" + sDocType,
                                              "_blank",
                                              0,
                                              new PropertyValue[ 0 ] ) );
        }
        catch( Exception ex )
        {
            failed( ex.getMessage() );
            ex.printStackTrace( (PrintWriter)log );
        }

        return aResult;
    }



    public XModel createChartModel()
    {
        XModel aResult = null;
        try
        {
            aResult = UnoRuntime.queryInterface(
                XModel.class,
                param.getMSF().createInstance( "com.sun.star.comp.chart2.ChartModel" ) );
        }
        catch( Exception ex )
        {
            failed( ex.getMessage() );
            ex.printStackTrace( (PrintWriter)log );
        }

        return aResult;
    }

    /** returns true if a and b differ no more than tolerance.

        @param tolerance
            must be non-negative
     */
    private boolean approxEqual( int a, int b, int tolerance )
    {
        if( a != b )
            log.println( "Integer values differ by " + Math.abs( a-b ));
        return ( ( a - tolerance <= b ) ||
                 ( a + tolerance >= b ));
    }
}
