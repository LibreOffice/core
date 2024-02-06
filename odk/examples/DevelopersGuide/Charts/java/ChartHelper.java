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


// base graphics things
import com.sun.star.awt.Point;
import com.sun.star.awt.Size;
import com.sun.star.beans.XPropertySet;
// application specific classes
import com.sun.star.chart.XChartDocument;
import com.sun.star.chart.XDiagram;
import com.sun.star.drawing.XDrawPageSupplier;
import com.sun.star.drawing.XDrawPagesSupplier;
import com.sun.star.drawing.XShape;
import com.sun.star.drawing.XShapes;
import com.sun.star.frame.XModel;
// factory for creating components
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XText;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.Any;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;

// __________ Implementation __________

// Helper for creating an OLE chart

public class ChartHelper
{
    public ChartHelper( XModel aContainerDoc )
    {
        maContainerDocument = aContainerDoc;
    }

    public XChartDocument insertOLEChartInWriter(
        Point  aUpperLeft,
        Size   aExtent,
        String sChartServiceName )
    {
        XChartDocument aResult = null;

        XMultiServiceFactory aFact = UnoRuntime.queryInterface(XMultiServiceFactory.class,
                                  maContainerDocument );

        if( aFact != null )
        {
            try
            {
                XTextContent xTextContent = UnoRuntime.queryInterface(
                XTextContent.class,
                aFact.createInstance("com.sun.star.text.TextEmbeddedObject"));

                if ( xTextContent != null )
                {
                    XPropertySet xPropSet = UnoRuntime.queryInterface(
                        XPropertySet.class, xTextContent);

                    Any aAny = new Any(new Type(String.class), msChartClassID);
                    xPropSet.setPropertyValue("CLSID", aAny );

                    XTextDocument xTextDoc = UnoRuntime.queryInterface(XTextDocument.class,
                                              maContainerDocument);
                    XText xText = xTextDoc.getText();
                    XTextCursor xCursor = xText.createTextCursor();

                    //insert embedded object in text -> object will be created
                    xText.insertTextContent( xCursor, xTextContent, true );

                    // set size and position
                    XShape xShape = UnoRuntime.queryInterface(
                        XShape.class, xTextContent);
                    xShape.setSize( aExtent );

                    aAny = new Any(new Type(Short.class),
                               Short.valueOf(com.sun.star.text.VertOrientation.NONE));
                    xPropSet.setPropertyValue("VertOrient", aAny );
                    aAny = new Any(new Type(Short.class),
                               Short.valueOf(com.sun.star.text.HoriOrientation.NONE));
                    xPropSet.setPropertyValue("HoriOrient", aAny );
                    aAny = new Any(new Type(Integer.class), Integer.valueOf(aUpperLeft.Y));
                    xPropSet.setPropertyValue("VertOrientPosition", aAny );
                    aAny = new Any(new Type(Integer.class), Integer.valueOf(aUpperLeft.X));
                    xPropSet.setPropertyValue("HoriOrientPosition", aAny );

                    // retrieve the chart document as model of the OLE shape
                    aResult = UnoRuntime.queryInterface(
                            XChartDocument.class,
                            xPropSet.getPropertyValue( "Model" ));

                    // create a diagram via the factory and set this as
                    // new diagram
                    aResult.setDiagram(
                        UnoRuntime.queryInterface(
                            XDiagram.class,
                            UnoRuntime.queryInterface(
                                XMultiServiceFactory.class,
                                aResult ).createInstance(sChartServiceName )));
                }
            } catch( Exception ex)
            {
                System.out.println( "caught exception: " + ex );
            }
        }

        return aResult;
    }

    public XChartDocument insertOLEChartInDraw(
        Point  aUpperLeft,
        Size   aExtent,
        String sChartServiceName )
    {
        XChartDocument aResult = null;

        XShapes aPage = null;

        // try interface for multiple pages in a document
        XDrawPagesSupplier aSupplier = UnoRuntime.queryInterface(XDrawPagesSupplier.class,
                                  maContainerDocument );

        if( aSupplier != null )
        {
            try
            {
                // get first page
                aPage = UnoRuntime.queryInterface(
                    XShapes.class, aSupplier.getDrawPages().getByIndex( 0 ) );
            }
            catch( Exception ex )
            {
                System.out.println( "First page not found in shape collection: " +
                                    ex );
            }
        }
        else
        {
            // try interface for single draw page (e.g. spreadsheet)
            XDrawPageSupplier aOnePageSupplier = UnoRuntime.queryInterface(XDrawPageSupplier.class,
                                      maContainerDocument );

            if( aOnePageSupplier != null )
            {
                aPage = UnoRuntime.queryInterface(
                    XShapes.class, aOnePageSupplier.getDrawPage());
            }
        }

        if( aPage != null )
        {
            XMultiServiceFactory aFact = UnoRuntime.queryInterface(XMultiServiceFactory.class,
                                      maContainerDocument );

            if( aFact != null )
            {
                try
                {
                    // create an OLE shape
                    XShape aShape = UnoRuntime.queryInterface(
                        XShape.class,
                        aFact.createInstance( "com.sun.star.drawing.OLE2Shape" ));

                    // insert the shape into the page
                    aPage.add( aShape );
                    aShape.setPosition( aUpperLeft );
                    aShape.setSize( aExtent );

                    // make the OLE shape a chart
                    XPropertySet aShapeProp = UnoRuntime.queryInterface(XPropertySet.class, aShape );
                    if( aShapeProp != null )
                    {
                        // set the class id for charts
                        aShapeProp.setPropertyValue( "CLSID", msChartClassID );

                        // retrieve the chart document as model of the OLE shape
                        aResult = UnoRuntime.queryInterface(
                            XChartDocument.class,
                            aShapeProp.getPropertyValue( "Model" ));

                        // create a diagram via the factory and set this as
                        // new diagram
                        aResult.setDiagram(
                            UnoRuntime.queryInterface(
                                XDiagram.class,
                                UnoRuntime.queryInterface(
                                    XMultiServiceFactory.class,
                                    aResult ).createInstance(sChartServiceName )));
                    }
                }
                catch( Exception ex )
                {
                    System.out.println( "Couldn't change the OLE shape into a chart: " + ex );
                }
            }
        }

        return aResult;
    }


    // __________ private members and methods __________

    private static final String  msChartClassID   = "12dcae26-281f-416f-a234-c3086127382e";

    private final XModel maContainerDocument;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
