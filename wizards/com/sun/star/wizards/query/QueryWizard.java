/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
import com.sun.star.wizards.common.Desktop;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.common.Properties;
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
    public static final String SFILLUPFIELDSLISTBOX = "fillUpFieldsListbox";
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
    private String resQueryWizard;
    private String reslblGroupBy;
    private String resmsgNonNumericAsGroupBy;
    private String m_createdQuery;

    public QueryWizard( XMultiServiceFactory xMSF, PropertyValue[] i_wizardContext )
    {
        super( xMSF, 40970, i_wizardContext );
        addResourceHandler("QueryWizard", "dbw");
        m_DBMetaData = new QuerySummary(xMSF, m_oResource);
    }

    public static void main(String i_args[])
    {
        final String settings[] = new String[] { null, null, null };
        final int IDX_PIPE_NAME = 0;
        final int IDX_LOCATION = 1;
        final int IDX_DSN = 2;

        // some simple parsing
        boolean failure = false;
        int settingsIndex = -1;
        for ( int i=0; i<i_args.length; ++i )
        {
            if ( settingsIndex >= 0 )
            {
                settings[ settingsIndex ] = i_args[i];
                settingsIndex = -1;
                continue;
            }

            if ( i_args[i].equals( "--pipe-name" ) )
            {
                settingsIndex = IDX_PIPE_NAME;
                continue;
            }

            if ( i_args[i].equals( "--database-location" ) )
            {
                settingsIndex = IDX_LOCATION;
                continue;
            }

            if ( i_args[i].equals( "--data-source-name" ) )
            {
                settingsIndex = IDX_DSN;
                continue;
            }

            failure = true;
        }

        if ( settings[ IDX_PIPE_NAME ] == null )
            failure = true;

        if ( ( settings[ IDX_DSN ] == null ) && ( settings[ IDX_LOCATION ] == null ) )
            failure = true;

        if ( failure )
        {
            System.err.println( "supported arguments: " );
            System.err.println( "  --pipe-name <name>           : specifies the name of the pipe to connect to the running OOo instance" );
            System.err.println( "  --database-location <url>    : specifies the URL of the database document to work with" );
            System.err.println( "  --data-source-name <name>    : specifies the name of the data source to work with" );
            return;
        }

        final String ConnectStr = "uno:pipe,name=" + settings[IDX_PIPE_NAME] + ";urp;StarOffice.ServiceManager";
        try
        {
            final XMultiServiceFactory serviceFactory = Desktop.connect(ConnectStr);
            if (serviceFactory != null)
            {
                PropertyValue[] curproperties = new PropertyValue[1];
                if ( settings[ IDX_LOCATION ] != null )
                    curproperties[0] = Properties.createProperty( "DatabaseLocation", settings[ IDX_LOCATION ] );
                else
                    curproperties[0] = Properties.createProperty( "DataSourceName", settings[ IDX_DSN ] );

                QueryWizard CurQueryWizard = new QueryWizard( serviceFactory, curproperties );
                CurQueryWizard.startQueryWizard();
            }
        }
        catch (java.lang.Exception jexception)
        {
            jexception.printStackTrace(System.out);
        }
    }

    public final XFrame getFrame()
    {
        return m_frame;
    }

    public String startQueryWizard()
    {
        try
        {
            if ( m_DBMetaData.getConnection( m_wizardContext ) )
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
                            PropertyNames.PROPERTY_HEIGHT, "Moveable", PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, "Title", PropertyNames.PROPERTY_WIDTH
                        },
                        new Object[]
                        {
                            new Integer(210), Boolean.TRUE, "DialogQuery", new Integer(102), new Integer(41), new Integer(1), new Short((short) 0), resQueryWizard, new Integer(310)
                        });
                drawNaviBar();
                setRightPaneHeaders(m_oResource, UIConsts.RID_QUERY + 70, 8);
                this.setMaxStep(8);
                buildSteps();
                this.m_DBCommandFieldSelectio.preselectCommand( m_wizardContext, false );

                XWindowPeer windowPeer = UnoRuntime.queryInterface( XWindowPeer.class, m_frame.getContainerWindow() );
                createWindowPeer(windowPeer);
                m_DBMetaData.setWindowPeer(this.xControl.getPeer());
                insertQueryRelatedSteps();
                executeDialog( m_frame.getContainerWindow().getPosSize() );
            }
        }
        catch (java.lang.Exception jexception)
        {
            jexception.printStackTrace(System.out);
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

    public void enableRoadmapItems(String[] _FieldNames, boolean _bEnabled)
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
                        if (_bEnabled == true)
                        {
                            bEnabled = ((m_DBMetaData.hasNumericalFields()) && (m_DBMetaData.xDBMetaData.supportsCoreSQLGrammar()));
                        }
                        break;
                    case SOGROUPSELECTION_PAGE:
                        bEnabled = m_DBMetaData.Type == QueryMetaData.QueryType.SOSUMMARYQUERY;
                        break;
                    case SOGROUPFILTER_PAGE:
                        bEnabled = false;
                        if (_bEnabled == true)
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
            if (m_DBMetaData.xDBMetaData.supportsCoreSQLGrammar())
            {
                i = insertRoadmapItem(i, m_DBMetaData.hasNumericalFields(), SOAGGREGATE_PAGE - 1, SOAGGREGATE_PAGE);
            }
            if (m_DBMetaData.xDBMetaData.supportsGroupBy())
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
            m_DBCommandFieldSelectio = new CommandFieldSelection(
                this, m_DBMetaData, 120, reslblFields, reslblSelFields, reslblTables,
                m_DBMetaData.supportsQueriesInFrom(), 40850);
            m_DBCommandFieldSelectio.setAppendMode(true);
            m_DBCommandFieldSelectio.addFieldSelectionListener(new FieldSelectionListener());
            m_sortingComponent = new SortingComponent(this, SOSORTING_PAGE, 95, 27, 210, 40865);
            m_filterComponent = new FilterComponent(this, xMSF, SOFILTER_PAGE, 97, 27, 209, 3, m_DBMetaData, 40878);
            m_filterComponent.addNumberFormats();

            if (m_DBMetaData.xDBMetaData.supportsCoreSQLGrammar())
            {
                m_aggregateComponent = new AggregateComponent(this, m_DBMetaData, SOAGGREGATE_PAGE, 97, 69, 209, 5, 40895);
            }
            if (m_DBMetaData.xDBMetaData.supportsGroupBy())
            {
                m_groupFieldSelection = new FieldSelection(this, SOGROUPSELECTION_PAGE, 95, 27, 210, 150, reslblFields, this.reslblGroupBy, 40915, false);
                m_groupFieldSelection.addFieldSelectionListener(new FieldSelectionListener());
                m_groupFilterComponent = new FilterComponent(this, xMSF, SOGROUPFILTER_PAGE, 97, 27, 209, 3, m_DBMetaData, 40923);
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

    public boolean finishWizard()
    {
        int ncurStep = getCurrentStep();
        if  (   ( ncurStep == SOSUMMARY_PAGE )
            ||  ( switchToStep( ncurStep, SOSUMMARY_PAGE ) )
            )
        {
            m_createdQuery = m_finalizer.finish();
            if ( m_createdQuery.length() > 0 )
            {
                loadSubComponent( CommandType.QUERY, m_createdQuery, m_finalizer.displayQueryDesign() );
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
            e.printStackTrace(System.out);
        }
    }

    protected void leaveStep(int nOldStep, int nNewStep)
    {
        switch (nOldStep)
        {
            case SOFIELDSELECTION_PAGE:
                m_DBMetaData.reorderFieldColumns(m_DBCommandFieldSelectio.getSelectedFieldNames());
                m_DBMetaData.initializeFieldTitleSet(true);
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
                        m_groupFieldSelection.intializeSelectedFields(m_DBMetaData.NonAggregateFieldNames);
                        m_groupFieldSelection.setMultipleMode(false);
                        setStepEnabled(SOGROUPFILTER_PAGE, m_aggregateComponent.isGroupingpossible() && m_DBMetaData.NonAggregateFieldNames.length > 0);
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
