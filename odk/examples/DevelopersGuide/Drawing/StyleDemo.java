/*************************************************************************
 *
 *  $RCSfile: StyleDemo.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-06-10 10:22:44 $
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

// size, point
import com.sun.star.awt.*;

// XModel
import com.sun.star.frame.*;



// __________ Implementation __________

/** StyleDemo
    @author Sven Jacobi
 */

public class StyleDemo
{
    public static void main( String args[] )
    {
        XComponent xComponent = null;
        try
        {
            String sConnection;
            if ( args.length >= 1 )
                sConnection = args[ 0 ];
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

            xComponent = Helper.createDocument( xServiceFactory,
                "private:factory/simpress", "_blank", 0, pPropValues );




            /* The first part of this demo will set each "CharColor" Property
               that is available within the styles of the document to red. It
               will also print each family and style name to the standard output */
            XModel xModel =
                (XModel)UnoRuntime.queryInterface(
                    XModel.class, xComponent );
            com.sun.star.style.XStyleFamiliesSupplier xSFS = (com.sun.star.style.XStyleFamiliesSupplier)
                UnoRuntime.queryInterface( com.sun.star.style.XStyleFamiliesSupplier.class, xModel );
            com.sun.star.container.XNameAccess xFamilies = xSFS.getStyleFamilies();

            // the element should now contain at least two Styles. The first is
            // "graphics" and the other one is the name of the Master page
            String[] Families = xFamilies.getElementNames();
            for ( int i = 0; i < Families.length; i++ )
            {
                // this is the family
                System.out.println( "\n" + Families[ i ] );

                // and now all available styles
                Object aFamilyObj = xFamilies.getByName( Families[ i ] );
                com.sun.star.container.XNameAccess xStyles = (com.sun.star.container.XNameAccess)
                    UnoRuntime.queryInterface( com.sun.star.container.XNameAccess.class, aFamilyObj );
                String[] Styles = xStyles.getElementNames();
                for( int j = 0; j < Styles.length; j++ )
                {
                    System.out.println( "   " + Styles[ j ] );
                    Object aStyleObj = xStyles.getByName( Styles[ j ] );
                    com.sun.star.style.XStyle xStyle = (com.sun.star.style.XStyle)
                        UnoRuntime.queryInterface( com.sun.star.style.XStyle.class, aStyleObj );
                    // now we have the XStyle Interface and the CharColor for all styles
                    // is exemplary be set to red.
                    XPropertySet xStylePropSet = (XPropertySet)
                        UnoRuntime.queryInterface( XPropertySet.class, xStyle );
                    XPropertySetInfo xStylePropSetInfo = xStylePropSet.getPropertySetInfo();
                    if ( xStylePropSetInfo.hasPropertyByName( "CharColor" ) )
                    {
                        xStylePropSet.setPropertyValue( "CharColor", new Integer( 0xff0000 ) );
                    }
                }
            }



            /* now create a rectangle and apply the "title1" style of
               the "graphics" family */

            Object obj = xFamilies.getByName( "graphics" );
            com.sun.star.container.XNameAccess xStyles = (XNameAccess)
                UnoRuntime.queryInterface( com.sun.star.container.XNameAccess.class, obj );
            obj = xStyles.getByName( "title1" );
            com.sun.star.style.XStyle xTitle1Style = (com.sun.star.style.XStyle)
                UnoRuntime.queryInterface( com.sun.star.style.XStyle.class, obj );

            XDrawPagesSupplier xDrawPagesSupplier =
                (XDrawPagesSupplier)UnoRuntime.queryInterface(
                    XDrawPagesSupplier.class, xComponent );
            XDrawPages xDrawPages = xDrawPagesSupplier.getDrawPages();
            XDrawPage xDrawPage = (XDrawPage)UnoRuntime.queryInterface(
                XDrawPage.class, xDrawPages.getByIndex( 0 ));
            XShapes xShapes = (XShapes)UnoRuntime.queryInterface( XShapes.class, xDrawPage );
            XShape xShape = ShapeHelper.createShape( xComponent, new Point( 0, 0 ),
                new Size( 5000, 5000 ), "com.sun.star.drawing.RectangleShape" );
            xShapes.add( xShape );
            XPropertySet xPropSet = (XPropertySet)
                UnoRuntime.queryInterface( XPropertySet.class, xShape );
            xPropSet.setPropertyValue( "Style", xTitle1Style );

        }
        catch( Exception ex )
        {
            System.out.println( ex );
        }
        System.exit( 0 );
    }
}
