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
package com.sun.star.wizards.document;

import com.sun.star.awt.Point;
import com.sun.star.awt.Size;
import com.sun.star.awt.XControlModel;
import com.sun.star.awt.XDevice;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.NoSuchElementException;
import com.sun.star.container.XChild;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNameContainer;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.view.XControlAccess;
import com.sun.star.wizards.common.*;

import com.sun.star.sdbc.DataType;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.Exception;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.XInterface;
import com.sun.star.drawing.XControlShape;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPageSupplier;
import com.sun.star.drawing.XShape;
import com.sun.star.drawing.XShapeGrouper;
import com.sun.star.drawing.XShapes;
import com.sun.star.form.XFormsSupplier;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.container.XNamed;
import java.util.logging.Level;
import java.util.logging.Logger;

public class FormHandler
{
    private static final String CHECKBOX = "CheckBox";
    private static final String DATEFIELD = "DateField";
    private static final String FORMATTEDFIELD = "FormattedField";
    private static final String TEXTFIELD = "TextField";
    private static final String TIMEFIELD = "TimeField";

    private XFormsSupplier xFormsSupplier;
    public XMultiServiceFactory xMSFDoc;
    public XMultiServiceFactory xMSF;
    public XDrawPage xDrawPage;
    public String[] sModelServices = new String[8];
    public static ControlData[] oControlData;

    public static final int SOLABEL = 0;
    public static final int SOTEXTBOX = 1;
    public static final int SOCHECKBOX = 2;
    public static final int SODATECONTROL = 3;
    public static final int SOTIMECONTROL = 4;
    public static final int SONUMERICCONTROL = 5;
    public static final int SOGRIDCONTROL = 6;
    public static final int SOIMAGECONTROL = 7;
    public static final int SODATETIMECONTROL = 8;
    public static String SOSIZETEXT = "The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog.";
    private int iXPixelFactor = -1;
    private int iYPixelFactor = -1;
    private int iXNirwanaPos = 50000;
    private int iYNirwanaPos = 50000;
    private int nLabelHeight = -1;
    private int nDBRefHeight = -1;
    private int BasicLabelDiffHeight = -1;
    private XNameAccess xNamedForms;
    XControlAccess xControlAccess;
    XShapeGrouper xShapeGrouper;

    public static class ControlData
    {
        int DataType;
        private int ControlType;
        String GridColumnName;
    }

    /** Creates a new instance of FormHandler */
    public FormHandler(XMultiServiceFactory _xMSF, XTextDocument xTextDocument)
    {
        this.xMSF = _xMSF;
        XDrawPageSupplier xDrawPageSupplier = UnoRuntime.queryInterface(XDrawPageSupplier.class, xTextDocument);
        xDrawPage = xDrawPageSupplier.getDrawPage();
        xFormsSupplier = UnoRuntime.queryInterface(XFormsSupplier.class, xDrawPage);
        xShapeGrouper = UnoRuntime.queryInterface(XShapeGrouper.class, xDrawPage);
        xControlAccess = UnoRuntime.queryInterface(XControlAccess.class, xTextDocument.getCurrentController());
        xMSFDoc = UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDocument);
        sModelServices[SOLABEL] = "com.sun.star.form.component.FixedText";
        sModelServices[SOTEXTBOX] = "com.sun.star.form.component.TextField";
        sModelServices[SOCHECKBOX] = "com.sun.star.form.component.CheckBox";
        sModelServices[SODATECONTROL] = "com.sun.star.form.component.DateField";
        sModelServices[SOTIMECONTROL] = "com.sun.star.form.component.TimeField";
        sModelServices[SONUMERICCONTROL] = "com.sun.star.form.component.FormattedField";
        sModelServices[SOGRIDCONTROL] = "com.sun.star.form.component.GridControl";
        sModelServices[SOIMAGECONTROL] = "com.sun.star.form.component.DatabaseImageControl";

