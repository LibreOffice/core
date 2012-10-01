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
package com.sun.star.wizards.reportbuilder;

import com.sun.star.util.XModeSelector;

import com.sun.star.awt.XWindowPeer;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XHierarchicalNameContainer;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XFrame;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.report.XReportDefinition;
import com.sun.star.sdb.XSubDocument;
import com.sun.star.sdb.application.DatabaseObject;
import com.sun.star.sdb.application.XDatabaseDocumentUI;
import com.sun.star.sdbc.XConnection;
import com.sun.star.ucb.XCommandProcessor;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XModifiable;
import com.sun.star.util.XURLTransformer;
import com.sun.star.wizards.common.NoValidPathException;
import com.sun.star.wizards.common.Resource;
import com.sun.star.wizards.db.FieldColumn;
import java.lang.reflect.Constructor;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.Set;
import com.sun.star.wizards.common.FileAccess;
import com.sun.star.wizards.common.NamedValueCollection;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.report.IReportBuilderLayouter;
import com.sun.star.wizards.report.IReportDefinitionReadAccess;
import com.sun.star.wizards.report.IReportDocument;
import com.sun.star.wizards.report.ReportImplementationHelper;
import com.sun.star.wizards.report.ReportLayouter;
import com.sun.star.wizards.report.ReportWizard;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * This class use the IReportDocument Interface to communicate between the UI
 * and the ReportBuilderLayouter which communicates to the new Sun Report Builder.
 */
