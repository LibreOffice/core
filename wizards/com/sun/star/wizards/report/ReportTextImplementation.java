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
package com.sun.star.wizards.report;

import com.sun.star.awt.VclWindowPeerAttribute;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.beans.PropertyValue;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNameContainer;
import com.sun.star.container.XNamed;
import com.sun.star.frame.XFrame;
import com.sun.star.lang.XComponent;
import com.sun.star.sdb.CommandType;
import com.sun.star.table.XCellRange;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextTable;
import com.sun.star.uno.Any;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.wizards.common.FileAccess;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.InvalidQueryException;
import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.common.Resource;
import com.sun.star.lang.XMultiServiceFactory;

import com.sun.star.sdb.application.DatabaseObject;
import com.sun.star.sdb.application.XDatabaseDocumentUI;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.common.SystemDialog;
import com.sun.star.wizards.db.DBMetaData;
import com.sun.star.wizards.document.OfficeDocument;
import com.sun.star.wizards.ui.UIConsts;
import java.util.ArrayList;
import java.util.logging.Level;
import java.util.logging.Logger;

public class ReportTextImplementation extends ReportImplementationHelper implements IReportDocument
{

    private ReportTextDocument  m_aDoc;
    private Object              m_aInitialDoc;
    private Resource            m_resource;
    private XDatabaseDocumentUI m_documentUI;

    private void setInitialDocument(Object _aDoc)
    {
        m_aInitialDoc = _aDoc;
    }

    /**
     * This is a TEMPORARY function to give direct access to the old text document.
     * We have to remove this!!!
     * @return
     */
    ReportTextDocument getDoc()
    {
        if (m_aDoc == null)
        {
            if (m_aInitialDoc instanceof XTextDocument)
            {
                m_aDoc = new ReportTextDocument(getMSF(), (XTextDocument) m_aInitialDoc, m_resource, getRecordParser());
            }
            else if (m_aInitialDoc instanceof String)
            {
                m_aDoc = new ReportTextDocument(getMSF(), (String) m_aInitialDoc, m_resource, getRecordParser());
            }
            else
            {
                throw new RuntimeException("Unknown type for setInitialDocument() given.");
            }
        }
        return m_aDoc;
    }

    public void clearDocument()
    {
        getDoc().oTextSectionHandler.removeAllTextSections();
        getDoc().oTextTableHandler.removeAllTextTables();
        getDoc().DBColumnsVector = new ArrayList<DBColumn>();
    }

    protected ReportTextImplementation( XMultiServiceFactory i_serviceFactory )
    {
        super( i_serviceFactory, ReportLayouter.SOOPTLANDSCAPE );
    }

    public void initialize( final XDatabaseDocumentUI i_documentUI, final Resource i_resource )
    {
        m_documentUI = i_documentUI;
        m_resource = i_resource;

        if ( m_aInitialDoc == null )
            setInitialDocument( getLayoutPath() );

        initialResources();
    }

    static IReportDocument create( XMultiServiceFactory i_serviceFactory, XDatabaseDocumentUI i_documentUI, XTextDocument i_initialDocument, Resource i_resources )
    {
        ReportTextImplementation a = new ReportTextImplementation( i_serviceFactory );
        a.setInitialDocument(i_initialDocument);
        a.initialize( i_documentUI, i_resources );
        return a;
    }

    public XWindowPeer getWizardParent()
    {
        return getDoc().xWindowPeer;
    }
    static String sMsgQueryCreationImpossible;
    static String sReportFormNotExisting;
    static String sMsgHiddenControlMissing;
    static String sMsgEndAutopilot;
    static String sMsgConnectionImpossible;
    static String sMsgNoConnection;
    static String[] ReportMessages = new String[4];

    private void initialResources()
    {
        sReportFormNotExisting = m_resource.getResText(UIConsts.RID_REPORT + 64);
        sMsgQueryCreationImpossible = m_resource.getResText(UIConsts.RID_REPORT + 65);
        sMsgHiddenControlMissing = m_resource.getResText(UIConsts.RID_REPORT + 66);
        sMsgEndAutopilot = m_resource.getResText(UIConsts.RID_DB_COMMON + 33);
        sMsgNoConnection = m_resource.getResText(UIConsts.RID_DB_COMMON + 14);
    }