        oControlData = new ControlData[22];
        oControlData[0] = createControlData(DataType.BIT,          SOCHECKBOX, CHECKBOX);
        oControlData[1] = createControlData(DataType.BOOLEAN,      SOCHECKBOX, CHECKBOX);
        oControlData[2] = createControlData(DataType.TINYINT,      SONUMERICCONTROL, FORMATTEDFIELD);
        oControlData[3] = createControlData(DataType.SMALLINT,     SONUMERICCONTROL, FORMATTEDFIELD);
        oControlData[4] = createControlData(DataType.INTEGER,      SONUMERICCONTROL, FORMATTEDFIELD);
        oControlData[5] = createControlData(DataType.BIGINT,       SONUMERICCONTROL, FORMATTEDFIELD);
        oControlData[6] = createControlData(DataType.FLOAT,        SONUMERICCONTROL, FORMATTEDFIELD);
        oControlData[7] = createControlData(DataType.REAL,         SONUMERICCONTROL, FORMATTEDFIELD);
        oControlData[8] = createControlData(DataType.DOUBLE,       SONUMERICCONTROL, FORMATTEDFIELD);
        oControlData[9] = createControlData(DataType.NUMERIC,      SONUMERICCONTROL, FORMATTEDFIELD);
        oControlData[10] = createControlData(DataType.DECIMAL,     SONUMERICCONTROL, FORMATTEDFIELD);
        oControlData[11] = createControlData(DataType.CHAR,        SOTEXTBOX, TEXTFIELD);
        oControlData[12] = createControlData(DataType.VARCHAR,     SOTEXTBOX, TEXTFIELD);
        oControlData[13] = createControlData(DataType.LONGVARCHAR, SOTEXTBOX, TEXTFIELD);
        oControlData[14] = createControlData(DataType.DATE,        SODATECONTROL, DATEFIELD);
        oControlData[15] = createControlData(DataType.TIME,        SOTIMECONTROL, TIMEFIELD);
        oControlData[16] = createControlData(DataType.TIMESTAMP,   SODATECONTROL, TEXTFIELD);
        oControlData[17] = createControlData(DataType.BINARY,      SOIMAGECONTROL, TEXTFIELD);
        oControlData[18] = createControlData(DataType.VARBINARY,   SOIMAGECONTROL, TEXTFIELD);
        oControlData[19] = createControlData(DataType.LONGVARBINARY, SOIMAGECONTROL, TEXTFIELD);
        oControlData[20] = createControlData(DataType.BLOB,        SOIMAGECONTROL, TEXTFIELD);

