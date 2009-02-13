/*
 ************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ReportBuilderImplementation.java,v $
 *
 * $Revision: 1.2.36.1 $
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
package com.sun.star.wizards.reportbuilder;

// import com.sun.star.deployment.XPackageInformationProvider;
// import com.sun.star.lang.Locale;
// import com.sun.star.uno.XComponentContext;
import com.sun.star.util.XModeSelector;
// import com.sun.star.wizards.common.PropertySetHelper;
// import com.sun.star.wizards.report.IReportDefinitionReadAccess;
// import com.sun.star.wizards.report.IReportBuilderLayouter;
// import com.sun.star.wizards.report.IReportDocument;

import com.sun.star.wizards.report.*;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNameContainer;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XFrame;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.report.XReportDefinition;
import com.sun.star.sdbc.XConnection;
import com.sun.star.ucb.XCommandProcessor;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XURLTransformer;
import com.sun.star.wizards.common.Resource;
import com.sun.star.wizards.db.FieldColumn;
// import java.io.File;
import java.io.File;
import java.lang.reflect.Constructor;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.Set;
import java.util.Vector;
// import com.sun.star.wizards.ui.UIConsts;
import com.sun.star.wizards.common.FileAccess;
// import com.sun.star.wizards.common.Configuration;
/**
 * This class use the IReportDocument Interface to communicate between the UI
 * and the ReportBuilderLayouter which communicates to the new Sun Report Builder.
 *
 * @author ll93751
 */
