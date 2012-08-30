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

// imports
import complexlib.ComplexTestCase;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.Type;
import com.sun.star.uno.XComponentContext;

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

/**
 * The following Complex Test will test the
 * com.sun.star.document.IndexedPropertyValues
 * service
 */

public class TestCaseOldAPI extends ComplexTestCase {

    // The name of the tested service
    private final String testedServiceName =
          "com.sun.star.chart.ChartDocument";

    // The first of the mandatory functions:
    /**
     * Return the name of the test.
     * In this case it is the actual name of the service.
     * @return The tested service.
     */
    public String getTestObjectName() {
        return testedServiceName;
    }

    // The second of the mandatory functions: return all test methods as an
    // array. There is only one test function in this example.
    /**
     * Return all test methods.
     * @return The test methods.
     */
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

    // ____________

    public void before()
    {
        // set to "true" to get a view
        mbCreateView = true;

        if( mbCreateView )
            mxChartModel = createDocument( "schart" );
        else
            mxChartModel = createChartModel();

        mxOldDoc = (XChartDocument) UnoRuntime.queryInterface(
            XChartDocument.class, mxChartModel );
    }

    // ____________

    public void after()
    {
        XCloseable xCloseable = (XCloseable) UnoRuntime.queryInterface(
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

    // ____________

    public void testTitle()
    {
        try
        {
            XPropertySet xDocProp = (XPropertySet) UnoRuntime.queryInterface(
                XPropertySet.class, mxOldDoc );
            assure( "Chart Document is no XPropertySet", xDocProp != null );
            xDocProp.setPropertyValue( "HasMainTitle", new Boolean( true ));
            assure( "Property HasMainTitle", AnyConverter.toBoolean(
                        xDocProp.getPropertyValue( "HasMainTitle" )));

            XShape xTitleShape = mxOldDoc.getTitle();
            XPropertySet xTitleProp = (XPropertySet) UnoRuntime.queryInterface(
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

    // ____________

    public void testSubTitle()
    {
        try
        {
            XPropertySet xDocProp = (XPropertySet) UnoRuntime.queryInterface(
                XPropertySet.class, mxOldDoc );
            assure( "Chart Document is no XPropertySet", xDocProp != null );
            xDocProp.setPropertyValue( "HasSubTitle", new Boolean( true ));
            assure( "Property HasSubTitle", AnyConverter.toBoolean(
                        xDocProp.getPropertyValue( "HasSubTitle" )));

            XShape xTitleShape = mxOldDoc.getSubTitle();
            XPropertySet xTitleProp = (XPropertySet) UnoRuntime.queryInterface(
                XPropertySet.class, xTitleShape );

            // set Property via old API
            if( xTitleProp != null )
            {
                int nColor = 0x009acd; // DeepSkyBlue3
                float fWeight = FontWeight.BOLD;
                float fHeight = (float)14.0;

                xTitleProp.setPropertyValue( "CharColor", new Integer( nColor ) );
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

    // ------------

    public void testDiagram()
    {
        try
        {
            // testing wall
            XDiagram xDia = mxOldDoc.getDiagram();
            if( xDia != null )
            {
                X3DDisplay xDisp = (X3DDisplay) UnoRuntime.queryInterface(
                    X3DDisplay.class, xDia );
                assure( "X3DDisplay not supported", xDisp != null );

                // Wall
                XPropertySet xProp = xDisp.getWall();
                if( xProp != null )
                {
                    int nColor = 0xffe1ff; // thistle1
                    xProp.setPropertyValue( "FillColor", new Integer( nColor ) );
                    int nNewColor = AnyConverter.toInt( xProp.getPropertyValue( "FillColor" ) );
                    assure( "Changing FillColor via old API failed", nNewColor == nColor );
                }

                assure( "Wrong Diagram Type", xDia.getDiagramType().equals(
                            "com.sun.star.chart.BarDiagram" ));

                // Diagram properties
                xProp = (XPropertySet) UnoRuntime.queryInterface( XPropertySet.class, xDia );
                assure( "Diagram is no property set", xProp != null );

                // y-axis
                boolean bFirstYAxisText = false;
                xProp.setPropertyValue( "HasYAxisDescription", new Boolean( bFirstYAxisText ));
                boolean bNewFirstYAxisText = AnyConverter.toBoolean(
                    xProp.getPropertyValue( "HasYAxisDescription" ));
                assure( "Removing description of first y-axis", bNewFirstYAxisText == bFirstYAxisText );

//                 boolean bYAxisTitle = true;
//                 xProp.setPropertyValue( "HasYAxisTitle", new Boolean( bYAxisTitle ));
//                 boolean bNewYAxisTitle = AnyConverter.toBoolean(
//                     xProp.getPropertyValue( "HasYAxisTitle" ));
//                 assure( "Adding y-axis title", bNewYAxisTitle == bYAxisTitle );

                // set title text
//                 XAxisYSupplier xYAxisSuppl = (XAxisYSupplier) UnoRuntime.queryInterface(
//                     XAxisYSupplier.class, mxOldDoc.getDiagram() );
//                 assure( "Diagram is no y-axis supplier", xYAxisSuppl != null );
//                 XPropertySet xAxisTitleProp = (XPropertySet) UnoRuntime.queryInterface(
//                     XPropertySet.class, xYAxisSuppl.getYAxisTitle() );
//                 assure( "Y-Axis Title is no XPropertySet", xAxisTitleProp != null );
//                 xAxisTitleProp.setPropertyValue( "String", "New y axis title" );

                // second y-axis
                boolean bSecondaryYAxis = true;
                xProp.setPropertyValue( "HasSecondaryYAxis", new Boolean( bSecondaryYAxis ));
                boolean bNewSecYAxisValue = AnyConverter.toBoolean(
                    xProp.getPropertyValue( "HasSecondaryYAxis" ));
                assure( "Adding a second y-axis does not work", bNewSecYAxisValue == bSecondaryYAxis );

                XTwoAxisYSupplier xSecYAxisSuppl = (XTwoAxisYSupplier) UnoRuntime.queryInterface(
                    XTwoAxisYSupplier.class, xDia );
                assure( "XTwoAxisYSupplier not implemented", xSecYAxisSuppl != null );
                assure( "No second y-axis found", xSecYAxisSuppl.getSecondaryYAxis() != null );
            }

            // move diagram
            {
                XShape xDiagramShape = (XShape) UnoRuntime.queryInterface(
                    XShape.class, xDia );

                Point aOldPos = xDiagramShape.getPosition();
                int xDiff = 20;
                int yDiff = 20;
                Point aSetPos = new Point();
                aSetPos.X = aOldPos.X + xDiff;
                aSetPos.Y = aOldPos.Y + yDiff;
                xDiagramShape.setPosition( aSetPos );

                Point aNewPos = xDiagramShape.getPosition();
                //System.out.println( "set X = " + aSetPos.X + ", new X = " + aNewPos.X );
                //System.out.println( "set Y = " + aSetPos.Y + ", new Y = " + aNewPos.Y );
                assure( "Diagram Position X", approxEqual( aNewPos.X, aSetPos.X, 1 ));
                assure( "Diagram Position Y", approxEqual( aNewPos.Y, aSetPos.Y, 1 ));
            }

            // size diagram
            {
                XShape xDiagramShape = (XShape) UnoRuntime.queryInterface(
                    XShape.class, xDia );

                Size aOldSize = xDiagramShape.getSize();
                int xDiff = aOldSize.Width/2+2;
                int yDiff = aOldSize.Height/2+2;
                Size aSetSize = new Size();
                aSetSize.Width = aOldSize.Width - xDiff;
                aSetSize.Height = aOldSize.Height - yDiff;
                xDiagramShape.setSize( aSetSize );

                Size aNewSize = xDiagramShape.getSize();
                //System.out.println( "set width = " + aSetSize.Width + ", new width = " + aNewSize.Width );
                //System.out.println( "set height = " + aSetSize.Height + ", new height = " + aNewSize.Height );
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

    // ------------

    public void testAxis()
    {
        try
        {
            XAxisYSupplier xYAxisSuppl = (XAxisYSupplier) UnoRuntime.queryInterface(
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
//             assure( "Max is 0.0", fMax1 > 0.0 );
            xProp.setPropertyValue( "AutoMax", new Boolean( false ));
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
                    approxEqual(
                        AnyConverter.toDouble( xProp.getPropertyValue( "Max" )),
                        nNewMax ));

            xProp.setPropertyValue( "AutoMin", new Boolean( true ));
            assure( "AutoMin is off", AnyConverter.toBoolean( xProp.getPropertyValue( "AutoMin" )) );

            xProp.setPropertyValue( "Origin", new Double( nNewOrigin ));
            assure( "Origin invalid",
                    approxEqual(
                        AnyConverter.toDouble( xProp.getPropertyValue( "Origin" )),
                        nNewOrigin ));
            xProp.setPropertyValue( "AutoOrigin", new Boolean( true ));
            assure( "AutoOrigin is off", AnyConverter.toBoolean( xProp.getPropertyValue( "AutoOrigin" )) );
            Object oOrigin = xProp.getPropertyValue( "Origin" );
            assure( "No Origin set", AnyConverter.isDouble( oOrigin ));
            log.println( "Origin retrieved: " + AnyConverter.toDouble( oOrigin ));

            xProp.setPropertyValue( "Logarithmic", new Boolean( true ));
            assure( "Scaling is not logarithmic",
                    AnyConverter.toBoolean( xProp.getPropertyValue( "Logarithmic" )) );
            xProp.setPropertyValue( "Logarithmic", new Boolean( false ));
            assure( "Scaling is not logarithmic",
                    ! AnyConverter.toBoolean( xProp.getPropertyValue( "Logarithmic" )) );

            int nNewColor =  0xcd853f; // peru
            xProp.setPropertyValue( "LineColor", new Integer( nNewColor ));
            assure( "Property LineColor",
                    AnyConverter.toInt( xProp.getPropertyValue( "LineColor" )) == nNewColor );
            float fNewCharHeight = (float)(16.0);
            xProp.setPropertyValue( "CharHeight", new Float( fNewCharHeight ));
            assure( "Property CharHeight",
                    AnyConverter.toFloat( xProp.getPropertyValue( "CharHeight" )) == fNewCharHeight );

            int nNewTextRotation = 700; // in 1/100 degrees
            xProp.setPropertyValue( "TextRotation", new Integer( nNewTextRotation ));
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

            xProp.setPropertyValue( "DisplayLabels", new Boolean( false ));
            assure( "Property DisplayLabels", ! AnyConverter.toBoolean(
                        xProp.getPropertyValue( "DisplayLabels" )));
        }
        catch( Exception ex )
        {
            failed( ex.getMessage() );
            ex.printStackTrace( (PrintWriter)log );
        }
    }

    // ------------

    public void testLegend()
    {
        XShape xLegend = mxOldDoc.getLegend();
        assure( "No Legend returned", xLegend != null );

        XPropertySet xLegendProp = (XPropertySet) UnoRuntime.queryInterface(
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

    // ------------

    public void testArea()
    {
        XPropertySet xArea = mxOldDoc.getArea();
        assure( "No Area", xArea != null );

        try
        {
            int nColor = 0xf5fffa; // mint cream
            xArea.setPropertyValue( "FillColor", new Integer( nColor ) );
            xArea.setPropertyValue( "FillStyle", FillStyle.SOLID );
//             XPropertySetInfo xInfo = xArea.getPropertySetInfo();
//             assure( "Area does not support ChartUserDefinedAttributes",
//                     xInfo.hasPropertyByName( "ChartUserDefinedAttributes" ));

//             String aTestAttributeName = "test:foo";
//             String aTestAttributeValue = "content";
//             XNameContainer xUserDefAttributes = (XNameContainer) AnyConverter.toObject(
//                 new Type( XNameContainer.class ), xArea.getPropertyValue( "ChartUserDefinedAttributes" ));
//             xUserDefAttributes.insertByName( aTestAttributeName, aTestAttributeValue );

//             String aContent = AnyConverter.toString( xUserDefAttributes.getByName( aTestAttributeName ));
//             assure( "Wrong content in UserDefinedAttributes container",
//                     aContent.equals( aTestAttributeValue ));

            int nNewColor = AnyConverter.toInt( xArea.getPropertyValue( "FillColor" ) );
            assure( "Changing FillColor of Area failed", nNewColor == nColor );
        }
        catch( Exception ex )
        {
            failed( ex.getMessage() );
            ex.printStackTrace( (PrintWriter)log );
        }
    }

    // ------------

    public void testChartType()
    {
        XMultiServiceFactory xFact = (XMultiServiceFactory) UnoRuntime.queryInterface(
            XMultiServiceFactory.class, mxOldDoc );
        assure( "document is no factory", xFact != null );

        try
        {
            String aMyServiceName = new String( "com.sun.star.chart.BarDiagram" );
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
                XDiagram xDia = (XDiagram) UnoRuntime.queryInterface(
                    XDiagram.class, xFact.createInstance( aMyServiceName ));
                assure( aMyServiceName + " could not be created", xDia != null );

                mxOldDoc.setDiagram( xDia );

                XPropertySet xDiaProp = (XPropertySet) UnoRuntime.queryInterface(
                    XPropertySet.class, xDia );
                assure( "Diagram is no XPropertySet", xDiaProp != null );

                xDiaProp.setPropertyValue( "Stacked", new Boolean( true ));
                assure( "StackMode could not be set correctly",
                        AnyConverter.toBoolean(
                            xDiaProp.getPropertyValue( "Stacked" )));

                xDiaProp.setPropertyValue( "Dim3D", new Boolean( false ));
                assure( "Dim3D could not be set correctly",
                        ! AnyConverter.toBoolean(
                            xDiaProp.getPropertyValue( "Dim3D" )));

                xDiaProp.setPropertyValue( "Vertical", new Boolean( true ));
                assure( "Vertical could not be set correctly",
                        AnyConverter.toBoolean(
                            xDiaProp.getPropertyValue( "Vertical" )));
            }

            // reset to bar-chart
//             aMyServiceName = new String( "com.sun.star.chart.BarDiagram" );
//             XDiagram xDia = (XDiagram) UnoRuntime.queryInterface(
//                 XDiagram.class, xFact.createInstance( aMyServiceName ));
//             assure( aMyServiceName + " could not be created", xDia != null );

//             mxOldDoc.setDiagram( xDia );
        }
        catch( Exception ex )
        {
            failed( ex.getMessage() );
            ex.printStackTrace( (PrintWriter)log );
        }
    }

    // ------------

    public void testAggregation()
    {
        // query to new type
        XChartDocument xDiaProv = (XChartDocument) UnoRuntime.queryInterface(
            XChartDocument.class, mxOldDoc );
        assure( "query to new interface failed", xDiaProv != null );

        com.sun.star.chart.XChartDocument xDoc = (com.sun.star.chart.XChartDocument) UnoRuntime.queryInterface(
            com.sun.star.chart.XChartDocument.class, xDiaProv );
        assure( "querying back to old interface failed", xDoc != null );
    }

    // ------------

    public void testDataSeriesAndPoints()
    {
        try
        {
            XDiagram xDia = mxOldDoc.getDiagram();
            assure( "Invalid Diagram", xDia != null );
            XMultiServiceFactory xFact = (XMultiServiceFactory) UnoRuntime.queryInterface(
                XMultiServiceFactory.class, mxOldDoc );
            assure( "document is no factory", xFact != null );

            // FillColor
            XPropertySet xProp = xDia.getDataRowProperties( 0 );
            int nColor = 0xffd700; // gold
            xProp.setPropertyValue( "FillColor", new Integer( nColor ));
            int nNewColor = AnyConverter.toInt( xProp.getPropertyValue( "FillColor" ) );
            assure( "Changing FillColor of Data Series failed", nNewColor == nColor );

            // Gradient
            assure( "No DataRowProperties for series 0", xProp != null );

            // note: the FillGradient property is optional, however it was
            // supported in the old chart's API
            XNameContainer xGradientTable = (XNameContainer) UnoRuntime.queryInterface(
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
            XNameContainer xHatchTable = (XNameContainer) UnoRuntime.queryInterface(
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
            xProp.setPropertyValue( "FillBackground", new Boolean( true ));
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

    // ------------

    public void testStatistics()
    {
        try
        {
            XDiagram xDia = mxOldDoc.getDiagram();
            assure( "Invalid Diagram", xDia != null );

            XPropertySet xProp = xDia.getDataRowProperties( 0 );
            assure( "No DataRowProperties for first series", xProp != null );

            xProp.setPropertyValue( "MeanValue", new Boolean( true ));
            assure( "No MeanValue", AnyConverter.toBoolean( xProp.getPropertyValue( "MeanValue" )) );
        }
        catch( Exception ex )
        {
            failed( ex.getMessage() );
            ex.printStackTrace( (PrintWriter)log );
        }
    }

    // ------------

    public void setStockData_Type4()
    {
        try
        {
            XPropertySet xDiaProp = (XPropertySet) UnoRuntime.queryInterface(
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
            XChartDataArray xDataArray = (XChartDataArray) UnoRuntime.queryInterface(
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

    // ------------

    public void testStockProperties()
    {
        try
        {
            setStockData_Type4();

            XMultiServiceFactory xFact = (XMultiServiceFactory) UnoRuntime.queryInterface(
                XMultiServiceFactory.class, mxOldDoc );
            assure( "document is no factory", xFact != null );

            String aMyServiceName = new String( "com.sun.star.chart.StockDiagram" );
            XDiagram xDia = (XDiagram) UnoRuntime.queryInterface(
                XDiagram.class, xFact.createInstance( aMyServiceName ));
            assure( aMyServiceName + " could not be created", xDia != null );

            mxOldDoc.setDiagram( xDia );

            XPropertySet xDiaProp = (XPropertySet) UnoRuntime.queryInterface(
                XPropertySet.class, xDia );
            assure( "Diagram is no XPropertySet", xDiaProp != null );

            xDiaProp.setPropertyValue( "Volume", new Boolean( true ));
            assure( "Has Volume", AnyConverter.toBoolean( xDiaProp.getPropertyValue( "Volume" )));

            xDiaProp.setPropertyValue( "UpDown", new Boolean( true ));
            assure( "Has UpDown", AnyConverter.toBoolean( xDiaProp.getPropertyValue( "UpDown" )));

            // MinMaxLine
            XStatisticDisplay xMinMaxProvider = (XStatisticDisplay) UnoRuntime.queryInterface(
                XStatisticDisplay.class, xDia );
            assure( "Diagram is no XStatisticDisplay", xMinMaxProvider != null );
            XPropertySet xMinMaxProp = xMinMaxProvider.getMinMaxLine();
            assure( "No MinMaxLine", xMinMaxProp != null );

            int nLineColor = 0x458b00; // chartreuse4
            xMinMaxProp.setPropertyValue( "LineColor", new Integer( nLineColor ));
            int nNewColor = AnyConverter.toInt( xMinMaxProp.getPropertyValue( "LineColor" ) );
            assure( "Changing LineColor of MinMax Line", nNewColor == nLineColor );
        }
        catch( Exception ex )
        {
            failed( ex.getMessage() );
            ex.printStackTrace( (PrintWriter)log );
        }
    }

    // ------------

    public void testFactory()
    {
        try
        {
            XMultiServiceFactory xFact = (XMultiServiceFactory) UnoRuntime.queryInterface(
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

    // ------------

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

            XPropertySet xDiaProp = (XPropertySet) UnoRuntime.queryInterface(
                    XPropertySet.class, mxOldDoc.getDiagram() );
            ChartDataRowSource eNewSource = ChartDataRowSource.ROWS;
            xDiaProp.setPropertyValue( "DataRowSource", eNewSource );
            assure( "Couldn't set \"DataRowSource\" property at Diagram",
                    AnyConverter.toObject(
                        new Type( ChartDataRowSource.class ),
                        xDiaProp.getPropertyValue( "DataRowSource" )) == eNewSource );

            XChartData xData = mxOldDoc.getData();
            XChartDataArray xDataArray = (XChartDataArray) UnoRuntime.queryInterface(
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
            xDataArray = (XChartDataArray) UnoRuntime.queryInterface(
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

    // ================================================================================

    private XModel                    mxChartModel;
    private XChartDocument            mxOldDoc;
    private boolean                   mbCreateView;

    // --------------------------------------------------------------------------------

    private XModel createDocument( String sDocType )
    {
        XModel aResult = null;
        try
        {
            XComponentLoader aLoader = (XComponentLoader) UnoRuntime.queryInterface(
                XComponentLoader.class,
                ((XMultiServiceFactory)param.getMSF()).createInstance( "com.sun.star.frame.Desktop" ) );

            aResult = (XModel) UnoRuntime.queryInterface(
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

    // ------------

    public XModel createChartModel()
    {
        XModel aResult = null;
        try
        {
            aResult = (XModel) UnoRuntime.queryInterface(
                XModel.class,
                ((XMultiServiceFactory)param.getMSF()).createInstance( "com.sun.star.comp.chart2.ChartModel" ) );
        }
        catch( Exception ex )
        {
            failed( ex.getMessage() );
            ex.printStackTrace( (PrintWriter)log );
        }

        return aResult;
    }

    // ------------

    private XComponentContext getComponentContext( XMultiServiceFactory xFact )
    {
        XComponentContext xResult = null;

        XPropertySet xProp = (XPropertySet) UnoRuntime.queryInterface(
            XPropertySet.class, xFact );
        if( xProp != null )
            try
            {
                xResult = (XComponentContext)
                    AnyConverter.toObject(
                        new Type( XComponentContext.class ),
                        xProp.getPropertyValue( "DefaultContext" ) );
            }
            catch( Exception ex )
            {
                failed( ex.getMessage() );
                ex.printStackTrace( (PrintWriter)log );
            }

        return xResult;
    }

    // ------------

    private void printInterfacesAndServices( Object oObj )
    {
        log.println( "Services:" );
        util.dbg.getSuppServices( oObj );
        log.println( "Interfaces:" );
        util.dbg.printInterfaces( (XInterface)oObj, true );
    }

    // ------------

    /// see rtl/math.hxx
    private boolean approxEqual( double a, double b )
    {
        if( a == b )
            return true;
        double x = a - b;
        return (x < 0.0 ? -x : x)
            < ((a < 0.0 ? -a : a) * (1.0 / (16777216.0 * 16777216.0)));
    }

    // ------------
    /** returns true if a and b differ no more than tolerance.

        @param tolerance
            must be non-negative
     */
    private boolean approxEqual( int a, int b, int tolerance )
    {
        if( a != b )
            log.println( "Integer values differ by " + java.lang.Math.abs( a-b ));
        return ( ( a - tolerance <= b ) ||
                 ( a + tolerance >= b ));
    }
}
