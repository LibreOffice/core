/*************************************************************************
*
*  $RCSfile: Dataimport.java,v $
*
*  $Revision: 1.28 $
*
*  last change: $Author: hr $ $Date: 2004-08-02 17:20:16 $
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

import java.util.Vector;

import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNamed;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.table.XCellRange;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextTable;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.wizards.db.*;
import com.sun.star.wizards.ui.*;
import com.sun.star.wizards.common.*;
import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.document.*;
import com.sun.star.wizards.text.*;
import com.sun.star.wizards.common.InvalidQueryException;
import com.sun.star.uno.Exception;

public class Dataimport extends UnoDialog2 { // extends ReportWizard

    ReportDocument CurReportDocument;
    static boolean bStopProcess;
    static boolean brenamefirstTable = true;
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
    //          sMsgConnectionImpossible = oResource.getResText(UIConsts.RID_COMMON + 35);
    static String[] ReportMessages = new String[4];

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

    class ActionListenerImpl implements com.sun.star.awt.XActionListener {

        public void disposing(com.sun.star.lang.EventObject eventObject) {
        }
        public void actionPerformed(com.sun.star.awt.ActionEvent actionEvent) {
            bStopProcess = true;
        }
    }

    public static void main(String args[]) {
        String ConnectStr = "uno:socket,host=localhost,port=8100;urp,negotiate=0,forcesynchronous=1;StarOffice.NamingService";
        XMultiServiceFactory xMSF = null;
        try {
            xMSF = com.sun.star.wizards.common.Desktop.connect(ConnectStr);
            if (xMSF != null)
                System.out.println("Connected to " + ConnectStr);
//          Dataimport CurDataimport = new Dataimport(xMSF);
//          CurDataimport.createReport(xMSF);
        } catch (Exception e) {
            e.printStackTrace(System.out);
        } catch (java.lang.Exception javaexception) {
            javaexception.printStackTrace(System.out);
        }
    }

    public void showProgressDisplay(XMultiServiceFactory xMSF, boolean bgetConnection) {
        try {
            int iHeight;
            Helper.setUnoPropertyValues(xDialogModel, new String[] { "Height", "Step", "Title", "Width" }, new Object[] { new Integer(84), new Integer(0), sProgressTitle, new Integer(180)});
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

            insertButton("cmdCancel", 10000, new ActionListenerImpl(),
                    new String[] { "Height", "HelpURL", "PositionX", "PositionY", "Step", "TabIndex", "Width", "Label" },
                    new Object[] { new Integer(14), "HID:34321", new Integer(74), new Integer(58), new Integer(0), new Short((short) 1), new Integer(40), sStop });
            createWindowPeer(CurReportDocument.xWindowPeer);
            calculateDialogPosition(CurReportDocument.xFrame.getComponentWindow().getPosSize());
            xWindow.setVisible(true);
            return;
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
            return;
        } catch (java.lang.Exception jexception) {
            jexception.printStackTrace(System.out);
            return;
        }
    }

    public void importReportData(final XMultiServiceFactory xMSF, final Dataimport CurDataimport, final ReportDocument CurReportDocument) {
        /*    Thread ProgressThread = new Thread(new Runnable(CurUnoProgressDialog) {
                private UnoControl dialog;
            public Runnable( UnoControl x )
            {
                dialog = x;
            }*/

        // TODO: the dialog has to be in a thread again, but before the deadlock has to be fixed which otherwise appears
        try {
            if (reconnectToDatabase(xMSF)) {
                modifyFontWeight("lblProgressDBConnection", com.sun.star.awt.FontWeight.NORMAL);
                modifyFontWeight("lblProgressDataImport", com.sun.star.awt.FontWeight.BOLD);
                insertDatabaseDatatoReportDocument(xMSF);
            }
            xComponent.dispose();
            CurReportDocument.CurDBMetaData.disposeDBMetaData();
        } catch (ThreadDeath td) {
            System.out.println("could not stop thread");
            xComponent.dispose();
        }