public class ReportBuilderImplementation extends ReportImplementationHelper
        implements IReportDocument, IReportDefinitionReadAccess
{

    private static final int MAXIMUM_GROUPCOUNT = 4;
//    public ReportTextDocument getDoc()
//    {
//        throw new UnsupportedOperationException("Not supported yet.");
//    }
    public void clearDocument()
    {
        // throw new UnsupportedOperationException("Not supported yet.");
    }
    private Resource m_aResource;

    public ReportBuilderImplementation()
    {
        // super(null, ReportLayouter.SOOPTPORTRAIT);
        super(null, ReportLayouter.SOOPTLANDSCAPE);
        m_aResource = null;
    }

    private ReportBuilderImplementation(XMultiServiceFactory _aMSF, Resource _oResource)
    {
        // creates an access to the ReportBuilder Extension
        // super(_aMSF, ReportLayouter.SOOPTPORTRAIT);
        super(_aMSF, ReportLayouter.SOOPTLANDSCAPE);
        m_aResource = _oResource;
    }

    /**
     * This is the Factory method. To create a ReportBuilderImplementation Object.
     *
     * @param _xMSF
     * @param _oResource
     * @return
     */
    public static IReportDocument create(XMultiServiceFactory _xMSF, Resource _oResource)
    {
        final ReportBuilderImplementation a = new ReportBuilderImplementation(_xMSF, _oResource);
        // a.m_xGlobalServiceFactory = _xGlobalServiceFactory;
        return a;
    }

//    public void setInitialDocument(Object _aDoc)
//    {
//        throw new UnsupportedOperationException("Not supported yet.");
//    }
    public XWindowPeer getWizardParent()
    {
        // throw new UnsupportedOperationException("Not supported yet.");
        // com.sun.star.frame.XFrame xFrame = thisComponent;
        // openReportBuilderView();
        // XInterface xInterface = (XInterface) getMSF().createInstance("com.sun.star.frame.Desktop");
        // XDesktop xDesktop = (XDesktop) UnoRuntime.queryInterface(XDesktop.class, xInterface);
        // XFrame xFrame = xDesktop.getCurrentFrame();

        final XWindowPeer aWindowPeer = (XWindowPeer) UnoRuntime.queryInterface(XWindowPeer.class, getFrame().getComponentWindow());
        return aWindowPeer;
    }
    private XFrame m_xFrame = null;
    // private ReportBuilderLayouter m_aReportBuilderLayouter = null;
    private String m_sReportBuilderLayoutName = "";

    /**
     * Get access to the current ReportLayouter, which depends on the name.
     * To change the ReportBuilderLayouter use <code>setReportBuilderLayouterName(String Name)</code>.
     * @return
     */
    private IReportBuilderLayouter getReportBuilderLayouter()
    {
        // if (m_aReportBuilderLayouter == null)
        // {
        // m_aReportBuilderLayouter = ReportBuilderLayouter.create(getRecordParser().getReportDocuments(), getConnection());
        // m_aReportBuilderLayouter = ReportBuilderLayouter.create(m_xReportDefinition /* , getConnection() */ );
        final IReportBuilderLayouter aReportBuilderLayouter = (IReportBuilderLayouter) getLayoutMap().get(m_sReportBuilderLayoutName);
        return aReportBuilderLayouter;
    // }
    }
    private Object m_aReportDocument;
    private XPropertySet m_aDocumentDefinition;
    private XReportDefinition m_xReportDefinition;

    /**
     * initialize the Report Builder and open it representation
     * @param _aDoc
     * @param _xConnection
     */
    private void initialize(Object _aDoc, XConnection _xConnection)
    {
        m_aReportDocument = _aDoc;
        // TODO: type down how we got such ID
        final String sClassID = "d7896d52-b7af-4820-9dfe-d404d015960f"; // CLASSID for Report Builder

        Object args[] = new Object[2];

        final PropertyValue aClassID = new PropertyValue();
        aClassID.Name = "ClassID";
        aClassID.Value = sClassID;
        args[0] = aClassID;

        PropertyValue aConnection = new PropertyValue();
        aConnection.Name = "ActiveConnection";
        aConnection.Value = _xConnection;
        args[1] = aConnection;

        XReportDefinition xReportDefinition = null;
        final XMultiServiceFactory xMSF = (XMultiServiceFactory) UnoRuntime.queryInterface(XMultiServiceFactory.class, /* getRecordParser().getReportDocuments() */ _aDoc);
        try
        {
            final Object aObj = xMSF.createInstanceWithArguments("com.sun.star.sdb.DocumentDefinition", args);
            final XPropertySet aDocumentDefinition = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, aObj);
            m_aDocumentDefinition = aDocumentDefinition;

            final XCommandProcessor xProcessor = (XCommandProcessor) UnoRuntime.queryInterface(XCommandProcessor.class, aObj);
            final com.sun.star.ucb.Command aCommand = new com.sun.star.ucb.Command();
            aCommand.Name = "openDesign";
            final com.sun.star.ucb.OpenCommandArgument2 aOpenCommand = new com.sun.star.ucb.OpenCommandArgument2();
            aOpenCommand.Mode = com.sun.star.ucb.OpenMode.DOCUMENT;
            aCommand.Argument = aOpenCommand;
            // com.sun.star.usb.XCommandEnvironment xEnv = new com.sun.star.ucb.XCommandEnvironment();
            final Object aObj2 = xProcessor.execute(aCommand, xProcessor.createCommandIdentifier(), null);
            xReportDefinition = (XReportDefinition) UnoRuntime.queryInterface(XReportDefinition.class, aObj2);
        }
        catch (com.sun.star.uno.Exception e)
        {
            ReportWizard.getLogger().log(com.sun.star.logging.LogLevel.SEVERE, "Problems with initialize the ReportDefinition" + e.getMessage());

        }
        m_xReportDefinition = xReportDefinition;

        switchOffPropertyBrowser();
        switchOffAddFieldWindow();

        setPageOrientation(m_nDefaultPageOrientation, false /* NO_LAYOUT*/);
    // try
    // {
    //     Thread.sleep(1000);
    // }
    // catch (java.lang.InterruptedException e)
    // {
    // }

    }

    private XModeSelector getModeSelector()
    {
        final XController xController = getReportDefinition().getCurrentController();
        final XModeSelector xModeSelector = (XModeSelector) UnoRuntime.queryInterface(XModeSelector.class, xController);
        return xModeSelector;
    }

    private void switchOffAddFieldWindow()
    {
        try
        {
            getModeSelector().setMode("remote");
        }
        catch (com.sun.star.uno.Exception e)
        {
            int dummy = 0;
        }
    }

    private void switchOnAddFieldWindow()
    {
        try
        {
            getModeSelector().setMode("normal");
        }
        catch (com.sun.star.uno.Exception e)
        {
            int dummy = 0;
        }
    }

    private void sleep(int _nSeconds)
    {
        try
        {
            Thread.sleep(_nSeconds * 1000);
        }
        catch (java.lang.InterruptedException e)
        {
        }

    }

    private void switchOffPropertyBrowser()
    {
        try
        {
            final XController xController = getReportDefinition().getCurrentController();
            final XDispatchProvider xDP = (XDispatchProvider) UnoRuntime.queryInterface(XDispatchProvider.class, xController);

            // Create special service for parsing of given URL.
            final Object aURLTransformer = getMSF().createInstance("com.sun.star.util.URLTransformer");
            final XURLTransformer xURLTransformer = (XURLTransformer) UnoRuntime.queryInterface(com.sun.star.util.XURLTransformer.class, aURLTransformer);

            com.sun.star.util.URL[] aURL = new com.sun.star.util.URL[1];
            aURL[0] = new com.sun.star.util.URL();
            aURL[0].Complete = ".uno:ControlProperties";
            xURLTransformer.parseStrict(aURL);

            final XDispatch xDispatch = xDP.queryDispatch(aURL[0], "_self", 0);
            PropertyValue[] xEmpty = new PropertyValue[0];
            xDispatch.dispatch(aURL[0], xEmpty);
        }
        catch (com.sun.star.uno.Exception e)
        {
            int dummy = 0;
        }
    }

    private void switchOnPropertyBrowser()
    {
        // This is implemented with a toggle algorithm.
        switchOffPropertyBrowser();
    }

    /**
     * Returns the Frame of the underlieing Document
     * @return Returns the Frame of the parent (underlieing) Document
     */
    public XFrame getFrame()
    {
        if (m_xFrame == null)
        {
            initialize(getRecordParser().getReportDocuments(), getConnection());
            // m_xFrame = getFrame();
            m_xFrame = getReportDefinition().getCurrentController().getFrame();
            setPageOrientation(m_nDefaultPageOrientation, true /* NO_LAYOUT*/);
        }
        return m_xFrame;
    }

