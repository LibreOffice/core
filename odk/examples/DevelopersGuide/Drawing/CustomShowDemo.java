/*************************************************************************
 *
 *  $RCSfile: CustomShowDemo.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-06-10 10:20:31 $
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

// __________ Implementation __________

/** presentation demo
    @author Sven Jacobi
 */

// This demo will demonstrate how to create a CustomShow

// The first parameter describes the connection that is to use. If there is no parameter
// "uno:socket,host=localhost,port=8100;urp;StarOffice.ServiceManager" is used.


public class CustomShowDemo
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

            XDrawPagesSupplier xDrawPagesSupplier =
                (XDrawPagesSupplier)UnoRuntime.queryInterface(
                    XDrawPagesSupplier.class, xDrawDoc );
            XDrawPages xDrawPages = xDrawPagesSupplier.getDrawPages();

            // take care that this document has ten pages
            while ( xDrawPages.getCount() < 10 )
                xDrawPages.insertNewByIndex( 0 );

            // assign a name to each page and also insert a text object including the name of the page
            String aNameArray[] = { "Introduction", "page one", "page two", "page three", "page four",
                                    "page five", "page six", "page seven", "page eight", "page nine" };
            int i;
            for ( i = 0; i < 10; i++ )
            {
                XDrawPage xDrawPage = (XDrawPage)UnoRuntime.queryInterface(
                    XDrawPage.class, xDrawPages.getByIndex( i ));
                XNamed xPageName = (XNamed)UnoRuntime.queryInterface(
                    XNamed.class, xDrawPage );
                xPageName.setName( aNameArray[ i ] );

                // now we will create and insert the text object
                XShape xTextObj = ShapeHelper.createShape( xDrawDoc, new Point( 10000, 9000 ),
                    new Size( 10000, 5000 ),
                        "com.sun.star.drawing.TextShape" );
                XShapes xShapes = (XShapes)
                        UnoRuntime.queryInterface( XShapes.class, xDrawPage );
                xShapes.add( xTextObj );
                ShapeHelper.addPortion( xTextObj, aNameArray[ i ], true );
            }

            /* create two custom shows, one will play slide 6 to 10 and is named "ShortVersion"
               the other one will play slide 2 til 10 and is named "LongVersion" */
            XCustomPresentationSupplier xCustPresSupplier = (XCustomPresentationSupplier)
                UnoRuntime.queryInterface( XCustomPresentationSupplier.class, xDrawDoc );

            /* the following container is a container for further container
               which concludes the list of pages that are to play within a custom show */
            XNameContainer xNameContainer = xCustPresSupplier.getCustomPresentations();
            XSingleServiceFactory xFactory = (XSingleServiceFactory)
                UnoRuntime.queryInterface( XSingleServiceFactory.class, xNameContainer );

            Object          xObj;
            XIndexContainer xContainer;

            /* instanciate an IndexContainer that will take
               a list of draw pages for the first custom show */
            xObj = xFactory.createInstance();
            xContainer = (XIndexContainer)UnoRuntime.queryInterface( XIndexContainer.class, xObj );
            for ( i = 5; i < 10; i++ )
                xContainer.insertByIndex( xContainer.getCount(), xDrawPages.getByIndex( i ) );
            xNameContainer.insertByName( "ShortVersion", xContainer );

            /* instanciate an IndexContainer that will take
               a list of draw page for the second custom show */
            xObj = xFactory.createInstance();
            xContainer = (XIndexContainer)UnoRuntime.queryInterface( XIndexContainer.class, xObj );
            for ( i = 1; i < 10; i++ )
                xContainer.insertByIndex( xContainer.getCount(), xDrawPages.getByIndex( i ) );
            xNameContainer.insertByName( "LongVersion", xContainer );

            /* which custom show is to use
               can been set in the presentation settings */

            XPresentationSupplier xPresSupplier = (XPresentationSupplier)
                UnoRuntime.queryInterface( XPresentationSupplier.class, xDrawDoc );
            XPresentation xPresentation = xPresSupplier.getPresentation();
            XPropertySet xPresPropSet = (XPropertySet)
                UnoRuntime.queryInterface( XPropertySet.class, xPresentation );
            xPresPropSet.setPropertyValue( "CustomShow", "ShortVersion" );
        }
        catch( Exception ex )
        {
            System.out.println( ex );
        }
        System.exit( 0 );
    }
}
