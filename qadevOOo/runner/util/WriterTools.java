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