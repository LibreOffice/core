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

import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.text.XRelativeTextContentRemove;
import com.sun.star.text.XTextContent;
import java.util.ArrayList;
import java.util.Comparator;
import com.sun.star.awt.Rectangle;
import com.sun.star.awt.VclWindowPeerAttribute;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.db.*;
import com.sun.star.wizards.common.*;
import com.sun.star.wizards.ui.*;
import com.sun.star.sdb.CommandType;
// import com.sun.star.table.XCell;
import com.sun.star.text.XTextDocument;
import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.text.TextTableHandler;
import com.sun.star.wizards.text.TextSectionHandler;
import com.sun.star.wizards.text.TextStyleHandler;
import com.sun.star.wizards.text.TextFieldHandler;
import com.sun.star.wizards.text.ViewHandler;
import com.sun.star.wizards.document.FormHandler;
import java.util.logging.Level;
import java.util.logging.Logger;

class ReportTextDocument extends com.sun.star.wizards.text.TextDocument implements Comparator<FieldColumn>
{

    private static final String ISLANDSCAPE = "IsLandscape";
    private static final String ISVISIBLE = "IsVisible";
    private static final String QUERYNAME = "QueryName";
    private static final String SIZE = "Size";
    // private XMultiServiceFactory xMSF;
    // private XMultiServiceFactory xMSFDoc;
    private Object ReportPageStyle;
    private Object FirstPageStyle;
    private int PageWidth;
    private Rectangle PosSize;
    // private String ReportFolderName;
    public ArrayList<DBColumn> DBColumnsVector;
    private RecordTable CurRecordTable;
    private String sMsgTableNotExisting;
    private String sMsgCommonReportError;
    private String ContentTemplatePath;
    private String LayoutTemplatePath;
    // private String sMsgInvalidTextField;
    private String sMsgEndAutopilot;
    public boolean bIsCurLandscape;
    public TextTableHandler oTextTableHandler;
    public TextSectionHandler oTextSectionHandler;
    public FormHandler oFormHandler;
    public TextStyleHandler oTextStyleHandler;
    public TextFieldHandler oTextFieldHandler;
    public ViewHandler oViewHandler;
    public NumberFormatter oNumberFormatter;
    public static final String TBLRECORDSECTION = "Tbl_RecordSection";
    public static final String TBLGROUPSECTION = "Tbl_GroupField";
    public static final String RECORDSECTION = "RecordSection";
    public static final String GROUPSECTION = "GroupField";
    public static final String COPYOFTBLRECORDSECTION = "CopyOf" + TBLRECORDSECTION;
    public static final String COPYOFTBLGROUPSECTION = "CopyOf" + TBLGROUPSECTION;
    public static final String COPYOFRECORDSECTION = "CopyOf" + RECORDSECTION;
    public static final String COPYOFGROUPSECTION = "CopyOf" + GROUPSECTION;
    private RecordParser CurDBMetaData;

    /** Creates new ReportDocument */
    public ReportTextDocument(XMultiServiceFactory xMSF, XTextDocument _textDocument, Resource oResource, RecordParser _aRP)
    {
        super(xMSF, _textDocument, false);
        lateInit(oResource, _aRP);
    }

    public ReportTextDocument(XMultiServiceFactory xMSF, String _sPreviewURL, Resource oResource, RecordParser _aRP)
    {
        super(xMSF, _sPreviewURL, true, null);
        lateInit(oResource, _aRP);
    }