//    public XMultiServiceFactory getDocumentServiceFactory()
//    {
//        throw new UnsupportedOperationException("Not supported yet.");
//    }

//    public void addTextSectionCopies()
//    {
//        throw new UnsupportedOperationException("Not supported yet.");
//    }
    public boolean reconnectToDatabase(XMultiServiceFactory xMSF, PropertyValue[] Properties)
    {
        throw new UnsupportedOperationException("Not supported yet.");
    }

    public void insertDatabaseDatatoReportDocument(XMultiServiceFactory xMSF)
    {
        // throw new UnsupportedOperationException("Not supported yet.");
    }

    public void StopProcess()
    {
        throw new UnsupportedOperationException("Not supported yet.");
    }

    public void store(String Name, int OpenMode)
    {
        // throw new UnsupportedOperationException("Not supported yet.");
        // getReportBuilderLayouter().store(Name);
        // store into the ZIP Storage
        if (OpenMode == 1 /* static Report */)
        {
            // we will store later
            return;
        }

        try
        {
            final XNameAccess aNameAccess = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, m_aReportDocument);
            final String[] aNames = aNameAccess.getElementNames();

//            m_xReportDefinition.storeToStorage(m_xReportDefinition.getDocumentStorage(), m_xReportDefinition.getArgs());

            final XCommandProcessor xProcessor = (XCommandProcessor) UnoRuntime.queryInterface(XCommandProcessor.class, m_aDocumentDefinition);
            com.sun.star.ucb.Command aCommand = new com.sun.star.ucb.Command();
            aCommand.Name = "storeOwn";

            final Object aObj2 = xProcessor.execute(aCommand, xProcessor.createCommandIdentifier(), null);

            final XNameContainer aNameContainer = (XNameContainer) UnoRuntime.queryInterface(XNameContainer.class, m_aReportDocument);
//             aNameContainer.insertByName(Name, m_xReportDefinition);
            aNameContainer.insertByName(Name, m_aDocumentDefinition);
        }
        catch (Exception e)
        {
            int dummy = 0;
        }
    }

    public boolean liveupdate_addGroupNametoDocument(String[] GroupNames, String CurGroupTitle, Vector GroupFieldVector, ArrayList ReportPath, int iSelCount)
    {
        // throw new UnsupportedOperationException("Not supported yet.");
        final int GroupCount = GroupFieldVector.size();
        if (GroupCount < MAXIMUM_GROUPCOUNT)
        {
            // removeGroupNamesofRecordTable(iSelCount);
            final FieldColumn CurFieldColumn = getRecordParser().getFieldColumnByTitle(CurGroupTitle);
            GroupFieldVector.addElement(CurFieldColumn.m_sFieldName);
        }
        return true;
    }

    public void refreshGroupFields(String[] _sNewNames)
    {
        // throw new UnsupportedOperationException("Not supported yet.");
    }