/*
        Thread ProgressThread = new Thread(new Runnable() {
            public void run() {
                try {
                    if (reconnectToDatabase(xMSF)) {
                        modifyFontWeight("lblProgressDBConnection", com.sun.star.awt.FontWeight.NORMAL);
                        modifyFontWeight("lblProgressDataImport", com.sun.star.awt.FontWeight.BOLD);
                        insertDatabaseDatatoReportDocument(xMSF);
                    }
                    xComponent.dispose();
                    CurReportDocument.CurDBMetaData.disposeDBMetaData();
                } catch (ThreadDeath td) {
                    System.out.println("could not stop thread");
                    xComponent.dispose();
                }
            }
        });

        ProgressThread.start();
        */
    }


    public void createReport(final XMultiServiceFactory xMSF,XTextDocument _textDocument) {
        try {
            CurReportDocument = new ReportDocument(xMSF, _textDocument,false, oResource);
            int iWidth = CurReportDocument.xFrame.getComponentWindow().getPosSize().Width;
            showProgressDisplay(xMSF, true);
            importReportData(xMSF, this, CurReportDocument);
        } catch (java.lang.Exception jexception) {
            jexception.printStackTrace(System.out);
        }
    }

    public boolean reconnectToDatabase(XMultiServiceFactory xMSF) {
        try {
            com.sun.star.container.XNameContainer xNamedForms = CurReportDocument.oFormHandler.getDocumentForms();
            Object oDBForm = Helper.getUnoObjectbyName(xNamedForms, ReportWizard.SOREPORTFORMNAME);
            boolean bgetConnection;
            if (oDBForm != null) {
                String sMsg = sMsgHiddenControlMissing + (char) 13 + sMsgEndAutopilot;
                XNameAccess xNamedForm = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, oDBForm);
                // Todo make up an exception class to catch that error
                String DataSourceName = (String) CurReportDocument.oFormHandler.getValueofHiddenControl(xNamedForm, "DataSourceName", sMsg);
                CurReportDocument.CurDBMetaData.Command = CurReportDocument.oFormHandler.getValueofHiddenControl(xNamedForm, "Command", sMsg);
                String sCommandType = CurReportDocument.oFormHandler.getValueofHiddenControl(xNamedForm, "CommandType", sMsg);
                String sGroupFieldNames = CurReportDocument.oFormHandler.getValueofHiddenControl(xNamedForm, "GroupFieldNames", sMsg);
                String sFieldNames = CurReportDocument.oFormHandler.getValueofHiddenControl(xNamedForm, "FieldNames", sMsg);
                String sRecordFieldNames = CurReportDocument.oFormHandler.getValueofHiddenControl(xNamedForm, "RecordFieldNames", sMsg);
                CurReportDocument.CurDBMetaData.FieldNames = JavaTools.ArrayoutofString(sFieldNames, ";");
                CurReportDocument.CurDBMetaData.RecordFieldNames = JavaTools.ArrayoutofString(sRecordFieldNames, ";");
                CurReportDocument.CurDBMetaData.GroupFieldNames = JavaTools.ArrayoutofString(sGroupFieldNames, ";");
                CurReportDocument.CurDBMetaData.CommandType = Integer.valueOf(sCommandType).intValue();
                sMsgQueryCreationImpossible = JavaTools.replaceSubString(sMsgQueryCreationImpossible, CurReportDocument.CurDBMetaData.Command, "<STATEMENT>");
                bgetConnection = CurReportDocument.CurDBMetaData.getConnection(DataSourceName);
                if (bgetConnection) {
                    boolean bexecute = CurReportDocument.CurDBMetaData.executeCommand(sMsgQueryCreationImpossible + (char) 13 + sMsgEndAutopilot, true);
                    if (bexecute == true)
                        CurReportDocument.getallDBColumns();
                    return bexecute;
                } else
                    return false;
            } else {
                sReportFormNotExisting = JavaTools.replaceSubString(sReportFormNotExisting, ReportWizard.SOREPORTFORMNAME, "<REPORTFORM>");
                com.sun.star.wizards.common.SystemDialog.showMessageBox(xMSF, "ErrorBox", com.sun.star.awt.VclWindowPeerAttribute.OK, sReportFormNotExisting + (char) 13 + sMsgEndAutopilot);
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
            int GroupFieldCount = CurDBMetaData.GroupFieldNames.length;
            int FieldCount = CurDBMetaData.FieldNames.length;
            Object[] OldGroupFieldValues = new Object[GroupFieldCount];
            XTextTable[] xGroupBaseTables = new XTextTable[GroupFieldCount];
            int RecordFieldCount = FieldCount - GroupFieldCount;
            com.sun.star.style.BreakType CorrBreakValue = null;
            String CorrPageDescName = "";
            XNameAccess xTextTables = CurReportDocument.oTextTableHandler.xTextTablesSupplier.getTextTables();
            XTextDocument xTextDocument = CurReportDocument.xTextDocument;
            XTextCursor xTextCursor = CurReportDocument.createTextCursor(CurReportDocument.xTextDocument.getText());
            //  xTextDocument.lockControllers();
            if (CurDBMetaData.ResultSet.next() == true) {
                replaceUserFields();
                Helper.setUnoPropertyValue(xTextCursor, "PageDescName", "First Page");
                for (ColIndex = 0; ColIndex < GroupFieldCount; ColIndex++) {
                    CurGroupTableName = "Tbl_GroupField" + Integer.toString(ColIndex + 1);
                    oTable = CurReportDocument.oTextTableHandler.xTextTablesSupplier.getTextTables().getByName(CurGroupTableName);
                    xGroupBaseTables[ColIndex] = (XTextTable) UnoRuntime.queryInterface(XTextTable.class, oTable);
                    if (ColIndex == 0) {
                        CorrBreakValue = TextTableHandler.resetBreakTypeofTextTable(xGroupBaseTables[ColIndex]);
                        String PageDescName = com.sun.star.uno.AnyConverter.toString(Helper.getUnoPropertyValue(xGroupBaseTables[ColIndex], "PageDescName"));
                        if (PageDescName.equals("") == false) {
                            CorrPageDescName = PageDescName;
                            Helper.setUnoPropertyValue(xGroupBaseTables[ColIndex], "PageDescName", "");
                        }
                    }
                    CurGroupValue = CurDBMetaData.getGroupColumnValue(ColIndex);
                    OldGroupFieldValues[ColIndex] = CurGroupValue;
                    CurDBColumn = (DBColumn) CurReportDocument.DBColumnsVector.elementAt(ColIndex);
                    addLinkedTextSection(xTextCursor, "GroupField" + Integer.toString(ColIndex + 1), CurDBColumn, CurGroupValue);
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
                                addLinkedTextSection(xTextCursor, "GroupField" + Integer.toString(ColIndex + 1), CurDBColumn, CurGroupValue);
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
            setLayoutSectionsInvisible(GroupFieldCount);
            CurReportDocument.oTextSectionHandler.breakLinkofTextSections();
            if (CurReportDocument.oTextTableHandler.xTextTablesSupplier.getTextTables().hasByName("FirstVisibleTextTable")) {
                Object oTextTable = CurReportDocument.oTextTableHandler.xTextTablesSupplier.getTextTables().getByName("FirstVisibleTextTable");
                if (CorrBreakValue != null)
                    Helper.setUnoPropertyValue(oTextTable, "BreakType", CorrBreakValue);
                if (CorrPageDescName != "")
                    Helper.setUnoPropertyValue(oTextTable, "PageDescName", CorrPageDescName);
            }
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        } catch (java.lang.Exception javaexception) {
            javaexception.printStackTrace(System.out);
        }
        CurReportDocument.unlockallControllers();
    }

    public void setLayoutSectionsInvisible(int GroupFieldCount) {
        try {
            XNameAccess xTextSections = CurReportDocument.oTextSectionHandler.xTextSectionsSupplier.getTextSections();
            Object oTextSection;
            for (int i = 0; i < GroupFieldCount; i++) {
                oTextSection = xTextSections.getByName("GroupField" + String.valueOf(i + 1));
                Helper.setUnoPropertyValue(oTextSection, "IsVisible", new Boolean(false));
            }
            if (xTextSections.hasByName("RecordSection")) {
                oTextSection = xTextSections.getByName("RecordSection");
                Helper.setUnoPropertyValue(oTextSection, "IsVisible", new Boolean(false));
            }
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
    }

    public void insertDataToRecordTable(XTextCursor xTextCursor, Vector DataVector, int FieldCount) {
        int DataLength = DataVector.size();
        if ((FieldCount > 0) && (DataLength > 0)) {
            addLinkedTextSection(xTextCursor, "RecordSection", null, null);
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
            }
        } catch (java.lang.Exception jexception) {
            jexception.printStackTrace(System.out);
        }
    }

    public void addLinkedTextSection(XTextCursor xTextCursor, String sLinkRegion, DBColumn CurDBColumn, Object CurGroupValue) {
        try {
            XInterface xTextSection = (XInterface) CurReportDocument.xMSFDoc.createInstance("com.sun.star.text.TextSection");
            XTextContent xTextSectionContent = (XTextContent) UnoRuntime.queryInterface(XTextContent.class, xTextSection);
            xTextCursor.gotoEnd(false);
            xTextCursor.getText().insertTextContent(xTextCursor, xTextSectionContent, true);
            Helper.setUnoPropertyValue(xTextSection, "LinkRegion", sLinkRegion);
            if (CurDBColumn != null) {
                boolean bIsGroupTable = (sLinkRegion.equals("RecordSection") != true);
                if (bIsGroupTable == true) {
                    XTextTable xTextTable = CurReportDocument.oTextTableHandler.getlastTextTable();
                    XCellRange xCellRange = (XCellRange) UnoRuntime.queryInterface(XCellRange.class, xTextTable);
                    CurDBColumn.modifyCellContent(xCellRange, CurGroupValue);
                }
            }
            if (brenamefirstTable == true) {
                brenamefirstTable = false;
                XTextTable xTextTable = CurReportDocument.oTextTableHandler.getlastTextTable();
                XNamed xNamedTable = (XNamed) UnoRuntime.queryInterface(XNamed.class, xTextTable);
                xNamedTable.setName("FirstVisibleTextTable");

            }
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
    }

    public void replaceUserFields() {
        DBColumn CurDBColumn;
        XTextCursor xNameCellCursor;
        String FieldContent;
        int iCount = CurReportDocument.DBColumnsVector.size();
        for (int i = 0; i < iCount; i++) {
            CurDBColumn = (DBColumn) CurReportDocument.DBColumnsVector.elementAt(i);
            xNameCellCursor = CurReportDocument.createTextCursor(CurDBColumn.xNameCell);
            xNameCellCursor.gotoStart(false);
            FieldContent = CurReportDocument.oTextFieldHandler.getUserFieldContent(xNameCellCursor);
            xNameCellCursor.goRight((short) 1, true);
            xNameCellCursor.setString(FieldContent);
        }
    }
}
