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
package com.sun.star.wizards.form;

import java.util.Vector;

import com.sun.star.wizards.text.TextDocument;
import com.sun.star.awt.Point;
import com.sun.star.awt.Size;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameContainer;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.db.*;
import com.sun.star.wizards.common.*;
import com.sun.star.wizards.ui.*;
import com.sun.star.wizards.text.TextStyleHandler;
import com.sun.star.wizards.text.ViewHandler;
import com.sun.star.wizards.document.Control;
import com.sun.star.wizards.document.DatabaseControl;
import com.sun.star.wizards.document.FormHandler;
import com.sun.star.wizards.document.GridControl;

public class FormDocument extends TextDocument
{
    protected Vector oControlForms = new Vector();
    protected CommandMetaData oMainFormDBMetaData;
    protected CommandMetaData oSubFormDBMetaData;
    protected String[][] LinkFieldNames;

    private FormHandler oFormHandler;
    private ViewHandler oViewHandler;
    private TextStyleHandler oTextStyleHandler;
    private XPropertySet xPropPageStyle;
    private final int SOFORMGAP = 2000;
    private boolean bhasSubForm;
    private UIControlArranger curUIControlArranger;
    private StyleApplier curStyleApplier;
    private int nPageWidth;
    private int nPageHeight;
    private int nFormWidth;
    private int nFormHeight;
    private Point aMainFormPoint;
    private final static String SOMAINFORM = "MainForm";
    private final static String SOSUBFORM = "SubForm";

    public FormDocument(XMultiServiceFactory xMSF)
    {
        super(xMSF, new TextDocument.ModuleIdentifier("com.sun.star.sdb.FormDesign"), true);
        try
        {
            oFormHandler = new FormHandler(xMSF, xTextDocument);
            oFormHandler.setDrawObjectsCaptureMode(false);
            oTextStyleHandler = new TextStyleHandler(xMSFDoc, xTextDocument);
            oViewHandler = new ViewHandler(xMSFDoc, xTextDocument);
            oMainFormDBMetaData = new CommandMetaData(xMSF);// , CharLocale);
            oSubFormDBMetaData = new CommandMetaData(xMSF);// , CharLocale);
            ViewHandler oViewHandler = new ViewHandler(xMSF, xTextDocument);
            TextStyleHandler oTextStyleSupplier = new TextStyleHandler(xMSFDoc, xTextDocument);
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
            e.printStackTrace(System.out);
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
            xPropPageStyle.setPropertyValue("RightMargin", new Integer(nMargin));
            xPropPageStyle.setPropertyValue("LeftMargin", new Integer(nMargin));
            xPropPageStyle.setPropertyValue("TopMargin", new Integer(nMargin));
            xPropPageStyle.setPropertyValue("BottomMargin", new Integer(nMargin));
            aMainFormPoint = new Point(nMargin, nMargin);
            nFormWidth = (int) (0.8 * (double) nPageWidth) - 2 * nMargin;
            nFormHeight = (int) (0.65 * (double) nPageHeight) - 2 * nMargin;
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
        }
    }

