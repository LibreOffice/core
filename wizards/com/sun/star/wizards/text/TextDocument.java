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

import java.util.Calendar;
import java.util.GregorianCalendar;

import com.sun.star.container.NoSuchElementException;
import com.sun.star.container.XNameAccess;
import com.sun.star.document.XDocumentProperties;
import com.sun.star.document.XDocumentPropertiesSupplier;
import com.sun.star.frame.XController;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XFramesSupplier;
import com.sun.star.frame.XLoadable;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XModule;
import com.sun.star.frame.XTerminateListener;
import com.sun.star.frame.XStorable;
import com.sun.star.i18n.NumberFormatIndex;
import com.sun.star.awt.Size;
import com.sun.star.awt.XWindow;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.PropertyVetoException;
import com.sun.star.lang.Locale;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.style.XStyle;
import com.sun.star.style.XStyleFamiliesSupplier;
import com.sun.star.task.XStatusIndicatorFactory;
import com.sun.star.text.XPageCursor;
import com.sun.star.text.XSimpleText;
import com.sun.star.text.XText;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextViewCursor;
import com.sun.star.text.XTextViewCursorSupplier;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.DateTime;
import com.sun.star.util.XModifiable;
import com.sun.star.util.XNumberFormatsSupplier;
import com.sun.star.util.XRefreshable;
import com.sun.star.wizards.common.Configuration;
import com.sun.star.wizards.common.Desktop;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.common.Helper.DateUtils;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.document.OfficeDocument;

public class TextDocument
{

    public XComponent xComponent;
    public com.sun.star.text.XTextDocument xTextDocument;
    public com.sun.star.util.XNumberFormats NumberFormats;
    public com.sun.star.document.XDocumentProperties m_xDocProps;
    public com.sun.star.task.XStatusIndicator xProgressBar;
    public com.sun.star.frame.XFrame xFrame;
    public XText xText;
    public XMultiServiceFactory xMSFDoc;
    public XMultiServiceFactory xMSF;
    public com.sun.star.util.XNumberFormatsSupplier xNumberFormatsSupplier;
    public com.sun.star.awt.XWindowPeer xWindowPeer;
    public int PageWidth;
    public int ScaleWidth;
    public Size DocSize;
    public com.sun.star.awt.Rectangle PosSize;
    public com.sun.star.lang.Locale CharLocale;
    public XStorable xStorable;

    // creates an instance of TextDocument and creates a named frame. No document is actually loaded into this frame.
    public TextDocument(XMultiServiceFactory xMSF, XTerminateListener listener, String FrameName)
    {
        this.xMSF = xMSF;
        xFrame = OfficeDocument.createNewFrame(xMSF, listener, FrameName);
    }

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

    // creates an instance of TextDocument from the desktop's current frame
    public TextDocument(XMultiServiceFactory xMSF, boolean bShowStatusIndicator, XTerminateListener listener)
    {
        this.xMSF = xMSF;

        XDesktop xDesktop = Desktop.getDesktop(xMSF);
        XFramesSupplier xFrameSupplier = UnoRuntime.queryInterface(XFramesSupplier.class, xDesktop);
        xFrame = xFrameSupplier.getActiveFrame();
        xComponent = UnoRuntime.queryInterface(XComponent.class, xFrame.getController().getModel());
        xTextDocument = UnoRuntime.queryInterface(XTextDocument.class, xComponent);

        if (bShowStatusIndicator)
        {
            showStatusIndicator();
        }
        init();
    }

    public static class ModuleIdentifier
    {

        private final String m_identifier;

        protected final String getIdentifier()
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
            // TODO: it seems the whole project does not really have an error handling. Other menthods
            // seem to generally silence errors, so we can't do anything else here ...
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
        xNumberFormatsSupplier = UnoRuntime.queryInterface(XNumberFormatsSupplier.class, xTextDocument);

        XDocumentPropertiesSupplier xDocPropsSuppl = UnoRuntime.queryInterface(XDocumentPropertiesSupplier.class, xTextDocument);
        m_xDocProps = xDocPropsSuppl.getDocumentProperties();
        CharLocale = (Locale) Helper.getUnoStructValue(xComponent, "CharLocale");
        xText = xTextDocument.getText();
    }