    public void addTextSectionCopies()
    {
        m_aDoc.setLayoutSectionsVisible(false);
        XTextCursor xTextCursor = ReportTextDocument.createTextCursor(m_aDoc.xTextDocument.getText());
        xTextCursor.gotoStart(false);
        for (int i = 0; i < getRecordParser().GroupFieldNames.length; i++)
        {
            XNamed xNamedTextSection = addLinkedTextSection(xTextCursor, ReportTextDocument.GROUPSECTION + Integer.toString(i + 1), null, null);
            xNamedTextSection.setName(ReportTextDocument.COPYOFGROUPSECTION + (i + 1));
            renameTableofLastSection(ReportTextDocument.COPYOFTBLGROUPSECTION + (i + 1));
        }
        if (getRecordParser().getRecordFieldNames().length > 0)
        {
            XNamed xNamedTextSection = addLinkedTextSection(xTextCursor, ReportTextDocument.RECORDSECTION, null, null);
            xNamedTextSection.setName(ReportTextDocument.COPYOFRECORDSECTION);
            renameTableofLastSection(ReportTextDocument.COPYOFTBLRECORDSECTION);
        }
    }

    private XNamed addLinkedTextSection(XTextCursor xTextCursor, String sLinkRegion, DBColumn CurDBColumn, Object CurGroupValue)
    {
        XNamed xNamedTextSection = null;
        try
        {
            XInterface xTextSection = (XInterface) getDocumentServiceFactory().createInstance("com.sun.star.text.TextSection");
            XTextContent xTextSectionContent = UnoRuntime.queryInterface( XTextContent.class, xTextSection );
            xNamedTextSection = UnoRuntime.queryInterface( XNamed.class, xTextSection );
            xTextCursor.gotoEnd(false);
            xTextCursor.getText().insertTextContent(xTextCursor, xTextSectionContent, true);
            Helper.setUnoPropertyValue(xTextSection, "LinkRegion", sLinkRegion);
            if (CurDBColumn != null)
            {
                boolean bIsGroupTable = (!sLinkRegion.equals(ReportTextDocument.RECORDSECTION));
                if (bIsGroupTable)
                {
                    XTextTable xTextTable = getDoc().oTextTableHandler.getlastTextTable();
                    XCellRange xCellRange = UnoRuntime.queryInterface( XCellRange.class, xTextTable );
                    CurDBColumn.modifyCellContent(xCellRange, CurGroupValue);
                }
            }
        }
        catch (Exception ex)
        {
            Logger.getLogger( ReportTextImplementation.class.getName() ).log( Level.SEVERE, null, ex );
        }
        return xNamedTextSection;
    }

    private void renameTableofLastSection(String _snewname)
    {
        XTextTable xTextTable = getDoc().oTextTableHandler.getlastTextTable();
        XNamed xNamedTable = UnoRuntime.queryInterface( XNamed.class, xTextTable );
        xNamedTable.setName(_snewname);
    }

    private int showMessageBox(String windowServiceName, int windowAttribute, String MessageText)
    {
        return SystemDialog.showMessageBox(getMSF(), getWizardParent(), windowServiceName, windowAttribute, MessageText);
    }