    private void lateInit(Resource oResource, RecordParser _aRecordParser)
    {
        oTextTableHandler = new TextTableHandler(xMSFDoc, xTextDocument);
        oTextSectionHandler = new TextSectionHandler(xMSFDoc, xTextDocument);
        oFormHandler = new FormHandler(xMSFDoc, xTextDocument);
        oTextStyleHandler = new TextStyleHandler(xMSFDoc, xTextDocument);
        oViewHandler = new ViewHandler(xMSFDoc, xTextDocument);
        oTextFieldHandler = new TextFieldHandler(xMSFDoc, xTextDocument);
        DBColumnsVector = new java.util.ArrayList<DBColumn>();
        oNumberFormatter = oTextTableHandler.getNumberFormatter();
        // CurDBMetaData = new RecordParser(xMSF); //, CharLocale, oNumberFormatter);
        CurDBMetaData = _aRecordParser;
        long lDateCorrection = oNumberFormatter.getNullDateCorrection();
        oNumberFormatter.setBooleanReportDisplayNumberFormat();
        oNumberFormatter.setNullDateCorrection(lDateCorrection);
        // sMsgInvalidTextField = oResource.getResText(UIConsts.RID_REPORT + 73);
        sMsgTableNotExisting = oResource.getResText(UIConsts.RID_REPORT + 61);
        sMsgCommonReportError = oResource.getResText(UIConsts.RID_REPORT + 72);
        sMsgCommonReportError = JavaTools.replaceSubString(sMsgCommonReportError, String.valueOf((char) 13), "<BR>");
        sMsgEndAutopilot = oResource.getResText(UIConsts.RID_DB_COMMON + 33);
        sMsgTableNotExisting = sMsgTableNotExisting + (char) 13 + sMsgEndAutopilot;
        bIsCurLandscape = true;
        getReportPageStyles();
    }

    public NumberFormatter getNumberFormatter()
    {
        return oNumberFormatter;
    }

    public boolean checkReportLayoutMode(String[] GroupFieldNames)
    {
        try
        {
            XNameAccess xTextSections = oTextSectionHandler.xTextSectionsSupplier.getTextSections();
            Object oTextSection;
            if (GroupFieldNames.length > 0)
            {
                oTextSection = xTextSections.getByName(GROUPSECTION + String.valueOf(1));
            }
            else
            {
                oTextSection = xTextSections.getByName(RECORDSECTION);
            }
            return AnyConverter.toBoolean(Helper.getUnoPropertyValue(oTextSection, ISVISIBLE));
        }
        catch (Exception e)
        {
            Logger.getLogger(ReportTextDocument.class.getName()).log(Level.SEVERE, null, e);
            // In doubt we rather suggest this is LayoutMode...
            return true;
        }
    }

    public void swapContentTemplate(String ContentTemplatePath)
    {
//  unlockallControllers();
        //xProgressBar.start("", 10);
        this.ContentTemplatePath = ContentTemplatePath;
        loadSectionsfromTemplate(ContentTemplatePath);
        // xProgressBar.setValue(40);
        oTextStyleHandler.loadStyleTemplates(ContentTemplatePath, "LoadTextStyles");
        // xProgressBar.setValue(70);
        if (CurRecordTable != null)
        {
            CurRecordTable.adjustOptimalTableWidths(xMSF, oViewHandler);
        }
        // xProgressBar.setValue(100);
        oViewHandler.selectFirstPage(oTextTableHandler);
        // xProgressBar.end();
    }

    public void swapLayoutTemplate(String LayoutTemplatePath/*, String BitmapPath*/)
    {
        try
        {
            // xProgressBar.start("", 10);
            this.LayoutTemplatePath = LayoutTemplatePath;
            boolean bOldIsCurLandscape = AnyConverter.toBoolean(Helper.getUnoPropertyValue(ReportPageStyle, ISLANDSCAPE));
            oTextStyleHandler.loadStyleTemplates(LayoutTemplatePath, "LoadPageStyles");
            // xProgressBar.setValue(60);
            changePageOrientation(bOldIsCurLandscape);
            // xProgressBar.setValue(100);
            oViewHandler.selectFirstPage(oTextTableHandler);
            // xProgressBar.end();
        }
        catch (Exception e)
        {
            Logger.getLogger(ReportTextDocument.class.getName()).log(Level.SEVERE, null, e);
        }
    }

