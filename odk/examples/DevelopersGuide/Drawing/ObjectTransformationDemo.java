/*************************************************************************
 *
 *  $RCSfile: ObjectTransformationDemo.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-06-10 10:21:46 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// __________ Imports __________

// base classes
import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.*;

// property access
import com.sun.star.beans.*;

// name access
import com.sun.star.container.*;


// application specific classes
import com.sun.star.drawing.*;

// presentation specific classes
import com.sun.star.presentation.*;

// Point, Size, ..
import com.sun.star.awt.*;
import java.io.File;


//
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
            String sConnection;
            if ( args.length >= 1 )
                sConnection = args[ 1 ];
            else
                sConnection = "uno:socket,host=localhost,port=8100;urp;StarOffice.ServiceManager";
            XMultiServiceFactory xServiceFactory =
                Helper.connect( sConnection );

            // suppress Presentation Autopilot when opening the document
            // properties are the same as described for com.sun.star.document.MediaDescriptor
            PropertyValue[] pPropValues = new PropertyValue[ 1 ];
            pPropValues[ 0 ] = new PropertyValue();
            pPropValues[ 0 ].Name = "Silent";
            pPropValues[ 0 ].Value = new Boolean( true );

            xDrawDoc = Helper.createDocument( xServiceFactory,
                "private:factory/simpress", "_blank", 0, pPropValues );

            XDrawPage       xPage       = PageHelper.getDrawPageByIndex( xDrawDoc, 0 );
            XPropertySet    xPagePropSet= (XPropertySet)
                    UnoRuntime.queryInterface( XPropertySet.class, xPage );

            XShapes xShapes = (XShapes)
                    UnoRuntime.queryInterface( XShapes.class, xPage );


            XShape xShape = ShapeHelper.createShape( xDrawDoc,
                new Point( 0, 0 ), new Size( 10000, 2500 ),
                    "com.sun.star.drawing.RectangleShape" );
            xShapes.add( xShape );

            XPropertySet xPropSet = (XPropertySet)
                    UnoRuntime.queryInterface( XPropertySet.class, xShape );

            HomogenMatrix3 aHomogenMatrix3 = (HomogenMatrix3)xPropSet.getPropertyValue( "Transformation" );
            java.awt.geom.AffineTransform aOriginalMatrix = new java.awt.geom.AffineTransform(
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
