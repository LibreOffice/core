/*************************************************************************
 *
 *  $RCSfile: GluePointDemo.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-06-10 10:21:05 $
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