    public void createReportForm(String SOREPORTFORMNAME)
    {
        com.sun.star.container.XNameContainer xNamedForm = oFormHandler.insertFormbyName(SOREPORTFORMNAME);
        XNameAccess xNameAccess = UnoRuntime.queryInterface(XNameAccess.class, xNamedForm);
        oFormHandler.insertHiddenControl(xNameAccess, xNamedForm, PropertyNames.COMMAND_TYPE, Integer.toString(CurDBMetaData.getCommandType()));
        oFormHandler.insertHiddenControl(xNameAccess, xNamedForm, QUERYNAME, CurDBMetaData.getCommandName());
        if (CurDBMetaData.getCommandType() == CommandType.QUERY)
        {
            oFormHandler.insertHiddenControl(xNameAccess, xNamedForm, PropertyNames.COMMAND, "");
            //DBMetaData.CommandObject oCommand = CurDBMetaData.getQueryByName(CurDBMetaData.getCommandName());
            //oFormHandler.insertHiddenControl(xNameAccess, xNamedForm, COMMAND, CurDBMetaData.Command);
        }
        else
        {
            oFormHandler.insertHiddenControl(xNameAccess, xNamedForm, PropertyNames.COMMAND, CurDBMetaData.Command);
        }
        oFormHandler.insertHiddenControl(xNameAccess, xNamedForm, "GroupFieldNames", JavaTools.ArraytoString(CurDBMetaData.GroupFieldNames));
        oFormHandler.insertHiddenControl(xNameAccess, xNamedForm, "FieldNames", JavaTools.ArraytoString(CurDBMetaData.getFieldNames()));
        String[][] sortFieldNames = CurDBMetaData.getSortFieldNames();
        if (sortFieldNames != null && sortFieldNames.length > 0)
        {
            final String space = ",";
            final String colon = ";";
            StringBuilder orderBy = new StringBuilder();
            for (String[] sortPair : sortFieldNames)
            {
                orderBy.append(sortPair[0]).append(space).append(sortPair[1]).append(colon);
            }
            if (orderBy.length() != 0)
            {
                orderBy.delete(orderBy.lastIndexOf(colon), orderBy.length());
            }
            oFormHandler.insertHiddenControl(xNameAccess, xNamedForm, "Sorting", orderBy.toString());
        }
        else
        {
            oFormHandler.insertHiddenControl(xNameAccess, xNamedForm, "Sorting", "");
        }
        oFormHandler.insertHiddenControl(xNameAccess, xNamedForm, "RecordFieldNames", JavaTools.ArraytoString(CurDBMetaData.getRecordFieldNames()));
    }

    public void updateReportTitle(String _sTitleName)
    {
        m_xDocProps.setTitle(_sTitleName);
    }

    public void getReportPageStyles()
    {
        try
        {
            Object oPageStyleFamily = oTextStyleHandler.xStyleFamiliesSupplier.getStyleFamilies().getByName("PageStyles");
            ReportPageStyle = Helper.getUnoObjectbyName(oPageStyleFamily, "Standard");
            FirstPageStyle = Helper.getUnoObjectbyName(oPageStyleFamily, "First Page");
        }
        catch (Exception e)
        {
            Logger.getLogger(ReportTextDocument.class.getName()).log(Level.SEVERE, null, e);
        }
    }

    public void changePageOrientation(boolean bNewLandscape)
    {
        try
        {
            com.sun.star.awt.Size oNewSize;
            getReportPageStyles();
            com.sun.star.awt.Size oLocSize = (com.sun.star.awt.Size) Helper.getUnoStructValue(ReportPageStyle, SIZE);
            bIsCurLandscape = AnyConverter.toBoolean(Helper.getUnoPropertyValue(ReportPageStyle, ISLANDSCAPE));
            if (bIsCurLandscape != bNewLandscape)
            {
                oNewSize = new com.sun.star.awt.Size(oLocSize.Height, oLocSize.Width);
                Helper.setUnoPropertyValue(ReportPageStyle, ISLANDSCAPE, bNewLandscape);
                Helper.setUnoPropertyValue(ReportPageStyle, SIZE, oNewSize);
                Helper.setUnoPropertyValue(FirstPageStyle, ISLANDSCAPE, bNewLandscape);
                Helper.setUnoPropertyValue(FirstPageStyle, SIZE, oNewSize);
                int iLeftMargin = AnyConverter.toInt(Helper.getUnoPropertyValue(ReportPageStyle, "LeftMargin"));
                int iRightMargin = AnyConverter.toInt(Helper.getUnoPropertyValue(ReportPageStyle, "RightMargin"));
                PageWidth = oNewSize.Width - iLeftMargin - iRightMargin;
                if (CurRecordTable != null)
                {
                    CurRecordTable.adjustOptimalTableWidths(xMSF, oViewHandler);
                }
            }
        }
        catch (Exception e)
        {
            Logger.getLogger(ReportTextDocument.class.getName()).log(Level.SEVERE, null, e);
        }
    }

