/*************************************************************************
*
*  $RCSfile: TextDocument.java,v $
*
*  $Revision: 1.3 $
*
*  last change: $Author: hr $ $Date: 2004-08-02 17:22:16 $
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
*/
package com.sun.star.wizards.text;

import com.sun.star.document.XDocumentInfoSupplier;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XFramesSupplier;
import com.sun.star.frame.XModel;
import com.sun.star.awt.XWindow;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.Locale;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;

import com.sun.star.task.XStatusIndicatorFactory;
import com.sun.star.text.XPageCursor;
import com.sun.star.text.XSimpleText;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextViewCursor;
import com.sun.star.text.XTextViewCursorSupplier;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XNumberFormatsSupplier;
import com.sun.star.wizards.common.Desktop;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.document.OfficeDocument;

public class TextDocument {
    public XComponent xComponent;
    public com.sun.star.text.XTextDocument xTextDocument;
    public com.sun.star.util.XNumberFormats NumberFormats;
    public com.sun.star.document.XDocumentInfo xDocInfo;
    public com.sun.star.task.XStatusIndicator xProgressBar;
    public com.sun.star.frame.XFrame xFrame;
    public XMultiServiceFactory xMSFDoc;
    public XMultiServiceFactory xMSF;
    public com.sun.star.util.XNumberFormatsSupplier xNumberFormatsSupplier;
    public com.sun.star.awt.XWindowPeer xWindowPeer;
    public int PageWidth;
    public int ScaleWidth;
    public com.sun.star.awt.Rectangle PosSize;
    public com.sun.star.lang.Locale CharLocale;

    //creates an instance of TextDocument and creates a frame
    public TextDocument(XMultiServiceFactory xMSF) {
        this.xMSF = xMSF;
        XDesktop xDesktop = Desktop.getDesktop(xMSF);
        xFrame = OfficeDocument.createNewFrame(xMSF);
    }

    //creates an instance of TextDocument and creates a frame with an empty document
    public TextDocument(XMultiServiceFactory xMSF, boolean bshowStatusIndicator, boolean bgetCurrentFrame) {
        this.xMSF = xMSF;
        XDesktop xDesktop = Desktop.getDesktop(xMSF);

        if (bgetCurrentFrame) {
            XFramesSupplier xFrameSupplier = (XFramesSupplier) UnoRuntime.queryInterface(XFramesSupplier.class, xDesktop);
            xFrame = xFrameSupplier.getActiveFrame();
            xComponent = (XComponent) UnoRuntime.queryInterface(XComponent.class, xFrame.getController().getModel());
            xTextDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, xComponent);
        } else {
            xFrame = OfficeDocument.createNewFrame(xMSF);
            PropertyValue[] xEmptyArgs = new PropertyValue[0];
            Object oDoc = OfficeDocument.load(xFrame, "private:factory/swriter", "_self", xEmptyArgs);
            xTextDocument = (XTextDocument) oDoc;
            xComponent = (XComponent) UnoRuntime.queryInterface(XComponent.class, xTextDocument);
        }
        XWindow xWindow = xFrame.getComponentWindow();
        //PosSize = xFrame.getComponentWindow().getPosSize();

