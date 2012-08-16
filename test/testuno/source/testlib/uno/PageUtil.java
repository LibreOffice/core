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

import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XServiceInfo;

import com.sun.star.awt.Size;

import com.sun.star.beans.XPropertySet;

import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPages;
import com.sun.star.drawing.XDrawPagesSupplier;
import com.sun.star.drawing.XMasterPageTarget;
import com.sun.star.drawing.XMasterPagesSupplier;

import com.sun.star.presentation.XPresentationPage;
import com.sun.star.presentation.XHandoutMasterSupplier;

public class PageUtil {
    /**
     * Get the page count for standard pages
     *
     * @param xComponent
     *            : The presentation document
     * @return slide count
     */
    static public int getDrawPageCount(XComponent xComponent) {
        XDrawPagesSupplier xDrawPagesSupplier = (XDrawPagesSupplier) UnoRuntime
                .queryInterface(XDrawPagesSupplier.class, xComponent);
        XDrawPages xDrawPages = xDrawPagesSupplier.getDrawPages();
        return xDrawPages.getCount();
    }

    /**
     * Get draw page by index
     *
     * @param xComponent
     *            : The presentation document
     * @param nIndex
     *            : index of slide pages, from 0 or 1?
     * @return Page of corresponding index.
     */
    static public XDrawPage getDrawPageByIndex(XComponent xComponent, int nIndex)
            throws com.sun.star.lang.IndexOutOfBoundsException,
            com.sun.star.lang.WrappedTargetException {
        XDrawPagesSupplier xDrawPagesSupplier = (XDrawPagesSupplier) UnoRuntime
                .queryInterface(XDrawPagesSupplier.class, xComponent);
        XDrawPages xDrawPages = xDrawPagesSupplier.getDrawPages();
        return (XDrawPage) UnoRuntime.queryInterface(XDrawPage.class,
                xDrawPages.getByIndex(nIndex));
    }

    /**
     * Create and insert a draw page into the giving position,the method returns
     * the new created page
     *
     * @param xComponent
     *            :The Presentation Document
     * @param nIndex
     *            :The index at which page will be inserted.
     * @return The newly created page.
     */
    static public XDrawPage insertNewDrawPageByIndex(XComponent xComponent,
            int nIndex) throws Exception {
        XDrawPagesSupplier xDrawPagesSupplier = (XDrawPagesSupplier) UnoRuntime
                .queryInterface(XDrawPagesSupplier.class, xComponent);
        XDrawPages xDrawPages = xDrawPagesSupplier.getDrawPages();
        return xDrawPages.insertNewByIndex(nIndex);
    }

    /**
     * Remove the given page
     *
     * @param xComponent
     *            : The Presentation Document
     * @param xDrawPage
     *            : The page want to be removed.
     */
    static public void removeDrawPage(XComponent xComponent, XDrawPage xDrawPage) {
        XDrawPagesSupplier xDrawPagesSupplier = (XDrawPagesSupplier) UnoRuntime
                .queryInterface(XDrawPagesSupplier.class, xComponent);
        XDrawPages xDrawPages = xDrawPagesSupplier.getDrawPages();
        xDrawPages.remove(xDrawPage);
    }

    /**
     * Get size of the given page
     *
     * @param xDrawPage
     *            : The specified target page
     * @return specifies the 2-dimensional size of the page using width and
     *         height.
     */
    static public Size getPageSize(XDrawPage xDrawPage)
            throws com.sun.star.beans.UnknownPropertyException,
            com.sun.star.lang.WrappedTargetException {
        XPropertySet xPageProperties = (XPropertySet) UnoRuntime
                .queryInterface(XPropertySet.class, xDrawPage);
        return new Size(
                ((Integer) xPageProperties.getPropertyValue("Width"))
                        .intValue(),
                ((Integer) xPageProperties.getPropertyValue("Height"))
                        .intValue());
    }

