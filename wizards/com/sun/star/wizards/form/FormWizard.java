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
package com.sun.star.wizards.form;

import com.sun.star.awt.XWindowPeer;
import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdb.application.DatabaseObject;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.common.NoValidPathException;
import com.sun.star.wizards.common.Properties;
import com.sun.star.wizards.db.DatabaseObjectWizard;
import com.sun.star.wizards.db.RelationController;
import com.sun.star.wizards.document.OfficeDocument;
import com.sun.star.wizards.ui.CommandFieldSelection;
import com.sun.star.wizards.ui.UIConsts;

public class FormWizard extends DatabaseObjectWizard
{

    private CommandFieldSelection curDBCommandFieldSelection;
    private FormConfiguration curFormConfiguration;
    private CommandFieldSelection curSubFormFieldSelection;
    private FormDocument curFormDocument;
    private FieldLinker curFieldLinker;
    private UIControlArranger curControlArranger;
    private DataEntrySetter CurDataEntrySetter;
    private StyleApplier curStyleApplier;
    private Finalizer curFinalizer;
    private static String slblFields;
    private static String slblSelFields;
    private String sShowBinaryFields = PropertyNames.EMPTY_STRING;
    private String serrFormNameexists = PropertyNames.EMPTY_STRING;
    public static final int SOMAIN_PAGE = 1;
    public static final int SOSUBFORM_PAGE = 2;
    public static final int SOSUBFORMFIELDS_PAGE = 3;
    public static final int SOFIELDLINKER_PAGE = 4;
    public static final int SOCONTROL_PAGE = 5;
    public static final int SODATA_PAGE = 6;
    public static final int SOSTYLE_PAGE = 7;
    public static final int SOSTORE_PAGE = 8;
    public static final int COLUMNAR_LEFT = 1;
    public static final int COLUMNAR_TOP = 2;
    public static final int AS_GRID = 3;
    public static final int IN_BLOCK_TOP = 4;
    private String slblTables;
    private boolean m_openForEditing;
    private boolean m_success = false;
    private String FormName;

