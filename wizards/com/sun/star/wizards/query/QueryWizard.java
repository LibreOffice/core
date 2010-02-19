/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: QueryWizard.java,v $
 * $Revision: 1.16 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
package com.sun.star.wizards.query;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.awt.VclWindowPeerAttribute;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XFrame;
import com.sun.star.sdb.CommandType;
import com.sun.star.sdbc.SQLException;
import com.sun.star.uno.AnyConverter;
import com.sun.star.wizards.ui.UIConsts;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.common.Resource;
import com.sun.star.wizards.db.DatabaseObjectWizard;
import com.sun.star.wizards.db.QueryMetaData;
import com.sun.star.wizards.ui.AggregateComponent;
import com.sun.star.wizards.ui.CommandFieldSelection;
import com.sun.star.wizards.ui.FieldSelection;
import com.sun.star.wizards.ui.FilterComponent;
import com.sun.star.wizards.ui.SortingComponent;
import com.sun.star.wizards.ui.TitlesComponent;

public class QueryWizard extends DatabaseObjectWizard
{
    public static final String SFILLUPFIELDSLISTBOX = "fillUpFieldsListbox";
    private static final int SOFIELDSELECTION_PAGE = 1;
    private static final int SOSORTING_PAGE = 2;
    private static final int SOFILTER_PAGE = 3;
    private static final int SOAGGREGATE_PAGE = 4;
    private static final int SOGROUPSELECTION_PAGE = 5;
    private static final int SOGROUPFILTER_PAGE = 6;
    private static final int SOTITLES_PAGE = 7;
    protected static final int SOSUMMARY_PAGE = 8;
    private CommandFieldSelection CurDBCommandFieldSelection;
    private SortingComponent CurSortingComponent;
    private FieldSelection CurGroupFieldSelection;
    private TitlesComponent CurTitlesComponent;
    private FilterComponent CurFilterComponent;
    private FilterComponent CurGroupFilterComponent;
    private AggregateComponent CurAggregateComponent;
    private Finalizer CurFinalizer;
    private QuerySummary CurDBMetaData;
    private String reslblFieldHeader;
    private String reslblAliasHeader;
    private String reslblFields;
    private String reslblSelFields;
    private String reslblTables;
    private String resQueryWizard;
    private String reslblGroupBy;
    private String resmsgNonNumericAsGroupBy;
    private String m_createdQuery;

    public QueryWizard( XMultiServiceFactory xMSF, PropertyValue[] i_wizardContext )
    {
        super( xMSF, 40970, i_wizardContext );
        addResourceHandler("QueryWizard", "dbw");
        CurDBMetaData = new QuerySummary(xMSF, m_oResource);
    }

/*    public static void main(String args[])
    {
        String ConnectStr = "uno:pipe,name=foo;urp;StarOffice.ServiceManager";
        try
        {
            XMultiServiceFactory xLocMSF = Desktop.connect(ConnectStr);
            if (xLocMSF != null)
            {
                PropertyValue[] curproperties = new PropertyValue[1];
                curproperties[0] = Properties.createProperty("DatabaseLocation", "file:///G:/temp/abc.odb");
//              curproperties[0] = Properties.createProperty("DataSourceName", "TESTDB");

                QueryWizard CurQueryWizard = new QueryWizard(xLocMSF);
                CurQueryWizard.startQueryWizard(xLocMSF, curproperties);
            }
        }
        catch (java.lang.Exception jexception)
        {
            jexception.printStackTrace(System.out);
        }
    }*/

    public final XFrame getFrame()
    {
        return m_frame;
    }

