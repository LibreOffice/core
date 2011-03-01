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
package com.sun.star.wizards.table;

import java.util.Hashtable;

import com.sun.star.awt.TextEvent;
import com.sun.star.awt.VclWindowPeerAttribute;
import com.sun.star.awt.XTextListener;
import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdb.CommandType;
import com.sun.star.sdb.application.DatabaseObject;
import com.sun.star.sdbc.SQLException;
import com.sun.star.task.XJobExecutor;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.*;
import com.sun.star.wizards.db.DatabaseObjectWizard;
import com.sun.star.wizards.db.TableDescriptor;
import com.sun.star.wizards.ui.*;

public class TableWizard extends DatabaseObjectWizard implements XTextListener, XCompletion
{

    static String slblFields;
    static String slblSelFields;
    Finalizer curFinalizer;
    ScenarioSelector curScenarioSelector;
    FieldFormatter curFieldFormatter;
    PrimaryKeyHandler curPrimaryKeyHandler;
    String sMsgWizardName = PropertyNames.EMPTY_STRING;
    public Hashtable fielditems;
    int wizardmode;
    String tablename;
    String serrToManyFields;
    String serrTableNameexists;
    String scomposedtablename;
    TableDescriptor curTableDescriptor;
    public static final int SONULLPAGE = 0;
    public static final int SOMAINPAGE = 1;
    public static final int SOFIELDSFORMATPAGE = 2;
    public static final int SOPRIMARYKEYPAGE = 3;
    public static final int SOFINALPAGE = 4;
    private String sMsgColumnAlreadyExists = PropertyNames.EMPTY_STRING;
    String WizardHeaderText[] = new String[8];

    private String m_tableName;

