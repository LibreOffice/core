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



// __________ Imports __________


// base classes
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.Any;

// factory for creating components
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XComponent;
import com.sun.star.beans.XPropertySet;

// application specific classes
import com.sun.star.chart.XChartDocument;
import com.sun.star.chart.XDiagram;
import com.sun.star.drawing.*;
import com.sun.star.frame.XModel;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XText;
//import com.sun.star.text.VertOrientation;
//import com.sun.star.text.HoriOrientation;
import com.sun.star.document.XEmbeddedObjectSupplier;

// base graphics things
import com.sun.star.awt.Point;
import com.sun.star.awt.Size;

// Exceptions
import com.sun.star.uno.RuntimeException;
import com.sun.star.container.NoSuchElementException;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.lang.IndexOutOfBoundsException;

// __________ Implementation __________

/** Helper for creating an OLE chart
    @author Bj&ouml;rn Milcke
 */
public class ChartHelper
{
    public ChartHelper( XModel aContainerDoc )
    {
        maContainerDocument = aContainerDoc;
    }

    public XChartDocument insertOLEChartInWriter(
        String sChartName,
        Point  aUpperLeft,
        Size   aExtent,
        String sChartServiceName )
    {
        XChartDocument aResult = null;

        XMultiServiceFactory aFact = (XMultiServiceFactory)
            UnoRuntime.queryInterface(XMultiServiceFactory.class,
                                      maContainerDocument );

        if( aFact != null )
        {
            try
            {
                XTextContent xTextContent = (XTextContent)UnoRuntime.queryInterface(
                XTextContent.class,
                aFact.createInstance("com.sun.star.text.TextEmbeddedObject"));

                if ( xTextContent != null )
                {
                    XPropertySet xPropSet = (XPropertySet)UnoRuntime.queryInterface(
                        XPropertySet.class, xTextContent);

                    Any aAny = new Any(String.class, msChartClassID);
                    xPropSet.setPropertyValue("CLSID", aAny );

                    XTextDocument xTextDoc = (XTextDocument)
                        UnoRuntime.queryInterface(XTextDocument.class,
                                                  maContainerDocument);
                    XText xText = xTextDoc.getText();
                    XTextCursor xCursor = xText.createTextCursor();

                    //insert embedded object in text -> object will be created
                    xText.insertTextContent( xCursor, xTextContent, true );

                    // set size and position
                    XShape xShape = (XShape)UnoRuntime.queryInterface(
                        XShape.class, xTextContent);
                    xShape.setSize( aExtent );

                    aAny = new Any(Short.class,
                               new Short(com.sun.star.text.VertOrientation.NONE));
                    xPropSet.setPropertyValue("VertOrient", aAny );
                    aAny = new Any(Short.class,
                               new Short(com.sun.star.text.HoriOrientation.NONE));
                    xPropSet.setPropertyValue("HoriOrient", aAny );
                    aAny = new Any(Integer.class, new Integer(aUpperLeft.Y));
                    xPropSet.setPropertyValue("VertOrientPosition", aAny );
                    aAny = new Any(Integer.class, new Integer(aUpperLeft.X));
                    xPropSet.setPropertyValue("HoriOrientPosition", aAny );

                    // retrieve the chart document as model of the OLE shape
                    aResult = (XChartDocument) UnoRuntime.queryInterface(
                            XChartDocument.class,
                            xPropSet.getPropertyValue( "Model" ));

                    // create a diagram via the factory and set this as
                    // new diagram
                    aResult.setDiagram(
                        (XDiagram) UnoRuntime.queryInterface(
                            XDiagram.class,
                            ((XMultiServiceFactory) UnoRuntime.queryInterface(
                                XMultiServiceFactory.class,
                                aResult )).createInstance(sChartServiceName )));
                }
            } catch( Exception ex)
            {
                System.out.println( "caught exception: " + ex );
            }
        }

        return aResult;
    }

    public XChartDocument insertOLEChartInDraw(
        String sChartName,
        Point  aUpperLeft,
        Size   aExtent,
        String sChartServiceName )
    {
        XChartDocument aResult = null;

        XShapes aPage = null;

        // try interface for multiple pages in a document
        XDrawPagesSupplier aSupplier = (XDrawPagesSupplier)
            UnoRuntime.queryInterface(XDrawPagesSupplier.class,
                                      maContainerDocument );

        if( aSupplier != null )
        {
            try
            {
                // get first page
                aPage = (XShapes) UnoRuntime.queryInterface(
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
            XDrawPageSupplier aOnePageSupplier = (XDrawPageSupplier)
                UnoRuntime.queryInterface(XDrawPageSupplier.class,
                                          maContainerDocument );

            if( aOnePageSupplier != null )
            {
                aPage = (XShapes) UnoRuntime.queryInterface(
                    XShapes.class, aOnePageSupplier.getDrawPage());
            }
        }

        if( aPage != null )
        {
            XMultiServiceFactory aFact = (XMultiServiceFactory)
                UnoRuntime.queryInterface(XMultiServiceFactory.class,
                                          maContainerDocument );

            if( aFact != null )
            {
                try
                {
                    // create an OLE shape
                    XShape aShape = (XShape) UnoRuntime.queryInterface(
                        XShape.class,
                        aFact.createInstance( "com.sun.star.drawing.OLE2Shape" ));

                    // insert the shape into the page
                    aPage.add( aShape );
                    aShape.setPosition( aUpperLeft );
                    aShape.setSize( aExtent );

                    // make the OLE shape a chart
                    XPropertySet aShapeProp = (XPropertySet)
                        UnoRuntime.queryInterface(XPropertySet.class, aShape );
                    if( aShapeProp != null )
                    {
                        // set the class id for charts
                        aShapeProp.setPropertyValue( "CLSID", msChartClassID );

                        // retrieve the chart document as model of the OLE shape
                        aResult = (XChartDocument) UnoRuntime.queryInterface(
                            XChartDocument.class,
                            aShapeProp.getPropertyValue( "Model" ));

                        // create a diagram via the factory and set this as
                        // new diagram
                        aResult.setDiagram(
                            (XDiagram) UnoRuntime.queryInterface(
                                XDiagram.class,
                                ((XMultiServiceFactory) UnoRuntime.queryInterface(
                                    XMultiServiceFactory.class,
                                    aResult )).createInstance(sChartServiceName )));
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

    private final String  msChartClassID   = "12dcae26-281f-416f-a234-c3086127382e";

    private XModel maContainerDocument;
}
