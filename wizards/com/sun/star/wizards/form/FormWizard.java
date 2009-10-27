/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: FormWizard.java,v $
 * $Revision: 1.17 $
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
package com.sun.star.wizards.form;

import com.sun.star.awt.XWindowPeer;
import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XComponent;
import com.sun.star.wizards.common.*;
import com.sun.star.wizards.db.RelationController;
import com.sun.star.wizards.document.OfficeDocument;
import com.sun.star.wizards.ui.*;

public class FormWizard extends WizardDialog
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
    private String sShowBinaryFields = "";
    private String serrFormNameexists = "";
    public static final int SOMAIN_PAGE = 1;
    public static final int SOSUBFORM_PAGE = 2;
    public static final int SOSUBFORMFIELDS_PAGE = 3;
    public static final int SOFIELDLINKER_PAGE = 4;
    public static final int SOCONTROL_PAGE = 5;
    public static final int SODATA_PAGE = 6;
    public static final int SOSTYLE_PAGE = 7;
    public static final int SOSTORE_PAGE = 8;
    public static final int SOCOLUMNARLEFT = 1;
    public static final int SOCOLUMNARTOP = 2;
    public static final int SOGRID = 3;
    public static final int SOTOPJUSTIFIED = 4;
    private String slblTables;
    private boolean bFormOpenMode;
    private boolean bcreateForm = false;
    private String FormName;

    public FormWizard(XMultiServiceFactory xMSF)
    {
        super(xMSF, 34400);
        super.addResourceHandler("FormWizard", "dbw");
        Helper.setUnoPropertyValues(xDialogModel,
                new String[]
                {
                    "Height", "Moveable", "Name", "PositionX", "PositionY", "Step", "TabIndex", "Title", "Width"
                },
                new Object[]
                {
                    new Integer(210), Boolean.TRUE, "DialogForm", new Integer(102), new Integer(41), new Integer(1), new Short((short) 0), m_oResource.getResText(UIConsts.RID_FORM), new Integer(310)
                });
        drawNaviBar();
        if (getFormResources() == true)
        {
            setRightPaneHeaders(m_oResource, UIConsts.RID_FORM + 90, 8);
        }
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
            e.printStackTrace(System.out);
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
//              curFormDocument.oMainFormDBMetaData.setFieldNames(curDBCommandFieldSelection.getSelectedFieldNames());
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
                {}
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
                {}
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

/*
    public static void main(String args[])
    {

        String ConnectStr = "uno:socket,host=localhost,port=8100;urp;StarOffice.ServiceManager";      //localhost  ;Lo-1.Germany.sun.com; 10.16.65.155
        PropertyValue[] curproperties = null;
        try
        {
            XMultiServiceFactory xLocMSF = com.sun.star.wizards.common.Desktop.connect(ConnectStr);
            FormWizard CurFormWizard = new FormWizard(xLocMSF);
            if (xLocMSF != null)
            {
                System.out.println("Connected to " + ConnectStr);
                curproperties = new PropertyValue[1];
//            curproperties[0] = Properties.createProperty("DatabaseLocation", "file:///C:/Documents and Settings/bc93774.EHAM02-DEV/My Documents/MyHSQL.odb");
                curproperties[0] = Properties.createProperty("DatabaseLocation", "file:///C:/Documents and Settings/bc93774.EHAM02-DEV/My Documents/MyHSQL.odb");
                curproperties[0] = Properties.createProperty("DataSourceName", "MyHSQLDatabase");
 // file:///C:/Documents and Settings/bc93774.EHAM02-DEV/My Documents/myjapanesehsqldatasourceMyDocAssign.odb");
 // MyDBase; Mydbwizard2DocAssign.odb; MyDBase.odb, Mydbwizard2DocAssign.odb ; Mydbwizard2DocAssign.odb; NewAccessDatabase, MyDocAssign baseLocation ); "DataSourceName", "db1");
 //          /--/curproperties[0] = Properties.createProperty("DatabaseLocation", "file:///x:/bc/MyHSQL Database.odb"); //MyDBase; Mydbwizard2DocAssign.odb; MyDBase.odb, Mydbwizard2DocAssign.odb ; Mydbwizard2DocAssign.odb; NewAccessDatabase,  baseLocation ); "DataSourceName", "db1");
 //          curproperties[0] = Properties.createProperty("DataSourceName", "Bibliography");
 //          curproperties[0] = Properties.createProperty("DataSourceName", "Bibliography");
 // Bibliography*         CurTableWizard.startTableWizard(xLocMSF, curproperties);

                CurFormWizard.startFormWizard(xLocMSF, curproperties);
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
        }
    }
*/

    public void buildSteps() throws NoValidPathException
    {
        curDBCommandFieldSelection = new CommandFieldSelection(this, curFormDocument.oMainFormDBMetaData, 92, slblFields, slblSelFields, slblTables, true, 34411);
        curDBCommandFieldSelection.addFieldSelectionListener(new FieldSelectionListener());
        curFormDocument.xProgressBar.setValue(20);
        // Label Help Text
        insertLabel("lblBinaryHelpText",
                new String[]
                {
                    "Height", "Label", "MultiLine", "PositionX", "PositionY", "Step", "Width"
                },
                new Object[]
                {
                    new Integer(28), sShowBinaryFields, Boolean.TRUE, new Integer(95), new Integer(154), new Integer(SOMAIN_PAGE), new Integer(210)
                });

        curFormConfiguration = new FormConfiguration(this);
        curFormDocument.xProgressBar.setValue(30);

        curSubFormFieldSelection = new CommandFieldSelection(this, curFormDocument.oSubFormDBMetaData, SOSUBFORMFIELDS_PAGE, 92, slblFields, slblSelFields, slblTables, true, 34431);
        curSubFormFieldSelection.addFieldSelectionListener(new FieldSelectionListener());
        insertLabel("lblSubFormBinaryHelpText",
                new String[]
                {
                    "Height", "Label", "MultiLine", "PositionX", "PositionY", "Step", "Width"
                },
                new Object[]
                {
                    new Integer(28), sShowBinaryFields, Boolean.TRUE, new Integer(95), new Integer(154), new Integer(SOSUBFORMFIELDS_PAGE), new Integer(210)
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
    public void finishWizard()
    {
        int ncurStep = getCurrentStep();
        if ((switchToStep(ncurStep, SOSTORE_PAGE)) || (ncurStep == SOSTORE_PAGE))
        {
            this.curFinalizer.initialize(curDBCommandFieldSelection.getSelectedCommandName(), this.curFormDocument);
            String sNewFormName = curFinalizer.getName();
            if (!curFormDocument.oMainFormDBMetaData.hasFormDocumentByName(sNewFormName))
            {
                bFormOpenMode = curFinalizer.getOpenMode();
                FormName = curFinalizer.getName();
                if (curFormDocument.finalizeForms(CurDataEntrySetter, curFieldLinker, curFormConfiguration))
                {

                    if (curFinalizer.finish())
                    {
                        bcreateForm = true;
                        xDialog.endExecute();
                    }
                }
            }
            else
            {
                String smessage = JavaTools.replaceSubString(serrFormNameexists, sNewFormName, "%FORMNAME");
                showMessageBox("WarningBox", com.sun.star.awt.VclWindowPeerAttribute.OK, smessage);
            }
        }
    }

    // @Override
    public void cancelWizard()
    {
        bcreateForm = false;
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

    public XComponent[] startFormWizard(XMultiServiceFactory _xMSF, PropertyValue[] CurPropertyValue)
    {
        XComponent[] ret = null;
        try
        {
            curFormDocument = new FormDocument(xMSF);
            if (curFormDocument.oMainFormDBMetaData.getConnection(CurPropertyValue))
            {
                curFormDocument.oSubFormDBMetaData.getConnection(new PropertyValue[]
                        {
                            Properties.createProperty("ActiveConnection", curFormDocument.oMainFormDBMetaData.DBConnection)
                        });
                curFormDocument.xProgressBar.setValue(20);
                // SystemDialog.showMessageBox(xMSF, "FormName", 0, "slblFields");
                buildSteps();
                // SystemDialog.showMessageBox(xMSF, "FormName", 0, "slblFields");
                this.curDBCommandFieldSelection.preselectCommand(CurPropertyValue, false);
                XWindowPeer xWindowPeer2 = createWindowPeer(curFormDocument.xWindowPeer);
                curFormDocument.oMainFormDBMetaData.setWindowPeer(xWindowPeer2 /* xControl.getPeer() */ );
                //      setAutoMnemonic("lblDialogHeader", false);
                insertFormRelatedSteps();
                short RetValue = executeDialog(curFormDocument.xFrame);
                xComponent.dispose();
                if (bcreateForm)
                {
                    curFormDocument.oMainFormDBMetaData.addFormDocument(curFormDocument.xComponent);
                    ret = curFormDocument.oMainFormDBMetaData.openFormDocument(FormName, bFormOpenMode);
                }
            }
        }
        catch (java.lang.Exception jexception)
        {
            jexception.printStackTrace(System.out);
        }
        if ((!bcreateForm) && (curFormDocument != null))
        {
            OfficeDocument.close(curFormDocument.xComponent);
        }
        return ret;
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
                if ((!sIncSuffix.equals("")) && (!sIncSuffix.equals("_")))
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
            boolean benable = curSubFormFieldSelection.getSelectedFieldNames().length > 0;
            enablefromStep(SOFIELDLINKER_PAGE, benable);
            if (benable)
            {
                curFieldLinker.enable(!curFormConfiguration.areexistingRelationsdefined());
            }
            return benable;
        }

        private void toggleMainFormSteps()
        {
            curDBCommandFieldSelection.setModified(true);
            boolean benable = curDBCommandFieldSelection.getSelectedFieldNames().length > 0;
            enablefromStep(SOSUBFORM_PAGE, benable);
            setControlProperty("btnWizardNext", "Enabled", new Boolean(benable));
            if (benable)
            {
                if (curFormConfiguration.hasSubForm())
                {
                    benable = toggleSubFormSteps();
                }
                else
                {
                    setStepEnabled(SOSUBFORMFIELDS_PAGE, false);
                    setStepEnabled(SOFIELDLINKER_PAGE, false);
                }
            }
            setControlProperty("btnWizardFinish", "Enabled", new Boolean(benable));
        }
    }
}





