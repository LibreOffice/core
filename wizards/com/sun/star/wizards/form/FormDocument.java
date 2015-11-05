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

import java.util.ArrayList;

import com.sun.star.awt.Point;
import com.sun.star.awt.Size;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameContainer;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.db.CommandMetaData;
import com.sun.star.wizards.document.Control;
import com.sun.star.wizards.document.DatabaseControl;
import com.sun.star.wizards.document.FormHandler;
import com.sun.star.wizards.document.GridControl;
import com.sun.star.wizards.text.TextDocument;
import com.sun.star.wizards.text.TextStyleHandler;
import com.sun.star.wizards.text.ViewHandler;

public class FormDocument extends TextDocument
{
    protected ArrayList<ControlForm> oControlForms = new ArrayList<ControlForm>();
    protected CommandMetaData oMainFormDBMetaData;
    protected CommandMetaData oSubFormDBMetaData;
    protected String[][] LinkFieldNames;

    private FormHandler oFormHandler;
    private XPropertySet xPropPageStyle;
    private static final int SOFORMGAP = 2000;
    private boolean bhasSubForm;
    private UIControlArranger curUIControlArranger;
    private StyleApplier curStyleApplier;
    private int nPageWidth;
    private int nPageHeight;
    private int nFormWidth;
    private int nFormHeight;
    private Point aMainFormPoint;
    private static final String SOMAINFORM = "MainForm";
    private static final String SOSUBFORM = "SubForm";

    public FormDocument(XMultiServiceFactory xMSF)
    {
        super(xMSF, new TextDocument.ModuleIdentifier("com.sun.star.sdb.FormDesign"), true);
        try
        {
            oFormHandler = new FormHandler(xMSF, xTextDocument);
            oFormHandler.setDrawObjectsCaptureMode(false);
            TextStyleHandler oTextStyleHandler = new TextStyleHandler(xTextDocument);
            new ViewHandler(xTextDocument);
            oMainFormDBMetaData = new CommandMetaData(xMSF);// , CharLocale);
            oSubFormDBMetaData = new CommandMetaData(xMSF);// , CharLocale);
            ViewHandler oViewHandler = new ViewHandler(xTextDocument);
            TextStyleHandler oTextStyleSupplier = new TextStyleHandler(xTextDocument);
            Helper.setUnoPropertyValue(xTextDocument, "ApplyFormDesignMode", Boolean.FALSE);
            oViewHandler.setViewSetting("ShowTableBoundaries", Boolean.FALSE);
            oViewHandler.setViewSetting("ShowOnlineLayout", Boolean.TRUE);
            xPropPageStyle = oTextStyleSupplier.getStyleByName("PageStyles", "Standard");
            Size aSize = oTextStyleHandler.changePageAlignment(xPropPageStyle, true);
            nPageWidth = aSize.Width;
            nPageHeight = aSize.Height;
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
        }
    }

    public void addUIFormController(UIControlArranger _curUIControlArranger)
    {
        this.curUIControlArranger = _curUIControlArranger;
    }

    public void addStyleApplier(StyleApplier _curStyleApplier)
    {
        this.curStyleApplier = _curStyleApplier;
    }

    private String getDataSourceName()
    {
        return this.oMainFormDBMetaData.DataSourceName;
    }

    private void adjustPageStyle()
    {
        try
        {
            int nMargin;
            int totfieldcount = getMainFieldCount() + getSubFieldCount();
            if (totfieldcount > 30)
            {
                nMargin = 500;
            }
            else if (totfieldcount > 20)
            {
                nMargin = 750;
            }
            else
            {
                nMargin = 1000;
            }
            xPropPageStyle.setPropertyValue("RightMargin", Integer.valueOf(nMargin));
            xPropPageStyle.setPropertyValue("LeftMargin", Integer.valueOf(nMargin));
            xPropPageStyle.setPropertyValue("TopMargin", Integer.valueOf(nMargin));
            xPropPageStyle.setPropertyValue("BottomMargin", Integer.valueOf(nMargin));
            aMainFormPoint = new Point(nMargin, nMargin);
            nFormWidth = (int) (0.8 * nPageWidth) - 2 * nMargin;
            nFormHeight = (int) (0.65 * nPageHeight) - 2 * nMargin;
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
        }
    }