    public FormWizard(XMultiServiceFactory i_servicFactory, final PropertyValue[] i_wizardContext)
    {
        super(i_servicFactory, 34400, i_wizardContext);
        super.addResourceHandler("FormWizard", "dbw");
        Helper.setUnoPropertyValues(xDialogModel,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_MOVEABLE, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_TITLE, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    210, Boolean.TRUE, "DialogForm", 102, 41, 1, new Short((short) 0), m_oResource.getResText(UIConsts.RID_FORM), 310
                });
        drawNaviBar();
        if (getFormResources())
        {
            setRightPaneHeaders(m_oResource, UIConsts.RID_FORM + 90, 8);
        }
    }

    public static void main(String i_args[])
    {
        executeWizardFromCommandLine( i_args, FormWizard.class.getName() );
    }

    // @Override
    protected void enterStep(int nOldStep, int nNewStep)
    {
        try
        {
            if ((nOldStep < SOCONTROL_PAGE) && (nNewStep >= SOCONTROL_PAGE))
            {
                curFormDocument.initialize(curDBCommandFieldSelection.isModified(), curFormConfiguration.hasSubForm(), curSubFormFieldSelection.isModified(), getBorderType());
                curDBCommandFieldSelection.setModified(false);
                curSubFormFieldSelection.setModified(false);
            }
            switch (nNewStep)
            {
                case SOMAIN_PAGE:
                    curDBCommandFieldSelection.setModified(false);
                    break;
                case SOSUBFORM_PAGE:
                {
                    final String sCommandName = curDBCommandFieldSelection.getSelectedCommandName();
                    RelationController oRelationController = new RelationController(curFormDocument.oMainFormDBMetaData, sCommandName);
                    curFormConfiguration.initialize(curSubFormFieldSelection, oRelationController);
                }
                break;
                case SOSUBFORMFIELDS_PAGE:
                {
                    String sPreSelectedTableName = curFormConfiguration.getreferencedTableName();
                    boolean bReadOnly = (sPreSelectedTableName.length() > 0);
                    if (sPreSelectedTableName.length() == 0)
                    {
                        final String sTableName = curSubFormFieldSelection.getSelectedCommandName();
                        String[] aFieldNames = curSubFormFieldSelection.getSelectedFieldNames();
                        curFormDocument.oSubFormDBMetaData.initializeFieldColumns(true, sTableName, aFieldNames);
                    }
                    else
                    {
                        curSubFormFieldSelection.preselectCommand(sPreSelectedTableName, bReadOnly);
                    }
                }
                break;
                case SOFIELDLINKER_PAGE:
                {
                    final String[] aMainFieldNames = curFormDocument.oMainFormDBMetaData.getFieldNames();
                    final String[] aSubFieldNames = curFormDocument.oSubFormDBMetaData.getFieldNames();
                    curFieldLinker.initialize(aMainFieldNames, aSubFieldNames, curFormDocument.LinkFieldNames);
                }
                break;
                case SOCONTROL_PAGE:
                    curControlArranger.enableSubFormImageList(curFormConfiguration.hasSubForm());
                    break;
                case SODATA_PAGE:
                    break;
                case SOSTYLE_PAGE:
                    break;
                case SOSTORE_PAGE:
                {
                    String sTableName = this.curDBCommandFieldSelection.getSelectedCommandName();
                    this.curFinalizer.initialize(sTableName, curFormDocument);
                }
                break;
                default:
                    break;
            }
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
        }
    }

    protected Short getBorderType()
    {
        return curStyleApplier.getBorderType();
    }

    // @Override
    protected void leaveStep(int nOldStep, int nNewStep)
    {
        switch (nOldStep)
        {
            case SOMAIN_PAGE:
            {
                final String sTableName = curDBCommandFieldSelection.getSelectedCommandName();
                final String[] aFieldNames = curDBCommandFieldSelection.getSelectedFieldNames();
                curFormDocument.oMainFormDBMetaData.initializeFieldColumns(true, sTableName, aFieldNames);

                final String[] aMainFieldNames = curFormDocument.oMainFormDBMetaData.getFieldNames();
                try
                {
                    curFormDocument.LinkFieldNames = JavaTools.removeOutdatedFields(curFormDocument.LinkFieldNames, aMainFieldNames, 1);
                }
                catch (java.lang.Exception e)
                {
                }
            }
            break;
            case SOSUBFORM_PAGE:
                break;
            case SOSUBFORMFIELDS_PAGE:
            {
                final String sTableName = curSubFormFieldSelection.getSelectedCommandName();
                final String[] aFieldNames = curSubFormFieldSelection.getSelectedFieldNames();
                curFormDocument.oSubFormDBMetaData.initializeFieldColumns(true, sTableName, aFieldNames);

                final String[] aSubFieldNames = curFormDocument.oSubFormDBMetaData.getFieldNames();
                try
                {
                    curFormDocument.LinkFieldNames = JavaTools.removeOutdatedFields(curFormDocument.LinkFieldNames, aSubFieldNames, 0);
                }
                catch (java.lang.Exception e)
                {
                }
            }
            break;
            case SOFIELDLINKER_PAGE:
                curFormDocument.LinkFieldNames = curFieldLinker.getLinkFieldNames();
                break;
            case SOCONTROL_PAGE:
                break;
            case SODATA_PAGE:
                break;
            case SOSTYLE_PAGE:
                break;
            case SOSTORE_PAGE:
                break;
            default:
                break;
        }
    }

    public void buildSteps() throws NoValidPathException
    {
        curDBCommandFieldSelection = new CommandFieldSelection(this, curFormDocument.oMainFormDBMetaData, 92, slblFields, slblSelFields, slblTables, true, 34411);
        curDBCommandFieldSelection.addFieldSelectionListener(new FieldSelectionListener());
        curFormDocument.xProgressBar.setValue(20);
        // Label Help Text
        insertLabel("lblBinaryHelpText",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL,
		    PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_POSITION_X,
		    PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP,
		    PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    28, sShowBinaryFields, Boolean.TRUE, 95, 154, new Integer(SOMAIN_PAGE), 210
                });

        curFormConfiguration = new FormConfiguration(this);
        curFormDocument.xProgressBar.setValue(30);

        curSubFormFieldSelection = new CommandFieldSelection(this, curFormDocument.oSubFormDBMetaData, SOSUBFORMFIELDS_PAGE, 92, slblFields, slblSelFields, slblTables, true, 34431);
        curSubFormFieldSelection.addFieldSelectionListener(new FieldSelectionListener());
        insertLabel("lblSubFormBinaryHelpText",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL,
		    PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_POSITION_X,
		    PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP,
		    PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    28, sShowBinaryFields, Boolean.TRUE, 95, 154, new Integer(SOSUBFORMFIELDS_PAGE), 210
                });

        curFormDocument.xProgressBar.setValue(40);

        curFieldLinker = new FieldLinker(this, SOFIELDLINKER_PAGE, 95, 30, 210, 34441);
        curFormDocument.xProgressBar.setValue(50);

        curControlArranger = new UIControlArranger(this, curFormDocument);
        curFormDocument.addUIFormController(curControlArranger);
        curFormDocument.xProgressBar.setValue(60);

        CurDataEntrySetter = new DataEntrySetter(this);
        curFormDocument.xProgressBar.setValue(70);

        curStyleApplier = new StyleApplier(this, curFormDocument);
        curFormDocument.addStyleApplier(curStyleApplier);
        curFormDocument.xProgressBar.setValue(80);

        curFinalizer = new Finalizer(this);
        curFormDocument.xProgressBar.setValue(100);

        enableNavigationButtons(false, false, false);
        curFormDocument.xProgressBar.end();
    }

    // @Override
    public boolean finishWizard()
    {
        int ncurStep = getCurrentStep();
        if ((switchToStep(ncurStep, SOSTORE_PAGE)) || (ncurStep == SOSTORE_PAGE))
        {
            this.curFinalizer.initialize(curDBCommandFieldSelection.getSelectedCommandName(), this.curFormDocument);
            String sNewFormName = curFinalizer.getName();
            if (!curFormDocument.oMainFormDBMetaData.hasFormDocumentByName(sNewFormName))
            {
                m_openForEditing = curFinalizer.getOpenForEditing();
                FormName = curFinalizer.getName();
                if (curFormDocument.finalizeForms(CurDataEntrySetter, curFieldLinker, curFormConfiguration))
                {

                    if (curFinalizer.finish())
                    {
                        m_success = true;
                        xDialog.endExecute();
                        return true;
                    }
                }
            }
            else
            {
                String smessage = JavaTools.replaceSubString(serrFormNameexists, sNewFormName, "%FORMNAME");
                showMessageBox("WarningBox", com.sun.star.awt.VclWindowPeerAttribute.OK, smessage);
            }
        }
        return false;
    }

    // @Override
    public void cancelWizard()
    {
        m_success = false;
        xDialog.endExecute();
    }

    public void insertFormRelatedSteps()
    {
        addRoadmap();
        int i = 0;
        i = insertRoadmapItem(0, true, m_oResource.getResText(UIConsts.RID_FORM + 80), SOMAIN_PAGE);
        i = insertRoadmapItem(i, false, m_oResource.getResText(UIConsts.RID_FORM + 81), SOSUBFORM_PAGE);
        i = insertRoadmapItem(i, false, m_oResource.getResText(UIConsts.RID_FORM + 82), SOSUBFORMFIELDS_PAGE);
        i = insertRoadmapItem(i, false, m_oResource.getResText(UIConsts.RID_FORM + 83), SOFIELDLINKER_PAGE);
        i = insertRoadmapItem(i, false, m_oResource.getResText(UIConsts.RID_FORM + 84), SOCONTROL_PAGE);
        i = insertRoadmapItem(i, false, m_oResource.getResText(UIConsts.RID_FORM + 85), SODATA_PAGE);
        i = insertRoadmapItem(i, false, m_oResource.getResText(UIConsts.RID_FORM + 86), SOSTYLE_PAGE);
        i = insertRoadmapItem(i, false, m_oResource.getResText(UIConsts.RID_FORM + 87), SOSTORE_PAGE);
        setRoadmapInteractive(true);
        setRoadmapComplete(true);
        setCurrentRoadmapItemID((short) 1);
    }

    public void start()
    {
        try
        {
            curFormDocument = new FormDocument(xMSF);
            if (curFormDocument.oMainFormDBMetaData.getConnection(m_wizardContext))
            {
                curFormDocument.oSubFormDBMetaData.getConnection(new PropertyValue[]
                        {
                            Properties.createProperty(PropertyNames.ACTIVE_CONNECTION, curFormDocument.oMainFormDBMetaData.DBConnection)
                        });
                curFormDocument.xProgressBar.setValue(20);
                buildSteps();
                this.curDBCommandFieldSelection.preselectCommand(m_wizardContext, false);
                XWindowPeer xWindowPeer2 = createWindowPeer(curFormDocument.xWindowPeer);
                curFormDocument.oMainFormDBMetaData.setWindowPeer(xWindowPeer2);
                insertFormRelatedSteps();
                short dialogReturn = executeDialog(curFormDocument.xFrame);
                xComponent.dispose();
                if ((dialogReturn == 0) && m_success)
                {
                    curFormDocument.oMainFormDBMetaData.addFormDocument(curFormDocument.xComponent);
                    loadSubComponent(DatabaseObject.FORM, FormName, m_openForEditing);
                }
            }
        }
        catch (java.lang.Exception jexception)
        {
            jexception.printStackTrace(System.err);
        }
        if ((!m_success) && (curFormDocument != null))
        {
            OfficeDocument.close(curFormDocument.xComponent);
        }
    }

    private boolean getFormResources()
    {
        sShowBinaryFields = m_oResource.getResText(UIConsts.RID_FORM + 2);
        slblTables = m_oResource.getResText(UIConsts.RID_FORM + 6);
        slblFields = m_oResource.getResText(UIConsts.RID_FORM + 12);
        slblSelFields = m_oResource.getResText(UIConsts.RID_FORM + 1);
        serrFormNameexists = m_oResource.getResText(UIConsts.RID_FORM + 98);

        return true;
    }

    public class FieldSelectionListener implements com.sun.star.wizards.ui.XFieldSelectionListener
    {

        protected int ID;

        // @Override
        public int getID()
        {
            return ID;
        }

        // @Override
        public void setID(String sIncSuffix)
        {
            ID = 1;
            if (sIncSuffix != null)
            {
                if ((!sIncSuffix.equals(PropertyNames.EMPTY_STRING)) && (!sIncSuffix.equals("_")))
                {
                    String sID = JavaTools.ArrayoutofString(sIncSuffix, "_")[1];
                    ID = Integer.parseInt(sID);
                    int a = 0;
                }
            }
        }

        // @Override
        public void shiftFromLeftToRight(String[] SelItems, String[] NewItems)
        {
            if (ID == 1)
            {
                toggleMainFormSteps();
            }
            else
            {
                toggleSubFormSteps();
            }
        }

        // @Override
        public void shiftFromRightToLeft(String[] SelItems, String[] NewItems)
        {
            // TODO When the ListFieldbox is refilled only fields of the current Command may be merged into the Listbox
            if (ID == 1)
            {
                toggleMainFormSteps();
            }
            else
            {
                toggleSubFormSteps();
            }
        }
        // @Override

        public void moveItemDown(String item)
        {
        }

        // @Override
        public void moveItemUp(String item)
        {
        }

        private boolean toggleSubFormSteps()
        {
            curSubFormFieldSelection.setModified(true);
            boolean benabled = curSubFormFieldSelection.getSelectedFieldNames().length > 0;
            enablefromStep(SOFIELDLINKER_PAGE, benabled);
            if (benabled)
                curFieldLinker.enable(!curFormConfiguration.areexistingRelationsdefined());
            return benabled;
        }

        private void toggleMainFormSteps()
        {
            curDBCommandFieldSelection.setModified(true);
            boolean enabled = curDBCommandFieldSelection.getSelectedFieldNames().length > 0;
            enablefromStep(SOSUBFORM_PAGE, enabled);
            setControlProperty("btnWizardNext", PropertyNames.PROPERTY_ENABLED, enabled);
            if (enabled)
            {
                if (curFormConfiguration.hasSubForm())
                {
                    enabled = toggleSubFormSteps();
                }
                else
                {
                    setStepEnabled(SOSUBFORMFIELDS_PAGE, false);
                    setStepEnabled(SOFIELDLINKER_PAGE, false);
                }
            }
            setControlProperty("btnWizardFinish", PropertyNames.PROPERTY_ENABLED, enabled);
        }
    }
}