    public String startQueryWizard()
    {
        try
        {
            if ( CurDBMetaData.getConnection( m_wizardContext ) )
            {
                reslblFields = m_oResource.getResText(UIConsts.RID_QUERY + 4);
                reslblFieldHeader = m_oResource.getResText(UIConsts.RID_QUERY + 19); //Fielnames in  AliasComponent
                reslblAliasHeader = m_oResource.getResText(UIConsts.RID_QUERY + 20); //Fieldtitles header in  AliasComponent
                reslblSelFields = m_oResource.getResText(UIConsts.RID_QUERY + 50);
                reslblTables = m_oResource.getResText(UIConsts.RID_QUERY + 3);
                reslblGroupBy = m_oResource.getResText(UIConsts.RID_QUERY + 18);
                resQueryWizard = m_oResource.getResText(UIConsts.RID_QUERY + 2);
                resmsgNonNumericAsGroupBy = m_oResource.getResText(UIConsts.RID_QUERY + 88);
                Helper.setUnoPropertyValues(xDialogModel, new String[]
                        {
                            "Height", "Moveable", "Name", "PositionX", "PositionY", "Step", "TabIndex", "Title", "Width"
                        },
                        new Object[]
                        {
                            new Integer(210), Boolean.TRUE, "DialogQuery", new Integer(102), new Integer(41), new Integer(1), new Short((short) 0), resQueryWizard, new Integer(310)
                        });
                drawNaviBar();
                setRightPaneHeaders(m_oResource, UIConsts.RID_QUERY + 70, 8);
                this.setMaxStep(8);
                buildSteps();
                this.CurDBCommandFieldSelection.preselectCommand( m_wizardContext, false );

                XWindowPeer windowPeer = UnoRuntime.queryInterface( XWindowPeer.class, m_frame.getContainerWindow() );
                createWindowPeer(windowPeer);
                CurDBMetaData.setWindowPeer(this.xControl.getPeer());
                insertQueryRelatedSteps();
                executeDialog( m_frame.getContainerWindow().getPosSize() );
            }
        }
        catch (java.lang.Exception jexception)
        {
            jexception.printStackTrace(System.out);
        }
        CurGroupFilterComponent = null;
        CurTitlesComponent = null;
        CurAggregateComponent = null;
        CurDBCommandFieldSelection = null;
        xWindowPeer = null;
        CurFinalizer = null;
        CurDBMetaData.finish();
        CurDBMetaData = null;
        System.gc();

        return m_createdQuery;
    }

    public void enableRoadmapItems(String[] _FieldNames, boolean _bEnabled)
    {
        try
        {
            Object oRoadmapItem;
            int CurStep = AnyConverter.toInt(Helper.getUnoPropertyValue(xDialogModel, "Step"));
            boolean bEnabled = false;
            int CurItemID;
            for (int i = 0; i < getRMItemCount(); i++)
            {
                oRoadmapItem = this.xIndexContRoadmap.getByIndex(i);
                CurItemID = AnyConverter.toInt(Helper.getUnoPropertyValue(oRoadmapItem, "ID"));
                switch (CurItemID)
                {
                    case SOAGGREGATE_PAGE:
                        if (_bEnabled == true)
                        {
                            bEnabled = ((CurDBMetaData.hasNumericalFields()) && (CurDBMetaData.xDBMetaData.supportsCoreSQLGrammar()));
                        }
                        break;
                    case SOGROUPSELECTION_PAGE:
                        bEnabled = CurDBMetaData.Type == QueryMetaData.QueryType.SOSUMMARYQUERY;
                        break;
                    case SOGROUPFILTER_PAGE:
                        bEnabled = false;
                        if (_bEnabled == true)
                        {
                            bEnabled = (CurDBMetaData.GroupByFilterConditions.length > 0);
                        }

                        break;
                    default:
                        if (CurItemID > CurStep)
                        {
                            bEnabled = _bEnabled;
                        }
                        else
                        {
                            bEnabled = true;
                        }
                        break;
                }
                super.setStepEnabled(CurItemID, bEnabled);
            }
        }
        catch (com.sun.star.uno.Exception exception)
        {
            exception.printStackTrace(System.out);
        }
    }

