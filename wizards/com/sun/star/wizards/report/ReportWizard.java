 /*************************************************************************
 *
 *  $RCSfile: ReportWizard.java,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: bc $ $Date: 2002-06-16 13:18:01 $
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
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.container.XEnumeration;
import com.sun.star.container.XNameContainer;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XMultiPropertySet;

import com.sun.star.comp.servicemanager.ServiceManager;

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
import com.sun.star.awt.XActionListener;
import com.sun.star.awt.XItemListener;
import com.sun.star.awt.ItemEvent;
import com.sun.star.awt.ActionEvent;
import com.sun.star.awt.XMouseListener;
import com.sun.star.awt.XTextComponent;
import com.sun.star.awt.MouseEvent;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XFramesSupplier;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XModel;
import com.sun.star.util.XURLTransformer;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;

import java.io.*;
import java.util.*;


/**
 *
 * @author  bc93774
 * @version
 */
public class ReportWizard {
    static long iStart;
    static XMultiServiceFactory xGlobalMSF;
    static Object xListBoxModel;
    static Object oDialogModel;
    static XNameAccess xDlgNameAccess;
    static Hashtable ControlList;
    static XNameContainer xDlgNames;
    static XStatement xStatement;
    static XDatabaseMetaData xDBMetaData;
    static XResultSet xResultSet;
    static XTextTable xTextTable;
    static XDialog xDialog;
    static DBMetaData.CommandMetaData CurDBMetaData;
    static boolean[] bModify = new boolean[] {true, true, true, true, true};
    static final int SOCMDCANCEL = 1;
    static final int SOCMDHELP = 2;
    static final int SOCMDBACK = 3;
    static final int SOCMDGOON = 4;
    static final int SOCMDMOVESEL = 5;
    static final int SOCMDMOVEALL = 6;
    static final int SOCMDREMOVESEL = 7;
    static final int SOCMDREMOVEALL = 8;
    static final int SOFLDSLST =  9;
    static final int SODBLST =  10;
    static final int SOTBLLST =  11;
    static final int SOSELFLDSLST =  12;
    static final int SOGROUPLST = 13;

    static final int SOFIRSTSORTLST = 14;
    static final int SOSECSORTLST = 15;
    static final int SOTHIRDSORTLST = 16;
    static final int SOFOURTHSORTLST = 17;
    static final int[] SOSORTLST = new int[]{SOFIRSTSORTLST, SOSECSORTLST, SOTHIRDSORTLST, SOFOURTHSORTLST};

    static final int SOFIRSTSORTASCOPT = 18;
    static final int SOSECSORTASCOPT = 19;
    static final int SOTHIRDSORTASCOPT = 20;
    static final int SOFOURTHSORTASCOPT = 21;

    static final int SOFIRSTSORTDESCOPT = 22;
    static final int SOSECSORTDESCOPT = 23;
    static final int SOTHIRDSORTDESCOPT = 24;
    static final int SOFOURTHSORTDESCOPT = 25;

    static final int[] SOSORTASCENDOPT = new int[] {SOFIRSTSORTASCOPT, SOSECSORTASCOPT, SOTHIRDSORTASCOPT, SOFOURTHSORTASCOPT};
    static final int[] SOSORTDESCENDOPT = new int[] {SOFIRSTSORTDESCOPT, SOSECSORTDESCOPT, SOTHIRDSORTDESCOPT, SOFOURTHSORTDESCOPT};

    static final int SOCMDGROUPIN = 26;
    static final int SOCMDGROUPOUT = 27;
    static final int SOTXTTITLE = 28;
    static final int SOCONTENTLST = 29;

    static final int SOOPTLANDSCAPE = 30;
    static final int SOOPTPORTRAIT = 31;
    static final int SOLAYOUTLST = 32;
    static final int SOSELGROUPLST = 33;
//    static final int SOOPTSAVEASTEMPLATE = 34;
//    static final int SOOPTSAVEASDOCUMENT = 35;

    static final int SOOPTSAVEASTEMPLATE = 40;
    static final int SOOPTEDITTEMPLATE = 41;
    static final int SOOPTUSEDOCUMENT = 42;
    static final int SOOPTSAVEASDOCUMENT = 43;

    static final int SOCMDFIRSTPATHSELECT = 44;
    static final int SOCMDSECPATHSELECT = 45;
    static final int SOTXTFIRSTSAVEPATH = 46;
    static final int SOTXTSECSAVEPATH = 47;

    static final int[] SOCMDSELECTPATH = new int[] {SOCMDFIRSTPATHSELECT, SOCMDSECPATHSELECT};
    static final int[] SOTXTSAVEPATH = new int[] {SOTXTFIRSTSAVEPATH, SOTXTSECSAVEPATH};

    static int MaxSortIndex = -1;
    static String[] sDatabaseList;
    static String[] TableNames;
    static String[] QueryNames;
    static String[][] LayoutFiles = new String[2][];
    static String[][] ContentFiles = new String[2][];
    static String[] sSortHeader = new String[4];
    static String[]sSortAscend = new String[4];
    static String[]sSortDescend = new String[4];
    static XTextComponent xTitleTextBox;
    static XTextComponent[] xSaveTextBox = new XTextComponent[2];

    static XListBox xDBListBox;
    static XListBox xTableListBox;
    static XListBox xFieldsListBox;
    static XListBox xSelFieldsListBox;
    static XListBox xContentListBox;
    static XListBox xSelGroupListBox;
    static Object chkTemplate;
    static XListBox[] xSortListBox = new XListBox[4];
    static XListBox xGroupListBox;
    static XListBox xLayoutListBox;
    static Object lblHeaderText;
    static XTablesSupplier xDBTables;
    static XNameAccess xTableNames;
    static XNameAccess xQueryNames;
    static String TableName;
    static Object oDBTable;
    static Object objectDialog;
    static int[] iCommandTypes;
    static int[][] WidthList;
    static boolean bEnableBinaryOptionGroup;
    static String[] OriginalList = new String[]{""};
    static XNameAccess xColumns;
    static XMultiServiceFactory  xMSFDialogModel;
    static XControlContainer xDialogContainer;
    static XControl xControl;
    static XDesktop xDesktop;
    static ReportDocument.RepWizardDocument CurReportDocument;
    static java.util.Vector GroupFieldVector;
    static String CurGroupName;
    static String TemplatePath;
    static boolean bGroupByIsSupported;
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
    static String sGroupings;
    static String[] WizardHeaderText = new String[5];
    static String[] WizardTitle = new String[5];
    static String sWriterFilterName;
    static String StorePath = "";

    static boolean bCloseDocument;

    public ReportWizard() {
    }

