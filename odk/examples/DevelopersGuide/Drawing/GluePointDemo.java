/*************************************************************************
 *
 *  $RCSfile: GluePointDemo.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-06-30 15:23:25 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright (c) 2003 by Sun Microsystems, Inc.
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

// base classes
import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.*;

// property access
import com.sun.star.beans.*;

// name access
import com.sun.star.container.*;

// text
import com.sun.star.text.*;
import com.sun.star.style.*;


// application specific classes
import com.sun.star.drawing.*;

// presentation specific classes
import com.sun.star.presentation.*;

// Point, Size, ..
import com.sun.star.awt.*;
import java.io.File;


// __________ Implementation __________

/** GluePointDemo
    @author Sven Jacobi
 */

public class GluePointDemo
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
                "private:factory/sdraw", "_blank", 0, pPropValues );


            XDrawPage xPage = PageHelper.getDrawPageByIndex( xDrawDoc, 0 );
            XShapes xShapes = (XShapes)
                    UnoRuntime.queryInterface( XShapes.class, xPage );

            // create two rectangles
            XShape xShape1 = ShapeHelper.createShape( xDrawDoc,
                new Point( 15000, 1000 ), new Size( 5000, 5000 ),
                    "com.sun.star.drawing.RectangleShape" );

            XShape xShape2 = ShapeHelper.createShape( xDrawDoc,
                new Point( 2000, 15000 ), new Size( 5000, 5000 ),
                    "com.sun.star.drawing.EllipseShape" );

            // and a connector
            XShape xConnector = ShapeHelper.createShape( xDrawDoc,
                new Point( 0, 0 ),
                    new Size( 0, 0 ),
                        "com.sun.star.drawing.ConnectorShape" );

            xShapes.add( xShape1 );
            xShapes.add( xShape2 );
            xShapes.add( xConnector );

            XPropertySet xConnectorPropSet = (XPropertySet)
                UnoRuntime.queryInterface( XPropertySet.class, xConnector );

//          Index value of 0 : the shape is connected at the top
//          Index value of 1 : the shape is connected at the left
//          Index value of 2 : the shape is connected at the bottom
//          Index value of 3 : the shape is connected at the right

            int nStartIndex = 3;
            int nEndIndex   = 1;

            // the "StartPosition" or "EndPosition" property needs not to be set
            // if there is a shape to connect
            xConnectorPropSet.setPropertyValue( "StartShape", xShape1 );
            xConnectorPropSet.setPropertyValue( "StartGluePointIndex", new Integer( nStartIndex ) );

            xConnectorPropSet.setPropertyValue( "EndShape", xShape2 );
            xConnectorPropSet.setPropertyValue( "EndGluePointIndex", new Integer( nEndIndex ) );





            XGluePointsSupplier  xGluePointsSupplier;
            XIndexContainer      xIndexContainer;
            XIdentifierContainer xIdentifierContainer;

            GluePoint2 aGluePoint = new GluePoint2();
            aGluePoint.IsRelative = false;
            aGluePoint.PositionAlignment = Alignment.CENTER;
            aGluePoint.Escape = EscapeDirection.SMART;
            aGluePoint.IsUserDefined = true;
            aGluePoint.Position.X = 0;
            aGluePoint.Position.Y = 0;

            // create and insert a glue point at shape1
            xGluePointsSupplier = (XGluePointsSupplier)
                UnoRuntime.queryInterface( XGluePointsSupplier.class, xShape1 );
            xIndexContainer = xGluePointsSupplier.getGluePoints();
            xIdentifierContainer = (XIdentifierContainer)
                UnoRuntime.queryInterface( XIdentifierContainer.class, xIndexContainer );
            int nIndexOfGluePoint1 = xIdentifierContainer.insert( aGluePoint );

            // create and insert a glue point at shape2
            xGluePointsSupplier = (XGluePointsSupplier)
                UnoRuntime.queryInterface( XGluePointsSupplier.class, xShape2 );
            xIndexContainer = xGluePointsSupplier.getGluePoints();
            xIdentifierContainer = (XIdentifierContainer)
                UnoRuntime.queryInterface( XIdentifierContainer.class, xIndexContainer );
            int nIndexOfGluePoint2 = xIdentifierContainer.insert( aGluePoint );

            // create and add a connector
            XShape xConnector2 = ShapeHelper.createShape( xDrawDoc,
                new Point( 0, 0 ),
                    new Size( 0, 0 ),
                        "com.sun.star.drawing.ConnectorShape" );
            xShapes.add( xConnector2 );

            XPropertySet xConnector2PropSet = (XPropertySet)
                UnoRuntime.queryInterface( XPropertySet.class, xConnector2 );

            xConnector2PropSet.setPropertyValue( "StartShape", xShape1 );
            xConnector2PropSet.setPropertyValue( "StartGluePointIndex",
                new Integer( nIndexOfGluePoint1 ) );

            xConnector2PropSet.setPropertyValue( "EndShape", xShape2 );
            xConnector2PropSet.setPropertyValue( "EndGluePointIndex",
                new Integer( nIndexOfGluePoint2 ) );


        }
        catch( Exception ex )
        {
            System.out.println( ex );
        }
        System.exit( 0 );
    }
}
