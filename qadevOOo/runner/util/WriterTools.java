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

import com.sun.star.graphic.XGraphic;
import com.sun.star.graphic.XGraphicProvider;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;

public class WriterTools {
    public static XTextDocument createTextDoc(XMultiServiceFactory xMSF) {
        PropertyValue[] Args = new PropertyValue[0];
        XComponent comp = DesktopTools.openNewDoc(xMSF, "swriter", Args);
        XTextDocument WriterDoc = UnoRuntime.queryInterface(
                                          XTextDocument.class, comp);

        return WriterDoc;
    } // finish createTextDoc

    public static XTextDocument loadTextDoc(XMultiServiceFactory xMSF,
                                            String url) {
        PropertyValue[] Args = new PropertyValue[0];
        XTextDocument WriterDoc = loadTextDoc(xMSF, url, Args);

        return WriterDoc;
    } // finish createTextDoc

    private static XTextDocument loadTextDoc(XMultiServiceFactory xMSF,
                                            String url, PropertyValue[] Args) {
        XComponent comp = DesktopTools.loadDoc(xMSF, url, Args);
        XTextDocument WriterDoc = UnoRuntime.queryInterface(
                                          XTextDocument.class, comp);

        return WriterDoc;
    } // finish createTextDoc

    public static XDrawPage getDrawPage(XTextDocument aDoc) {
        XDrawPage oDP = null;

        try {
            XDrawPageSupplier oDPS = UnoRuntime.queryInterface(
                                             XDrawPageSupplier.class, aDoc);
            oDP = oDPS.getDrawPage();
        } catch (Exception e) {
            throw new IllegalArgumentException("Couldn't get drawpage", e);
        }

        return oDP;
    }

    public static void insertTextGraphic(XTextDocument aDoc,
                                         XMultiServiceFactory xMSF, XComponentContext xContext, int hpos,
                                         int vpos, int width, int height,
                                         String pic, String name) {
        try {
            Object oGObject = xMSF.createInstance(
                                      "com.sun.star.text.GraphicObject");

            XGraphicProvider xGraphicProvider = UnoRuntime.queryInterface(
                XGraphicProvider.class,
                xContext.getServiceManager().createInstanceWithContext(
                    "com.sun.star.graphic.GraphicProvider", xContext));

            String fullURL = util.utils.getFullTestURL(pic);

            PropertyValue[] aMediaProps = new PropertyValue[] { new PropertyValue() };
            aMediaProps[0].Name = "URL";
            aMediaProps[0].Value = fullURL;

            XGraphic xGraphic = UnoRuntime.queryInterface(XGraphic.class,
                                xGraphicProvider.queryGraphic(aMediaProps));

            XText the_text = aDoc.getText();
            XTextCursor the_cursor = the_text.createTextCursor();
            XTextContent the_content = UnoRuntime.queryInterface(
                                               XTextContent.class, oGObject);
            the_text.insertTextContent(the_cursor, the_content, true);

            XPropertySet oProps = UnoRuntime.queryInterface(
                                          XPropertySet.class, oGObject);

            oProps.setPropertyValue("Graphic", xGraphic);
            oProps.setPropertyValue("HoriOrientPosition", Integer.valueOf(hpos));
            oProps.setPropertyValue("VertOrientPosition", Integer.valueOf(vpos));
            oProps.setPropertyValue("Width", Integer.valueOf(width));
            oProps.setPropertyValue("Height", Integer.valueOf(height));

            XNamed the_name = UnoRuntime.queryInterface(XNamed.class,
                                                                 oGObject);
            the_name.setName(name);
        } catch (Exception ex) {
            System.out.println("Exception while inserting TextGraphic");
            ex.printStackTrace();
        }
    }
}
