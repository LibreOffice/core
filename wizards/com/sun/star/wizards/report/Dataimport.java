/*************************************************************************
*
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Dataimport.java,v $
 *
 *  $Revision: 1.41 $
 *
 *  last change: $Author: vg $ $Date: 2008-01-28 15:30:30 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
package com.sun.star.wizards.report;

import java.util.Vector;

import com.sun.star.awt.VclWindowPeerAttribute;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNameContainer;
import com.sun.star.container.XNamed;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdb.CommandType;
import com.sun.star.style.BreakType;
import com.sun.star.style.CaseMap;
import com.sun.star.table.XCellRange;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextTable;

import com.sun.star.uno.Any;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.wizards.db.*;
import com.sun.star.wizards.ui.*;
import com.sun.star.wizards.common.*;
import com.sun.star.wizards.document.*;
import com.sun.star.wizards.text.*;
import com.sun.star.wizards.common.InvalidQueryException;
import com.sun.star.uno.Exception;

public class Dataimport extends UnoDialog2 implements com.sun.star.awt.XActionListener{ // extends ReportWizard

    ReportDocument CurReportDocument;
    PropertyValue[] CurProperties;
    static boolean bStopProcess;
    static String sProgressDBConnection;
    static String sProgressDataImport;
    static String sProgressBaseCurRecord;
    static String sProgressCurRecord;
    static String sProgressTitle;
    static String sMsgQueryCreationImpossible;
    static String sReportFormNotExisting;
    static String sStop;
    static String sMsgHiddenControlMissing;
    static String sMsgEndAutopilot;
    static String sMsgConnectionImpossible;
    static String sMsgNoConnection;
    static String[] ReportMessages = new String[4];
    public static final String TBLRECORDSECTION = ReportDocument.TBLRECORDSECTION;
    public static final String TBLGROUPSECTION = ReportDocument.TBLGROUPSECTION;
    public static final String RECORDSECTION = ReportDocument.RECORDSECTION;
    public static final String GROUPSECTION = ReportDocument.GROUPSECTION;
    public static final String COPYOFTBLRECORDSECTION = ReportDocument.COPYOFTBLRECORDSECTION;
    public static final String COPYOFTBLGROUPSECTION = ReportDocument.COPYOFTBLGROUPSECTION;
    public static final String COPYOFRECORDSECTION = ReportDocument.COPYOFRECORDSECTION;
    public static final String COPYOFGROUPSECTION = ReportDocument.COPYOFGROUPSECTION;


    public Dataimport(XMultiServiceFactory _xMSF) {
        super(_xMSF);
        super.addResourceHandler("ReportWizard", "dbw");
        sProgressDBConnection = oResource.getResText(UIConsts.RID_DB_COMMON + 34);
        sProgressDataImport = oResource.getResText(UIConsts.RID_REPORT + 67);
        sProgressTitle = oResource.getResText(UIConsts.RID_REPORT + 62);
        sProgressBaseCurRecord = oResource.getResText(UIConsts.RID_REPORT + 63);
        sReportFormNotExisting = oResource.getResText(UIConsts.RID_REPORT + 64);
        sMsgQueryCreationImpossible = oResource.getResText(UIConsts.RID_REPORT + 65);
        sStop = oResource.getResText(UIConsts.RID_DB_COMMON + 21);
        sMsgHiddenControlMissing = oResource.getResText(UIConsts.RID_REPORT + 66);
        sMsgEndAutopilot = oResource.getResText(UIConsts.RID_DB_COMMON + 33);
        sMsgNoConnection = oResource.getResText(UIConsts.RID_DB_COMMON + 14);
    }

    public void disposing(com.sun.star.lang.EventObject eventObject){
    }


    public void actionPerformed(com.sun.star.awt.ActionEvent actionEvent) {
        bStopProcess = true;
    }

    public static void main(String args[]) {
        String ConnectStr = "uno:socket,host=localhost,port=8100;urp,negotiate=0,forcesynchronous=1;StarOffice.NamingService";
        XMultiServiceFactory xMSF = null;
        try {
            xMSF = com.sun.star.wizards.common.Desktop.connect(ConnectStr);
            if (xMSF != null)
                System.out.println("Connected to " + ConnectStr);
            PropertyValue[] curproperties = new PropertyValue[3];
            curproperties[0] = Properties.createProperty("DatabaseLocation", "file:///C:/Documents and Settings/bc93774.EHAM02-DEV/My Documents/MyHSQL.odb");
//            curproperties[0] = Properties.createProperty("DatabaseLocation", "file:///C:/Documents and Settings/bc93774.EHAM02-DEV/My Documents/MyDocAssign.odb"); //baseLocation ); "DataSourceName", "db1");
//            curproperties[0] = Properties.createProperty("DataSourceName", "Bibliography");
            curproperties[1] = Properties.createProperty("CommandType", new Integer(CommandType.TABLE));
            curproperties[2] = Properties.createProperty("Command", "Table2");

            Dataimport CurDataimport = new Dataimport(xMSF);
            TextDocument oTextDocument = new TextDocument(xMSF, true, null);
            CurDataimport.createReport(xMSF, oTextDocument.xTextDocument, curproperties);

        } catch (Exception e) {
            e.printStackTrace(System.out);
        } catch (java.lang.Exception javaexception) {
            javaexception.printStackTrace(System.out);
        }
    }

    public void showProgressDisplay(XMultiServiceFactory xMSF, boolean bgetConnection) {
        try {
            Helper.setUnoPropertyValues(xDialogModel,
                        new String[] { "Height", "Step", "Title", "Width" },
                        new Object[] { new Integer(84), new Integer(0), sProgressTitle, new Integer(180)});
            com.sun.star.awt.FontDescriptor oFontDesc = new com.sun.star.awt.FontDescriptor();
            oFontDesc.Weight = com.sun.star.awt.FontWeight.BOLD;
            if (bgetConnection) {
                insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblProgressDBConnection",
                    new String[] { "FontDescriptor", "Height", "Label", "PositionX", "PositionY", "Step", "Width" },
                    new Object[] { oFontDesc, new Integer(10), sProgressDBConnection, new Integer(6), new Integer(6), new Integer(0), new Integer(150)});

                insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblProgressDataImport",
                    new String[] { "Height", "Label", "PositionX", "PositionY", "Step", "Width" },
                    new Object[] { new Integer(10), sProgressDataImport, new Integer(6), new Integer(24), new Integer(0), new Integer(120)});
            } else
                insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblProgressDataImport",
                    new String[] { "FontDescriptor", "Height", "Label", "PositionX", "PositionY", "Step", "Width" },
                    new Object[] { oFontDesc, new Integer(10), sProgressDataImport, new Integer(6), new Integer(24), new Integer(0), new Integer(120)});

            insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblCurProgress",
                    new String[] { "Height", "Label", "PositionX", "PositionY", "Step", "Width" },
                    new Object[] { new Integer(10), "", new Integer(12), new Integer(42), new Integer(0), new Integer(120)});

            insertButton("cmdCancel", 10000, this,
                    new String[] { "Height", "HelpURL", "PositionX", "PositionY", "Step", "TabIndex", "Width", "Label" },
                    new Object[] { new Integer(14), "HID:34321", new Integer(74), new Integer(58), new Integer(0), new Short((short) 1), new Integer(40), sStop });
            createWindowPeer(CurReportDocument.xWindowPeer);
            calculateDialogPosition(CurReportDocument.xFrame.getComponentWindow().getPosSize());
            xWindow.setVisible(true);
            super.xReschedule.reschedule();
            return;
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
            return;
        } catch (java.lang.Exception jexception) {
            jexception.printStackTrace(System.out);
            return;
        }
    }


    private void addTextSectionCopies(){
        CurReportDocument.setLayoutSectionsVisible(false);
        XTextCursor xTextCursor = ReportDocument.createTextCursor(CurReportDocument.xTextDocument.getText());
        xTextCursor.gotoStart(false);
        for (int i = 0; i < CurReportDocument.CurDBMetaData.GroupFieldNames.length; i++){
            XNamed xNamedTextSection = addLinkedTextSection(xTextCursor, GROUPSECTION + Integer.toString(i + 1), null, null);
            xNamedTextSection.setName(COPYOFGROUPSECTION + (i+1));
            renameTableofLastSection(COPYOFTBLGROUPSECTION + (i+1));
        }
        if( CurReportDocument.CurDBMetaData.RecordFieldNames.length > 0){
            XNamed xNamedTextSection = addLinkedTextSection(xTextCursor, RECORDSECTION, null, null);
            xNamedTextSection.setName(COPYOFRECORDSECTION);
            renameTableofLastSection(COPYOFTBLRECORDSECTION);
        }
    }


    private void renameTableofLastSection(String _snewname){
        XTextTable xTextTable = CurReportDocument.oTextTableHandler.getlastTextTable();
        XNamed xNamedTable = (XNamed) UnoRuntime.queryInterface(XNamed.class, xTextTable);
        xNamedTable.setName(_snewname);
    }


    public void importReportData(final XMultiServiceFactory xMSF, final Dataimport CurDataimport, final ReportDocument CurReportDocument) {
        if (reconnectToDatabase(xMSF)) {
            // The following calls to remove the Sections must occur after the connection to the database
            modifyFontWeight("lblProgressDBConnection", com.sun.star.awt.FontWeight.NORMAL);
            modifyFontWeight("lblProgressDataImport", com.sun.star.awt.FontWeight.BOLD);
            insertDatabaseDatatoReportDocument(xMSF);
        }
        xComponent.dispose();
        CurReportDocument.CurDBMetaData.dispose();
    }


    public void createReport(final XMultiServiceFactory xMSF,XTextDocument _textDocument, PropertyValue[] properties) {
        CurReportDocument = new ReportDocument(xMSF, _textDocument,oResource);
        CurProperties = properties;
        showProgressDisplay(xMSF, true);
        importReportData(xMSF, this, CurReportDocument);
    }

    public boolean reconnectToDatabase(XMultiServiceFactory xMSF) {
        try {
            XNameContainer xNamedForms = CurReportDocument.oFormHandler.getDocumentForms();
            Object oDBForm = Helper.getUnoObjectbyName(xNamedForms, ReportWizard.SOREPORTFORMNAME);
            boolean bgetConnection;
            String sQueryName = "";
            if (oDBForm != null) {
                String sMsg = sMsgHiddenControlMissing + (char) 13 + sMsgEndAutopilot;
                XNameAccess xNamedForm = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, oDBForm);
                CurReportDocument.CurDBMetaData.Command = CurReportDocument.oFormHandler.getValueofHiddenControl(xNamedForm, "Command", sMsg);
                String sCommandType = CurReportDocument.oFormHandler.getValueofHiddenControl(xNamedForm, "CommandType", sMsg);
                String sGroupFieldNames = CurReportDocument.oFormHandler.getValueofHiddenControl(xNamedForm, "GroupFieldNames", sMsg);
                String sFieldNames = CurReportDocument.oFormHandler.getValueofHiddenControl(xNamedForm, "FieldNames", sMsg);
                String sRecordFieldNames = CurReportDocument.oFormHandler.getValueofHiddenControl(xNamedForm, "RecordFieldNames", sMsg);
                if (xNamedForm.hasByName("QueryName"))
                    sQueryName = CurReportDocument.oFormHandler.getValueofHiddenControl(xNamedForm, "QueryName", sMsg);
        String[] sFieldNameList =  JavaTools.ArrayoutofString(sFieldNames, ";");
                CurReportDocument.CurDBMetaData.RecordFieldNames = JavaTools.ArrayoutofString(sRecordFieldNames, ";");
                CurReportDocument.CurDBMetaData.GroupFieldNames = JavaTools.ArrayoutofString(sGroupFieldNames, ";");
                CurReportDocument.CurDBMetaData.setCommandType(Integer.valueOf(sCommandType).intValue());
                sMsgQueryCreationImpossible = JavaTools.replaceSubString(sMsgQueryCreationImpossible, CurReportDocument.CurDBMetaData.Command, "<STATEMENT>");
                bgetConnection = CurReportDocument.CurDBMetaData.getConnection(CurProperties);
                int nCommandType = com.sun.star.sdb.CommandType.COMMAND;
                boolean bexecute = false;
                if (bgetConnection){
                    bexecute = CurReportDocument.CurDBMetaData.executeCommand(nCommandType); //sMsgQueryCreationImpossible + (char) 13 + sMsgEndAutopilot, sFieldNameList, true);
                    if (bexecute){
                        bexecute = CurReportDocument.CurDBMetaData.getFields(sFieldNameList, true);
                    }
                    return bexecute;
                    } else
                        return false;
            } else {
                sReportFormNotExisting = JavaTools.replaceSubString(sReportFormNotExisting, ReportWizard.SOREPORTFORMNAME, "<REPORTFORM>");
                showMessageBox("ErrorBox", VclWindowPeerAttribute.OK, sReportFormNotExisting + (char) 13 + sMsgEndAutopilot);
                return false;
            }
        } catch (InvalidQueryException queryexception) {
            return false;
        } catch (java.lang.Exception javaexception) {
            javaexception.printStackTrace(System.out);
            return false;
        } catch (com.sun.star.wizards.document.FormHandler.UnknownHiddenControlException exception) {
            return false;
        }
    }

    public void insertDatabaseDatatoReportDocument(XMultiServiceFactory xMSF) {
        try {
            int ColIndex;
            boolean breset;
            Object oTable;
            Vector DataVector = new Vector();
            DBColumn CurDBColumn;
            Object CurGroupValue;
            String CurGroupTableName;
            RecordParser CurDBMetaData = CurReportDocument.CurDBMetaData;
            com.sun.star.style.BreakType CorrBreakValue = null;
            String CorrPageDescName = "";
            CurReportDocument.oTextFieldHandler.fixDateFields(true);
            CurReportDocument.removeAllVisibleTextSections();
            CurReportDocument.removeNonLayoutTextTables();
            addTextSectionCopies();
            CurReportDocument.getallDBColumns();
            int GroupFieldCount = CurDBMetaData.GroupFieldNames.length;
            int FieldCount = CurDBMetaData.FieldColumns.length;
            Object[] OldGroupFieldValues = new Object[GroupFieldCount];
            XTextTable[] xGroupBaseTables = new XTextTable[GroupFieldCount];
            int RecordFieldCount = FieldCount - GroupFieldCount;
            XTextDocument xTextDocument = CurReportDocument.xTextDocument;
            XTextCursor xTextCursor = ReportDocument.createTextCursor(CurReportDocument.xTextDocument.getText());
            xTextDocument.lockControllers();
            if (CurDBMetaData.ResultSet.next() == true) {
                replaceUserFields();
                Helper.setUnoPropertyValue(xTextCursor, "PageDescName", "First Page");
                for (ColIndex = 0; ColIndex < GroupFieldCount; ColIndex++) {
                    CurGroupTableName = TBLGROUPSECTION + Integer.toString(ColIndex + 1);
                    oTable = CurReportDocument.oTextTableHandler.xTextTablesSupplier.getTextTables().getByName(CurGroupTableName);
                    xGroupBaseTables[ColIndex] = (XTextTable) UnoRuntime.queryInterface(XTextTable.class, oTable);
                    CurGroupValue = CurDBMetaData.getGroupColumnValue(ColIndex);
                    OldGroupFieldValues[ColIndex] = CurGroupValue;
                    CurDBColumn = (DBColumn) CurReportDocument.DBColumnsVector.elementAt(ColIndex);
                    addLinkedTextSection(xTextCursor, GROUPSECTION + Integer.toString(ColIndex + 1), CurDBColumn, CurGroupValue); //COPYOF!!!!
                }
                if (CurDBMetaData.getcurrentRecordData(DataVector) == true) {
                    int RowIndex = 1;
                    bStopProcess = false;
                    while ((CurDBMetaData.ResultSet.next() == true) && (bStopProcess == false)) {
                        RowIndex += 1;
                        breset = false;
                        for (ColIndex = 0; ColIndex < GroupFieldCount; ColIndex++) {
                            CurGroupValue = CurDBMetaData.getGroupColumnValue(ColIndex);
                            if ((CurGroupValue.equals((Object) OldGroupFieldValues[ColIndex]) == false) || (breset)) {
                                breset = true;
                                insertDataToRecordTable(xTextCursor, DataVector, RecordFieldCount);
                                CurDBColumn = (DBColumn) CurReportDocument.DBColumnsVector.elementAt(ColIndex);
                                addLinkedTextSection(xTextCursor, COPYOFGROUPSECTION + Integer.toString(ColIndex + 1), CurDBColumn, CurGroupValue);
                                OldGroupFieldValues[ColIndex] = CurGroupValue;
                                breset = !(ColIndex == GroupFieldCount - 1);
                            }
                        }
                        CurDBMetaData.getcurrentRecordData(DataVector);
                        updateProgressDisplay(RowIndex);
                    }
                    insertDataToRecordTable(xTextCursor, DataVector, RecordFieldCount);
                } else {
                    CurReportDocument.unlockallControllers();
                    return;
                }
            }
            else{
                for (ColIndex = 0; ColIndex < GroupFieldCount; ColIndex++) {
                    CurDBColumn = (DBColumn) CurReportDocument.DBColumnsVector.elementAt(ColIndex);
                    Object oValue = "";
                    addLinkedTextSection(xTextCursor, COPYOFGROUPSECTION + Integer.toString(ColIndex + 1), CurDBColumn, oValue);
                }
                addLinkedTextSection(xTextCursor, COPYOFRECORDSECTION, null, null);
                Object[][] RecordArray = new Object[1][RecordFieldCount];
                for (int i = 0; i < RecordArray[0].length; i++){
                    RecordArray[0][i] = Any.VOID;
                }
                XTextTable xTextTable = CurReportDocument.oTextTableHandler.getlastTextTable();
                OfficeDocument.ArraytoCellRange(RecordArray, xTextTable, 0, 1);
            }
            CurReportDocument.oTextSectionHandler.breakLinkofTextSections();
        } catch (Exception exception) {
        } catch (java.lang.Exception javaexception) {
            javaexception.printStackTrace(System.out);
        }
        CurReportDocument.unlockallControllers();
        CurReportDocument.setLayoutSectionsVisible(false);
        CurReportDocument.removeCopiedTextSections();
        CurReportDocument.oTextSectionHandler.removeInvisibleTextSections();
        CurReportDocument.removeLayoutTextTables();
    }


    public void insertDataToRecordTable(XTextCursor xTextCursor, Vector DataVector, int FieldCount) {
        int DataLength = DataVector.size();
        if ((FieldCount > 0) && (DataLength > 0)) {
            addLinkedTextSection(xTextCursor, COPYOFRECORDSECTION, null, null);
            Object[][] RecordArray = new Object[DataLength][FieldCount];
            DataVector.copyInto(RecordArray);
            XTextTable xTextTable = CurReportDocument.oTextTableHandler.getlastTextTable();
            if (DataLength > 1)
                xTextTable.getRows().insertByIndex(xTextTable.getRows().getCount(), DataLength - 1);
            OfficeDocument.ArraytoCellRange(RecordArray, xTextTable, 0, 1);
        }
        DataVector.removeAllElements();
    }



    public void updateProgressDisplay(int iCounter) {
        try {
            if (iCounter % 10 == 0) {
                sProgressCurRecord = JavaTools.replaceSubString(sProgressBaseCurRecord, String.valueOf(iCounter), "<COUNT>");
                setControlProperty("lblCurProgress", "Label", sProgressCurRecord);
                super.xReschedule.reschedule();
            }
        } catch (java.lang.Exception jexception) {
            jexception.printStackTrace(System.out);
        }
    }

    public XNamed addLinkedTextSection(XTextCursor xTextCursor, String sLinkRegion, DBColumn CurDBColumn, Object CurGroupValue) {
    XNamed xNamedTextSection = null;
    try {
        XInterface xTextSection = (XInterface) CurReportDocument.xMSFDoc.createInstance("com.sun.star.text.TextSection");
        XTextContent xTextSectionContent = (XTextContent) UnoRuntime.queryInterface(XTextContent.class, xTextSection);
        xNamedTextSection = (XNamed) UnoRuntime.queryInterface(XNamed.class, xTextSection);
        xTextCursor.gotoEnd(false);
        xTextCursor.getText().insertTextContent(xTextCursor, xTextSectionContent, true);
        Helper.setUnoPropertyValue(xTextSection, "LinkRegion", sLinkRegion);
        if (CurDBColumn != null){
            boolean bIsGroupTable = (sLinkRegion.equals(RECORDSECTION) != true);
            if (bIsGroupTable == true){
                XTextTable xTextTable = CurReportDocument.oTextTableHandler.getlastTextTable();
                XCellRange xCellRange = (XCellRange) UnoRuntime.queryInterface(XCellRange.class, xTextTable);
                CurDBColumn.modifyCellContent(xCellRange, CurGroupValue);
            }
        }
    } catch (Exception exception) {
        exception.printStackTrace(System.out);
    }
    return xNamedTextSection;
    }


    public void replaceUserFields() {
        DBColumn CurDBColumn;
        XTextCursor xNameCellCursor;
        String FieldContent;
        int iCount = CurReportDocument.DBColumnsVector.size();
        for (int i = 0; i < iCount; i++) {
            CurDBColumn = (DBColumn) CurReportDocument.DBColumnsVector.elementAt(i);
            xNameCellCursor = ReportDocument.createTextCursor(CurDBColumn.xNameCell);
            xNameCellCursor.gotoStart(false);
            FieldContent = CurReportDocument.oTextFieldHandler.getUserFieldContent(xNameCellCursor);
            if (!FieldContent.equals("")){
                xNameCellCursor.goRight((short) 1, true);
                xNameCellCursor.setString(FieldContent);
            }
        }
    }
}