    /**
     * Get the page count for master pages
     *
     * @param xComponent
     *            : The presentation document
     * @return Count of master pages.
     */
    static public int getMasterPageCount(XComponent xComponent) {
        XMasterPagesSupplier xMasterPagesSupplier = (XMasterPagesSupplier) UnoRuntime
                .queryInterface(XMasterPagesSupplier.class, xComponent);
        XDrawPages xDrawPages = xMasterPagesSupplier.getMasterPages();
        return xDrawPages.getCount();
    }

    /**
     * Get master page by index
     *
     * @param xComponent
     *            : The Presentation document
     * @param nIndex
     *            : Index of target master page.
     * @return Page of
     */
    static public XDrawPage getMasterPageByIndex(XComponent xComponent,
            int nIndex) throws com.sun.star.lang.IndexOutOfBoundsException,
            com.sun.star.lang.WrappedTargetException {
        XMasterPagesSupplier xMasterPagesSupplier = (XMasterPagesSupplier) UnoRuntime
                .queryInterface(XMasterPagesSupplier.class, xComponent);
        XDrawPages xDrawPages = xMasterPagesSupplier.getMasterPages();
        return (XDrawPage) UnoRuntime.queryInterface(XDrawPage.class,
                xDrawPages.getByIndex(nIndex));
    }

    /**
     * creates and inserts a new master page into the giving position, the
     * method returns the new created page
     */
    static public XDrawPage insertNewMasterPageByIndex(XComponent xComponent,
            int nIndex) {
        XMasterPagesSupplier xMasterPagesSupplier = (XMasterPagesSupplier) UnoRuntime
                .queryInterface(XMasterPagesSupplier.class, xComponent);
        XDrawPages xDrawPages = xMasterPagesSupplier.getMasterPages();
        return xDrawPages.insertNewByIndex(nIndex);
    }

    /**
     * removes the given page
     */
    static public void removeMasterPage(XComponent xComponent,
            XDrawPage xDrawPage) {
        XMasterPagesSupplier xMasterPagesSupplier = (XMasterPagesSupplier) UnoRuntime
                .queryInterface(XMasterPagesSupplier.class, xComponent);
        XDrawPages xDrawPages = xMasterPagesSupplier.getMasterPages();
        xDrawPages.remove(xDrawPage);
    }

    /**
     * return the corresponding masterpage for the giving drawpage
     */
    static public XDrawPage getMasterPage(XDrawPage xDrawPage) {
        XMasterPageTarget xMasterPageTarget = (XMasterPageTarget) UnoRuntime
                .queryInterface(XMasterPageTarget.class, xDrawPage);
        return xMasterPageTarget.getMasterPage();
    }

    /**
     * sets given masterpage at the drawpage
     */
    static public void setMasterPage(XDrawPage xDrawPage, XDrawPage xMasterPage) {
        XMasterPageTarget xMasterPageTarget = (XMasterPageTarget) UnoRuntime
                .queryInterface(XMasterPageTarget.class, xDrawPage);
        xMasterPageTarget.setMasterPage(xMasterPage);
    }

    // __________ presentation pages __________

    /**
     * test if a Presentation Document is supported. This is important, because
     * only presentation documents have notes and handout pages
     */
    static public boolean isImpressDocument(XComponent xComponent) {
        XServiceInfo xInfo = (XServiceInfo) UnoRuntime.queryInterface(
                XServiceInfo.class, xComponent);
        return xInfo
                .supportsService("com.sun.star.presentation.PresentationDocument");
    }

    /**
     * in impress documents each normal draw page has a corresponding notes page
     */
    static public XDrawPage getNotesPage(XDrawPage xDrawPage) {
        XPresentationPage aPresentationPage = (XPresentationPage) UnoRuntime
                .queryInterface(XPresentationPage.class, xDrawPage);
        return aPresentationPage.getNotesPage();
    }

    /**
     * in impress each documents has one handout page
     */
    static public XDrawPage getHandoutMasterPage(XComponent xComponent) {
        XHandoutMasterSupplier aHandoutMasterSupplier = (XHandoutMasterSupplier) UnoRuntime
                .queryInterface(XHandoutMasterSupplier.class, xComponent);
        return aHandoutMasterSupplier.getHandoutMasterPage();
    }
}
