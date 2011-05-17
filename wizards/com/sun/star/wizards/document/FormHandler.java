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
package com.sun.star.wizards.document;

import com.sun.star.awt.Point;
import com.sun.star.awt.Size;
import com.sun.star.awt.VclWindowPeerAttribute;
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

public class FormHandler
{

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
        xDrawPageSupplier = (XDrawPageSupplier) UnoRuntime.queryInterface(XDrawPageSupplier.class, xTextDocument);
        xDrawPage = xDrawPageSupplier.getDrawPage();
        xFormsSupplier = (XFormsSupplier) UnoRuntime.queryInterface(XFormsSupplier.class, xDrawPage);
        xShapeGrouper = (XShapeGrouper) UnoRuntime.queryInterface(XShapeGrouper.class, xDrawPage);
        xControlAccess = (XControlAccess) UnoRuntime.queryInterface(XControlAccess.class, xTextDocument.getCurrentController());
        xMSFDoc = (XMultiServiceFactory) UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDocument);
        sModelServices[SOLABEL] = "com.sun.star.form.component.FixedText";
        sModelServices[SOTEXTBOX] = "com.sun.star.form.component.TextField";
        sModelServices[SOCHECKBOX] = "com.sun.star.form.component.CheckBox";
        sModelServices[SODATECONTROL] = "com.sun.star.form.component.DateField";
        sModelServices[SOTIMECONTROL] = "com.sun.star.form.component.TimeField";
        sModelServices[SONUMERICCONTROL] = "com.sun.star.form.component.FormattedField";
        sModelServices[SOGRIDCONTROL] = "com.sun.star.form.component.GridControl";
        sModelServices[SOIMAGECONTROL] = "com.sun.star.form.component.DatabaseImageControl";

        oControlData = new ControlData[22];
        oControlData[0] = createControlData(DataType.BIT,          SOCHECKBOX, "CheckBox", "CheckBox", false);
        oControlData[1] = createControlData(DataType.BOOLEAN,      SOCHECKBOX, "CheckBox", "CheckBox", false);
        oControlData[2] = createControlData(DataType.TINYINT,      SONUMERICCONTROL, "FormattedField", "FormattedField", false);
        oControlData[3] = createControlData(DataType.SMALLINT,     SONUMERICCONTROL, "FormattedField", "FormattedField", false);
        oControlData[4] = createControlData(DataType.INTEGER,      SONUMERICCONTROL, "FormattedField", "FormattedField", false);
        oControlData[5] = createControlData(DataType.BIGINT,       SONUMERICCONTROL, "FormattedField", "FormattedField", false);
        oControlData[6] = createControlData(DataType.FLOAT,        SONUMERICCONTROL, "FormattedField", "FormattedField", false);
        oControlData[7] = createControlData(DataType.REAL,         SONUMERICCONTROL, "FormattedField", "FormattedField", false);
        oControlData[8] = createControlData(DataType.DOUBLE,       SONUMERICCONTROL, "FormattedField", "FormattedField", false);
        oControlData[9] = createControlData(DataType.NUMERIC,      SONUMERICCONTROL, "FormattedField", "FormattedField", false);
        oControlData[10] = createControlData(DataType.DECIMAL,     SONUMERICCONTROL, "FormattedField", "FormattedField", false);
        oControlData[11] = createControlData(DataType.CHAR,        SOTEXTBOX, "TextField", "TextField", false);
        oControlData[12] = createControlData(DataType.VARCHAR,     SOTEXTBOX, "TextField", "TextField", true);
        oControlData[13] = createControlData(DataType.LONGVARCHAR, SOTEXTBOX, "TextField", "TextField", true);
        oControlData[14] = createControlData(DataType.DATE,        SODATECONTROL, "DateField", "DateField", false);
        oControlData[15] = createControlData(DataType.TIME,        SOTIMECONTROL, "TimeField", "TimeField", false);
        oControlData[16] = createControlData(DataType.TIMESTAMP,   SODATECONTROL, "DateField", "TextField", false);
        // oImageControlData = new ControlData[4];
        oControlData[17] = createControlData(DataType.BINARY,      SOIMAGECONTROL, "ImageControl", "TextField", false);
        oControlData[18] = createControlData(DataType.VARBINARY,   SOIMAGECONTROL, "ImageControl", "TextField", false);
        oControlData[19] = createControlData(DataType.LONGVARBINARY, SOIMAGECONTROL, "ImageControl", "TextField", false);
        oControlData[20] = createControlData(DataType.BLOB,        SOIMAGECONTROL, "ImageControl", "TextField", false);

        oControlData[21] = createControlData(DataType.OTHER,       SOIMAGECONTROL, "ImageControl", "TextField", false);
    }

    public int getControlType(int _fieldtype)
    {
        for (int i = 0; i < oControlData.length; i++)
        {
            if (oControlData[i].DataType == _fieldtype)
            {
                final int nType = oControlData[i].ControlType;
                return nType;
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
        XDevice xDevice = (XDevice) UnoRuntime.queryInterface(XDevice.class, oLabelControl.xWindowPeer);
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
        XNameContainer xNamedForms = xFormsSupplier.getForms();
        return xNamedForms;
    }

    public String getValueofHiddenControl(XNameAccess xNamedForm, String ControlName, String sMsg) throws com.sun.star.wizards.document.FormHandler.UnknownHiddenControlException
    {
        try
        {
            if (xNamedForm.hasByName(ControlName))
            {
                String ControlValue = AnyConverter.toString(com.sun.star.wizards.common.Helper.getUnoPropertyValue(xNamedForm.getByName(ControlName), "HiddenValue"));
                return ControlValue;
            }
            else
            {
                throw new UnknownHiddenControlException(xNamedForm, ControlName, sMsg);
            }
        }
        catch (Exception exception)
        {
            throw new UnknownHiddenControlException(xNamedForm, ControlName, sMsg);
        }
    }

    public void insertHiddenControl(XNameAccess xNameAccess, XNameContainer xNamedForm, String ControlName, String ControlValue)
    {
        try
        {
            XInterface xHiddenControl;
            if (xNameAccess.hasByName(ControlName) == true)
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
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
        }
    }

    public class UnknownHiddenControlException extends java.lang.Throwable
    {

        public UnknownHiddenControlException(XNameAccess xNamedForm, String ControlName, String sMsgHiddenControlisMissing)
        {
            XNamed xNamed = (XNamed) UnoRuntime.queryInterface(XNamed.class, xNamedForm);
            String FormName = xNamed.getName();
            sMsgHiddenControlisMissing = JavaTools.replaceSubString(sMsgHiddenControlisMissing, FormName, "<REPORTFORM>");
            sMsgHiddenControlisMissing = JavaTools.replaceSubString(sMsgHiddenControlisMissing, ControlName, "<CONTROLNAME>");
            SystemDialog.showMessageBox(xMSFDoc, "ErrorBox", VclWindowPeerAttribute.OK, sMsgHiddenControlisMissing);
        }
    }

    public boolean hasFormByName(String _FormName)
    {
        xNamedFormContainer = getDocumentForms();
        xNamedForms = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, xNamedFormContainer);
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
        catch (com.sun.star.uno.Exception exception)
        {
            exception.printStackTrace(System.out);
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
                    XShape xShape = (XShape) UnoRuntime.queryInterface(XShape.class, xDrawPage.getByIndex(i));
                    xDrawPage.remove(xShape);
                }
            }
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
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
            e.printStackTrace( System.err );
        }
        catch( final NoSuchElementException e )
        {
            e.printStackTrace( System.err );
        }
    }

    public boolean belongsToForm(Object _oDrawPageElement, String _FormName)
    {
        XServiceInfo xServiceInfo = (XServiceInfo) UnoRuntime.queryInterface(XServiceInfo.class, _oDrawPageElement);
        if (xServiceInfo.supportsService("com.sun.star.drawing.ControlShape"))
        {
            XControlShape xControlShape = (XControlShape) UnoRuntime.queryInterface(XControlShape.class, _oDrawPageElement);
            XControlModel xControlModel = xControlShape.getControl();
            xServiceInfo = (XServiceInfo) UnoRuntime.queryInterface(XServiceInfo.class, xControlShape.getControl());
            if (xServiceInfo.supportsService("com.sun.star.form.FormComponent"))
            {
                XChild xChild = (XChild) UnoRuntime.queryInterface(XChild.class, xControlModel);
                XNamed xNamed = (XNamed) UnoRuntime.queryInterface(XNamed.class, xChild.getParent());
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
                xNamedForm = (XNameContainer) UnoRuntime.queryInterface(XNameContainer.class, oDBForm);
                return xNamedForm;
            }
            else
            {
                return getFormByName(_FormName);
            }
        }
        catch (com.sun.star.uno.Exception exception)
        {
            exception.printStackTrace(System.out);
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
                xNamedForm = (XNameContainer) UnoRuntime.queryInterface(XNameContainer.class, oDBForm);
            }
        }
        catch (IllegalArgumentException e)
        {
            e.printStackTrace(System.out);
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
//                  e.printStackTrace(System.out);
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
                XShape xShape = (XShape) UnoRuntime.queryInterface(XShape.class, xDrawPage.getByIndex(i));
                xShape.setPosition(new Point(this.iXNirwanaPos, this.iYNirwanaPos));
            }
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
        }
    }

    public void removeAllShapes() throws Exception
    {
        for (int i = this.xDrawPage.getCount(); i > -1; i--)
        {
            XShape xShape = (XShape) UnoRuntime.queryInterface(XShape.class, xDrawPage.getByIndex(i));
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
        XComponent xComponent = (XComponent) UnoRuntime.queryInterface(XComponent.class, _xShape);
        xComponent.dispose();
    }
    // Destroy all Shapes in Nirwana
    public void removeNirwanaShapes() throws Exception
    {
        for (int i = this.xDrawPage.getCount(); i > -1; i--)
        {
            XShape xShape = (XShape) UnoRuntime.queryInterface(XShape.class, xDrawPage.getByIndex(i));
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
            XShapes xShapes = (XShapes) UnoRuntime.queryInterface(XShapes.class, oGroupShape);
            xShapes.add(_xLabelShape);
            xShapes.add(_xControlShape);
            return this.xShapeGrouper.group(xShapes);
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
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
            XPropertySet xPropertySet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, this.xMSFDoc.createInstance("com.sun.star.text.DocumentSettings"));
            xPropertySet.setPropertyValue("DoNotCaptureDrawObjsOnPage", new Boolean(!_bCaptureObjects));
        }
        catch (Exception ex)
        {
            ex.printStackTrace(System.out);
        }
    }
}
