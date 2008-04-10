/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: WriterTools.java,v $
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

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNamed;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPageSupplier;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XText;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;

// access the implementations via names
import com.sun.star.uno.XInterface;

import util.DesktopTools;


public class WriterTools {
    public static XTextDocument createTextDoc(XMultiServiceFactory xMSF) {
        PropertyValue[] Args = new PropertyValue[0];
        XComponent comp = DesktopTools.openNewDoc(xMSF, "swriter", Args);
        XTextDocument WriterDoc = (XTextDocument) UnoRuntime.queryInterface(
                                          XTextDocument.class, comp);

        return WriterDoc;
    } // finish createTextDoc

    public static XTextDocument loadTextDoc(XMultiServiceFactory xMSF,
                                            String url) {
        PropertyValue[] Args = new PropertyValue[0];
        XTextDocument WriterDoc = loadTextDoc(xMSF, url, Args);

        return WriterDoc;
    } // finish createTextDoc

    public static XTextDocument loadTextDoc(XMultiServiceFactory xMSF,
                                            String url, PropertyValue[] Args) {
        XComponent comp = DesktopTools.loadDoc(xMSF, url, Args);
        XTextDocument WriterDoc = (XTextDocument) UnoRuntime.queryInterface(
                                          XTextDocument.class, comp);

        return WriterDoc;
    } // finish createTextDoc

    public static XDrawPage getDrawPage(XTextDocument aDoc) {
        XDrawPage oDP = null;

        try {
            XDrawPageSupplier oDPS = (XDrawPageSupplier) UnoRuntime.queryInterface(
                                             XDrawPageSupplier.class, aDoc);
            oDP = (XDrawPage) oDPS.getDrawPage();
        } catch (Exception e) {
            throw new IllegalArgumentException("Couldn't get drawpage");
        }

        return oDP;
    }

    public static void insertTextGraphic(XTextDocument aDoc,
                                         XMultiServiceFactory xMSF, int hpos,
                                         int vpos, int width, int height,
                                         String pic, String name) {
        try {
            Object oGObject = (XInterface) xMSF.createInstance(
                                      "com.sun.star.text.GraphicObject");

            XText the_text = aDoc.getText();
            XTextCursor the_cursor = the_text.createTextCursor();
            XTextContent the_content = (XTextContent) UnoRuntime.queryInterface(
                                               XTextContent.class, oGObject);
            the_text.insertTextContent(the_cursor, the_content, true);

            XPropertySet oProps = (XPropertySet) UnoRuntime.queryInterface(
                                          XPropertySet.class, oGObject);

            String fullURL = util.utils.getFullTestURL(pic);
            oProps.setPropertyValue("GraphicURL", fullURL);
            oProps.setPropertyValue("HoriOrientPosition", new Integer(hpos));
            oProps.setPropertyValue("VertOrientPosition", new Integer(vpos));
            oProps.setPropertyValue("Width", new Integer(width));
            oProps.setPropertyValue("Height", new Integer(height));

            XNamed the_name = (XNamed) UnoRuntime.queryInterface(XNamed.class,
                                                                 oGObject);
            the_name.setName(name);
        } catch (Exception ex) {
            System.out.println("Exception while insertin TextGraphic");
            ex.printStackTrace();
        }
    }
}