    public void initialize(boolean _baddParentForm, boolean _bShouldHaveSubForm, boolean _bModifySubForm, Short _NBorderType)
    {
        bhasSubForm = _bShouldHaveSubForm;
        adjustPageStyle();
        if (_baddParentForm)
        {
            if (oControlForms.isEmpty())
            {
                final ControlForm aMainControlForm = new ControlForm(this, SOMAINFORM, aMainFormPoint, getMainFormSize(FormWizard.AS_GRID));
                oControlForms.add(aMainControlForm);
            }
            else
            {
                oFormHandler.removeControlsofForm(SOMAINFORM);
                oControlForms.get(0).oFormController = null;
            }
            oControlForms.get(0).initialize(curUIControlArranger.getSelectedArrangement(0), _NBorderType);
        }
        if (_bShouldHaveSubForm)
        {
            if (oControlForms.size() == 1)
            {
                adjustMainFormSize(_NBorderType);
                final ControlForm aSubControlForm = new ControlForm(this, SOSUBFORM, getSubFormPoint(), getSubFormSize());
                oControlForms.add(aSubControlForm);
                /* ((ControlForm) oControlForms.get(1))*/
                aSubControlForm.initialize(curUIControlArranger.getSelectedArrangement(1), _NBorderType);
            }
            else if (_bModifySubForm)
            {
                if (oControlForms.size() > 1)
                {
                    oFormHandler.removeControlsofForm(SOSUBFORM);
                    oControlForms.get(1).oFormController = null;
                    oControlForms.get(1).initialize(curUIControlArranger.getSelectedArrangement(1), _NBorderType);
                }
            }
        }
        else
        {
            ControlForm aMainForm = oControlForms.get(0);
            // WRONG if (oFormHandler.hasFormByName(SOSUBFORM))
            if (aMainForm.xFormContainer != null && aMainForm.xFormContainer.hasByName(SOSUBFORM))
            {
                oFormHandler.removeControlsofForm(SOSUBFORM);
                oFormHandler.removeElement( aMainForm.xFormContainer, SOSUBFORM );
                oControlForms.get(1).oFormController = null;
                oControlForms.remove(1);
                adjustMainFormSize(_NBorderType);
            }
        }
    }

    private int getMainFieldCount()
    {
        return oMainFormDBMetaData.getFieldNames().length;
    }
    private int getSubFieldCount()
    {
        return oSubFormDBMetaData.getFieldNames().length;
    }

    private Size getMainFormSize(int _curArrangement)
    {
        int nMainFormHeight = nFormHeight;
        if (bhasSubForm)
        {
            if (_curArrangement == FormWizard.AS_GRID)
            {
                nMainFormHeight = (int) ((double) (nFormHeight - SOFORMGAP) / 2);
            }
            else
            {
                int nTotalFieldCount = getMainFieldCount() + getSubFieldCount();
                nMainFormHeight = (int) (((double) getMainFieldCount() / (double) nTotalFieldCount) * ((double) (nFormHeight - SOFORMGAP) / 2));
            }
        }
        return new Size(nFormWidth, nMainFormHeight);
    }

    private Size getSubFormSize()
    {
        int nMainFormHeight = oControlForms.get(0).getActualFormHeight();
        return new Size(nFormWidth, nFormHeight - nMainFormHeight - SOFORMGAP);
    }

    private Point getSubFormPoint()
    {
        ControlForm curMainControlForm = oControlForms.get(0);
        return new Point(curMainControlForm.aStartPoint.X,
                (curMainControlForm.aStartPoint.Y + curMainControlForm.getFormSize().Height + SOFORMGAP));
    }

    private void adjustMainFormSize(Short _NBorderType)
    {
        ControlForm oMainControlForm = oControlForms.get(0);
        oMainControlForm.setFormSize(getMainFormSize(oMainControlForm.curArrangement));
        if (oMainControlForm.curArrangement == FormWizard.AS_GRID)
        {
            oMainControlForm.oGridControl.setSize(oMainControlForm.getFormSize());
        }
        else
        {
            oMainControlForm.oFormController.positionControls(oMainControlForm.curArrangement,
                    oMainControlForm.aStartPoint,
                    curUIControlArranger.getAlignValue(),
                    _NBorderType);
        }
    }

    private void adjustSubFormPosSize(Short _NBorderType)
    {
        ControlForm oMainControlForm = oControlForms.get(0);
        ControlForm oSubControlForm = oControlForms.get(1);
        oSubControlForm.setFormSize(new Size(nFormWidth, nFormHeight - oMainControlForm.getFormSize().Height));
        if (oSubControlForm.curArrangement == FormWizard.AS_GRID)
        {
            Point aPoint = oSubControlForm.oGridControl.getPosition();
            int idiffheight = oSubControlForm.getEntryPointY() - oMainControlForm.getActualFormHeight() - oMainControlForm.aStartPoint.Y - SOFORMGAP;
            oSubControlForm.setStartPoint(new Point(aPoint.X, (aPoint.Y - idiffheight)));
            oSubControlForm.oGridControl.setPosition(oSubControlForm.aStartPoint);
            oSubControlForm.oGridControl.setSize(getSubFormSize());
        }
        else
        {
//          oSubControlForm.oFormController.adjustYPositions(_idiffheight);
            oSubControlForm.setStartPoint(new Point(oSubControlForm.aStartPoint.X, oMainControlForm.getActualFormHeight() + oMainControlForm.aStartPoint.Y + SOFORMGAP));
            oSubControlForm.oFormController.positionControls(oSubControlForm.curArrangement, oSubControlForm.aStartPoint, curUIControlArranger.getAlignValue(), _NBorderType);
        }
    }