        if (bshowStatusIndicator) {
            XStatusIndicatorFactory xStatusIndicatorFactory = (XStatusIndicatorFactory) UnoRuntime.queryInterface(XStatusIndicatorFactory.class, xFrame);
            xProgressBar = xStatusIndicatorFactory.createStatusIndicator();
            xProgressBar.start("", 100);
            xProgressBar.setValue(5);
        }
        xWindowPeer = (XWindowPeer) UnoRuntime.queryInterface(XWindowPeer.class, xFrame.getComponentWindow());
        xMSFDoc = (XMultiServiceFactory) UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDocument);
        xNumberFormatsSupplier = (XNumberFormatsSupplier) UnoRuntime.queryInterface(XNumberFormatsSupplier.class, xTextDocument);

        XDocumentInfoSupplier xDocInfoSuppl = (XDocumentInfoSupplier) UnoRuntime.queryInterface(XDocumentInfoSupplier.class, xTextDocument);
        xDocInfo = xDocInfoSuppl.getDocumentInfo();
        CharLocale = (Locale) Helper.getUnoStructValue((Object) xComponent, "CharLocale");
    }

        //creates an instance of TextDocument and creates a frame with an empty document
    public TextDocument(XMultiServiceFactory xMSF,XTextDocument _textDocument, boolean bshowStatusIndicator) {
        this.xMSF = xMSF;
        XDesktop xDesktop = Desktop.getDesktop(xMSF);

                xFrame = _textDocument.getCurrentController().getFrame();
                xComponent = (XComponent) UnoRuntime.queryInterface(XComponent.class, _textDocument);
                xTextDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, xComponent);
        XWindow xWindow = xFrame.getComponentWindow();
        //PosSize = xFrame.getComponentWindow().getPosSize();

        if (bshowStatusIndicator) {
            XStatusIndicatorFactory xStatusIndicatorFactory = (XStatusIndicatorFactory) UnoRuntime.queryInterface(XStatusIndicatorFactory.class, xFrame);
            xProgressBar = xStatusIndicatorFactory.createStatusIndicator();
            xProgressBar.start("", 100);
            xProgressBar.setValue(5);
        }
        xWindowPeer = (XWindowPeer) UnoRuntime.queryInterface(XWindowPeer.class, xFrame.getComponentWindow());
        xMSFDoc = (XMultiServiceFactory) UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDocument);
        xNumberFormatsSupplier = (XNumberFormatsSupplier) UnoRuntime.queryInterface(XNumberFormatsSupplier.class, xTextDocument);

        XDocumentInfoSupplier xDocInfoSuppl = (XDocumentInfoSupplier) UnoRuntime.queryInterface(XDocumentInfoSupplier.class, xTextDocument);
        xDocInfo = xDocInfoSuppl.getDocumentInfo();
        CharLocale = (Locale) Helper.getUnoStructValue((Object) xComponent, "CharLocale");
    }

    //creates an instance of TextDocument and creates a frame and loads a document
    public TextDocument(XMultiServiceFactory xMSF, String URL, PropertyValue[] xArgs) {
        this.xMSF = xMSF;
        XDesktop xDesktop = Desktop.getDesktop(xMSF);

        xFrame = OfficeDocument.createNewFrame(xMSF);
        Object oDoc = OfficeDocument.load(xFrame, URL, "_self", xArgs);
        xTextDocument = (XTextDocument) oDoc;
        xComponent = (XComponent) UnoRuntime.queryInterface(XComponent.class, xTextDocument);
        XWindow xWindow = xFrame.getComponentWindow();

        xWindowPeer = (XWindowPeer) UnoRuntime.queryInterface(XWindowPeer.class, xFrame.getComponentWindow());
        xMSFDoc = (XMultiServiceFactory) UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDocument);
        xNumberFormatsSupplier = (XNumberFormatsSupplier) UnoRuntime.queryInterface(XNumberFormatsSupplier.class, xTextDocument);

        XDocumentInfoSupplier xDocInfoSuppl = (XDocumentInfoSupplier) UnoRuntime.queryInterface(XDocumentInfoSupplier.class, xTextDocument);
        xDocInfo = xDocInfoSuppl.getDocumentInfo();
        CharLocale = (Locale) Helper.getUnoStructValue((Object) xComponent, "CharLocale");
    }


    public static XTextCursor createTextCursor(Object oCursorContainer) {
        XSimpleText xText = (XSimpleText) UnoRuntime.queryInterface(XSimpleText.class, oCursorContainer);
        XTextCursor xTextCursor = xText.createTextCursor();
        return xTextCursor;
    }

    // Todo: This method is  unsecure because the last index is not necessarily the last section

    // Todo: This Routine should be  modified, because I cannot rely on the last Table in the document to be the last in the TextTables sequence
    // to make it really safe you must acquire the Tablenames before the insertion and after the insertion of the new Table. By comparing the
    // two sequences of tablenames you can find out the tablename of the last inserted Table

    // Todo: This method is  unsecure because the last index is not necessarily the last section

    public int getCharWidth(String ScaleString) {
        int iScale = 200;
        xTextDocument.lockControllers();
        int iScaleLen = ScaleString.length();
        com.sun.star.text.XTextCursor xTextCursor = createTextCursor(xTextDocument.getText());
        xTextCursor.gotoStart(false);
        com.sun.star.wizards.common.Helper.setUnoPropertyValue(xTextCursor, "PageDescName", "First Page");
        xTextCursor.setString(ScaleString);
        XTextViewCursorSupplier xViewCursor = (XTextViewCursorSupplier) UnoRuntime.queryInterface(XTextViewCursorSupplier.class, xTextDocument.getCurrentController());
        XTextViewCursor xTextViewCursor = xViewCursor.getViewCursor();
        xTextViewCursor.gotoStart(false);
        int iFirstPos = xTextViewCursor.getPosition().X;
        xTextViewCursor.gotoEnd(false);
        int iLastPos = xTextViewCursor.getPosition().X;
        iScale = (iLastPos - iFirstPos) / iScaleLen;
        xTextCursor.gotoStart(false);
        xTextCursor.gotoEnd(true);
        xTextCursor.setString("");
        xTextDocument.unlockControllers();
        return iScale;
    }

    public void unlockallControllers() {
        while (xTextDocument.hasControllersLocked() == true) {
            xTextDocument.unlockControllers();
        }
    }

    /**
     * Apparently there is no other way to get the
     * page count of a text document other than using a cursor and
     * making it jump to the last page...
     * @param model the document model.
     * @return the page count of the document.
     */
    public static int getPageCount(Object model) {
        XModel xModel = (XModel)UnoRuntime.queryInterface(XModel.class, model);
        XController xController = xModel.getCurrentController();
        XTextViewCursorSupplier xTextVCS = (XTextViewCursorSupplier)
            UnoRuntime.queryInterface(XTextViewCursorSupplier.class,xController);
        XTextViewCursor xTextVC = xTextVCS.getViewCursor();
        XPageCursor xPC = (XPageCursor) UnoRuntime.queryInterface(XPageCursor.class,xTextVC);
        xPC.jumpToLastPage();
        return xPC.getPage();
    }

    /* Possible Values for "OptionString" are: "LoadCellStyles", "LoadTextStyles", "LoadFrameStyles",
                           "LoadPageStyles", "LoadNumberingStyles", "OverwriteStyles" */
}
