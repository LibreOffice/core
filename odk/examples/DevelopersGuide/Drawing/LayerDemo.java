/*************************************************************************
 *
 *  $RCSfile: LayerDemo.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-06-10 10:21:29 $
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
            XPropertySet xTextProp = ShapeHelper.addPortion( xRect2, "this shape is locked", false );
            xTextProp.setPropertyValue( "ParaAdjust", ParagraphAdjust.CENTER );
            ShapeHelper.addPortion( xRect2, "and the shape above is not visible", true );
            ShapeHelper.addPortion( xRect2, "(switch to the layer view to gain access)", true );


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
            XLayer xNotVisibleAndEditable = xLayerManager.insertNewByIndex( xLayerManager.getCount() );
            xLayerPropSet = (XPropertySet)
                (XPropertySet)UnoRuntime.queryInterface(
                    XPropertySet.class, xNotVisibleAndEditable );
            xLayerPropSet.setPropertyValue( "Name", "NotVisibleAndEditable" );
            xLayerPropSet.setPropertyValue( "IsVisible", new Boolean( false ) );
            xLayerPropSet.setPropertyValue( "IsLocked", new Boolean( true ) );

            // create a second layer
            XLayer xNotEditable = xLayerManager.insertNewByIndex( xLayerManager.getCount() );
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
