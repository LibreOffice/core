/*
 * fillCellBridge.java
 *
 * Created on 14. März 2002, 14:42
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

import com.sun.star.awt.XListBox;
import com.sun.star.awt.XRadioButton;
import com.sun.star.awt.XControl;
import com.sun.star.awt.XControlModel;
import com.sun.star.awt.XControlContainer;
import com.sun.star.awt.XWindow;
import com.sun.star.awt.XToolkit;
import com.sun.star.awt.XDialog;
import com.sun.star.awt.XButton;
import com.sun.star.awt.XActionListener;
import com.sun.star.awt.XItemListener;
import com.sun.star.awt.ItemEvent;
import com.sun.star.awt.ActionEvent;
import com.sun.star.awt.XMouseListener;
import com.sun.star.awt.XTextComponent;
import com.sun.star.awt.MouseEvent;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XComponentLoader;

import java.io.*;
import java.util.*;


/**
 *
 * @author  bc93774
 * @version
 */
public class ReportWizard {
    static long iStart;
    static XMultiComponentFactory xMultiComponentFactory;
    static XComponentContext xcomponentcontext;
    static XMultiServiceFactory xMSF;
    static Object xDatabaseContext;
    static Object xListBoxModel;
    static Object oDialogModel;
    static XNameAccess xDlgNameAccess;
    static Hashtable ControlList;
    static XNameContainer xDlgNames;
    static XStatement xStatement;
    static com.sun.star.sdbc.XConnection xDBConnection;
    static XDatabaseMetaData xDBMetaData;
    static XResultSet xResultSet;
    static XTextTable xTextTable;
    static DBMetaData.CommandMetaData CurMetaData;
    static String ReportFolderName;
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

    static final int SOFIRSTSORTCMD = 18;
    static final int SOSECSORTCMD = 19;
    static final int SOTHIRDSORTCMD = 20;
    static final int SOFOURTHSORTCMD = 21;
    static final int[] SOSORTCMD = new int[]{SOFIRSTSORTCMD, SOSECSORTCMD, SOTHIRDSORTCMD, SOFOURTHSORTCMD};
    static final int SOCMDGROUPIN = 22;
    static final int SOCMDGROUPOUT = 23;
    static final int SOTXTTITLE = 24;

    static final int SOFIRSTARRANGECMD = 25;
    static final int SOSECARRANGECMD = 26;
    static final int SOTHIRDARRANGECMD = 27;
    static final int SOFOURTHARRANGECMD = 28;
    static final int[] SOARRANGECMD = new int[]{SOFIRSTARRANGECMD, SOSECARRANGECMD, SOTHIRDARRANGECMD, SOFOURTHARRANGECMD};

    static final int SOOPTLANDSCAPE = 29;
    static final int SOOPTPORTRAIT = 30;
    static final int SOLAYOUTLST = 31;

