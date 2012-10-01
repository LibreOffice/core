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

// ObjectTransformationDemo

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
