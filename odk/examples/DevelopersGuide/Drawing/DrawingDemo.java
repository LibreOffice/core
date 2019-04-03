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

import com.sun.star.awt.Point;
import com.sun.star.awt.Size;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;

import com.sun.star.container.XNamed;

import com.sun.star.drawing.PolygonFlags;
import com.sun.star.drawing.PolyPolygonBezierCoords;
import com.sun.star.drawing.XShape;
import com.sun.star.drawing.XShapes;
import com.sun.star.drawing.XShapeGrouper;
import com.sun.star.drawing.XDrawPage;

import java.util.Random;


// __________ Implementation __________

// drawing demo

// This drawing demo will create/load a document, and show how to
// handle pages and shapes using the Office API,

// Calling this demo two parameter can be used. The first parameter
// describes if a document is to create or load: "draw" creates a
// draw document, "impress" creates an impress document, any other
// parameter is interpreted as URL and loads the corresponding
// document. ( example for a URL is: "file:///c:/test.odp" )
// The second parameter is the connection that is to use. If no parameter
// is given a standard impress document is created by using following
// connection: "uno:socket,host=localhost,port=2083;urp;StarOffice.ServiceManager";

public class DrawingDemo
{
    public static void main( String args[] )
    {
        XComponent xDrawDoc = null;
        try
        {
            // get the remote office context of a running office (a new office
            // instance is started if necessary)
            com.sun.star.uno.XComponentContext xOfficeContext = Helper.connect();

            String sURL;
            if ( args.length == 0 )
                sURL = "impress";
            else
                sURL = args[ 0 ];

            if ( sURL.equals( "draw" ) )
                sURL = "private:factory/sdraw";
            else if ( sURL.equals( "impress" ) )
                sURL = "private:factory/simpress";

            // suppress Presentation Autopilot when opening the document
            // properties are the same as described for
            // com.sun.star.document.MediaDescriptor
            PropertyValue[] pPropValues = new PropertyValue[ 1 ];
            pPropValues[ 0 ] = new PropertyValue();
            pPropValues[ 0 ].Name = "Silent";
            pPropValues[ 0 ].Value = Boolean.TRUE;

            xDrawDoc = Helper.createDocument( xOfficeContext,
                sURL, "_blank", 0, pPropValues );
        }
        catch( Exception ex )
        {
            System.out.println( ex );
            System.exit( 0 );
        }


        Demo_PageCreation( xDrawDoc, 10 );
        Demo_PageNaming( xDrawDoc, "this page is called: LastPage" );
        Demo_ShapeCreation( xDrawDoc );
        Demo_PolyPolygonBezier( xDrawDoc );
        Demo_Group1( xDrawDoc );
        Demo_Group2( xDrawDoc );
        System.exit( 0 );
    }

    // This method appends draw pages to the document, so that a
    // minimum of n draw pages will be available.
    // For each second draw page also a new master page is created.
    public static void Demo_PageCreation( XComponent xDrawDoc, int n )
    {
        try
        {
            // If the document has less than n draw pages, append them,
            // a minimum of n draw pages will be available
            int i, nDrawPages;
            for ( nDrawPages = PageHelper.getDrawPageCount( xDrawDoc );
                nDrawPages < n; nDrawPages++ )
                PageHelper.insertNewDrawPageByIndex( xDrawDoc, nDrawPages );
            // Create a master page for each second drawpage
            int nMasterPages;
            for ( nMasterPages = PageHelper.getMasterPageCount( xDrawDoc );
                nMasterPages < ( ( nDrawPages + 1 ) / 2 ); nMasterPages++ )
                PageHelper.insertNewMasterPageByIndex( xDrawDoc, nMasterPages );

            // Now connect master page 1 to draw page 1 and 2,
            // master page 2 to draw page 3 and 4 and so on.
            for ( i = 0; i < nDrawPages; i++ )
            {
                XDrawPage xDrawPage = PageHelper.getDrawPageByIndex( xDrawDoc, i );
                XDrawPage xMasterPage = PageHelper.getMasterPageByIndex(
                    xDrawDoc, i / 2 );
                PageHelper.setMasterPage( xDrawPage, xMasterPage );
            }
        }
        catch( Exception ex )
        {
            System.out.println("Demo_PageCreation: I have a page creation problem");
        }
    }

