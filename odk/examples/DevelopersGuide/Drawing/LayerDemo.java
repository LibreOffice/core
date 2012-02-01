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

import com.sun.star.container.XNameAccess;

import com.sun.star.style.ParagraphAdjust;

import com.sun.star.drawing.XShape;
import com.sun.star.drawing.XShapes;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XLayer;
import com.sun.star.drawing.XLayerManager;
import com.sun.star.drawing.XLayerSupplier;


// __________ Implementation __________

/** LayerDemo
    @author Sven Jacobi
 */

public class LayerDemo
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


            // create two rectangles
            XDrawPage xPage = PageHelper.getDrawPageByIndex( xDrawDoc, 0 );
            XShapes xShapes = (XShapes)
                    UnoRuntime.queryInterface( XShapes.class, xPage );

            XShape xRect1 = ShapeHelper.createShape( xDrawDoc,
                new Point( 1000, 1000 ), new Size( 5000, 5000 ),
                    "com.sun.star.drawing.RectangleShape" );

            XShape xRect2 = ShapeHelper.createShape( xDrawDoc,
                new Point( 1000, 7000 ), new Size( 5000, 5000 ),
                    "com.sun.star.drawing.RectangleShape" );

            xShapes.add( xRect1 );
            xShapes.add( xRect2 );
            XPropertySet xTextProp = ShapeHelper.addPortion( xRect2,
                                                             "this shape is locked",
                                                             false );
            xTextProp.setPropertyValue( "ParaAdjust", ParagraphAdjust.CENTER );
            ShapeHelper.addPortion( xRect2, "and the shape above is not visible",
                                    true );
            ShapeHelper.addPortion( xRect2,
                                    "(switch to the layer view to gain access)",
                                    true );


            // query for the XLayerManager
            XLayerSupplier xLayerSupplier = (XLayerSupplier)
                (XLayerSupplier)UnoRuntime.queryInterface(
                    XLayerSupplier.class, xDrawDoc );
            XNameAccess xNameAccess = xLayerSupplier.getLayerManager();
            XLayerManager xLayerManager = (XLayerManager)
                (XLayerManager)UnoRuntime.queryInterface(
                    XLayerManager.class, xNameAccess );

            // create a layer and set its properties
            XPropertySet xLayerPropSet;
            XLayer xNotVisibleAndEditable = xLayerManager.insertNewByIndex(
                xLayerManager.getCount() );

            xLayerPropSet = (XPropertySet)
                (XPropertySet)UnoRuntime.queryInterface(
                    XPropertySet.class, xNotVisibleAndEditable );
            xLayerPropSet.setPropertyValue( "Name", "NotVisibleAndEditable" );
            xLayerPropSet.setPropertyValue( "IsVisible", new Boolean( false ) );
            xLayerPropSet.setPropertyValue( "IsLocked", new Boolean( true ) );

            // create a second layer
            XLayer xNotEditable = xLayerManager.insertNewByIndex(
                xLayerManager.getCount() );

            xLayerPropSet = (XPropertySet)
                (XPropertySet)UnoRuntime.queryInterface(
                    XPropertySet.class, xNotEditable );
            xLayerPropSet.setPropertyValue( "Name", "NotEditable" );
            xLayerPropSet.setPropertyValue( "IsVisible", new Boolean( true ) );
            xLayerPropSet.setPropertyValue( "IsLocked", new Boolean( true ) );

            // attach the layer to the rectangles
            xLayerManager.attachShapeToLayer( xRect1, xNotVisibleAndEditable );
            xLayerManager.attachShapeToLayer( xRect2, xNotEditable );

        }
        catch( Exception ex )
        {
            System.out.println( ex );
        }
        System.exit( 0 );
    }
}