    static int MaxSortIndex = -1;
    static String[] sDatabaseList;
    static String[] TableNames;
    static String[] QueryNames;
    static String[][] LayoutFiles = new String[2][];
    static XListBox xDBListBox;
    static XListBox xTableListBox;
    static XListBox xFieldsListBox;
    static XListBox xSelFieldsListBox;
    static XListBox[] xSortListBox = new XListBox[4];
    static XListBox xGroupListBox;
    static XListBox xLayoutListBox;
    static XTextComponent xTitleTextBox;
    static XTablesSupplier xDBTables;
    static XNameAccess xTableNames;
    static XNameAccess xQueryNames;
    static String TableName;
    static Object oDBTable;
    static int[] iCommandTypes;
    static int[][] WidthList;
    static String[][] SortFieldNames;
    static String[] GroupFieldNames;
    static boolean bEnableBinaryOptionGroup;
    static String[] OriginalList = new String[]{""};
    static XNameAccess xColumns;
    static XMultiServiceFactory  xMSFDialogModel;
    static XControlContainer xDialogContainer;
    static XDesktop xDesktop;
    static XTextDocument xReportDocument;
    static java.util.Vector GroupFieldVector;
    static String CurGroupName;
    static String TemplatePath;
    static boolean bGroupByIsSupported;



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
               System.err.println( exception);
               iKey = 2000;
          }
          return iKey;
    }


    private static void getSelectedDBMetaData()
        throws com.sun.star.container.NoSuchElementException, com.sun.star.beans.UnknownPropertyException,
               com.sun.star.lang.IllegalArgumentException, com.sun.star.lang.WrappedTargetException{
        try{
            short DBIndex = xDBListBox.getSelectedItemPos();
            String sDBName = sDatabaseList[DBIndex];
            if (DBMetaData.getConnection(sDBName) == true){
                if (DBMetaData.getDBMetaData() == true){
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


    private static void FillUpFieldsListbox(boolean bgetCommandType){
        try{
    if (xTableListBox.getSelectedItems().length > 0) {
           int CurCommandType;
           String TableName;
           int SelIndex = xTableListBox.getSelectedItemPos();
           if (bgetCommandType){
               CurCommandType = iCommandTypes[SelIndex];
               if (CurCommandType == com.sun.star.sdb.CommandType.QUERY){
                    int QueryIndex = SelIndex - TableNames.length - 1;
                    TableName = QueryNames[QueryIndex];
                    oDBTable = xQueryNames.getByName(TableName);
                }
                else{
                    TableName = TableNames[SelIndex];
                    oDBTable = xTableNames.getByName(TableName);
                }
           }
           XColumnsSupplier xTableCols = (XColumnsSupplier) UnoRuntime.queryInterface(XColumnsSupplier.class, oDBTable);
           xColumns = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, xTableCols.getColumns());
           if (DBMetaData.getSpecificFieldNames() > 0)
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
        //XControl xControl = (XControl) UnoRuntime.queryInterface(XControl.class, EventObject);
        String TitleName = xTitleTextBox.getText();
        ReportDocument.updateReportTitle(xReportDocument, xTitleTextBox);
    }

    public void disposing(com.sun.star.lang.EventObject EventObject) {
        }
    }



   static class ItemListenerImpl implements com.sun.star.awt.XItemListener{

        // XEventListener
       public void itemStateChanged(ItemEvent EventObject ){
       try{
            int iKey  =  getControlKey(EventObject.Source);
            switch (iKey) {
              case SODBLST:
                 getSelectedDBMetaData();
                 break;
              case SOTBLLST:
                 FillUpFieldsListbox(true);
                 TableName = xTableListBox.getSelectedItem();
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
                break;

             case SOTHIRDSORTLST:
                enableNextSortListBox(2);
                break;

             case SOFOURTHSORTLST:
                MaxSortIndex = 3;
                break;

             case SOGROUPLST:
                  UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "cmdGroupOut", "Enabled", new Boolean(xGroupListBox.getSelectedItems().length > 0));
               break;

         case SOLAYOUTLST:
        int iPos = xLayoutListBox.getSelectedItemPos();
        ReportDocument.loadStyleTemplates(xReportDocument, LayoutFiles[0][iPos], "LoadPageStyles");
          break;

            default:
//               System.err.println( exception);
               break;
           }
        }
        catch( Exception exception ){
            System.err.println( exception);
        }
     }

        public void disposing(EventObject eventObject) {
          System.out.println( getClass().getName() + ".disposing" + eventObject);
        }

    }


    static class ActionListenerImpl implements com.sun.star.awt.XActionListener {

        // XEventListener
        public void disposing(EventObject eventObject) {
        System.out.println( getClass().getName() + ".disposing:" + eventObject);
        }

        public void actionPerformed(ActionEvent actionEvent) {
            try{
                int iKey  =  getControlKey(actionEvent.Source);
                switch (iKey) {
                    case SOFLDSLST:
                       UNODialogs.MoveOrderedSelectedListBox(xDlgNameAccess, xFieldsListBox, xSelFieldsListBox, CurMetaData.FieldNames,  false);
                       break;

                    case SOCMDMOVESEL:
                        UNODialogs.MoveSelectedListBox(xDlgNameAccess, xFieldsListBox, xSelFieldsListBox);
                        break;

                    case SOCMDMOVEALL:
                        UNODialogs.FormMoveAll(xDlgNameAccess, xFieldsListBox, xSelFieldsListBox, CurMetaData.FieldNames);
                        break;

                    case SOCMDREMOVESEL:
                        UNODialogs.MoveOrderedSelectedListBox(xDlgNameAccess, xFieldsListBox, xSelFieldsListBox, CurMetaData.FieldNames, false);
                        break;

                    case SOCMDREMOVEALL:
                        UNODialogs.MoveOrderedSelectedListBox(xDlgNameAccess, xFieldsListBox, xSelFieldsListBox, CurMetaData.FieldNames, true);
                        break;

                    case SOCMDGOON:
                        gotoNextStep();
                        break;

                    case SOCMDBACK:
                        gotoPrevStep();
                        break;

                    case SOFIRSTSORTCMD:
                       setSortButtonLabel(0);
                       break;

                    case SOSECSORTCMD:
                       setSortButtonLabel(1);
                       break;

                    case SOTHIRDSORTCMD:
                       setSortButtonLabel(2);
                       break;

                    case SOFOURTHSORTCMD:
                       setSortButtonLabel(3);
                       break;

                    case SOCMDGROUPOUT:
                       addGroupNametoDocument();
                       break;

                    case SOCMDGROUPIN:
                        removeGroupName();
                       break;

            case SOFIRSTARRANGECMD:
            xTextTable = ReportDocument.insertReportTextTable(CurMetaData, xReportDocument, CurMetaData.FieldNames);
            ReportDocument.addDBRecordstoReportDocumentTable(xReportDocument, xTextTable, xResultSet, CurMetaData.FieldNames);
                       break;

            case SOSECARRANGECMD:
            ReportDocument.addDBRecordstoReportDocument(xReportDocument, xResultSet, CurMetaData.FieldNames);
                       break;


                    case SOOPTLANDSCAPE:
                        ReportDocument.changePageOrientation(xReportDocument, true);
                        break;

                    case SOOPTPORTRAIT:
                        ReportDocument.changePageOrientation(xReportDocument, false);
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

//  public void disposing(com.sun.star.lang.EventObject eventObject) {
//  }

//  public void actionPerformed(com.sun.star.awt.ActionEvent actionEvent) {
//  }

    }


    public static void gotoNextStep(){
    try{
        int PageCount = 4;
        int iPage = ((Integer) tools.getUNOPropertyValue(oDialogModel, "Step")).intValue();
        switch (iPage){
            case 1:
                UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "cmdBack", "Enabled", new Boolean(true));
                CurMetaData.FieldNames = xSelFieldsListBox.getItems();
                fillSecondStep();
                break;
            case 2:
                GroupFieldNames = new String[GroupFieldVector.size()];
                GroupFieldVector.copyInto(GroupFieldNames);
        fillThirdStep();
                break;
            case 3:
                UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "cmdGoOn", "Label", "~Fertigstellen");
                setUpSortList();
        DBMetaData.setupWidthList();
                xResultSet = DBMetaData.combineSelectStatement(xDBConnection, xDBMetaData, TableName, CurMetaData.FieldNames, GroupFieldNames, SortFieldNames);
        ReportDocument.insertTextArrangement(xReportDocument, ReportFolderName + "Example.stw", CurMetaData, null);
        ReportDocument.setupRecordSection(xReportDocument, ReportFolderName + "parDefault.stw", CurMetaData);
        fillFourthStep();
        break;

        default:
                break;
        }
        if (iPage < PageCount)
            tools.setUNOPropertyValues(oDialogModel, new String[]{"Step", "Title"},
                                                          new Object[]{ new Integer(iPage + 1), new String("Seite " + new Integer(iPage+1).toString())});
    }
    catch( Exception exception ){
        exception.printStackTrace(System.out);
    }}


    public static void addGroupNametoDocument(){
    try{
      CurGroupName = xGroupListBox.getSelectedItem();
      UNODialogs.removeSelectedItems(xGroupListBox);
      GroupFieldVector.addElement(CurGroupName);
      UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "cmdGroupOut", "Enabled", new Boolean(xGroupListBox.getSelectedItems().length > 0));
      UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "cmdGroupIn", "Enabled", new Boolean(true));
      int GroupIndex = GroupFieldVector.size();
      ReportDocument.insertTextSection(xReportDocument, "GroupField" + GroupIndex, ReportFolderName + "parDefault.stw");
      ReportDocument.replaceFieldValueInGroupTable(xReportDocument, "Tbl_GroupField" + GroupIndex, CurGroupName, GroupIndex);
    }
      catch( Exception exception ){
         exception.printStackTrace(System.out);
    }}


    public static void removeGroupName(){
    try{
      CurGroupName = (String) GroupFieldVector.lastElement();
      GroupFieldVector.removeElement(CurGroupName);
      xGroupListBox.addItem(CurGroupName, (short) xGroupListBox.getItemCount());
      UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "cmdGroupOut", "Enabled", new Boolean(xGroupListBox.getSelectedItems().length > 0));
      UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "cmdGroupIn", "Enabled", new Boolean(GroupFieldVector.isEmpty() == false));
      ReportDocument.removeLastTextSection(xReportDocument);
      ReportDocument.removeLastTextTable(xReportDocument);
    }
      catch( Exception exception ){
         exception.printStackTrace(System.out);
    }}


    public static void gotoPrevStep(){
    try{
        int iPage = ((Integer) tools.getUNOPropertyValue(oDialogModel, "Step")).intValue();
        switch (iPage){
            case 1:
                break;
            case 2:
                break;
            case 3:
        break;
        case 4:
                UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "cmdGoOn", "Label", "~Weiter");
                fillThirdStep();
                break;
            default:
                break;
        }
        UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "cmdBack", "Enabled", new Boolean(iPage > 2));

        if (iPage > 1)
            tools.setUNOPropertyValues(oDialogModel, new String[]{"Step", "Title"},
                       new Object[]{ new Integer(iPage - 1), new String("Seite " + new Integer(iPage-1).toString())});
    }
      catch( Exception exception ){
         System.err.println( exception);
    }}


    public static void setSortButtonLabel(int iKey){
    try{
        String sTag = (String) UNODialogs.getPropertyOfDialogControl(xDlgNameAccess, "cmdSort_" + new Integer(iKey+1).toString(), "Tag");
        if (sTag.compareTo("ASC") == 0){
            UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "cmdSort_" + new Integer(iKey + 1).toString(), "Label", "ZA");
            UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "cmdSort_" + new Integer(iKey + 1).toString(), "Tag", "DESC");
        }
        else{
            UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "cmdSort_" + new Integer(iKey + 1).toString(), "Label", "AZ");
            UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "cmdSort_" + new Integer(iKey + 1).toString(), "Tag", "ASC");
        }
    }
    catch( Exception exception ){
        System.err.println( exception);
    }}


    public static void setUpSortList(){
    try{
        SortFieldNames = new String[MaxSortIndex+1][2];
        for (int i=0;i<=MaxSortIndex;i++){
            SortFieldNames[i][0] = xSortListBox[i].getSelectedItem();
            SortFieldNames[i][1] = (String) UNODialogs.getPropertyOfDialogControl(xDlgNameAccess, "cmdSort_" + new Integer(i+1).toString(), "Tag");
        }
    }
    catch( Exception exception ){
        System.err.println( exception);
    }}


    public static void enableNextSortListBox(int CurIndex){
    try{
        short iNextItemPos;
        if (CurIndex > MaxSortIndex)
            MaxSortIndex = CurIndex;
        boolean bDoEnable = (xSortListBox[CurIndex].getSelectedItemPos()!= 0);
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
         System.err.println( exception);
    }}


     public static void toggleSortListBox(int CurIndex, boolean bDoEnable){
     try{
        UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "lstSort_" + new Integer(CurIndex + 1).toString(), "Enabled", new Boolean(bDoEnable));
        UNODialogs.AssignPropertyToDialogControl(xDlgNameAccess, "cmdSort_" + new Integer(CurIndex + 1).toString(), "Enabled", new Boolean(bDoEnable));
        if (bDoEnable == false)
            xSortListBox[CurIndex].selectItemPos((short)0,true);
     }
     catch( Exception exception ){
         System.err.println( exception);
     }}


    public static void fillFourthStep(){
    try{

        InsertControlModel("com.sun.star.awt.UnoControlFixedTextModel", xMSFDialogModel, xDlgNames, "lblTitle",
                            new String[] {"Height", "PositionX", "PositionY", "Step", "Width", "Label"},
                            new Object[] {new Integer(8), new Integer(6), new Integer(41), new Integer(4), new Integer(40), "lblTitle"});

    xTitleTextBox = InsertTextField(xMSFDialogModel, xDlgNames, xDialogContainer, "txtTitle", SOTXTTITLE,
                            new String[] {"Height", "PositionX", "PositionY", "Step", "Width"},
                            new Object[] {new Integer(12), new Integer(35), new Integer(40), new Integer(4), new Integer(100)});

        InsertControlModel("com.sun.star.awt.UnoControlFixedTextModel", xMSFDialogModel, xDlgNames, "lblArrange",
                            new String[] {"Height", "PositionX", "PositionY", "Step", "Width", "Label"},
                            new Object[] {new Integer(8), new Integer(6), new Integer(58), new Integer(4), new Integer(40), "lblArrange"});

    int XPos = 12;
        for (int i = 0; i < 4;i++){
             InsertButton(xMSFDialogModel, xDlgNames, xDialogContainer, "cmdArrange_" + new Integer(i+1).toString(), SOARRANGECMD[i],
                  new String[] {"Height", "PositionX", "PositionY", "Step", "Width"},
                  new Object[] {new Integer(25), new Integer(XPos), new Integer(72), new Integer(4), new Integer(23)});
             XPos = XPos + 29;
        }

    InsertControlModel("com.sun.star.awt.UnoControlFixedTextModel", xMSFDialogModel, xDlgNames, "lblOrientation",
                            new String[] {"Height", "PositionX", "PositionY", "Step", "Width", "Label"},
                            new Object[] {new Integer(8), new Integer(6), new Integer(103), new Integer(4), new Integer(74), "lblOrientation"});

        InsertRadioButton(xMSFDialogModel, xDlgNames, xDialogContainer, "optLandscape", SOOPTLANDSCAPE,
                            new String[] {"Height", "Label", "PositionX", "PositionY", "Step", "Width"},
                            new Object[] {new Integer(10), "Horizontale Orientierung", new Integer(12), new Integer(117), new Integer(4), new Integer(100)});

        InsertRadioButton(xMSFDialogModel, xDlgNames, xDialogContainer, "optPortrait", SOOPTPORTRAIT,
                            new String[] {"Height", "Label", "PositionX", "PositionY", "State", "Step", "Width"},
                            new Object[] {new Integer(10), "Vertikale Orientierung", new Integer(12), new Integer(131), new Short((short) 1), new Integer(4), new Integer(100)});

    InsertControlModel("com.sun.star.awt.UnoControlFixedTextModel", xMSFDialogModel, xDlgNames, "lblLayout",
            new String[] {"Height", "PositionX", "PositionY", "Step", "Width", "Label"},
            new Object[] {new Integer(8), new Integer(155), new Integer(40), new Integer(4), new Integer(109), "lblLayout"});
    LayoutFiles = tools.getFolderTitles(xMSF);
    xLayoutListBox = InsertListbox(xMSFDialogModel, xDlgNames, xDialogContainer, "lstLayout", SOLAYOUTLST,
                    new String[] {"Height", "PositionX", "PositionY", "Step", "StringItemList", "Width"},
                            new Object[] {new Integer(133), new Integer(155), new Integer(51), new Integer(4), LayoutFiles[1], new Integer(109)});

    }
    catch( Exception exception ){
        exception.printStackTrace(System.out);
    }}


    public static void fillThirdStep(){
    try{
        boolean bDoEnable;
        int YPos = 60;
        int FieldCount = CurMetaData.FieldNames.length;
    int SortFieldCount = FieldCount + 1-GroupFieldNames.length;
        String SortFieldNames[] = new String[SortFieldCount];
        SortFieldNames[0] = "(Keine)";
        String CurFieldName;
    int a = 1;
    for (int i = 0; i < FieldCount;i++){
        CurFieldName = CurMetaData.FieldNames[i];
        if (tools.FieldInList(GroupFieldNames, CurFieldName) == -1){
        SortFieldNames[a] = CurFieldName;
        a +=1;
        }
    }
    for (int i = 0; i<4; i++){
            bDoEnable = (i == 0);
            xSortListBox[i] = InsertListbox(xMSFDialogModel, xDlgNames, xDialogContainer, "lstSort_" + new Integer(i+1).toString(), SOSORTLST[i],
                           new String[] {"Enabled", "Height", "PositionX", "PositionY", "Step", "StringItemList", "Width", "Dropdown","LineCount", "Name"},
                           new Object[] {new Boolean(bDoEnable), new Integer(12), new Integer(144), new Integer(YPos), new Integer(3), SortFieldNames, new Integer(92), new Boolean(true), new Short("5"), "lstSort_" + new Integer(i+1)});
        InsertButton(xMSFDialogModel, xDlgNames, xDialogContainer, "cmdSort_" + new Integer(i+1).toString(), SOSORTCMD[i],
                 new String[] {"Enabled", "Height", "Label", "PositionX", "PositionY", "Step", "Tag", "Width"},
                 new Object[] {new Boolean(bDoEnable), new Integer(14), "AZ", new Integer(246), new Integer(YPos-1), new Integer(3), "ASC", new Integer(14)});
            YPos = YPos + 25;

        InsertControlModel("com.sun.star.awt.UnoControlFixedTextModel", xMSFDialogModel, xDlgNames, "lblSort",
                new String[] {"Height", "Label", "PositionX", "PositionY", "Step", "Width"},
                new Object[] {new Integer(8), "lblTables", new Integer(144), new Integer(40), new Integer(3), new Integer(72)});
    }
    }
    catch( Exception exception ){
        exception.printStackTrace(System.out);
    }}



    public static void fillSecondStep(){
    try{

    xGroupListBox = InsertListbox(xMSFDialogModel, xDlgNames, xDialogContainer, "lstGroup", SOGROUPLST,
                    new String[] {"Enabled", "Height", "PositionX", "PositionY", "Step", "StringItemList", "Width", "MultiSelection"},
                            new Object[] {new Boolean(bGroupByIsSupported), new Integer(133), new Integer(6), new Integer(51), new Integer(2), CurMetaData.FieldNames, new Integer(80), new Boolean(true)});

       InsertControlModel("com.sun.star.awt.UnoControlFixedTextModel", xMSFDialogModel, xDlgNames, "lblGroups",
            new String[] {"Enabled", "Height", "PositionX", "PositionY", "Step", "Width", "Label"},
            new Object[] {new Boolean(bGroupByIsSupported), new Integer(8), new Integer(6), new Integer(40), new Integer(2), new Integer(74), "lblDatabases"});
       InsertButton(xMSFDialogModel, xDlgNames, xDialogContainer, "cmdGroupOut", SOCMDGROUPOUT,
            new String[] {"Enabled", "Height", "PositionX", "PositionY", "Step", "Width", "Label"},
            new Object[] {new Boolean(false), new Integer(14), new Integer(92), new Integer(93), new Integer(2), new Integer(14),">>"});
       InsertButton(xMSFDialogModel, xDlgNames, xDialogContainer, "cmdGroupIn", SOCMDGROUPIN,
            new String[] {"Enabled", "Height", "PositionX", "PositionY", "Step", "Width", "Label"},
            new Object[] {new Boolean(false), new Integer(14), new Integer(92), new Integer(118), new Integer(2), new Integer(14), "<<"});
            GroupFieldVector = new java.util.Vector(CurMetaData.FieldNames.length);
    }
    catch( Exception exception ){
        exception.printStackTrace(System.out);
    }}


    public static void main (String args[]) {

        String[] ConnectStr;
        ConnectStr = new String[1];
// connecting synchronously with the office
//        String connectStr = "uno:socket,host=localhost,port=8100;urp,negotiate=0,forcesynchronous=1;StarOffice.NamingService";
            ConnectStr[0] = "uno:socket,host=localhost,port=8100;urp;StarOffice.ServiceManager";
//        ConnectStr[1] = "uno:socket,host=localhost,port=8100;urp;StarOffice.NamingService;StarOffice.NamingService";     //ServiceManager

        xMSF = null;
        try {
            xMSF = connect( ConnectStr[0] );
            if( xMSF != null )  System.out.println("Connected to "+ ConnectStr );
        iStart = System.currentTimeMillis();
        xDesktop = tools.getDesktop( xMSF );
        xReportDocument = (XTextDocument) tools.createNewDocument(xDesktop, "swriter");
        ReportDocument.initializeReportDocument(xReportDocument);
            ShowDialog();
        }
        catch(Exception e) {
            e.printStackTrace(System.out);
            System.exit( 0 );
        }
    }


    public static void ShowDialog()
        {
            try
            {
                long iStart = System.currentTimeMillis();
                DBMetaData.InitializeWidthList();

                ControlList = new Hashtable();

                XPropertySet xPropertySetMultiComponentFactory = ( XPropertySet ) UnoRuntime.queryInterface( XPropertySet.class, xMultiComponentFactory );

                // Get the default context from the office server.
                Object objectDefaultContext = xPropertySetMultiComponentFactory.getPropertyValue( "DefaultContext" );

                // Query for the interface XComponentContext.
                xcomponentcontext = ( XComponentContext ) UnoRuntime.queryInterface(XComponentContext.class, objectDefaultContext );

                // create a dialog model
                oDialogModel =  xMultiComponentFactory.createInstanceWithContext("com.sun.star.awt.UnoControlDialogModel", xcomponentcontext );

                XMultiPropertySet xMultiPSetDlg = (XMultiPropertySet) UnoRuntime.queryInterface(XMultiPropertySet.class, oDialogModel);
                xMultiPSetDlg.setPropertyValues(new String[] {"Height", "PositionX", "PositionY", "Step", "Title", "Width"},
                                                new Object[] {new Integer(210), new Integer(16), new Integer(28), new Integer(1), "DlgReportDB",new Integer(270)});

                // get service manager from dialog model
        xMSFDialogModel = (XMultiServiceFactory) UnoRuntime.queryInterface(XMultiServiceFactory.class, oDialogModel);

                // create dialog control
                Object objectDialog = xMultiComponentFactory.createInstanceWithContext("com.sun.star.awt.UnoControlDialog", xcomponentcontext );

                // set dialog model at dialog control
                XControl xControl = ( XControl ) UnoRuntime.queryInterface( XControl.class, objectDialog );
                XControlModel xControlModel = ( XControlModel ) UnoRuntime.queryInterface( XControlModel.class, oDialogModel );
                xControl.setModel( xControlModel );

                xDialogContainer = ( XControlContainer ) UnoRuntime.queryInterface( XControlContainer.class, objectDialog );
                xDlgNames = (XNameContainer) UnoRuntime.queryInterface( XNameContainer.class, oDialogModel );
                xDlgNameAccess = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, oDialogModel);
            InsertButton(xMSFDialogModel, xDlgNames, xDialogContainer, "cmdCancel", SOCMDCANCEL,
                            new String[] {"Height", "PositionX", "PositionY", "PushButtonType", "Step", "Width", "Label"},
                            new Object[] {new Integer(14), new Integer(6), new Integer(190), new Short((short)com.sun.star.awt.PushButtonType.CANCEL_value), new Integer(0), new Integer(53), "~Cancel"});

        InsertButton(xMSFDialogModel, xDlgNames, xDialogContainer, "cmdHelp", SOCMDHELP,
                            new String[] {"Height", "PositionX", "PositionY", "Step", "Width", "Label"},
                            new Object[] {new Integer(14), new Integer(63), new Integer(190), new Integer(0), new Integer(53), "~Help"});

                InsertButton(xMSFDialogModel, xDlgNames, xDialogContainer, "cmdBack", SOCMDBACK,
                            new String[] {"Enabled", "Height", "PositionX", "PositionY", "Step", "Width", "Label"},
                            new Object[] {new Boolean(false), new Integer(14), new Integer(155), new Integer(190), new Integer(0), new Integer(53),"~Back"});

                InsertButton(xMSFDialogModel, xDlgNames, xDialogContainer, "cmdGoOn", SOCMDGOON,
                            new String[] {"Height", "PositionX", "PositionY", "Step", "Width", "Label"},
                            new Object[] {new Integer(14), new Integer(211), new Integer(190), new Integer(0), new Integer(53),"~GoOn"});

        InsertButton(xMSFDialogModel, xDlgNames, xDialogContainer, "cmdMoveSelected", SOCMDMOVESEL,
                            new String[] {"Enabled", "Height", "PositionX", "PositionY", "Step", "Width", "Label"},
                            new Object[] {new Boolean(false), new Integer(14), new Integer(122), new Integer(88), new Integer(1), new Integer(25),"->"});

                InsertButton(xMSFDialogModel, xDlgNames, xDialogContainer, "cmdMoveAll", SOCMDMOVEALL,
                            new String[] {"Enabled", "Height", "PositionX", "PositionY", "Step", "Width", "Label"},
                            new Object[] {new Boolean(false), new Integer(14), new Integer(122), new Integer(113), new Integer(1), new Integer(25),"=>>"});

        InsertButton(xMSFDialogModel, xDlgNames, xDialogContainer, "cmdRemoveSelected", SOCMDREMOVESEL,
                            new String[] {"Enabled", "Height", "PositionX", "PositionY", "Step", "Width", "Label"},
                            new Object[] {new Boolean(false), new Integer(14), new Integer(122), new Integer(138), new Integer(1), new Integer(25), "<-"});

                InsertButton(xMSFDialogModel, xDlgNames, xDialogContainer, "cmdRemoveAll", SOCMDREMOVEALL,
                            new String[] {"Enabled", "Height", "PositionX", "PositionY", "Step", "Width", "Label"},
                            new Object[] {new Boolean(false), new Integer(14), new Integer(122), new Integer(163), new Integer(1), new Integer(25), "<<="});

                InsertControlModel("com.sun.star.awt.UnoControlFixedTextModel", xMSFDialogModel, xDlgNames, "lblDatabases",
                            new String[] {"Height", "PositionX", "PositionY", "Step", "Width", "Label"},
                            new Object[] {new Integer(8), new Integer(6), new Integer(40), new Integer(1), new Integer(74), "lblDatabases"});

        InsertControlModel("com.sun.star.awt.UnoControlFixedTextModel", xMSFDialogModel, xDlgNames, "lblTables",
                            new String[] {"Enabled", "Height", "Label", "PositionX", "PositionY", "Step", "Width"},
                            new Object[] {new Boolean(false), new Integer(8), "lblTables", new Integer(122), new Integer(40), new Integer(1), new Integer(72)});

                InsertControlModel("com.sun.star.awt.UnoControlFixedTextModel", xMSFDialogModel, xDlgNames, "lblFields",
                            new String[] {"Enabled", "Height", "PositionX", "PositionY", "Step", "Width", "Label"},
                            new Object[] {new Boolean(false), new Integer(8), new Integer(6), new Integer(70), new Integer(1), new Integer(109), "lblFields"});

                InsertControlModel("com.sun.star.awt.UnoControlFixedTextModel", xMSFDialogModel, xDlgNames, "lblSelFields",
                            new String[] {"Enabled", "Height", "PositionX", "PositionY", "Step", "Width", "Label"},
                            new Object[] {new Boolean(false), new Integer(8), new Integer(154), new Integer(70), new Integer(1), new Integer(110), "lblSelFields"});

                InsertControlModel("com.sun.star.awt.UnoControlImageControlModel", xMSFDialogModel, xDlgNames, "imgTheme",
                            new String[] {"Height", "PositionX", "PositionY", "Step", "Width"},
                            new Object[] {new Integer(26), new Integer(6), new Integer(6), new Integer(0), new Integer(258)});

                xDBListBox = InsertListbox(xMSFDialogModel, xDlgNames, xDialogContainer, "lstDatabases", SODBLST,
                            new String[] {"Height", "PositionX", "PositionY", "Step", "StringItemList", "Width", "Dropdown","LineCount", "Name"},
                            new Object[] {new Integer(12), new Integer(6), new Integer(51), new Integer(1), DBMetaData.getDatabaseNames(), new Integer(110), new Boolean(true), new Short("5"), "lstDatabases"});
        xTableListBox = InsertListbox(xMSFDialogModel, xDlgNames, xDialogContainer, "lstTables", SOTBLLST,
                            new String[] {"Enabled", "Height", "PositionX", "PositionY", "Step", "Width", "Dropdown", "LineCount"},
                            new Object[] {new Boolean(false), new Integer(12), new Integer(122), new Integer(51), new Integer(1), new Integer(110), new Boolean(true), new Short("5")});
        xFieldsListBox = InsertListbox(xMSFDialogModel, xDlgNames, xDialogContainer, "lstFields", SOFLDSLST,
                new String[] {"Enabled", "Height", "PositionX", "PositionY", "Step", "Width", "MultiSelection"},
                            new Object[] {new Boolean(false), new Integer(103), new Integer(6), new Integer(81), new Integer(1), new Integer(110), new Boolean(true)});
        xSelFieldsListBox = InsertListbox(xMSFDialogModel, xDlgNames, xDialogContainer, "lstSelFields", SOSELFLDSLST,
                            new String[] {"Enabled", "Height", "PositionX", "PositionY", "Step", "Width", "MultiSelection"},
                            new Object[] {new Boolean(false), new Integer(103), new Integer(154), new Integer(81), new Integer(1), new Integer(110), new Boolean(true)});
                XWindow xWindow = ( XWindow ) UnoRuntime.queryInterface( XWindow.class, objectDialog );
                xWindow.setVisible( false );
                XDialog xDialog = ( XDialog ) UnoRuntime.queryInterface( XDialog.class, objectDialog );
                Object objectToolkit =  xMultiComponentFactory.createInstanceWithContext("com.sun.star.awt.ExtToolkit", xcomponentcontext);
                XToolkit xToolkit = ( XToolkit ) UnoRuntime.queryInterface( XToolkit.class, objectToolkit );
                xControl.createPeer( xToolkit, null );

                long iEnd = System.currentTimeMillis();
                long iTime = (long) (iEnd - iStart);
                System.out.println("Die Ausführung dauerte " + iTime + " Millisekunden");
                short retvalue = xDialog.execute();
                switch (retvalue){
                    case 0:
                        break;
                    case 1:
                        break;
                }
                // dispose dialog
                XComponent xComponent = ( XComponent ) UnoRuntime.queryInterface( XComponent.class, objectDialog );
                xComponent.dispose();

                System.exit(0);
            }
            catch( Exception exception )
            {
                System.err.println( exception);
            }
        }

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

        // Get component context
        XComponentContext xcomponentcontext =   com.sun.star.comp.helper.Bootstrap.createInitialComponentContext( null );

        // initial serviceManager
        xMultiComponentFactory =  xcomponentcontext.getServiceManager();

        // create a connector, so that it can contact the office
        Object  xUrlResolver  = xMultiComponentFactory.createInstanceWithContext("com.sun.star.bridge.UnoUrlResolver", xcomponentcontext );
        XUnoUrlResolver urlResolver = (XUnoUrlResolver)UnoRuntime.queryInterface( XUnoUrlResolver.class, xUrlResolver );

        Object rInitialObject = urlResolver.resolve( connectStr );
        // Create a service manager from the initial object
        xMultiComponentFactory = ( XMultiComponentFactory ) UnoRuntime.queryInterface( XMultiComponentFactory.class, rInitialObject);

        xDatabaseContext = (Object) xMultiComponentFactory.createInstanceWithContext("com.sun.star.sdb.DatabaseContext",xcomponentcontext);

        rInitialObject = urlResolver.resolve("uno:socket,host=localhost,port=8100;urp;StarOffice.NamingService");

        XNamingService rName = (XNamingService)UnoRuntime.queryInterface(XNamingService.class, rInitialObject );

        XMultiServiceFactory xMSF = null;
        if( rName != null ) {
            System.err.println( "got the remote naming service !" );
            Object rXsmgr = rName.getRegisteredObject("StarOffice.ServiceManager" );

            xMSF = (XMultiServiceFactory) UnoRuntime.queryInterface( XMultiServiceFactory.class, rXsmgr );
        ReportFolderName = tools.getOfficePath(xMSF, "Template", "share") + "/wizard/report/";

        }

        return ( xMSF );
    }
}