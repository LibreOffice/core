/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
package com.sun.star.wizards.query;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.awt.VclWindowPeerAttribute;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.beans.PropertyValue;
import com.sun.star.sdb.CommandType;
import com.sun.star.sdbc.SQLException;
import com.sun.star.uno.AnyConverter;
import com.sun.star.wizards.ui.UIConsts;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.common.PropertyNames;
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

    private static final int SOFIELDSELECTION_PAGE = 1;
    private static final int SOSORTING_PAGE = 2;
    private static final int SOFILTER_PAGE = 3;
    private static final int SOAGGREGATE_PAGE = 4;
    private static final int SOGROUPSELECTION_PAGE = 5;
    private static final int SOGROUPFILTER_PAGE = 6;
    private static final int SOTITLES_PAGE = 7;
    protected static final int SOSUMMARY_PAGE = 8;
    private CommandFieldSelection m_DBCommandFieldSelectio;
    private SortingComponent m_sortingComponent;
    private FieldSelection m_groupFieldSelection;
    private TitlesComponent m_titlesComponent;
    private FilterComponent m_filterComponent;
    private FilterComponent m_groupFilterComponent;
    private AggregateComponent m_aggregateComponent;
    private Finalizer m_finalizer;
    private QuerySummary m_DBMetaData;
    private String reslblFieldHeader;
    private String reslblAliasHeader;
    private String reslblFields;
    private String reslblSelFields;
    private String reslblTables;
    private String reslblGroupBy;
    private String resmsgNonNumericAsGroupBy;
    private String m_createdQuery;

    public QueryWizard(XMultiServiceFactory xMSF, PropertyValue[] i_wizardContext)
    {
        super(xMSF, 40970, i_wizardContext);
        addResourceHandler();
        m_DBMetaData = new QuerySummary(xMSF, m_oResource);
    }

    public static void main(String i_args[])
    {
        executeWizardFromCommandLine( i_args, new WizardFromCommandLineStarter() {
            public void start(XMultiServiceFactory factory, PropertyValue[] curproperties) {
                QueryWizard wizard = new QueryWizard(factory, curproperties);
                wizard.start();
            }
        });
    }

    public String start()
    {
        try
        {
            if (m_DBMetaData.getConnection(m_wizardContext))
            {
                reslblFields = m_oResource.getResText("RID_QUERY_4");
                reslblFieldHeader = m_oResource.getResText("RID_QUERY_19"); //Fielnames in  AliasComponent
                reslblAliasHeader = m_oResource.getResText("RID_QUERY_20"); //Fieldtitles header in  AliasComponent
                reslblSelFields = m_oResource.getResText("RID_QUERY_50");
                reslblTables = m_oResource.getResText("RID_QUERY_3");
                reslblGroupBy = m_oResource.getResText("RID_QUERY_18");
                String resQueryWizard = m_oResource.getResText("RID_QUERY_2");
                resmsgNonNumericAsGroupBy = m_oResource.getResText("RID_QUERY_88");
                Helper.setUnoPropertyValues(xDialogModel, new String[]
                        {
                            PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_MOVEABLE, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_TITLE, PropertyNames.PROPERTY_WIDTH
                        },
                        new Object[]
                        {
                            210, Boolean.TRUE, "DialogQuery", 102, 41, 1, Short.valueOf((short) 0), resQueryWizard, 310
                        });
                drawNaviBar();
                setRightPaneHeaders(m_oResource, "RID_QUERY_", 70, 8);
                this.setMaxStep(8);
                buildSteps();
                this.m_DBCommandFieldSelectio.preselectCommand(m_wizardContext, false);

                XWindowPeer windowPeer = UnoRuntime.queryInterface(XWindowPeer.class, m_frame.getContainerWindow());
                createWindowPeer(windowPeer);
                m_DBMetaData.setWindowPeer(this.xControl.getPeer());
                insertQueryRelatedSteps();
                executeDialog(m_frame.getContainerWindow().getPosSize());
            }
        }
        catch (java.lang.Exception jexception)
        {
            jexception.printStackTrace(System.err);
        }
        m_groupFilterComponent = null;
        m_titlesComponent = null;
        m_aggregateComponent = null;
        m_DBCommandFieldSelectio = null;
        xWindowPeer = null;
        m_finalizer = null;
        m_DBMetaData.finish();
        m_DBMetaData = null;
        System.gc();

        return m_createdQuery;
    }

    private void enableRoadmapItems(boolean _bEnabled)
    {
        try
        {
            Object oRoadmapItem;
            int CurStep = AnyConverter.toInt(Helper.getUnoPropertyValue(xDialogModel, PropertyNames.PROPERTY_STEP));
            boolean bEnabled = false;
            int CurItemID;
            for (int i = 0; i < getRMItemCount(); i++)
            {
                oRoadmapItem = this.xIndexContRoadmap.getByIndex(i);
                CurItemID = AnyConverter.toInt(Helper.getUnoPropertyValue(oRoadmapItem, "ID"));
                switch (CurItemID)
                {
                    case SOAGGREGATE_PAGE:
                        if (_bEnabled)
                        {
                            bEnabled = ((m_DBMetaData.hasNumericalFields()) && (m_DBMetaData.xDBMetaData.supportsCoreSQLGrammar()));
                        }
                        break;
                    case SOGROUPSELECTION_PAGE:
                        bEnabled = m_DBMetaData.Type == QueryMetaData.QueryType.SOSUMMARYQUERY;
                        break;
                    case SOGROUPFILTER_PAGE:
                        bEnabled = false;
                        if (_bEnabled)
                        {
                            bEnabled = (m_DBMetaData.GroupByFilterConditions.length > 0);
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
            exception.printStackTrace(System.err);
        }
    }

    private void insertQueryRelatedSteps()
    {
        try
        {
            setRMItemLabels(m_oResource);
            addRoadmap();
            int i = 0;
            i = insertRoadmapItem(0, true, SOFIELDSELECTION_PAGE - 1, SOFIELDSELECTION_PAGE);
            i = insertRoadmapItem(i, false, SOSORTING_PAGE - 1, SOSORTING_PAGE); // Orderby is always supported
            i = insertRoadmapItem(i, false, SOFILTER_PAGE - 1, SOFILTER_PAGE);
            if (m_DBMetaData.xDBMetaData.supportsCoreSQLGrammar())
            {
                i = insertRoadmapItem(i, m_DBMetaData.hasNumericalFields(), SOAGGREGATE_PAGE - 1, SOAGGREGATE_PAGE);
            }
            if (m_DBMetaData.xDBMetaData.supportsGroupBy())
            {
                i = insertRoadmapItem(i, false, SOGROUPSELECTION_PAGE - 1, SOGROUPSELECTION_PAGE);
                i = insertRoadmapItem(i, false, SOGROUPFILTER_PAGE - 1, SOGROUPFILTER_PAGE);
            }
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

    private void buildSteps()
    {
        try
        {
            m_DBCommandFieldSelectio = new CommandFieldSelection(
                    this, m_DBMetaData, 120, reslblFields, reslblSelFields, reslblTables,
                    m_DBMetaData.supportsQueriesInFrom(), 40850);
            m_DBCommandFieldSelectio.setAppendMode(true);
            m_DBCommandFieldSelectio.addFieldSelectionListener(new FieldSelectionListener());
            m_sortingComponent = new SortingComponent(this, SOSORTING_PAGE, 95, 27, 210, 40865);
            m_filterComponent = new FilterComponent(this, SOFILTER_PAGE, 97, 27, 3, m_DBMetaData, 40878);
            m_filterComponent.addNumberFormats();

            if (m_DBMetaData.xDBMetaData.supportsCoreSQLGrammar())
            {
                m_aggregateComponent = new AggregateComponent(this, m_DBMetaData, SOAGGREGATE_PAGE, 97, 69, 209, 5, 40895);
            }
            if (m_DBMetaData.xDBMetaData.supportsGroupBy())
            {
                m_groupFieldSelection = new FieldSelection(this, SOGROUPSELECTION_PAGE, 95, 27, 210, 150, reslblFields, this.reslblGroupBy, 40915, false);
                m_groupFieldSelection.addFieldSelectionListener(new FieldSelectionListener());
                m_groupFilterComponent = new FilterComponent(this, SOGROUPFILTER_PAGE, 97, 27, 3, m_DBMetaData, 40923);
            }
            m_titlesComponent = new TitlesComponent(this, SOTITLES_PAGE, 97, 37, 207, 7, reslblFieldHeader, reslblAliasHeader, 40940);
            m_finalizer = new Finalizer(this, m_DBMetaData);
            enableNavigationButtons(false, false, false);
        }
        catch (com.sun.star.uno.Exception exception)
        {
            Resource.showCommonResourceError(xMSF);
        }
    }

    @Override
    public boolean finishWizard()
    {
        int ncurStep = getCurrentStep();
        if ((ncurStep == SOSUMMARY_PAGE)
                || (switchToStep(ncurStep, SOSUMMARY_PAGE)))
        {
            m_createdQuery = m_finalizer.finish();
            if (m_createdQuery.length() > 0)
            {
                loadSubComponent(CommandType.QUERY, m_createdQuery, m_finalizer.displayQueryDesign());
                xDialog.endExecute();
                return true;
            }
        }
        return false;
    }

    @Override
    protected void enterStep(int nOldStep, int nNewStep)
    {
        try
        {
            if (nOldStep <= SOGROUPSELECTION_PAGE && nNewStep > SOGROUPSELECTION_PAGE)
            {
                if (m_DBMetaData.xDBMetaData.supportsGroupBy())
                {
                    m_DBMetaData.setGroupFieldNames(m_groupFieldSelection.getSelectedFieldNames());
                    m_DBMetaData.GroupFieldNames = JavaTools.removeOutdatedFields(m_DBMetaData.GroupFieldNames, m_DBMetaData.NonAggregateFieldNames);
                    m_DBMetaData.GroupByFilterConditions = JavaTools.removeOutdatedFields(m_DBMetaData.GroupByFilterConditions, m_DBMetaData.GroupFieldNames);
                }
            }
            switch (nNewStep)
            {
                case SOFIELDSELECTION_PAGE:
                    break;
                case SOSORTING_PAGE:
                    m_sortingComponent.initialize(m_DBMetaData.getDisplayFieldNames(), m_DBMetaData.getSortFieldNames());
                    break;
                case SOFILTER_PAGE:
                    m_filterComponent.initialize(m_DBMetaData.getFilterConditions(), m_DBMetaData.getDisplayFieldNames());
                    break;
                case SOAGGREGATE_PAGE:
                    m_aggregateComponent.initialize();
                    break;
                case SOGROUPSELECTION_PAGE:
                    break;
                case SOGROUPFILTER_PAGE:
                    m_groupFilterComponent.initialize(m_DBMetaData.GroupByFilterConditions, m_DBMetaData.getGroupFieldNames());
                    break;
                case SOTITLES_PAGE:
                    m_titlesComponent.initialize(m_DBMetaData.getDisplayFieldNames(), m_DBMetaData.FieldTitleSet);
                    break;
                case SOSUMMARY_PAGE:
                    m_finalizer.initialize();
                    break;
                default:
                    break;
            }
        }
        catch (SQLException e)
        {
            e.printStackTrace(System.err);
        }
    }

    @Override
    protected void leaveStep(int nOldStep, int nNewStep)
    {
        switch (nOldStep)
        {
            case SOFIELDSELECTION_PAGE:
                m_DBMetaData.reorderFieldColumns(m_DBCommandFieldSelectio.getSelectedFieldNames());
                m_DBMetaData.initializeFieldTitleSet();
                m_DBMetaData.setNumericFields();
                searchForOutdatedFields();
                break;
            case SOSORTING_PAGE:
                m_DBMetaData.setSortFieldNames(m_sortingComponent.getSortFieldNames());
                break;
            case SOFILTER_PAGE:
                m_DBMetaData.setFilterConditions(m_filterComponent.getFilterConditions());
                break;
            case SOAGGREGATE_PAGE:
                m_DBMetaData.AggregateFieldNames = m_aggregateComponent.getAggregateFieldNames();
                break;
            case SOGROUPSELECTION_PAGE:
                break;
            case SOGROUPFILTER_PAGE:
                m_DBMetaData.setGroupByFilterConditions(this.m_groupFilterComponent.getFilterConditions());
                break;
            case SOTITLES_PAGE:
                m_DBMetaData.setFieldTitles(m_titlesComponent.getFieldTitles());
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
                if (m_DBMetaData.Type == QueryMetaData.QueryType.SOSUMMARYQUERY)
                {
                    if (m_DBMetaData.xDBMetaData.supportsGroupBy())
                    {
                        m_DBMetaData.setNonAggregateFieldNames();
                        m_groupFieldSelection.initialize(m_DBMetaData.getUniqueAggregateFieldNames(), false, m_DBMetaData.xDBMetaData.getMaxColumnsInGroupBy());
                        m_groupFieldSelection.initializeSelectedFields(m_DBMetaData.NonAggregateFieldNames);
                        m_groupFieldSelection.setMultipleMode(false);
                        setStepEnabled(SOGROUPFILTER_PAGE, m_aggregateComponent.isGroupingpossible() && m_DBMetaData.NonAggregateFieldNames.length > 0);
                    }
                }
            }
            catch (SQLException e)
            {
                e.printStackTrace( System.err );
            }
        }
    }

    private void searchForOutdatedFields()
    {
        String[] sFieldNames = m_DBMetaData.getFieldNames();
        String[][] sRemovedFields = JavaTools.removeOutdatedFields(m_DBMetaData.getSortFieldNames(), sFieldNames);
        m_DBMetaData.setSortFieldNames(sRemovedFields);
        m_DBMetaData.setFilterConditions(JavaTools.removeOutdatedFields(m_DBMetaData.getFilterConditions(), sFieldNames));
        m_DBMetaData.AggregateFieldNames = JavaTools.removeOutdatedFields(m_DBMetaData.AggregateFieldNames, sFieldNames);
    }

    private void enableWizardSteps(String[] NewItems)
    {
        boolean bEnabled = NewItems.length > 0;
        setControlProperty("btnWizardNext", PropertyNames.PROPERTY_ENABLED, bEnabled);
        setControlProperty("btnWizardFinish", PropertyNames.PROPERTY_ENABLED, bEnabled);
        enableRoadmapItems(bEnabled); // Note: Performance wise this could be improved
    }

    private class FieldSelectionListener implements com.sun.star.wizards.ui.XFieldSelectionListener
    {

        private int ID;

        public void setID(String sIncSuffix)
        {
            ID = 1;
            if (sIncSuffix != null)
            {
                if ((!sIncSuffix.equals(PropertyNames.EMPTY_STRING)) && (!sIncSuffix.equals("_")))
                {
                    String sID = JavaTools.ArrayoutofString(sIncSuffix, "_")[1];
                    ID = Integer.parseInt(sID);
                }
            }
        }

        public void shiftFromLeftToRight(String[] SelItems, String[] NewItems)
        {
            if (ID == 1)
            {
                m_DBMetaData.addSeveralFieldColumns(SelItems, m_DBCommandFieldSelectio.getSelectedCommandName());
                enableWizardSteps(NewItems);
                m_DBCommandFieldSelectio.changeSelectedFieldNames(m_DBMetaData.getDisplayFieldNames());
                m_DBCommandFieldSelectio.toggleCommandListBox(NewItems);
            }
            else
            {
                boolean bEnabled = (m_groupFieldSelection.getSelectedFieldNames().length > 0);
                Helper.setUnoPropertyValue(getRoadmapItemByID(SOGROUPFILTER_PAGE), PropertyNames.PROPERTY_ENABLED, bEnabled);
            }
        }

        public void shiftFromRightToLeft(String[] SelItems, String[] NewItems)
        {
            // TODO When the ListFieldbox is refilled only fields of the current Command may be merged into the Listbox
            if (ID == 1)
            {
                enableWizardSteps(NewItems);
                String[] sSelfieldNames = m_DBMetaData.getFieldNames(SelItems, m_DBCommandFieldSelectio.getSelectedCommandName());
                m_DBCommandFieldSelectio.addItemsToFieldsListbox(sSelfieldNames);
                m_DBMetaData.removeSeveralFieldColumnsByDisplayFieldName(SelItems);
                m_DBCommandFieldSelectio.toggleCommandListBox(NewItems);

            }
            else
            {
                boolean bEnabled = (m_groupFieldSelection.getSelectedFieldNames().length > 0);
                String CurDisplayFieldName = SelItems[0];
                if (JavaTools.FieldInList(m_DBMetaData.NonAggregateFieldNames, CurDisplayFieldName) > -1)
                {
                    showMessageBox("ErrorBox", VclWindowPeerAttribute.OK, resmsgNonNumericAsGroupBy);
                    m_groupFieldSelection.xSelectedFieldsListBox.addItems(SelItems, m_groupFieldSelection.xSelectedFieldsListBox.getItemCount());
                    String FieldList[] = m_groupFieldSelection.xFieldsListBox.getItems();
                    int index = JavaTools.FieldInList(FieldList, CurDisplayFieldName);
                    if (index > -1)
                    {
                        m_groupFieldSelection.xFieldsListBox.removeItems((short) index, (short) 1);
                    }
                }
                else
                {
                    Helper.setUnoPropertyValue(getRoadmapItemByID(SOGROUPFILTER_PAGE), PropertyNames.PROPERTY_ENABLED, bEnabled);
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