public class ReportBuilderImplementation extends ReportImplementationHelper
        implements IReportDocument, IReportDefinitionReadAccess
{

    private Resource m_resource;
    private XDatabaseDocumentUI m_documentUI;
    private static final int MAXIMUM_GROUPCOUNT = 4;
    private String[] groupNames = null;

    private ReportBuilderImplementation(XMultiServiceFactory _serviceFactory)
    {
        // creates an access to the ReportBuilder Extension
        super(_serviceFactory, ReportLayouter.SOOPTLANDSCAPE);
    }

    public static IReportDocument create(XMultiServiceFactory i_serviceFactory)
    {
        return new ReportBuilderImplementation(i_serviceFactory);
    }

    public void initialize(final XDatabaseDocumentUI i_documentUI, final Resource i_resource)
    {
        m_documentUI = i_documentUI;
        m_resource = i_resource;
    }

    public void clearDocument()
    {
    }

    public XWindowPeer getWizardParent()
    {
        return UnoRuntime.queryInterface(XWindowPeer.class, getFrame().getComponentWindow());
    }
    private XFrame m_xFrame = null;
    private String m_sReportBuilderLayoutName = PropertyNames.EMPTY_STRING;

    /**
     * Get access to the current ReportLayouter, which depends on the name.
     * To change the ReportBuilderLayouter use <code>setReportBuilderLayouterName(String Name)</code>.
     * @return
     */
    private IReportBuilderLayouter getReportBuilderLayouter()
    {
        return getLayoutMap().get(m_sReportBuilderLayoutName);
    }
    private Object m_aReportDocument;
    private XPropertySet m_documentDefinition;
    private XReportDefinition m_reportDocument;

    /**
     * initialize the Report Builder and open it representation
     * @param _aDoc
     */
    private void initialize(Object _aDoc)
    {
        m_aReportDocument = _aDoc;

        try
        {
            NamedValueCollection creationArgs = new NamedValueCollection();
            creationArgs.put("DocumentServiceName", "com.sun.star.report.ReportDefinition");
            creationArgs.put("Mode", "remote");

            XComponent[] docDefinition = new XComponent[]
            {
                null
            };
            XComponent reportDefinitionComp = m_documentUI.createComponentWithArguments(
                    DatabaseObject.REPORT, creationArgs.getPropertyValues(), docDefinition);

            m_documentDefinition = UnoRuntime.queryInterface(XPropertySet.class, docDefinition[0]);
            m_reportDocument = UnoRuntime.queryInterface(XReportDefinition.class, reportDefinitionComp);
        }
        catch (com.sun.star.uno.Exception e)
        {
            ReportWizard.getLogger().log(com.sun.star.logging.LogLevel.SEVERE, "Problems with initialize the ReportDefinition" + e.getMessage());

        }

        switchOffPropertyBrowser();
        switchOffAddFieldWindow();

        setPageOrientation(m_nDefaultPageOrientation, false /* NO_LAYOUT*/);
    }

    private XModeSelector getModeSelector()
    {
        final XController xController = getReportDefinition().getCurrentController();
        return UnoRuntime.queryInterface(XModeSelector.class, xController);
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
            final XDispatchProvider xDP = UnoRuntime.queryInterface(XDispatchProvider.class, xController);

            // Create special service for parsing of given URL.
            final Object aURLTransformer = getMSF().createInstance("com.sun.star.util.URLTransformer");
            final XURLTransformer xURLTransformer = UnoRuntime.queryInterface(com.sun.star.util.XURLTransformer.class, aURLTransformer);

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
            initialize(getRecordParser().getReportDocuments());
            m_xFrame = getReportDefinition().getCurrentController().getFrame();
            setPageOrientation(m_nDefaultPageOrientation, true /* NO_LAYOUT*/);
        }
        return m_xFrame;
    }

    public boolean reconnectToDatabase(XMultiServiceFactory xMSF, PropertyValue[] Properties)
    {
        return false;
    }

    public void insertDatabaseDatatoReportDocument(XMultiServiceFactory xMSF)
    {
    }

    public void StopProcess()
    {
    }

    public void store(String Name, int OpenMode) throws com.sun.star.uno.Exception
    {
        // store into the ZIP Storage
        if (OpenMode == 1 /* static Report */)
        {
            // we will store later
            return;
        }

        final XCommandProcessor xProcessor = UnoRuntime.queryInterface(XCommandProcessor.class, m_documentDefinition);
        final com.sun.star.ucb.Command aCommand = new com.sun.star.ucb.Command();
        aCommand.Name = "store";

        xProcessor.execute(aCommand, xProcessor.createCommandIdentifier(), null);

        final XHierarchicalNameContainer aNameContainer = UnoRuntime.queryInterface(XHierarchicalNameContainer.class, m_aReportDocument);
        aNameContainer.insertByHierarchicalName(Name, m_documentDefinition);
    }

    public boolean liveupdate_addGroupNametoDocument(String[] GroupNames, String CurGroupTitle, ArrayList<String> GroupFieldVector, ArrayList<String> ReportPath, int iSelCount)
    {
        final int GroupCount = GroupFieldVector.size();
        if (GroupCount < MAXIMUM_GROUPCOUNT)
        {
            final FieldColumn CurFieldColumn = getRecordParser().getFieldColumnByTitle(CurGroupTitle);
            GroupFieldVector.add(CurFieldColumn.getFieldName());
        }
        return true;
    }

    public void refreshGroupFields(String[] _sNewNames)
    {
    }

    public void liveupdate_removeGroupName(String[] NewSelGroupNames, String CurGroupTitle, ArrayList<String> GroupFieldVector)
    {
        final FieldColumn CurFieldColumn = getRecordParser().getFieldColumnByTitle(CurGroupTitle);
        GroupFieldVector.remove(CurFieldColumn.getFieldName());
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
        setPageOrientation(nOrientation, true);
    }

    public void liveupdate_changeLayoutTemplate(String LayoutTemplatePath/*, String BitmapPath*/)
    {
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
        final IReportBuilderLayouter aNewLayouter = m_aLayoutMap.get(_sName);
        if (aNewLayouter != null)
        {
            m_sReportBuilderLayoutName = _sName;
            aNewLayouter.initializeData(aCurrentLayouter);
            aNewLayouter.layout();
        }
    }

    public void liveupdate_changeContentTemplate(String ContentTemplatePath)
    {
        /* Left Listbox */
        setReportBuilderLayouterName(ContentTemplatePath);
    }

    public void layout_setupRecordSection(String TemplateName)
    {
    }

    public void removeTextTableAndTextSection()
    {
    }

    public void layout_selectFirstPage()
    {
    }

    private void closeReportDefinition()
    {
        try
        {
            if (m_documentDefinition != null)
            {
                // set the document to "not modified", to ensure that it won't ask the user before closing
                XModifiable documentModify = UnoRuntime.queryInterface(XModifiable.class, m_reportDocument);
                documentModify.setModified(false);
                // actually close
                XSubDocument subComponent = UnoRuntime.queryInterface(XSubDocument.class, m_documentDefinition);
                subComponent.close();
            }
        }
        catch (Exception ex)
        {
            Logger.getLogger(ReportBuilderImplementation.class.getName()).log(Level.SEVERE, null, ex);
        }
        m_documentDefinition = null;
        m_reportDocument = null;
    }

    public void dispose()
    {
        getReportBuilderLayouter().dispose();
        closeReportDefinition();
    }

    public XComponent getComponent()
    {
        return null;
    }

    public void liveupdate_changeUserFieldContent(String fieldName, String titlename)
    {
    }

    public void liveupdate_updateReportTitle(String _sTitleName)
    {
    }

    public void addReportToDBView()
    {
    }

    private XComponent loadReport(final String i_reportName)
    {
        try
        {
            return m_documentUI.loadComponent(DatabaseObject.REPORT, i_reportName, false);
        }
        catch (Exception ex)
        {
            Logger.getLogger(ReportBuilderImplementation.class.getName()).log(Level.SEVERE, null, ex);
        }
        return null;
    }

    private XComponent loadReportFromDocumentDefinition()
    {
        final XCommandProcessor commandProcessor = UnoRuntime.queryInterface(XCommandProcessor.class, m_documentDefinition);

        com.sun.star.ucb.Command aCommand = new com.sun.star.ucb.Command();
        aCommand.Name = "open";
        try
        {
            final Object result = commandProcessor.execute(aCommand, commandProcessor.createCommandIdentifier(), null);
            return UnoRuntime.queryInterface(XComponent.class, result);
        }
        catch (Exception ex)
        {
            Logger.getLogger(ReportBuilderImplementation.class.getName()).log(Level.SEVERE, null, ex);
        }
        return null;
    }

    public void createAndOpenReportDocument(String i_name, boolean i_asTemplate, boolean i_openForEditing)
    {
        if (i_openForEditing)
        {
            // we won't destroy the report builder window, also don't create a document
            // Do we need to reopen the report builder with the known name?
            switchOnAddFieldWindow();
            switchOnPropertyBrowser();
            return;
        }

        if (i_asTemplate)
        {
            // don't need the report definition anymore - the document it represents has already been stored
            closeReportDefinition();

            // open the report, again, this time not in design, but containing data
            loadReport(i_name);
        }
        else
        {
            // execute the report from the (yet unsaved) report definition
            XComponent document = loadReportFromDocumentDefinition();

            // don't need the report definition anymore
            closeReportDefinition();

            // store the generated report
            if (getRecordParser().storeDatabaseDocumentToTempPath(document, i_name))
            {
                getRecordParser().addReportDocument(document, false);
            }
        }

        dispose();
    }

    private XConnection getConnection()
    {
        return getRecordParser().DBConnection;
    }

    public void initializeFieldColumns(final int _nType, final String TableName, final String[] FieldNames)
    {
        getRecordParser().initializeFieldColumns(FieldNames, TableName);

        final com.sun.star.wizards.db.RecordParser a = getRecordParser();
        int[] FieldTypes = new int[FieldNames.length];
        int[] FieldWidths = new int[FieldNames.length];
        for (int i = 0; i < FieldNames.length; i++)
        {
            FieldTypes[i] = a.FieldColumns[i].getFieldType();
            FieldWidths[i] = a.FieldColumns[i].getFieldWidth();
        }
        getReportBuilderLayouter().setTableName(_nType, TableName);
        getReportBuilderLayouter().insertFieldNames(FieldNames);
        getReportBuilderLayouter().insertFieldTypes(FieldTypes);
        getReportBuilderLayouter().insertFieldWidths(FieldWidths);

        getReportBuilderLayouter().layout();
    }

    public void setFieldTitles(String[] _aFieldTitles)
    {
        getRecordParser().setFieldTitles(_aFieldTitles);

        getReportBuilderLayouter().insertFieldTitles(_aFieldTitles);
        getReportBuilderLayouter().layout();
    }

    public void setSorting(String[][] _aSortFieldNames)
    {
        getRecordParser().setSortFieldNames(_aSortFieldNames);
        if (groupNames != null)
        {
            getRecordParser().prependSortFieldNames(groupNames);
        }
        getReportBuilderLayouter().insertSortingNames(_aSortFieldNames);
        getReportBuilderLayouter().layout();
    }

    public void setGrouping(String[] _aGroupFieldNames)
    {
        groupNames = _aGroupFieldNames;
        getRecordParser().prependSortFieldNames(_aGroupFieldNames);

        getReportBuilderLayouter().insertGroupNames(_aGroupFieldNames);
        getReportBuilderLayouter().layout();
    }

    public String getLayoutPath()
    {
        return "default";
    }
    private ArrayList<String> m_aReportPath;

    public ArrayList<String> getReportPath()
    {
        if (m_aReportPath == null)
        {
            try
            {
                // Check general availability of office paths
                m_aReportPath = FileAccess.getOfficePaths(getMSF(), "Template", "share", "/wizard");
                FileAccess.combinePaths(getMSF(), m_aReportPath, "/wizard/report");
            }
            catch (NoValidPathException ex)
            {
                Logger.getLogger(ReportBuilderImplementation.class.getName()).log(Level.SEVERE, null, ex);
            }
        }
        return m_aReportPath;
    }

    public String getContentPath()
    {
        return PropertyNames.EMPTY_STRING;
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
            final Class<?> a = Class.forName(_sClassName);

            final Constructor<?> cTor = a.getConstructor(new Class[]
                    {
                        IReportDefinitionReadAccess.class, Resource.class
                    });
            Object[] aParams = new Object[2];
            aParams[0] = this;
            aParams[1] = m_resource;
            return (IReportBuilderLayouter) cTor.newInstance(aParams);
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
        return null;
    }
    private LinkedHashMap<String, IReportBuilderLayouter> m_aLayoutMap = null;

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
    private LinkedHashMap<String, IReportBuilderLayouter> getLayoutMap()
    {
        if (m_aLayoutMap == null)
        {
            // The LayoutMap is empty, so we create a new LinkedHashMap
            m_aLayoutMap = new LinkedHashMap<String, IReportBuilderLayouter>();

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
        final Set<String> aKeys = getLayoutMap().keySet();
        final Iterator<String> aKeyIterator = aKeys.iterator();
        int i = 0;
        while (aKeyIterator.hasNext())
        {
            final String sKey = aKeyIterator.next();
            a[i] = sKey;
            final IReportBuilderLayouter aLayouter = m_aLayoutMap.get(sKey);
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
            ArrayList<String> aReportPath = FileAccess.getOfficePaths(getMSF(), "Template", "share", "/wizard");
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
        if (m_reportDocument == null)
        {
            throw new NullPointerException("Report Definition is not already initialized, check if you too early access the report definition.");
        }
        return m_reportDocument;
    }

    public XMultiServiceFactory getGlobalMSF()
    {
        return getMSF();
    }

    public void importReportData(ReportWizard aWizard)
    {
    }

    public String getDefaultHeaderLayout()
    {
        return m_sDefaultHeaderLayoutPath;
    }

    public void setCommand(String _sCommand)
    {
        getRecordParser().Command = _sCommand;
        getReportDefinition().setCommand(_sCommand);
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

        FileAccess aAccess = new FileAccess(getGlobalMSF());
        if (!aAccess.exists(sDefaultHeaderLayoutPath, true))
        {
            throw new java.io.IOException("default.otr");
        }
    }
}
