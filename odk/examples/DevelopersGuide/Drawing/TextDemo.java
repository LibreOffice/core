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

import com.sun.star.drawing.XShape;
import com.sun.star.drawing.XShapes;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.TextFitToSizeType;

import com.sun.star.style.LineSpacing;
import com.sun.star.style.LineSpacingMode;
import com.sun.star.style.ParagraphAdjust;



// __________ Implementation __________

// text demo

public class TextDemo
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
                "private:factory/sdraw", "_blank", 0, pPropValues );

            XDrawPage xPage = PageHelper.getDrawPageByIndex( xDrawDoc, 0 );
            XShapes xShapes = UnoRuntime.queryInterface( XShapes.class, xPage );


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
            xShapePropSet = UnoRuntime.queryInterface( XPropertySet.class, xRectangle );


            // first paragraph
            xTextPropSet = ShapeHelper.addPortion( xRectangle, "Portion1", false );
            xTextPropSet.setPropertyValue( "CharColor", Integer.valueOf( 0xff0000 ) );
            xTextPropSet = ShapeHelper.addPortion( xRectangle, "Portion2", false );
            xTextPropSet.setPropertyValue( "CharColor", Integer.valueOf( 0x8080ff ) );
            aLineSpacing.Height = 100;
            ShapeHelper.setPropertyForLastParagraph( xRectangle, "ParaLineSpacing",
                                                     aLineSpacing );

            // second paragraph
            xTextPropSet = ShapeHelper.addPortion( xRectangle, "Portion3", true );
            xTextPropSet.setPropertyValue( "CharColor", Integer.valueOf( 0xff ) );
            aLineSpacing.Height = 200;
            ShapeHelper.setPropertyForLastParagraph( xRectangle, "ParaLineSpacing",
                                                     aLineSpacing );



            // second shape
            xRectangle = ShapeHelper.createShape( xDrawDoc,
                new Point( 0, 10000 ),
                    new Size( 21000, 12500 ),
                        "com.sun.star.drawing.RectangleShape" );
            xShapes.add( xRectangle );
            xShapePropSet = UnoRuntime.queryInterface( XPropertySet.class, xRectangle );
            xShapePropSet.setPropertyValue( "TextFitToSize",
                                            TextFitToSizeType.PROPORTIONAL );
            xShapePropSet.setPropertyValue( "TextLeftDistance",  Integer.valueOf(2500));
            xShapePropSet.setPropertyValue( "TextRightDistance", Integer.valueOf(2500));
            xShapePropSet.setPropertyValue( "TextUpperDistance", Integer.valueOf(2500));
            xShapePropSet.setPropertyValue( "TextLowerDistance", Integer.valueOf(2500));
            xTextPropSet = ShapeHelper.addPortion( xRectangle,
                                                   "using TextFitToSize", false );
            xTextPropSet.setPropertyValue( "ParaAdjust", ParagraphAdjust.CENTER );
            xTextPropSet.setPropertyValue( "CharColor",  Integer.valueOf(0xff00));
            xTextPropSet = ShapeHelper.addPortion(xRectangle,
                                                  "and a Border distance of 2,5 cm",
                                                  true );
            xTextPropSet.setPropertyValue( "CharColor",  Integer.valueOf( 0xff0000 ) );

        }
        catch( Exception ex )
        {
            System.out.println( ex );
        }
        System.exit( 0 );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