    public boolean loadSectionsfromTemplate(String sTemplateUrl)
    {
        try
        {
            int i;
            DBColumn CurDBColumn;
            int GroupCount = CurDBMetaData.GroupFieldNames.length;
            CurRecordTable = null;
            for (i = 0; i < GroupCount; i++)
            {
                String SectionName = GROUPSECTION + Integer.toString(i + 1);
                oTextTableHandler.renameTextTable("Tbl_" + SectionName, "MyTextTable");
                oTextSectionHandler.linkSectiontoTemplate(sTemplateUrl, SectionName);
                oTextTableHandler.renameTextTable("MyTextTable", "Tbl_" + SectionName);
            }
            if (oTextSectionHandler.xTextSectionsSupplier.getTextSections().getElementNames().length > CurDBMetaData.GroupFieldNames.length)
            {
                oTextSectionHandler.linkSectiontoTemplate(sTemplateUrl, RECORDSECTION);
                CurRecordTable = new RecordTable(oTextTableHandler);
                insertColumnstoRecordTable();
            }
            for (i = 0; i < GroupCount; i++)
            {
                CurDBColumn = new DBColumn(oTextTableHandler, CurDBMetaData, CurDBMetaData.GroupFieldNames[i], i, TBLGROUPSECTION + (i + 1));
                CurDBColumn.formatValueCell();
                DBColumnsVector.set(i, CurDBColumn);
                replaceFieldValueInGroupTable(CurDBColumn, i);
            }
            return true;
        }
        catch (Exception e)
        {
            Logger.getLogger(ReportTextDocument.class.getName()).log(Level.SEVERE, null, e);
            return false;
        }
    }

    public void setupRecordSection(String TemplateName)
    {
        this.ContentTemplatePath = TemplateName;
        if (CurDBMetaData.getRecordFieldNames().length > 0)
        {
            boolean bAddParagraph = true;
            if (CurDBMetaData.GroupFieldNames != null)
            {
                bAddParagraph = (CurDBMetaData.GroupFieldNames.length == 0);
            }
            oTextSectionHandler.insertTextSection(RECORDSECTION, TemplateName, bAddParagraph);
            CurRecordTable = new RecordTable(oTextTableHandler);
            insertColumnstoRecordTable();
            if (CurRecordTable != null)
            {
                CurRecordTable.adjustOptimalTableWidths(xMSF, oViewHandler);
            }
        }
        else
        {
            CurRecordTable = null;
        }
    }

    public void refreshGroupFields(String[] _sNewNames)
    {
        int nSize = DBColumnsVector.size();
        for (int i = 0; i < nSize; i++)
        {
            DBColumn CurDBColumn = DBColumnsVector.get(i);
            String sFieldName = CurDBColumn.CurDBField.getFieldName();
            if (!sFieldName.equals(_sNewNames[i]))
            {
                CurDBColumn.CurDBField = CurDBMetaData.getFieldColumnByDisplayName(_sNewNames[i]);
                CurDBColumn.insertColumnData(oTextFieldHandler, bIsCurLandscape);
            }
        }
    }
    //public boolean isGroupField(String _FieldName)
    //    {
    //        return (JavaTools.FieldInList(CurDBMetaData.GroupFieldNames, _FieldName) != -1);
    //    }

    public void replaceFieldValueInRecordSection(int RecordCount)
    {
        int GroupCount = CurDBMetaData.GroupFieldNames.length;
        int FieldCount = CurDBMetaData.getFieldNames().length;
        for (int i = GroupCount; i < FieldCount; i++)
        {
            DBColumnsVector.get(i).insertColumnData(oTextFieldHandler, this.bIsCurLandscape);
        }
    }