//    public boolean isGroupField(String _FieldName)
//    {
//        throw new UnsupportedOperationException("Not supported yet.");
//    }
    public void liveupdate_removeGroupName(String[] NewSelGroupNames, String CurGroupTitle, Vector GroupFieldVector)
    {
        // throw new UnsupportedOperationException("Not supported yet.");
        final FieldColumn CurFieldColumn = getRecordParser().getFieldColumnByTitle(CurGroupTitle);
        GroupFieldVector.removeElement(CurFieldColumn.m_sFieldName);
    }

    private void setPageOrientation(int nOrientation, boolean bDoLayout)
    {
        getReportBuilderLayouter().setPageOrientation(nOrientation);
        if (bDoLayout)
        {
            getReportBuilderLayouter().layout();
        }
    }

    public void setPageOrientation(int nOrientation)
    {
        // throw new UnsupportedOperationException("Not supported yet.");
        setPageOrientation(nOrientation, true);
    }

    public void liveupdate_changeLayoutTemplate(String LayoutTemplatePath/*, String BitmapPath*/)
    {
        // throw new UnsupportedOperationException("Not supported yet.");
        /* Right Listbox */

        final IReportBuilderLayouter aLayouter = getReportBuilderLayouter();
        aLayouter.loadAndSetBackgroundTemplate(LayoutTemplatePath);
        aLayouter.layout();
    }

    /**
     * Change the current Layouter by a new one with the given Name _sName.
     * If there exist no other layouter with the given name, nothing happens, the layouter will not change to
     * an illegal one.
     * @param _sName Name of the other layouter.
     */
    public void setReportBuilderLayouterName(String _sName)
    {
        final IReportBuilderLayouter aCurrentLayouter = getReportBuilderLayouter();
        final IReportBuilderLayouter aNewLayouter = (IReportBuilderLayouter) m_aLayoutMap.get(_sName);
        if (aNewLayouter != null)
        {
            m_sReportBuilderLayoutName = _sName;
            aNewLayouter.initializeData(aCurrentLayouter);
            aNewLayouter.layout();
        }
    }

    public void liveupdate_changeContentTemplate(String ContentTemplatePath)
    {
        // throw new UnsupportedOperationException("Not supported yet.");
        /* Left Listbox */
        setReportBuilderLayouterName(ContentTemplatePath);
    }

    public void layout_setupRecordSection(String TemplateName)
    {
        // throw new UnsupportedOperationException("Not supported yet.");
    }

    public void removeTextTableAndTextSection()
    {
        // throw new UnsupportedOperationException("Not supported yet.");
    }

    public void layout_selectFirstPage()
    {
        // throw new UnsupportedOperationException("Not supported yet.");
    }

    public void dispose()
    {
        // throw new UnsupportedOperationException("Not supported yet.");
        getReportBuilderLayouter().dispose();

        try
        {
            // XCloseable xClose = (XCloseable)UnoRuntime.queryInterface(XCloseable.class, m_xReportDefinition);
            // xClose.close(true);
            // Failed!

            // next idea, which should always work.
            // XController xController = m_xReportDefinition.getCurrentController();
            // XDispatchProvider xDispatcher = (XDispatchProvider)UnoRuntime.queryInterface(XDispatchProvider.class, xController);
            // xDispatcher.queryDispatch();

            final XComponent xDocumentComponent = (XComponent) UnoRuntime.queryInterface(XComponent.class, m_aDocumentDefinition);
            xDocumentComponent.dispose();
            m_xReportDefinition = null;

        // TODO: dispose() office will be killed.
        // m_xReportDefinition.dispose();
        }
        catch (Exception e)
        {
            // catch all possible exceptions
            int dummy = 0;
        }
    }

    public XComponent getComponent()
    {
        // throw new UnsupportedOperationException("Not supported yet.");
        return null;
    }

    public void liveupdate_changeUserFieldContent(String fieldName, String titlename)
    {
        // throw new UnsupportedOperationException("Not supported yet.");
    }

    public void liveupdate_updateReportTitle(String _sTitleName)
    {
        // throw new UnsupportedOperationException("Not supported yet.");
    }

