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
import com.sun.star.beans.XPropertySetInfo;

import com.sun.star.container.XNameAccess;

import com.sun.star.drawing.XShape;
import com.sun.star.drawing.XShapes;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPages;
import com.sun.star.drawing.XDrawPagesSupplier;

import com.sun.star.frame.XModel;



// __________ Implementation __________

// StyleDemo

public class StyleDemo
{
    public static void main( String args[] )
    {
        XComponent xComponent = null;
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

            xComponent = Helper.createDocument( xOfficeContext,
                "private:factory/simpress", "_blank", 0, pPropValues );




            /* The first part of this demo will set each "CharColor" Property
               that is available within the styles of the document to red. It
               will also print each family and style name to the standard output */
            XModel xModel =
                (XModel)UnoRuntime.queryInterface(
                    XModel.class, xComponent );
            com.sun.star.style.XStyleFamiliesSupplier xSFS =
                (com.sun.star.style.XStyleFamiliesSupplier)
                UnoRuntime.queryInterface(
                    com.sun.star.style.XStyleFamiliesSupplier.class, xModel );

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
                com.sun.star.container.XNameAccess xStyles =
                    (com.sun.star.container.XNameAccess)
                    UnoRuntime.queryInterface(
                        com.sun.star.container.XNameAccess.class, aFamilyObj );
                String[] Styles = xStyles.getElementNames();
                for( int j = 0; j < Styles.length; j++ )
                {
                    System.out.println( "   " + Styles[ j ] );
                    Object aStyleObj = xStyles.getByName( Styles[ j ] );
                    com.sun.star.style.XStyle xStyle = (com.sun.star.style.XStyle)
                        UnoRuntime.queryInterface(
                            com.sun.star.style.XStyle.class, aStyleObj );
                    // now we have the XStyle Interface and the CharColor for
                    // all styles is exemplary be set to red.
                    XPropertySet xStylePropSet = (XPropertySet)
                        UnoRuntime.queryInterface( XPropertySet.class, xStyle );
                    XPropertySetInfo xStylePropSetInfo =
                        xStylePropSet.getPropertySetInfo();
                    if ( xStylePropSetInfo.hasPropertyByName( "CharColor" ) )
                    {
                        xStylePropSet.setPropertyValue( "CharColor",
                                                        new Integer( 0xff0000 ) );
                    }
                }
            }



            /* now create a rectangle and apply the "title1" style of
               the "graphics" family */

            Object obj = xFamilies.getByName( "graphics" );
            com.sun.star.container.XNameAccess xStyles = (XNameAccess)
                UnoRuntime.queryInterface(com.sun.star.container.XNameAccess.class,
                                          obj );
            obj = xStyles.getByName( "title1" );
            com.sun.star.style.XStyle xTitle1Style = (com.sun.star.style.XStyle)
                UnoRuntime.queryInterface( com.sun.star.style.XStyle.class, obj );

            XDrawPagesSupplier xDrawPagesSupplier =
                (XDrawPagesSupplier)UnoRuntime.queryInterface(
                    XDrawPagesSupplier.class, xComponent );
            XDrawPages xDrawPages = xDrawPagesSupplier.getDrawPages();
            XDrawPage xDrawPage = (XDrawPage)UnoRuntime.queryInterface(
                XDrawPage.class, xDrawPages.getByIndex( 0 ));
            XShapes xShapes = (XShapes)UnoRuntime.queryInterface(XShapes.class,
                                                                 xDrawPage );
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