    public void insertQueryRelatedSteps()
    {
        try
        {
            setRMItemLabels(m_oResource, UIConsts.RID_QUERY + 80);
            addRoadmap();
            int i = 0;
            i = insertRoadmapItem(0, true, SOFIELDSELECTION_PAGE - 1, SOFIELDSELECTION_PAGE);
            i = insertRoadmapItem(i, false, SOSORTING_PAGE - 1, SOSORTING_PAGE); // Orderby is always supported
            i = insertRoadmapItem(i, false, SOFILTER_PAGE - 1, SOFILTER_PAGE);
            if (CurDBMetaData.xDBMetaData.supportsCoreSQLGrammar())
            {
                i = insertRoadmapItem(i, CurDBMetaData.hasNumericalFields(), SOAGGREGATE_PAGE - 1, SOAGGREGATE_PAGE);
            }
            if (CurDBMetaData.xDBMetaData.supportsGroupBy())
            {
                i = insertRoadmapItem(i, false, SOGROUPSELECTION_PAGE - 1, SOGROUPSELECTION_PAGE);
                i = insertRoadmapItem(i, false, SOGROUPFILTER_PAGE - 1, SOGROUPFILTER_PAGE);
            }
            //      if (CurDBMetaData.xDBMetaData.supportsColumnAliasing()) don't use -> too dangerous!!!
            i = insertRoadmapItem(i, false, SOTITLES_PAGE - 1, SOTITLES_PAGE);
            i = insertRoadmapItem(i, false, SOSUMMARY_PAGE - 1, SOSUMMARY_PAGE);
            setRoadmapInteractive(true);
            setRoadmapComplete(true);
            setCurrentRoadmapItemID((short) 1);
        }
        catch (com.sun.star.uno.Exception exception)
        {
            Resource.showCommonResourceError(xMSF);
        }
    }

    public void buildSteps()
    {
        try
        {
//            curDBCommandFieldSelection = new CommandFieldSelection(this, curFormDocument.oMainFormDBMetaData, 92, slblFields, slblSelFields,  slblTables, true, 34411);
//            curDBCommandFieldSelection.addFieldSelectionListener(new FieldSelectionListener());

            CurDBCommandFieldSelection = new CommandFieldSelection(this, CurDBMetaData, 120, reslblFields, reslblSelFields, reslblTables, true, 40850);
            CurDBCommandFieldSelection.setAppendMode(true);
            CurDBCommandFieldSelection.addFieldSelectionListener(new FieldSelectionListener());
            CurSortingComponent = new SortingComponent(this, SOSORTING_PAGE, 95, 27, 210, 40865);
            CurFilterComponent = new FilterComponent(this, xMSF, SOFILTER_PAGE, 97, 27, 209, 3, CurDBMetaData, 40878);
            CurFilterComponent.addNumberFormats();

            if (CurDBMetaData.xDBMetaData.supportsCoreSQLGrammar())
            {
                CurAggregateComponent = new AggregateComponent(this, CurDBMetaData, SOAGGREGATE_PAGE, 97, 69, 209, 5, 40895);
            }
            if (CurDBMetaData.xDBMetaData.supportsGroupBy())
            {
                CurGroupFieldSelection = new FieldSelection(this, SOGROUPSELECTION_PAGE, 95, 27, 210, 150, reslblFields, this.reslblGroupBy, 40915, false);
                CurGroupFieldSelection.addFieldSelectionListener(new FieldSelectionListener());
                CurGroupFilterComponent = new FilterComponent(this, xMSF, SOGROUPFILTER_PAGE, 97, 27, 209, 3, CurDBMetaData, 40923);
            }
            CurTitlesComponent = new TitlesComponent(this, SOTITLES_PAGE, 97, 37, 207, 7, reslblFieldHeader, reslblAliasHeader, 40940);
            CurFinalizer = new Finalizer(this, CurDBMetaData);
            enableNavigationButtons(false, false, false);
        }
        catch (com.sun.star.uno.Exception exception)
        {
            Resource.showCommonResourceError(xMSF);
        }
    }

    public boolean finishWizard()
    {
        int ncurStep = getCurrentStep();
        if  (   ( ncurStep == SOSUMMARY_PAGE )
            ||  ( switchToStep( ncurStep, SOSUMMARY_PAGE ) )
            )
        {
            m_createdQuery = CurFinalizer.finish();
            if ( m_createdQuery.length() > 0 )
            {
                loadSubComponent( CommandType.QUERY, m_createdQuery, CurFinalizer.displayQueryDesign() );
                xDialog.endExecute();
                return true;
            }
        }
        return false;
    }