    // this method shows how to name a page, this is exemplary
    // be done for the last draw page
    public static void Demo_PageNaming(
        XComponent xDrawDoc, String sLastPageName )
    {
        try
        {
            XDrawPage xLastPage = PageHelper.getDrawPageByIndex( xDrawDoc,
                PageHelper.getDrawPageCount( xDrawDoc ) - 1 );

            // each drawpage is supporting an XNamed interface
            XNamed xNamed = UnoRuntime.queryInterface(
                XNamed.class, xLastPage );

            // beware, the page must have a unique name
            xNamed.setName( sLastPageName );
        }
        catch( Exception ex )
        {
            System.out.println( "Demo_PageNaming: can't set page name" );
        }
    }

    // This method will add one rectangle shape into the lower left quarter of
    // every page that is available,
    public static void Demo_ShapeCreation( XComponent xDrawDoc )
    {
        try
        {
            boolean bIsImpressDocument = PageHelper.isImpressDocument( xDrawDoc );

            int nDrawingPages = PageHelper.getDrawPageCount( xDrawDoc );
            int nMasterPages =  PageHelper.getMasterPageCount( xDrawDoc );
            int nGlobalPageCount = nDrawingPages + nMasterPages;

            if ( bIsImpressDocument )
            {
                // in impress each draw page also has a notes page
                nGlobalPageCount += nDrawingPages;
                // for each drawing master is also a notes master available
                nGlobalPageCount += nMasterPages;
                // one handout is existing
                nGlobalPageCount += 1;
            }

            // create and fill a container with all draw pages
            XDrawPage[] pPages = new XDrawPage[ nGlobalPageCount ];
            int i, nCurrentPageIndex = 0;

            // insert handout page
            if ( bIsImpressDocument )
                pPages[ nCurrentPageIndex++ ] = PageHelper.getHandoutMasterPage(
                    xDrawDoc );

            // inserting all master pages
            for( i = 0; i < nMasterPages; i++ )
            {
                XDrawPage xMasterPage = PageHelper.getMasterPageByIndex(
                    xDrawDoc, i );
                pPages[ nCurrentPageIndex++ ] = xMasterPage;

                // if the document is an impress, get the corresponding notes
                // master page
                if ( bIsImpressDocument )
                    pPages[ nCurrentPageIndex++ ] = PageHelper.getNotesPage(
                        xMasterPage );
            }
            for ( i = 0; i < nDrawingPages; i++ )
            {
                XDrawPage xDrawPage = PageHelper.getDrawPageByIndex( xDrawDoc, i );
                pPages[ nCurrentPageIndex++ ] = xDrawPage;

                // if the document is an impress, get the corresponding notes page
                if ( bIsImpressDocument )
                    pPages[ nCurrentPageIndex++ ] = PageHelper.getNotesPage(
                        xDrawPage );
            }

            // Now a complete list of pages is available in pPages.
            // The following code will insert a rectangle into each page.
            for ( i = 0; i < nGlobalPageCount; i++ )
            {
                Size aPageSize = PageHelper.getPageSize( pPages[ i ] );
                int nHalfWidth  = aPageSize.Width / 2;
                int nHalfHeight = aPageSize.Height / 2;

                Random aRndGen = new Random();
                int nRndObjWidth  = aRndGen.nextInt( nHalfWidth );
                int nRndObjHeight = aRndGen.nextInt( nHalfHeight );

                int nRndObjPosX = aRndGen.nextInt( nHalfWidth - nRndObjWidth );
                int nRndObjPosY = aRndGen.nextInt( nHalfHeight - nRndObjHeight )
                    + nHalfHeight;

                XShapes xShapes = UnoRuntime.queryInterface( XShapes.class, pPages[ i ] );
                ShapeHelper.createAndInsertShape( xDrawDoc, xShapes,
                    new Point( nRndObjPosX, nRndObjPosY ),
                        new Size( nRndObjWidth, nRndObjHeight ),
                            "com.sun.star.drawing.RectangleShape" );
            }
        }
        catch( Exception ex )
        {
            System.out.println( "Demo_ShapeCreation:" + ex );
        }
    }