    public void updateTextSections(String[] SelGroupNames) throws Exception
    {
        String TableName;
        DBColumn OldDBColumn;
        DBColumn CurDBColumn;
        // XNameAccess xTableNames = oTextTableHandler.xTextTablesSupplier.getTextTables();
        int GroupFieldCount = SelGroupNames.length;
        for (int i = 0; i < GroupFieldCount; i++)
        {
            TableName = TBLGROUPSECTION + Integer.toString(i + 1);
            OldDBColumn = DBColumnsVector.get(i);
            CurDBColumn = new DBColumn(oTextTableHandler, CurDBMetaData, SelGroupNames[i], i, TableName, OldDBColumn);
            CurDBColumn.formatValueCell();
            DBColumnsVector.set(i, CurDBColumn);
            CurDBColumn.insertColumnData(oTextFieldHandler, this.bIsCurLandscape);
        }
    }

    public void replaceFieldValueInGroupTable(DBColumn CurDBColumn, int TableIndex)
    {
        String TableName = TBLGROUPSECTION + (TableIndex + 1);
        // Note: for some reason the table might lose its name and has to be renamed therefor
        String OldTableName = CurDBColumn.xTableName.getName();
        if (OldTableName.compareTo(TableName) != 0)
        {
            CurDBColumn.xTableName.setName(TableName);
        }
        CurDBColumn.insertColumnData(oTextFieldHandler, this.bIsCurLandscape);
        CurDBColumn.setCellFont();
    }

    public void replaceFieldValueInRecordTable()
    {
        String TableName = TBLRECORDSECTION;
        String OldTableName = CurRecordTable.xTableName.getName();
        if (OldTableName.compareTo(TableName) != 0)
        {
            CurRecordTable.xTableName.setName(TableName);
        }
        int GroupCount = CurDBMetaData.GroupFieldNames.length;
        int RecordCount = CurDBMetaData.getRecordFieldNames().length;
        for (int i = GroupCount; i < RecordCount; i++)
        {
            DBColumnsVector.get(i).insertColumnData(oTextFieldHandler, this.bIsCurLandscape);
        }
    }

    public void insertColumnstoRecordTable()
    {
        try
        {
            int GroupCount = CurDBMetaData.GroupFieldNames.length;
            DBColumn CurDBColumn;
            // Note for some reason the table might lose its name and has to be renamed therefor
            String OldTableName = CurRecordTable.xTableName.getName();
            if (OldTableName.compareTo(TBLRECORDSECTION) != 0)
            {
                CurRecordTable = new RecordTable(oTextTableHandler);
            }
            com.sun.star.table.XTableColumns xColumns = CurRecordTable.xTextTable.getColumns();
            int ColCount = xColumns.getCount();
            int RecordCount = CurDBMetaData.getRecordFieldNames().length;
            if (ColCount > RecordCount)
            {
                int RemoveCount = ColCount - RecordCount;
                xColumns.removeByIndex(0, RemoveCount);
            }
            else if (ColCount < RecordCount)
            {
                int AddCount = RecordCount - ColCount;
                CurRecordTable.xTextTable.getColumns().insertByIndex(ColCount, AddCount);
            }
            for (int i = 0; i < RecordCount; i++)
            {
                CurDBColumn = new DBColumn(CurRecordTable, oTextTableHandler, CurDBMetaData, i, true);
                CurDBColumn.initializeNumberFormat();
                CurDBColumn.insertColumnData(oTextFieldHandler, this.bIsCurLandscape);
                if (DBColumnsVector.size() <= (i + GroupCount))
                {
                    DBColumnsVector.add(CurDBColumn);
                }
                else
                {
                    DBColumnsVector.set(i + GroupCount, CurDBColumn);
                }
            }
        }
        catch (Exception exception)
        {
            showCommonReportErrorBox(exception);
        }
    }

