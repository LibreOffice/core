/*************************************************************************
*
*  $RCSfile: QueryWizard.java,v $
*
*  $Revision: 1.2 $
*
*  last change: $Author: kz $ $Date: 2004-05-19 12:45:33 $
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

package com.sun.star.wizards.query;

import com.sun.star.frame.XFrame;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.awt.VclWindowPeerAttribute;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.beans.PropertyValue;
import com.sun.star.wizards.common.*;
import com.sun.star.wizards.db.*;
import com.sun.star.sdbc.SQLException;
import com.sun.star.uno.*;
import com.sun.star.wizards.ui.*;
import com.sun.star.wizards.ui.UIConsts;
import com.sun.star.uno.UnoRuntime;

public class QueryWizard extends WizardDialog {

    public XFrame CurFrame;
    public XWindowPeer xWindowPeer;
    public static final String SFILLUPFIELDSLISTBOX = "fillUpFieldsListbox";

    // TODO remove redundancy
    public static final int SOFIELDSELECTIONPAGE = 1;
    public static final int SOSORTINGPAGE = 2;
    public static final int SOFILTERPAGE = 3;
    public static final int SOAGGREGATEPAGE = 4;
    public static final int SOGROUPSELECTIONPAGE = 5;
    public static final int SOGROUPFILTERPAGE = 6;
    public static final int SOTITLESPAGE = 7;
    public static final int SOSUMMARYPAGE = 8;

    CommandFieldSelection CurDBCommandFieldSelection;
    SortingComponent CurSortingComponent;
    FieldSelection CurGroupFieldSelection;
    TitlesComponent CurTitlesComponent;
    FilterComponent CurFilterComponent;
    FilterComponent CurGroupFilterComponent;
    AggregateComponent CurAggregateComponent;
    Finalizer CurFinalizer;
    WizardDialog CurWizardDialog;
    QuerySummary CurDBMetaData;
    Object[] CurPropertyValue;
    String[][] UIRepresentation;
    String reslblFieldHeader;
    String reslblAliasHeader;
    String reslblFields;
    String reslblSelFields;
    String reslblTables;
    String resQuery;
    String resQueryWizard;
    String reslblGroupBy;
    String resmsgNonNumericAsGroupBy;

    //Resources Object
    short CurTabIndex = 0;

    public QueryWizard(XMultiServiceFactory xMSF) {
        super(xMSF, 40970);
        addResourceHandler("QueryWizard", "dbw");
        CurDBMetaData = new QuerySummary(xMSF, oResource);
    }

    public static void main(String args[]) {
        String ConnectStr = "uno:socket,host=localhost,port=8100;urp,negotiate=0,forcesynchronous=1;StarOffice.NamingService"; //  //localhost  ;Lo-1.Germany.sun.com; 10.16.65.155
        try {
            XMultiServiceFactory xLocMSF = Desktop.connect(ConnectStr);
            if (xLocMSF != null) {
                PropertyValue[] curproperties = new PropertyValue[1];
                curproperties[0] = Properties.createProperty("DataSourceName", "TESTDB");
                QueryWizard CurQueryWizard = new QueryWizard(xLocMSF);
                CurQueryWizard.startQueryWizard(xLocMSF, curproperties);
            }
        } catch (java.lang.Exception jexception) {
            jexception.printStackTrace(System.out);
        }
    }


    public void startQueryWizard(XMultiServiceFactory xMSF, PropertyValue[] CurPropertyValues) {
    try {
        if (CurDBMetaData.getConnection(CurPropertyValues)){
            reslblFields = oResource.getResText(UIConsts.RID_QUERY + 4);
            reslblFieldHeader = oResource.getResText(UIConsts.RID_QUERY + 19); //Fielnames in  AliasComponent
            reslblAliasHeader = oResource.getResText(UIConsts.RID_QUERY + 20); //Fieldtitles header in  AliasComponent
            reslblSelFields = oResource.getResText(UIConsts.RID_QUERY + 50);
            reslblTables = oResource.getResText(UIConsts.RID_QUERY + 3);
            reslblGroupBy =  oResource.getResText(UIConsts.RID_QUERY + 18);
            resQueryWizard = oResource.getResText(UIConsts.RID_QUERY + 2);
            resmsgNonNumericAsGroupBy = oResource.getResText(UIConsts.RID_QUERY + 88);
            Helper.setUnoPropertyValues(xDialogModel, new String[] { "Height", "Moveable", "Name", "PositionX", "PositionY", "Step", "TabIndex", "Title", "Width" },
                                                    new Object[] { new Integer(210), Boolean.TRUE, "DialogQuery", new Integer(102), new Integer(41), new Integer(1), new Short((short) 0), resQueryWizard, new Integer(310)});
            drawNaviBar();
            setRightPaneHeaders(oResource, UIConsts.RID_QUERY + 70, 8);
            this.setMaxStep(8);
            buildSteps();
            CurFrame = Desktop.getActiveFrame(xMSF);
            xWindowPeer = (XWindowPeer) UnoRuntime.queryInterface(XWindowPeer.class, CurFrame.getComponentWindow());
            this.xMSF = xMSF;
            createWindowPeer(xWindowPeer);
            insertQueryRelatedSteps();
            short RetValue = executeDialog(CurFrame.getComponentWindow().getPosSize());
        }
    } catch (java.lang.Exception jexception) {
        jexception.printStackTrace(System.out);
    }}


    public void enableRoadmapItems(String[] _FieldNames, boolean _bEnabled) {
        try {
            Object oRoadmapItem;
            int CurStep = AnyConverter.toInt(Helper.getUnoPropertyValue(xDialogModel, "Step"));
            boolean bEnabled = false;
            int CurItemID;
            for (int i = 0; i < getRMItemCount(); i++) {
                oRoadmapItem = this.xIndexContRoadmap.getByIndex(i);
                CurItemID = AnyConverter.toInt(Helper.getUnoPropertyValue(oRoadmapItem, "ID"));
                switch (CurItemID) {
                    case SOAGGREGATEPAGE :
                        if (_bEnabled == true)
                            bEnabled = ((CurDBMetaData.hasNumericalFields(_FieldNames)) && (CurDBMetaData.xDBMetaData.supportsCoreSQLGrammar()));
                        break;
                    case SOGROUPSELECTIONPAGE :
                        bEnabled = CurDBMetaData.Type == QueryMetaData.QueryType.SOSUMMARYQUERY;
                        break;
                    case SOGROUPFILTERPAGE :
                        bEnabled = false;
                        if (_bEnabled == true)
                            bEnabled = (CurDBMetaData.GroupByFilterConditions.length > 0);

                        break;
                    default :
                        if (CurItemID > CurStep)
                            bEnabled = _bEnabled;
                        else
                            bEnabled = true;
                        break;
                }
                super.setStepEnabled(CurItemID, bEnabled);
            }
        } catch (com.sun.star.uno.Exception exception) {
            exception.printStackTrace(System.out);
        }
    }

    public void insertQueryRelatedSteps() {
        try {
            String[] sRMItemLabels = getRMItemLabels();
            setRMItemLabels(oResource, UIConsts.RID_QUERY + 80);
            addRoadmap();
            int i = 0;
            i = insertRoadmapItem(0, true, SOFIELDSELECTIONPAGE - 1, SOFIELDSELECTIONPAGE);
            i = insertRoadmapItem(i, false, SOSORTINGPAGE - 1, SOSORTINGPAGE); // Orderby is always supported
            i = insertRoadmapItem(i, false, SOFILTERPAGE - 1, SOFILTERPAGE);
            if (CurDBMetaData.xDBMetaData.supportsCoreSQLGrammar())
                i = insertRoadmapItem(i, CurDBMetaData.hasNumericalFields(), SOAGGREGATEPAGE - 1, SOAGGREGATEPAGE);
            if (CurDBMetaData.xDBMetaData.supportsGroupBy()) {
                i = insertRoadmapItem(i, false, SOGROUPSELECTIONPAGE - 1, SOGROUPSELECTIONPAGE);
                i = insertRoadmapItem(i, false, SOGROUPFILTERPAGE - 1, SOGROUPFILTERPAGE);
            }
            //      if (CurDBMetaData.xDBMetaData.supportsColumnAliasing()) don't use -> too dangerous!!!
            i = insertRoadmapItem(i, false, SOTITLESPAGE - 1, SOTITLESPAGE);
            i = insertRoadmapItem(i, false, SOSUMMARYPAGE - 1, SOSUMMARYPAGE);
            setRoadmapInteractive(false);
            setRoadmapComplete(true);
            setCurrentRoadmapItemID((short) 1);
        } catch (com.sun.star.uno.Exception exception) {
            Resource.showCommonResourceError(xMSF);
        }
    }

    public void buildSteps() {
        try {
            boolean bEnabled;
            CurDBCommandFieldSelection = new CommandFieldSelection(this, CurDBMetaData, 120, reslblFields, reslblSelFields, reslblTables, false, 40850);
            CurDBCommandFieldSelection.addFieldSelectionListener(new FieldSelectionListener());
            CurSortingComponent = new SortingComponent(this, SOSORTINGPAGE, 95, 30, 210, 40865);
            CurFilterComponent = new FilterComponent(this, xMSF, SOFILTERPAGE, 97, 27, 209, 3, CurDBMetaData, 40878);
            CurFilterComponent.addNumberFormatsSupplier(CurDBMetaData.xNumberFormatsSupplier);
            int i = CurDBMetaData.xDBMetaData.getMaxTablesInSelect();
            if (CurDBMetaData.xDBMetaData.supportsCoreSQLGrammar())
                CurAggregateComponent = new AggregateComponent(this, CurDBMetaData, SOAGGREGATEPAGE, 97, 61, 209, 5, 40895);
            if (CurDBMetaData.xDBMetaData.supportsGroupBy()) {
                CurGroupFieldSelection = new FieldSelection(this, SOGROUPSELECTIONPAGE, 95, 27, 210, 150, reslblFields, this.reslblGroupBy, 40915, false);
                CurGroupFieldSelection.addFieldSelectionListener(new FieldSelectionListener());
                CurGroupFilterComponent = new FilterComponent(this, xMSF, SOGROUPFILTERPAGE, 97, 27, 209, 3, CurDBMetaData, 40925);
            }
            CurTitlesComponent = new TitlesComponent(this, SOTITLESPAGE, 97, 31, 210, 7, reslblFieldHeader, reslblAliasHeader, 40940);
            CurFinalizer = new Finalizer(this, CurDBMetaData);
            enableNavigationButtons(false, false, false);
        } catch (com.sun.star.uno.Exception exception){
            Resource.showCommonResourceError(xMSF);
        }
    }


    public void finishWizard() {
        int ncurStep = getCurrentStep();
        if ((switchToStep(ncurStep, SOSUMMARYPAGE)) || (ncurStep == SOSUMMARYPAGE))
            CurFinalizer.finish();
    }


    protected void enterStep(int nOldStep, int nNewStep) {
    try {
        if (nOldStep <= SOGROUPSELECTIONPAGE && nNewStep > SOGROUPSELECTIONPAGE) {
            if (CurDBMetaData.xDBMetaData.supportsGroupBy()) {
                CurDBMetaData.setGroupFieldNames(CurGroupFieldSelection.getSelectedFieldNames());
                CurDBMetaData.GroupFieldNames = JavaTools.removeOutdatedFields(CurDBMetaData.GroupFieldNames, CurDBMetaData.NonAggregateFieldNames);
                CurDBMetaData.GroupByFilterConditions = JavaTools.removeOutdatedFields(CurDBMetaData.GroupByFilterConditions, CurDBMetaData.GroupFieldNames);
            }
        }
        switch (nNewStep) {
            case SOFIELDSELECTIONPAGE :
                break;
            case SOSORTINGPAGE :
                CurSortingComponent.initialize(CurDBMetaData.FieldNames, CurDBMetaData.SortFieldNames);
                break;
            case SOFILTERPAGE :
                CurFilterComponent.initialize(CurDBMetaData.FilterConditions, CurDBMetaData.FieldNames);
                break;
            case SOAGGREGATEPAGE :
                CurAggregateComponent.initialize();
                break;
            case SOGROUPSELECTIONPAGE :
                break;
            case SOGROUPFILTERPAGE :
                CurGroupFilterComponent.initialize(CurDBMetaData.GroupByFilterConditions, CurDBMetaData.getGroupFieldNames());
                break;
            case SOTITLESPAGE :
                CurTitlesComponent.initialize(CurDBMetaData.FieldNames, CurDBMetaData.FieldTitleSet);
                break;
            case SOSUMMARYPAGE :
                CurFinalizer.initialize();
                break;
            default :
                break;
        }
    } catch (SQLException e) {
        e.printStackTrace(System.out);
    }}



    protected void leaveStep(int nOldStep, int nNewStep) {
        switch (nOldStep) {
            case SOFIELDSELECTIONPAGE :
                CurDBMetaData.setFieldNames(CurDBCommandFieldSelection.getSelectedFieldNames());
                CurDBMetaData.setAllIncludedFieldNames();
                CurDBMetaData.setFieldColumns(false);
                CurDBMetaData.setNumericFields();
                searchForOutdatedFields();
                break;
            case SOSORTINGPAGE :
                CurDBMetaData.SortFieldNames = CurSortingComponent.getSortFieldNames();
                break;
            case SOFILTERPAGE :
                CurDBMetaData.setFilterConditions(CurFilterComponent.getFilterConditions());
                break;
            case SOAGGREGATEPAGE :
                CurDBMetaData.AggregateFieldNames = CurAggregateComponent.getAggregateFieldNames();
                break;
            case SOGROUPSELECTIONPAGE :
                break;
            case SOGROUPFILTERPAGE :
                CurDBMetaData.setGroupByFilterConditions(this.CurGroupFilterComponent.getFilterConditions());
                break;
            case SOTITLESPAGE :
                CurDBMetaData.FieldTitleSet = CurTitlesComponent.getFieldTitles();
                CurDBMetaData.setfieldtitles();
                break;
            case SOSUMMARYPAGE :
                break;
            default :
                break;
        }
        if (nOldStep < SOGROUPSELECTIONPAGE && nNewStep >= SOGROUPSELECTIONPAGE){
            try {
                if (CurDBMetaData.Type == QueryMetaData.QueryType.SOSUMMARYQUERY){
                    if (CurDBMetaData.xDBMetaData.supportsGroupBy()) {
                        CurDBMetaData.setNonAggregateFieldNames();
                        CurGroupFieldSelection.initialize(CurDBMetaData.getUniqueAggregateFieldNames(), false, CurDBMetaData.xDBMetaData.getMaxColumnsInGroupBy());
                        CurGroupFieldSelection.intializeSelectedFields(CurDBMetaData.NonAggregateFieldNames);
                        CurGroupFieldSelection.setMultipleMode(false);
                        setStepEnabled(SOGROUPFILTERPAGE, CurDBMetaData.NonAggregateFieldNames.length > 0);
                    }
                }
            } catch (SQLException e) {
                e.printStackTrace();
            }
        }
    }


    private void searchForOutdatedFields() {
        String[] GroupCompNames;
        CurDBMetaData.SortFieldNames = JavaTools.removeOutdatedFields(CurDBMetaData.SortFieldNames, CurDBMetaData.FieldNames);
        CurDBMetaData.FilterConditions = JavaTools.removeOutdatedFields(CurDBMetaData.FilterConditions, CurDBMetaData.FieldNames);
        CurDBMetaData.AggregateFieldNames = JavaTools.removeOutdatedFields(CurDBMetaData.AggregateFieldNames, CurDBMetaData.FieldNames);
    }

    private void enableWizardSteps(String[] NewItems) {
        boolean bEnabled = NewItems.length > 0;
        setControlProperty("btnWizardNext", "Enabled", new Boolean(bEnabled));
        setControlProperty("btnWizardFinish", "Enabled", new Boolean(bEnabled));
        enableRoadmapItems(NewItems, bEnabled); // Note: Performancewise this could be improved
    }


    public class FieldSelectionListener implements com.sun.star.wizards.ui.XFieldSelectionListener {
        protected int ID;

        public int getID() {
            return ID;
        }

        public void setID(String sIncSuffix) {
            ID = 1;
            if (sIncSuffix != null) {
                if (sIncSuffix != "") {
                    String sID = JavaTools.ArrayoutofString(sIncSuffix, "_")[1];
                    ID = Integer.parseInt(sID);
                    int a = 0;
                }
            }
        }

        public void shiftFromLeftToRight(String[] SelItems, String[] NewItems) {
            if (ID == 1) {
                enableWizardSteps(NewItems);
                CurDBCommandFieldSelection.toggleCommandListBox(NewItems);
            } else {
                boolean bEnabled = (CurGroupFieldSelection.getSelectedFieldNames().length > 0);
                Helper.setUnoPropertyValue(getRoadmapItemByID(SOGROUPFILTERPAGE), "Enabled", new Boolean(bEnabled));
            }
        }

        public void shiftFromRightToLeft(String[] SelItems, String[] NewItems) {
            // TODO When the ListFieldbox is refilled only fields of the current Command may be merged into the Listbox
            if (ID == 1) {
                enableWizardSteps(NewItems);
                CurDBCommandFieldSelection.toggleCommandListBox(NewItems);
            } else {
                boolean bEnabled = (CurGroupFieldSelection.getSelectedFieldNames().length > 0);
                String CurFieldName = SelItems[0];
                if (JavaTools.FieldInList(CurDBMetaData.NonAggregateFieldNames, CurFieldName) > -1) {
                    SystemDialog.showMessageBox(xMSF, "ErrorBox", VclWindowPeerAttribute.OK, resmsgNonNumericAsGroupBy);
                    CurGroupFieldSelection.xSelFieldsListBox.addItems(SelItems, CurGroupFieldSelection.xSelFieldsListBox.getItemCount());
                    String FieldList[] = CurGroupFieldSelection.xFieldsListBox.getItems();
                    int index = JavaTools.FieldInList(FieldList, CurFieldName);
                    if (index > -1)
                        CurGroupFieldSelection.xFieldsListBox.removeItems((short) index, (short) 1);
                } else
                    Helper.setUnoPropertyValue(getRoadmapItemByID(SOGROUPFILTERPAGE), "Enabled", new Boolean(bEnabled));
            }
        }
        public void moveItemDown(String item) {
        }

        public void moveItemUp(String item) {
        }
    }
}