    public TableWizard( XMultiServiceFactory xMSF, PropertyValue[] i_wizardContext )
    {
        super( xMSF, 41200, i_wizardContext );
        super.addResourceHandler("TableWizard", "dbw");
        String sTitle = m_oResource.getResText(UIConsts.RID_TABLE + 1);
        Helper.setUnoPropertyValues(xDialogModel,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_MOVEABLE, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_TITLE, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    218, Boolean.TRUE, "DialogTable", 102, 41, 1, new Short((short) 0), sTitle, 330
                });
        drawNaviBar();
        fielditems = new Hashtable();
        //TODO if reportResouces cannot be gotten dispose officedocument
        if (getTableResources() == true)
        {
            setRightPaneHeaders(m_oResource, UIConsts.RID_TABLE + 8, 4);
        }
    }

    protected void leaveStep(int nOldStep, int nNewStep)
    {
        switch (nOldStep)
        {
            case SOMAINPAGE:
                curScenarioSelector.addColumnsToDescriptor();
                break;
            case SOFIELDSFORMATPAGE:
                curFieldFormatter.updateColumnofColumnDescriptor();
                String[] sfieldnames = curFieldFormatter.getFieldNames();
                super.setStepEnabled(SOFIELDSFORMATPAGE, sfieldnames.length > 0);
                curScenarioSelector.setSelectedFieldNames(sfieldnames);
                break;
            case SOPRIMARYKEYPAGE:
                break;
            case SOFINALPAGE:
                break;
            default:
                break;
        }
    }

    protected void enterStep(int nOldStep, int nNewStep)
    {
        switch (nNewStep)
        {
            case SOMAINPAGE:
                break;
            case SOFIELDSFORMATPAGE:
                curFieldFormatter.initialize(curTableDescriptor, this.curScenarioSelector.getSelectedFieldNames());
                break;
            case SOPRIMARYKEYPAGE:
                curPrimaryKeyHandler.initialize();
                break;
            case SOFINALPAGE:
                curFinalizer.initialize(curScenarioSelector.getFirstTableName());
                break;
            default:
                break;
        }
    }


    /* (non-Javadoc)
     * @see com.sun.star.wizards.ui.XCompletion#iscompleted(int)
     */
    public boolean iscompleted(int _ndialogpage)
    {
        switch (_ndialogpage)
        {
            case SOMAINPAGE:
                return curScenarioSelector.iscompleted();
            case SOFIELDSFORMATPAGE:
                return this.curFieldFormatter.iscompleted();
            case SOPRIMARYKEYPAGE:
                if (curPrimaryKeyHandler != null)
                {
                    return this.curPrimaryKeyHandler.iscompleted();
                }
            case SOFINALPAGE:
                return this.curFinalizer.iscompleted();
            default:
                return false;
        }
    }


    /* (non-Javadoc)
     * @see com.sun.star.wizards.ui.XCompletion#setcompleted(int, boolean)
     */
    public void setcompleted(int _ndialogpage, boolean _biscompleted)
    {
        boolean bScenarioiscompleted = _biscompleted;
        boolean bFieldFormatsiscompleted = _biscompleted;
        boolean bPrimaryKeysiscompleted = _biscompleted;
        boolean bFinalPageiscompleted = _biscompleted;
        if (_ndialogpage == SOMAINPAGE)
        {
            curFinalizer.initialize(curScenarioSelector.getFirstTableName());
        }
        else
        {
            bScenarioiscompleted = iscompleted(SOMAINPAGE);
        }
        if (_ndialogpage != TableWizard.SOFIELDSFORMATPAGE)
        {
            bFieldFormatsiscompleted = iscompleted(SOFIELDSFORMATPAGE);
            if (!bFieldFormatsiscompleted)                              // it might be that the Fieldformatter has not yet been initialized
            {
                bFieldFormatsiscompleted = bScenarioiscompleted;        // in this case query the scenarioselector
            }
        }
        if (_ndialogpage != TableWizard.SOPRIMARYKEYPAGE && (this.curPrimaryKeyHandler != null))
        {
            bPrimaryKeysiscompleted = iscompleted(SOPRIMARYKEYPAGE);
        }
        if (_ndialogpage != TableWizard.SOFINALPAGE)
        {
            bFinalPageiscompleted = iscompleted(SOFINALPAGE);           // Basically the finalpage is always enabled
        }
        if (bScenarioiscompleted)
        {
            super.setStepEnabled(SOFIELDSFORMATPAGE, true);
            super.setStepEnabled(SOPRIMARYKEYPAGE, true);
            if (bPrimaryKeysiscompleted)
            {
                super.enablefromStep(SOFINALPAGE, true);
                super.enableFinishButton(bFinalPageiscompleted);
            }
            else
            {
                super.enablefromStep(SOFINALPAGE, false);
                enableNextButton(false);
            }
        }
        else if (_ndialogpage == SOFIELDSFORMATPAGE)
        {
            super.enablefromStep(super.getCurrentStep() + 1, iscompleted(SOFIELDSFORMATPAGE));
        }
        else
        {
            super.enablefromStep(super.getCurrentStep() + 1, false);
        }
    }

