/*************************************************************************
 *
 *  $RCSfile: FillAndLineStyleDemo.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-06-10 10:20:55 $
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
            xPropSet.setPropertyValue( "FillStyle", com.sun.star.drawing.FillStyle.GRADIENT );
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
            xPropSet.setPropertyValue( "LineStyle", com.sun.star.drawing.LineStyle.DASH );
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
