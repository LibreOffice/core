/*************************************************************************
 *
 *  $RCSfile: TextDemo.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-06-10 10:22:55 $
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
            ShapeHelper.setPropertyForLastParagraph( xRectangle, "ParaLineSpacing", aLineSpacing );

            // second paragraph
            xTextPropSet = ShapeHelper.addPortion( xRectangle, "Portion3", true );
            xTextPropSet.setPropertyValue( "CharColor", new Integer( 0xff ) );
            aLineSpacing.Height = 200;
            ShapeHelper.setPropertyForLastParagraph( xRectangle, "ParaLineSpacing", aLineSpacing );



            // second shape
            xRectangle = ShapeHelper.createShape( xDrawDoc,
                new Point( 0, 10000 ),
                    new Size( 21000, 12500 ),
                        "com.sun.star.drawing.RectangleShape" );
            xShapes.add( xRectangle );
            xShapePropSet = (XPropertySet)
                    UnoRuntime.queryInterface( XPropertySet.class, xRectangle );
            xShapePropSet.setPropertyValue( "TextFitToSize", TextFitToSizeType.PROPORTIONAL );
            xShapePropSet.setPropertyValue( "TextLeftDistance",  new Integer( 2500 ) );
            xShapePropSet.setPropertyValue( "TextRightDistance", new Integer( 2500 ) );
            xShapePropSet.setPropertyValue( "TextUpperDistance", new Integer( 2500 ) );
            xShapePropSet.setPropertyValue( "TextLowerDistance", new Integer( 2500 ) );
            xTextPropSet = ShapeHelper.addPortion( xRectangle, "using TextFitToSize", false );
            xTextPropSet.setPropertyValue( "ParaAdjust", ParagraphAdjust.CENTER );
            xTextPropSet.setPropertyValue( "CharColor",  new Integer( 0xff00 ) );
            xTextPropSet = ShapeHelper.addPortion( xRectangle, "and a Border distance of 2,5 cm", true );
            xTextPropSet.setPropertyValue( "CharColor",  new Integer( 0xff0000 ) );

        }
        catch( Exception ex )
        {
            System.out.println( ex );
        }
        System.exit( 0 );
    }
}