/*
    public static void main(String args[])
    {
        String ConnectStr = "uno:socket,host=localhost,port=8100;urp,negotiate=0,forcesynchronous=1;StarOffice.NamingService";
        PropertyValue[] curproperties = null;
        try
        {
            XMultiServiceFactory xLocMSF = com.sun.star.wizards.common.Desktop.connect(ConnectStr);
            TableWizard CurTableWizard = new TableWizard(xLocMSF);
            if (xLocMSF != null)
            {
                System.out.println("Connected to " + ConnectStr);
                curproperties = new PropertyValue[1];
                curproperties[0] = Properties.createProperty("DataSourceName", "Bibliography");
                //curproperties[0] = Properties.createProperty("DatabaseLocation", "file:///path/to/database.odb");
                CurTableWizard.startTableWizard(xLocMSF, curproperties);
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
        }
    }
*/
    public void buildSteps()
    {
        curScenarioSelector = new ScenarioSelector(this, this.curTableDescriptor, slblFields, slblSelFields);
        curFieldFormatter = new FieldFormatter(this, curTableDescriptor);
        if ( this.curTableDescriptor.supportsPrimaryKeys() )
        {
            curPrimaryKeyHandler = new PrimaryKeyHandler(this, curTableDescriptor);
        }
        curFinalizer = new Finalizer(this, curTableDescriptor);
        enableNavigationButtons(false, false, false);
    }

    public boolean createTable()
    {
        boolean bIsSuccessfull = true;
        boolean bTableCreated = false;
        String schemaname = curFinalizer.getSchemaName();
        String catalogname = curFinalizer.getCatalogName();
        if (curTableDescriptor.supportsPrimaryKeys())
        {
            String[] keyfieldnames = curPrimaryKeyHandler.getPrimaryKeyFields(curTableDescriptor);
            if (keyfieldnames != null)
            {
                if (keyfieldnames.length > 0)
                {
                    boolean bIsAutoIncrement = curPrimaryKeyHandler.isAutoIncremented();
                    bIsSuccessfull = curTableDescriptor.createTable(catalogname, schemaname, tablename, keyfieldnames, bIsAutoIncrement, curScenarioSelector.getSelectedFieldNames());
                    bTableCreated = true;
                }
            }
        }
        if (!bTableCreated)
        {
            bIsSuccessfull = curTableDescriptor.createTable(catalogname, schemaname, tablename, curScenarioSelector.getSelectedFieldNames());
        }
        if ((!bIsSuccessfull) && (curPrimaryKeyHandler.isAutomaticMode()))
        {
            curTableDescriptor.dropColumnbyName(curPrimaryKeyHandler.getAutomaticFieldName());
        }
        return bIsSuccessfull;
    }

    public boolean finishWizard()
    {
        super.switchToStep(super.getCurrentStep(), SOFINALPAGE);
        tablename = curFinalizer.getTableName(curScenarioSelector.getFirstTableName());
        scomposedtablename = curFinalizer.getComposedTableName(tablename);
        if (this.curTableDescriptor.isSQL92CheckEnabled())
        {
            Desktop.removeSpecialCharacters(curTableDescriptor.xMSF, Configuration.getOfficeLocale(this.curTableDescriptor.xMSF), tablename);
        }
        if ( tablename.length() > 0 )
        {
            if (!curTableDescriptor.hasTableByName(scomposedtablename))
            {
                wizardmode = curFinalizer.finish();
                if (createTable())
                {
                    final boolean editTableDesign = (wizardmode == Finalizer.MODIFYTABLEMODE );
                    loadSubComponent( DatabaseObject.TABLE, curTableDescriptor.getComposedTableName(), editTableDesign );
                    m_tableName = curTableDescriptor.getComposedTableName();
                    super.xDialog.endExecute();
                    return true;
                }
            }
            else
            {
                String smessage = JavaTools.replaceSubString(serrTableNameexists, tablename, "%TABLENAME");
                super.showMessageBox("WarningBox", com.sun.star.awt.VclWindowPeerAttribute.OK, smessage);
                curFinalizer.setFocusToTableNameControl();
            }
        }
        return false;
    }

    private void callFormWizard()
    {
        try
        {
            Object oFormWizard = this.xMSF.createInstance("com.sun.star.wizards.form.CallFormWizard");

            NamedValueCollection wizardContext = new NamedValueCollection();
            wizardContext.put( PropertyNames.ACTIVE_CONNECTION, curTableDescriptor.DBConnection );
            wizardContext.put( "DataSource", curTableDescriptor.getDataSource() );
            wizardContext.put( PropertyNames.COMMAND_TYPE, CommandType.TABLE );
            wizardContext.put( PropertyNames.COMMAND, scomposedtablename );
            wizardContext.put( "DocumentUI", m_docUI );
            XInitialization xInitialization = UnoRuntime.queryInterface( XInitialization.class, oFormWizard );
            xInitialization.initialize( wizardContext.getPropertyValues() );
            XJobExecutor xJobExecutor = UnoRuntime.queryInterface( XJobExecutor.class, oFormWizard );
            xJobExecutor.trigger(PropertyNames.START);
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
        }
    }

    public void cancelWizard()
    {
        xDialog.endExecute();
    }

    public void insertFormRelatedSteps()
    {
        addRoadmap();
        int i = 0;
        i = insertRoadmapItem(0, true, m_oResource.getResText(UIConsts.RID_TABLE + 2), SOMAINPAGE);
        i = insertRoadmapItem(i, false, m_oResource.getResText(UIConsts.RID_TABLE + 3), SOFIELDSFORMATPAGE);
        if (this.curTableDescriptor.supportsPrimaryKeys())
        {
            i = insertRoadmapItem(i, false, m_oResource.getResText(UIConsts.RID_TABLE + 4), SOPRIMARYKEYPAGE);
        }
        i = insertRoadmapItem(i, false, m_oResource.getResText(UIConsts.RID_TABLE + 5), SOFINALPAGE);        // Orderby is always supported
        setRoadmapInteractive(true);
        setRoadmapComplete(true);
        setCurrentRoadmapItemID((short) 1);
    }

    public String startTableWizard(  )
    {
        try
        {
            curTableDescriptor = new TableDescriptor(xMSF, super.xWindow, this.sMsgColumnAlreadyExists);
            if ( curTableDescriptor.getConnection( m_wizardContext ) )
            {
                buildSteps();
                createWindowPeer();
                curTableDescriptor.setWindowPeer(this.xControl.getPeer());
                insertFormRelatedSteps();
                short RetValue = executeDialog();
                xComponent.dispose();
                if ( RetValue == 0 )
                {
                    if (  wizardmode == Finalizer.STARTFORMWIZARDMODE )
                        callFormWizard();
                    return m_tableName;
                }
            }
        }
        catch (java.lang.Exception jexception)
        {
            jexception.printStackTrace(System.out);
        }
        return PropertyNames.EMPTY_STRING;
    }

    public boolean getTableResources()
    {
        sMsgWizardName = super.m_oResource.getResText(UIConsts.RID_TABLE + 1);
        slblFields = m_oResource.getResText(UIConsts.RID_TABLE + 19);
        slblSelFields = m_oResource.getResText(UIConsts.RID_TABLE + 25);
        serrToManyFields = m_oResource.getResText(UIConsts.RID_TABLE + 47);
        serrTableNameexists = m_oResource.getResText(UIConsts.RID_TABLE + 48);
        sMsgColumnAlreadyExists = m_oResource.getResText(UIConsts.RID_TABLE + 51);
        return true;
    }

    public boolean verifyfieldcount(int _icount)
    {
        try
        {
            int maxfieldcount = curTableDescriptor.getMaxColumnsInTable();
            if (_icount >= (maxfieldcount - 1))
            {   // keep one column as reserve for the automaticcally created key
                String smessage = serrToManyFields;
                smessage = JavaTools.replaceSubString(smessage, String.valueOf(maxfieldcount), "%COUNT");
                showMessageBox("ErrorBox", VclWindowPeerAttribute.OK, smessage);
                return false;
            }
        }
        catch (SQLException e)
        {
            e.printStackTrace(System.out);
        }
        return true;
    }


    /* (non-Javadoc)
     * @see com.sun.star.awt.XTextListener#textChanged(com.sun.star.awt.TextEvent)
     */
    public void textChanged(TextEvent aTextEvent)
    {
        if (this.curTableDescriptor.isSQL92CheckEnabled())
        {
            Object otextcomponent = UnoDialog.getModel(aTextEvent.Source);
            String sName = (String) Helper.getUnoPropertyValue(otextcomponent, "Text");
            sName = Desktop.removeSpecialCharacters(curTableDescriptor.xMSF, Configuration.getOfficeLocale(curTableDescriptor.xMSF), sName);
            Helper.setUnoPropertyValue(otextcomponent, "Text", sName);
        }
    }
}