    private void init()
    {
        xWindowPeer = UnoRuntime.queryInterface(XWindowPeer.class, xFrame.getComponentWindow());
        xMSFDoc = UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDocument);
        xNumberFormatsSupplier = UnoRuntime.queryInterface(XNumberFormatsSupplier.class, xTextDocument);
        XDocumentPropertiesSupplier xDocPropsSuppl = UnoRuntime.queryInterface(XDocumentPropertiesSupplier.class, xTextDocument);
        m_xDocProps = xDocPropsSuppl.getDocumentProperties();
        CharLocale = (Locale) Helper.getUnoStructValue(xComponent, "CharLocale");
        xStorable = UnoRuntime.queryInterface(XStorable.class, xTextDocument);
        xText = xTextDocument.getText();
    }

    private void showStatusIndicator()
    {
        XStatusIndicatorFactory xStatusIndicatorFactory = UnoRuntime.queryInterface(XStatusIndicatorFactory.class, xFrame);
        xProgressBar = xStatusIndicatorFactory.createStatusIndicator();
        xProgressBar.start(PropertyNames.EMPTY_STRING, 100);
        xProgressBar.setValue(5);
    }

    public XTextDocument loadAsPreview(String sDefaultTemplate, boolean asTemplate)
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
        DocSize = getPageSize();
        xMSFDoc = UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDocument);

        ViewHandler myViewHandler = new ViewHandler(xMSFDoc, xTextDocument);
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

    public Size getPageSize()
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

    //creates an instance of TextDocument and creates a frame and loads a document
    public TextDocument(XMultiServiceFactory xMSF, String URL, PropertyValue[] xArgs, XTerminateListener listener)
    {
        this.xMSF = xMSF;
        XDesktop xDesktop = Desktop.getDesktop(xMSF);

        xFrame = OfficeDocument.createNewFrame(xMSF, listener);
        Object oDoc = OfficeDocument.load(xFrame, URL, "_self", xArgs);
        xTextDocument = (XTextDocument) oDoc;
        xComponent = UnoRuntime.queryInterface(XComponent.class, xTextDocument);
        XWindow xWindow = xFrame.getComponentWindow();

        xWindowPeer = UnoRuntime.queryInterface(XWindowPeer.class, xWindow);
        xMSFDoc = UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDocument);
        xNumberFormatsSupplier = UnoRuntime.queryInterface(XNumberFormatsSupplier.class, xTextDocument);

        XDocumentPropertiesSupplier xDocPropsSuppl = UnoRuntime.queryInterface(XDocumentPropertiesSupplier.class, xTextDocument);
        m_xDocProps = xDocPropsSuppl.getDocumentProperties();
        CharLocale = (Locale) Helper.getUnoStructValue(xComponent, "CharLocale");
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

    // Todo: This method is  unsecure because the last index is not necessarily the last section
    public int getCharWidth(String ScaleString)
    {
        int iScale = 200;
        xTextDocument.lockControllers();
        int iScaleLen = ScaleString.length();
        com.sun.star.text.XTextCursor xTextCursor = createTextCursor(xTextDocument.getText());
        xTextCursor.gotoStart(false);
        com.sun.star.wizards.common.Helper.setUnoPropertyValue(xTextCursor, "PageDescName", "First Page");
        xTextCursor.setString(ScaleString);
        XTextViewCursorSupplier xViewCursor = UnoRuntime.queryInterface(XTextViewCursorSupplier.class, xTextDocument.getCurrentController());
        XTextViewCursor xTextViewCursor = xViewCursor.getViewCursor();
        xTextViewCursor.gotoStart(false);
        int iFirstPos = xTextViewCursor.getPosition().X;
        xTextViewCursor.gotoEnd(false);
        int iLastPos = xTextViewCursor.getPosition().X;
        iScale = (iLastPos - iFirstPos) / iScaleLen;
        xTextCursor.gotoStart(false);
        xTextCursor.gotoEnd(true);
        xTextCursor.setString(PropertyNames.EMPTY_STRING);
        unlockallControllers();
        return iScale;
    }

    public void unlockallControllers()
    {
        while (xTextDocument.hasControllersLocked())
        {
            xTextDocument.unlockControllers();
        }
    }

    public void refresh()
    {
        XRefreshable xRefreshable = UnoRuntime.queryInterface(XRefreshable.class, xTextDocument);
        xRefreshable.refresh();
    }

    /**
     * This method sets the Author of a Wizard-generated template correctly
     * and adds a explanatory sentence to the template description.
     * @param WizardName The name of the Wizard.
     * @param TemplateDescription The old Description which is being appended with another sentence.
     */
    public void setWizardTemplateDocInfo(String WizardName, String TemplateDescription)
    {
        try
        {
            Object uD = Configuration.getConfigurationRoot(xMSF, "/org.openoffice.UserProfile/Data", false);
            XNameAccess xNA = UnoRuntime.queryInterface(XNameAccess.class, uD);
            Object gn = xNA.getByName("givenname");
            Object sn = xNA.getByName("sn");
            String fullname = gn + PropertyNames.SPACE + sn;

            Calendar cal = new GregorianCalendar();
            int year = cal.get(Calendar.YEAR);
            int month = cal.get(Calendar.MONTH);
            int day = cal.get(Calendar.DAY_OF_MONTH);
            DateTime currentDate = new DateTime();
            currentDate.Day = (short) day;
            currentDate.Month = (short) month;
            currentDate.Year = (short) year;
            DateUtils du = new DateUtils(xMSF, this.xTextDocument);
            int ff = du.getFormat(NumberFormatIndex.DATE_SYS_DDMMYY);
            String myDate = du.format(ff, currentDate);

            XDocumentPropertiesSupplier xDocPropsSuppl = UnoRuntime.queryInterface(XDocumentPropertiesSupplier.class, xTextDocument);
            XDocumentProperties xDocProps2 = xDocPropsSuppl.getDocumentProperties();
            xDocProps2.setAuthor(fullname);
            xDocProps2.setModifiedBy(fullname);
            String description = xDocProps2.getDescription();
            description = description + PropertyNames.SPACE + TemplateDescription;
            description = JavaTools.replaceSubString(description, WizardName, "<wizard_name>");
            description = JavaTools.replaceSubString(description, myDate, "<current_date>");
            xDocProps2.setDescription(description);
        }
        catch (NoSuchElementException e)
        {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        catch (WrappedTargetException e)
        {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        catch (Exception e)
        {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }

    /**
     * removes an arbitrary Object which supports the  'XTextContent' interface
     */
    public boolean removeTextContent(Object oTextContent)
    {
        try
        {
            XTextContent xTextContent = UnoRuntime.queryInterface(XTextContent.class, oTextContent);
            xText.removeTextContent(xTextContent);
            return true;
        }
        catch (NoSuchElementException e)
        {
            e.printStackTrace(System.err);
            return false;
        }
    }

    /**
     * Apparently there is no other way to get the
     * page count of a text document other than using a cursor and
     * making it jump to the last page...
     * @param model the document model.
     * @return the page count of the document.
     */
    public static int getPageCount(Object model)
    {
        XModel xModel = UnoRuntime.queryInterface(XModel.class, model);
        XController xController = xModel.getCurrentController();
        XTextViewCursorSupplier xTextVCS = UnoRuntime.queryInterface(XTextViewCursorSupplier.class, xController);
        XTextViewCursor xTextVC = xTextVCS.getViewCursor();
        XPageCursor xPC = UnoRuntime.queryInterface(XPageCursor.class, xTextVC);
        xPC.jumpToLastPage();
        return xPC.getPage();
    }

    /* Possible Values for "OptionString" are: "LoadCellStyles", "LoadTextStyles", "LoadFrameStyles",
    "LoadPageStyles", "LoadNumberingStyles", "OverwriteStyles" */
}
