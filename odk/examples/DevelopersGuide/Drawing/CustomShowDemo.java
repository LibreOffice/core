/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// __________ Imports __________

import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XSingleServiceFactory;

import com.sun.star.awt.Point;
import com.sun.star.awt.Size;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;

import com.sun.star.container.XNamed;
import com.sun.star.container.XNameContainer;
import com.sun.star.container.XIndexContainer;

import com.sun.star.drawing.XShape;
import com.sun.star.drawing.XShapes;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPages;
import com.sun.star.drawing.XDrawPagesSupplier;

import com.sun.star.presentation.XPresentation;
import com.sun.star.presentation.XPresentationSupplier;
import com.sun.star.presentation.XCustomPresentationSupplier;


// __________ Implementation __________

/** presentation demo
    @author Sven Jacobi
 */

// This demo will demonstrate how to create a CustomShow

// The first parameter describes the connection that is to use. If there is no parameter
// "uno:socket,host=localhost,port=2083;urp;StarOffice.ServiceManager" is used.


public class CustomShowDemo
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
