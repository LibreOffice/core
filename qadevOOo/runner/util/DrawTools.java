/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package util;

// access the implementations via names
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;

import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.XComponent;
import com.sun.star.drawing.XDrawPages;
import com.sun.star.drawing.XDrawPagesSupplier;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XShapes;
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
            XDrawPagesSupplier oDPS = UnoRuntime.queryInterface(XDrawPagesSupplier.class,aDoc);

            oDPn = oDPS.getDrawPages();
        } catch ( Exception e ) {
            throw new IllegalArgumentException( "Couldn't get drawpages", e );
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
            throw new IllegalArgumentException( "Couldn't get drawpage", e );
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
        return UnoRuntime.queryInterface(XShapes.class,oDP);
    }

}