    public boolean addGroupNametoDocument(String[] GroupNames, String CurGroupTitle, ArrayList<String> GroupFieldVector, ArrayList<String> ReportPath, int iSelCount)
    {
        DBColumn CurDBColumn = null;
        int GroupCount = GroupFieldVector.size();
        if (GroupCount < 4)
        {
            removeGroupNamesofRecordTable(iSelCount);
            FieldColumn CurFieldColumn = CurDBMetaData.getFieldColumnByTitle(CurGroupTitle);
            GroupFieldVector.add(CurFieldColumn.getFieldName());
            GroupCount += 1;
            try
            {
                String sPath = FileAccess.getPathFromList(xMSF, ReportPath, "cnt-default.ott");
                oTextSectionHandler.insertTextSection(GROUPSECTION + GroupCount, sPath, GroupCount == 1);
                CurDBColumn = new DBColumn(oTextTableHandler, CurDBMetaData, CurFieldColumn.getFieldName(), GroupCount - 1, TBLGROUPSECTION + (GroupCount));
                CurDBColumn.formatValueCell();
                DBColumnsVector.add(CurDBColumn);
                replaceFieldValueInGroupTable(CurDBColumn, GroupCount - 1);
            }
            catch (Exception exception)
            {
                showCommonReportErrorBox(exception);
            }

        }

        return (CurDBColumn != null);
    }

    public void removeGroupName(String[] NewSelGroupNames, String CurGroupTitle, java.util.ArrayList<String> GroupFieldVector)
    {
        removeGroupNamesofRecordTable(NewSelGroupNames.length + 1);
        FieldColumn CurFieldColumn = CurDBMetaData.getFieldColumnByTitle(CurGroupTitle);
        GroupFieldVector.remove(CurFieldColumn.getFieldName());
        try
        {
            oTextSectionHandler.removeLastTextSection();
            oTextTableHandler.removeLastTextTable();
            // if the previously selected item is somewhere in the middle of the listbox (and not at the end) the
            // Textsections have to be updated
            if (JavaTools.FieldInList(NewSelGroupNames, CurGroupTitle) == -1)
            {
                updateTextSections(NewSelGroupNames);
            }
            int iSelItemCount = NewSelGroupNames.length;
            DBColumnsVector.remove(iSelItemCount);
        }
        catch (Exception exception)
        {
            showCommonReportErrorBox(exception);
        }
    }

    public void removeGroupNamesofRecordTable(int GroupFieldCount)
    {
        int CurFieldCount = DBColumnsVector.size();
        if (CurFieldCount > GroupFieldCount)
        {
            for (int i = CurFieldCount - 1; i >= GroupFieldCount; i--)
            {
                DBColumnsVector.remove(i);
            }
        }
    }

    public void showCommonReportErrorBox(Exception exception)
    {
        String SystemContentPath = JavaTools.convertfromURLNotation(ContentTemplatePath);
        String sMsgCurCommonReportError = JavaTools.replaceSubString(sMsgCommonReportError, SystemContentPath, "%PATH");
        CurDBMetaData.showMessageBox("ErrorBox", VclWindowPeerAttribute.OK, sMsgCurCommonReportError);
        exception.printStackTrace(System.err);
    }

    public void getallDBColumns()
    {
        try
        {
            DBColumn CurDBColumn;
            for (int i = 0; i < CurDBMetaData.FieldColumns.length; i++)
            {
                if (i < CurDBMetaData.GroupFieldNames.length)
                {
                    CurDBColumn = new DBColumn(oTextTableHandler, CurDBMetaData, CurDBMetaData.GroupFieldNames[i], i, COPYOFTBLGROUPSECTION + (i + 1));
                }
                else
                {
                    if (CurRecordTable == null)
                    {
                        CurRecordTable = new RecordTable(oTextTableHandler);
                    }
                    CurDBColumn = new DBColumn(CurRecordTable, oTextTableHandler, CurDBMetaData, i - CurDBMetaData.GroupFieldNames.length);
                }
                if (CurDBColumn.xNameCell != null)
                {
                    DBColumnsVector.add(CurDBColumn);
                }
                else
                {
                    String DelFieldName;
                    if (i < CurDBMetaData.GroupFieldNames.length)
                    {
                        DelFieldName = CurDBMetaData.GroupFieldNames[i];
                        CurDBMetaData.GroupFieldNames = JavaTools.removefromList(CurDBMetaData.GroupFieldNames, new String[]
                                {
                                    DelFieldName
                                });
                        CurDBMetaData.GroupFieldColumns = removeFieldColumnByFieldName(DelFieldName, CurDBMetaData.GroupFieldColumns);
                    }
                    else
                    {
                        DelFieldName = CurDBMetaData.getRecordFieldName(i - CurDBMetaData.GroupFieldNames.length);
                        String[] aNewList = JavaTools.removefromList(CurDBMetaData.getRecordFieldNames(), new String[]
                                {
                                    DelFieldName
                                });
                        CurDBMetaData.setRecordFieldNames(aNewList);
                        CurDBMetaData.RecordFieldColumns = removeFieldColumnByFieldName(DelFieldName, CurDBMetaData.RecordFieldColumns);
                        CurDBMetaData.FieldColumns = removeFieldColumnByFieldName(DelFieldName, CurDBMetaData.FieldColumns);

                    }
                    i--;
                }
            }
            java.util.Arrays.sort(CurDBMetaData.RecordFieldColumns, this);
        }
        catch (Exception exception)
        {
            showCommonReportErrorBox(exception);
        }
    }

