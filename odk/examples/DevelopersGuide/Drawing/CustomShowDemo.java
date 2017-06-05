/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// presentation demo

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
            pPropValues[ 0 ].Value = Boolean.TRUE;

            xDrawDoc = Helper.createDocument( xOfficeContext,
                    "private:factory/simpress", "_blank", 0, pPropValues );

            XDrawPagesSupplier xDrawPagesSupplier =
                UnoRuntime.queryInterface(
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
                XDrawPage xDrawPage = UnoRuntime.queryInterface(
                    XDrawPage.class, xDrawPages.getByIndex( i ));
                XNamed xPageName = UnoRuntime.queryInterface(
                    XNamed.class, xDrawPage );
                xPageName.setName( aNameArray[ i ] );

                // now we will create and insert the text object
                XShape xTextObj = ShapeHelper.createShape( xDrawDoc, new Point( 10000, 9000 ),
                    new Size( 10000, 5000 ),
                        "com.sun.star.drawing.TextShape" );
                XShapes xShapes = UnoRuntime.queryInterface( XShapes.class, xDrawPage );
                xShapes.add( xTextObj );
                ShapeHelper.addPortion( xTextObj, aNameArray[ i ], true );
            }

            /* create two custom shows, one will play slide 6 to 10 and is named "ShortVersion"
               the other one will play slide 2 til 10 and is named "LongVersion" */
            XCustomPresentationSupplier xCustPresSupplier = UnoRuntime.queryInterface( XCustomPresentationSupplier.class, xDrawDoc );

            /* the following container is a container for further container
               which concludes the list of pages that are to play within a custom show */
            XNameContainer xNameContainer = xCustPresSupplier.getCustomPresentations();
            XSingleServiceFactory xFactory = UnoRuntime.queryInterface( XSingleServiceFactory.class, xNameContainer );

            Object          xObj;
            XIndexContainer xContainer;

            /* instantiate an IndexContainer that will take
               a list of draw pages for the first custom show */
            xObj = xFactory.createInstance();
            xContainer = UnoRuntime.queryInterface( XIndexContainer.class, xObj );
            for ( i = 5; i < 10; i++ )
                xContainer.insertByIndex( xContainer.getCount(), xDrawPages.getByIndex( i ) );
            xNameContainer.insertByName( "ShortVersion", xContainer );

            /* instantiate an IndexContainer that will take
               a list of draw page for the second custom show */
            xObj = xFactory.createInstance();
            xContainer = UnoRuntime.queryInterface( XIndexContainer.class, xObj );
            for ( i = 1; i < 10; i++ )
                xContainer.insertByIndex( xContainer.getCount(), xDrawPages.getByIndex( i ) );
            xNameContainer.insertByName( "LongVersion", xContainer );

            /* which custom show is to use
               can been set in the presentation settings */

            XPresentationSupplier xPresSupplier = UnoRuntime.queryInterface( XPresentationSupplier.class, xDrawDoc );
            XPresentation xPresentation = xPresSupplier.getPresentation();
            XPropertySet xPresPropSet = UnoRuntime.queryInterface( XPropertySet.class, xPresentation );
            xPresPropSet.setPropertyValue( "CustomShow", "ShortVersion" );
        }
        catch( Exception ex )
        {
            System.out.println( ex );
        }
        System.exit( 0 );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
