/*************************************************************************
 *
 *  $RCSfile: DrawTools.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change:$Date: 2003-01-27 16:27:02 $
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
            oDP = (XDrawPage) getDrawPages( aDoc ).getByIndex( nr );
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