    private ControlForm getControlFormByName(String _sname)
    {
        for (int i = 0; i < oControlForms.size(); i++)
        {
            ControlForm curControlForm = oControlForms.get(i);
            if (curControlForm.Name.equals(_sname))
            {
                return curControlForm;
            }
        }
        return null;
    }

    public boolean finalizeForms(DataEntrySetter _curDataEntrySetter, FieldLinker _curFieldLinker, FormConfiguration _curFormConfiguration)
    {
        try
        {
            this.xTextDocument.lockControllers();
            PropertyValue[] aFormProperties = _curDataEntrySetter.getFormProperties();
            ControlForm oMasterControlForm = getControlFormByName(SOMAINFORM);
            oMasterControlForm.setFormProperties(aFormProperties, oMainFormDBMetaData);
            oMasterControlForm.finalizeControls();
            if (oMasterControlForm.xFormContainer.hasByName(SOSUBFORM))
            {
                ControlForm oSubControlForm = getControlFormByName(SOSUBFORM);
                oSubControlForm.setFormProperties(aFormProperties, oSubFormDBMetaData);
                String sRefTableName = _curFormConfiguration.getreferencedTableName();
                if (sRefTableName.equals(PropertyNames.EMPTY_STRING))
                {
                    LinkFieldNames = _curFieldLinker.getLinkFieldNames();
                }
                else
                {
                    LinkFieldNames = _curFieldLinker.getLinkFieldNames(_curFormConfiguration.getRelationController(), sRefTableName);
                }
                if (LinkFieldNames != null && LinkFieldNames.length > 0)
                {
                    oSubControlForm.xPropertySet.setPropertyValue("DetailFields", LinkFieldNames[0]);
                    oSubControlForm.xPropertySet.setPropertyValue("MasterFields", LinkFieldNames[1]);
                    oSubControlForm.finalizeControls();
                    return true;
                }
                return false;
            }
            return true;
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
            return false;
        }
        finally
        {
            unlockallControllers();
        }
    }

    public class ControlForm
    {

        private XNameContainer xFormContainer;
        GridControl oGridControl;
        private FormControlArranger oFormController;
        private int curArrangement;
        private FormDocument oFormDocument;
        private String Name;
        private Point aStartPoint;
        private Size aFormSize;
        private CommandMetaData oDBMetaData;
        private XPropertySet xPropertySet;

        public ControlForm(FormDocument _oFormDocument, String _sname, Point _astartPoint, Size _aFormSize)
        {
            aStartPoint = _astartPoint;
            aFormSize = _aFormSize;
            oFormDocument = _oFormDocument;
            Name = _sname;
            if (_sname.equals(SOSUBFORM))
            {
                ControlForm oMainControlForm = oControlForms.get(0);
                xFormContainer = oFormHandler.insertFormbyName(_sname, oMainControlForm.xFormContainer);
            }
            else
            {
                xFormContainer = oFormHandler.insertFormbyName(_sname);
            }
            xPropertySet = UnoRuntime.queryInterface(XPropertySet.class, xFormContainer);
            if (_sname.equals(SOMAINFORM))
            {
                oDBMetaData = oFormDocument.oMainFormDBMetaData;
            }
            else
            {
                oDBMetaData = oFormDocument.oSubFormDBMetaData;
            }
        }

