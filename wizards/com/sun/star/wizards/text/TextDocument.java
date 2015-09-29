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
package com.sun.star.wizards.text;

import com.sun.star.document.XDocumentPropertiesSupplier;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XLoadable;
import com.sun.star.frame.XModule;
import com.sun.star.frame.XTerminateListener;
import com.sun.star.frame.XStorable;
import com.sun.star.awt.Size;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.PropertyVetoException;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.style.XStyle;
import com.sun.star.style.XStyleFamiliesSupplier;
import com.sun.star.task.XStatusIndicatorFactory;
import com.sun.star.text.XSimpleText;
import com.sun.star.text.XText;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XModifiable;
import com.sun.star.util.XNumberFormatsSupplier;
import com.sun.star.wizards.common.Desktop;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.document.OfficeDocument;

public class TextDocument
{

    public XComponent xComponent;
    public com.sun.star.text.XTextDocument xTextDocument;
    public com.sun.star.task.XStatusIndicator xProgressBar;
    public com.sun.star.frame.XFrame xFrame;
    public XText xText;
    public XMultiServiceFactory xMSFDoc;
    public XMultiServiceFactory xMSF;
    public com.sun.star.awt.XWindowPeer xWindowPeer;

    // creates an instance of TextDocument by loading a given URL as preview
    public TextDocument(XMultiServiceFactory xMSF, String _sPreviewURL, boolean bShowStatusIndicator, XTerminateListener listener)
    {
        this.xMSF = xMSF;

        xFrame = OfficeDocument.createNewFrame(xMSF, listener);
        xTextDocument = loadAsPreview(_sPreviewURL, true);
        xComponent = UnoRuntime.queryInterface(XComponent.class, xTextDocument);

        if (bShowStatusIndicator)
        {
            showStatusIndicator();
        }
        init();
    }

    public static class ModuleIdentifier
    {

        private final String m_identifier;

        private final String getIdentifier()
        {
            return m_identifier;
        }

        public ModuleIdentifier(String _identifier)
        {
            m_identifier = _identifier;
        }
    }

    // creates an instance of TextDocument containing a blank text document
    public TextDocument(XMultiServiceFactory xMSF, ModuleIdentifier _moduleIdentifier, boolean bShowStatusIndicator)
    {
        this.xMSF = xMSF;

        try
        {
            // create the empty document, and set its module identifier
            xTextDocument = UnoRuntime.queryInterface(XTextDocument.class,
                    xMSF.createInstance("com.sun.star.text.TextDocument"));

            XLoadable xLoadable = UnoRuntime.queryInterface(XLoadable.class, xTextDocument);
            xLoadable.initNew();

            XModule xModule = UnoRuntime.queryInterface(XModule.class,
                    xTextDocument);
            xModule.setIdentifier(_moduleIdentifier.getIdentifier());

            // load the document into a blank frame
            XDesktop xDesktop = Desktop.getDesktop(xMSF);
            XComponentLoader xLoader = UnoRuntime.queryInterface(XComponentLoader.class, xDesktop);
            PropertyValue[] loadArgs = new PropertyValue[]
            {
                new PropertyValue("Model", -1, xTextDocument, com.sun.star.beans.PropertyState.DIRECT_VALUE)
            };
            xLoader.loadComponentFromURL("private:object", "_blank", 0, loadArgs);

            // remember some things for later usage
            xFrame = xTextDocument.getCurrentController().getFrame();
            xComponent = UnoRuntime.queryInterface(XComponent.class, xTextDocument);
        }
        catch (Exception e)
        {
            // TODO: it seems the whole project does not really have an error handling. Other methods
            // seem to generally silence errors, so we can't do anything else here...
        }

        if (bShowStatusIndicator)
        {
            showStatusIndicator();
        }
        init();
    }

    //creates an instance of TextDocument from a given XTextDocument
    public TextDocument(XMultiServiceFactory xMSF, XTextDocument _textDocument, boolean bshowStatusIndicator)
    {
        this.xMSF = xMSF;
        xFrame = _textDocument.getCurrentController().getFrame();
        xComponent = UnoRuntime.queryInterface(XComponent.class, _textDocument);
        xTextDocument = UnoRuntime.queryInterface(XTextDocument.class, xComponent);
        //PosSize = xFrame.getComponentWindow().getPosSize();
        if (bshowStatusIndicator)
        {
            XStatusIndicatorFactory xStatusIndicatorFactory = UnoRuntime.queryInterface(XStatusIndicatorFactory.class, xFrame);
            xProgressBar = xStatusIndicatorFactory.createStatusIndicator();
            xProgressBar.start(PropertyNames.EMPTY_STRING, 100);
            xProgressBar.setValue(5);
        }
        xWindowPeer = UnoRuntime.queryInterface(XWindowPeer.class, xFrame.getComponentWindow());
        xMSFDoc = UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDocument);
        UnoRuntime.queryInterface(XNumberFormatsSupplier.class, xTextDocument);

