 /*************************************************************************
 *
 *  $RCSfile: ReportWizard.java,v $
 *
 *  $Revision: 1.27 $
 *
 *  last change: $Author: bc $ $Date: 2002-08-14 15:24:48 $
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

import com.sun.star.registry.XRegistryKey;

import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XElementAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.container.XEnumeration;
import com.sun.star.container.XNameContainer;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XMultiPropertySet;
import com.sun.star.beans.PropertyAttribute;

import com.sun.star.comp.servicemanager.ServiceManager;
import com.sun.star.comp.loader.FactoryHelper;

import com.sun.star.connection.XConnector;
import com.sun.star.connection.XConnection;
import com.sun.star.sdb.XCompletedConnection;
import com.sun.star.sdbcx.XColumnsSupplier;
import com.sun.star.sdbc.*;
import com.sun.star.sdb.*;
import com.sun.star.sdbcx.XTablesSupplier;
import com.sun.star.sdbcx.XColumnsSupplier;
import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.XNamingService;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Any;

import com.sun.star.sheet.*;
import com.sun.star.document.*;
import com.sun.star.table.*;
import com.sun.star.text.*;
import com.sun.star.text.XTextRange;
import com.sun.star.text.XTextDocument;
import com.sun.star.task.XInteractionHandler;
import com.sun.star.util.XNumberFormats;
import com.sun.star.util.XNumberFormatsSupplier;
import com.sun.star.util.NumberFormat;

import com.sun.star.lang.Locale;
import com.sun.star.lang.*;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.XTypeProvider;

import com.sun.star.script.XInvocation;
import com.sun.star.awt.XListBox;
import com.sun.star.awt.XRadioButton;
import com.sun.star.awt.XControl;
import com.sun.star.awt.XControlModel;
import com.sun.star.awt.XControlContainer;
import com.sun.star.awt.XWindow;
import com.sun.star.awt.XToolkit;
import com.sun.star.awt.XDialog;
import com.sun.star.awt.XButton;
import com.sun.star.awt.XFixedText;
import com.sun.star.awt.ItemEvent;
import com.sun.star.awt.ActionEvent;
import com.sun.star.awt.XActionListener;
import com.sun.star.awt.XItemListener;
import com.sun.star.awt.XMouseListener;
import com.sun.star.awt.XTextComponent;
import com.sun.star.awt.MouseEvent;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XFramesSupplier;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.util.XURLTransformer;

import java.io.*;
import java.util.*;



public class ReportWizard {
     static XMultiServiceFactory xGlobalMSF;
     DBMetaData CurDBMetaData;
     UNODialogs CurUNODialog;
     UNODialogs CurUNOProgressDialog;
     String[] DatabaseNames;
     final String SOREPORTFORMNAME = "ReportSource";
     final int MAXSORTCRITERIA = 3;

     final int SOCMDCANCEL = 1;
     final int SOCMDHELP = 2;
     final int SOCMDBACK = 3;
     final int SOCMDGOON = 4;
     final int SOCMDMOVESEL = 5;
     final int SOCMDMOVEALL = 6;
     final int SOCMDREMOVESEL = 7;
     final int SOCMDREMOVEALL = 8;
     final int SOFLDSLST =  9;
     final int SODBLST =  10;
     final int SOTBLLST =  11;
     final int SOSELFLDSLST =  12;
     final int SOGROUPLST = 13;

     final int SOFIRSTSORTLST = 14;
     final int SOSECSORTLST = 15;
     final int SOTHIRDSORTLST = 16;
     final int SOFOURTHSORTLST = 17;
     final int[] SOSORTLST = new int[]{SOFIRSTSORTLST, SOSECSORTLST, SOTHIRDSORTLST, SOFOURTHSORTLST};

     final int SOFIRSTSORTASCOPT = 18;
     final int SOSECSORTASCOPT = 19;
     final int SOTHIRDSORTASCOPT = 20;
     final int SOFOURTHSORTASCOPT = 21;

     final int SOFIRSTSORTDESCOPT = 22;
     final int SOSECSORTDESCOPT = 23;
     final int SOTHIRDSORTDESCOPT = 24;
     final int SOFOURTHSORTDESCOPT = 25;

     final int[] SOSORTASCENDOPT = new int[] {SOFIRSTSORTASCOPT, SOSECSORTASCOPT, SOTHIRDSORTASCOPT, SOFOURTHSORTASCOPT};
     final int[] SOSORTDESCENDOPT = new int[] {SOFIRSTSORTDESCOPT, SOSECSORTDESCOPT, SOTHIRDSORTDESCOPT, SOFOURTHSORTDESCOPT};

     final int SOCMDGROUPIN = 26;
     final int SOCMDGROUPOUT = 27;
     final int SOTXTTITLE = 28;
     final int SOCONTENTLST = 29;

     final int SOOPTLANDSCAPE = 30;
     final int SOOPTPORTRAIT = 31;
     final int SOLAYOUTLST = 32;
     final int SOSELGROUPLST = 33;
//     final int SOOPTSAVEASTEMPLATE = 34;
//     final int SOOPTSAVEASDOCUMENT = 35;

     final int SOOPTSAVEASTEMPLATE = 40;
     final int SOOPTEDITTEMPLATE = 41;
     final int SOOPTUSEDOCUMENT = 42;
     final int SOOPTSAVEASDOCUMENT = 43;

     final int SOCMDFIRSTPATHSELECT = 44;
     final int SOCMDSECPATHSELECT = 45;
     final int SOTXTFIRSTSAVEPATH = 46;
     final int SOTXTSECSAVEPATH = 47;

     final int[] SOCMDSELECTPATH = new int[] {SOCMDFIRSTPATHSELECT, SOCMDSECPATHSELECT};
     final int[] SOTXTSAVEPATH = new int[] {SOTXTFIRSTSAVEPATH, SOTXTSECSAVEPATH};

     int MaxSortIndex = -1;
     String[] sDatabaseList;
     static String[] TableNames;
     static String[] QueryNames;
     static String[] sSortHeader = new String[4];
     static String[]sSortAscend = new String[4];
     static String[]sSortDescend = new String[4];
     XTextComponent xTitleTextBox;
     XTextComponent[] xSaveTextBox = new XTextComponent[2];

     XListBox xDBListBox;
     XListBox xTableListBox;
     XListBox xFieldsListBox;
     XListBox xSelFieldsListBox;
     XListBox xContentListBox;
     XListBox xSelGroupListBox;
     Object chkTemplate;
     XListBox[] xSortListBox = new XListBox[4];
     XListBox xGroupListBox;
     XListBox xLayoutListBox;
     Object lblHeaderText;
     XNameAccess xTableNames;
     XNameAccess xQueryNames;

     XTablesSupplier xDBTables;
     String TableName;
     Object oDBTable;
     int[] iCommandTypes;
     int[][] WidthList;
     boolean bEnableBinaryOptionGroup;
     boolean bcreateTemplate;
     boolean buseTemplate;
     boolean bcreateLink;

     String[] OriginalList = new String[]{""};
     static XDesktop xDesktop;
     ReportDocument CurReportDocument;
     ReportPaths CurReportPaths;

     java.util.Vector GroupFieldVector;
     java.util.Vector GroupFormatVector = new java.util.Vector();
     String TemplatePath;

     static String sMsgWizardName;
     static String scmdReady;
     static String scmdCancel;
     static String scmdBack;
     static String scmdHelp;
     static String scmdGoOn;
     static String slblDatabases;
     static String slblTables;
     static String slblFields;
     static String slblSelFields;
     static String sShowBinaryFields;
     static String slblDataStructure;
     static String slblPageLayout;
     static String sOrganizeFields;
     static String sNoSorting;
     static String sOrientationHeader;
     static String sOrientVertical;
     static String sOrientHorizontal;
     static String sSaveAsTemplate;
     static String sReportTitle;
     static String sSaveAsDocument;
     static String sSaveAs;
     static String sUseTemplate;
     static String sEditTemplate;
     static String sCreateLink;
     static String sCreateLinkAutomatically;
     static String sGroupings;
     static String[] WizardHeaderText = new String[5];
     static String[] WizardTitle = new String[5];
     static String sWriterFilterName;
     static String StorePath = "";
     static String sMsgErrorOccured;
     static String sMsgNoDatabaseAvailable;
     static String sMsgCommandCouldNotbeOpened;
     static String sMsgSavingImpossible;
     static String sMsgLinkCreationImpossible;
     static String sMsgNoTableInDatabase;
     static String sMsgNoConnection;
// Progress display relevant Strings
     static String sProgressTitle;
     static String sStop;
     static String sProgressDBConnection;
     static String sProgressDataImport;
     static String sProgressBaseCurRecord;
     static String sProgressCurRecord;
     static String sReportFormNotExisting;
     static String sMsgEndAutopilot;
     static String sMsgTableNotExisting;
     static String sMsgNoDBConnection;
     static String sMsgConnectionImpossible;
     static String sMsgNoConnectionforDataimport;
     static String sMsgQueryCreationImpossible;
     static String sMsgHiddenControlMissing;
     static String sMsgFilePathInvalid;
     boolean bCloseDocument;

    public ReportWizard() {
    }

    private void getSelectedDBMetaData(XMultiServiceFactory xMSF)
        throws com.sun.star.container.NoSuchElementException, com.sun.star.beans.UnknownPropertyException,
               com.sun.star.lang.IllegalArgumentException, com.sun.star.lang.WrappedTargetException{
    try{
        short DBIndex = xDBListBox.getSelectedItemPos();
        String sDBName = DatabaseNames[DBIndex];
    boolean bGetConnection = CurDBMetaData.getConnection(xMSF,CurReportDocument, sDBName, sMsgNoConnection, sMsgConnectionImpossible);
        if (bGetConnection == true){
        CurDBMetaData.DataSourceName = sDBName;
        String[] ContentList = CurDBMetaData.getDBMetaData(CurReportDocument);
            CurUNODialog.assignPropertyToDialogControl("lstTables", "StringItemList", ContentList);
            if (ContentList != null){
                iCommandTypes = DBMetaData.createCommandTypeList();
            }
        }
    else{
            CurUNODialog.assignPropertyToDialogControl("lstDatabases", "SelectedItems", new Short[]{});
    }
        CurUNODialog.emptyFieldsListBoxes(xFieldsListBox, xSelFieldsListBox);
        CurUNODialog.assignPropertyToDialogControl("lstTables", "Enabled", new Boolean(bGetConnection));
        CurUNODialog.assignPropertyToDialogControl("lblTables", "Enabled", new Boolean(bGetConnection));
    CurUNODialog.assignPropertyToDialogControl("cmdGoOn", "Enabled", new Boolean(false));
    }
    catch(Exception exception)
    {
        exception.printStackTrace(System.out);
    }}


    public int addFieldNamestoListBox(UNODialogs CurUNODialog){
    int FieldCount = 0;
    try{
        if (xTableListBox.getItemCount() > 0){
        CurDBMetaData.getSpecificFieldNames(WidthList);
        FieldCount = CurDBMetaData.AllFieldNames.length;
        if (FieldCount > 0){
        bEnableBinaryOptionGroup = false;
        xFieldsListBox.removeItems((short) 0, xFieldsListBox.getItemCount());
        xFieldsListBox.addItems(CurDBMetaData.AllFieldNames, (short) 0);
        CurUNODialog.initializeListboxProcedures(xFieldsListBox, xSelFieldsListBox, OriginalList);
        }
        }
    }
    catch(Exception exception){
    exception.printStackTrace(System.out);
        FieldCount = 0;
    }
    return FieldCount;
    }


    private void fillUpFieldsListbox(DBMetaData CurDBMetaData, boolean bgetCommandType){
    try{
    //As the peer of the control might not yet exist we have to query the model for the SelectedItems
    short[] SelItems = (short[]) CurUNODialog.getPropertyOfDialogControl("lstTables", "SelectedItems");
    if (SelItems.length > 0) {
           int CurCommandType;
       int SelIndex = SelItems[0];
           if (bgetCommandType == true){
               CurCommandType = iCommandTypes[SelIndex];
               if (CurCommandType == com.sun.star.sdb.CommandType.QUERY){
                    int QueryIndex = SelIndex - TableNames.length;
                    TableName = QueryNames[QueryIndex];
                    oDBTable = CurDBMetaData.xQueryNames.getByName(TableName);
            CurDBMetaData.Command = (String) tools.getUNOPropertyValue(oDBTable, "Command");
           }
        else{
                    TableName = TableNames[SelIndex];
                    oDBTable = CurDBMetaData.xTableNames.getByName(TableName);
                }
        CurDBMetaData.CommandType = CurCommandType;
        CurDBMetaData.OldMainCommandName = CurDBMetaData.MainCommandName;
        CurDBMetaData.MainCommandName = TableName;
       }
           XColumnsSupplier xTableCols = (XColumnsSupplier) UnoRuntime.queryInterface(XColumnsSupplier.class, oDBTable);
           CurDBMetaData.xColumns = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, xTableCols.getColumns());
           if (addFieldNamestoListBox(CurUNODialog) > 0)
               CurUNODialog.toggleListboxControls(xFieldsListBox, xSelFieldsListBox, new Boolean(true));
           else
               CurUNODialog.emptyFieldsListBoxes(xFieldsListBox, xSelFieldsListBox);
        }
        else
            CurUNODialog.emptyFieldsListBoxes(xFieldsListBox, xSelFieldsListBox);
        }
        catch(Exception exception){
            exception.printStackTrace(System.out);
        }
    }


    class TextListenerImpl implements com.sun.star.awt.XTextListener{

    public void textChanged(com.sun.star.awt.TextEvent EventObject){
    int iPos;
    try{
            int iKey  =  CurUNODialog.getControlKey(EventObject.Source, CurUNODialog.ControlList);
            switch (iKey) {
        case SOTXTFIRSTSAVEPATH:
                CurUNODialog.assignPropertyToDialogControl("cmdGoOn", "Enabled", new Boolean(xSaveTextBox[0].getText().length() > 0));
            break;
        case SOTXTSECSAVEPATH:
                CurUNODialog.assignPropertyToDialogControl("cmdGoOn", "Enabled", new Boolean(xSaveTextBox[1].getText().length() > 0));
            break;
        case SOTXTTITLE:
            String TitleName = xTitleTextBox.getText();
            CurReportDocument.updateReportTitle(xTitleTextBox);
            break;
        }
        }
        catch(Exception exception){
            exception.printStackTrace(System.out);
    }}

    public void disposing(com.sun.star.lang.EventObject EventObject) {
        }
    }


   class ItemListenerImpl implements com.sun.star.awt.XItemListener{

       public void itemStateChanged(ItemEvent EventObject ){
       int iPos;
    try{
            int iKey  =  CurUNODialog.getControlKey(EventObject.Source, CurUNODialog.ControlList);
            switch (iKey) {
        case SODBLST:
            getSelectedDBMetaData(xGlobalMSF);
            break;

        case SOTBLLST:
            fillUpFieldsListbox(CurDBMetaData, true);
            break;

        case SOFLDSLST:
            CurUNODialog.FormSetMoveRights(xFieldsListBox, xSelFieldsListBox, (short)-1, (short)-1);
            break;

        case SOSELFLDSLST:
            CurUNODialog.FormSetMoveRights(xFieldsListBox, xSelFieldsListBox, (short)-1, (short)-1);
            break;

        case SOFIRSTSORTLST:
            enableNextSortListBox(0);
            break;

        case SOSECSORTLST:
            enableNextSortListBox(1);

        case SOTHIRDSORTLST:
            enableNextSortListBox(2);
            break;

        case SOFOURTHSORTLST:
            MaxSortIndex = 3;
            break;

        case SOGROUPLST:
            boolean bDoEnable = ((xGroupListBox.getSelectedItems().length > 0) && (GroupFieldVector.size() < 4));
            CurUNODialog.assignPropertyToDialogControl("cmdGroupOut", "Enabled", new Boolean(bDoEnable));
            break;

        case SOSELGROUPLST:
            toggleRemoveGroupButton();
            break;

        case SOCONTENTLST:
            CurReportDocument.ReportTextDocument.lockControllers();
            iPos = xContentListBox.getSelectedItemPos();
            CurReportDocument.loadSectionsfromTemplate( CurDBMetaData, GroupFormatVector, CurReportPaths.ContentFiles[0][iPos]);
            CurReportDocument.loadStyleTemplates(CurReportPaths.ContentFiles[0][iPos], "LoadTextStyles");
            CurReportDocument.ReportTextDocument.unlockControllers();
            CurReportDocument.selectFirstPage();
            break;

        case SOLAYOUTLST:
            CurReportDocument.ReportTextDocument.lockControllers();
            iPos = xLayoutListBox.getSelectedItemPos();
            boolean bOldIsCurLandscape = ((Boolean) tools.getUNOPropertyValue(CurReportDocument.ReportPageStyle, "IsLandscape")).booleanValue();
            CurReportDocument.loadStyleTemplates(CurReportPaths.LayoutFiles[0][iPos], "LoadPageStyles");
            CurReportDocument.changePageOrientation(CurReportPaths.BitmapPath, CurUNODialog, CurDBMetaData, bOldIsCurLandscape);
            CurReportDocument.ReportTextDocument.unlockControllers();
            CurReportDocument.selectFirstPage();
            break;
        default:
            break;
           }
        }
        catch( Exception exception ){
               exception.printStackTrace(System.out);
        }
     }

        public void disposing(EventObject eventObject) {
        }
    }


    class ActionListenerImpl implements com.sun.star.awt.XActionListener {

        public void disposing(EventObject eventObject) {
        }
        public void actionPerformed(ActionEvent actionEvent) {
        boolean bGoOn = true;
            try{
                int iKey  =  CurUNODialog.getControlKey(actionEvent.Source, CurUNODialog.ControlList);
                switch (iKey) {
                    case SOFLDSLST:
                       CurUNODialog.MoveSelectedListBox(xFieldsListBox, xSelFieldsListBox);
                       break;

            case SOSELFLDSLST:
                        CurUNODialog.MoveOrderedSelectedListBox(xFieldsListBox, xSelFieldsListBox, CurDBMetaData.AllFieldNames, false);
                        break;

                    case SOCMDMOVESEL:
                        CurUNODialog.MoveSelectedListBox(xFieldsListBox, xSelFieldsListBox);
                        break;

                    case SOCMDMOVEALL:
                        CurUNODialog.FormMoveAll(xFieldsListBox, xSelFieldsListBox, CurDBMetaData.AllFieldNames);
                        break;

                    case SOCMDREMOVESEL:
                        CurUNODialog.MoveOrderedSelectedListBox(xFieldsListBox, xSelFieldsListBox, CurDBMetaData.AllFieldNames, false);
                        break;

                    case SOCMDREMOVEALL:
                        CurUNODialog.MoveOrderedSelectedListBox(xFieldsListBox, xSelFieldsListBox, CurDBMetaData.AllFieldNames, true);
                        break;

                    case SOCMDGOON:
                        gotoNextStep(xGlobalMSF);
                        break;

                    case SOCMDBACK:
                        gotoPrevStep();
                        break;

                    case SOGROUPLST:
                       bGoOn = CurReportDocument.addGroupNametoDocument(xGlobalMSF, CurDBMetaData, CurUNODialog,
                                     xGroupListBox, xSelGroupListBox, GroupFieldVector, GroupFormatVector,
                                     CurReportPaths.ReportPath, sMsgTableNotExisting + (char) 13 + sMsgEndAutopilot);
                       break;

                    case SOSELGROUPLST:
                       CurReportDocument.removeGroupName(CurDBMetaData, CurUNODialog, xGroupListBox, xSelGroupListBox, GroupFieldVector, GroupFormatVector);
                       break;

                    case SOCMDGROUPOUT:
                       bGoOn = CurReportDocument.addGroupNametoDocument(xGlobalMSF, CurDBMetaData, CurUNODialog,
                                    xGroupListBox, xSelGroupListBox, GroupFieldVector, GroupFormatVector,
                                    CurReportPaths.ReportPath, sMsgTableNotExisting + (char) 13 + sMsgEndAutopilot);
                       break;

                    case SOCMDGROUPIN:
            CurReportDocument.removeGroupName(CurDBMetaData, CurUNODialog, xGroupListBox, xSelGroupListBox, GroupFieldVector, GroupFormatVector);
                       break;

                    case SOOPTLANDSCAPE:
                        CurReportDocument.changePageOrientation(CurReportPaths.BitmapPath, CurUNODialog, CurDBMetaData, true);
                        break;

                    case SOOPTPORTRAIT:
                        CurReportDocument.changePageOrientation(CurReportPaths.BitmapPath, CurUNODialog, CurDBMetaData, false);
                        break;

            case SOOPTSAVEASTEMPLATE:
            toggleSaveControls();
            break;

            case SOOPTSAVEASDOCUMENT:
            toggleSaveControls();
            break;

            case SOCMDFIRSTPATHSELECT:
            insertStorePathToTextBox(xGlobalMSF);
            break;

            case SOCMDSECPATHSELECT:
            insertStorePathToTextBox(xGlobalMSF);
            break;

            default:
//               System.err.println( exception);
                        break;
                }
        if (bGoOn == false){
            CurUNODialog.xDialog.endExecute();
        }
            }
            catch( Exception exception ){
               exception.printStackTrace(System.out);
            }
        }
    }


    public void gotoNextStep(XMultiServiceFactory xMSF){
    try{
    XWindow xWindow = null;
    boolean bSetTitle = true;
        int PageCount = 5;
        int iPage = ((Integer) tools.getUNOPropertyValue(CurUNODialog.DialogModel, "Step")).intValue();
        switch (iPage){
            case 1:
                updateSecondStep();
                break;
            case 2:
        updateThirdStep();
                break;
            case 3:
                setUpSortList();
        CurDBMetaData.RecordFieldNames = CurDBMetaData.setRecordFieldNames();
                CurDBMetaData.combineSelectStatement(TableName);
        CurReportDocument.setupRecordSection(CurReportPaths.ReportPath + "/cnt-default.stw", CurDBMetaData);
        xWindow = (XWindow) UnoRuntime.queryInterface(XWindow.class, xContentListBox);
        //TODO: A message box should pop up when a single sorting criteria has been selected more than once
        break;
        case 4:
        CurUNODialog.assignPropertyToDialogControl("cmdGoOn", "Enabled", new Boolean(false));
        CurUNODialog.assignPropertyToDialogControl("cmdGoOn", "Label", scmdReady);
        Object oFocusButton = CurUNODialog.xDlgContainer.getControl("optCreateReportTemplate");
        xWindow = (XWindow) UnoRuntime.queryInterface(XWindow.class, oFocusButton);
        break;
        case 5:
        bcreateTemplate = ((Short)  CurUNODialog.getPropertyOfDialogControl("optCreateReportTemplate", "State")).shortValue() == (short) 1;
        boolean bDocisStored;
        StorePath = getStorePath();
        if (tools.PathisValid(xGlobalMSF, StorePath)){
            if (bcreateTemplate == true){
            CurReportDocument.createDBForm(xMSF, CurDBMetaData, SOREPORTFORMNAME);
            tools.attachEventCall(CurReportDocument.ReportTextDocument, "OnNew", "macro:///Tools.Debug.FillDocument()");      //"service:com.sun.star.wizards.report.CallReportWizard?fill"
            buseTemplate = ((Short) CurUNODialog.getPropertyOfDialogControl("optUseTemplate", "State")).shortValue() == (short) 1;
            bDocisStored = tools.storeDocument(xMSF, CurReportDocument.Component , StorePath, "swriter: writer_StarOffice_XML_Writer_Template",
                                   buseTemplate, sMsgSavingImpossible + (char)13 + sMsgLinkCreationImpossible);
            if (bDocisStored == true)
                CurDBMetaData.createDBLink(CurDBMetaData.DataSource, StorePath);
            }
            else{
            bcreateLink = ((Short) CurUNODialog.getPropertyOfDialogControl("chkcreateLink", "State")).shortValue() == (short) 1;
            }
            bSetTitle = false;
            bCloseDocument = false;
            CurUNODialog.xDialog.endExecute();
        }
        else{
            UNODialogs.showMessageBox(xMSF, CurUNODialog.xWindowPeer, "ErrorBox", com.sun.star.awt.VclWindowPeerAttribute.OK, sMsgFilePathInvalid);
            if (bcreateTemplate){
            xWindow = (XWindow) UnoRuntime.queryInterface(XWindow.class, xSaveTextBox[0]);
            }
            else{
            xWindow = (XWindow) UnoRuntime.queryInterface(XWindow.class, xSaveTextBox[1]);
            }
        }
        break;
        default:
                break;
        }
    if (xWindow != null)
        xWindow.setFocus();
    if (bSetTitle == true){
        if (iPage < PageCount){
        tools.setUNOPropertyValues(CurUNODialog.DialogModel, new String[]{"Step", "Title"}, new Object[]{ new Integer(iPage + 1), WizardTitle[iPage]});
        CurUNODialog.assignPropertyToDialogControl("lblDialogHeader", "Label", WizardHeaderText[iPage]);
        }
    }
    }
    catch( Exception exception ){
        exception.printStackTrace(System.out);
    }}


    public void gotoPrevStep(){
    try{
        int iPage = ((Integer) tools.getUNOPropertyValue(CurUNODialog.DialogModel, "Step")).intValue();
        switch (iPage){
            case 2:
        CurDBMetaData.OldGroupFieldNames = CurDBMetaData.GroupFieldNames;
        break;
            case 3:
        CurDBMetaData.OldSortFieldNames = CurDBMetaData.SortFieldNames;
        break;
        case 4:
        CurReportDocument.removeTextSectionbyName("RecordSection");
        CurReportDocument.removeTextTablebyName("Tbl_RecordSection");
        break;
        case 5:
        CurUNODialog.assignPropertyToDialogControl("cmdGoOn", "Label", scmdGoOn);
                CurUNODialog.assignPropertyToDialogControl("cmdGoOn", "Enabled", new Boolean(true));
                break;
            default:
                break;
        }
        CurUNODialog.assignPropertyToDialogControl("cmdBack", "Enabled", new Boolean(iPage > 2));

        if (iPage > 1)
            tools.setUNOPropertyValues(CurUNODialog.DialogModel, new String[]{"Step", "Title"}, new Object[]{ new Integer(iPage - 1), WizardTitle[iPage-2]});
        CurUNODialog.assignPropertyToDialogControl("lblDialogHeader", "Label", WizardHeaderText[iPage-2]);
    }
      catch( Exception exception ){
           exception.printStackTrace(System.out);
    }}


   public void toggleRemoveGroupButton(){
   try{
    boolean bDoEnable = xSelGroupListBox.getSelectedItems().length > 0;
    CurUNODialog.assignPropertyToDialogControl("cmdGroupIn", "Enabled", new Boolean(true));
    }
      catch( Exception exception ){
         exception.printStackTrace(System.out);
    }}


    public void setUpSortList(){
    try{
    short iCurState = 0;
    MaxSortIndex = -1;
    String CurFieldName;
    for (int i = 0; i < 4; i++){
        if (xSortListBox[i].getSelectedItemPos() > 0)
        MaxSortIndex += 1;
        else
        break;
    }
        CurDBMetaData.SortFieldNames = new String[MaxSortIndex+1][2];
        for (int i=0; i<=MaxSortIndex; i++){
        CurFieldName = xSortListBox[i].getSelectedItem();
        // Todo: Messagebox in case a sorting criteria is set twice
        CurDBMetaData.SortFieldNames[i][0] = xSortListBox[i].getSelectedItem();
        iCurState = ((Short) CurUNODialog.getPropertyOfDialogControl("optAscend" + new Integer(i+1).toString(), "State")).shortValue();
        if (iCurState == 1)
        CurDBMetaData.SortFieldNames[i][1] = "ASC";
        else
        CurDBMetaData.SortFieldNames[i][1] = "DESC";
        }
    }
    catch( Exception exception ){
        exception.printStackTrace(System.out);
     }}


    public void enableNextSortListBox(int CurIndex){
    try{
        short iNextItemPos;
        boolean bDoEnable = (xSortListBox[CurIndex].getSelectedItemPos() > 0);      // the first Item is for "undefined"
    if (bDoEnable == true){
        if (CurIndex > MaxSortIndex)
        MaxSortIndex = CurIndex;
    }
    if (bDoEnable == false){
        if (CurIndex < MaxSortIndex - 1){
        for (int i = CurIndex + 1; i <= MaxSortIndex; i++){
            toggleSortListBox(i, false);
            if (i < MaxSortIndex)
            xSortListBox[i+1].selectItemPos((short)0,true);
        }
        }
    }
        else
            toggleSortListBox(CurIndex+1, bDoEnable);
/*  if ((bDoEnable == false) && (MaxSortIndex > CurIndex)){
            for (int i= CurIndex; i < MaxSortIndex; i++){
                iNextItemPos = xSortListBox[i+1].getSelectedItemPos();
           if (iNextItemPos != 0){
                    xSortListBox[i].selectItemPos(iNextItemPos,true);
                    xSortListBox[i+1].selectItemPos(iNextItemPos,false);
                    toggleSortListBox(i,true);
        xSortListBox[i+1].selectItemPos((short)0,true);
                }
            }
//            MaxSortIndex = MaxSortIndex - 1;
//            if (MaxSortIndex < xSortListBox.length -2)
//                toggleSortListBox(MaxSortIndex + 2, false);
        }
        else
            toggleSortListBox(CurIndex+1, bDoEnable); */
    }
      catch( Exception exception ){
        exception.printStackTrace(System.out);
     }}


    public void toggleSortListBox(int CurIndex, boolean bDoEnable){
    try{
    CurUNODialog.assignPropertyToDialogControl("lblSort" + new Integer(CurIndex+1).toString(), "Enabled", new Boolean(bDoEnable));
    CurUNODialog.assignPropertyToDialogControl("lstSort" + new Integer(CurIndex+1).toString(), "Enabled", new Boolean(bDoEnable));
    CurUNODialog.assignPropertyToDialogControl("optAscend" + new Integer(CurIndex+1).toString(), "Enabled", new Boolean(bDoEnable));
    CurUNODialog.assignPropertyToDialogControl("optDescend" + new Integer(CurIndex+1).toString(), "Enabled", new Boolean(bDoEnable));
    if (bDoEnable == false)
           xSortListBox[CurIndex].selectItemPos((short)0,true);
    }
    catch( Exception exception ){
        exception.printStackTrace(System.out);
    }}


    public String getStorePath(){
    String StorePath = "";
    try{
    boolean bStoreAsTemplate = ((Short) CurUNODialog.getPropertyOfDialogControl("optCreateReportTemplate", "State")).shortValue() == (short) 1;
    if (bStoreAsTemplate == true)
        StorePath = (String) CurUNODialog.getPropertyOfDialogControl("txtSavePath_1", "Text");
    else
        StorePath = (String) CurUNODialog.getPropertyOfDialogControl("txtSavePath_2", "Text");
    StorePath = tools.converttoURLNotation(StorePath);
    }
    catch( Exception exception ){
       exception.printStackTrace(System.out);
    }
    return StorePath;
    }


    public void insertStorePathToTextBox(XMultiServiceFactory xMSF){
    try{
    String sStorePath = "";
    String DefaultName = "Report_" + CurDBMetaData.DataSourceName + "_" + CurDBMetaData.MainCommandName;
    boolean bStoreAsTemplate = ((Short) CurUNODialog.getPropertyOfDialogControl("optCreateReportTemplate", "State")).shortValue() == (short) 1;
    if (bStoreAsTemplate == true){
        if (CurReportPaths.UserTemplatePath == null)
        CurReportPaths.UserTemplatePath = tools.getOfficePath(xMSF, "Template","user");
        sStorePath = tools.callStoreDialog(xMSF, CurReportPaths.UserTemplatePath, DefaultName + ".stw", "writer_StarOffice_XML_Writer_Template");
        CurUNODialog.assignPropertyToDialogControl("txtSavePath_1", "Text", sStorePath);
    }
    else{
        if (CurReportPaths.WorkPath == null)
        CurReportPaths.WorkPath = tools.getOfficePath(xMSF, "Work","");
        sStorePath = tools.callStoreDialog(xMSF, CurReportPaths.WorkPath, DefaultName + ".sxw",  "writer_StarOffice_XML_Writer");
        CurUNODialog.assignPropertyToDialogControl("txtSavePath_2", "Text", sStorePath);
    }
    CurUNODialog.assignPropertyToDialogControl("cmdGoOn", "Enabled", new Boolean(sStorePath != ""));
    }
    catch( Exception exception ){
        exception.printStackTrace(System.out);
    }}


    public void insertSaveControls(int YPos, int Index, boolean bDoEnable, int TabIndex, int BaseHelpID){
    try{
    String HIDString;
    CurUNODialog.insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblSaveAs_" + Integer.toString(Index+1),
                            new String[] {"Enabled", "Height", "Label", "PositionX", "PositionY", "Step", "Width"},
                            new Object[] {new Boolean(bDoEnable), new Integer(8), sSaveAs, new Integer(16), new Integer(YPos), new Integer(5), new Integer(130)});

    HIDString = "HID:" + Integer.toString(BaseHelpID);
    xSaveTextBox[Index] = CurUNODialog.insertTextField("txtSavePath_" + Integer.toString(Index+1), SOTXTSAVEPATH[Index], new TextListenerImpl(),
                            new String[] {"Enabled", "Height", "HelpURL", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
                            new Object[] {new Boolean(bDoEnable), new Integer(12), HIDString, new Integer(16), new Integer(YPos + 10), new Integer(5), new Short((short)TabIndex), new Integer(222)});

    HIDString = "HID:" + Integer.toString(BaseHelpID+1);
    CurUNODialog.insertButton("cmdSelectPath_" + Integer.toString(Index+1), SOCMDSELECTPATH[Index], new ActionListenerImpl(),
                            new String[] {"Enabled", "Height", "HelpURL", "Label", "PositionX", "PositionY", "Step",  "TabIndex", "Width"},
                            new Object[] {new Boolean(bDoEnable), new Integer(14), HIDString, "...", new Integer(248), new Integer(YPos + 9), new Integer(5), new Short((short) (TabIndex + 1)), new Integer(16)});
    }
    catch( Exception exception ){
        exception.printStackTrace(System.out);
    }}


    public void toggleSaveControls(){
    try{
    String sStorePath = "";
    Short iState = (Short) CurUNODialog.getPropertyOfDialogControl("optCreateReportTemplate", "State");
    boolean bDoTemplateEnable = iState.shortValue() == 1;
        CurUNODialog.assignPropertyToDialogControl("optEditTemplate", "Enabled", new Boolean(bDoTemplateEnable));
        CurUNODialog.assignPropertyToDialogControl("optUseTemplate", "Enabled", new Boolean(bDoTemplateEnable));

    CurUNODialog.assignPropertyToDialogControl("lblSaveAs_1", "Enabled", new Boolean(bDoTemplateEnable));
    CurUNODialog.assignPropertyToDialogControl("txtSavePath_1", "Enabled", new Boolean(bDoTemplateEnable));
    CurUNODialog.assignPropertyToDialogControl("cmdSelectPath_1", "Enabled", new Boolean(bDoTemplateEnable));
    CurUNODialog.assignPropertyToDialogControl("lblAutomaticLink", "Enabled", new Boolean(bDoTemplateEnable));

    CurUNODialog.assignPropertyToDialogControl("lblSaveAs_2", "Enabled", new Boolean(!bDoTemplateEnable));
    CurUNODialog.assignPropertyToDialogControl("txtSavePath_2", "Enabled", new Boolean(!bDoTemplateEnable));
    CurUNODialog.assignPropertyToDialogControl("cmdSelectPath_2", "Enabled", new Boolean(!bDoTemplateEnable));
    CurUNODialog.assignPropertyToDialogControl("chkcreateLink", "Enabled", new Boolean(!bDoTemplateEnable));

    if (bDoTemplateEnable == true)
        sStorePath = (String) CurUNODialog.getPropertyOfDialogControl("txtSavePath_1", "Text");
    else
        sStorePath = (String) CurUNODialog.getPropertyOfDialogControl("txtSavePath_2", "Text");
    boolean bDoEnable = sStorePath.compareTo("") != 0;
    CurUNODialog.assignPropertyToDialogControl("cmdGoOn", "Enabled", new Boolean(bDoEnable));
    }
    catch( Exception exception ){
        exception.printStackTrace(System.out);
    }}



    public void fillFifthStep(){
    try{
    CurUNODialog.insertRadioButton("optCreateReportTemplate", SOOPTSAVEASTEMPLATE, new ActionListenerImpl(),
                            new String[] {"Height", "HelpURL", "Label", "PositionX", "PositionY", "State", "Step", "TabIndex", "Width"},
                            new Object[] {new Integer(8), "HID:34370", sSaveAsTemplate, new Integer(6), new Integer(41), new Short((short) 1), new Integer(5), new Short((short) 40), new Integer(250)});

    CurUNODialog.insertRadioButton("optCreateDocument", SOOPTSAVEASDOCUMENT, new ActionListenerImpl(),
                            new String[] {"Height", "HelpURL", "Label", "PositionX", "PositionY", "State", "Step", "TabIndex", "Width"},
                            new Object[] {new Integer(10), "HID:34371", sSaveAsDocument, new Integer(6), new Integer(125), new Short((short) 0), new Integer(5), new Short((short) 41), new Integer(138)});

    insertSaveControls(55, 0, true, 42, 34372);

    CurUNODialog.insertRadioButton("optEditTemplate", SOOPTEDITTEMPLATE, new ActionListenerImpl(),
                            new String[] {"Height", "HelpURL", "Label", "PositionX", "PositionY", "State", "Step", "TabIndex", "Width"},
                            new Object[] {new Integer(10), "HID:34374", sEditTemplate, new Integer(16), new Integer(84), new Short((short) 1), new Integer(5), new Short((short) 44), new Integer(138)});

    CurUNODialog.insertRadioButton("optUseTemplate", SOOPTUSEDOCUMENT, new ActionListenerImpl(),
                            new String[] {"Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
                            new Object[] {new Integer(10), "HID:34375", sUseTemplate, new Integer(16), new Integer(95), new Integer(5), new Short((short) 45), new Integer(138)});

    CurUNODialog.insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblAutomaticLink",
                new String[] {"Height", "Label", "PositionX", "PositionY", "Step", "Width"},
                new Object[] {new Integer(8), sCreateLinkAutomatically, new Integer(16), new Integer(108), new Integer(5), new Integer(200)});

    insertSaveControls(140, 1, false, 46, 34376);

    chkTemplate = CurUNODialog.insertControlModel("com.sun.star.awt.UnoControlCheckBoxModel", "chkcreateLink",
                new String[] {"Enabled", "Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
                new Object[] {new Boolean(false), new Integer(8), "HID:34378", sCreateLink, new Integer(16), new Integer(170), new Integer(5), new Short((short) 48), new Integer(130)});
    }
    catch( Exception exception ){
        exception.printStackTrace(System.out);
    }}


    public void fillFourthStep(XMultiServiceFactory xMSF){
    try{
        CurUNODialog.insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblTitle",
                            new String[] {"Height", "Label", "PositionX", "PositionY", "Step", "Width"},
                            new Object[] {new Integer(8), sReportTitle, new Integer(6), new Integer(40), new Integer(4), new Integer(200)});

        xTitleTextBox = CurUNODialog.insertTextField("txtTitle", SOTXTTITLE, new TextListenerImpl(),
                            new String[] {"Height", "HelpURL", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
                            new Object[] {new Integer(12), "HID:34362", new Integer(6), new Integer(50), new Integer(4), new Short((short)31), new Integer(258)});

        CurUNODialog.insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblContent",
                new String[] {"Height", "Label", "PositionX", "PositionY", "Step", "Width"},
                new Object[] {new Integer(8), slblDataStructure, new Integer(6), new Integer(70), new Integer(4), new Integer(125)});

        short iSelPos = (short) tools.FieldInList(CurReportPaths.ContentFiles[0], CurReportPaths.ReportPath + "/cnt-default.stw");
        xContentListBox = CurUNODialog.insertListBox("lstContent", SOCONTENTLST, new ActionListenerImpl(), new ItemListenerImpl(),
                    new String[] {"Height", "HelpURL", "PositionX", "PositionY", "SelectedItems", "Step", "StringItemList", "TabIndex", "Width"},
                new Object[] {new Integer(63), "HID:34363", new Integer(6), new Integer(80), new short[] {iSelPos},  new Integer(4), CurReportPaths.ContentFiles[1], new Short((short)32), new Integer(125)});

        CurUNODialog.insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblLayout",
                new String[] {"Height", "Label", "PositionX", "PositionY", "Step", "Width"},
                new Object[] {new Integer(8), slblPageLayout, new Integer(140), new Integer(70), new Integer(4), new Integer(125)});

        short iSelLayoutPos = (short) tools.FieldInList(CurReportPaths.LayoutFiles[0], CurReportPaths.ReportPath + "/stl-default.stw");
        xLayoutListBox = CurUNODialog.insertListBox("lstLayout", SOLAYOUTLST, new ActionListenerImpl(), new ItemListenerImpl(),
                    new String[] {"Height", "HelpURL", "PositionX", "PositionY", "SelectedItems", "Step", "StringItemList", "TabIndex", "Width"},
                new Object[] {new Integer(63), "HID:34364", new Integer(140), new Integer(80), new short[] {iSelLayoutPos}, new Integer(4), CurReportPaths.LayoutFiles[1], new Short((short)33), new Integer(125)});

        CurUNODialog.insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblOrientation",
                            new String[] {"Height", "Label", "PositionX", "PositionY", "Step", "Width"},
                            new Object[] {new Integer(8), sOrientationHeader, new Integer(6), new Integer(149), new Integer(4), new Integer(74)});

        CurUNODialog.insertRadioButton("optLandscape", SOOPTLANDSCAPE, new ActionListenerImpl(),
                            new String[] {"Height", "HelpURL", "Label", "PositionX", "PositionY", "State", "Step", "TabIndex", "Width"},
                            new Object[] {new Integer(10), "HID:34365", sOrientHorizontal, new Integer(12), new Integer(160), new Short((short) 1), new Integer(4), new Short((short)34), new Integer(100)});

        CurUNODialog.insertRadioButton("optPortrait", SOOPTPORTRAIT, new ActionListenerImpl(),
                            new String[] {"Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
                            new Object[] {new Integer(10), "HID:34366", sOrientVertical, new Integer(12), new Integer(173), new Integer(4), new Short((short)35), new Integer(100)});
            CurUNODialog.insertControlModel("com.sun.star.awt.UnoControlImageControlModel", "imgOrientation",
                            new String[] {"Border", "Height", "ImageURL", "PositionX", "PositionY", "ScaleImage", "Step", "Width"},
                            new Object[] {new Short("0"), new Integer(23), CurReportPaths.BitmapPath + "/landscape.gif", new Integer(80), new Integer(158), new Boolean(false), new Integer(4), new Integer(30)});
    }
    catch( Exception exception ){
        exception.printStackTrace(System.out);
    }}


    public boolean checkIfToupdateStep(){
    boolean bIsSame = CurDBMetaData.MainCommandName.equals(CurDBMetaData.OldMainCommandName) && (CurDBMetaData.MainCommandName != null);
    if (bIsSame == true)
        bIsSame = Arrays.equals(CurDBMetaData.FieldNames, CurDBMetaData.OldFieldNames) && (CurDBMetaData.FieldNames != null);
    if (bIsSame == true)
        bIsSame = Arrays.equals(CurDBMetaData.GroupFieldNames, CurDBMetaData.OldGroupFieldNames) && (CurDBMetaData.GroupFieldNames != null);
    if (bIsSame == true)
       bIsSame = Arrays.equals(CurDBMetaData.SortFieldNames, CurDBMetaData.OldSortFieldNames) && (CurDBMetaData.SortFieldNames != null);
    return !bIsSame;
    }


    public void updateThirdStep(){
    try{
    XWindow xWindow = (XWindow) UnoRuntime.queryInterface(XWindow.class, xSortListBox[0]);
    xWindow.setFocus();
    if (checkIfToupdateStep() == true){
        CurDBMetaData.OldGroupFieldNames = CurDBMetaData.GroupFieldNames;
        String[] GroupFieldNames = new String[GroupFieldVector.size()];
        GroupFieldVector.copyInto(GroupFieldNames);
        CurDBMetaData.GroupFieldNames = GroupFieldNames;
        int FieldCount = CurDBMetaData.FieldNames.length;
        int SortFieldCount = FieldCount + 1 - CurDBMetaData.GroupFieldNames.length;
        String SortFieldNames[] = new String[SortFieldCount];
        SortFieldNames[0] = sNoSorting;
        String CurFieldName;
        int a = 1;
        for (int i = 0; i < FieldCount;i++){
        CurFieldName = CurDBMetaData.FieldNames[i];
        if (tools.FieldInList(CurDBMetaData.GroupFieldNames, CurFieldName) == -1){
            SortFieldNames[a] = CurFieldName;
            a +=1;
        }
        }
        short[] SelList;
        for (int i = 0; i<4; i++){
        if (i == 0)
            // Todo: As soon as Vcl is able to recognize the selected Item set the following line to '0' (Bug #91459 of MT)
            SelList = new short[] {(short) 1};
        else{
            SelList = new short[] {(short) 0};
        }
        if (i > 1)
            toggleSortListBox(i, (false));
        CurUNODialog.assignPropertyToDialogControl("lstSort" + new Integer(i+1).toString(), "StringItemList", SortFieldNames);
        CurUNODialog.assignPropertyToDialogControl("lstSort" + new Integer(i+1).toString(), "SelectedItems", SelList);
        }
        System.out.println(xSortListBox[0].getSelectedItemPos());
    }
    }
    catch(Exception exception){
        exception.printStackTrace(System.out);
    }}


    public void fillThirdStep(){
    try{
    boolean bDoEnable;
    String HIDString;
    int YPos = 40;
    int BaseHelpID = 34345;
    for (int i = 0; i<4; i++){
        bDoEnable = (i < 2);
        CurUNODialog.insertControlModel("com.sun.star.awt.UnoControlFixedLineModel", "lblSort" + new Integer(i+1),
                new String[] {"Enabled", "Height", "Label", "Orientation", "PositionX", "PositionY", "Step", "Width"},
                new Object[] {new Boolean(bDoEnable), new Integer(8), sSortHeader[i], new Integer(0), new Integer(12), new Integer(YPos), new Integer(3), new Integer(252)});

        HIDString = "HID:" + Integer.toString(BaseHelpID);
        xSortListBox[i] = CurUNODialog.insertListBox("lstSort" + new Integer(i+1).toString(), SOSORTLST[i], new ActionListenerImpl(), new ItemListenerImpl(),
                new String[] {"Dropdown", "Enabled", "Height", "HelpURL", "LineCount", "Name", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
                new Object[] {new Boolean(true), new Boolean(bDoEnable), new Integer(12), HIDString, new Short("7"), "lstSort" + new Integer(i+1),  new Integer(12), new Integer(YPos + 14), new Integer(3), new Short((short) (17+i*4)), new Integer(154)});

        HIDString = "HID:" + Integer.toString(BaseHelpID+1);
        CurUNODialog.insertRadioButton("optAscend" + Integer.toString(i+1), SOSORTASCENDOPT[i], new ActionListenerImpl(),
                new String[] {"Enabled", "Height", "HelpURL", "Label", "PositionX", "PositionY", "State", "Step", "TabIndex", "Tag", "Width"},
                new Object[] {new Boolean(bDoEnable), new Integer(10), HIDString,  sSortAscend[i], new Integer(186), new Integer(YPos+10), new Short((short) 1), new Integer(3), new Short((short) (18+i*4)), new String("ASC"), new Integer(65)});

        HIDString = "HID:" + Integer.toString(BaseHelpID+2);
        CurUNODialog.insertRadioButton("optDescend" + Integer.toString(i+1), SOSORTDESCENDOPT[i], new ActionListenerImpl(),
                           new String[] {"Enabled", "Height", "HelpURL", "Label", "PositionX", "PositionY", "State", "Step", "TabIndex", "Tag", "Width"},
                           new Object[] {new Boolean(bDoEnable), new Integer(10), HIDString, sSortDescend[i], new Integer(186), new Integer(YPos+24), new Short((short) 0), new Integer(3), new Short((short) (19+i*4)), new String("DESC"), new Integer(65)});
        YPos = YPos + 36;
        BaseHelpID += 3;
    }
    }
    catch(Exception exception){
        exception.printStackTrace(System.out);
    }}


    public void updateSecondStep(){
    try{
    CurUNODialog.assignPropertyToDialogControl("cmdBack", "Enabled", new Boolean(true));
        CurDBMetaData.FieldNames = xSelFieldsListBox.getItems();
    XWindow xWindow = (XWindow) UnoRuntime.queryInterface(XWindow.class, xGroupListBox);
    xWindow.setFocus();

    if (checkIfToupdateStep() == true){
        CurDBMetaData.GroupFieldNames = null;
        CurDBMetaData.SortFieldNames = null;
        CurDBMetaData.RecordFieldNames = null;
        CurReportDocument.removeAllTextSections();
        CurReportDocument.removeAllTextTables();
        CurDBMetaData.OldFieldNames = CurDBMetaData.FieldNames;
        // Todo: Nur ausführen, when FieldNames anders als vorher-> dann muss auch Selektionslistbox leer gemacht werden.
        CurUNODialog.assignPropertyToDialogControl("lstGroup", "StringItemList", CurDBMetaData.FieldNames);
        xSelGroupListBox.removeItems((short) 0, xSelGroupListBox.getItemCount());
        GroupFieldVector = new java.util.Vector(CurDBMetaData.FieldNames.length);
    }
    }
    catch( Exception exception ){
        exception.printStackTrace(System.out);
    }}


    public void fillSecondStep(){
    try{
    CurUNODialog.insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblGroups",
            new String[] {"Height", "Label", "PositionX", "PositionY", "Step", "Width"},
            new Object[] {new Integer(8), sOrganizeFields, new Integer(6), new Integer(38), new Integer(2), new Integer(100)});
    xGroupListBox = CurUNODialog.insertListBox("lstGroup", SOGROUPLST, new ActionListenerImpl(), new ItemListenerImpl(),
                new String[] {"Height", "HelpURL", "MultiSelection", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
                        new Object[] {new Integer(125), "HID:34340", new Boolean(false), new Integer(6), new Integer(49), new Integer(2), new Short((short) 11), new Integer(110)});
    CurUNODialog.insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblSelGroups",
            new String[] {"Height", "Label", "PositionX", "PositionY", "Step", "Width"},
            new Object[] {new Integer(8), sGroupings, new Integer(154), new Integer(38), new Integer(2), new Integer(100)});
    xSelGroupListBox = CurUNODialog.insertListBox("lstSelGroup", SOSELGROUPLST, new ActionListenerImpl(), new ItemListenerImpl(),
                new String[] {"Height", "HelpURL", "MultiSelection", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
                        new Object[] {new Integer(125), "HID:34343", new Boolean(true), new Integer(154), new Integer(49), new Integer(2), new Short((short) 12), new Integer(110)});
    CurUNODialog.insertButton("cmdGroupOut", SOCMDGROUPOUT, new ActionListenerImpl(),
            new String[] {"Enabled", "Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
            new Object[] {new Boolean(false), new Integer(14), "HID:34341", "->", new Integer(122), new Integer(93), new Integer(2), new Short((short) 13), new Integer(25)});
    CurUNODialog.insertButton("cmdGroupIn", SOCMDGROUPIN, new ActionListenerImpl(),
            new String[] {"Enabled", "Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
            new Object[] {new Boolean(false), new Integer(14), "HID:34342", "<-", new Integer(122), new Integer(115), new Integer(2), new Short((short) 14), new Integer(25)});
    }
    catch( Exception exception ){
        exception.printStackTrace(System.out);
    }}


    public void fillFirstStep(XMultiServiceFactory xMSF, ReportDocument CurReportDocument, String[] DatabaseNames,  Object[] CurPropertyValue)

// Scenario 1. No parameters are given
//  MainWithDefault()

// Scenario 2: Only Datasourcename is given, but no connection and no Content
//  MainWithDefault("Bibliography")

// Scenario 3: a data source and a connection are given
//  oLocDBContext = CreateUnoService("com.sun.star.sdb.DatabaseContext")
//  oLocConnection = oLocDBContext.GetByName("Bibliography").GetConnection("","")
//  MainWithDefault("Bibliography", oLocConnection)

// Scenario 4: all parameters (data source name, connection, object type and object) are given
//  oLocDBContext = CreateUnoService("com.sun.star.sdb.DatabaseContext")
//  oLocConnection = oLocDBContext.GetByName("Bibliography").GetConnection("","")
//  MainWithDefault("Bibliography", oLocConnection, com.sun.star.sdb.CommandType.TABLE, "biblio")

    {
    try{
    com.sun.star.awt.FontDescriptor oFontDesc = new com.sun.star.awt.FontDescriptor();
    oFontDesc.Weight = com.sun.star.awt.FontWeight.BOLD;
    Object oHeaderLabel = CurUNODialog.insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblDialogHeader",
                            new String[] {"BackgroundColor", "FontDescriptor", "Height", "Label", "MultiLine", "PositionX", "PositionY", "Step", "Width"},
                            new Object[] {new Integer(16777215), oFontDesc, new Integer(30), WizardHeaderText[0], new Boolean(true), new Integer(50), new Integer(0), new Integer(0), new Integer(220)});
    CurUNODialog.insertButton("cmdCancel", SOCMDCANCEL, new ActionListenerImpl(),
                            new String[] {"Height", "HelpURL", "Label", "PositionX", "PositionY", "PushButtonType", "Step", "TabIndex", "Width"},
                            new Object[] {new Integer(14), "HID:34321", scmdCancel, new Integer(6), new Integer(190), new Short((short)com.sun.star.awt.PushButtonType.CANCEL_value), new Integer(0), new Short((short) 60), new Integer(53)});

    CurUNODialog.insertButton("cmdHelp", SOCMDHELP, new ActionListenerImpl(),
                            new String[] {"Height", "Label", "PositionX", "PositionY", "PushButtonType", "Step", "TabIndex", "Width"},
                            new Object[] {new Integer(14), scmdHelp, new Integer(63), new Integer(190), new Short((short)com.sun.star.awt.PushButtonType.HELP_value), new Integer(0), new Short((short) 61), new Integer(53)});

        CurUNODialog.insertButton("cmdBack", SOCMDBACK, new ActionListenerImpl(),
                            new String[] {"Enabled", "Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
                            new Object[] {new Boolean(false), new Integer(14), "HID:34322", scmdBack, new Integer(155), new Integer(190), new Integer(0), new Short((short) 62), new Integer(53)});

    CurUNODialog.insertControlModel("com.sun.star.awt.UnoControlFixedLineModel", "hlnCommandButtons",
                new String[] {"Height", "Label", "Orientation", "PositionX", "PositionY", "Step", "Width"},
                new Object[] {new Integer(2), "", new Integer(0), new Integer(6), new Integer(184), new Integer(0), new Integer(258)});

    CurUNODialog.insertButton("cmdMoveSelected", SOCMDMOVESEL, new ActionListenerImpl(),
                            new String[] {"Enabled", "Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
                            new Object[] {new Boolean(false), new Integer(14), "HID:34334", "->", new Integer(122), new Integer(84), new Integer(1), new Short((short) 4), new Integer(25)});

        CurUNODialog.insertButton("cmdMoveAll", SOCMDMOVEALL, new ActionListenerImpl(),
                            new String[] {"Enabled", "Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
                            new Object[] {new Boolean(false), new Integer(14), "HID:34335", "=>>", new Integer(122), new Integer(102), new Integer(1), new Short((short) 5), new Integer(25)});

    CurUNODialog.insertButton("cmdRemoveSelected", SOCMDREMOVESEL, new ActionListenerImpl(),
                            new String[] {"Enabled", "Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
                            new Object[] {new Boolean(false), new Integer(14), "HID:34336", "<-", new Integer(122), new Integer(120), new Integer(1), new Short((short) 6), new Integer(25)});

        CurUNODialog.insertButton("cmdRemoveAll", SOCMDREMOVEALL, new ActionListenerImpl(),
                            new String[] {"Enabled", "Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
                            new Object[] {new Boolean(false), new Integer(14), "HID:34337", "<<=", new Integer(122), new Integer(138), new Integer(1), new Short((short) 7), new Integer(25)});

        CurUNODialog.insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblDatabases",
                            new String[] {"Height", "Label", "PositionX", "PositionY", "Step", "Width"},
                            new Object[] {new Integer(8), slblDatabases, new Integer(6), new Integer(39), new Integer(1), new Integer(74)});

    CurUNODialog.insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblTables",
                            new String[] {"Enabled", "Height", "Label", "PositionX", "PositionY", "Step", "Width"},
                            new Object[] {new Boolean(false), new Integer(8), slblTables, new Integer(122), new Integer(39), new Integer(1), new Integer(72)});

        CurUNODialog.insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblFields",
                            new String[] {"Enabled", "Height", "Label", "PositionX", "PositionY", "Step", "Width"},
                            new Object[] {new Boolean(false), new Integer(8), slblFields, new Integer(6), new Integer(69), new Integer(1), new Integer(109)});

        CurUNODialog.insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblSelFields",
                            new String[] {"Enabled", "Height", "Label", "PositionX", "PositionY", "Step", "Width"},
                            new Object[] {new Boolean(false), new Integer(8), slblSelFields, new Integer(154), new Integer(69), new Integer(1), new Integer(110)});

        CurUNODialog.insertControlModel("com.sun.star.awt.UnoControlImageControlModel", "imgTheme",
                            new String[] {"BackgroundColor", "Border", "Height", "ImageURL", "PositionX", "PositionY", "ScaleImage", "Step", "Width"},
                            new Object[] {new Integer(16777215), new Short("0"), new Integer(30), CurReportPaths.BitmapPath + "/report.bmp", new Integer(0), new Integer(0), new Boolean(false), new Integer(0), new Integer(50)});

        xDBListBox = CurUNODialog.insertListBox("lstDatabases", SODBLST, new ActionListenerImpl(), new ItemListenerImpl(),
                            new String[] {"Dropdown", "Height", "HelpURL", "LineCount", "Name", "PositionX", "PositionY", "Step", "StringItemList", "TabIndex", "Width"},
                            new Object[] {new Boolean(true), new Integer(12), "HID:34330", new Short("7"), "lstDatabases", new Integer(6), new Integer(49), new Integer(1), DatabaseNames, new Short((short) 1), new Integer(110)});
    xTableListBox = CurUNODialog.insertListBox("lstTables", SOTBLLST, new ActionListenerImpl(), new ItemListenerImpl(),
                            new String[] {"Dropdown", "Enabled", "Height", "HelpURL", "LineCount", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
                            new Object[] {new Boolean(true), new Boolean(false), new Integer(12), "HID:34331", new Short("7"), new Integer(122), new Integer(49), new Integer(1), new Short((short) 2), new Integer(110)});
    xFieldsListBox = CurUNODialog.insertListBox("lstFields", SOFLDSLST, new ActionListenerImpl(), new ItemListenerImpl(),
                new String[] {"Enabled", "Height", "HelpURL", "MultiSelection", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
                            new Object[] {new Boolean(false), new Integer(77), "HID:34332", new Boolean(true), new Integer(6), new Integer(79), new Integer(1), new Short((short) 3), new Integer(110)});
    xSelFieldsListBox = CurUNODialog.insertListBox("lstSelFields", SOSELFLDSLST, new ActionListenerImpl(), new ItemListenerImpl(),
                            new String[] {"Enabled", "Height", "HelpURL", "MultiSelection", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
                            new Object[] {new Boolean(false), new Integer(77), "HID:34333", new Boolean(true), new Integer(154), new Integer(79), new Integer(1), new Short((short) 8), new Integer(110)});
    CurUNODialog.insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblBinaryFields",
                            new String[] {"Height", "Label", "PositionX", "PositionY", "Step", "Width"},
                            new Object[] {new Integer(16), sShowBinaryFields, new Integer(6), new Integer(162), new Integer(1), new Integer(210)});

    XWindow xWindow = (XWindow) UnoRuntime.queryInterface(XWindow.class, xDBListBox);
    CurDBMetaData.DataSourceName = (String) tools.getPropertyValueFromAny(CurPropertyValue, "DataSourceName");
    if (CurDBMetaData.DataSourceName != null){
        if (CurDBMetaData.DataSourceName.equals("") == false){
        short iPos = (short) tools.FieldInList(DatabaseNames, CurDBMetaData.DataSourceName);
        if (iPos > -1){
            Short[] SelList = new Short[] {new Short(iPos)};
            CurUNODialog.assignPropertyToDialogControl("lstDatabases", "SelectedItems", SelList);
            xWindow = (XWindow) UnoRuntime.queryInterface(XWindow.class, xTableListBox);
        }
        }
    }
    CurDBMetaData.DBConnection = (com.sun.star.sdbc.XConnection) tools.getPropertyValueFromAny(CurPropertyValue, "Connection");
    if (CurDBMetaData.DBConnection != null){
        if (CurDBMetaData.DataSourceName == null)
        System.out.println(" Overgiven DataSourcename invalid");
        String[] ContentList = CurDBMetaData.getDBMetaData(CurReportDocument);
            CurUNODialog.assignPropertyToDialogControl("lstTables", "StringItemList", ContentList);
            CurUNODialog.assignPropertyToDialogControl("lstTables", "Enabled", new Boolean(true));
            CurUNODialog.assignPropertyToDialogControl("lblTables", "Enabled", new Boolean(true));
            if (ContentList != null){
        iCommandTypes = DBMetaData.createCommandTypeList();
            }
    }

    if (CurDBMetaData.Command != null){
        CurDBMetaData.CommandType = ((Integer) tools.getPropertyValueFromAny(CurPropertyValue, "CommandType")).intValue();
        // Todo: find whether it is query or Table in case there is a table and a query with the same name
        // Note: for some reasons I cannot access the Listbox directly to select the item so I have to go the way over the model.
        short iPos = (short) tools.FieldInList(DatabaseNames, CurDBMetaData.DataSourceName);
        if (iPos > -1){
        Short[] SelList = new Short[] {new Short((short) 0)};
        CurUNODialog.assignPropertyToDialogControl("lstTables", "SelectedItems", SelList);
        fillUpFieldsListbox(CurDBMetaData, true);
        xWindow = (XWindow) UnoRuntime.queryInterface(XWindow.class, xFieldsListBox);
        }
    }
    xWindow.setFocus();

    // DefaultButton has to be inserted after the focus has been set
    CurUNODialog.insertButton("cmdGoOn", SOCMDGOON, new ActionListenerImpl(),
                            new String[] {"DefaultButton", "Enabled", "Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
                            new Object[] {new Boolean(true), new Boolean(false), new Integer(14), "HID:34323", scmdGoOn, new Integer(211), new Integer(190), new Integer(0), new Short((short) 63), new Integer(53)});

    }
    catch(Exception exception){
        exception.printStackTrace(System.out);
    }}


    public static void main (String args[]) {
        String ConnectStr = "uno:socket,host=localhost,port=8100;urp,negotiate=0,forcesynchronous=1;StarOffice.NamingService";
    try {
            xGlobalMSF = tools.connect(ConnectStr);
        if(xGlobalMSF != null)  System.out.println("Connected to "+ ConnectStr);
        ReportWizard CurReportWizard = new ReportWizard();
        CurReportWizard.startReportWizard(xGlobalMSF, null);
    }
        catch(Exception exception) {
            exception.printStackTrace(System.out);
        }
    }


    public void startReportWizard(XMultiServiceFactory xMSF, Object[] CurPropertyValue){
    try{
    xGlobalMSF = xMSF;
    xDesktop = tools.getDesktop(xMSF);
    XFramesSupplier xFrameSuppl = (XFramesSupplier) UnoRuntime.queryInterface(XFramesSupplier.class, xDesktop);
    CurReportDocument =  new ReportDocument(xMSF, true, false);
    CurDBMetaData = new DBMetaData();
    CurReportPaths = new ReportPaths(xMSF);
    CurReportDocument.changePageOrientation(CurReportPaths.BitmapPath, null, null, true);
    getReportResources(xMSF, false);
    DatabaseNames = CurDBMetaData.getDatabaseNames(CurReportDocument);
    if (DatabaseNames.length > 0){
        CurReportDocument.ProgressBar.setValue(20);
        CurReportDocument.loadStyleTemplates(CurReportPaths.ReportPath + "/stl-default.stw", "LoadPageStyles");
        WidthList = DBMetaData.InitializeWidthList();
        CurUNODialog = new UNODialogs(xMSF, new String[] {"Height", "HelpURL", "Step", "Title", "Width"},
                            new Object[] {new Integer(210), "HID:34320", new Integer(1), WizardTitle[0], new Integer(270)});
        CurReportDocument.ProgressBar.setValue(35);
        fillFirstStep(xMSF, CurReportDocument, DatabaseNames, CurPropertyValue);
        CurReportDocument.ProgressBar.setValue(50);
        fillSecondStep();
        CurReportDocument.ProgressBar.setValue(65);
        fillThirdStep();
        CurReportDocument.ProgressBar.setValue(80);
        fillFourthStep(xMSF);
        CurReportDocument.ProgressBar.setValue(95);
        fillFifthStep();
        CurReportDocument.ProgressBar.setValue(100);
        bCloseDocument = true;
        CurReportDocument.ProgressBar.end();
        short RetValue = CurUNODialog.executeDialog(xMSF, CurReportDocument.Frame.getComponentWindow().getPosSize());
        switch (RetValue){
        case 0:     // via Cancelbutton or via sourceCode with "endExecute"
            if (bCloseDocument == true){
            CurUNODialog.xComponent.dispose();
            CurReportDocument.Component.dispose();
            return;
            }
            if ((buseTemplate == true) || (bcreateTemplate == false)){
            if (CurReportDocument.checkReportLayoutMode(CurDBMetaData.GroupFieldNames)){
                Dataimport CurDataimport = new Dataimport();
                CurUNOProgressDialog = CurDataimport.showProgressDisplay(xMSF, CurReportDocument, false);  // CurReportDocument.Frame.getComponentWindow().getPosSize().Width);
                if (CurDBMetaData.executeCommand(xMSF, CurReportDocument.Frame, sMsgQueryCreationImpossible + (char) 13 + sMsgEndAutopilot)){;
                CurDataimport.insertDatabaseDatatoReportDocument(xMSF, CurDBMetaData, CurReportDocument, CurUNOProgressDialog);
                CurUNOProgressDialog.xComponent.dispose();
                }
                else{
                CurUNOProgressDialog.xComponent.dispose();
                return;
                }
            }
            if (bcreateTemplate == false){
                boolean bDocisStored = tools.storeDocument(xMSF, CurReportDocument.Component, StorePath, "swriter: StarOffice XML (Writer)",
                                    false, sMsgSavingImpossible + (char)13 + sMsgLinkCreationImpossible);
                if (bcreateLink && bDocisStored)
                CurDBMetaData.createDBLink(CurDBMetaData.DataSource, StorePath);
            }
            }
            return;
        case 1:
            break;
        }
        CurUNODialog.xComponent.dispose();
    }
    else{
        int iMsg = UNODialogs.showMessageBox(xMSF, CurReportDocument.Frame, "ErrorBox", com.sun.star.awt.VclWindowPeerAttribute.OK, sMsgNoDatabaseAvailable);
        CurReportDocument.Component.dispose();
    }
    }
    catch(java.lang.Exception jexception ){
    jexception.printStackTrace(System.out);
    }}



    public static void getReportResources(XMultiServiceFactory xMSF, boolean bgetProgressResourcesOnly){
        final int RID_COMMON = 1000;
        final int RID_FORM = 2200;
        final int RID_REPORT = 2400;
        XInvocation xResInvoke = tools.initResources(xMSF, "ReportWizard","dbw");
    sMsgWizardName = tools.getResText(xResInvoke, RID_REPORT);
    if (bgetProgressResourcesOnly == false){
        scmdReady = tools.getResText(xResInvoke, RID_COMMON + 0);
        scmdCancel = tools.getResText(xResInvoke, RID_COMMON + 1);
        scmdBack = tools.getResText(xResInvoke, RID_COMMON + 2);
        scmdGoOn = tools.getResText(xResInvoke, RID_COMMON + 3);

        sMsgErrorOccured = tools.getResText(xResInvoke, RID_COMMON + 6);
        sMsgNoTableInDatabase = tools.getResText(xResInvoke, RID_COMMON + 9);
        sMsgCommandCouldNotbeOpened = tools.getResText(xResInvoke, RID_COMMON + 13);
        sMsgNoConnection = tools.getResText(xResInvoke, RID_COMMON + 14);
        scmdHelp = tools.getResText(xResInvoke, RID_COMMON + 20);
        sMsgNoDatabaseAvailable = tools.getResText(xResInvoke, RID_REPORT + 2);

        slblTables = tools.getResText(xResInvoke, RID_FORM + 6);
        slblDatabases = tools.getResText(xResInvoke, RID_FORM + 11);
        slblFields = tools.getResText(xResInvoke, RID_FORM + 12);
        slblSelFields = tools.getResText(xResInvoke, RID_REPORT + 9);
        slblDataStructure = tools.getResText(xResInvoke, RID_REPORT + 15);
        slblPageLayout = tools.getResText(xResInvoke, RID_REPORT + 16);
        sOrganizeFields = tools.getResText(xResInvoke, RID_REPORT + 19);
        sSortHeader[0] = tools.getResText(xResInvoke, RID_REPORT + 20);
        sSortHeader[1] = tools.getResText(xResInvoke, RID_REPORT + 21);
        sSortHeader[2] = tools.getResText(xResInvoke, RID_REPORT + 51);
        sSortHeader[3] = tools.getResText(xResInvoke, RID_REPORT + 52);

        sNoSorting = tools.getResText(xResInvoke, RID_REPORT + 8);
        sOrientationHeader =  tools.getResText(xResInvoke, RID_REPORT + 22);
        sOrientVertical = tools.getResText(xResInvoke, RID_REPORT + 23);
        sOrientHorizontal = tools.getResText(xResInvoke, RID_REPORT + 24);
        sReportTitle = tools.getResText(xResInvoke, RID_REPORT + 33);
        sSortAscend[0] = tools.getResText(xResInvoke, RID_REPORT + 36);
        sSortAscend[1] = tools.getResText(xResInvoke, RID_REPORT + 53);
        sSortAscend[2] = tools.getResText(xResInvoke, RID_REPORT + 54);
        sSortAscend[3] = tools.getResText(xResInvoke, RID_REPORT + 55);

        sSortDescend[0] = tools.getResText(xResInvoke, RID_REPORT + 37);
        sSortDescend[1] = tools.getResText(xResInvoke, RID_REPORT + 56);
        sSortDescend[2] = tools.getResText(xResInvoke, RID_REPORT + 57);
        sSortDescend[3] = tools.getResText(xResInvoke, RID_REPORT + 58);

        WizardHeaderText[0] = (char) 13 +  " " + tools.getResText(xResInvoke, RID_REPORT + 28);
        WizardHeaderText[1] = (char) 13 +  " " + tools.getResText(xResInvoke, RID_REPORT + 29);
        WizardHeaderText[2] = (char) 13 +  " " + tools.getResText(xResInvoke, RID_REPORT + 30);
        WizardHeaderText[3] = (char) 13 +  " " + tools.getResText(xResInvoke, RID_REPORT + 31);
        WizardHeaderText[4] = (char) 13 +  " " + tools.getResText(xResInvoke, RID_REPORT + 32);
        WizardTitle = new String[5];
        WizardTitle[0] = sMsgWizardName + " - " + tools.getResText(xResInvoke, RID_FORM + 45);
        WizardTitle[1] = sMsgWizardName + " - " + tools.getResText(xResInvoke, RID_REPORT + 11);
        WizardTitle[2] = sMsgWizardName + " - " + tools.getResText(xResInvoke, RID_REPORT + 12);
        WizardTitle[3] = sMsgWizardName + " - " + tools.getResText(xResInvoke, RID_REPORT + 13);
        WizardTitle[4] = sMsgWizardName + " - " + tools.getResText(xResInvoke, RID_REPORT + 14);
        sSaveAsTemplate = tools.getResText(xResInvoke, RID_REPORT + 40);
        sUseTemplate = tools.getResText(xResInvoke, RID_REPORT + 41);
            sEditTemplate = tools.getResText(xResInvoke, RID_REPORT + 42);
        sSaveAsDocument = tools.getResText(xResInvoke, RID_REPORT + 43);
        sSaveAs = tools.getResText(xResInvoke, RID_REPORT + 44);
        sCreateLink = tools.getResText(xResInvoke, RID_REPORT + 45);
        sGroupings = tools.getResText(xResInvoke, RID_REPORT + 50);
            sMsgSavingImpossible = tools.getResText(xResInvoke, RID_COMMON + 30);
// todo: the following message also has to show up when saving failed: sLinkCreationImpossible
        sMsgLinkCreationImpossible = tools.getResText(xResInvoke, RID_COMMON + 31);
        sCreateLinkAutomatically = tools.getResText(xResInvoke, RID_COMMON + 32);
        sShowBinaryFields = tools.getResText(xResInvoke, RID_REPORT + 60);
    }
    sMsgEndAutopilot = tools.getResText(xResInvoke, RID_COMMON + 33);
        sProgressDBConnection = tools.getResText(xResInvoke, RID_COMMON + 34);
    sMsgConnectionImpossible = tools.getResText(xResInvoke, RID_COMMON + 35);
    sMsgFilePathInvalid = tools.getResText(xResInvoke, RID_COMMON + 36);

    sStop = tools.getResText(xResInvoke, RID_COMMON + 21);
    sMsgTableNotExisting = tools.getResText(xResInvoke, RID_REPORT + 61);
    sProgressTitle = tools.getResText(xResInvoke, RID_REPORT + 62);
    sProgressBaseCurRecord = tools.getResText(xResInvoke, RID_REPORT + 63);
    sReportFormNotExisting = tools.getResText(xResInvoke, RID_REPORT + 64);
    sMsgQueryCreationImpossible =  tools.getResText(xResInvoke, RID_REPORT + 65);
    sMsgHiddenControlMissing = tools.getResText(xResInvoke, RID_REPORT + 66);
    sProgressDataImport = tools.getResText(xResInvoke, RID_REPORT + 67);
    sMsgNoConnection = tools.getResText(xResInvoke, RID_COMMON + 14);
    }

    class ReportPaths{
    public String TemplatePath;
    public String BitmapPath;
    public String ReportPath;
    public String[][] LayoutFiles;
    public String[][] ContentFiles;
    public String UserTemplatePath;
    public String WorkPath;
    public ReportPaths(XMultiServiceFactory xMSF){
        TemplatePath = tools.getOfficePath(xMSF, "Template","share");
        BitmapPath = TemplatePath + "/wizard/bitmap";
        ReportPath = TemplatePath +  "/wizard/report";
        ContentFiles = tools.getFolderTitles(xMSF, "cnt", ReportPath);
        LayoutFiles = tools.getFolderTitles(xMSF,"stl", ReportPath);
    }
    }
}
