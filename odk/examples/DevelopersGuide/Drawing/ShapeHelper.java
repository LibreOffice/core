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
import com.sun.star.lang.XMultiServiceFactory;

import com.sun.star.awt.Point;
import com.sun.star.awt.Size;

import com.sun.star.beans.XPropertySet;

import com.sun.star.container.XEnumeration;
import com.sun.star.container.XEnumerationAccess;

import com.sun.star.drawing.XShape;
import com.sun.star.drawing.XShapes;

import com.sun.star.text.ControlCharacter;
import com.sun.star.text.XText;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextRange;


public class ShapeHelper
{
    // __________ static helper methods __________

    public static XPropertySet createAndInsertShape( XComponent xDrawDoc,
            XShapes xShapes, Point aPos, Size aSize, String sShapeType )
        throws java.lang.Exception
    {
        XShape xShape = createShape( xDrawDoc, aPos, aSize, sShapeType );
        xShapes.add( xShape );
        XPropertySet xPropSet = UnoRuntime.queryInterface( XPropertySet.class, xShape );
        return xPropSet;
    }

    /** create a Shape
    */
    public static XShape createShape( XComponent xDrawDoc,
            Point aPos, Size aSize, String sShapeType )
        throws java.lang.Exception
    {
        XShape xShape = null;
        XMultiServiceFactory xFactory =
            UnoRuntime.queryInterface(
                    XMultiServiceFactory.class, xDrawDoc );
        Object xObj = xFactory.createInstance( sShapeType );
        xShape = UnoRuntime.queryInterface(
            XShape.class, xObj );
        xShape.setPosition( aPos );
        xShape.setSize( aSize );
        return xShape;
    }

    /**
        add text to a shape. the return value is the PropertySet
        of the text range that has been added
    */
    public static XPropertySet addPortion( XShape xShape, String sText, boolean bNewParagraph )
        throws com.sun.star.lang.IllegalArgumentException
    {
        XText xText = UnoRuntime.queryInterface( XText.class, xShape );

        XTextCursor xTextCursor = xText.createTextCursor();
        xTextCursor.gotoEnd( false );
        if ( bNewParagraph == true )
        {
            xText.insertControlCharacter( xTextCursor, ControlCharacter.PARAGRAPH_BREAK, false );
            xTextCursor.gotoEnd( false );
        }
        XTextRange xTextRange = UnoRuntime.queryInterface( XTextRange.class, xTextCursor );
        xTextRange.setString( sText );
        xTextCursor.gotoEnd( true );
        XPropertySet xPropSet = UnoRuntime.queryInterface( XPropertySet.class, xTextRange );
        return xPropSet;
    }

    public static void setPropertyForLastParagraph( XShape xText, String sPropName,
        Object aValue )
            throws com.sun.star.beans.UnknownPropertyException,
                com.sun.star.beans.PropertyVetoException,
                    com.sun.star.lang.IllegalArgumentException,
                        com.sun.star.lang.WrappedTargetException,
                            com.sun.star.container.NoSuchElementException
    {
        XEnumerationAccess xEnumerationAccess = UnoRuntime.queryInterface( XEnumerationAccess.class, xText );
        if ( xEnumerationAccess.hasElements() )
        {
            XEnumeration xEnumeration = xEnumerationAccess.createEnumeration();
            while( xEnumeration.hasMoreElements () )
            {
                Object xObj = xEnumeration.nextElement();
                if ( xEnumeration.hasMoreElements() == false )
                {
                    XTextContent xTextContent = UnoRuntime.queryInterface(
                        XTextContent.class, xObj );
                    XPropertySet xParaPropSet = UnoRuntime.queryInterface( XPropertySet.class, xTextContent );
                    xParaPropSet.setPropertyValue( sPropName, aValue );
                }
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
