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



// __________ Implementation __________

/** ChangeOrderDemo
    @author Sven Jacobi
 */

public class ChangeOrderDemo
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

            XShape xShape1 = ShapeHelper.createShape( xDrawDoc,
                new Point( 1000, 1000 ), new Size( 5000, 5000 ),
                    "com.sun.star.drawing.RectangleShape" );

            XShape xShape2 = ShapeHelper.createShape( xDrawDoc,
                new Point( 2000, 2000 ), new Size( 5000, 5000 ),
                    "com.sun.star.drawing.EllipseShape" );

            xShapes.add( xShape1 );
            ShapeHelper.addPortion( xShape1, "     this shape was inserted first", false );
            ShapeHelper.addPortion( xShape1, "by changing the ZOrder it lie now on top", true );
            xShapes.add( xShape2 );

            XPropertySet xPropSet1 = (XPropertySet)
                    UnoRuntime.queryInterface( XPropertySet.class, xShape1 );
            XPropertySet xPropSet2 = (XPropertySet)
                    UnoRuntime.queryInterface( XPropertySet.class, xShape2 );

            int nOrderOfShape1 = ((Integer)xPropSet1.getPropertyValue( "ZOrder" )).intValue();
            int nOrderOfShape2 = ((Integer)xPropSet2.getPropertyValue( "ZOrder" )).intValue();

            xPropSet1.setPropertyValue( "ZOrder", new Integer( nOrderOfShape2 ) );
            xPropSet2.setPropertyValue( "ZOrder", new Integer( nOrderOfShape1 ) );
        }
        catch( Exception ex )
        {
            System.out.println( ex );
        }
        System.exit( 0 );
    }
}
