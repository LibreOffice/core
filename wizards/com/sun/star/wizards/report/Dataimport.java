 /*************************************************************************
 *
 *  $RCSfile: Dataimport.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: bc $ $Date: 2002-06-27 09:41:14 $
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



public class Dataimport{
    static String sProgressTitle;
    static String sProgressDBConnection;
    static String sProgressDataImport;
    static String sProgressBaseCurRecord;
    static String sProgressCurRecord;
    static UNODialogs.UNODialog CurUNODialog;
    static boolean bStopProcess;


    public Dataimport() {
    }

    static class ActionListenerImpl implements com.sun.star.awt.XActionListener {

        public void disposing(EventObject eventObject) {
        }
        public void actionPerformed(ActionEvent actionEvent) {
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
        createReport(xMSF);
        }
        catch(com.sun.star.uno.Exception e) {
            e.printStackTrace(System.out);
            System.exit( 0 );
        }
        catch(java.lang.Exception javaexception ){
        javaexception.printStackTrace(System.out);
    }
    }


    public static void createReport(XMultiServiceFactory xMSF){
    try{
    ReportDocument.RepWizardDocument CurReportDocument;
    DBMetaData.CommandMetaData CurDBMetaData;
    XDesktop xDesktop = tools.getDesktop(xMSF);
    XFramesSupplier xFrameSuppl = (XFramesSupplier) UnoRuntime.queryInterface(XFramesSupplier.class, xDesktop);
    XModel xComponent = xFrameSuppl.getActiveFrame().getController().getModel();
    CurUNODialog = Dataimport.showProgressDisplay(xMSF);
    CurReportDocument =  new ReportDocument.RepWizardDocument();
    CurReportDocument.Component = (XComponent) xComponent;
    CurReportDocument.ReportTextDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, CurReportDocument.Component);
    CurReportDocument.DatabaseContext = xMSF.createInstance("com.sun.star.sdb.DatabaseContext");
    CurReportDocument.TextTablesSupplier = (XTextTablesSupplier) UnoRuntime.queryInterface(XTextTablesSupplier.class, CurReportDocument.ReportTextDocument);
    CurDBMetaData = new DBMetaData.CommandMetaData();
    if (reconnectToDatabase(xMSF, CurDBMetaData, CurReportDocument)){
        UNODialogs.modifyFontWeight(CurUNODialog.xDlgNameAccess, "lblProgressDBConnection", com.sun.star.awt.FontWeight.NORMAL);
        UNODialogs.modifyFontWeight(CurUNODialog.xDlgNameAccess, "lblProgressDataImport", com.sun.star.awt.FontWeight.BOLD);
        insertDatabaseDatatoReportDocument(xMSF, CurDBMetaData, CurReportDocument);
    }
    CurUNODialog.xComponent.dispose();
    }
    catch(com.sun.star.uno.Exception exception ){
    exception.printStackTrace(System.out);
    }}


    public static boolean reconnectToDatabase(XMultiServiceFactory xMSF, DBMetaData.CommandMetaData CurDBMetaData, ReportDocument.RepWizardDocument CurReportDocument){
    try{
    XNameContainer xNamedForms = ReportDocument.getDocumentForms(CurReportDocument.ReportTextDocument);
    Object oDBForm = tools.getUNOObjectbyName(xNamedForms, "ReportSource");
    XNameAccess xNamedForm = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, oDBForm);
    CurDBMetaData. DataSourceName = (String) tools.getUNOPropertyValue(xNamedForm.getByName("DataSourceName"), "HiddenValue");
    CurDBMetaData.Command = (String) tools.getUNOPropertyValue(xNamedForm.getByName("Command"), "HiddenValue");
    String sCommandType = (String) tools.getUNOPropertyValue(xNamedForm.getByName("CommandType"), "HiddenValue");
    String sGroupFieldNames = (String) tools.getUNOPropertyValue(xNamedForm.getByName("GroupFieldNames"), "HiddenValue");
    String sFieldNames = (String) tools.getUNOPropertyValue(xNamedForm.getByName("FieldNames"), "HiddenValue");
    String sRecordFieldNames = (String) tools.getUNOPropertyValue(xNamedForm.getByName("RecordFieldNames"), "HiddenValue");
    CurDBMetaData.FieldNames = tools.ArrayoutofString(sFieldNames,";");
    CurDBMetaData.RecordFieldNames = tools.ArrayoutofString(sRecordFieldNames,";");
    CurDBMetaData.GroupFieldNames = tools.ArrayoutofString(sGroupFieldNames,";");
    CurDBMetaData.CommandType = Integer.valueOf(sCommandType).intValue();
        XInvocation xResInvoke = tools.initResources(xMSF, "ReportWizard","dbw");
        final int RID_COMMON = 1000;
    String sMsgNoConnection = tools.getResText(xResInvoke, RID_COMMON + 14);
    if (DBMetaData.getConnection(xMSF, CurDBMetaData, CurReportDocument, CurDBMetaData.DataSourceName, sMsgNoConnection) == true){
           CurDBMetaData.Statement = CurDBMetaData.DBConnection.createStatement();
       CurDBMetaData.ResultSet = CurDBMetaData.Statement.executeQuery(CurDBMetaData.Command);
       return true;
    }
    else
        return false;
    }
    catch( com.sun.star.uno.Exception exception ){
        exception.printStackTrace(System.out);
    return false;
    }
    catch(java.lang.Exception javaexception ){
        javaexception.printStackTrace(System.out);
    return false;
    }}


    public static void insertDatabaseDatatoReportDocument(XMultiServiceFactory xMSF, DBMetaData.CommandMetaData CurDBMetaData, ReportDocument.RepWizardDocument CurReportDocument){
    try{
        int ColIndex;
        int i;
    XTextCursor xTextCursor;
    java.util.Vector DataVector = new java.util.Vector();
    String CurGroupValue;
    String CurGroupTableName;
    com.sun.star.sdbc.XRow xResultSetRow;
        int GroupFieldCount = CurDBMetaData.GroupFieldNames.length;
    int FieldCount = CurDBMetaData.FieldNames.length;
    String[] OldGroupFieldValues = new String[GroupFieldCount];
    XTextTable[] xGroupBaseTables = new XTextTable[GroupFieldCount];
    int RecordFieldCount = FieldCount - GroupFieldCount;
    int[] SelColIndices = null;
    int[] GroupColIndices = null;
    int iCommandType = CurDBMetaData.CommandType;
    if ((iCommandType == com.sun.star.sdb.CommandType.QUERY) || (iCommandType == com.sun.star.sdb.CommandType.COMMAND)){
        SelColIndices = DBMetaData.getSelectedQueryFields(CurDBMetaData, CurDBMetaData.RecordFieldNames);
        GroupColIndices = DBMetaData.getSelectedQueryFields(CurDBMetaData, CurDBMetaData.GroupFieldNames);
    }
    xResultSetRow = (com.sun.star.sdbc.XRow) UnoRuntime.queryInterface(com.sun.star.sdbc.XRow.class, CurDBMetaData.ResultSet);
    XNameAccess xTextTables = CurReportDocument.TextTablesSupplier.getTextTables();
    CurReportDocument.ReportTextDocument.lockControllers();
        if (CurDBMetaData.ResultSet.next() == true){
            XMultiServiceFactory xMSFDoc = (XMultiServiceFactory) UnoRuntime.queryInterface(XMultiServiceFactory.class, CurReportDocument.ReportTextDocument);
            xTextCursor = ReportDocument.createTextCursor(CurReportDocument.ReportTextDocument.getText());
        tools.setUNOPropertyValue(xTextCursor, "PageDescName", "First Page");
        for (ColIndex=0; ColIndex<GroupFieldCount;ColIndex++){
            CurGroupTableName = "Tbl_GroupField" + Integer.toString(ColIndex+1);
            xGroupBaseTables[ColIndex] = (XTextTable) CurReportDocument.TextTablesSupplier.getTextTables().getByName(CurGroupTableName);
            OldGroupFieldValues[ColIndex] = DBMetaData.getColumnValue(xResultSetRow, iCommandType, GroupColIndices, ColIndex);
            ReportDocument.addLinkedTextSection(xMSFDoc, xTextCursor, "GroupField" + String.valueOf(ColIndex+1));
            CurGroupValue = DBMetaData.getColumnValue(xResultSetRow, iCommandType, GroupColIndices, ColIndex);
            ReportDocument.insertStringToLastTextTableCell(CurReportDocument.ReportTextDocument, xResultSetRow, 1, 0, CurGroupValue);
        }
        DataVector.addElement(DBMetaData.getcurrentRecordData(xResultSetRow, ColIndex, FieldCount, RecordFieldCount, SelColIndices, iCommandType));
        int RowIndex = 1;
        bStopProcess = false;
        while (CurDBMetaData.ResultSet.next() == true){
        if (bStopProcess == true)
            return;
            RowIndex += 1;
            for (ColIndex = 0; ColIndex < GroupFieldCount; ColIndex++){
            CurGroupValue = DBMetaData.getColumnValue(xResultSetRow, iCommandType, GroupColIndices, ColIndex);
            if (CurGroupValue.equals((Object) OldGroupFieldValues[ColIndex]) == false){
                if (DataVector.size() > 0){
                    ReportDocument.addLinkedTextSection(xMSFDoc, xTextCursor, "RecordSection");
                insertDataToRecordTable(CurReportDocument.ReportTextDocument, DataVector, RecordFieldCount);
                DataVector.removeAllElements();
                OldGroupFieldValues[ColIndex] = CurGroupValue;
            }
            ReportDocument.addLinkedTextSection(xMSFDoc, xTextCursor, "GroupField" + String.valueOf(ColIndex+1));
            ReportDocument.insertStringToLastTextTableCell(CurReportDocument.ReportTextDocument, xResultSetRow, 1, 0, CurGroupValue);
            }
        }
        DataVector.addElement(DBMetaData.getcurrentRecordData(xResultSetRow, ColIndex, FieldCount, RecordFieldCount, SelColIndices, iCommandType));
        updateProgressDisplay(RowIndex);
        }
        if (DataVector.size() > 0){
            ReportDocument.addLinkedTextSection(xMSFDoc, xTextCursor, "RecordSection");
            insertDataToRecordTable(CurReportDocument.ReportTextDocument, DataVector, RecordFieldCount);
            DataVector.removeAllElements();
        }
        }
    setLayoutSectionsInvisible(CurReportDocument.TextSectionsSupplier, GroupFieldCount);
//  breakLinksofTextSections(CurReportDocument.ReportTextDocument);
    }
    catch( com.sun.star.uno.Exception exception ){
        exception.printStackTrace(System.out);
    }

    catch(java.lang.Exception javaexception ){
        javaexception.printStackTrace(System.out);
    }
    CurReportDocument.ReportTextDocument.unlockControllers();
    }


    public static void setLayoutSectionsInvisible(XTextSectionsSupplier xTextSectionsSupplier, int GroupFieldCount){
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


    public static void insertDataToRecordTable(XTextDocument xTextDocument, java.util.Vector RecordVector, int FieldCount){
    try{
    int DataLength = RecordVector.size();
    if (FieldCount > 0){
        String[][] RecordArray = new String[DataLength][FieldCount];
        RecordVector.copyInto(RecordArray);
        XTextTable xTextTable = ReportDocument.getLastTextTable(xTextDocument);
        if (DataLength > 1){
        xTextTable.getRows().insertByIndex(xTextTable.getRows().getCount(), DataLength - 1);
        }
        XCellRange xCellRange = (XCellRange) UnoRuntime.queryInterface(XCellRange.class, xTextTable);
        int iRowCount = xTextTable.getRows().getCount();
        int iColCount = xTextTable.getColumns().getCount();

        XCellRange xNewRange = xCellRange.getCellRangeByPosition(0,1,FieldCount -1,DataLength);
        XCellRangeData xDataArray = (XCellRangeData) UnoRuntime.queryInterface(XCellRangeData.class, xNewRange);
        xDataArray.setDataArray(RecordArray);
    }
    }
    catch( com.sun.star.uno.Exception exception ){
        exception.printStackTrace(System.out);
     }}


    public static void addDBRecordstoReportDocument(XTextDocument xTextDocument, XResultSet xResultSet, String[] FieldNames)
    throws com.sun.star.sdbc.SQLException, com.sun.star.container.NoSuchElementException, com.sun.star.lang.IllegalArgumentException,
        com.sun.star.lang.WrappedTargetException{
    int iColIndex;
    int FieldCount = FieldNames.length;
    com.sun.star.sdbc.XRow xResultSetRow;
    String sValue;
    String sResultSet = null;
    xTextDocument.lockControllers();
    xResultSetRow = (com.sun.star.sdbc.XRow) UnoRuntime.queryInterface(com.sun.star.sdbc.XRow.class, xResultSet);
    XTextCursor xTextCursor = ReportDocument.createTextCursor(xTextDocument.getText());
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
        xTextDocument.getText().insertControlCharacter(xTextCursor, com.sun.star.text.ControlCharacter.PARAGRAPH_BREAK, false);
    }
       xTextDocument.unlockControllers();
    }



    public static void addDBRecordstoReportDocumentTable(XTextDocument xTextDocument, XTextTable xTextTable, XResultSet xResultSet, String[] FieldNames)
    throws com.sun.star.sdbc.SQLException, com.sun.star.container.NoSuchElementException, com.sun.star.lang.WrappedTargetException{
    int iColIndex;
    int FieldCount = FieldNames.length;
//  xTextDocument.lockControllers();
    com.sun.star.sdbc.XRow xResultSetRow;
    XTextCursor xCellCursor;
    String CellName;
    String sValue;
    xResultSetRow = (com.sun.star.sdbc.XRow) UnoRuntime.queryInterface(com.sun.star.sdbc.XRow.class, xResultSet);
    int iRow = 1;
    while (xResultSet.next() == true){
        iRow +=1;
        char iChar = 'A';
        for (int i=0; i<FieldCount;i++){
        sValue = xResultSetRow.getString(i+1);
        if (xResultSetRow.wasNull() == false){
            CellName = iChar + Integer.toString(iRow);
            xCellCursor = ReportDocument.createTextCursor(xTextTable.getCellByName(CellName));
            xCellCursor.setString(sValue);
        }
        iChar = (char)((int) iChar + 1);
        }
        xTextTable.getRows().insertByIndex(xTextTable.getRows().getCount(), 1);
       }
       int iCount = xTextTable.getRows().getCount();
       xTextTable.getRows().removeByIndex(iCount-1, 1);
//       xTextDocument.unlockControllers();
    }


    public static void updateProgressDisplay(int iCounter){
    try{
    if (iCounter % 10 == 0){
        sProgressCurRecord = tools.replaceSubString(sProgressBaseCurRecord, String.valueOf(iCounter), "<Count>");
            UNODialogs.assignPropertyToDialogControl(CurUNODialog.xDlgNameAccess, "lblCurProgress", "Label", sProgressCurRecord);
    }
    }
    catch(com.sun.star.uno.Exception exception)
    {
        exception.printStackTrace(System.out);
    }
    catch(java.lang.Exception jexception ){
    jexception.printStackTrace(System.out);
    }}



    public static UNODialogs.UNODialog showProgressDisplay(XMultiServiceFactory xMSF){
    try{
    getProgressResources(xMSF);
    UNODialogs.UNODialog CurUNODialog = UNODialogs.initializeDialog(xMSF, new String[] {"Height", "PositionX", "PositionY", "Step", "Title", "Width"},
                            new Object[] {new Integer(84), new Integer(250), new Integer(150), new Integer(0), sProgressTitle, new Integer(180)});

    com.sun.star.awt.FontDescriptor oFontDesc = new com.sun.star.awt.FontDescriptor();
    oFontDesc.Weight = com.sun.star.awt.FontWeight.BOLD;
    UNODialogs.insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", CurUNODialog, "lblProgressDBConnection",
                            new String[] {"FontDescriptor", "Height", "Label", "PositionX", "PositionY", "Step", "Width"},
                            new Object[] {oFontDesc, new Integer(10), sProgressDBConnection, new Integer(6), new Integer(6), new Integer(0), new Integer(150)});

    UNODialogs.insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", CurUNODialog, "lblProgressDataImport",
                            new String[] {"Height", "Label", "PositionX", "PositionY", "Step", "Width"},
                            new Object[] {new Integer(10), sProgressDataImport, new Integer(6), new Integer(24), new Integer(0), new Integer(120)});

    UNODialogs.insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", CurUNODialog, "lblCurProgress",
                            new String[] {"Height", "Label", "PositionX", "PositionY", "Step", "Width"},
                            new Object[] {new Integer(10), sProgressCurRecord, new Integer(12), new Integer(42), new Integer(0), new Integer(120)});

    UNODialogs.insertButton(CurUNODialog, "cmdCancel", 10000, null,
                            new String[] {"Height", "HelpURL", "PositionX", "PositionY", "PushButtonType", "Step", "TabIndex", "Width", "Label"},
                            new Object[] {new Integer(14), "HID:34321", new Integer(74), new Integer(58), new Short((short)com.sun.star.awt.PushButtonType.CANCEL_value), new Integer(0), new Short((short) 1), new Integer(40), "Stop"});

    CurUNODialog.xWindow = (XWindow) UnoRuntime.queryInterface(XWindow.class, CurUNODialog.objectDialog);
        CurUNODialog.xWindow.setVisible(true);
    return CurUNODialog;
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


    public static void getProgressResources(XMultiServiceFactory xMSF){
        XInvocation xResInvoke = tools.initResources(xMSF, "ReportWizard","dbw");
    sProgressTitle = "Bericht wird erstellt...";
        sProgressDBConnection = "Verbindung zur Datenbank wird hergestellt...";
    sProgressDataImport = "Daten werden importiert...";
    sProgressBaseCurRecord = "Anzahl der eingefügten Datensätze: <Count>";
    sProgressCurRecord = "";
//  sMsgWizardName = tools.getResText(xResInvoke, RID_REPORT);
    }


/*    public static void breakLinkofTextSections(XTextDocument xTextDocument){
    try{
    Object oTextSection;
    com.sun.star.text.SectionFileLink oSectionLink = new com.sun.star.text.SectionFileLink();
    oSectionLink.FileURL = "";
    for (int i =0; i < iSectionCount; i++){
        oTextSection = xAllTextSections.getByIndex(i);
        tools.setUNOPropertyValue(oTextSection, "FileLink", oSectionLink);
        tools.setUNOPropertyValue(oTextSection, "LinkRegion", "");
    }
     }
     catch( com.sun.star.uno.Exception exception ){
        exception.printStackTrace(System.out);
     }} */

}
