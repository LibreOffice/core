 /*************************************************************************
 *
 *  $RCSfile: Dataimport.java,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: bc $ $Date: 2002-08-30 16:57:39 $
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


package com.sun.star.wizards.report;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XElementAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNamed;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.container.XEnumeration;
import com.sun.star.container.XNameContainer;
import com.sun.star.container.XIndexContainer;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XMultiPropertySet;

import com.sun.star.comp.servicemanager.ServiceManager;
import com.sun.star.connection.XConnector;
import com.sun.star.connection.XConnection;
import com.sun.star.sdb.XCompletedConnection;
import com.sun.star.sdbcx.XColumnsSupplier;
import com.sun.star.sdbc.*;
import com.sun.star.sdb.XColumn;
import com.sun.star.sdb.*;
import com.sun.star.sdbc.XColumnLocate;
import com.sun.star.sdbcx.XTablesSupplier;
import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.XNamingService;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Any;
import com.sun.star.uno.*;
import com.sun.star.sheet.*;
import com.sun.star.document.XDocumentInfoSupplier;
import com.sun.star.document.XDocumentInfo;
import com.sun.star.document.XEventsSupplier;
import com.sun.star.document.*;
import com.sun.star.table.*;
import com.sun.star.sdb.*;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.text.XTextFieldsSupplier;
import com.sun.star.text.XTextRange;
import com.sun.star.text.XTextTablesSupplier;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextSectionsSupplier;
import com.sun.star.text.XTextViewCursorSupplier;
import com.sun.star.text.XTextViewCursor;
import com.sun.star.text.XSimpleText;
import com.sun.star.text.XTextViewCursorSupplier;
import com.sun.star.text.XPageCursor;
import com.sun.star.text.XText;
import com.sun.star.table.XCellRange;
import com.sun.star.task.XInteractionHandler;
import com.sun.star.task.XStatusIndicatorFactory;
import com.sun.star.task.XStatusIndicator;
import com.sun.star.util.XLinkUpdate;
import com.sun.star.util.XNumberFormats;
import com.sun.star.util.XNumberFormatsSupplier;
import com.sun.star.util.NumberFormat;
import com.sun.star.util.XRefreshable;
import com.sun.star.view.XViewSettingsSupplier;
import com.sun.star.form.XFormsSupplier;
import com.sun.star.drawing.XDrawPageSupplier;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.lang.Locale;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.lang.*;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.EventObject;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XFramesSupplier;
import com.sun.star.frame.XModel;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextTable;
import com.sun.star.text.TableColumnSeparator;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.script.XInvocation;
import com.sun.star.style.XStyleFamiliesSupplier;
import com.sun.star.style.XStyleLoader;
import com.sun.star.style.TabStop;
import com.sun.star.frame.XFrame;
import com.sun.star.awt.XListBox;
import com.sun.star.awt.XToolkit;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.awt.Size;
import com.sun.star.awt.XTextComponent;
import com.sun.star.awt.XWindow;
import com.sun.star.awt.ActionEvent;
import com.sun.star.awt.XActionListener;



public class Dataimport extends ReportWizard{

    UNODialogs CurUNOProgressDialog;
    static boolean bStopProcess;


    public Dataimport() {
    }

    class ActionListenerImpl implements com.sun.star.awt.XActionListener {

        public void disposing(EventObject eventObject) {
        }
        public void actionPerformed(ActionEvent actionEvent){
        bStopProcess = true;
        }
    }


    public static void main (String args[])
    {
        String ConnectStr = "uno:socket,host=localhost,port=8100;urp,negotiate=0,forcesynchronous=1;StarOffice.NamingService";
        XMultiServiceFactory xMSF = null;
        try {
            xMSF = tools.connect(ConnectStr);
        if( xMSF != null )  System.out.println("Connected to "+ ConnectStr );
        Dataimport CurDataimport = new Dataimport();
        CurDataimport.createReport(xMSF);
        }
        catch(com.sun.star.uno.Exception e) {
            e.printStackTrace(System.out);
        }
        catch(java.lang.Exception javaexception ){
        javaexception.printStackTrace(System.out);
    }
    }


    public void createReport(XMultiServiceFactory xMSF){
    try{
    ReportDocument CurReportDocument;
    DBMetaData CurDBMetaData;
    CurReportDocument = new ReportDocument(xMSF, false, true);
    int iWidth = CurReportDocument.Frame.getComponentWindow().getPosSize().Width;
    CurUNOProgressDialog = showProgressDisplay(xMSF, CurReportDocument, true);
    CurDBMetaData = new DBMetaData();
    if (reconnectToDatabase(xMSF, CurDBMetaData, CurReportDocument)){
        getGroupFieldFortmats(xMSF, CurReportDocument, CurDBMetaData);
        CurUNOProgressDialog.modifyFontWeight("lblProgressDBConnection", com.sun.star.awt.FontWeight.NORMAL);
        CurUNOProgressDialog.modifyFontWeight("lblProgressDataImport", com.sun.star.awt.FontWeight.BOLD);
        insertDatabaseDatatoReportDocument(xMSF, CurReportDocument, CurUNOProgressDialog);
    }
    CurUNOProgressDialog.xComponent.dispose();
    }
    catch(java.lang.Exception jexception ){
    jexception.printStackTrace(System.out);
    }}


    public String getValueofHiddenControl(XMultiServiceFactory xMSF, XNameAccess xNamedForm, String ControlName, ReportDocument CurReportDocument, boolean[] bgoOn){
    try{
    if (bgoOn[0] == true){
        String ControlValue = (String) tools.getUNOPropertyValue(xNamedForm.getByName(ControlName), "HiddenValue");
        return ControlValue;
    }
    else
        return null;
    }
    catch(com.sun.star.uno.Exception exception){
    sMsgHiddenControlMissing = tools.replaceSubString(sMsgHiddenControlMissing, SOREPORTFORMNAME, "<REPORTFORM>");
    sMsgHiddenControlMissing = tools.replaceSubString(sMsgHiddenControlMissing, ControlName, "<CONTROLNAME>");
    UNODialogs.showMessageBox(xMSF, CurReportDocument.Frame, "ErrorBox", com.sun.star.awt.VclWindowPeerAttribute.OK, sMsgHiddenControlMissing + (char) 13 + sMsgEndAutopilot);
    bgoOn[0] = false;
    return null;
    }}


    public boolean reconnectToDatabase(XMultiServiceFactory xMSF, DBMetaData CurDBMetaData, ReportDocument CurReportDocument){
    try{
    XNameContainer xNamedForms = CurReportDocument.getDocumentForms();
    Object oDBForm = tools.getUNOObjectbyName(xNamedForms, SOREPORTFORMNAME);
    boolean bgetConnection;
    if (oDBForm != null){
        boolean[] bgoOn = new boolean[1];
        bgoOn[0] = true;
        XNameAccess xNamedForm = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, oDBForm);
        CurDBMetaData.DataSourceName = (String) getValueofHiddenControl(xMSF, xNamedForm, "DataSourceName", CurReportDocument, bgoOn);
        CurDBMetaData.Command = getValueofHiddenControl(xMSF, xNamedForm, "Command", CurReportDocument, bgoOn);
        String sCommandType = getValueofHiddenControl(xMSF, xNamedForm, "CommandType", CurReportDocument, bgoOn);
        String sGroupFieldNames = getValueofHiddenControl(xMSF, xNamedForm, "GroupFieldNames", CurReportDocument, bgoOn);
        String sFieldNames = getValueofHiddenControl(xMSF, xNamedForm, "FieldNames", CurReportDocument, bgoOn);
        String sRecordFieldNames = getValueofHiddenControl(xMSF, xNamedForm, "RecordFieldNames", CurReportDocument, bgoOn);
        CurDBMetaData.FieldNames = tools.ArrayoutofString(sFieldNames,";");
        CurDBMetaData.RecordFieldNames = tools.ArrayoutofString(sRecordFieldNames,";");
        CurDBMetaData.GroupFieldNames = tools.ArrayoutofString(sGroupFieldNames,";");
        CurDBMetaData.CommandType = Integer.valueOf(sCommandType).intValue();
        sMsgQueryCreationImpossible = tools.replaceSubString(sMsgQueryCreationImpossible, CurDBMetaData.Command, "<STATEMENT>");
        bgetConnection = CurDBMetaData.getConnection(xMSF, CurReportDocument, CurDBMetaData.DataSourceName, sMsgNoConnection, sMsgConnectionImpossible);
        if (bgoOn[0] == false)
        return false;
        if (bgetConnection){
        boolean bexecute = CurDBMetaData.executeCommand(xMSF, CurReportDocument.Frame, sMsgQueryCreationImpossible + (char) 13 + sMsgEndAutopilot);
        if (bexecute){;
            XColumnsSupplier xDBCols = (XColumnsSupplier) UnoRuntime.queryInterface(XColumnsSupplier.class, CurDBMetaData.ResultSet);
            CurDBMetaData.xColumns = xDBCols.getColumns();
        }
        return bexecute;
        }
        else
        return false;
    }
    else{
        sReportFormNotExisting = tools.replaceSubString(sReportFormNotExisting, SOREPORTFORMNAME, "<REPORTFORM>");
        UNODialogs.showMessageBox(xMSF, CurReportDocument.Frame, "ErrorBox", com.sun.star.awt.VclWindowPeerAttribute.OK, sReportFormNotExisting + (char) 13 + sMsgEndAutopilot);
        return false;
    }
    }

//    catch(com.sun.star.uno.Exception exception ){
//        exception.printStackTrace(System.out);

//  UNODialogs.showMessageBox(xMSF, CurReportDocument.Frame, "ErrorBox", com.sun.star.awt.VclWindowPeerAttribute.OK_CANCEL, sMsgNoConnectionforDataimport);
//  "Fehler beim Auslesen der Reportdaten: Verstecktes Control """"GroupFieldNames"""" enthält fehlerhafte Daten"
//  return false;
//    }
    catch(java.lang.Exception javaexception ){
        javaexception.printStackTrace(System.out);
    return false;
    }}


    public void insertDatabaseDatatoReportDocument(XMultiServiceFactory xMSF, ReportDocument CurReportDocument, UNODialogs CurUNOProgressDialog){
    try{
        int ColIndex;
        int i;
    boolean breset;
    Object oTextTable;
    XCellRange xCellRange;
    XTextCursor xTextCursor;
    XTextDocument xTextDocument;
    java.util.Vector DataVector = new java.util.Vector();
    ReportDocument.GroupFieldFormat CurGroupFieldFormat;
    String CurGroupValue;
    String CurGroupTableName;
    com.sun.star.sdbc.XRow xResultSetRow;
    DBMetaData CurDBMetaData = CurReportDocument.CurDBMetaData;
        int GroupFieldCount = CurDBMetaData.GroupFieldNames.length;
    int FieldCount = CurDBMetaData.FieldNames.length;
    String[] OldGroupFieldValues = new String[GroupFieldCount];
    XTextTable[] xGroupBaseTables = new XTextTable[GroupFieldCount];
    int RecordFieldCount = FieldCount - GroupFieldCount;
    int[] SelColIndices = null;
    int[] GroupColIndices = null;
    int iCommandType = CurDBMetaData.CommandType;
    if ((iCommandType == com.sun.star.sdb.CommandType.QUERY) || (iCommandType == com.sun.star.sdb.CommandType.COMMAND)){
        SelColIndices = CurDBMetaData.getSelectedQueryFields(CurDBMetaData.RecordFieldNames);
        GroupColIndices = CurDBMetaData.getSelectedQueryFields(CurDBMetaData.GroupFieldNames);
    }
    xResultSetRow = (com.sun.star.sdbc.XRow) UnoRuntime.queryInterface(com.sun.star.sdbc.XRow.class, CurDBMetaData.ResultSet);
    XNameAccess xTextTables = CurReportDocument.TextTablesSupplier.getTextTables();
    xTextDocument = CurReportDocument.ReportTextDocument;
        xTextCursor = CurReportDocument.createTextCursor(CurReportDocument.ReportTextDocument.getText());
    XFrame xFrame = CurReportDocument.Frame;
    xTextDocument.lockControllers();
        if (CurDBMetaData.ResultSet.next() == true){
        tools.setUNOPropertyValue(xTextCursor, "PageDescName", "First Page");
        for (ColIndex = 0; ColIndex < GroupFieldCount; ColIndex++){
            CurGroupTableName = "Tbl_GroupField" + Integer.toString(ColIndex+1);
            xGroupBaseTables[ColIndex] = (XTextTable) CurReportDocument.TextTablesSupplier.getTextTables().getByName(CurGroupTableName);
            OldGroupFieldValues[ColIndex] = CurDBMetaData.getColumnValue(xResultSetRow, iCommandType, GroupColIndices, ColIndex);
            CurGroupValue = CurDBMetaData.getColumnValue(xResultSetRow, iCommandType, GroupColIndices, ColIndex);
        CurGroupFieldFormat = (ReportDocument.GroupFieldFormat) CurReportDocument.GroupFormatVector.elementAt(ColIndex);
        addLinkedTextSection(CurReportDocument, xTextCursor, "GroupField" + Integer.toString(ColIndex+1), CurGroupFieldFormat, CurGroupValue);
        }
        if (CurDBMetaData.getcurrentRecordData(xMSF, xResultSetRow, xFrame, ColIndex, FieldCount, RecordFieldCount, SelColIndices, iCommandType, DataVector, sMsgQueryCreationImpossible) == true){
        int RowIndex = 1;
        bStopProcess = false;
        while (CurDBMetaData.ResultSet.next() == true){
            if (bStopProcess == true){
            xTextDocument.unlockControllers();
            return;
            }
            RowIndex += 1;
            breset = false;
            for (ColIndex = 0; ColIndex < GroupFieldCount; ColIndex++){
            CurGroupValue = CurDBMetaData.getColumnValue(xResultSetRow, iCommandType, GroupColIndices, ColIndex);
            if ((CurGroupValue.equals((Object) OldGroupFieldValues[ColIndex]) == false) || (breset)){
                breset = true;
                insertDataToRecordTable(CurReportDocument, xTextCursor, DataVector, RecordFieldCount);
                CurGroupFieldFormat = (ReportDocument.GroupFieldFormat) CurReportDocument.GroupFormatVector.elementAt(ColIndex);
                addLinkedTextSection(CurReportDocument, xTextCursor, "GroupField" + Integer.toString(ColIndex+1), CurGroupFieldFormat, CurGroupValue);
                OldGroupFieldValues[ColIndex] = CurGroupValue;
                breset = !(ColIndex == GroupFieldCount-1);
            }
            }
            CurDBMetaData.getcurrentRecordData(xMSF, xResultSetRow, xFrame, ColIndex, FieldCount, RecordFieldCount, SelColIndices, iCommandType, DataVector, sMsgQueryCreationImpossible);
            updateProgressDisplay(RowIndex, CurUNOProgressDialog);
        }
        insertDataToRecordTable(CurReportDocument, xTextCursor, DataVector, RecordFieldCount);
        }
        else{
        xTextDocument.unlockControllers();
        return;
        }
        }
    setLayoutSectionsInvisible(CurReportDocument.TextSectionsSupplier, GroupFieldCount);
    CurReportDocument.breakLinkofTextSections();
    }
    catch( com.sun.star.uno.Exception exception ){
        exception.printStackTrace(System.out);
    }

    catch(java.lang.Exception javaexception ){
        javaexception.printStackTrace(System.out);
    }
    CurReportDocument.ReportTextDocument.unlockControllers();
    }


    public void setLayoutSectionsInvisible(XTextSectionsSupplier xTextSectionsSupplier, int GroupFieldCount){
    try{
    XNameAccess xTextSections = xTextSectionsSupplier.getTextSections();
    Object oTextSection;
    for (int i = 0; i< GroupFieldCount; i++){
        oTextSection = xTextSections.getByName("GroupField" + String.valueOf(i+1));
        tools.setUNOPropertyValue(oTextSection, "IsVisible", new Boolean(false));
    }
    if (xTextSections.hasByName("RecordSection")){
        oTextSection = xTextSections.getByName("RecordSection");
        tools.setUNOPropertyValue(oTextSection, "IsVisible", new Boolean(false));
    }
    }
    catch( com.sun.star.uno.Exception exception ){
        exception.printStackTrace(System.out);
    }}


    public void insertDataToRecordTable(ReportDocument CurReportDocument, XTextCursor xTextCursor, java.util.Vector DataVector, int FieldCount){
    try{
    int DataLength = DataVector.size();
    if ((FieldCount > 0)&& (DataLength > 0)){
        addLinkedTextSection(CurReportDocument, xTextCursor, "RecordSection", null, null);
        String[][] RecordArray = new String[DataLength][FieldCount];
        DataVector.copyInto(RecordArray);
        XTextTable xTextTable = getlastTextTable(CurReportDocument.ReportTextDocument);
        if (DataLength > 1){
        xTextTable.getRows().insertByIndex(xTextTable.getRows().getCount(), DataLength - 1);
        }
        XCellRange xCellRange = (XCellRange) UnoRuntime.queryInterface(XCellRange.class, xTextTable);
        int iRowCount = xTextTable.getRows().getCount();
        int iColCount = xTextTable.getColumns().getCount();

        XCellRange xNewRange = xCellRange.getCellRangeByPosition(0,1,FieldCount -1, DataLength);
        XCellRangeData xDataArray = (XCellRangeData) UnoRuntime.queryInterface(XCellRangeData.class, xNewRange);
        xDataArray.setDataArray(RecordArray);
    }
    DataVector.removeAllElements();
    }
    catch( com.sun.star.uno.Exception exception ){
    exception.printStackTrace(System.out);
     }}


    public void addDBRecordstoReportDocument(ReportDocument CurReportDocument, XResultSet xResultSet, String[] FieldNames)
    throws com.sun.star.sdbc.SQLException, com.sun.star.container.NoSuchElementException, com.sun.star.lang.IllegalArgumentException,
        com.sun.star.lang.WrappedTargetException{
    int iColIndex;
    int FieldCount = FieldNames.length;
    com.sun.star.sdbc.XRow xResultSetRow;
    String sValue;
    String sResultSet = null;
    CurReportDocument.ReportTextDocument.lockControllers();
    xResultSetRow = (com.sun.star.sdbc.XRow) UnoRuntime.queryInterface(com.sun.star.sdbc.XRow.class, xResultSet);
    XTextCursor xTextCursor = CurReportDocument.createTextCursor(CurReportDocument.ReportTextDocument.getText());
    while (xResultSet.next() == true){
        sResultSet = "";
        for (int i=0; i<FieldCount;i++){
        sValue = xResultSetRow.getString(i+1);
        if (xResultSetRow.wasNull() == false){
            sResultSet += sValue;
        }
        if (i < FieldCount-1)
            sResultSet += (char) (9);
        }
        xTextCursor.setString(sResultSet);
        xTextCursor.collapseToEnd();
        CurReportDocument.ReportTextDocument.getText().insertControlCharacter(xTextCursor, com.sun.star.text.ControlCharacter.PARAGRAPH_BREAK, false);
    }
       CurReportDocument.ReportTextDocument.unlockControllers();
    }


    public void updateProgressDisplay(int iCounter, UNODialogs CurUNOProgressDialog){
    try{
    if (iCounter % 10 == 0){
        sProgressCurRecord = tools.replaceSubString(sProgressBaseCurRecord, String.valueOf(iCounter), "<COUNT>");
            CurUNOProgressDialog.assignPropertyToDialogControl("lblCurProgress", "Label", sProgressCurRecord);
    }
    }
    catch(com.sun.star.uno.Exception exception)
    {
        exception.printStackTrace(System.out);
    }
    catch(java.lang.Exception jexception ){
    jexception.printStackTrace(System.out);
    }}


    public UNODialogs showProgressDisplay(XMultiServiceFactory xMSF, ReportDocument CurReportDocument, boolean bgetConnection){
    try{
    int iHeight;
    ReportWizard.getReportResources(xMSF, true);
    UNODialogs CurUNOProgressDialog = new UNODialogs(xMSF, new String[] {"Height", "Step", "Title", "Width"},
                            new Object[] {new Integer(84), new Integer(0), sProgressTitle, new Integer(180)});
        com.sun.star.awt.FontDescriptor oFontDesc = new com.sun.star.awt.FontDescriptor();
        oFontDesc.Weight = com.sun.star.awt.FontWeight.BOLD;

    if (bgetConnection){
        CurUNOProgressDialog.insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblProgressDBConnection",
                            new String[] {"FontDescriptor", "Height", "Label", "PositionX", "PositionY", "Step", "Width"},
                            new Object[] {oFontDesc, new Integer(10), sProgressDBConnection, new Integer(6), new Integer(6), new Integer(0), new Integer(150)});

        CurUNOProgressDialog.insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblProgressDataImport",
                            new String[] {"Height", "Label", "PositionX", "PositionY", "Step", "Width"},
                            new Object[] {new Integer(10), sProgressDataImport, new Integer(6), new Integer(24), new Integer(0), new Integer(120)});

    }
    else{
        CurUNOProgressDialog.insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblProgressDataImport",
                        new String[] {"FontDescriptor", "Height", "Label", "PositionX", "PositionY", "Step", "Width"},
                        new Object[] {oFontDesc, new Integer(10), sProgressDataImport, new Integer(6), new Integer(24), new Integer(0), new Integer(120)});
    }

    CurUNOProgressDialog.insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblCurProgress",
                            new String[] {"Height", "Label", "PositionX", "PositionY", "Step", "Width"},
                            new Object[] {new Integer(10), "", new Integer(12), new Integer(42), new Integer(0), new Integer(120)});

    CurUNOProgressDialog.insertButton("cmdCancel", 10000, new ActionListenerImpl(),
                            new String[] {"Height", "HelpURL", "PositionX", "PositionY", "Step", "TabIndex", "Width", "Label"},
                            new Object[] {new Integer(14), "HID:34321", new Integer(74), new Integer(58), new Integer(0), new Short((short) 1), new Integer(40), sStop});

    CurUNOProgressDialog.calculateDialogPosition(xMSF, CurReportDocument.Frame.getComponentWindow().getPosSize());
    CurUNOProgressDialog.xWindow.setVisible(true);
    return CurUNOProgressDialog;
    }
    catch(com.sun.star.uno.Exception exception)
    {
        exception.printStackTrace(System.out);
    return null;
    }
    catch(java.lang.Exception jexception ){
    jexception.printStackTrace(System.out);
    return null;
    }}


    public void getGroupFieldFortmats(XMultiServiceFactory xMSF, ReportDocument CurReportDocument, DBMetaData CurDBMetaData){
    int GroupCount = CurDBMetaData.GroupFieldNames.length;
    ReportDocument.GroupFieldFormat CurGroupFieldFormat;
    for (int i = 0; i < GroupCount; i++){
        CurGroupFieldFormat = CurReportDocument.addGroupTableFormat(CurDBMetaData.GroupFieldNames[i], i, "Tbl_GroupField" + (i + 1));
        if (CurGroupFieldFormat != null){
        CurReportDocument.GroupFormatVector.addElement(CurGroupFieldFormat);
        }
        else{
        String sMessage = tools.replaceSubString(sMsgTableNotExisting, "Tbl_GroupField" + (GroupCount), "<TABLENAME>");
        UNODialogs.showMessageBox(xMSF, CurReportDocument.Frame, "ErrorBox", com.sun.star.awt.VclWindowPeerAttribute.OK, sMessage);
        }
    }
    }


    public static void addLinkedTextSection(ReportDocument CurReportDocument, XTextCursor xTextCursor, String sLinkRegion,
                        ReportDocument.GroupFieldFormat CurGroupFieldFormat, String CurGroupValue){
    try{
    Object oTextSection =  CurReportDocument.MSFDoc.createInstance("com.sun.star.text.TextSection");
    XTextContent xTextSectionContent = (XTextContent) UnoRuntime.queryInterface(XTextContent.class, oTextSection);
    xTextCursor.gotoEnd(false);
    xTextCursor.getText().insertTextContent(xTextCursor, xTextSectionContent, true);
    tools.setUNOPropertyValue(oTextSection, "LinkRegion", sLinkRegion);
    if (CurGroupFieldFormat != null){
        Object oTextTable = getlastTextTable(CurReportDocument.ReportTextDocument);
        XCellRange xCellRange = (XCellRange) UnoRuntime.queryInterface(XCellRange.class, oTextTable);
        CurReportDocument.replaceValueCellofTable(xCellRange, CurGroupFieldFormat, CurGroupValue);
    }
    }
    catch( com.sun.star.uno.Exception exception ){
        exception.printStackTrace(System.out);
     }}


    // Todo: This Routine should be  modified, because I cannot rely on the last Table in the document to be the last in the TextTables sequence
    public static XTextTable getlastTextTable(XTextDocument xTextDocument){
    try{
    XTextTablesSupplier xTextTablesSuppl = (XTextTablesSupplier) UnoRuntime.queryInterface(XTextTablesSupplier.class, xTextDocument);
    XIndexAccess xAllTextTables = (XIndexAccess) UnoRuntime.queryInterface(XIndexAccess.class, xTextTablesSuppl.getTextTables());
    int MaxIndex = xAllTextTables.getCount()-1;
    XTextTable xTextTable = (XTextTable) xAllTextTables.getByIndex(MaxIndex);
    return xTextTable;
    }
    catch( com.sun.star.uno.Exception exception ){
        exception.printStackTrace(System.out);
     return null;
    }}
}