//    public void finish()
//    {
//        throw new UnsupportedOperationException("Not supported yet.");
//    }
    public void addReportToDBView()
    {
        // throw new UnsupportedOperationException("Not supported yet.");
    }

    private XComponent[] createFinalReportDocument(String Name, Object _aDBConnection, boolean _bAsTemplate, boolean _bOpenInDesign)
    {
        XComponent[] xComponents = new XComponent[1];
        try
        {
            PropertyValue[] aProperties = new PropertyValue[2];
            aProperties[0] = new PropertyValue();
            aProperties[0].Name = "ActiveConnection";
            // aProperties[0].Value = m_aDocumentDefinition;
            aProperties[0].Value = _aDBConnection;

            final com.sun.star.ucb.OpenCommandArgument2 aOpenCommand = new com.sun.star.ucb.OpenCommandArgument2();
            aOpenCommand.Mode = com.sun.star.ucb.OpenMode.DOCUMENT;

            aProperties[1] = new PropertyValue();
            aProperties[1].Name = "OpenCommand"; // This name is 'Schall und Rauch'
//            // since Java 6
//            // aProperties[1].Value = Integer.valueOf(com.sun.star.ucb.OpenMode.DOCUMENT);
            aProperties[1].Value = aOpenCommand;

//            aProperties[2] = new PropertyValue();
//            aProperties[2].Name = "Title"; // This name is 'Schall und Rauch'
//            aProperties[2].Value = Name;

            final XCommandProcessor xProcessor = (XCommandProcessor) UnoRuntime.queryInterface(XCommandProcessor.class, m_aDocumentDefinition);
            com.sun.star.ucb.Command aCommand = new com.sun.star.ucb.Command();
            aCommand.Name = "open";
            aCommand.Argument = aProperties;

            final Object aObj2 = xProcessor.execute(aCommand, xProcessor.createCommandIdentifier(), null);
            xComponents[0] = (XComponent) UnoRuntime.queryInterface(XComponent.class, aObj2);
        }
        catch (com.sun.star.uno.Exception e)
        {
            int dummy = 0;
        }
        return xComponents;
    }

    public XComponent[] createFinalReportDocument(String Name, boolean _bAsTemplate, boolean _bOpenInDesign)
    {
        // XComponent[] xComponents = getReportBuilderLayouter().createFinalReportDocument(Name, getRecordParser().DBConnection ,_bAsTemplate, _bOpenInDesign);
        if (_bAsTemplate == true && _bOpenInDesign == false)
        {
            final XComponent[] xComponents = createFinalReportDocument(Name, getRecordParser().DBConnection, _bAsTemplate, _bOpenInDesign);
            dispose();
            return xComponents;
        }
        else if (_bAsTemplate == false)
        {
            final XComponent[] xComponents = createFinalReportDocument(Name, getRecordParser().DBConnection, _bAsTemplate, _bOpenInDesign);
            boolean bDocisStored = getRecordParser().storeDatabaseDocumentToTempPath(xComponents[0], Name);
            if (bDocisStored)
            {
                getRecordParser().addReportDocument(xComponents[0], false);
            }
            dispose();
        }
        else
        {
            // we won't destroy the report builder window, also don't create a document
            // Do we need to reopen the report builder with the known name?
            switchOnAddFieldWindow();
            switchOnPropertyBrowser();
        }
        return null;
    }

    private XConnection getConnection()
    {
        return getRecordParser().DBConnection;
    }

    public void initializeFieldColumns(final int _nType, final String TableName, final String[] FieldNames)
    {
        getRecordParser().initializeFieldColumns(FieldNames, TableName);
//        getRecordParser().createRecordFieldNames();

        final com.sun.star.wizards.db.RecordParser a = getRecordParser();
        int[] FieldTypes = new int[FieldNames.length];
        int[] FieldWidths = new int[FieldNames.length];
        for (int i = 0; i < FieldNames.length; i++)
        {
            FieldTypes[i] = a.FieldColumns[i].FieldType;
            FieldWidths[i] = a.FieldColumns[i].FieldWidth;
        }
        getReportBuilderLayouter().setTableName(_nType, TableName);
//        getReportBuilderLayouter().insertFields(getRecordParser().getRecordFieldNames());
        getReportBuilderLayouter().insertFieldNames(FieldNames);
        getReportBuilderLayouter().insertFieldTypes(FieldTypes);
        getReportBuilderLayouter().insertFieldWidths(FieldWidths);

        getReportBuilderLayouter().layout();
    }

    public void setFieldTitles(String[] _aFieldTitles)
    {
        getRecordParser().setFieldTitles(_aFieldTitles);
//        getRecordParser().createRecordFieldNames();

        getReportBuilderLayouter().insertFieldTitles(_aFieldTitles);
        getReportBuilderLayouter().layout();
    }

    public void setSorting(String[][] _aSortFieldNames)
    {
        getRecordParser().setSortFieldNames(_aSortFieldNames);
//        getRecordParser().createRecordFieldNames();
    }

    public void setGrouping(String[] _aGroupFieldNames)
    {
        getRecordParser().prependSortFieldNames(_aGroupFieldNames);

        // getRecordParser().createRecordFieldNames();

        // getReportBuilderLayouter().insertFields(getRecordParser().getRecordFieldNames());
//         getReportBuilderLayouter().insertFieldTitles(getRecordParser().get);
        // getReportBuilderLayouter().insertGroups(_aGroupFieldNames);
        getReportBuilderLayouter().insertGroupNames(_aGroupFieldNames);
        getReportBuilderLayouter().layout();
    }

    public String getLayoutPath()
    {
        return "default";
    }
    private ArrayList m_aReportPath;

    public ArrayList getReportPath()
    {
        if (m_aReportPath == null)
        {
            // Check general availability of office paths
            try
            {
                m_aReportPath = FileAccess.getOfficePaths(getMSF(), "Template", "share", "/wizard");
                FileAccess.combinePaths(getMSF(), m_aReportPath, "/wizard/report");
            }
            catch (Exception e)
            {
            }
        }
        return m_aReportPath;
    // return "";
    }

    public String getContentPath()
    {
        return "";
    }

    public int getDefaultPageOrientation()
    {
        return m_nDefaultPageOrientation;
    }

    /**
     * Helper function to get a com.sun.star.wizards.report.layout.ReportBuilderLayouter by it's name
     *
     * @param _sClassName
     * @return the object or null
     */
    private IReportBuilderLayouter getLayoutInstanceFrom(String _sClassName)
    {
        try
        {
            // TODO: Use Package.getPackages(...)
            final Class a = Class.forName(_sClassName);

            final Constructor cTor = a.getConstructor(new Class[]
                    {
                        IReportDefinitionReadAccess.class, Resource.class
                    });
            Object[] aParams = new Object[2];
            aParams[0] = this;
            aParams[1] = m_aResource;
            final IReportBuilderLayouter aReportBuilderLayouter = (IReportBuilderLayouter) cTor.newInstance(aParams);
            return aReportBuilderLayouter;
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
//            catch (NoSuchMethodException ex)
//            {
//                ex.printStackTrace();
//            }
//            catch (SecurityException ex)
//            {
//                ex.printStackTrace();
//            }
//            catch (InstantiationException ex)
//            {
//                ex.printStackTrace();
//            }
//            catch (IllegalAccessException ex)
//            {
//                ex.printStackTrace();
//            }
//            catch (IllegalArgumentException ex)
//            {
//                ex.printStackTrace();
//            }
//            catch (InvocationTargetException ex)
//            {
//                ex.printStackTrace();
//            }
//            catch (ClassNotFoundException e)
//            {
//                e.printStackTrace();
//            }
        return null;
    }
    private LinkedHashMap m_aLayoutMap = null;

    private void insertIntoLayoutMap(IReportBuilderLayouter _aLayout)
    {
        insertIntoLayoutMap(_aLayout, false);
    }

    private void insertIntoLayoutMap(IReportBuilderLayouter _aLayout, boolean _bDefault)
    {
        if (_aLayout != null)
        {
            final String sName = _aLayout.getName();
            if (_bDefault)
            {
                m_sReportBuilderLayoutName = sName;  // here we set the default layout name!
            }
            m_aLayoutMap.put(sName, _aLayout);
        }
    }

    /**
     * Initialize all well known com.sun.star.wizards.report.layout.ReportBuilderLayouter Objects and create exact one instance.
     */
    private LinkedHashMap getLayoutMap()
    {
        if (m_aLayoutMap == null)
        {
            // The LayoutMap is empty, so we create a new LinkedHashMap
            m_aLayoutMap = new LinkedHashMap();

            // TODO: We must know the name of a layouts, There should be a way to say where to find, not the names.
            IReportBuilderLayouter aLayout = getLayoutInstanceFrom("com.sun.star.wizards.reportbuilder.layout.Tabular");
            insertIntoLayoutMap(aLayout, true);
            aLayout = getLayoutInstanceFrom("com.sun.star.wizards.reportbuilder.layout.ColumnarSingleColumn");
            insertIntoLayoutMap(aLayout);
            aLayout = getLayoutInstanceFrom("com.sun.star.wizards.reportbuilder.layout.ColumnarTwoColumns");
            insertIntoLayoutMap(aLayout);
            aLayout = getLayoutInstanceFrom("com.sun.star.wizards.reportbuilder.layout.ColumnarThreeColumns");
            insertIntoLayoutMap(aLayout);
            aLayout = getLayoutInstanceFrom("com.sun.star.wizards.reportbuilder.layout.InBlocksLabelsAbove");
            insertIntoLayoutMap(aLayout);
            aLayout = getLayoutInstanceFrom("com.sun.star.wizards.reportbuilder.layout.InBlocksLabelsLeft");
            insertIntoLayoutMap(aLayout);

        }
        return m_aLayoutMap;
    }

    /**
     * Return a string array array with all found layouts
     * At the moment these layout are hard coded
     * @return
     */
    public String[][] getDataLayout()
    {
        String[][] ContentFiles;
        ContentFiles = new String[2][];
        String[] a = new String[getLayoutMap().size()];
        String[] b = new String[getLayoutMap().size()];

        // run through the whole layoutmap and
        final Set aKeys = getLayoutMap().keySet();
        final Iterator aKeyIterator = aKeys.iterator();
        int i = 0;
        while (aKeyIterator.hasNext())
        {
            final String sKey = (String) aKeyIterator.next();
            a[i] = sKey;
            final IReportBuilderLayouter aLayouter = (IReportBuilderLayouter) m_aLayoutMap.get(sKey);
            b[i++] = aLayouter.getLocalizedName();
        }

        ContentFiles[1] = a;
        ContentFiles[0] = b;
        return ContentFiles;
    }
    private String m_sDefaultHeaderLayoutPath;

    public String[][] getHeaderLayout()
    {
        String[][] LayoutFiles;
        try
        {
            // TODO: check different languages in header layouts
            ArrayList aReportPath = FileAccess.getOfficePaths(getMSF(), "Template", "share", "/wizard");
            FileAccess.combinePaths(getMSF(), aReportPath, "/wizard/report");

            LayoutFiles = FileAccess.getFolderTitles(getMSF(), null, aReportPath, ".otr");
        }
        catch (com.sun.star.wizards.common.NoValidPathException e)
        {
            // if there are problems, don't show anything is a little bit hard.
            LayoutFiles = new String[2][];
            String[] a = new String[1];
            String[] b = new String[1];
            a[0] = "DefaultLayoutOfHeaders";
            b[0] = "default";
            LayoutFiles[1] = a;
            LayoutFiles[0] = b;
        }
        for (int i = 0; i < LayoutFiles[0].length; i++)
        {
            if (LayoutFiles[0][i].equals("default"))
            {
                m_sDefaultHeaderLayoutPath = LayoutFiles[1][i];
                break;
            }
        }
        return LayoutFiles;
    }

    public XReportDefinition getReportDefinition()
    {
        if (m_xReportDefinition == null)
        {
            throw new NullPointerException("Report Definition is not already initialized, check if you too early access the report definition.");
        }
        return m_xReportDefinition;
    }

    public XMultiServiceFactory getGlobalMSF()
    {
        return getMSF();
    }

    public void importReportData(ReportWizard aWizard)
    {
        // throw new UnsupportedOperationException("Not supported yet.");
    }

    public String getDefaultHeaderLayout()
    {
        return m_sDefaultHeaderLayoutPath;
    }

    public void setCommand(String _sCommand)
    {
        getRecordParser().Command = _sCommand;
        getReportDefinition().setCommand(_sCommand);
    // throw new UnsupportedOperationException("Not supported yet.");
    }

    public void setCommandType(int _nCommand)
    {
        getReportDefinition().setCommandType(_nCommand);
    }


    public void checkInvariants() throws java.lang.Exception
    {
        final String sDefaultHeaderLayoutPath = getDefaultHeaderLayout();
        if (sDefaultHeaderLayoutPath == null)
        {
            throw new java.io.IOException("default.otr");
        }

        final String sName = FileAccess.getFilename(sDefaultHeaderLayoutPath);
        // if (sName.toLowerCase().equals("default.otr_") ||
        //         LayoutTemplatePath.equals("DefaultLayoutOfHeaders"))
        // File aFile = new File(sDefaultHeaderLayoutPath);
        // File aFile = new File(sName);
        FileAccess aAccess = new FileAccess(getGlobalMSF());
        if (! aAccess.exists(sDefaultHeaderLayoutPath, true))
        {
            throw new java.io.IOException("default.otr");
        }
    }

}