    protected void enterStep(int nOldStep, int nNewStep)
    {
        try
        {
            if (nOldStep <= SOGROUPSELECTION_PAGE && nNewStep > SOGROUPSELECTION_PAGE)
            {
                if (CurDBMetaData.xDBMetaData.supportsGroupBy())
                {
                    CurDBMetaData.setGroupFieldNames(CurGroupFieldSelection.getSelectedFieldNames());
                    CurDBMetaData.GroupFieldNames = JavaTools.removeOutdatedFields(CurDBMetaData.GroupFieldNames, CurDBMetaData.NonAggregateFieldNames);
                    CurDBMetaData.GroupByFilterConditions = JavaTools.removeOutdatedFields(CurDBMetaData.GroupByFilterConditions, CurDBMetaData.GroupFieldNames);
                }
            }
            switch (nNewStep)
            {
                case SOFIELDSELECTION_PAGE:
                    break;
                case SOSORTING_PAGE:
                    CurSortingComponent.initialize(CurDBMetaData.getDisplayFieldNames(), CurDBMetaData.getSortFieldNames());
                    break;
                case SOFILTER_PAGE:
                    CurFilterComponent.initialize(CurDBMetaData.getFilterConditions(), CurDBMetaData.getDisplayFieldNames());
                    break;
                case SOAGGREGATE_PAGE:
                    CurAggregateComponent.initialize();
                    break;
                case SOGROUPSELECTION_PAGE:
                    break;
                case SOGROUPFILTER_PAGE:
                    CurGroupFilterComponent.initialize(CurDBMetaData.GroupByFilterConditions, CurDBMetaData.getGroupFieldNames());
                    break;
                case SOTITLES_PAGE:
                    CurTitlesComponent.initialize(CurDBMetaData.getDisplayFieldNames(), CurDBMetaData.FieldTitleSet);
                    break;
                case SOSUMMARY_PAGE:
                    CurFinalizer.initialize();
                    break;
                default:
                    break;
            }
        }
        catch (SQLException e)
        {
            e.printStackTrace(System.out);
        }
    }

    protected void leaveStep(int nOldStep, int nNewStep)
    {
        switch (nOldStep)
        {
            case SOFIELDSELECTION_PAGE:
                CurDBMetaData.reorderFieldColumns(CurDBCommandFieldSelection.getSelectedFieldNames());
                CurDBMetaData.initializeFieldTitleSet(true);
                CurDBMetaData.setNumericFields();
                searchForOutdatedFields();
                break;
            case SOSORTING_PAGE:
                CurDBMetaData.setSortFieldNames(CurSortingComponent.getSortFieldNames());
                break;
            case SOFILTER_PAGE:
                CurDBMetaData.setFilterConditions(CurFilterComponent.getFilterConditions());
                break;
            case SOAGGREGATE_PAGE:
                CurDBMetaData.AggregateFieldNames = CurAggregateComponent.getAggregateFieldNames();
                break;
            case SOGROUPSELECTION_PAGE:
                break;
            case SOGROUPFILTER_PAGE:
                CurDBMetaData.setGroupByFilterConditions(this.CurGroupFilterComponent.getFilterConditions());
                break;
            case SOTITLES_PAGE:
                CurDBMetaData.setFieldTitles(CurTitlesComponent.getFieldTitles());
                break;
            case SOSUMMARY_PAGE:
                break;
            default:
                break;
        }
        if (nOldStep < SOGROUPSELECTION_PAGE && nNewStep >= SOGROUPSELECTION_PAGE)
        {
            try
            {
                if (CurDBMetaData.Type == QueryMetaData.QueryType.SOSUMMARYQUERY)
                {
                    if (CurDBMetaData.xDBMetaData.supportsGroupBy())
                    {
                        CurDBMetaData.setNonAggregateFieldNames();
                        CurGroupFieldSelection.initialize(CurDBMetaData.getUniqueAggregateFieldNames(), false, CurDBMetaData.xDBMetaData.getMaxColumnsInGroupBy());
                        CurGroupFieldSelection.intializeSelectedFields(CurDBMetaData.NonAggregateFieldNames);
                        CurGroupFieldSelection.setMultipleMode(false);
                        setStepEnabled(SOGROUPFILTER_PAGE, CurAggregateComponent.isGroupingpossible() && CurDBMetaData.NonAggregateFieldNames.length > 0);
                    }
                }
            }
            catch (SQLException e)
            {
                e.printStackTrace();
            }
        }
    }