    private static int getControlKey(Object EventObject){
    int iKey;
        XControl xControl = (XControl) UnoRuntime.queryInterface(XControl.class, EventObject);
        XControlModel xControlModel = xControl.getModel();
        XPropertySet xPSet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xControlModel);
        try{
            String sName = (String) xPSet.getPropertyValue("Name");
            Integer KeyObject = (Integer) ControlList.get(sName);
            iKey = KeyObject.intValue();
        }
        catch( Exception exception )
        {
          exception.printStackTrace(System.out);
              iKey = 2000;
        }
        return iKey;
    }


    private static void getSelectedDBMetaData(XMultiServiceFactory xMSF)
        throws com.sun.star.container.NoSuchElementException, com.sun.star.beans.UnknownPropertyException,
               com.sun.star.lang.IllegalArgumentException, com.sun.star.lang.WrappedTargetException{
        try{
            short DBIndex = xDBListBox.getSelectedItemPos();
            String sDBName = sDatabaseList[DBIndex];
            if (DBMetaData.getConnection(xMSF, CurDBMetaData, CurReportDocument, sDBName) == true){
        CurDBMetaData.DataSourceName = sDBName;
                if (DBMetaData.getDBMetaData(CurReportDocument) == true){
                    UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "lstTables", "Enabled", new Boolean(true));
                    UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "lblTables", "Enabled", new Boolean(true));
                    iCommandTypes = DBMetaData.createCommandTypeList();
                    UNODialogs.emptyFieldsListBoxes(xDlgNameAccess, xFieldsListBox, xSelFieldsListBox);
                }
            }
        }
        catch(Exception exception)
        {
            exception.printStackTrace(System.out);
        }
    }


    private static void FillUpFieldsListbox(DBMetaData.CommandMetaData CurDBMetaData, boolean bgetCommandType){
        try{
    if (xTableListBox.getSelectedItems().length > 0) {
           int CurCommandType;
           int SelIndex = xTableListBox.getSelectedItemPos();
           if (bgetCommandType == true){
               CurCommandType = iCommandTypes[SelIndex];
               if (CurCommandType == com.sun.star.sdb.CommandType.QUERY){
                    int QueryIndex = SelIndex - TableNames.length;
                    TableName = QueryNames[QueryIndex];
                    oDBTable = xQueryNames.getByName(TableName);
            CurDBMetaData.Command = (String) tools.getUNOPropertyValue(oDBTable, "Command");
           }
        else{
                    TableName = TableNames[SelIndex];
                    oDBTable = xTableNames.getByName(TableName);
                }
        CurDBMetaData.CommandType = CurCommandType;
       }
           XColumnsSupplier xTableCols = (XColumnsSupplier) UnoRuntime.queryInterface(XColumnsSupplier.class, oDBTable);
           xColumns = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, xTableCols.getColumns());
           if (DBMetaData.getSpecificFieldNames(CurDBMetaData) > 0)
               UNODialogs.toggleListboxControls(xDlgNameAccess, xFieldsListBox, xSelFieldsListBox, new Boolean(true));
           else
               UNODialogs.emptyFieldsListBoxes(xDlgNameAccess, xFieldsListBox, xSelFieldsListBox);
        }
        else
            UNODialogs.emptyFieldsListBoxes(xDlgNameAccess, xFieldsListBox, xSelFieldsListBox);
        }
        catch(Exception exception){
            exception.printStackTrace(System.out);
        }
    }


    static class TextListenerImpl implements com.sun.star.awt.XTextListener{

    public void textChanged(com.sun.star.awt.TextEvent EventObject){
    int iPos;
    try{
            int iKey  =  getControlKey(EventObject.Source);
            switch (iKey) {
//      case SOTXTFIRSTSAVEPATH:
//          UNODialogs.toggleGoOnButton(xSaveTextBox[0], xDlgNameAccess, CurReportDocument.xSimpleFileAccess, "cmdGoOn");
//          break;
//      case SOTXTSECSAVEPATH:
//          UNODialogs.toggleGoOnButton(xSaveTextBox[1], xDlgNameAccess, CurReportDocument.xSimpleFileAccess, "cmdGoOn");
//          break;
        case SOTXTTITLE:
            String TitleName = xTitleTextBox.getText();
            ReportDocument.updateReportTitle(CurReportDocument, xTitleTextBox);
            break;
        }
        }
        catch(Exception exception){
            exception.printStackTrace(System.out);
    }}

    public void disposing(com.sun.star.lang.EventObject EventObject) {
        }
    }


   static class ItemListenerImpl implements com.sun.star.awt.XItemListener{

       public void itemStateChanged(ItemEvent EventObject ){
       int iPos;
    try{
            int iKey  =  getControlKey(EventObject.Source);
            switch (iKey) {
        case SODBLST:
            getSelectedDBMetaData(xGlobalMSF);
            break;

        case SOTBLLST:
            FillUpFieldsListbox(CurDBMetaData, true);
            break;

        case SOFLDSLST:
            UNODialogs.FormSetMoveRights(xDlgNameAccess, xFieldsListBox, xSelFieldsListBox, (short)-1, (short)-1);
            break;

        case SOSELFLDSLST:
            UNODialogs.FormSetMoveRights(xDlgNameAccess, xFieldsListBox, xSelFieldsListBox, (short)-1, (short)-1);
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
            UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "cmdGroupOut", "Enabled", new Boolean(bDoEnable));
            break;

        case SOSELGROUPLST:
            toggleRemoveGroupButton();
            break;

        case SOCONTENTLST:
            CurReportDocument.ReportTextDocument.lockControllers();
            iPos = xContentListBox.getSelectedItemPos();
            ReportDocument.loadSectionsfromTemplate(CurReportDocument, CurDBMetaData, ContentFiles[0][iPos]);
            ReportDocument.loadStyleTemplates(CurReportDocument.ReportTextDocument, ContentFiles[0][iPos], "LoadTextStyles");
            CurReportDocument.ReportTextDocument.unlockControllers();
            ReportDocument.selectFirstPage(CurReportDocument.ReportTextDocument);
            break;

        case SOLAYOUTLST:
            CurReportDocument.ReportTextDocument.lockControllers();
            iPos = xLayoutListBox.getSelectedItemPos();
            boolean bOldIsCurLandscape = ((Boolean) tools.getUNOPropertyValue(CurReportDocument.ReportPageStyle, "IsLandscape")).booleanValue();
            ReportDocument.loadStyleTemplates(CurReportDocument.ReportTextDocument, LayoutFiles[0][iPos], "LoadPageStyles");
            ReportDocument.changePageOrientation(xGlobalMSF, xDlgNameAccess, CurReportDocument, bOldIsCurLandscape);
            CurReportDocument.ReportTextDocument.unlockControllers();
            ReportDocument.selectFirstPage(CurReportDocument.ReportTextDocument);
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


    static class ActionListenerImpl implements com.sun.star.awt.XActionListener {

        public void disposing(EventObject eventObject) {
        }
        public void actionPerformed(ActionEvent actionEvent) {
            try{
                int iKey  =  getControlKey(actionEvent.Source);
                switch (iKey) {
                    case SOFLDSLST:
                       UNODialogs.MoveSelectedListBox(xDlgNameAccess, xFieldsListBox, xSelFieldsListBox); //, CurDBMetaData.AllFieldNames,  false);
                       break;

            case SOSELFLDSLST:
                        UNODialogs.MoveOrderedSelectedListBox(xDlgNameAccess, xFieldsListBox, xSelFieldsListBox, CurDBMetaData.AllFieldNames, false);
                        break;

                    case SOCMDMOVESEL:
                        UNODialogs.MoveSelectedListBox(xDlgNameAccess, xFieldsListBox, xSelFieldsListBox);
                        break;

                    case SOCMDMOVEALL:
                        UNODialogs.FormMoveAll(xDlgNameAccess, xFieldsListBox, xSelFieldsListBox, CurDBMetaData.AllFieldNames);
                        break;

                    case SOCMDREMOVESEL:
                        UNODialogs.MoveOrderedSelectedListBox(xDlgNameAccess, xFieldsListBox, xSelFieldsListBox, CurDBMetaData.AllFieldNames, false);
                        break;

                    case SOCMDREMOVEALL:
                        UNODialogs.MoveOrderedSelectedListBox(xDlgNameAccess, xFieldsListBox, xSelFieldsListBox, CurDBMetaData.AllFieldNames, true);
                        break;

                    case SOCMDGOON:
                        gotoNextStep(xGlobalMSF);
                        break;

                    case SOCMDBACK:
                        gotoPrevStep();
                        break;

                    case SOGROUPLST:
                       addGroupNametoDocument();
                       break;

                    case SOSELGROUPLST:
                       removeGroupName();
                       break;

                    case SOCMDGROUPOUT:
                       addGroupNametoDocument();
                       break;

                    case SOCMDGROUPIN:
            removeGroupName();
                       break;

                    case SOOPTLANDSCAPE:
                        ReportDocument.changePageOrientation(xGlobalMSF, xDlgNameAccess, CurReportDocument, true);
                        break;

                    case SOOPTPORTRAIT:
                        ReportDocument.changePageOrientation(xGlobalMSF, xDlgNameAccess, CurReportDocument, false);
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
            }
            catch( Exception exception ){
               exception.printStackTrace(System.out);
            }
        }
    }


    public static void gotoNextStep(XMultiServiceFactory xMSF){
    try{
    boolean bSetTitle = true;
        int PageCount = 5;
        int iPage = ((Integer) tools.getUNOPropertyValue(oDialogModel, "Step")).intValue();
        switch (iPage){
            case 1:
                updateSecondStep();
//      oFrame.dispose();
                break;
            case 2:
        updateThirdStep();
                break;
            case 3:
                setUpSortList();
        CurDBMetaData.RecordFieldNames = DBMetaData.setRecordFieldNames(CurDBMetaData);
                CurDBMetaData.ResultSet = DBMetaData.combineSelectStatement(CurDBMetaData.DBConnection, xDBMetaData, TableName, CurDBMetaData);
        ReportDocument.setupRecordSection(CurReportDocument, CurReportDocument.ReportFolderName + "/cnt-default.stw", CurDBMetaData);
        //TODO: A message box should pop up when a single sorting criteria has been selected more than once
        break;
        case 4:
        UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "cmdGoOn", "Enabled", new Boolean(false));
        UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "cmdGoOn", "Label", scmdReady);
        break;
        case 5:
        boolean bCreateTemplate = ((Short)  UNODialogs.getPropertyOfDialogControl(xDlgNameAccess, "optCreateReportTemplate", "State")).shortValue() == (short) 1;
        StorePath = getStorePath();
        if (bCreateTemplate == true){
            ReportDocument.createDBForm(xMSF, CurReportDocument, CurDBMetaData);
            ReportDocument.attachEventCall(CurReportDocument.ReportTextDocument, "OnNew", "macro:///Tools.Debug.FillDocument()");     //"service:com.sun.star.wizards.report.CallReportWizard?fill"
            boolean bUseTemplate = ((Short) UNODialogs.getPropertyOfDialogControl(xDlgNameAccess, "optUseTemplate", "State")).shortValue() == (short) 1;
            tools.storeDocument(xMSF, (XComponent) CurReportDocument.Component , StorePath, "swriter: writer_StarOffice_XML_Writer_Template", bUseTemplate);
            DBMetaData.createDBLink(CurDBMetaData.DataSource, StorePath);
            if (bUseTemplate == true)
            ReportDocument.insertDatabaseDatatoReportDocument(xMSF, CurDBMetaData, CurReportDocument);
        }
        else{
            boolean bcreateLink = ((Short) UNODialogs.getPropertyOfDialogControl(xDlgNameAccess, "chkcreateLink", "State")).shortValue() == (short) 1;
            ReportDocument.insertDatabaseDatatoReportDocument(xMSF, CurDBMetaData, CurReportDocument);
            tools.storeDocument(xMSF, CurReportDocument.Component , StorePath, "swriter: StarOffice XML (Writer)", false);
            if (bcreateLink == true)
            DBMetaData.createDBLink(CurDBMetaData.DataSource, StorePath);
        }
        bSetTitle = false;
        bCloseDocument = false;
        xDialog.endExecute();

        default:
                break;
        }
    if (bSetTitle == true){
        if (iPage < PageCount){
        tools.setUNOPropertyValues(oDialogModel, new String[]{"Step", "Title"}, new Object[]{ new Integer(iPage + 1), WizardTitle[iPage]});
        UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "lblDialogHeader", "Label", WizardHeaderText[iPage]);
        }
    }
    }
    catch( Exception exception ){
        exception.printStackTrace(System.out);
    }}


    public static void gotoPrevStep(){
    try{
        int iPage = ((Integer) tools.getUNOPropertyValue(oDialogModel, "Step")).intValue();
        switch (iPage){
            case 2:
        CurDBMetaData.OldGroupFieldNames = CurDBMetaData.GroupFieldNames;
        break;
            case 3:
        CurDBMetaData.OldSortFieldNames = CurDBMetaData.SortFieldNames;
        break;
        case 4:
        ReportDocument.removeTextSectionbyName(CurReportDocument, "RecordSection");
        ReportDocument.removeTextTablebyName(CurReportDocument, "Tbl_RecordSection");
        break;
        case 5:
        UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "cmdGoOn", "Label", scmdGoOn);
                UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "cmdGoOn", "Enabled", new Boolean(true));

                break;
            default:
                break;
        }
        UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "cmdBack", "Enabled", new Boolean(iPage > 2));

        if (iPage > 1)
            tools.setUNOPropertyValues(oDialogModel, new String[]{"Step", "Title"}, new Object[]{ new Integer(iPage - 1), WizardTitle[iPage-2]});
        UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "lblDialogHeader", "Label", WizardHeaderText[iPage-2]);
    }
      catch( Exception exception ){
           exception.printStackTrace(System.out);
    }}


    public static void addGroupNametoDocument(){
    try{
    CurReportDocument.ReportTextDocument.lockControllers();
    CurGroupName = xGroupListBox.getSelectedItem();
    short iSelPos = xGroupListBox.getSelectedItemPos();
    UNODialogs.removeSelectedItems(xGroupListBox);
    GroupFieldVector.addElement(CurGroupName);
    int GroupCount = GroupFieldVector.size();
    ReportDocument.insertTextSection(CurReportDocument, "GroupField" + GroupCount, CurReportDocument.ReportFolderName + "/cnt-default.stw");
    ReportDocument.replaceFieldValueInGroupTable(CurReportDocument, "Tbl_GroupField" + GroupCount, CurGroupName, GroupCount-1);
    xSelGroupListBox.addItem(CurGroupName, xSelGroupListBox.getItemCount());
    UNODialogs.selectListBoxItem(xGroupListBox, iSelPos);
    boolean bDoEnable = ((GroupCount < 4) && (xGroupListBox.getSelectedItems().length > 0));
    UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "cmdGroupOut", "Enabled", new Boolean(bDoEnable));
    }
    catch( Exception exception ){
         exception.printStackTrace(System.out);
    }
    CurReportDocument.ReportTextDocument.unlockControllers();
    }


   public static void toggleRemoveGroupButton(){
   try{
    boolean bDoEnable = xSelGroupListBox.getSelectedItems().length > 0;
    UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "cmdGroupIn", "Enabled", new Boolean(true));
    }
      catch( Exception exception ){
         exception.printStackTrace(System.out);
    }}


    public static void removeGroupName(){
    try{
    CurReportDocument.ReportTextDocument.lockControllers();
    short iSelPos = xSelGroupListBox.getSelectedItemPos();
    CurGroupName = xSelGroupListBox.getItem(iSelPos);
    GroupFieldVector.removeElement(CurGroupName);
    xGroupListBox.addItem(CurGroupName, (short) xGroupListBox.getItemCount());
    UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "cmdGroupOut", "Enabled", new Boolean(xGroupListBox.getSelectedItems().length > 0));
    UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "cmdGroupIn", "Enabled", new Boolean(GroupFieldVector.isEmpty() == false));
    ReportDocument.removeLastTextSection(CurReportDocument);
    ReportDocument.removeLastTextTable(CurReportDocument);
    xSelGroupListBox.removeItems( iSelPos,(short)1);
    if (iSelPos != xSelGroupListBox.getItemCount())
        ReportDocument.updateTextSections(CurReportDocument, xSelGroupListBox, GroupFieldVector.size());
    UNODialogs.selectListBoxItem(xSelGroupListBox, iSelPos);
    }
      catch( Exception exception ){
         exception.printStackTrace(System.out);
    }
    CurReportDocument.ReportTextDocument.unlockControllers();
    }


    public static void setUpSortList(){
    try{
    short iCurState = 0;
    MaxSortIndex = -1;
    for (int i = 0; i < 4; i++){
        if (xSortListBox[i].getSelectedItemPos() > 0)
        MaxSortIndex += 1;
        else
        break;
    }
        CurDBMetaData.SortFieldNames = new String[MaxSortIndex+1][2];
        for (int i=0;i<=MaxSortIndex;i++){
            CurDBMetaData.SortFieldNames[i][0] = xSortListBox[i].getSelectedItem();
        iCurState = ((Short) UNODialogs.getPropertyOfDialogControl(xDlgNameAccess, "optAscend" + new Integer(i+1).toString(), "State")).shortValue();
        if (iCurState == 1)
        CurDBMetaData.SortFieldNames[i][1] = "ASC";
        else
        CurDBMetaData.SortFieldNames[i][1] = "DESC";
        }
    }
    catch( Exception exception ){
        exception.printStackTrace(System.out);
     }}


    public static void enableNextSortListBox(int CurIndex){
    try{
        short iNextItemPos;
        boolean bDoEnable = (xSortListBox[CurIndex].getSelectedItemPos() > 0);      // the first Item is for "undefined"
    if (bDoEnable == true){
        if (CurIndex > MaxSortIndex)
        MaxSortIndex = CurIndex;
    }
        if ((bDoEnable == false) && (MaxSortIndex > CurIndex)){
            for (int i= CurIndex; i < MaxSortIndex; i++){
                iNextItemPos = xSortListBox[i+1].getSelectedItemPos();
                if (iNextItemPos != 0){
                    xSortListBox[i].selectItemPos(iNextItemPos,true);
                    xSortListBox[i+1].selectItemPos(iNextItemPos,false);
                    toggleSortListBox(i,true);
                    xSortListBox[i+1].selectItemPos((short)0,true);
                }
            }
            MaxSortIndex = MaxSortIndex - 1;
            if (MaxSortIndex < xSortListBox.length -2)
                toggleSortListBox(MaxSortIndex + 2, false);
        }
        else
            toggleSortListBox(CurIndex+1, bDoEnable);
    }
      catch( Exception exception ){
        exception.printStackTrace(System.out);
     }}


     public static void toggleSortListBox(int CurIndex, boolean bDoEnable){
     try{
        UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "lblSort" + new Integer(CurIndex+1).toString(), "Enabled", new Boolean(bDoEnable));
        UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "lstSort" + new Integer(CurIndex+1).toString(), "Enabled", new Boolean(bDoEnable));
    UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "optAscend" + new Integer(CurIndex+1).toString(), "Enabled", new Boolean(bDoEnable));
    UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "optDescend" + new Integer(CurIndex+1).toString(), "Enabled", new Boolean(bDoEnable));
    if (bDoEnable == false)
            xSortListBox[CurIndex].selectItemPos((short)0,true);
     }
     catch( Exception exception ){
        exception.printStackTrace(System.out);
      }}


    public static String getStorePath(){
    String sStorePath = "";
    try{
    boolean bStoreAsTemplate = ((Short) UNODialogs.getPropertyOfDialogControl(xDlgNameAccess, "optCreateReportTemplate", "State")).shortValue() == (short) 1;
    if (bStoreAsTemplate == true)
        sStorePath = (String) UNODialogs.getPropertyOfDialogControl(xDlgNameAccess, "txtSavePath_1", "Text");
    else
        sStorePath = (String) UNODialogs.getPropertyOfDialogControl(xDlgNameAccess, "txtSavePath_2", "Text");
    }
    catch( Exception exception ){
       exception.printStackTrace(System.out);
    }
    return sStorePath;
    }


    public static void insertStorePathToTextBox(XMultiServiceFactory xMSF){
    try{
    String sStorePath = "";
    boolean bStoreAsTemplate = ((Short) UNODialogs.getPropertyOfDialogControl(xDlgNameAccess, "optCreateReportTemplate", "State")).shortValue() == (short) 1;
    if (bStoreAsTemplate == true){
        sStorePath = tools.callStoreDialog(xMSF, tools.getOfficePath(xMSF, "Template","user"), "Report_" + CurDBMetaData.DataSourceName + ".stw", "writer_StarOffice_XML_Writer_Template");
        UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "txtSavePath_1", "Text", sStorePath);
    }
    else{
        sStorePath = tools.callStoreDialog(xMSF, tools.getOfficePath(xMSF, "Work",""),"Report_" + CurDBMetaData.DataSourceName + ".sxw",  "writer_StarOffice_XML_Writer");
        UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "txtSavePath_2", "Text", sStorePath);
    }
    UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "cmdGoOn", "Enabled", new Boolean(sStorePath != ""));
    }
    catch( Exception exception ){
        exception.printStackTrace(System.out);
    }}


    public static void insertSaveControls(int YPos, int Index, boolean bDoEnable, int TabIndex, int BaseHelpID){
    try{
    String HIDString;
    InsertControlModel("com.sun.star.awt.UnoControlFixedTextModel", xMSFDialogModel, xDlgNames, "lblSaveAs_" + Integer.toString(Index+1),
                            new String[] {"Enabled", "Height", "PositionX", "PositionY", "Step", "Width", "Label"},
                            new Object[] {new Boolean(bDoEnable), new Integer(8), new Integer(16), new Integer(YPos), new Integer(5), new Integer(130), sSaveAs});

    HIDString = "HID:" + Integer.toString(BaseHelpID);
    xSaveTextBox[Index] = InsertTextField(xMSFDialogModel, xDlgNames, xDialogContainer, "txtSavePath_" + Integer.toString(Index+1), SOTXTSAVEPATH[Index],
                            new String[] {"Enabled", "Height", "HelpURL", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
                            new Object[] {new Boolean(bDoEnable), new Integer(12), HIDString, new Integer(16), new Integer(YPos + 10), new Integer(5), new Short((short)TabIndex), new Integer(222)});

    HIDString = "HID:" + Integer.toString(BaseHelpID+1);
    InsertButton(xMSFDialogModel, xDlgNames, xDialogContainer, "cmdSelectPath_" + Integer.toString(Index+1), SOCMDSELECTPATH[Index],
                            new String[] {"Enabled", "Height", "HelpURL", "PositionX", "PositionY", "Step",  "TabIndex", "Width", "Label"},
                            new Object[] {new Boolean(bDoEnable), new Integer(14), HIDString, new Integer(248), new Integer(YPos + 9), new Integer(5), new Short((short) (TabIndex + 1)), new Integer(16), "..."});
    }
    catch( Exception exception ){
        exception.printStackTrace(System.out);
    }}


    public static void toggleSaveControls(){
    try{
    String sStorePath = "";
    Short iState = (Short) UNODialogs.getPropertyOfDialogControl(xDlgNameAccess, "optCreateReportTemplate", "State");
    boolean bDoTemplateEnable = iState.shortValue() == 1;
        UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "optEditTemplate", "Enabled", new Boolean (bDoTemplateEnable));
        UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "optUseTemplate", "Enabled", new Boolean (bDoTemplateEnable));

    UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "lblSaveAs_1", "Enabled", new Boolean (bDoTemplateEnable));
    UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "txtSavePath_1", "Enabled", new Boolean (bDoTemplateEnable));
    UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "cmdSelectPath_1", "Enabled", new Boolean (bDoTemplateEnable));

    UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "lblSaveAs_2", "Enabled", new Boolean (!bDoTemplateEnable));
    UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "txtSavePath_2", "Enabled", new Boolean(!bDoTemplateEnable));
    UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "cmdSelectPath_2", "Enabled", new Boolean(!bDoTemplateEnable));
    UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "chkcreateLink", "Enabled", new Boolean(!bDoTemplateEnable));

    if (bDoTemplateEnable == true)
        sStorePath = (String) UNODialogs.getPropertyOfDialogControl(xDlgNameAccess, "txtSavePath_1", "Text");
    else
        sStorePath = (String) UNODialogs.getPropertyOfDialogControl(xDlgNameAccess, "txtSavePath_2", "Text");
    boolean bDoEnable = sStorePath.compareTo("") != 0;
    UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "cmdGoOn", "Enabled", new Boolean(bDoEnable));
    }
    catch( Exception exception ){
        exception.printStackTrace(System.out);
    }}



    public static void fillFifthStep(){
    try{
    InsertRadioButton(xMSFDialogModel, xDlgNames, xDialogContainer, "optCreateReportTemplate", SOOPTSAVEASTEMPLATE,
                            new String[] {"Height", "HelpURL", "Label", "PositionX", "PositionY", "State", "Step", "TabIndex", "Width"},
                            new Object[] {new Integer(8), "HID:34370", sSaveAsTemplate, new Integer(6), new Integer(41), new Short((short) 1), new Integer(5), new Short((short) 40), new Integer(250)});

    InsertRadioButton(xMSFDialogModel, xDlgNames, xDialogContainer, "optCreateDocument", SOOPTSAVEASDOCUMENT,
                            new String[] {"Height", "HelpURL", "Label", "PositionX", "PositionY", "State", "Step", "TabIndex", "Width"},
                            new Object[] {new Integer(10), "HID:34371", sSaveAsDocument, new Integer(6), new Integer(117), new Short((short) 0), new Integer(5), new Short((short) 41), new Integer(138)});

    insertSaveControls(55, 0, true, 42, 34372);

    InsertRadioButton(xMSFDialogModel, xDlgNames, xDialogContainer, "optEditTemplate", SOOPTEDITTEMPLATE,
                            new String[] {"Height", "HelpURL", "Label", "PositionX", "PositionY", "State", "Step", "TabIndex", "Width"},
                            new Object[] {new Integer(10), "HID:34374", sEditTemplate, new Integer(16), new Integer(84), new Short((short) 1), new Integer(5), new Short((short) 44), new Integer(138)});

    InsertRadioButton(xMSFDialogModel, xDlgNames, xDialogContainer, "optUseTemplate", SOOPTUSEDOCUMENT,
                            new String[] {"Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
                            new Object[] {new Integer(10), "HID:34375", sUseTemplate, new Integer(16), new Integer(95), new Integer(5), new Short((short) 45), new Integer(138)});

    insertSaveControls(132, 1, false, 46, 34376);

    chkTemplate = InsertControlModel("com.sun.star.awt.UnoControlCheckBoxModel", xMSFDialogModel, xDlgNames, "chkcreateLink",
                new String[] {"Enabled", "Height", "PositionX", "PositionY", "Step", "TabIndex", "Width", "Label"},
                new Object[] {new Boolean(false), new Integer(8), new Integer(16), new Integer(161), new Integer(5), new Short((short) 48), new Integer(130), sCreateLink});
    }
    catch( Exception exception ){
        exception.printStackTrace(System.out);
    }}


    public static void fillFourthStep(XMultiServiceFactory xMSF){
    try{
        InsertControlModel("com.sun.star.awt.UnoControlFixedTextModel", xMSFDialogModel, xDlgNames, "lblTitle",
                            new String[] {"Height", "PositionX", "PositionY", "Step", "Width", "Label"},
                            new Object[] {new Integer(8), new Integer(6), new Integer(40), new Integer(4), new Integer(200), sReportTitle});

        xTitleTextBox = InsertTextField(xMSFDialogModel, xDlgNames, xDialogContainer, "txtTitle", SOTXTTITLE,
                            new String[] {"Height", "HelpURL", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
                            new Object[] {new Integer(12), "HID:34362", new Integer(6), new Integer(50), new Integer(4), new Short((short)31), new Integer(258)});

        InsertControlModel("com.sun.star.awt.UnoControlFixedTextModel", xMSFDialogModel, xDlgNames, "lblContent",
                new String[] {"Height", "PositionX", "PositionY", "Step", "Width", "Label"},
                new Object[] {new Integer(8), new Integer(6), new Integer(70), new Integer(4), new Integer(125), slblDataStructure});

        ContentFiles = tools.getFolderTitles(xMSF, "cnt", CurReportDocument.ReportFolderName);
        xContentListBox = InsertListbox(xMSFDialogModel, xDlgNames, xDialogContainer, "lstContent", SOCONTENTLST,
                    new String[] {"Height", "HelpURL", "PositionX", "PositionY", "Step", "StringItemList", "TabIndex", "Width"},
                            new Object[] {new Integer(58), "HID:34363", new Integer(6), new Integer(80), new Integer(4), ContentFiles[1], new Short((short)32), new Integer(125)});
        short iSelPos = (short) tools.FieldInList(ContentFiles[0], CurReportDocument.ReportFolderName + "/cnt-default.stw");
        xContentListBox.selectItemPos(iSelPos, true);

        InsertControlModel("com.sun.star.awt.UnoControlFixedTextModel", xMSFDialogModel, xDlgNames, "lblLayout",
                new String[] {"Height", "PositionX", "PositionY", "Step", "Width", "Label"},
                new Object[] {new Integer(8), new Integer(140), new Integer(70), new Integer(4), new Integer(125), slblPageLayout});

        LayoutFiles = tools.getFolderTitles(xMSF,"stl", CurReportDocument.ReportFolderName);
        xLayoutListBox = InsertListbox(xMSFDialogModel, xDlgNames, xDialogContainer, "lstLayout", SOLAYOUTLST,
                    new String[] {"Height", "HelpURL", "PositionX", "PositionY", "Step", "StringItemList", "TabIndex", "Width"},
                            new Object[] {new Integer(58), "HID:34364", new Integer(140), new Integer(80), new Integer(4), LayoutFiles[1], new Short((short)33), new Integer(125)});
        short iSelLayoutPos = (short) tools.FieldInList(LayoutFiles[0], CurReportDocument.ReportFolderName + "/stl-default.stw");
        xLayoutListBox.selectItemPos(iSelLayoutPos, true);

        InsertControlModel("com.sun.star.awt.UnoControlFixedLineModel", xMSFDialogModel, xDlgNames, "hlnOrientation",
                new String[] {"Height", "Label", "Orientation", "PositionX", "PositionY", "Step", "Width"},
                new Object[] {new Integer(2), "", new Integer(0), new Integer(6), new Integer(144), new Integer(4), new Integer(258)});

        InsertControlModel("com.sun.star.awt.UnoControlFixedTextModel", xMSFDialogModel, xDlgNames, "lblOrientation",
                            new String[] {"Height", "PositionX", "PositionY", "Step", "Width", "Label"},
                            new Object[] {new Integer(8), new Integer(6), new Integer(149), new Integer(4), new Integer(74), sOrientationHeader});

        InsertRadioButton(xMSFDialogModel, xDlgNames, xDialogContainer, "optLandscape", SOOPTLANDSCAPE,
                            new String[] {"Height", "HelpURL", "Label", "PositionX", "PositionY", "State", "Step", "TabIndex", "Width"},
                            new Object[] {new Integer(10), "HID:34365", sOrientHorizontal, new Integer(12), new Integer(160), new Short((short) 1), new Integer(4), new Short((short)34), new Integer(100)});

        InsertRadioButton(xMSFDialogModel, xDlgNames, xDialogContainer, "optPortrait", SOOPTPORTRAIT,
                            new String[] {"Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
                            new Object[] {new Integer(10), "HID:34366", sOrientVertical, new Integer(12), new Integer(173), new Integer(4), new Short((short)35), new Integer(100)});
        String sTemplatePath = tools.getOfficePath(xMSF, "Template","share") + "/wizard/bitmap/landscape.gif";

            InsertControlModel("com.sun.star.awt.UnoControlImageControlModel", xMSFDialogModel, xDlgNames, "imgOrientation",
                            new String[] {"Border", "Height", "ImageURL", "PositionX", "PositionY", "ScaleImage", "Step", "Width"},
                            new Object[] {new Short("0"), new Integer(23), sTemplatePath, new Integer(80), new Integer(158), new Boolean(false), new Integer(4), new Integer(30)});
    }
    catch( Exception exception ){
        exception.printStackTrace(System.out);
    }}


    public static boolean checkIfToupdateStep(){
    boolean bIsSame = Arrays.equals(CurDBMetaData.FieldNames, CurDBMetaData.OldFieldNames) && (CurDBMetaData.FieldNames != null);
    if (bIsSame == true)
        bIsSame = Arrays.equals(CurDBMetaData.GroupFieldNames, CurDBMetaData.OldGroupFieldNames) && (CurDBMetaData.GroupFieldNames != null);
    if (bIsSame == true)
        bIsSame = Arrays.equals(CurDBMetaData.SortFieldNames, CurDBMetaData.OldSortFieldNames) && (CurDBMetaData.SortFieldNames != null);
    return !bIsSame;
    }


    public static void updateThirdStep(){
    try{
    if (checkIfToupdateStep() == true){
        CurDBMetaData.OldGroupFieldNames = CurDBMetaData.GroupFieldNames;
        String[] GroupFieldNames = new String[GroupFieldVector.size()];
        GroupFieldVector.copyInto(GroupFieldNames);
        CurDBMetaData.GroupFieldNames = GroupFieldNames;
        int FieldCount = CurDBMetaData.FieldNames.length;
        int SortFieldCount = FieldCount + 1-CurDBMetaData.GroupFieldNames.length;
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
        for (int i = 0; i<4; i++){
        UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "lstSort" + new Integer(i+1).toString(), "StringItemList", SortFieldNames);
        }
    }
    }
    catch(Exception exception){
        exception.printStackTrace(System.out);
    }}


    public static void fillThirdStep(){
    try{
    boolean bDoEnable;
    String HIDString;
    int YPos = 40;
    int BaseHelpID = 34321;
    for (int i = 0; i<4; i++){
        bDoEnable = (i == 0);
        InsertControlModel("com.sun.star.awt.UnoControlFixedLineModel", xMSFDialogModel, xDlgNames, "lblSort" + new Integer(i+1),
                new String[] {"Enabled", "Height", "Label", "Orientation", "PositionX", "PositionY", "Step", "Width"},
                new Object[] {new Boolean(bDoEnable), new Integer(8), sSortHeader[i], new Integer(0), new Integer(12), new Integer(YPos), new Integer(3), new Integer(252)});

        HIDString = "HID:" + Integer.toString(BaseHelpID);
        xSortListBox[i] = InsertListbox(xMSFDialogModel, xDlgNames, xDialogContainer, "lstSort" + new Integer(i+1).toString(), SOSORTLST[i],
                new String[] {"Enabled", "Height", "HelpURL", "PositionX", "PositionY", "Step", "TabIndex", "Width", "Dropdown", "LineCount", "Name"},
                new Object[] {new Boolean(bDoEnable), new Integer(12), HIDString,  new Integer(12), new Integer(YPos + 14), new Integer(3), new Short((short) (17+i*4)), new Integer(154), new Boolean(true), new Short("7"), "lstSort" + new Integer(i+1)});

        HIDString = "HID:" + Integer.toString(BaseHelpID+1);
        InsertRadioButton(xMSFDialogModel, xDlgNames, xDialogContainer, "optAscend" + Integer.toString(i+1), SOSORTASCENDOPT[i],
                new String[] {"Enabled", "Height", "HelpURL", "Label", "PositionX", "PositionY", "State", "Step", "TabIndex", "Tag", "Width"},
                new Object[] {new Boolean(bDoEnable), new Integer(10), HIDString,  sSortAscend[i], new Integer(186), new Integer(YPos+10), new Short((short) 1), new Integer(3), new Short((short) (18+i*4)), new String("ASC"), new Integer(65)});

        HIDString = "HID:" + Integer.toString(BaseHelpID+2);
        InsertRadioButton(xMSFDialogModel, xDlgNames, xDialogContainer, "optDescend" + Integer.toString(i+1), SOSORTDESCENDOPT[i],
                           new String[] {"Enabled", "Height", "HelpURL", "Label", "PositionX", "PositionY", "State", "Step", "TabIndex", "Tag", "Width"},
                           new Object[] {new Boolean(bDoEnable), new Integer(10), HIDString, sSortDescend[i], new Integer(186), new Integer(YPos+24), new Short((short) 0), new Integer(3), new Short((short) (19+i*4)), new String("DESC"), new Integer(65)});
        YPos = YPos + 36;
        BaseHelpID += 3;
    }
    }
    catch(Exception exception){
        exception.printStackTrace(System.out);
    }}


    public static void updateSecondStep(){
    try{
    UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "cmdBack", "Enabled", new Boolean(true));
        CurDBMetaData.FieldNames = xSelFieldsListBox.getItems();
    if (checkIfToupdateStep() == true){
        ReportDocument.removeAllTextSections(CurReportDocument.TextSectionsSupplier, CurReportDocument.ReportTextDocument.getText());
        ReportDocument.removeAllTextTables(CurReportDocument.TextTablesSupplier, CurReportDocument.ReportTextDocument.getText());
        CurDBMetaData.OldFieldNames = CurDBMetaData.FieldNames;
        // Todo: Nur ausführen, when FieldNames anders als vorher-> dann muss auch Selektionslistbox leer gemacht werden.
        UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "lstGroup", "StringItemList", CurDBMetaData.FieldNames);
        xSelGroupListBox.removeItems((short) 0, xSelGroupListBox.getItemCount());
        GroupFieldVector = new java.util.Vector(CurDBMetaData.FieldNames.length);
    }
    }
    catch( Exception exception ){
        exception.printStackTrace(System.out);
    }}


    public static void fillSecondStep(){
    try{
        InsertControlModel("com.sun.star.awt.UnoControlFixedTextModel", xMSFDialogModel, xDlgNames, "lblGroups",
            new String[] {"Height", "PositionX", "PositionY", "Step", "Width", "Label"},
            new Object[] {new Integer(8), new Integer(6), new Integer(38), new Integer(2), new Integer(100), sOrganizeFields});

        xGroupListBox = InsertListbox(xMSFDialogModel, xDlgNames, xDialogContainer, "lstGroup", SOGROUPLST,
                new String[] {"Height", "HelpURL", "PositionX", "PositionY", "Step", "TabIndex", "Width", "MultiSelection"},
                        new Object[] {new Integer(125), "HID:34340", new Integer(6), new Integer(49), new Integer(2), new Short((short) 11), new Integer(110), new Boolean(false)});

        InsertControlModel("com.sun.star.awt.UnoControlFixedTextModel", xMSFDialogModel, xDlgNames, "lblSelGroups",
            new String[] {"Height", "PositionX", "PositionY", "Step", "Width", "Label"},
            new Object[] {new Integer(8), new Integer(154), new Integer(38), new Integer(2), new Integer(100), sGroupings});
        xSelGroupListBox = InsertListbox(xMSFDialogModel, xDlgNames, xDialogContainer, "lstSelGroup", SOSELGROUPLST,
                new String[] {"Height", "HelpURL", "PositionX", "PositionY", "Step", "TabIndex", "Width", "MultiSelection"},
                        new Object[] {new Integer(125), "HID:34343", new Integer(154), new Integer(49), new Integer(2), new Short((short) 12), new Integer(110), new Boolean(true)});
        InsertButton(xMSFDialogModel, xDlgNames, xDialogContainer, "cmdGroupOut", SOCMDGROUPOUT,
            new String[] {"Enabled", "HelpURL", "Height", "PositionX", "PositionY", "Step", "TabIndex", "Width", "Label"},
            new Object[] {new Boolean(false), "HID:34341", new Integer(14), new Integer(122), new Integer(93), new Integer(2), new Short((short) 13), new Integer(25),"->"});
        InsertButton(xMSFDialogModel, xDlgNames, xDialogContainer, "cmdGroupIn", SOCMDGROUPIN,
            new String[] {"Enabled", "HelpURL", "Height", "PositionX", "PositionY", "Step", "TabIndex", "Width", "Label"},
            new Object[] {new Boolean(false), "HID:34342", new Integer(14), new Integer(122), new Integer(115), new Integer(2), new Short((short) 14), new Integer(25), "<-"});
    }
    catch( Exception exception ){
        exception.printStackTrace(System.out);
    }}



    public static void main (String args[]) {
        String ConnectStr = "uno:socket,host=localhost,port=8100;urp,negotiate=0,forcesynchronous=1;StarOffice.NamingService";
    try {
            xGlobalMSF = connect(ConnectStr);
        if(xGlobalMSF != null)  System.out.println("Connected to "+ ConnectStr);
        startReportWizard(xGlobalMSF);
    }
        catch(Exception exception) {
            exception.printStackTrace(System.out);
        }
        System.exit(0);
    }


    public static void startReportWizard(XMultiServiceFactory xMSF){
    try{
    xGlobalMSF = xMSF;
    xDesktop = tools.getDesktop( xMSF );
    XFramesSupplier xFrameSuppl = (XFramesSupplier) UnoRuntime.queryInterface(XFramesSupplier.class, xDesktop);
    getReportResources(xMSF);
    CurReportDocument =  new ReportDocument.RepWizardDocument();
    CurDBMetaData = new DBMetaData.CommandMetaData();
    CurReportDocument.ReportTextDocument =  (XTextDocument) tools.createNewDocument(xDesktop, CurReportDocument.Component, "swriter");
    ReportDocument.initializeReportDocument(xMSF, CurReportDocument);
    ReportDocument.loadStyleTemplates(CurReportDocument.ReportTextDocument, CurReportDocument.ReportFolderName + "/stl-default.stw", "LoadPageStyles");
        initializeDialog(xMSF, CurReportDocument);
    CurReportDocument.ProgressBar.setValue(16);
    fillFirstStep(xMSF, CurReportDocument);
    CurReportDocument.ProgressBar.setValue(32);
    fillSecondStep();
    CurReportDocument.ProgressBar.setValue(48);
    fillThirdStep();
    CurReportDocument.ProgressBar.setValue(64);
    fillFourthStep(xMSF);
    CurReportDocument.ProgressBar.setValue(80);
    fillFifthStep();
    CurReportDocument.ProgressBar.setValue(100);
    bCloseDocument = true;
    executeDialog(xMSF, CurReportDocument);
    }
    catch(java.lang.Exception jexception ){
    jexception.printStackTrace(System.out);
    }}


    public static void initializeDialog(XMultiServiceFactory xMSF, ReportDocument.RepWizardDocument CurReportDocument){
    try{
    DBMetaData.InitializeWidthList();
        ControlList = new Hashtable();
    oDialogModel = xMSF.createInstance("com.sun.star.awt.UnoControlDialogModel");
    XMultiPropertySet xMultiPSetDlg = (XMultiPropertySet) UnoRuntime.queryInterface(XMultiPropertySet.class, oDialogModel);
        xMultiPSetDlg.setPropertyValues(new String[] {"Height", "PositionX", "PositionY", "Step", "Title", "Width"},
                                        new Object[] {new Integer(210), new Integer(200), new Integer(250), new Integer(1), WizardTitle[0], new Integer(270)});
    // get service manager from dialog model
    xMSFDialogModel = (XMultiServiceFactory) UnoRuntime.queryInterface(XMultiServiceFactory.class, oDialogModel);

        // create dialog control
    objectDialog = xMSF.createInstance("com.sun.star.awt.UnoControlDialog");
        xControl = ( XControl ) UnoRuntime.queryInterface( XControl.class, objectDialog );
        XControlModel xControlModel = ( XControlModel ) UnoRuntime.queryInterface( XControlModel.class, oDialogModel );
        xControl.setModel( xControlModel );

        xDialogContainer = ( XControlContainer ) UnoRuntime.queryInterface( XControlContainer.class, objectDialog );
        xDlgNames = (XNameContainer) UnoRuntime.queryInterface( XNameContainer.class, oDialogModel );
        xDlgNameAccess = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, oDialogModel);
    }
    catch(Exception exception)
    {
        exception.printStackTrace(System.out);
    }}



    public static void executeDialog(XMultiServiceFactory xMSF, ReportDocument.RepWizardDocument CurReportDocument){
    try{
    XWindow xWindow = (XWindow) UnoRuntime.queryInterface(XWindow.class, objectDialog);
        xWindow.setVisible(false);
        xDialog = (XDialog) UnoRuntime.queryInterface( XDialog.class, objectDialog );
    Object objectToolkit = xMSF.createInstance("com.sun.star.awt.ExtToolkit");
        XToolkit xToolkit = (XToolkit) UnoRuntime.queryInterface( XToolkit.class, objectToolkit );
        xControl.createPeer(xToolkit, null);
    CurReportDocument.ProgressBar.end();
    short retvalue = xDialog.execute();
        switch (retvalue){
        case 0:     // Cancel
        XNameContainer xNamedForms = ReportDocument.getDocumentForms(CurReportDocument.ReportTextDocument);
        XNameAccess xName = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, xNamedForms);
        if (xName.hasByName("ReportSource") == false){
            if (bCloseDocument == true){
            XComponent xComponent = ( XComponent ) UnoRuntime.queryInterface(XComponent.class, objectDialog);
            xComponent.dispose();
            CurReportDocument.Component.dispose();
            }
            return;
        }
        break;
            case 1:
                break;
        }
        XComponent xComponent = ( XComponent ) UnoRuntime.queryInterface(XComponent.class, objectDialog);
    xComponent.dispose();
    }
    catch(Exception exception)
    {
        exception.printStackTrace(System.out);
    }}


    public static void fillFirstStep(XMultiServiceFactory xMSF, ReportDocument.RepWizardDocument CurReportDocument){
    try{
    com.sun.star.awt.FontDescriptor oFontDesc = new com.sun.star.awt.FontDescriptor();
    oFontDesc.Weight = com.sun.star.awt.FontWeight.BOLD;
    Object oHeaderLabel =InsertControlModel("com.sun.star.awt.UnoControlFixedTextModel", xMSFDialogModel, xDlgNames, "lblDialogHeader",
                            new String[] {"BackgroundColor", "FontDescriptor", "Height", "Label", "MultiLine", "PositionX", "PositionY", "Step", "Width"},
                            new Object[] {new Integer(16777215), oFontDesc, new Integer(30), WizardHeaderText[0], new Boolean(true), new Integer(50), new Integer(0), new Integer(0), new Integer(220)});
    InsertButton(xMSFDialogModel, xDlgNames, xDialogContainer, "cmdCancel", SOCMDCANCEL,
                            new String[] {"Height", "HelpURL", "PositionX", "PositionY", "PushButtonType", "Step", "TabIndex", "Width", "Label"},
                            new Object[] {new Integer(14), "HID:34321", new Integer(6), new Integer(190), new Short((short)com.sun.star.awt.PushButtonType.CANCEL_value), new Integer(0), new Short((short) 60), new Integer(53), scmdCancel});

    InsertButton(xMSFDialogModel, xDlgNames, xDialogContainer, "cmdHelp", SOCMDHELP,
                            new String[] {"Height", "PositionX", "PositionY", "PushButtonType", "Step", "TabIndex", "Width", "Label"},
                            new Object[] {new Integer(14), new Integer(63), new Integer(190), new Short((short)com.sun.star.awt.PushButtonType.HELP_value), new Integer(0), new Short((short) 61), new Integer(53), scmdHelp});

        InsertButton(xMSFDialogModel, xDlgNames, xDialogContainer, "cmdBack", SOCMDBACK,
                            new String[] {"Enabled", "Height", "HelpURL", "PositionX", "PositionY", "Step", "TabIndex", "Width", "Label"},
                            new Object[] {new Boolean(false), new Integer(14), "HID:34322", new Integer(155), new Integer(190), new Integer(0), new Short((short) 62), new Integer(53), scmdBack});

        InsertButton(xMSFDialogModel, xDlgNames, xDialogContainer, "cmdGoOn", SOCMDGOON,
                            new String[] {"Enabled", "Height", "HelpURL", "PositionX", "PositionY", "Step", "TabIndex", "Width", "Label"},
                            new Object[] {new Boolean(false), new Integer(14), "HID:34323", new Integer(211), new Integer(190), new Integer(0), new Short((short) 63), new Integer(53), scmdGoOn});

    InsertControlModel("com.sun.star.awt.UnoControlFixedLineModel", xMSFDialogModel, xDlgNames, "hlnCommandButtons",
                new String[] {"Height", "Label", "Orientation", "PositionX", "PositionY", "Step", "Width"},
                new Object[] {new Integer(2), "", new Integer(0), new Integer(6), new Integer(184), new Integer(0), new Integer(258)});

    InsertButton(xMSFDialogModel, xDlgNames, xDialogContainer, "cmdMoveSelected", SOCMDMOVESEL,
                            new String[] {"Enabled", "Height", "HelpURL", "PositionX", "PositionY", "Step", "TabIndex", "Width", "Label"},
                            new Object[] {new Boolean(false), new Integer(14), "HID:34334", new Integer(122), new Integer(84), new Integer(1), new Short((short) 4), new Integer(25), "->"});

        InsertButton(xMSFDialogModel, xDlgNames, xDialogContainer, "cmdMoveAll", SOCMDMOVEALL,
                            new String[] {"Enabled", "Height", "HelpURL", "PositionX", "PositionY", "Step", "TabIndex", "Width", "Label"},
                            new Object[] {new Boolean(false), new Integer(14), "HID:34335", new Integer(122), new Integer(108), new Integer(1), new Short((short) 5), new Integer(25), "=>>"});

    InsertButton(xMSFDialogModel, xDlgNames, xDialogContainer, "cmdRemoveSelected", SOCMDREMOVESEL,
                            new String[] {"Enabled", "Height", "HelpURL", "PositionX", "PositionY", "Step", "TabIndex", "Width", "Label"},
                            new Object[] {new Boolean(false), new Integer(14), "HID:34336", new Integer(122), new Integer(131), new Integer(1), new Short((short) 6), new Integer(25), "<-"});

        InsertButton(xMSFDialogModel, xDlgNames, xDialogContainer, "cmdRemoveAll", SOCMDREMOVEALL,
                            new String[] {"Enabled", "Height", "HelpURL", "PositionX", "PositionY", "Step", "TabIndex", "Width", "Label"},
                            new Object[] {new Boolean(false), new Integer(14), "HID:34337", new Integer(122), new Integer(157), new Integer(1), new Short((short) 7), new Integer(25), "<<="});

        InsertControlModel("com.sun.star.awt.UnoControlFixedTextModel", xMSFDialogModel, xDlgNames, "lblDatabases",
                            new String[] {"Height", "PositionX", "PositionY", "Step", "Width", "Label"},
                            new Object[] {new Integer(8), new Integer(6), new Integer(39), new Integer(1), new Integer(74), slblDatabases});

    InsertControlModel("com.sun.star.awt.UnoControlFixedTextModel", xMSFDialogModel, xDlgNames, "lblTables",
                            new String[] {"Enabled", "Height", "Label", "PositionX", "PositionY", "Step", "Width"},
                            new Object[] {new Boolean(false), new Integer(8), slblTables, new Integer(122), new Integer(39), new Integer(1), new Integer(72)});

        InsertControlModel("com.sun.star.awt.UnoControlFixedTextModel", xMSFDialogModel, xDlgNames, "lblFields",
                            new String[] {"Enabled", "Height", "PositionX", "PositionY", "Step", "Width", "Label"},
                            new Object[] {new Boolean(false), new Integer(8), new Integer(6), new Integer(69), new Integer(1), new Integer(109), slblFields});

        InsertControlModel("com.sun.star.awt.UnoControlFixedTextModel", xMSFDialogModel, xDlgNames, "lblSelFields",
                            new String[] {"Enabled", "Height", "PositionX", "PositionY", "Step", "Width", "Label"},
                            new Object[] {new Boolean(false), new Integer(8), new Integer(154), new Integer(69), new Integer(1), new Integer(110), slblSelFields});

    String sTemplatePath = tools.getOfficePath(xMSF, "Template","share") + "/wizard/bitmap/report.bmp";
        InsertControlModel("com.sun.star.awt.UnoControlImageControlModel", xMSFDialogModel, xDlgNames, "imgTheme",
                            new String[] {"BackgroundColor", "Border", "Height", "ImageURL", "PositionX", "PositionY", "ScaleImage", "Step", "Width"},
                            new Object[] {new Integer(16777215), new Short("0"), new Integer(30), sTemplatePath, new Integer(0), new Integer(0), new Boolean(false), new Integer(0), new Integer(50)});

        xDBListBox = InsertListbox(xMSFDialogModel, xDlgNames, xDialogContainer, "lstDatabases", SODBLST,
                            new String[] {"Height", "HelpURL", "PositionX", "PositionY", "Step", "StringItemList", "TabIndex", "Width", "Dropdown","LineCount", "Name"},
                            new Object[] {new Integer(12), "HID:34330", new Integer(6), new Integer(49), new Integer(1), DBMetaData.getDatabaseNames(CurReportDocument), new Short((short) 1), new Integer(110), new Boolean(true), new Short("7"), "lstDatabases"});
    xTableListBox = InsertListbox(xMSFDialogModel, xDlgNames, xDialogContainer, "lstTables", SOTBLLST,
                            new String[] {"Enabled", "Height", "HelpURL", "PositionX", "PositionY", "Step", "TabIndex", "Width", "Dropdown", "LineCount"},
                            new Object[] {new Boolean(false), new Integer(12), "HID:34331", new Integer(122), new Integer(49), new Integer(1), new Short((short) 2), new Integer(110), new Boolean(true), new Short("7")});
    xFieldsListBox = InsertListbox(xMSFDialogModel, xDlgNames, xDialogContainer, "lstFields", SOFLDSLST,
                new String[] {"Enabled", "Height", "HelpURL", "PositionX", "PositionY", "Step", "TabIndex", "Width", "MultiSelection"},
                            new Object[] {new Boolean(false), new Integer(96), "HID:34332", new Integer(6), new Integer(79), new Integer(1), new Short((short) 3), new Integer(110), new Boolean(true)});
    xSelFieldsListBox = InsertListbox(xMSFDialogModel, xDlgNames, xDialogContainer, "lstSelFields", SOSELFLDSLST,
                            new String[] {"Enabled", "Height", "HelpURL", "PositionX", "PositionY", "Step", "TabIndex", "Width", "MultiSelection"},
                            new Object[] {new Boolean(false), new Integer(96), "HID:34333", new Integer(154), new Integer(79), new Integer(1), new Short((short) 8), new Integer(110), new Boolean(true)});
        }
        catch(Exception exception){
        exception.printStackTrace(System.out);
        }}


        public static void InsertButton(XMultiServiceFactory xMSFDialogModel, XNameContainer xDlgNames, XControlContainer xDialogContainer, String sName,
                                        int iControlKey, String[] sProperties, Object[] sValues)
        throws com.sun.star.uno.Exception{
           Object oButtonModel = InsertControlModel("com.sun.star.awt.UnoControlButtonModel", xMSFDialogModel, xDlgNames, sName, sProperties, sValues);
           XPropertySet xPSet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, oButtonModel);
           xPSet.setPropertyValue("Name", sName);
           Object objectButton = xDialogContainer.getControl( new String(sName));

           XButton xButton = ( XButton ) UnoRuntime.queryInterface( XButton.class, objectButton );
           xButton.addActionListener( new ActionListenerImpl() );
           Integer ControlKey = new Integer(iControlKey);
           ControlList.put(sName, ControlKey);
        }


        public static XTextComponent InsertTextField(XMultiServiceFactory xMSFDialogModel, XNameContainer xDlgNames, XControlContainer xDialogContainer, String sName,
                                        int iControlKey, String[] sProperties, Object[] sValues)
        throws com.sun.star.uno.Exception{
           Object oButtonModel = InsertControlModel("com.sun.star.awt.UnoControlEditModel", xMSFDialogModel, xDlgNames, sName, sProperties, sValues);
           XPropertySet xPSet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, oButtonModel);
           xPSet.setPropertyValue("Name", sName);
           Object oTextField = xDialogContainer.getControl( new String(sName));
           XTextComponent xTextBox = (XTextComponent) UnoRuntime.queryInterface(XTextComponent.class, oTextField);
       xTextBox.addTextListener( new TextListenerImpl());
           Integer ControlKey = new Integer(iControlKey);
           ControlList.put(sName, ControlKey);
       return xTextBox;
        }


        static public XListBox InsertListbox(XMultiServiceFactory xMSFDialogModel, XNameContainer xDlgNames, XControlContainer xDialogContainer, String sName,
                                           int iControlKey, String[] sProperties, Object[] sValues)
        throws com.sun.star.uno.Exception{
            Object xListBoxModel = InsertControlModel("com.sun.star.awt.UnoControlListBoxModel", xMSFDialogModel, xDlgNames, sName, sProperties, sValues);
            XPropertySet xPSet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xListBoxModel);
            xPSet.setPropertyValue("Name", sName);
            Object oListBox = xDialogContainer.getControl( new String(sName));
            com.sun.star.awt.XListBox xListBox = (com.sun.star.awt.XListBox) UnoRuntime.queryInterface(com.sun.star.awt.XListBox.class, oListBox);
            xListBox.addItemListener( new ItemListenerImpl());
            xListBox.addActionListener( new ActionListenerImpl() );
            Integer ControlKey = new Integer(iControlKey);
            ControlList.put(sName, ControlKey);
            return xListBox;
       }


       static public XButton InsertRadioButton(XMultiServiceFactory xMSFDialogModel, XNameContainer xDlgNames, XControlContainer xDialogContainer, String sName,
                                           int iControlKey, String[] sProperties, Object[] sValues)
        throws com.sun.star.uno.Exception{
            Object oRadioButtonModel = InsertControlModel("com.sun.star.awt.UnoControlRadioButtonModel", xMSFDialogModel, xDlgNames, sName, sProperties, sValues);
            XPropertySet xPSet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, oRadioButtonModel);
            xPSet.setPropertyValue("Name", sName);
            Object oRadioButton = xDialogContainer.getControl( new String(sName));
            com.sun.star.awt.XButton xRadioButton = (com.sun.star.awt.XButton) UnoRuntime.queryInterface(com.sun.star.awt.XButton.class, oRadioButton);
            xRadioButton.addActionListener(new ActionListenerImpl());
            Integer ControlKey = new Integer(iControlKey);
            ControlList.put(sName, ControlKey);
            return xRadioButton;
       }


        public static Object InsertControlModel(String ServiceName, XMultiServiceFactory xMSFDialogModel, XNameContainer xDlgNames, String sName,
                                                String[] sProperties, Object[] sValues)
        throws com.sun.star.uno.Exception{
           Object objectControlModel = xMSFDialogModel.createInstance(ServiceName);
           tools.setUNOPropertyValues(objectControlModel, sProperties, sValues);
//           XMultiPropertySet xMultiPSetLst = (XMultiPropertySet) UnoRuntime.queryInterface(XMultiPropertySet.class, objectControlModel);
//           xMultiPSetLst.setPropertyValues(sProperties, sValues);
       xDlgNames.insertByName(sName, objectControlModel);
       return objectControlModel;
        }


    public static XMultiServiceFactory connect( String connectStr )
    throws com.sun.star.uno.Exception, com.sun.star.uno.RuntimeException, Exception {
        XComponentContext xcomponentcontext = null;
    XMultiComponentFactory xMultiComponentFactory = tools.getMultiComponentFactory();
        // create a connector, so that it can contact the office
        Object  xUrlResolver  = xMultiComponentFactory.createInstanceWithContext("com.sun.star.bridge.UnoUrlResolver", xcomponentcontext );
        XUnoUrlResolver urlResolver = (XUnoUrlResolver)UnoRuntime.queryInterface( XUnoUrlResolver.class, xUrlResolver );

        Object rInitialObject = urlResolver.resolve( connectStr );
    xMultiComponentFactory = tools.getMultiComponentFactory();

        XNamingService rName = (XNamingService)UnoRuntime.queryInterface(XNamingService.class, rInitialObject );

        XMultiServiceFactory xMSF = null;
        if( rName != null ) {
            System.err.println( "got the remote naming service !" );
            Object rXsmgr = rName.getRegisteredObject("StarOffice.ServiceManager" );
            xMSF = (XMultiServiceFactory) UnoRuntime.queryInterface( XMultiServiceFactory.class, rXsmgr );
    }
        return ( xMSF );
    }



    public static void getReportResources(XMultiServiceFactory xMSF){
        final int RID_COMMON = 1000;
        final int RID_FORM = 2200;
        final int RID_REPORT = 2400;

        XInvocation xResInvoke = tools.initResources(xMSF, "ReportWizard","dbw");
        sMsgWizardName = tools.getResText(xResInvoke, RID_REPORT);
        scmdReady = tools.getResText(xResInvoke, RID_COMMON + 0);
        scmdCancel = tools.getResText(xResInvoke, RID_COMMON + 1);
        scmdBack = tools.getResText(xResInvoke, RID_COMMON + 2);
    scmdGoOn = tools.getResText(xResInvoke, RID_COMMON + 3);
        scmdHelp = tools.getResText(xResInvoke, RID_COMMON + 20);

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

    sWriterFilterName = tools.getResText(xResInvoke, RID_FORM + 70);
    }
}   