 /*************************************************************************
 *
 *  $RCSfile: Dataimport.java,v $
 *
 *  $Revision: 1.23 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-11 15:03:37 $
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
import com.sun.star.wizards.common.TextDocument;
import com.sun.star.wizards.common.UNODialogs;
import com.sun.star.wizards.common.Tools;
import com.sun.star.wizards.common.TextDocument.UnknownHiddenControlException;
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
import com.sun.star.script.XInvocation;
import com.sun.star.style.XStyleFamiliesSupplier;
import com.sun.star.style.XStyleLoader;
import com.sun.star.style.BreakType;
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

    ReportDocument CurReportDocument;
    UNODialogs CurUNOProgressDialog;
    static boolean bStopProcess;
    static boolean brenamefirstTable = true;

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
            xMSF = Tools.connect(ConnectStr);
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


    public void showProgressDisplay(XMultiServiceFactory xMSF, boolean bgetConnection){
    try{
        int iHeight;
        CurUNOProgressDialog = new UNODialogs(xMSF, new String[] {"Height", "Step", "Title", "Width"},
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
        CurUNOProgressDialog.createWindowPeer(CurReportDocument.xWindowPeer);
        CurUNOProgressDialog.calculateDialogPosition(CurReportDocument.xFrame.getComponentWindow().getPosSize());
        CurUNOProgressDialog.xWindow.setVisible(true);

    return;
    }
    catch(com.sun.star.uno.Exception exception)
    {
        exception.printStackTrace(System.out);
    return;
    }
    catch(java.lang.Exception jexception ){
    jexception.printStackTrace(System.out);
    return;
    }}


    public void importReportData(final XMultiServiceFactory xMSF, final Dataimport CurDataimport, final ReportDocument CurReportDocument){
/*    Thread ProgressThread = new Thread(new Runnable(CurUNOProgressDialog) {
        private UNODialogs dialog;
    public Runnable( UNODialogs x )
    {
        dialog = x;
    }*/
    Thread ProgressThread = new Thread(new Runnable() {
    public void run(){
    try{
        if (reconnectToDatabase(xMSF)){
        CurUNOProgressDialog.modifyFontWeight("lblProgressDBConnection", com.sun.star.awt.FontWeight.NORMAL);
        CurUNOProgressDialog.modifyFontWeight("lblProgressDataImport", com.sun.star.awt.FontWeight.BOLD);
        insertDatabaseDatatoReportDocument(xMSF);
        }
        CurDataimport.CurUNOProgressDialog.xComponent.dispose();
        CurReportDocument.CurDBMetaData.disposeDBMetaData();
    }
    catch (ThreadDeath td){
        System.out.println("could not stop thread");
        CurUNOProgressDialog.xComponent.dispose();
    }
    }
        });

    ProgressThread.start();