        public void initialize(int _curArrangement, Short _NBorderType)
        {
            boolean adaptControlStyles = false;
            xTextDocument.lockControllers();
            curArrangement = _curArrangement;
            if (oGridControl != null)
            {
                oFormHandler.xDrawPage.remove(oGridControl.xShape);
                oGridControl.xComponent.dispose();
                oGridControl = null;
            }
            if (oFormController == null)
            {
                oFormController = new FormControlArranger(oFormHandler, xFormContainer, oDBMetaData, xProgressBar, aStartPoint, aFormSize);
            }
            else
            {
                if (curArrangement == FormWizard.AS_GRID)
                {
                    oFormHandler.moveShapesToNirwana(getLabelControls());
                    oFormHandler.moveShapesToNirwana(getDatabaseControls());
                }
            }
            if (curArrangement == FormWizard.AS_GRID)
            {
                insertGridControl(_NBorderType);
                adaptControlStyles = true;
            }
            else
            {
                adaptControlStyles = !oFormController.areControlsexisting();
                oFormController.positionControls(_curArrangement, aStartPoint, curUIControlArranger.getAlignValue(), _NBorderType);
            }
            if (adaptControlStyles)
            {
                curStyleApplier.applyStyle(true);
            }
            if ((Name.equals(SOMAINFORM)) && (oControlForms.size() > 1))
            {
                ControlForm curSubControlForm = oControlForms.get(1);
                if (curSubControlForm != null)
                {
                    adjustSubFormPosSize(_NBorderType);
                }
            }
            setFormSize(new Size(aFormSize.Width, getActualFormHeight()));
            unlockallControllers();
        }

        public Control[] getLabelControls()
        {
            if (oFormController != null)
            {
                return oFormController.getLabelControlList();
            }
            else
            {
                return null;
            }
        }

        private Size getFormSize()
        {
            return aFormSize;
        }

        private void setFormSize(Size _aSize)
        {
            aFormSize = _aSize;
            oFormController.setFormSize(aFormSize);
        }

        private void setStartPoint(Point _aPoint)
        {
            aStartPoint = _aPoint;
            if (oFormController != null)
            {
                oFormController.setStartPoint(_aPoint);
            }
        }

        private int getActualFormHeight()
        {
            if (curArrangement == FormWizard.AS_GRID)
            {
                return oGridControl.xShape.getSize().Height;
            }
            else
            {
                return oFormController.getFormHeight();
            }
        }

        private int getEntryPointY()
        {
            if (curArrangement == FormWizard.AS_GRID)
            {
                return oGridControl.xShape.getPosition().Y;
            }
            else
            {
                return oFormController.getEntryPointY();
            }
        }

        private void setFormProperties(PropertyValue[] _aPropertySetList, CommandMetaData _oDBMetaData)
        {
            try
            {
                xPropertySet.setPropertyValue("DataSourceName", getDataSourceName());
                xPropertySet.setPropertyValue(PropertyNames.COMMAND, _oDBMetaData.getCommandName());
                xPropertySet.setPropertyValue(PropertyNames.COMMAND_TYPE, Integer.valueOf(_oDBMetaData.getCommandType()));
                for (int i = 0; i < _aPropertySetList.length; i++)
                {
                    xPropertySet.setPropertyValue(_aPropertySetList[i].Name, _aPropertySetList[i].Value);
                }
            }
            catch (Exception e)
            {
                e.printStackTrace(System.err);
            }
        }

        public DatabaseControl[] getDatabaseControls()
        {
            if (oFormController != null)
            {
                return oFormController.DBControlList;
            }
            else
            {
                return null;
            }
        }

        public GridControl getGridControl()
        {
            return oGridControl;
        }

        public int getArrangemode()
        {
            return curArrangement;
        }

        private void insertGridControl(Short _NBorderType)
        {
            try
            {
                curArrangement = FormWizard.AS_GRID;
                if (Name.equals(SOMAINFORM))
                {
                    oGridControl = new GridControl(xMSF, Name + "_Grid", oFormHandler, xFormContainer, oDBMetaData.FieldColumns, aStartPoint, getMainFormSize(FormWizard.AS_GRID));
                }
                else
                {
                    oGridControl = new GridControl(xMSF, Name + "_Grid", oFormHandler, xFormContainer, oDBMetaData.FieldColumns, aStartPoint, getSubFormSize());
                }
                oGridControl.xPropertySet.setPropertyValue(PropertyNames.PROPERTY_BORDER, _NBorderType);
            }
            catch (Exception e)
            {
                e.printStackTrace(System.err);
            }
        }

        private void finalizeControls()
        {
            Control[] oLabelControls = getLabelControls();
            Control[] oDBControls = getDatabaseControls();
            if (oLabelControls != null)
            {
                for (int i = 0; i < getLabelControls().length; i++)
                {
                    if (curArrangement == FormWizard.AS_GRID)
                    {
                        if ((oLabelControls[i] != null) && (oDBControls[i] != null))
                        {
                            oFormHandler.removeShape(oLabelControls[i].xShape);
                            oFormHandler.removeShape(oDBControls[i].xShape);
                        }
                    }
                    else
                    {
                        oFormHandler.groupShapesTogether(xMSF, oLabelControls[i].xShape, oDBControls[i].xShape);
                    }
                }
            }
        }
    }
}
