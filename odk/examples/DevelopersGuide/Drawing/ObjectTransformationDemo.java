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
import com.sun.star.drawing.HomogenMatrix3;

import java.awt.geom.AffineTransform;

// __________ Implementation __________

/** ObjectTransformationDemo
    @author Sven Jacobi
 */

public class ObjectTransformationDemo
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
                "private:factory/simpress", "_blank", 0, pPropValues );

            XDrawPage xPage = PageHelper.getDrawPageByIndex( xDrawDoc, 0 );
            XPropertySet xPagePropSet= (XPropertySet)
                    UnoRuntime.queryInterface( XPropertySet.class, xPage );

            XShapes xShapes = (XShapes)
                    UnoRuntime.queryInterface( XShapes.class, xPage );


            XShape xShape = ShapeHelper.createShape( xDrawDoc,
                new Point( 0, 0 ), new Size( 10000, 2500 ),
                    "com.sun.star.drawing.RectangleShape" );
            xShapes.add( xShape );

            XPropertySet xPropSet = (XPropertySet)
                    UnoRuntime.queryInterface( XPropertySet.class, xShape );

            HomogenMatrix3 aHomogenMatrix3 = (HomogenMatrix3)
                xPropSet.getPropertyValue( "Transformation" );

            java.awt.geom.AffineTransform aOriginalMatrix =
                new java.awt.geom.AffineTransform(
                    aHomogenMatrix3.Line1.Column1, aHomogenMatrix3.Line2.Column1,
                    aHomogenMatrix3.Line1.Column2, aHomogenMatrix3.Line2.Column2,
                    aHomogenMatrix3.Line1.Column3, aHomogenMatrix3.Line2.Column3 );


            AffineTransform aNewMatrix1 = new AffineTransform();
            aNewMatrix1.setToRotation( Math.PI /180 * 15 );
            aNewMatrix1.concatenate( aOriginalMatrix );

            AffineTransform aNewMatrix2 = new AffineTransform();
            aNewMatrix2.setToTranslation( 2000, 2000 );
            aNewMatrix2.concatenate( aNewMatrix1 );

            double aFlatMatrix[] = new double[ 6 ];
            aNewMatrix2.getMatrix( aFlatMatrix );
            aHomogenMatrix3.Line1.Column1 = aFlatMatrix[ 0 ];
            aHomogenMatrix3.Line2.Column1 = aFlatMatrix[ 1 ];
            aHomogenMatrix3.Line1.Column2 = aFlatMatrix[ 2 ];
            aHomogenMatrix3.Line2.Column2 = aFlatMatrix[ 3 ];
            aHomogenMatrix3.Line1.Column3 = aFlatMatrix[ 4 ];
            aHomogenMatrix3.Line2.Column3 = aFlatMatrix[ 5 ];
            xPropSet.setPropertyValue( "Transformation", aHomogenMatrix3 );


        }
        catch( Exception ex )
        {
            System.out.println( ex );
        }
        System.exit( 0 );
    }
}