    private void searchForOutdatedFields()
    {
        String[] sFieldNames = CurDBMetaData.getFieldNames();
        String[][] sRemovedFields = JavaTools.removeOutdatedFields(CurDBMetaData.getSortFieldNames(), sFieldNames);
        CurDBMetaData.setSortFieldNames(sRemovedFields);
        CurDBMetaData.setFilterConditions(JavaTools.removeOutdatedFields(CurDBMetaData.getFilterConditions(), sFieldNames));
        CurDBMetaData.AggregateFieldNames = JavaTools.removeOutdatedFields(CurDBMetaData.AggregateFieldNames, sFieldNames);
    }

    private void enableWizardSteps(String[] NewItems)
    {
        boolean bEnabled = NewItems.length > 0;
        setControlProperty("btnWizardNext", "Enabled", new Boolean(bEnabled));
        setControlProperty("btnWizardFinish", "Enabled", new Boolean(bEnabled));
        enableRoadmapItems(NewItems, bEnabled); // Note: Performancewise this could be improved
    }

    public class FieldSelectionListener implements com.sun.star.wizards.ui.XFieldSelectionListener
    {

        protected int ID;

        public int getID()
        {
            return ID;
        }

        public void setID(String sIncSuffix)
        {
            ID = 1;
            if (sIncSuffix != null)
            {
                if ((!sIncSuffix.equals("")) && (!sIncSuffix.equals("_")))
                {
                    String sID = JavaTools.ArrayoutofString(sIncSuffix, "_")[1];
                    ID = Integer.parseInt(sID);
                    int a = 0;
                }
            }
        }

        public void shiftFromLeftToRight(String[] SelItems, String[] NewItems)
        {
            if (ID == 1)
            {
                CurDBMetaData.addSeveralFieldColumns(SelItems, CurDBCommandFieldSelection.getSelectedCommandName());
                enableWizardSteps(NewItems);
                CurDBCommandFieldSelection.changeSelectedFieldNames(CurDBMetaData.getDisplayFieldNames());
                CurDBCommandFieldSelection.toggleCommandListBox(NewItems);
            }
            else
            {
                boolean bEnabled = (CurGroupFieldSelection.getSelectedFieldNames().length > 0);
                Helper.setUnoPropertyValue(getRoadmapItemByID(SOGROUPFILTER_PAGE), "Enabled", new Boolean(bEnabled));
            }
        }

        public void shiftFromRightToLeft(String[] SelItems, String[] NewItems)
        {
            // TODO When the ListFieldbox is refilled only fields of the current Command may be merged into the Listbox
            if (ID == 1)
            {
                enableWizardSteps(NewItems);
                String[] sSelfieldNames = CurDBMetaData.getFieldNames(SelItems, CurDBCommandFieldSelection.getSelectedCommandName());
                CurDBCommandFieldSelection.addItemsToFieldsListbox(sSelfieldNames);
                CurDBMetaData.removeSeveralFieldColumnsByDisplayFieldName(SelItems);
                CurDBCommandFieldSelection.toggleCommandListBox(NewItems);

            }
            else
            {
                boolean bEnabled = (CurGroupFieldSelection.getSelectedFieldNames().length > 0);
                String CurDisplayFieldName = SelItems[0];
                if (JavaTools.FieldInList(CurDBMetaData.NonAggregateFieldNames, CurDisplayFieldName) > -1)
                {
                    showMessageBox("ErrorBox", VclWindowPeerAttribute.OK, resmsgNonNumericAsGroupBy);
                    CurGroupFieldSelection.xSelectedFieldsListBox.addItems(SelItems, CurGroupFieldSelection.xSelectedFieldsListBox.getItemCount());
                    String FieldList[] = CurGroupFieldSelection.xFieldsListBox.getItems();
                    int index = JavaTools.FieldInList(FieldList, CurDisplayFieldName);
                    if (index > -1)
                    {
                        CurGroupFieldSelection.xFieldsListBox.removeItems((short) index, (short) 1);
                    }
                }
                else
                {
                    Helper.setUnoPropertyValue(getRoadmapItemByID(SOGROUPFILTER_PAGE), "Enabled", new Boolean(bEnabled));
                }
            }
        }

        public void moveItemDown(String item)
        {
        }

        public void moveItemUp(String item)
        {
        }
    }
}
