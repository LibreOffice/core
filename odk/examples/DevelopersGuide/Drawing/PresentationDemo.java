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
import com.sun.star.lang.XServiceInfo;

import com.sun.star.awt.Point;
import com.sun.star.awt.Size;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;

import com.sun.star.container.XNamed;

import com.sun.star.drawing.XShape;
import com.sun.star.drawing.XShapes;
import com.sun.star.drawing.XDrawPage;

import com.sun.star.presentation.XPresentation;
import com.sun.star.presentation.XPresentationSupplier;



// __________ Implementation __________

/** presentation demo
    @author Sven Jacobi
 */

// This demo will demonstrate how to create a presentation using the Office API

// The first parameter describes the connection that is to use. If there is no parameter
// "uno:socket,host=localhost,port=2083;urp;StarOffice.ServiceManager" is used.


public class PresentationDemo
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


            XDrawPage    xPage;
            XShapes      xShapes;
            XPropertySet xShapePropSet;

            // create pages, so that three are available
            while ( PageHelper.getDrawPageCount( xDrawDoc ) < 3 )
                PageHelper.insertNewDrawPageByIndex( xDrawDoc, 0 );


            // set the slide transition for the first page
            xPage = PageHelper.getDrawPageByIndex( xDrawDoc, 0 );
            xShapes = (XShapes)
                UnoRuntime.queryInterface( XShapes.class, xPage );
            // set slide transition effect
            setSlideTransition( xPage,
                com.sun.star.presentation.FadeEffect.FADE_FROM_RIGHT,
                    com.sun.star.presentation.AnimationSpeed.FAST,
                        1, 0 ); // automatic object and slide transition

            // create a rectangle that is placed on the top left of the page
            xShapePropSet = ShapeHelper.createAndInsertShape( xDrawDoc,
                xShapes,new Point( 1000, 1000 ), new Size( 5000, 5000 ),
                    "com.sun.star.drawing.RectangleShape" );
            xShapePropSet.setPropertyValue("Effect",
                com.sun.star.presentation.AnimationEffect.WAVYLINE_FROM_BOTTOM );

            /* the following three properties provokes that the shape is dimmed
               to red
               after the animation has been finished */
            xShapePropSet.setPropertyValue( "DimHide", new Boolean( false ) );
            xShapePropSet.setPropertyValue( "DimPrevious", new Boolean( true ) );
            xShapePropSet.setPropertyValue( "DimColor", new Integer( 0xff0000 ) );


            // set the slide transition for the second page
            xPage = PageHelper.getDrawPageByIndex( xDrawDoc, 1 );
            xShapes = (XShapes)
                UnoRuntime.queryInterface( XShapes.class, xPage );
            setSlideTransition( xPage,
                com.sun.star.presentation.FadeEffect.FADE_FROM_RIGHT,
                    com.sun.star.presentation.AnimationSpeed.SLOW,
                        1, 0 ); // automatic object and slide transition

            // create an ellipse that is placed on the bottom right of second page
            xShapePropSet = ShapeHelper.createAndInsertShape( xDrawDoc,
                xShapes, new Point( 21000, 15000 ), new Size( 5000, 5000 ),
                    "com.sun.star.drawing.EllipseShape" );
            xShapePropSet.setPropertyValue(
                "Effect", com.sun.star.presentation.AnimationEffect.HIDE );


            // create two objects for the third page
            // clicking the first object lets the presentation jump
            // to page one by using ClickAction.FIRSTPAGE,
            // the second object lets the presentation jump to page two
            // by using a ClickAction.BOOKMARK;
            xPage = PageHelper.getDrawPageByIndex( xDrawDoc, 2 );
            xShapes = (XShapes)
                UnoRuntime.queryInterface( XShapes.class, xPage );
            setSlideTransition( xPage,
                com.sun.star.presentation.FadeEffect.ROLL_FROM_LEFT,
                    com.sun.star.presentation.AnimationSpeed.MEDIUM,
                        2, 0 );
            XShape xShape = ShapeHelper.createShape( xDrawDoc,
                    new Point( 1000, 8000 ), new Size( 5000, 5000 ),
                    "com.sun.star.drawing.EllipseShape" );
            xShapes.add( xShape );
            ShapeHelper.addPortion( xShape, "click to go", false );
            ShapeHelper.addPortion( xShape, "to first page", true );
            xShapePropSet = (XPropertySet)
                UnoRuntime.queryInterface( XPropertySet.class, xShape );
            xShapePropSet.setPropertyValue("Effect",
                com.sun.star.presentation.AnimationEffect.FADE_FROM_BOTTOM );
            xShapePropSet.setPropertyValue(
                "OnClick", com.sun.star.presentation.ClickAction.FIRSTPAGE );


            xShape = ShapeHelper.createShape( xDrawDoc,
                new Point( 22000, 8000 ), new Size( 5000, 5000 ),
                    "com.sun.star.drawing.RectangleShape" );
            xShapes.add( xShape );
            ShapeHelper.addPortion( xShape, "click to go", false );
            ShapeHelper.addPortion( xShape, "to the second page", true );
            xShapePropSet = (XPropertySet)
                UnoRuntime.queryInterface( XPropertySet.class, xShape );
            xShapePropSet.setPropertyValue("Effect",
                com.sun.star.presentation.AnimationEffect.FADE_FROM_BOTTOM );

            xShapePropSet.setPropertyValue(
                "OnClick", com.sun.star.presentation.ClickAction.BOOKMARK );
            // set the name of page two, and use it with the bookmark action
            XNamed xPageName = (XNamed)UnoRuntime.queryInterface(
                XNamed.class, PageHelper.getDrawPageByIndex( xDrawDoc, 1 ) );
            xPageName.setName( "page two" );
            xShapePropSet.setPropertyValue(
                "Bookmark", xPageName.getName() );


            /* start an endless presentation which is displayed in
               full-screen mode and placed on top */

            XPresentationSupplier xPresSupplier = (XPresentationSupplier)
                UnoRuntime.queryInterface( XPresentationSupplier.class, xDrawDoc );
            XPresentation xPresentation = xPresSupplier.getPresentation();
            XPropertySet xPresPropSet = (XPropertySet)
                UnoRuntime.queryInterface( XPropertySet.class, xPresentation );
            xPresPropSet.setPropertyValue( "IsEndless", new Boolean( true ) );
            xPresPropSet.setPropertyValue( "IsAlwaysOnTop", new Boolean( true ) );
            xPresPropSet.setPropertyValue( "Pause", new Integer( 0 ) );
            xPresentation.start();
        }
        catch( Exception ex )
        {
            System.out.println( ex );
        }
        System.exit( 0 );
    }

    // this simple method applies the slide transition to a page
    public static void setSlideTransition( XDrawPage xPage,
        com.sun.star.presentation.FadeEffect eEffect,
            com.sun.star.presentation.AnimationSpeed eSpeed,
            int nChange,
                int nDuration )
    {
        // the following test is only sensible if you do not exactly know
        // what type of page xPage is, for this purpose it can been tested
        // if the com.sun.star.presentation.DrawPage service is supported
        XServiceInfo xInfo = (XServiceInfo)UnoRuntime.queryInterface(
                XServiceInfo.class, xPage );
        if ( xInfo.supportsService( "com.sun.star.presentation.DrawPage" ) == true )
        {
            try
            {
                XPropertySet xPropSet = (XPropertySet)
                    UnoRuntime.queryInterface( XPropertySet.class, xPage );
                xPropSet.setPropertyValue( "Effect",   eEffect );
                xPropSet.setPropertyValue( "Speed",    eSpeed );
                xPropSet.setPropertyValue( "Change",   new Integer( nChange ) );
                xPropSet.setPropertyValue( "Duration", new Integer( nDuration ) );
            }
            catch( Exception ex )
            {
            }
        }
    }
}
