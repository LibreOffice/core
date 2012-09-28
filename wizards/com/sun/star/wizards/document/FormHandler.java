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
    private static final String IMAGECONTROL = "ImageControl";
    private static final String TEXTFIELD = "TextField";
    private static final String TIMEFIELD = "TimeField";

    public XFormsSupplier xFormsSupplier;
    public XMultiServiceFactory xMSFDoc;
    public XMultiServiceFactory xMSF;
    public XDrawPage xDrawPage;
    private XDrawPageSupplier xDrawPageSupplier;
    public String[] sModelServices = new String[8];
    public static ControlData[] oControlData;

    public final static int SOLABEL = 0;
    public final static int SOTEXTBOX = 1;
    public final static int SOCHECKBOX = 2;
    public final static int SODATECONTROL = 3;
    public final static int SOTIMECONTROL = 4;
    public final static int SONUMERICCONTROL = 5;
    public final static int SOGRIDCONTROL = 6;
    public final static int SOIMAGECONTROL = 7;
    public final static int SODATETIMECONTROL = 8;
    int iImageControlHeight = 2000;
    public static String SOSIZETEXT = "The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog.";
    int iXPixelFactor = -1;
    int iYPixelFactor = -1;
    int iXNirwanaPos = 50000;
    int iYNirwanaPos = 50000;
    public int nLabelHeight = -1;
    public int nDBRefHeight = -1;
    public int BasicLabelDiffHeight = -1;
    XNameAccess xNamedForms;
    XControlAccess xControlAccess;
    XShapeGrouper xShapeGrouper;
    XNameContainer xNamedFormContainer;

    public class ControlData
    {

        int DataType;
        int ControlType;
        String ControlService;
        String GridColumnName;
        boolean bIsText;
    }

    /** Creates a new instance of FormHandler */
    public FormHandler(XMultiServiceFactory _xMSF, XTextDocument xTextDocument)
    {
        this.xMSF = _xMSF;
        xDrawPageSupplier = UnoRuntime.queryInterface(XDrawPageSupplier.class, xTextDocument);
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
        oControlData[0] = createControlData(DataType.BIT,          SOCHECKBOX, CHECKBOX, CHECKBOX, false);
        oControlData[1] = createControlData(DataType.BOOLEAN,      SOCHECKBOX, CHECKBOX, CHECKBOX, false);
        oControlData[2] = createControlData(DataType.TINYINT,      SONUMERICCONTROL, FORMATTEDFIELD, FORMATTEDFIELD, false);
        oControlData[3] = createControlData(DataType.SMALLINT,     SONUMERICCONTROL, FORMATTEDFIELD, FORMATTEDFIELD, false);
        oControlData[4] = createControlData(DataType.INTEGER,      SONUMERICCONTROL, FORMATTEDFIELD, FORMATTEDFIELD, false);
        oControlData[5] = createControlData(DataType.BIGINT,       SONUMERICCONTROL, FORMATTEDFIELD, FORMATTEDFIELD, false);
        oControlData[6] = createControlData(DataType.FLOAT,        SONUMERICCONTROL, FORMATTEDFIELD, FORMATTEDFIELD, false);
        oControlData[7] = createControlData(DataType.REAL,         SONUMERICCONTROL, FORMATTEDFIELD, FORMATTEDFIELD, false);
        oControlData[8] = createControlData(DataType.DOUBLE,       SONUMERICCONTROL, FORMATTEDFIELD, FORMATTEDFIELD, false);
        oControlData[9] = createControlData(DataType.NUMERIC,      SONUMERICCONTROL, FORMATTEDFIELD, FORMATTEDFIELD, false);
        oControlData[10] = createControlData(DataType.DECIMAL,     SONUMERICCONTROL, FORMATTEDFIELD, FORMATTEDFIELD, false);
        oControlData[11] = createControlData(DataType.CHAR,        SOTEXTBOX, TEXTFIELD, TEXTFIELD, false);
        oControlData[12] = createControlData(DataType.VARCHAR,     SOTEXTBOX, TEXTFIELD, TEXTFIELD, true);
        oControlData[13] = createControlData(DataType.LONGVARCHAR, SOTEXTBOX, TEXTFIELD, TEXTFIELD, true);
        oControlData[14] = createControlData(DataType.DATE,        SODATECONTROL, DATEFIELD, DATEFIELD, false);
        oControlData[15] = createControlData(DataType.TIME,        SOTIMECONTROL, TIMEFIELD, TIMEFIELD, false);
        oControlData[16] = createControlData(DataType.TIMESTAMP,   SODATECONTROL, DATEFIELD, TEXTFIELD, false);
        // oImageControlData = new ControlData[4];
        oControlData[17] = createControlData(DataType.BINARY,      SOIMAGECONTROL, IMAGECONTROL, TEXTFIELD, false);
        oControlData[18] = createControlData(DataType.VARBINARY,   SOIMAGECONTROL, IMAGECONTROL, TEXTFIELD, false);
        oControlData[19] = createControlData(DataType.LONGVARBINARY, SOIMAGECONTROL, IMAGECONTROL, TEXTFIELD, false);
        oControlData[20] = createControlData(DataType.BLOB,        SOIMAGECONTROL, IMAGECONTROL, TEXTFIELD, false);

        oControlData[21] = createControlData(DataType.OTHER,       SOIMAGECONTROL, IMAGECONTROL, TEXTFIELD, false);
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

    public void setglobalMultiServiceFactory(XMultiServiceFactory _xMSF)
    {
        xMSF = _xMSF;
    }

    public String getModelServiceName(int _fieldtype)
    {
        int icontroltype = getControlType(_fieldtype);
        if (icontroltype > -1)
        {
            return sModelServices[icontroltype];
        }
        else
        {
            return null;
        }
    }

    public void initializeBasicControlValues()
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

    public ControlData createControlData(int _datatype, int _controltype, String _scontrolservicename, String _gridcolumnname, boolean _bIsTextControl)
    {
        ControlData curControlData = new ControlData();
        curControlData.DataType = _datatype;
        curControlData.ControlType = _controltype;
        curControlData.ControlService = _scontrolservicename;
        curControlData.GridColumnName = _gridcolumnname;
        curControlData.bIsText = _bIsTextControl;
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

    public boolean hasFormByName(String _FormName)
    {
        xNamedFormContainer = getDocumentForms();
        xNamedForms = UnoRuntime.queryInterface(XNameAccess.class, xNamedFormContainer);
        return xNamedForms.hasByName(_FormName);
    }

    public void removeFormByName(String _FormName)
    {
        try
        {
            if (hasFormByName(_FormName))
            {
                removeControlsofForm(_FormName);
                xNamedFormContainer.removeByName(_FormName);
            }
        }
        catch (com.sun.star.uno.Exception ex)
        {
            Logger.getLogger(FormHandler.class.getName()).log(Level.SEVERE, null, ex);
        }
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

    public boolean belongsToForm(Object _oDrawPageElement, String _FormName)
    {
        XServiceInfo xServiceInfo = UnoRuntime.queryInterface(XServiceInfo.class, _oDrawPageElement);
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

    public XNameContainer insertSubFormbyName(String _FormName, XNameContainer _xNamedFormContainer)
    {
        return insertFormbyName(_FormName, _xNamedFormContainer);
    }

    public XNameContainer insertFormbyName(String _FormName)
    {
        return insertFormbyName(_FormName, getDocumentForms());
    }

    public XNameContainer getFormByName(String _sname)
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

    /**
     * @return
     */
    public int getXPixelFactor()
    {
        if (iXPixelFactor == -1)
        {
            initializeBasicControlValues();
        }
        return iXPixelFactor;
    }

    /**
     * @return
     */
    public int getYPixelFactor()
    {
        if (iYPixelFactor == -1)
        {
            initializeBasicControlValues();
        }
        return iYPixelFactor;
    }

    /**
     * @param i
     */
    public void setXPixelFactor(int i)
    {
        iXPixelFactor = i;
    }

    /**
     * @param i
     */
    public void setYPixelFactor(int i)
    {
        iYPixelFactor = i;
    }

    /**
     * @return
     */
    public int getImageControlHeight()
    {
        return iImageControlHeight;
    }

    /**
     * @param i
     */
    public void setImageControlHeight(int i)
    {
        iImageControlHeight = i;
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
//              try {
//                  this.xDrawPage.remove(ControlList[i].xShape);
                {
                    ControlList[i].setPosition(new Point(this.iXNirwanaPos, this.iYNirwanaPos));
//                  String sControlName = (String) ControlList[i].xPropertySet.getPropertyValue(PropertyNames.PROPERTY_NAME);
//
//                  if (_xNamedForm.hasByName(sControlName))
//                      _xNamedForm.removeByName(sControlName);
//              } catch (Exception e) {
//                  e.printStackTrace(System.err);
//              }
                }
            }
        }
    }

    public void moveShapesToNirwana()
    {
        try
        {
            for (int i = 0; i < this.xDrawPage.getCount(); i++)
            {
                XShape xShape = UnoRuntime.queryInterface(XShape.class, xDrawPage.getByIndex(i));
                xShape.setPosition(new Point(this.iXNirwanaPos, this.iYNirwanaPos));
            }
        }
        catch (Exception e)
        {
            Logger.getLogger(FormHandler.class.getName()).log(Level.SEVERE, null, e);
        }
    }

    public void removeAllShapes() throws Exception
    {
        for (int i = this.xDrawPage.getCount(); i > -1; i--)
        {
            XShape xShape = UnoRuntime.queryInterface(XShape.class, xDrawPage.getByIndex(i));
            removeShape(xShape);
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
    // Destroy all Shapes in Nirwana
    public void removeNirwanaShapes() throws Exception
    {
        for (int i = this.xDrawPage.getCount(); i > -1; i--)
        {
            XShape xShape = UnoRuntime.queryInterface(XShape.class, xDrawPage.getByIndex(i));
            if (xShape.getPosition().Y < this.iYNirwanaPos)
            {
                xDrawPage.remove(xShape);
            }
        }
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

    /**
     * @return
     */
    public int getBasicLabelDiffHeight()
    {
        if (this.BasicLabelDiffHeight == -1)
        {
            initializeBasicControlValues();
        }
        return BasicLabelDiffHeight;
    }

    /**
     * @return
     */
    public int getControlReferenceHeight()
    {
        if (this.nDBRefHeight == -1)
        {
            initializeBasicControlValues();
        }
        return nDBRefHeight;
    }

    /**
     * @return
     */
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