    public int compare(FieldColumn oFieldColumn1, FieldColumn oFieldColumn2)
    {
        DBColumn oDBColumn1 = getDBColumnByName(oFieldColumn1.getFieldName());
        DBColumn oDBColumn2 = getDBColumnByName(oFieldColumn2.getFieldName());
        if (oDBColumn1.ValColumn < oDBColumn2.ValColumn)
        {
            return -1;
        }
        else if (oDBColumn1.ValColumn == oDBColumn2.ValColumn)
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }

    private DBColumn getDBColumnByName(String _FieldName)
    {
        for (int i = 0; i < DBColumnsVector.size(); i++)
        {
            DBColumn oDBColumn = DBColumnsVector.get(i);
            if (oDBColumn.CurDBField.getFieldName().equals(_FieldName))
            {
                return oDBColumn;
            }
        }
        return null;
    }

    public static FieldColumn[] removeFieldColumnByFieldName(String _FieldName, FieldColumn[] _FieldColumns)
    {
        try
        {
            ArrayList<FieldColumn> aFieldColumns = new ArrayList<FieldColumn>();
            for (int i = 0; i < _FieldColumns.length; i++)
            {
                FieldColumn CurFieldColumn = _FieldColumns[i];
                if (!CurFieldColumn.getFieldName().equals(_FieldName))
                {
                    aFieldColumns.add(CurFieldColumn);
                }
            }
            FieldColumn[] aRetList = new FieldColumn[aFieldColumns.size()];
            aFieldColumns.toArray(aRetList);
            return aRetList;
        }
        catch (RuntimeException e)
        {
            Logger.getLogger(ReportTextDocument.class.getName()).log(Level.SEVERE, null, e);
            return null;
        }
    }

    public void removeAllVisibleTextSections()
    {
        int GroupCount = CurDBMetaData.GroupFieldNames.length;
        String[] sInvisibleSectionNames = new String[GroupCount + 1];
        sInvisibleSectionNames[0] = RECORDSECTION;
        for (int i = 1; i <= GroupCount; i++)
        {
            sInvisibleSectionNames[i] = GROUPSECTION + i;
        }
        XNameAccess xNameAccessTextSections = UnoRuntime.queryInterface(XNameAccess.class, oTextSectionHandler.xTextSectionsSupplier.getTextSections());
        String[] sSectionNames = xNameAccessTextSections.getElementNames();
        for (int i = 0; i < sSectionNames.length; i++)
        {
            String sSectionName = sSectionNames[i];
            if (JavaTools.FieldInList(sInvisibleSectionNames, sSectionName) < 0)
            {
                oTextSectionHandler.removeTextSectionbyName(sSectionName);
            }
        }
    }

    private String[] getLayoutTextTableNames()
    {
        int GroupCount = CurDBMetaData.GroupFieldNames.length;
        String[] sLayoutTableNames = new String[GroupCount + 1];
        for (int i = 0; i < GroupCount; i++)
        {
            sLayoutTableNames[i] = TBLGROUPSECTION + (i + 1);
        }
        sLayoutTableNames[GroupCount] = TBLRECORDSECTION;
        return sLayoutTableNames;
    }