    public boolean reconnectToDatabase(XMultiServiceFactory xMSF, PropertyValue[] _properties)
    {
        try
        {
            XNameContainer xNamedForms = getDoc().oFormHandler.getDocumentForms();
            Object oDBForm = Helper.getUnoObjectbyName(xNamedForms, ReportWizard.SOREPORTFORMNAME);
            boolean bgetConnection;
            if (oDBForm != null)
            {
                String sMsg = sMsgHiddenControlMissing + (char) 13 + sMsgEndAutopilot;
                XNameAccess xNamedForm = UnoRuntime.queryInterface( XNameAccess.class, oDBForm );
                getRecordParser().Command = getDoc().oFormHandler.getValueofHiddenControl(xNamedForm, PropertyNames.COMMAND);
                String sCommandType = getDoc().oFormHandler.getValueofHiddenControl(xNamedForm, PropertyNames.COMMAND_TYPE);
                String sGroupFieldNames = getDoc().oFormHandler.getValueofHiddenControl(xNamedForm, "GroupFieldNames");
                String sFieldNames = getDoc().oFormHandler.getValueofHiddenControl(xNamedForm, "FieldNames");
                String sRecordFieldNames = getDoc().oFormHandler.getValueofHiddenControl(xNamedForm, "RecordFieldNames");

                String sorting = PropertyNames.EMPTY_STRING;
                if ( xNamedForm.hasByName( "Sorting" ) )
                    sorting = getDoc().oFormHandler.getValueofHiddenControl(xNamedForm, "Sorting");

                String sQueryName = PropertyNames.EMPTY_STRING;
                if ( xNamedForm.hasByName( "QueryName" ) )
                    sQueryName = getDoc().oFormHandler.getValueofHiddenControl(xNamedForm, "QueryName");

                String[] sFieldNameList = JavaTools.ArrayoutofString(sFieldNames, PropertyNames.SEMI_COLON);
                String[] sNewList = JavaTools.ArrayoutofString(sRecordFieldNames, PropertyNames.SEMI_COLON);
                if ( sorting.length() > 0)
                {
                    String[] sortList = JavaTools.ArrayoutofString(sorting, PropertyNames.SEMI_COLON);
                    ArrayList<String[]> aSortFields = new ArrayList<String[]>();
                    for (String sortEntry : sortList)
                    {
                        aSortFields.add(JavaTools.ArrayoutofString(sortEntry, ","));
                    }
                    String[][] sortFieldNames = new String[aSortFields.size()][2];
                    aSortFields.toArray(sortFieldNames);
                    getRecordParser().setSortFieldNames(sortFieldNames);
                }
                getRecordParser().setRecordFieldNames(sNewList);
                getRecordParser().GroupFieldNames = JavaTools.ArrayoutofString(sGroupFieldNames, PropertyNames.SEMI_COLON);
                int nOrigCommandType = Integer.valueOf(sCommandType).intValue();
                getRecordParser().setCommandType(nOrigCommandType);

                sMsgQueryCreationImpossible = JavaTools.replaceSubString(sMsgQueryCreationImpossible, getRecordParser().Command, "<STATEMENT>");
                bgetConnection = getRecordParser().getConnection(_properties);
                int nCommandType = com.sun.star.sdb.CommandType.COMMAND;
                boolean bexecute = false;
                if (bgetConnection)
                {
                    if ((getRecordParser().getCommandType() == CommandType.QUERY) && (getRecordParser().Command.equals(PropertyNames.EMPTY_STRING)))
                    {
                        DBMetaData.CommandObject oCommand = getRecordParser().getQueryByName(sQueryName);
                        if (getRecordParser().hasEscapeProcessing(oCommand.getPropertySet()))
                        {
                            getRecordParser().Command = (String) oCommand.getPropertySet().getPropertyValue(PropertyNames.COMMAND);
                        }
                        else
                        {
                            nCommandType = com.sun.star.sdb.CommandType.QUERY;
                            getRecordParser().Command = sQueryName;
                        }
                    }

                    bexecute = getRecordParser().executeCommand(nCommandType); //sMsgQueryCreationImpossible + (char) 13 + sMsgEndAutopilot, sFieldNameList, true);
                    if (bexecute)
                    {
                        bexecute = getRecordParser().getFields(sFieldNameList, true);

                        boolean addSort = true;
                        if ( (nOrigCommandType == CommandType.QUERY) && !sQueryName.equals(PropertyNames.EMPTY_STRING) )
                        {
                            DBMetaData.CommandObject oCommand = getRecordParser().getQueryByName(sQueryName);
                            if (!getRecordParser().hasEscapeProcessing(oCommand.getPropertySet()))
                                addSort = false;
                        }
                        if ( !(addSort && bexecute) )
                        {
                            return bexecute;
                        }
                        getRecordParser().getSQLQueryComposer().m_xQueryAnalyzer.setQuery(getRecordParser().Command);
                        getRecordParser().getSQLQueryComposer().prependSortingCriteria();
                        getRecordParser().Command = getRecordParser().getSQLQueryComposer().getQuery();

                        bexecute = getRecordParser().executeCommand(nCommandType);
                    }
                    return bexecute;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                sReportFormNotExisting = JavaTools.replaceSubString(sReportFormNotExisting, ReportWizard.SOREPORTFORMNAME, "<REPORTFORM>");
                showMessageBox("ErrorBox", VclWindowPeerAttribute.OK, sReportFormNotExisting + (char) 13 + sMsgEndAutopilot);
                return false;
            }
        }
        catch (InvalidQueryException queryexception)
        {
            return false;
        }
        catch (java.lang.Exception ex)
        {
            Logger.getLogger( ReportTextImplementation.class.getName() ).log( Level.SEVERE, null, ex );
            return false;
        }
    }
    private boolean m_bStopProcess;

    public void StopProcess()
    {
        m_bStopProcess = true;
    }

    public void insertDatabaseDatatoReportDocument(XMultiServiceFactory xMSF)
    {
        try
        {
            int ColIndex;
            boolean breset;
            Object oTable;
            ArrayList<Object[]> DataVector = new ArrayList<Object[]>();
            DBColumn CurDBColumn;
            Object CurGroupValue;
            String CurGroupTableName;
            getDoc().oTextFieldHandler.fixDateFields(true);
            getDoc().removeAllVisibleTextSections();
            getDoc().removeNonLayoutTextTables();
            addTextSectionCopies();
            getDoc().getallDBColumns();
            int GroupFieldCount = getRecordParser().GroupFieldNames.length;
            int FieldCount = getRecordParser().FieldColumns.length;
            Object[] OldGroupFieldValues = new Object[GroupFieldCount];
            XTextTable[] xGroupBaseTables = new XTextTable[GroupFieldCount];
            int RecordFieldCount = FieldCount - GroupFieldCount;
            XTextDocument xTextDocument = getDoc().xTextDocument;
            XTextCursor xTextCursor = ReportTextDocument.createTextCursor(getDoc().xTextDocument.getText());
            xTextDocument.lockControllers();

            if (getRecordParser().ResultSet.next())
            {
                replaceUserFields();
                Helper.setUnoPropertyValue(xTextCursor, "PageDescName", "First Page");
                for (ColIndex = 0; ColIndex < GroupFieldCount; ColIndex++)
                {
                    CurGroupTableName = ReportTextDocument.TBLGROUPSECTION + Integer.toString(ColIndex + 1);
                    oTable = getDoc().oTextTableHandler.xTextTablesSupplier.getTextTables().getByName(CurGroupTableName);
                    xGroupBaseTables[ColIndex] = UnoRuntime.queryInterface( XTextTable.class, oTable );
                    CurGroupValue = getRecordParser().getGroupColumnValue(ColIndex);
                    OldGroupFieldValues[ColIndex] = CurGroupValue;
                    CurDBColumn = getDoc().DBColumnsVector.get(ColIndex);
                    addLinkedTextSection(xTextCursor, ReportTextDocument.GROUPSECTION + Integer.toString(ColIndex + 1), CurDBColumn, CurGroupValue); //COPYOF!!!!
                }
                if (getRecordParser().getcurrentRecordData(DataVector))
                {
                    m_bStopProcess = false;
                    while ((getRecordParser().ResultSet.next()) && (!m_bStopProcess))
                    {
                        breset = false;
                        for (ColIndex = 0; ColIndex < GroupFieldCount; ColIndex++)
                        {
                            CurGroupValue = getRecordParser().getGroupColumnValue(ColIndex);
                            if ((!CurGroupValue.equals(OldGroupFieldValues[ColIndex])) || (breset))
                            {
                                breset = true;
                                insertDataToRecordTable(xTextCursor, DataVector, RecordFieldCount);
                                CurDBColumn = getDoc().DBColumnsVector.get(ColIndex);
                                addLinkedTextSection(xTextCursor, ReportTextDocument.COPYOFGROUPSECTION + Integer.toString(ColIndex + 1), CurDBColumn, CurGroupValue);
                                OldGroupFieldValues[ColIndex] = CurGroupValue;
                                breset = !(ColIndex == GroupFieldCount - 1);
                            }
                        }
                        getRecordParser().getcurrentRecordData(DataVector);
                    }
                    insertDataToRecordTable(xTextCursor, DataVector, RecordFieldCount);
                }
                else
                {
                    getDoc().unlockallControllers();
                    return;
                }
            }
            else
            {
                for (ColIndex = 0; ColIndex < GroupFieldCount; ColIndex++)
                {
                    CurDBColumn = getDoc().DBColumnsVector.get(ColIndex);
                    Object oValue = PropertyNames.EMPTY_STRING;
                    addLinkedTextSection(xTextCursor, ReportTextDocument.COPYOFGROUPSECTION + Integer.toString(ColIndex + 1), CurDBColumn, oValue);
                }
                addLinkedTextSection(xTextCursor, ReportTextDocument.COPYOFRECORDSECTION, null, null);
                Object[][] RecordArray = new Object[1][RecordFieldCount];
                for (int i = 0; i < RecordArray[0].length; i++)
                {
                    RecordArray[0][i] = Any.VOID;
                }
                XTextTable xTextTable = getDoc().oTextTableHandler.getlastTextTable();
                OfficeDocument.ArraytoCellRange(RecordArray, xTextTable, 0, 1);
            }
            getDoc().oTextSectionHandler.breakLinkofTextSections();
        }
        catch (Exception ex)
        {
            Logger.getLogger( ReportTextImplementation.class.getName() ).log( Level.SEVERE, null, ex );
        }
        getDoc().unlockallControllers();
        getDoc().setLayoutSectionsVisible(false);
        getDoc().removeCopiedTextSections();
        getDoc().oTextSectionHandler.removeInvisibleTextSections();
        getDoc().removeLayoutTextTables();
    }

    private void insertDataToRecordTable(XTextCursor xTextCursor, ArrayList<Object[]> DataVector, int FieldCount)
    {
        int DataLength = DataVector.size();
        if ((FieldCount > 0) && (DataLength > 0))
        {
            addLinkedTextSection(xTextCursor, ReportTextDocument.COPYOFRECORDSECTION, null, null);
            Object[][] RecordArray = new Object[DataLength][FieldCount];
            DataVector.toArray(RecordArray);
            XTextTable xTextTable = getDoc().oTextTableHandler.getlastTextTable();
            if (DataLength > 1)
            {
                xTextTable.getRows().insertByIndex(xTextTable.getRows().getCount(), DataLength - 1);
            }
            OfficeDocument.ArraytoCellRange(RecordArray, xTextTable, 0, 1);
        }
        DataVector.clear();
    }
    private void replaceUserFields()
    {
        DBColumn CurDBColumn;
        XTextCursor xNameCellCursor;
        String FieldContent;
        int iCount = getDoc().DBColumnsVector.size();
        for (int i = 0; i < iCount; i++)
        {
            CurDBColumn = getDoc().DBColumnsVector.get(i);
            xNameCellCursor = ReportTextDocument.createTextCursor(CurDBColumn.xNameCell);
            xNameCellCursor.gotoStart(false);
            FieldContent = getDoc().oTextFieldHandler.getUserFieldContent(xNameCellCursor);
            if (!FieldContent.equals(PropertyNames.EMPTY_STRING))
            {
                xNameCellCursor.goRight((short) 1, true);
                xNameCellCursor.setString(FieldContent);
            }
        }
    }

    public XFrame getFrame()
    {
        return m_aDoc.xFrame;
    }

    public XMultiServiceFactory getDocumentServiceFactory()
    {
        return m_aDoc.xMSFDoc;
    }

    public void store(String _sName, int _nOpenMode) throws com.sun.star.uno.Exception
    {
        getDoc().createReportForm(ReportWizard.SOREPORTFORMNAME);
        getDoc().oTextFieldHandler.updateDateFields();
        getDoc().oTextFieldHandler.fixDateFields(false);
        if ((_nOpenMode == ReportFinalizer.SOCREATETEMPLATE || _nOpenMode == ReportFinalizer.SOUSETEMPLATE))
        {
            getDoc().oTextSectionHandler.breakLinkofTextSections();
            getRecordParser().storeDatabaseDocumentToTempPath(getComponent(), _sName);
        }
    }

    public boolean liveupdate_addGroupNametoDocument(String[] GroupNames, String CurGroupTitle, ArrayList<String> GroupFieldVector, ArrayList<String> ReportPath, int iSelCount)
    {
        return getDoc().addGroupNametoDocument(GroupNames, CurGroupTitle, GroupFieldVector, ReportPath, iSelCount);
    }

    public void refreshGroupFields(String[] _sNewNames)
    {
        getDoc().refreshGroupFields(_sNewNames);
    }

    public void liveupdate_removeGroupName(String[] NewSelGroupNames, String CurGroupTitle, ArrayList<String> GroupFieldVector)
    {
        getDoc().removeGroupName(NewSelGroupNames, CurGroupTitle, GroupFieldVector);
    }

    public void setPageOrientation(int nOrientation) throws com.sun.star.lang.IllegalArgumentException
    {
        // LLA: should we lock controllers here?
        // CurReportDocument.getDoc().xTextDocument.lockControllers();
        if (nOrientation == ReportLayouter.SOOPTLANDSCAPE)
        {
            getDoc().changePageOrientation(true);
        }
        else if (nOrientation == ReportLayouter.SOOPTPORTRAIT)
        {
            getDoc().changePageOrientation(false);
        }
        else
        {
            throw new com.sun.star.lang.IllegalArgumentException("Unknown Orientation.");
        }
    }

    public void liveupdate_changeLayoutTemplate(String LayoutTemplatePath/*, String BitmapPath*/)
    {
        getDoc().swapLayoutTemplate(LayoutTemplatePath/*, BitmapPath*/);
    }

    public void liveupdate_changeContentTemplate(String ContentTemplatePath)
    {
        getDoc().swapContentTemplate(ContentTemplatePath);
    }

    public void layout_setupRecordSection(String TemplateName)
    {
        getDoc().setupRecordSection(TemplateName);
    }

    public void removeTextTableAndTextSection()
    {
        getDoc().oTextSectionHandler.removeTextSectionbyName("RecordSection");
        getDoc().oTextTableHandler.removeTextTablebyName("Tbl_RecordSection");
    }

    public void layout_selectFirstPage()
    {
        getDoc().oViewHandler.selectFirstPage(getDoc().oTextTableHandler);
    }

    public void dispose()
    {
        OfficeDocument.dispose(getMSF(), getComponent());
    }

    public XComponent getComponent()
    {
        return getDoc().xComponent;
    }

    public void liveupdate_changeUserFieldContent(String fieldname, String sfieldtitle)
    {
        getDoc().oTextFieldHandler.changeUserFieldContent(fieldname, sfieldtitle);
    }

    public void liveupdate_updateReportTitle(String _sTitleName)
    {
    }

    public void addReportToDBView()
    {
        getRecordParser().addReportDocument(getComponent(), true);
    }

    public void createAndOpenReportDocument( String sReportName, boolean _bAsTemplate, boolean _bOpenInDesign )
    {
        try
        {
            m_documentUI.loadComponent( DatabaseObject.REPORT, sReportName, _bOpenInDesign );
        }
        catch ( Exception ex )
        {
            Logger.getLogger( ReportTextImplementation.class.getName() ).log( Level.SEVERE, null, ex );
        }
    }

    public void initializeFieldColumns(final int _nType, final String TableName, final String[] FieldNames)
    {
        getRecordParser().initializeFieldColumns(FieldNames, TableName);
    }

    public void setFieldTitles(String[] sFieldTitles)
    {
        getRecordParser().setFieldTitles(sFieldTitles);
    }

    public void setSorting(String[][] aSortFieldNames)
    {
        getRecordParser().setSortFieldNames(aSortFieldNames);
    }

    public void setGrouping(String[] aGroupFieldNames)
    {
        getRecordParser().prependSortFieldNames(aGroupFieldNames);
    }

// TODO: we have to change to String List!!!!
    private ArrayList<String> m_aReportPath = null;

    public ArrayList<String> getReportPath()
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
                Logger.getLogger( ReportTextImplementation.class.getName() ).log( Level.SEVERE, null, e );
            }
        }
        return m_aReportPath;
    }

    public String getContentPath()
    {
        ArrayList<String> aReportPath = getReportPath();
        for (int i = 0; i < aReportPath.size(); i++)
        {
            String sPath = aReportPath.get(i);
            sPath += "/cnt-default.ott";
            if (FileAccess.isPathValid(getMSF(), sPath))
            {
                return sPath;
            }
        }
        return PropertyNames.EMPTY_STRING;
    }

    public String getLayoutPath()
    {
        ArrayList<String> aReportPath = getReportPath();
        for (int i = 0; i < aReportPath.size(); i++)
        {
            String sPath = aReportPath.get(i);
            sPath += "/stl-default.ott";
            if (FileAccess.isPathValid(getMSF(), sPath))
            {
                return sPath;
            }
        }
        return PropertyNames.EMPTY_STRING;
    }

    public int getDefaultPageOrientation()
    {
        return m_nDefaultPageOrientation;
    }

    public String[][] getDataLayout()
    {
        String[][] ContentFiles;
        try
        {
            ContentFiles = FileAccess.getFolderTitles(getMSF(), "cnt", getReportPath());
            exchangeContentTitlesWithLocalisedOnes(ContentFiles);
        }
        catch (com.sun.star.wizards.common.NoValidPathException e)
        {
            ContentFiles = new String[2][];
            String[] a = new String[1];
            String[] b = new String[1];
            a[0] = "DefaultLayoutOfData";
            b[0] = "default";
            ContentFiles[1] = a;
            ContentFiles[0] = b;
        }
        return ContentFiles;
    }

    public String[][] getHeaderLayout()
    {
        String[][] LayoutFiles;
        try
        {
            LayoutFiles = FileAccess.getFolderTitles(getMSF(), "stl", getReportPath());
            exchangeLayoutTitlesWithLocalisedOnes(LayoutFiles);
        }
        catch (com.sun.star.wizards.common.NoValidPathException e)
        {
            LayoutFiles = new String[2][];
            String[] a = new String[1];
            String[] b = new String[1];
            a[0] = "DefaultLayoutOfHeaders";
            b[0] = "default";
            LayoutFiles[1] = a;
            LayoutFiles[0] = b;
        }
        return LayoutFiles;
    }

    public void importReportData(ReportWizard _aWizard)
    {
        Dataimport CurDataimport = new Dataimport(_aWizard.xMSF);
        CurDataimport.CurReportDocument = this;
        _aWizard.importReportData(_aWizard.xMSF, CurDataimport);
    }

    public void setCommandType(int _nCommand)
    {
        // already set somewhere else
    }

    public void setCommand(String _sCommand)
    {
        getRecordParser().Command = _sCommand;
    }

    public void checkInvariants() throws java.lang.Exception
    {
    }

    private void exchangeContentTitlesWithLocalisedOnes(String[][] ContentFiles)
    {
      for(int i = 0; i < ContentFiles[0].length; ++i)
        {
          if( ContentFiles[0][i].equals("Align Left - Border") )
          {
            ContentFiles[0][i] = m_resource.getResText(UIConsts.RID_REPORT + 94);
          }
          else if( ContentFiles[0][i].equals("Align Left - Compact") )
          {
            ContentFiles[0][i] = m_resource.getResText(UIConsts.RID_REPORT + 95);
          }
          else if( ContentFiles[0][i].equals("Align Left - Elegant") )
          {
            ContentFiles[0][i] = m_resource.getResText(UIConsts.RID_REPORT + 96);
          }
          else if( ContentFiles[0][i].equals("Align Left - Highlighted") )
          {
            ContentFiles[0][i] = m_resource.getResText(UIConsts.RID_REPORT + 97);
          }
          else if( ContentFiles[0][i].equals("Align Left - Modern") )
          {
            ContentFiles[0][i] = m_resource.getResText(UIConsts.RID_REPORT + 98);
          }
          else if( ContentFiles[0][i].equals("Align Left - Red & Blue") )
          {
            ContentFiles[0][i] = m_resource.getResText(UIConsts.RID_REPORT + 99);
          }
          else if( ContentFiles[0][i].equals("Default") )
          {
            ContentFiles[0][i] = m_resource.getResText(UIConsts.RID_REPORT + 100);
          }
          else if( ContentFiles[0][i].equals("Outline - Borders") )
          {
            ContentFiles[0][i] = m_resource.getResText(UIConsts.RID_REPORT + 101);
          }
          else if( ContentFiles[0][i].equals("Outline - Compact") )
          {
            ContentFiles[0][i] = m_resource.getResText(UIConsts.RID_REPORT + 102);
          }
          else if( ContentFiles[0][i].equals("Outline - Elegant") )
          {
            ContentFiles[0][i] = m_resource.getResText(UIConsts.RID_REPORT + 103);
          }
          else if( ContentFiles[0][i].equals("Outline - Highlighted") )
          {
            ContentFiles[0][i] = m_resource.getResText(UIConsts.RID_REPORT + 104);
          }
          else if( ContentFiles[0][i].equals("Outline - Modern") )
          {
            ContentFiles[0][i] = m_resource.getResText(UIConsts.RID_REPORT + 105);
          }
          else if( ContentFiles[0][i].equals("Outline - Red & Blue") )
          {
            ContentFiles[0][i] = m_resource.getResText(UIConsts.RID_REPORT + 106);
          }
          else if( ContentFiles[0][i].equals("Outline, indented - Borders") )
          {
            ContentFiles[0][i] = m_resource.getResText(UIConsts.RID_REPORT + 107);
          }
          else if( ContentFiles[0][i].equals("Outline, indented - Compact") )
          {
            ContentFiles[0][i] = m_resource.getResText(UIConsts.RID_REPORT + 108);
          }
          else if( ContentFiles[0][i].equals("Outline, indented - Elegant") )
          {
            ContentFiles[0][i] = m_resource.getResText(UIConsts.RID_REPORT + 109);
          }
          else if( ContentFiles[0][i].equals("Outline, indented - Highlighted") )
          {
            ContentFiles[0][i] = m_resource.getResText(UIConsts.RID_REPORT + 110);
          }
          else if( ContentFiles[0][i].equals("Outline, indented - Modern") )
          {
            ContentFiles[0][i] = m_resource.getResText(UIConsts.RID_REPORT + 111);
          }
          else if( ContentFiles[0][i].equals("Outline, indented - Red & Blue") )
          {
            ContentFiles[0][i] = m_resource.getResText(UIConsts.RID_REPORT + 112);
          }
        }
    }

    private void exchangeLayoutTitlesWithLocalisedOnes(String[][] LayoutFiles)
    {
        for(int i = 0; i < LayoutFiles[0].length; ++i)
        {
          if( LayoutFiles[0][i].equals("Bubbles") )
          {
            LayoutFiles[0][i] = m_resource.getResText(UIConsts.RID_REPORT + 113);
          }
          else if( LayoutFiles[0][i].equals("Cinema") )
          {
            LayoutFiles[0][i] = m_resource.getResText(UIConsts.RID_REPORT + 114);
          }
          else if( LayoutFiles[0][i].equals("Controlling") )
          {
            LayoutFiles[0][i] = m_resource.getResText(UIConsts.RID_REPORT + 115);
          }
          else if( LayoutFiles[0][i].equals("Default") )
          {
            LayoutFiles[0][i] = m_resource.getResText(UIConsts.RID_REPORT + 116);
          }
          else if( LayoutFiles[0][i].equals("Drafting") )
          {
            LayoutFiles[0][i] = m_resource.getResText(UIConsts.RID_REPORT + 117);
          }
          else if( LayoutFiles[0][i].equals("Finances") )
          {
            LayoutFiles[0][i] = m_resource.getResText(UIConsts.RID_REPORT + 118);
          }
          else if( LayoutFiles[0][i].equals("Flipchart") )
          {
            LayoutFiles[0][i] = m_resource.getResText(UIConsts.RID_REPORT + 119);
          }
          else if( LayoutFiles[0][i].equals("Formal with Company Logo") )
          {
            LayoutFiles[0][i] = m_resource.getResText(UIConsts.RID_REPORT + 120);
          }
          else if( LayoutFiles[0][i].equals("Generic") )
          {
            LayoutFiles[0][i] = m_resource.getResText(UIConsts.RID_REPORT + 121);
          }
          else if( LayoutFiles[0][i].equals("Worldmap") )
          {
            LayoutFiles[0][i] = m_resource.getResText(UIConsts.RID_REPORT + 122);
          }
        }
    }
}
