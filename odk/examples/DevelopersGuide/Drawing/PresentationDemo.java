/*************************************************************************
 *
 *  $RCSfile: PresentationDemo.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-06-10 10:22:19 $
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
import com.sun.star.text.*;

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

// This demo will demonstrate how to create a presentation using the Office API

// The first parameter describes the connection that is to use. If there is no parameter
// "uno:socket,host=localhost,port=8100;urp;StarOffice.ServiceManager" is used.


public class PresentationDemo
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
            xShapePropSet.setPropertyValue(
                "Effect", com.sun.star.presentation.AnimationEffect.WAVYLINE_FROM_BOTTOM );

            /* the following three properties provokes that the shape is dimmed to red
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
            xShapePropSet.setPropertyValue(
                "Effect", com.sun.star.presentation.AnimationEffect.FADE_FROM_BOTTOM );
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
            xShapePropSet.setPropertyValue(
                "Effect", com.sun.star.presentation.AnimationEffect.FADE_FROM_BOTTOM );
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
