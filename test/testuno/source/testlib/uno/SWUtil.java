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
package testlib.uno;


import org.openoffice.test.uno.UnoApp;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XEnumeration;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.container.XNamed;
import com.sun.star.document.XDocumentInfo;
import com.sun.star.document.XDocumentInfoSupplier;
import com.sun.star.frame.XStorable;
import com.sun.star.io.IOException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.style.BreakType;
import com.sun.star.text.ControlCharacter;
import com.sun.star.text.XText;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XController;
import com.sun.star.uno.UnoRuntime;

public class SWUtil {




    public static void saveAsDoc(XTextDocument document, String url) throws IOException {
        saveAs(document, "MS Word 97", url);

    }


    public static void saveAsODT(XTextDocument document, String url) throws IOException {
        saveAs(document, "writer8", url);
    }

    public static void saveAs(XTextDocument document, String filterValue, String url) throws IOException {
        XStorable store = UnoRuntime.queryInterface(XStorable.class, document);
        PropertyValue[] propsValue = new PropertyValue[1];
        propsValue[0] = new PropertyValue();
        propsValue[0].Name = "FilterName";
        propsValue[0].Value = filterValue;
        store.storeAsURL(url, propsValue);

    }

    public static void save(XTextDocument document) throws IOException {
        XStorable store = UnoRuntime.queryInterface(XStorable.class, document);
        store.store();
    }

    public static XTextDocument saveAndReload(XTextDocument document, UnoApp app) throws Exception {
        XStorable store = UnoRuntime.queryInterface(XStorable.class, document);
        store.store();
        String url = document.getURL();
        app.closeDocument(document);
        return openDocumentFromURL(url, app);

    }

    public static XTextDocument newDocument(UnoApp app) throws Exception {
        return (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));

    }

    public static XTextDocument openDocumentFromURL(String url, UnoApp app) throws Exception {
        return (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.loadDocumentFromURL(url));

    }
    public static XTextDocument openDocument(String filePath, UnoApp app) throws Exception {

        return (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(filePath));

    }

    public static void moveCuror2End(XTextDocument document) {
        XText xText = document.getText();
        XTextCursor xTextCursor = xText.createTextCursor();
        xTextCursor.gotoEnd(false);
    }

    public static void moveCuror2Start(XTextDocument document) {
        XText xText = document.getText();
        XTextCursor xTextCursor = xText.createTextCursor();
        xTextCursor.gotoStart(false);
    }

    /**
     * Set document properties. such as subject, title etc
     * @param document - set document information on this document
     * @param prop - document information, including "Subject" ,"Title", "Author", "Title", "KeyWords"
     * @param propValue - value you want to set for prop
     * @throws Exception
     */
    public static void setDocumentProperty(XTextDocument document, String prop, String propValue) throws Exception {
        XDocumentInfoSupplier docInfoSupplier = UnoRuntime.queryInterface(XDocumentInfoSupplier.class, document);
        XDocumentInfo docInfo = docInfoSupplier.getDocumentInfo();
        XPropertySet propsDocInfo = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, docInfo);
        propsDocInfo.setPropertyValue(prop, propValue);
    }


    /**
     * Insert a bookmark into text document
     * @param document text document
     * @param textCursor which part will be bookmarked
     * @param bookmarkName bookmark name
     * @throws Exception
     */
    public static void insertBookmark(XTextDocument document, XTextCursor textCursor, String bookmarkName) throws Exception {
        XMultiServiceFactory xDocFactory = UnoRuntime.queryInterface(XMultiServiceFactory.class, document);
        Object xBookmark = xDocFactory.createInstance("com.sun.star.text.Bookmark");
        XTextContent xBookmarkAsTextContent = UnoRuntime.queryInterface(XTextContent.class, xBookmark);
        XNamed xBookmarkAsNamed = UnoRuntime.queryInterface(XNamed.class, xBookmark);
        xBookmarkAsNamed.setName(bookmarkName);
        document.getText().insertTextContent(textCursor, xBookmarkAsTextContent, true);
    }

    /**
     * insert column break in current cursor
     * @param xText
     * @param currentCursor
     * @throws Exception
     */
    public static void insertColumnBreak(XText xText, XTextCursor currentCursor) throws Exception
    {
        XPropertySet xCursorProps = (XPropertySet)UnoRuntime.queryInterface(
                XPropertySet.class, currentCursor);
        xCursorProps.setPropertyValue("BreakType", BreakType.COLUMN_AFTER);
        xText.insertControlCharacter(currentCursor,ControlCharacter.PARAGRAPH_BREAK,false);
    }

    /**
     * insert page break in current cursor
     * @param xText
     * @param currentCursor
     * @throws Exception
     */
    public static void insertPageBreak(XText xText, XTextCursor currentCursor) throws Exception
    {
        XPropertySet xCursorProps = (XPropertySet)UnoRuntime.queryInterface(
                XPropertySet.class, currentCursor);
        xCursorProps.setPropertyValue("BreakType", BreakType.PAGE_AFTER);
        xText.insertControlCharacter(currentCursor,ControlCharacter.PARAGRAPH_BREAK,false);
    }


    /**
     * get page count
     * @param document
     * @return
     * @throws Exception
     */
    public static int getPageCount(XTextDocument document) throws Exception
    {
        XModel xmodel = (XModel)UnoRuntime.queryInterface(XModel.class, document);
        XController xcont = xmodel.getCurrentController();

        XPropertySet xps = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xcont);
        Integer pageCount = (Integer) xps.getPropertyValue("PageCount");
        return pageCount.intValue();
    }

}