//        try {
//      ProgressThread.join();
//       }
//  catch(InterruptedException e){
//      System.out.println("could not join Threads");
//  }
    }


    public void createReport(final XMultiServiceFactory xMSF){
    try{
    if (ReportWizard.getReportResources(xMSF, true)){
        CurReportDocument = new ReportDocument(xMSF, false, true, ReportMessages);
        int iWidth = CurReportDocument.xFrame.getComponentWindow().getPosSize().Width;
        showProgressDisplay(xMSF, true);
        importReportData(xMSF, this, CurReportDocument);
    }
    }
    catch(java.lang.Exception jexception ){
    jexception.printStackTrace(System.out);
    }}


    public boolean reconnectToDatabase(XMultiServiceFactory xMSF){
    try{
    XNameContainer xNamedForms = CurReportDocument.getDocumentForms();
    Object oDBForm = Tools.getUNOObjectbyName(xNamedForms, SOREPORTFORMNAME);
    boolean bgetConnection;
    if (oDBForm != null){
        String sMsg = sMsgHiddenControlMissing + (char) 13 + sMsgEndAutopilot;
        XNameAccess xNamedForm = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, oDBForm);
        // Todo make up an exception class to catch that error
        CurReportDocument.CurDBMetaData.DataSourceName = (String) CurReportDocument.getValueofHiddenControl(xNamedForm, "DataSourceName", sMsg);
        CurReportDocument.CurDBMetaData.Command = CurReportDocument.getValueofHiddenControl(xNamedForm, "Command", sMsg);
        String sCommandType = CurReportDocument.getValueofHiddenControl(xNamedForm, "CommandType", sMsg);
        String sGroupFieldNames = CurReportDocument.getValueofHiddenControl(xNamedForm, "GroupFieldNames", sMsg);
        String sFieldNames = CurReportDocument.getValueofHiddenControl(xNamedForm, "FieldNames", sMsg);
        String sRecordFieldNames = CurReportDocument.getValueofHiddenControl(xNamedForm, "RecordFieldNames", sMsg);
        CurReportDocument.CurDBMetaData.FieldNames = Tools.ArrayoutofString(sFieldNames,";");
        CurReportDocument.CurDBMetaData.RecordFieldNames = Tools.ArrayoutofString(sRecordFieldNames,";");
        CurReportDocument.CurDBMetaData.GroupFieldNames = Tools.ArrayoutofString(sGroupFieldNames,";");
        CurReportDocument.CurDBMetaData.CommandType = Integer.valueOf(sCommandType).intValue();
        sMsgQueryCreationImpossible = Tools.replaceSubString(sMsgQueryCreationImpossible, CurReportDocument.CurDBMetaData.Command, "<STATEMENT>");
        bgetConnection = CurReportDocument.CurDBMetaData.getConnection(sMsgNoConnection, sMsgConnectionImpossible);
        if (bgetConnection){
        boolean bexecute = CurReportDocument.CurDBMetaData.executeCommand(sMsgQueryCreationImpossible + (char) 13 + sMsgEndAutopilot);
        if (bexecute == true){
            CurReportDocument.CurDBMetaData.initializeFields(CurReportDocument.CurDBMetaData.FieldNames, false);
            CurReportDocument.getallDBColumns();
        }

        return bexecute;
        }
        else
        return false;
    }
    else{
        sReportFormNotExisting = Tools.replaceSubString(sReportFormNotExisting, SOREPORTFORMNAME, "<REPORTFORM>");
        UNODialogs.showMessageBox(xMSF, "ErrorBox", com.sun.star.awt.VclWindowPeerAttribute.OK, sReportFormNotExisting + (char) 13 + sMsgEndAutopilot);
        return false;
    }
    }
    catch(java.lang.Exception javaexception ){
    javaexception.printStackTrace(System.out);
    return false;
    }
    catch(UnknownHiddenControlException exception){
    return false;
     }}



    public void insertDatabaseDatatoReportDocument(XMultiServiceFactory xMSF){
    try{
        int ColIndex;
        int i;
    boolean breset;
    Object oTable;
    XCellRange xCellRange;
    XTextCursor xTextCursor;
    XTextDocument xTextDocument;
    java.util.Vector DataVector = new java.util.Vector();
    ReportDocument.DBColumn CurDBColumn;
    Object CurGroupValue;
    String CurGroupTableName;
    DBMetaData CurDBMetaData = CurReportDocument.CurDBMetaData;
    int GroupFieldCount = Tools.getArraylength(CurDBMetaData.GroupFieldNames);
    int FieldCount = Tools.getArraylength(CurDBMetaData.FieldNames);
    Object[] OldGroupFieldValues = new Object[GroupFieldCount];
    XTextTable[] xGroupBaseTables = new XTextTable[GroupFieldCount];
    int RecordFieldCount = FieldCount - GroupFieldCount;
    int[] SelColIndices = null;
    int[] GroupColIndices = null;
    BreakType CorrBreakValue = null;
    String CorrPageDescName = "";
    int iCommandType = CurDBMetaData.CommandType;
    if ((iCommandType == com.sun.star.sdb.CommandType.QUERY) || (iCommandType == com.sun.star.sdb.CommandType.COMMAND)){
        SelColIndices = CurDBMetaData.getSelectedQueryFields(CurDBMetaData.RecordFieldNames);
        GroupColIndices = CurDBMetaData.getSelectedQueryFields(CurDBMetaData.GroupFieldNames);
    }
    XNameAccess xTextTables = CurReportDocument.xTextTablesSupplier.getTextTables();
    xTextDocument = CurReportDocument.xTextDocument;
        xTextCursor = CurReportDocument.createTextCursor(CurReportDocument.xTextDocument.getText());
    xTextDocument.lockControllers();
        if (CurDBMetaData.ResultSet.next() == true){
        replaceUserFields();
        Tools.setUNOPropertyValue(xTextCursor, "PageDescName", "First Page");
        for (ColIndex = 0; ColIndex < GroupFieldCount; ColIndex++){
            CurGroupTableName = "Tbl_GroupField" + Integer.toString(ColIndex+1);
            oTable = CurReportDocument.xTextTablesSupplier.getTextTables().getByName(CurGroupTableName);
        xGroupBaseTables[ColIndex] = (XTextTable) UnoRuntime.queryInterface(XTextTable.class, oTable);
        if (ColIndex == 0){
            CorrBreakValue = CurReportDocument.resetBreakTypeofTextTable(xGroupBaseTables[ColIndex]);
            String PageDescName = AnyConverter.toString(Tools.getUNOPropertyValue(xGroupBaseTables[ColIndex], "PageDescName"));
            if (PageDescName.equals("") == false){
            CorrPageDescName = PageDescName;
            Tools.setUNOPropertyValue(xGroupBaseTables[ColIndex], "PageDescName", "");
            }
        }
            CurGroupValue = CurDBMetaData.getGroupColumnValue(iCommandType, GroupColIndices, ColIndex);
        OldGroupFieldValues[ColIndex] = CurGroupValue;
        CurDBColumn = (ReportDocument.DBColumn) CurReportDocument.GroupFormatVector.elementAt(ColIndex);
        addLinkedTextSection(xTextCursor, "GroupField" + Integer.toString(ColIndex+1), CurDBColumn, CurGroupValue);
        }
        if (CurDBMetaData.getcurrentRecordData(ColIndex, FieldCount, RecordFieldCount, SelColIndices, iCommandType, DataVector, sMsgQueryCreationImpossible) == true){
        int RowIndex = 1;
        bStopProcess = false;
        while ((CurDBMetaData.ResultSet.next() == true) && (bStopProcess == false)){
            RowIndex += 1;
            breset = false;
            for (ColIndex = 0; ColIndex < GroupFieldCount; ColIndex++){
            CurGroupValue = CurDBMetaData.getGroupColumnValue(iCommandType, GroupColIndices, ColIndex);
            if ((CurGroupValue.equals((Object) OldGroupFieldValues[ColIndex]) == false) || (breset)){
                breset = true;
                insertDataToRecordTable(xTextCursor, DataVector, RecordFieldCount);
                CurDBColumn = (ReportDocument.DBColumn) CurReportDocument.GroupFormatVector.elementAt(ColIndex);
                addLinkedTextSection(xTextCursor, "GroupField" + Integer.toString(ColIndex+1), CurDBColumn, CurGroupValue);
                OldGroupFieldValues[ColIndex] = CurGroupValue;
                breset = !(ColIndex == GroupFieldCount-1);
            }
            }
            CurDBMetaData.getcurrentRecordData(ColIndex, FieldCount, RecordFieldCount, SelColIndices, iCommandType, DataVector, sMsgQueryCreationImpossible);
            updateProgressDisplay(RowIndex);
        }
        insertDataToRecordTable(xTextCursor, DataVector, RecordFieldCount);
        }
        else{
        CurReportDocument.unlockallControllers();
        return;
        }
        }
    setLayoutSectionsInvisible(GroupFieldCount);
    CurReportDocument.breakLinkofTextSections();
    Object oTextTable= CurReportDocument.xTextTablesSupplier.getTextTables().getByName("FirstVisibleTextTable");
    if (CorrBreakValue != null)
        Tools.setUNOPropertyValue(oTextTable, "BreakType", CorrBreakValue);
    if (CorrPageDescName != "")
        Tools.setUNOPropertyValue(oTextTable, "PageDescName", CorrPageDescName);
    }
    catch( com.sun.star.uno.Exception exception ){
        exception.printStackTrace(System.out);
    }

    catch(java.lang.Exception javaexception ){
        javaexception.printStackTrace(System.out);
    }
    CurReportDocument.unlockallControllers();
    }


    public void setLayoutSectionsInvisible(int GroupFieldCount){
    try{
    XNameAccess xTextSections = CurReportDocument.xTextSectionsSupplier.getTextSections();
    Object oTextSection;
    for (int i = 0; i< GroupFieldCount; i++){
        oTextSection = xTextSections.getByName("GroupField" + String.valueOf(i+1));
        Tools.setUNOPropertyValue(oTextSection, "IsVisible", new Boolean(false));
    }
    if (xTextSections.hasByName("RecordSection")){
        oTextSection = xTextSections.getByName("RecordSection");
        Tools.setUNOPropertyValue(oTextSection, "IsVisible", new Boolean(false));
    }
    }
    catch( com.sun.star.uno.Exception exception ){
        exception.printStackTrace(System.out);
    }}


    public void insertDataToRecordTable(XTextCursor xTextCursor, java.util.Vector DataVector, int FieldCount){
    try{
    int DataLength = DataVector.size();
    if ((FieldCount > 0)&& (DataLength > 0)){
        addLinkedTextSection(xTextCursor, "RecordSection", null, null);
        Object[][] RecordArray = new Object[DataLength][FieldCount];
        DataVector.copyInto(RecordArray);
        XTextTable xTextTable = CurReportDocument.getlastTextTable();
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


    public void updateProgressDisplay(int iCounter){
    try{
    if (iCounter % 10 == 0){
        sProgressCurRecord = Tools.replaceSubString(sProgressBaseCurRecord, String.valueOf(iCounter), "<COUNT>");
            CurUNOProgressDialog.assignPropertyToDialogControl("lblCurProgress", "Label", sProgressCurRecord);
    }
    }
    catch(java.lang.Exception jexception ){
    jexception.printStackTrace(System.out);
    }}


    public void addLinkedTextSection(XTextCursor xTextCursor, String sLinkRegion, ReportDocument.DBColumn CurDBColumn, Object CurGroupValue){
    try{
    XInterface xTextSection = (XInterface) CurReportDocument.xMSFDoc.createInstance("com.sun.star.text.TextSection");
    XTextContent xTextSectionContent = (XTextContent) UnoRuntime.queryInterface(XTextContent.class, xTextSection);
    xTextCursor.gotoEnd(false);
    xTextCursor.getText().insertTextContent(xTextCursor, xTextSectionContent, true);
    Tools.setUNOPropertyValue(xTextSection, "LinkRegion", sLinkRegion);
    if (CurDBColumn != null){
        boolean bIsGroupTable = (sLinkRegion.equals("RecordSection") != true);
        if (bIsGroupTable == true){
        XTextTable xTextTable = CurReportDocument.getlastTextTable();
        XCellRange xCellRange = (XCellRange) UnoRuntime.queryInterface(XCellRange.class, xTextTable);
        CurDBColumn.modifyCellContent(xCellRange, CurGroupValue);
        }
    }
    if (brenamefirstTable == true){
        brenamefirstTable = false;
        XTextTable xTextTable = CurReportDocument.getlastTextTable();
        XNamed xNamedTable = (XNamed) UnoRuntime.queryInterface(XNamed.class, xTextTable);
        xNamedTable.setName("FirstVisibleTextTable");

    }
    }
    catch( com.sun.star.uno.Exception exception ){
        exception.printStackTrace(System.out);
    }}


    public void replaceUserFields(){
    ReportDocument.DBColumn CurDBColumn;
    XTextCursor xNameCellCursor;
    String FieldContent;
    int iCount = CurReportDocument.GroupFormatVector.size();
    for (int i = 0; i < iCount; i++){
        CurDBColumn = (ReportDocument.DBColumn) CurReportDocument.GroupFormatVector.elementAt(i);
        xNameCellCursor = CurReportDocument.createTextCursor(CurDBColumn.xNameCell);
        xNameCellCursor.gotoStart(false);
        FieldContent = CurReportDocument.getUserFieldContent(xNameCellCursor);
        xNameCellCursor.goRight((short)1, true);
        xNameCellCursor.setString(FieldContent);
    }
    }
}
