/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DrawTools.java,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package util;

// access the implementations via names
import com.sun.star.uno.XInterface;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;

import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.XComponent;
import com.sun.star.drawing.XDrawPages;
import com.sun.star.drawing.XDrawPagesSupplier;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XShapes;
import com.sun.star.drawing.XShape;


import util.DesktopTools;
import util.InstCreator;
import util.ShapeDsc;

import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;

/**
 * contains helper methods for draw documents
 */


public class DrawTools {

    /**
     * Opens a new draw document
     * with arguments
     * @param xMSF the MultiServiceFactory
     * @return the XComponent Interface of the document
    */

    public static XComponent createDrawDoc( XMultiServiceFactory xMSF ) {
        PropertyValue[] Args = new PropertyValue [0];
        XComponent DrawDoc = DesktopTools.openNewDoc(  xMSF, "sdraw", Args );
        return DrawDoc;
    } // finish createDrawDoc

    /**
     * gets the XDrawPages container of a draw document
     *
     * @param aDoc the draw document
     * @return the XDrawpages container of the document
    */

    public static XDrawPages getDrawPages ( XComponent aDoc ) {
        XDrawPages oDPn = null;
        try {
            XDrawPagesSupplier oDPS = (XDrawPagesSupplier)
            UnoRuntime.queryInterface(XDrawPagesSupplier.class,aDoc);

            oDPn = oDPS.getDrawPages();
        } catch ( Exception e ) {
            throw new IllegalArgumentException( "Couldn't get drawpages" );
        }
        return oDPn;
    } // finish getDrawPages

    /**
     * gets the specified XDrawPage of a draw document
     *
     * @param aDoc the draw document
     * @param nr the index of the DrawPage
     * @return the XDrawpage with index nr of the document
    */

    public static XDrawPage getDrawPage ( XComponent aDoc, int nr ) {
        XDrawPage oDP = null;
        try {
            oDP = (XDrawPage) AnyConverter.toObject(
                new Type(XDrawPage.class),getDrawPages( aDoc ).getByIndex( nr ));
        } catch ( Exception e ) {
            throw new IllegalArgumentException( "Couldn't get drawpage" );
        }
        return oDP;
    }

    /**
     * gets the XShapes container of a draw page
     *
     * @param oDP the draw page
     * @return the XDrawShape container of the drawpage
    */

    public static XShapes getShapes ( XDrawPage oDP ) {
        return (XShapes) UnoRuntime.queryInterface(XShapes.class,oDP);
    }

    /**
     * creates a XShape
     *
     * @param oDoc the document
     * @param height the height of the shape
     * @param width the width of the shape
     * @param x the x-position of the shape
     * @param y the y-position of the shape
     * @param kind the kind of the shape ('Ellipse', 'Line' or 'Rectangle')
     * @return the created XShape
    */

    public XShape createShape( XComponent oDoc, int height, int width, int x,
                                                     int y, String kind ) {
        //possible values for kind are 'Ellipse', 'Line' and 'Rectangle'

        ShapeDsc sDsc = new ShapeDsc( height, width, x, y, kind );
        InstCreator instCreate = new InstCreator( oDoc, sDsc );
        XShape oShape = (XShape)instCreate.getInstance();

        return oShape;
    }

    /**
     * creates a XShape and adds it to the documents
     * first drawpage
     * @param oDoc the document
     * @param height the height of the shape
     * @param width the width of the shape
     * @param x the x-position of the shape
     * @param y the y-position of the shape
     * @param kind the kind of the shape ('Ellipse', 'Line' or 'Rectangle')
     * @return the created XShape
    */

    public void addShape(  XComponent oDoc, int height, int width, int x,
                                                         int y, String kind ) {

        getShapes(getDrawPage(oDoc,0)).add(createShape( oDoc, height, width, x,
                                                                    y, kind ) );
    }

}