    // This method will show how to create a PolyPolygonBezier that lies is in the
    // topleft quarter of the page and positioned at the back
    public static void Demo_PolyPolygonBezier( XComponent xDrawDoc )
    {
        try
        {
            XShape xPolyPolygonBezier = ShapeHelper.createShape( xDrawDoc,
                new Point( 0, 0 ),
                    new Size( 0, 0 ),
                        "com.sun.star.drawing.ClosedBezierShape" );

            // the fact that the shape must have been added to the page before
            // it is possible to apply changes to the PropertySet, it is a good
            // proceeding to add the shape as soon as possible
            XDrawPage xDrawPage;
            // if possible insert our new shape in the master page
            if ( PageHelper.isImpressDocument( xDrawDoc ) )
                xDrawPage = PageHelper.getMasterPageByIndex( xDrawDoc, 0 );
            else
                xDrawPage = PageHelper.getDrawPageByIndex( xDrawDoc, 0 );
            XShapes xShapes = UnoRuntime.queryInterface( XShapes.class, xDrawPage );
            xShapes.add( xPolyPolygonBezier );

            XPropertySet xShapeProperties = UnoRuntime.queryInterface( XPropertySet.class, xPolyPolygonBezier );

            // get pagesize
            XPropertySet xPageProperties = UnoRuntime.queryInterface( XPropertySet.class, xDrawPage );
            int nPageWidth = ((Integer)xPageProperties.getPropertyValue( "Width" )).intValue() / 2;
            int nPageHeight = ((Integer)xPageProperties.getPropertyValue( "Height" )).intValue() / 2;

            PolyPolygonBezierCoords aCoords = new PolyPolygonBezierCoords();
            // allocating the outer sequence
            int nPolygonCount = 50;
            aCoords.Coordinates = new Point[ nPolygonCount ][ ];
            aCoords.Flags = new PolygonFlags[ nPolygonCount ][ ];
            int i, n, nY;
            // fill the inner point sequence now
            for ( nY = 0, i = 0; i < nPolygonCount; i++, nY += nPageHeight / nPolygonCount )
            {
                // create a polygon using two normal and two control points
                // allocating the inner sequence
                int nPointCount = 8;
                Point[]         pPolyPoints = new Point[ nPointCount ];
                PolygonFlags[]  pPolyFlags  = new PolygonFlags[ nPointCount ];

                for ( n = 0; n < nPointCount; n++ )
                    pPolyPoints[ n ] = new Point();

                pPolyPoints[ 0 ].X = 0;
                pPolyPoints[ 0 ].Y = nY;
                pPolyFlags[ 0 ] = PolygonFlags.NORMAL;
                pPolyPoints[ 1 ].X = nPageWidth / 2;
                pPolyPoints[ 1 ].Y = nPageHeight;
                pPolyFlags[ 1 ] = PolygonFlags.CONTROL;
                pPolyPoints[ 2 ].X = nPageWidth / 2;
                pPolyPoints[ 2 ].Y = nPageHeight;
                pPolyFlags[ 2 ] = PolygonFlags.CONTROL;
                pPolyPoints[ 3 ].X = nPageWidth;
                pPolyPoints[ 3 ].Y = nY;
                pPolyFlags[ 3 ] = PolygonFlags.NORMAL;

                pPolyPoints[ 4 ].X = nPageWidth - 1000;
                pPolyPoints[ 4 ].Y = nY;
                pPolyFlags[ 4 ] = PolygonFlags.NORMAL;
                pPolyPoints[ 5 ].X = nPageWidth / 2;
                pPolyPoints[ 5 ].Y = nPageHeight / 2;
                pPolyFlags[ 5 ] = PolygonFlags.CONTROL;
                pPolyPoints[ 6 ].X = nPageWidth / 2;
                pPolyPoints[ 6 ].Y = nPageHeight / 2;
                pPolyFlags[ 6 ] = PolygonFlags.CONTROL;
                pPolyPoints[ 7 ].X = 1000;
                pPolyPoints[ 7 ].Y = nY;
                pPolyFlags[ 7 ] = PolygonFlags.NORMAL;

                aCoords.Coordinates[ i ]= pPolyPoints;
                aCoords.Flags[ i ]      = pPolyFlags;
            }
            xShapeProperties.setPropertyValue( "PolyPolygonBezier", aCoords );

            // move the shape to the back by changing the ZOrder
            xShapeProperties.setPropertyValue( "ZOrder", Integer.valueOf( 1 ) );
        }
        catch ( Exception ex )
        {
            System.out.println( "Demo_PolyPolygonBezier:" + ex );
        }
    }