        oControlData[21] = createControlData(DataType.OTHER,       SOIMAGECONTROL, TEXTFIELD);
    }

    public int getControlType(int _fieldtype)
    {
        for (int i = 0; i < oControlData.length; i++)
        {
            if (oControlData[i].DataType == _fieldtype)
            {
                return oControlData[i].ControlType;
            }
        }
        return -1;
    }

    private void initializeBasicControlValues()
    {
        Control oLabelControl = new Control(this, SOLABEL, new Point(), new Size());
        XDevice xDevice = UnoRuntime.queryInterface(XDevice.class, oLabelControl.xWindowPeer);
        iXPixelFactor = (int) (100000 / xDevice.getInfo().PixelPerMeterX);
        iYPixelFactor = (int) (100000 / xDevice.getInfo().PixelPerMeterY);

        nLabelHeight = (oLabelControl.getPreferredHeight("The quick brown fox...") + 1);
        Control oTextControl = new Control(this, SOTEXTBOX, new Point(), new Size());
        nDBRefHeight = (oTextControl.getPreferredHeight("The quick brown fox...") + 1);
        BasicLabelDiffHeight = (nDBRefHeight - nLabelHeight) / 2;
        xDrawPage.remove(oLabelControl.xShape);
        xDrawPage.remove(oTextControl.xShape);
    }

    private ControlData createControlData(int _datatype, int _controltype, String _gridcolumnname)
    {
        ControlData curControlData = new ControlData();
        curControlData.DataType = _datatype;
        curControlData.ControlType = _controltype;
        curControlData.GridColumnName = _gridcolumnname;
        return curControlData;
    }

    public XNameContainer getDocumentForms()
    {
        return xFormsSupplier.getForms();
    }

    public String getValueofHiddenControl(XNameAccess xNamedForm, String ControlName)
    {
        String value = "";
        try
        {
            if (xNamedForm.hasByName(ControlName))
            {
                value = AnyConverter.toString(com.sun.star.wizards.common.Helper.getUnoPropertyValue(xNamedForm.getByName(ControlName), "HiddenValue"));
            }
        }
        catch (Exception ex)
        {
            Logger.getLogger( FormHandler.class.getName() ).log( Level.SEVERE, null, ex );
        }
        return value;
    }

    public void insertHiddenControl(XNameAccess xNameAccess, XNameContainer xNamedForm, String ControlName, String ControlValue)
    {
        try
        {
            XInterface xHiddenControl;
            if (xNameAccess.hasByName(ControlName))
            {
                xHiddenControl = (XInterface) AnyConverter.toObject(new Type(XInterface.class), xNameAccess.getByName(ControlName));
            }
            else
            {
                xHiddenControl = (XInterface) xMSFDoc.createInstance("com.sun.star.form.component.HiddenControl");
                xNamedForm.insertByName(ControlName, xHiddenControl);
            }
            Helper.setUnoPropertyValue(xHiddenControl, "HiddenValue", ControlValue);
        }
        catch (Exception ex)
        {
            Logger.getLogger(FormHandler.class.getName()).log(Level.SEVERE, null, ex);
        }
    }

    private boolean hasFormByName(String _FormName)
    {
        XNameContainer xNamedFormContainer = getDocumentForms();
        xNamedForms = UnoRuntime.queryInterface(XNameAccess.class, xNamedFormContainer);
        return xNamedForms.hasByName(_FormName);
    }

    public void removeControlsofForm(String _FormName)
    {
        try
        {
            for (int i = xDrawPage.getCount() - 1; i >= 0; i--)
            {
                if (belongsToForm(xDrawPage.getByIndex(i), _FormName))
                {
                    XShape xShape = UnoRuntime.queryInterface(XShape.class, xDrawPage.getByIndex(i));
                    xDrawPage.remove(xShape);
                }
            }
        }
        catch (Exception e)
        {
            Logger.getLogger(FormHandler.class.getName()).log(Level.SEVERE, null, e);
        }
    }

    public void removeElement( XNameContainer _parentContainer, String _formName )
    {
        try
        {
            _parentContainer.removeByName( _formName );
        }
        catch ( WrappedTargetException e )
        {
            Logger.getLogger(FormHandler.class.getName()).log(Level.SEVERE, null, e);
        }
        catch( final NoSuchElementException e )
        {
            Logger.getLogger(FormHandler.class.getName()).log(Level.SEVERE, null, e);
        }
    }

    private boolean belongsToForm(Object _oDrawPageElement, String _FormName)
    {
        XServiceInfo xServiceInfo = UnoRuntime.queryInterface(XServiceInfo.class, _oDrawPageElement);
        while (xServiceInfo.supportsService("com.sun.star.drawing.GroupShape"))
        {
            XShapes xShapes = UnoRuntime.queryInterface(XShapes.class, _oDrawPageElement);
            try
            {
                _oDrawPageElement = xShapes.getByIndex(0);
                xServiceInfo = UnoRuntime.queryInterface(XServiceInfo.class, _oDrawPageElement);
            }
            catch(final com.sun.star.uno.Exception e)
            {
                return false;
            }
        }
        if (xServiceInfo.supportsService("com.sun.star.drawing.ControlShape"))
        {
            XControlShape xControlShape = UnoRuntime.queryInterface(XControlShape.class, _oDrawPageElement);
            XControlModel xControlModel = xControlShape.getControl();
            xServiceInfo = UnoRuntime.queryInterface(XServiceInfo.class, xControlShape.getControl());
            if (xServiceInfo.supportsService("com.sun.star.form.FormComponent"))
            {
                XChild xChild = UnoRuntime.queryInterface(XChild.class, xControlModel);
                XNamed xNamed = UnoRuntime.queryInterface(XNamed.class, xChild.getParent());
                String sName = xNamed.getName();
                return _FormName.equals(sName);
            }
        }
        return false;
    }

    public XNameContainer insertFormbyName(String _FormName, XNameContainer _xNamedFormContainer)
    {
        try
        {
            Object oDBForm;
            if (!hasFormByName(_FormName))
            {
                oDBForm = xMSFDoc.createInstance("com.sun.star.form.component.Form");
                _xNamedFormContainer.insertByName(_FormName, oDBForm);
                XNameContainer xNamedForm;
                xNamedForm = UnoRuntime.queryInterface(XNameContainer.class, oDBForm);
                return xNamedForm;
            }
            else
            {
                return getFormByName(_FormName);
            }
        }
        catch (com.sun.star.uno.Exception e)
        {
            Logger.getLogger(FormHandler.class.getName()).log(Level.SEVERE, null, e);
            return null;
        }
    }

    public XNameContainer insertFormbyName(String _FormName)
    {
        return insertFormbyName(_FormName, getDocumentForms());
    }

    private XNameContainer getFormByName(String _sname)
    {
        XNameContainer xNamedForm = null;
        try
        {
            if (xNamedForms.hasByName(_sname))
            {
                Object oDBForm = AnyConverter.toObject(new Type(XInterface.class), Helper.getUnoObjectbyName(xNamedForms, _sname));
                xNamedForm = UnoRuntime.queryInterface(XNameContainer.class, oDBForm);
            }
        }
        catch (IllegalArgumentException e)
        {
            Logger.getLogger(FormHandler.class.getName()).log(Level.SEVERE, null, e);
        }
        return xNamedForm;
    }

    public int getXPixelFactor()
    {
        if (iXPixelFactor == -1)
        {
            initializeBasicControlValues();
        }
        return iXPixelFactor;
    }

    public int getYPixelFactor()
    {
        if (iYPixelFactor == -1)
        {
            initializeBasicControlValues();
        }
        return iYPixelFactor;
    }

    // Note: as Shapes cannot be removed from the DrawPage without destroying
    // the object we have to park them somewhere beyond the visible area of the page
    public void moveShapesToNirwana(Control[] ControlList)
    {
        if (ControlList != null)
        {
            for (int i = 0; i < ControlList.length; i++)
            {
                if (ControlList[i] != null)
                {
                    ControlList[i].setPosition(new Point(this.iXNirwanaPos, this.iYNirwanaPos));
                }
            }
        }
    }

    /**
     * By removing the shape the whole control is disposed too
     *
     */
    public void removeShape(XShape _xShape)
    {
        xDrawPage.remove(_xShape);
        XComponent xComponent = UnoRuntime.queryInterface(XComponent.class, _xShape);
        xComponent.dispose();
    }

    public XShape groupShapesTogether(XMultiServiceFactory _xMSF, XShape _xLabelShape, XShape _xControlShape)
    {
        try
        {
            Object oGroupShape = _xMSF.createInstance("com.sun.star.drawing.ShapeCollection");
            XShapes xShapes = UnoRuntime.queryInterface(XShapes.class, oGroupShape);
            xShapes.add(_xLabelShape);
            xShapes.add(_xControlShape);
            return this.xShapeGrouper.group(xShapes);
        }
        catch (Exception e)
        {
            Logger.getLogger(FormHandler.class.getName()).log(Level.SEVERE, null, e);
            return null;
        }
    }

    public int getBasicLabelDiffHeight()
    {
        if (this.BasicLabelDiffHeight == -1)
        {
            initializeBasicControlValues();
        }
        return BasicLabelDiffHeight;
    }

    public int getControlReferenceHeight()
    {
        if (this.nDBRefHeight == -1)
        {
            initializeBasicControlValues();
        }
        return nDBRefHeight;
    }

    public int getLabelHeight()
    {
        if (this.nLabelHeight == -1)
        {
            initializeBasicControlValues();
        }
        return nLabelHeight;
    }

    public void setDrawObjectsCaptureMode(boolean _bCaptureObjects)
    {
        try
        {
            XPropertySet xPropertySet = UnoRuntime.queryInterface(XPropertySet.class, this.xMSFDoc.createInstance("com.sun.star.text.DocumentSettings"));
            xPropertySet.setPropertyValue("DoNotCaptureDrawObjsOnPage", Boolean.valueOf(!_bCaptureObjects));
        }
        catch (Exception e)
        {
            Logger.getLogger(FormHandler.class.getName()).log(Level.SEVERE, null, e);
        }
    }
}
