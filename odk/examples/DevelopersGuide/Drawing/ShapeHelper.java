/*************************************************************************
 *
 *  $RCSfile: ShapeHelper.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-06-10 10:22:27 $
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


// application specific classes
import com.sun.star.drawing.*;

import com.sun.star.text.*;
import com.sun.star.container.*;

// presentation specific classes

import com.sun.star.beans.XPropertySet;
import com.sun.star.awt.Size;
import com.sun.star.awt.Point;


public class ShapeHelper
{
    // __________ static helper methods __________
    //
    public static XPropertySet createAndInsertShape( XComponent xDrawDoc,
            XShapes xShapes, Point aPos, Size aSize, String sShapeType )
        throws java.lang.Exception
    {
        XShape xShape = createShape( xDrawDoc, aPos, aSize, sShapeType );
        xShapes.add( xShape );
        XPropertySet xPropSet = (XPropertySet)
            UnoRuntime.queryInterface( XPropertySet.class, xShape );
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
            (XMultiServiceFactory )UnoRuntime.queryInterface(
                XMultiServiceFactory.class, xDrawDoc );
        Object xObj = xFactory.createInstance( sShapeType );
        xShape = (XShape)UnoRuntime.queryInterface(
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
        XText xText = (XText)
            UnoRuntime.queryInterface( XText.class, xShape );

        XTextCursor xTextCursor = xText.createTextCursor();
        xTextCursor.gotoEnd( false );
        if ( bNewParagraph == true )
        {
            xText.insertControlCharacter( xTextCursor, ControlCharacter.PARAGRAPH_BREAK, false );
            xTextCursor.gotoEnd( false );
        }
        XTextRange xTextRange = (XTextRange)
            UnoRuntime.queryInterface( XTextRange.class, xTextCursor );
        xTextRange.setString( sText );
        xTextCursor.gotoEnd( true );
        XPropertySet xPropSet = (XPropertySet)
            UnoRuntime.queryInterface( XPropertySet.class, xTextRange );
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
        XEnumerationAccess xEnumerationAccess = (XEnumerationAccess)
            UnoRuntime.queryInterface( XEnumerationAccess.class, xText );
        if ( xEnumerationAccess.hasElements() )
        {
            XEnumeration xEnumeration = xEnumerationAccess.createEnumeration();
            while( xEnumeration.hasMoreElements () )
            {
                Object xObj = xEnumeration.nextElement();
                if ( xEnumeration.hasMoreElements() == false )
                {
                    XTextContent xTextContent = (XTextContent)UnoRuntime.queryInterface(
                        XTextContent.class, xObj );
                    XPropertySet xParaPropSet = (XPropertySet)
                        UnoRuntime.queryInterface( XPropertySet.class, xTextContent );
                    xParaPropSet.setPropertyValue( sPropName, aValue );
                }
            }
        }
    }
}