    // This method will create a group containing two ellipses
    // the shapes will be added into the top right corner of the first
    // draw page
    public static void Demo_Group1( XComponent xDrawDoc )
    {
        try
        {
            XShape xGroup = ShapeHelper.createShape( xDrawDoc,
                new Point( 0, 0 ),
                    new Size( 0, 0 ),
                        "com.sun.star.drawing.GroupShape" );

            // before it is possible to insert shapes,
            // the group must have been added to the page
            XDrawPage xDrawPage = PageHelper.getDrawPageByIndex( xDrawDoc, 0 );
            XShapes xShapes = UnoRuntime.queryInterface( XShapes.class, xDrawPage );
            xShapes.add( xGroup );

            XShapes xShapesGroup = UnoRuntime.queryInterface( XShapes.class, xGroup );

            Size aPageSize = PageHelper.getPageSize( xDrawPage );

            int nWidth  = 4000;
            int nHeight = 2000;
            int nPosX = ( aPageSize.Width * 3 ) / 4 - nWidth / 2;
            int nPosY1 = 2000;
            int nPosY2 = aPageSize.Height / 2 - ( nPosY1 + nHeight );
            XShape xRect1 = ShapeHelper.createShape( xDrawDoc,
                new Point( nPosX, nPosY1 ),
                    new Size( nWidth, nHeight ),
                        "com.sun.star.drawing.EllipseShape" );
            XShape xRect2 = ShapeHelper.createShape( xDrawDoc,
                new Point( nPosX, nPosY2 ),
                    new Size( nWidth, nHeight ),
                        "com.sun.star.drawing.EllipseShape" );

            xShapesGroup.add( xRect1 );
            xShapesGroup.add( xRect2 );
        }
        catch ( Exception ex )
        {
            System.out.println( "Demo_Group1:" + ex );
        }
    }

    // This method will group all available objects on the
    // first page.
    public static void Demo_Group2( XComponent xDrawDoc )
    {
        try
        {
            XDrawPage xDrawPage = PageHelper.getDrawPageByIndex( xDrawDoc, 0 );
            XShapeGrouper xShapeGrouper = UnoRuntime.queryInterface( XShapeGrouper.class, xDrawPage );

            XShapes xShapesPage = UnoRuntime.queryInterface( XShapes.class, xDrawPage );

            xShapeGrouper.group( xShapesPage );
        }
        catch ( Exception ex )
        {
            System.out.println( "Demo_Group2:" + ex );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