    public void removeNonLayoutTextTables()
    {
        String[] sLayoutTableNames = getLayoutTextTableNames();
        XNameAccess xNameAccessTextTables = UnoRuntime.queryInterface(XNameAccess.class, oTextTableHandler.xTextTablesSupplier.getTextTables());
        String[] sTableNames = xNameAccessTextTables.getElementNames();
        for (int i = 0; i < sTableNames.length; i++)
        {
            String sTableName = sTableNames[i];
            if (JavaTools.FieldInList(sLayoutTableNames, sTableName) < 0)
            {
                oTextTableHandler.removeTextTablebyName(sTableName);
            }
        }
    }

    public void removeLayoutTextTables()
    {
        String[] sLayoutTableNames = getLayoutTextTableNames();
        XNameAccess xNameAccessTextTables = UnoRuntime.queryInterface(XNameAccess.class, oTextTableHandler.xTextTablesSupplier.getTextTables());
        XRelativeTextContentRemove xRelativeTextContentRemove = UnoRuntime.queryInterface(XRelativeTextContentRemove.class, xText);
        String[] sTableNames = xNameAccessTextTables.getElementNames();
        for (int i = 0; i < sTableNames.length; i++)
        {
            String sTableName = sTableNames[i];
            if (JavaTools.FieldInList(sLayoutTableNames, sTableName) > -1)
            {
                if (!sTableName.equals(sLayoutTableNames[0]))
                {
                    XTextContent xTextContent = UnoRuntime.queryInterface(XTextContent.class, oTextTableHandler.getByName(sTableName));
                    boolean bleaveloop = false;
                    while (!bleaveloop)
                    {
                        try
                        {
//                                xRelativeTextContentRemove.removeTextContentBefore(xTextContent);
                            xRelativeTextContentRemove.removeTextContentAfter(xTextContent);
                            // IllegalArgumentException may be thrown when no paragraphbreak is there
                        }
                        catch (IllegalArgumentException iexception)
                        {
                            bleaveloop = true;
                        }
                    }
                }
                oTextTableHandler.removeTextTablebyName(sTableName);
            }
        }
    }

    public void setLayoutSectionsVisible(boolean _IsVisible)
    {
        try
        {
            XNameAccess xTextSections = oTextSectionHandler.xTextSectionsSupplier.getTextSections();
            Object oTextSection;
            int GroupFieldCount = CurDBMetaData.GroupFieldNames.length;
            for (int i = 0; i < GroupFieldCount; i++)
            {
                oTextSection = xTextSections.getByName(GROUPSECTION + String.valueOf(i + 1));
                Helper.setUnoPropertyValue(oTextSection, ISVISIBLE, _IsVisible);
            }
            if (xTextSections.hasByName(RECORDSECTION))
            {
                oTextSection = xTextSections.getByName(RECORDSECTION);
                Helper.setUnoPropertyValue(oTextSection, ISVISIBLE, _IsVisible);
            }
        }
        catch (Exception e)
        {
            Logger.getLogger(ReportTextDocument.class.getName()).log(Level.SEVERE, null, e);
        }
    }

    public void removeCopiedTextSections()
    {
        int GroupCount = CurDBMetaData.GroupFieldNames.length;
        String[] sCopyTextSections = new String[GroupCount + 1];
        String[] sCopyTextTables = new String[GroupCount + 1];
        sCopyTextSections[0] = COPYOFRECORDSECTION;
        sCopyTextTables[0] = COPYOFTBLRECORDSECTION;
        for (int i = 1; i <= GroupCount; i++)
        {
            sCopyTextSections[i] = COPYOFGROUPSECTION + (i);
            sCopyTextTables[i] = COPYOFTBLGROUPSECTION + (i);
        }
        for (int i = 0; i <= GroupCount; i++)
        {
            oTextTableHandler.removeTextTablebyName(sCopyTextTables[i]);
            oTextSectionHandler.removeTextSectionbyName(sCopyTextSections[i]);
        }
    }
}