        XDocumentPropertiesSupplier xDocPropsSuppl = UnoRuntime.queryInterface(XDocumentPropertiesSupplier.class, xTextDocument);
        xDocPropsSuppl.getDocumentProperties();
        Helper.getUnoStructValue(xComponent, "CharLocale");
        xText = xTextDocument.getText();
    }

    private void init()
    {
        xWindowPeer = UnoRuntime.queryInterface(XWindowPeer.class, xFrame.getComponentWindow());
        xMSFDoc = UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDocument);
        UnoRuntime.queryInterface(XNumberFormatsSupplier.class, xTextDocument);
        XDocumentPropertiesSupplier xDocPropsSuppl = UnoRuntime.queryInterface(XDocumentPropertiesSupplier.class, xTextDocument);
        xDocPropsSuppl.getDocumentProperties();
        Helper.getUnoStructValue(xComponent, "CharLocale");
        UnoRuntime.queryInterface(XStorable.class, xTextDocument);
        xText = xTextDocument.getText();
    }

    private void showStatusIndicator()
    {
        XStatusIndicatorFactory xStatusIndicatorFactory = UnoRuntime.queryInterface(XStatusIndicatorFactory.class, xFrame);
        xProgressBar = xStatusIndicatorFactory.createStatusIndicator();
        xProgressBar.start(PropertyNames.EMPTY_STRING, 100);
        xProgressBar.setValue(5);
    }

    private XTextDocument loadAsPreview(String sDefaultTemplate, boolean asTemplate)
    {
        PropertyValue loadValues[] = new PropertyValue[3];
        //      open document in the Preview mode
        loadValues[0] = new PropertyValue();
        loadValues[0].Name = PropertyNames.READ_ONLY;
        loadValues[0].Value = Boolean.TRUE;
        loadValues[1] = new PropertyValue();
        loadValues[1].Name = "AsTemplate";
        loadValues[1].Value = asTemplate ? Boolean.TRUE : Boolean.FALSE;
        loadValues[2] = new PropertyValue();
        loadValues[2].Name = "Preview";
        loadValues[2].Value = Boolean.TRUE;

        //set the preview document to non-modified mode in order to avoid the 'do u want to save' box
        if (xTextDocument != null)
        {
            try
            {
                XModifiable xModi = UnoRuntime.queryInterface(XModifiable.class, xTextDocument);
                xModi.setModified(false);
            }
            catch (PropertyVetoException e1)
            {
                e1.printStackTrace(System.err);
            }
        }
        Object oDoc = OfficeDocument.load(xFrame, sDefaultTemplate, "_self", loadValues);
        xTextDocument = (com.sun.star.text.XTextDocument) oDoc;
        getPageSize();
        xMSFDoc = UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDocument);

        ViewHandler myViewHandler = new ViewHandler(xTextDocument);
        try
        {
            myViewHandler.setViewSetting("ZoomType", Short.valueOf(com.sun.star.view.DocumentZoomType.ENTIRE_PAGE));
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }

        TextFieldHandler myFieldHandler = new TextFieldHandler(xMSF, xTextDocument);
        myFieldHandler.updateDocInfoFields();

        return xTextDocument;

    }

    private Size getPageSize()
    {
        try
        {
            XStyleFamiliesSupplier xStyleFamiliesSupplier = UnoRuntime.queryInterface(XStyleFamiliesSupplier.class, xTextDocument);
            com.sun.star.container.XNameAccess xNameAccess = null;
            xNameAccess = xStyleFamiliesSupplier.getStyleFamilies();
            com.sun.star.container.XNameContainer xPageStyleCollection = null;
            xPageStyleCollection = UnoRuntime.queryInterface(com.sun.star.container.XNameContainer.class, xNameAccess.getByName("PageStyles"));
            XStyle xPageStyle = UnoRuntime.queryInterface(XStyle.class, xPageStyleCollection.getByName("First Page"));
            return (Size) Helper.getUnoPropertyValue(xPageStyle, "Size");

        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
            return null;
        }
    }

    public static XTextCursor createTextCursor(Object oCursorContainer)
    {
        XSimpleText xText = UnoRuntime.queryInterface(XSimpleText.class, oCursorContainer);
        return xText.createTextCursor();
    }

    // Todo: This method is  unsecure because the last index is not necessarily the last section

    // Todo: This Routine should be  modified, because I cannot rely on the last Table in the document to be the last in the TextTables sequence
    // to make it really safe you must acquire the Tablenames before the insertion and after the insertion of the new Table. By comparing the
    // two sequences of tablenames you can find out the tablename of the last inserted Table



    public void unlockallControllers()
    {
        while (xTextDocument.hasControllersLocked())
        {
            xTextDocument.unlockControllers();
        }
    }

}
