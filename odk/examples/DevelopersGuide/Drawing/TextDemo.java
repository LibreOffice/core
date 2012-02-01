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

import com.sun.star.awt.Point;
import com.sun.star.awt.Size;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;

import com.sun.star.drawing.XShape;
import com.sun.star.drawing.XShapes;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.TextFitToSizeType;

import com.sun.star.style.LineSpacing;
import com.sun.star.style.LineSpacingMode;
import com.sun.star.style.ParagraphAdjust;



// __________ Implementation __________

/** text demo
    @author Sven Jacobi
 */

public class TextDemo
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
            XShapes xShapes = (XShapes)
                    UnoRuntime.queryInterface( XShapes.class, xPage );


            XShape       xRectangle;
            XPropertySet xTextPropSet, xShapePropSet;
            LineSpacing  aLineSpacing = new LineSpacing();
            aLineSpacing.Mode = LineSpacingMode.PROP;



            // first shape
            xRectangle = ShapeHelper.createShape( xDrawDoc,
                new Point( 0, 0 ),
                    new Size( 15000, 7500 ),
                        "com.sun.star.drawing.RectangleShape" );
            xShapes.add( xRectangle );
            xShapePropSet = (XPropertySet)
                    UnoRuntime.queryInterface( XPropertySet.class, xRectangle );


            // first paragraph
            xTextPropSet = ShapeHelper.addPortion( xRectangle, "Portion1", false );
            xTextPropSet.setPropertyValue( "CharColor", new Integer( 0xff0000 ) );
            xTextPropSet = ShapeHelper.addPortion( xRectangle, "Portion2", false );
            xTextPropSet.setPropertyValue( "CharColor", new Integer( 0x8080ff ) );
            aLineSpacing.Height = 100;
            ShapeHelper.setPropertyForLastParagraph( xRectangle, "ParaLineSpacing",
                                                     aLineSpacing );

            // second paragraph
            xTextPropSet = ShapeHelper.addPortion( xRectangle, "Portion3", true );
            xTextPropSet.setPropertyValue( "CharColor", new Integer( 0xff ) );
            aLineSpacing.Height = 200;
            ShapeHelper.setPropertyForLastParagraph( xRectangle, "ParaLineSpacing",
                                                     aLineSpacing );



            // second shape
            xRectangle = ShapeHelper.createShape( xDrawDoc,
                new Point( 0, 10000 ),
                    new Size( 21000, 12500 ),
                        "com.sun.star.drawing.RectangleShape" );
            xShapes.add( xRectangle );
            xShapePropSet = (XPropertySet)
                    UnoRuntime.queryInterface( XPropertySet.class, xRectangle );
            xShapePropSet.setPropertyValue( "TextFitToSize",
                                            TextFitToSizeType.PROPORTIONAL );
            xShapePropSet.setPropertyValue( "TextLeftDistance",  new Integer(2500));
            xShapePropSet.setPropertyValue( "TextRightDistance", new Integer(2500));
            xShapePropSet.setPropertyValue( "TextUpperDistance", new Integer(2500));
            xShapePropSet.setPropertyValue( "TextLowerDistance", new Integer(2500));
            xTextPropSet = ShapeHelper.addPortion( xRectangle,
                                                   "using TextFitToSize", false );
            xTextPropSet.setPropertyValue( "ParaAdjust", ParagraphAdjust.CENTER );
            xTextPropSet.setPropertyValue( "CharColor",  new Integer(0xff00));
            xTextPropSet = ShapeHelper.addPortion(xRectangle,
                                                  "and a Border distance of 2,5 cm",
                                                  true );
            xTextPropSet.setPropertyValue( "CharColor",  new Integer( 0xff0000 ) );

        }
        catch( Exception ex )
        {
            System.out.println( ex );
        }
        System.exit( 0 );
    }
}
