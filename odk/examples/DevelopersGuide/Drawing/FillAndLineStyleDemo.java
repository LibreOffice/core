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

import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XComponent;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;

import com.sun.star.drawing.LineDash;
import com.sun.star.drawing.XShape;
import com.sun.star.drawing.XShapes;
import com.sun.star.drawing.XDrawPage;

import com.sun.star.awt.Gradient;
import com.sun.star.awt.GradientStyle;
import com.sun.star.awt.Point;
import com.sun.star.awt.Size;


// __________ Implementation __________

/** FillStyle and LineStyle demo
    @author Sven Jacobi
 */

public class FillAndLineStyleDemo
{
    public static void main( String args[] )
    {
        XComponent xDrawDoc = null;
        try
        {
            // get the remote office context of a running office (a new office
            // instance is started if necessary)
            com.sun.star.uno.XComponentContext xOfficeContext = Helper.connect();

            // suppress Presentation Autopilot when opening the document
            // properties are the same as described for
            // com.sun.star.document.MediaDescriptor
            PropertyValue[] pPropValues = new PropertyValue[ 1 ];
            pPropValues[ 0 ] = new PropertyValue();
            pPropValues[ 0 ].Name = "Silent";
            pPropValues[ 0 ].Value = new Boolean( true );

            xDrawDoc = Helper.createDocument( xOfficeContext,
                "private:factory/sdraw", "_blank", 0, pPropValues );

            XDrawPage xPage = PageHelper.getDrawPageByIndex( xDrawDoc, 0 );

            XShape xRectangle = ShapeHelper.createShape( xDrawDoc,
                new Point( 0, 0 ),
                    new Size( 15000, 12000 ),
                        "com.sun.star.drawing.RectangleShape" );

            XShapes xShapes = (XShapes)
                    UnoRuntime.queryInterface( XShapes.class, xPage );
            xShapes.add( xRectangle );

            XPropertySet xPropSet = (XPropertySet)
                UnoRuntime.queryInterface( XPropertySet.class, xRectangle );

            /* apply a gradient fill style that goes from top left to bottom
               right and is changing its color from green to yellow */
            xPropSet.setPropertyValue( "FillStyle",
                                       com.sun.star.drawing.FillStyle.GRADIENT );
            Gradient aGradient = new Gradient();
            aGradient.Style = GradientStyle.LINEAR;
            aGradient.StartColor = 0x00ff00;
            aGradient.EndColor = 0xffff00;
            aGradient.Angle = 450;
            aGradient.Border = 0;
            aGradient.XOffset = 0;
            aGradient.YOffset = 0;
            aGradient.StartIntensity = 100;
            aGradient.EndIntensity = 100;
            aGradient.StepCount = 10;
            xPropSet.setPropertyValue( "FillGradient", aGradient );

            /* create a blue line with dashes and dots */
            xPropSet.setPropertyValue( "LineStyle",
                                       com.sun.star.drawing.LineStyle.DASH );
            LineDash aLineDash = new LineDash();
            aLineDash.Dots = 3;
            aLineDash.DotLen = 150;
            aLineDash.Dashes = 3;
            aLineDash.DashLen = 300;
            aLineDash.Distance = 150;
            xPropSet.setPropertyValue( "LineDash", aLineDash );
            xPropSet.setPropertyValue( "LineColor", new Integer( 0x0000ff ) );
            xPropSet.setPropertyValue( "LineWidth", new Integer( 200 ) );

        }
        catch( Exception ex )
        {
            System.out.println( ex );
        }
        System.exit( 0 );
    }
}