    public void initialize(boolean _baddParentForm, boolean _bShouldHaveSubForm, boolean _bModifySubForm, Short _NBorderType)
    {
        bhasSubForm = _bShouldHaveSubForm;
        adjustPageStyle();
        if (_baddParentForm)
        {
            if (oControlForms.size() == 0)
            {
                final ControlForm aMainControlForm = new ControlForm(this, SOMAINFORM, aMainFormPoint, getMainFormSize(FormWizard.AS_GRID));
                oControlForms.addElement(aMainControlForm);
            }
            else
            {
                oFormHandler.removeControlsofForm(SOMAINFORM);
                ((ControlForm) oControlForms.get(0)).oFormController = null;
            }
            ((ControlForm) oControlForms.get(0)).initialize(curUIControlArranger.getSelectedArrangement(0), _NBorderType);
        }
        if (_bShouldHaveSubForm)
        {
            if (oControlForms.size() == 1)
            {
                adjustMainFormSize(_NBorderType);
                final ControlForm aSubControlForm = new ControlForm(this, SOSUBFORM, getSubFormPoint(), getSubFormSize());
                oControlForms.addElement(aSubControlForm);
                /* ((ControlForm) oControlForms.get(1))*/
                aSubControlForm.initialize(curUIControlArranger.getSelectedArrangement(1), _NBorderType);
            }
            else if (_bModifySubForm)
            {
                if (oControlForms.size() > 1)
                {
                    oFormHandler.removeControlsofForm(SOSUBFORM);
                    ((ControlForm) oControlForms.get(1)).oFormController = null;
                    ((ControlForm) oControlForms.get(1)).initialize(curUIControlArranger.getSelectedArrangement(1), _NBorderType);
                }
            }
        }
        else
        {
            ControlForm aMainForm = (ControlForm) oControlForms.get(0);
            // boolean bHasSubForm = aMainForm.xFormContainer.hasByName(SOSUBFORM);
            // WRONG if (oFormHandler.hasFormByName(SOSUBFORM))
            if (aMainForm.xFormContainer != null && aMainForm.xFormContainer.hasByName(SOSUBFORM))
            {
                oFormHandler.removeControlsofForm(SOSUBFORM);
                oFormHandler.removeElement( aMainForm.xFormContainer, SOSUBFORM );
                ((ControlForm) oControlForms.get(1)).oFormController = null;
                // aMainForm.xFormContainer = null; // .removeFormByName(SOSUBFORM);
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
        Size aMainFormSize = new Size(nFormWidth, nMainFormHeight);
        return aMainFormSize;
    }

    private Size getSubFormSize()
    {
//      int nSubFormHeight = (int) ((double)nFormHeight/2) - SOFORMGAP;
//      int nSubFormFieldCount = this.oSubFormDBMetaData.FieldNames.length;
//      int totfieldcount = oMainFormDBMetaData.FieldNames.length + nSubFormFieldCount;
        int nMainFormHeight = ((ControlForm) oControlForms.get(0)).getActualFormHeight();
        Size aSubFormSize = new Size(nFormWidth, nFormHeight - nMainFormHeight - SOFORMGAP);
        return aSubFormSize;
    }

    private Point getSubFormPoint()
    {
        ControlForm curMainControlForm = ((ControlForm) oControlForms.get(0));
        return new Point(curMainControlForm.aStartPoint.X,
                (curMainControlForm.aStartPoint.Y + curMainControlForm.getFormSize().Height + SOFORMGAP));
    }

    private void adjustMainFormSize(Short _NBorderType)
    {
        ControlForm oMainControlForm = (ControlForm) oControlForms.get(0);
        oMainControlForm.setFormSize(getMainFormSize(oMainControlForm.curArrangement));
        if (oMainControlForm.curArrangement == FormWizard.AS_GRID)
        {
            oMainControlForm.oGridControl.setSize(oMainControlForm.getFormSize());
        }
        else
        {
            oMainControlForm.oFormController.positionControls(oMainControlForm.curArrangement,
                    oMainControlForm.aStartPoint,
                    oMainControlForm.getFormSize(),
                    curUIControlArranger.getAlignValue(), _NBorderType);
        }
    }

    private void adjustSubFormPosSize(Short _NBorderType)
    {
        ControlForm oMainControlForm = (ControlForm) oControlForms.get(0);
        ControlForm oSubControlForm = (ControlForm) oControlForms.get(1);
        oSubControlForm.setFormSize(new Size(nFormWidth, (int) nFormHeight - oMainControlForm.getFormSize().Height));
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
            oSubControlForm.oFormController.positionControls(oSubControlForm.curArrangement, oSubControlForm.aStartPoint, oSubControlForm.getAvailableFormSize(), curUIControlArranger.getAlignValue(), _NBorderType);
        }
    }

    public ControlForm getControlFormByName(String _sname)
    {
        for (int i = 0; i < oControlForms.size(); i++)
        {
            ControlForm curControlForm = ((ControlForm) oControlForms.get(i));
            if (curControlForm.Name.equals(_sname))
            {
                return curControlForm;
            }
        }
        return null;
    }

    public ControlForm[] getControlForms()
    {
        ControlForm[] aResult = new ControlForm[oControlForms.size()];
        oControlForms.toArray( aResult );
        return aResult;
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
                if (sRefTableName.equals(""))
                {
                    LinkFieldNames = _curFieldLinker.getLinkFieldNames();
                }
                else
                {
                    LinkFieldNames = _curFieldLinker.getLinkFieldNames(_curFormConfiguration.getRelationController(), sRefTableName);
                }
                if (LinkFieldNames != null)
                {
                    if (LinkFieldNames.length > 0)
                    {
                        oSubControlForm.xPropertySet.setPropertyValue("DetailFields", LinkFieldNames[0]);
                        oSubControlForm.xPropertySet.setPropertyValue("MasterFields", LinkFieldNames[1]);
                        oSubControlForm.finalizeControls();
                        return true;
                    }
                }
                return false;
            }
            return true;
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
            return false;
        }
        finally
        {
            unlockallControllers();
        }
    }

    public class ControlForm
    {

        XNameContainer xFormContainer;
        GridControl oGridControl;
        FormControlArranger oFormController;
        int curArrangement;
        FormDocument oFormDocument;
        String Name;
        Point aStartPoint;
        private Size aFormSize;
        CommandMetaData oDBMetaData;
        XPropertySet xPropertySet;

        public ControlForm(FormDocument _oFormDocument, String _sname, Point _astartPoint, Size _aFormSize)
        {
            aStartPoint = _astartPoint;
            aFormSize = _aFormSize;
            oFormDocument = _oFormDocument;
            Name = _sname;
            if (_sname.equals(SOSUBFORM))
            {
                ControlForm oMainControlForm = ((ControlForm) oControlForms.get(0));
                xFormContainer = oFormHandler.insertFormbyName(_sname, oMainControlForm.xFormContainer);
            }
            else
            {
                xFormContainer = oFormHandler.insertFormbyName(_sname);
            }
            xPropertySet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xFormContainer);
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
            boolean badaptControlStyles = false;
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
                badaptControlStyles = true;
            }
            else
            {
                badaptControlStyles = !oFormController.areControlsexisting();
                oFormController.positionControls(_curArrangement, aStartPoint, getAvailableFormSize(), curUIControlArranger.getAlignValue(), _NBorderType);
            }
            if (badaptControlStyles)
            {
                curStyleApplier.applyStyle(false, true);
            }
            if ((Name.equals(SOMAINFORM)) && (oControlForms.size() > 1))
            {
                ControlForm curSubControlForm = ((ControlForm) oControlForms.get(1));
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

        public Size getFormSize()
        {
            return aFormSize;
        }

        private Size getAvailableFormSize()
        {
            if (this.Name.equals(SOMAINFORM))
            {
                setFormSize(getMainFormSize(curArrangement));
            }
            else
            {
                setFormSize(getSubFormSize());
            }
            return aFormSize;
        }

        public void setFormSize(Size _aSize)
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
                xPropertySet.setPropertyValue("Command", _oDBMetaData.getCommandName());
                xPropertySet.setPropertyValue("CommandType", new Integer(_oDBMetaData.getCommandType()));
                for (int i = 0; i < _aPropertySetList.length; i++)
                {
                    xPropertySet.setPropertyValue(_aPropertySetList[i].Name, _aPropertySetList[i].Value);
                }
            }
            catch (Exception e)
            {
                e.printStackTrace(System.out);
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
                e.printStackTrace(System.out);
            }
        }

        public void finalizeControls()
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
