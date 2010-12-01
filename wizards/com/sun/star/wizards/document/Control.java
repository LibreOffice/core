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
import com.sun.star.awt.XControl;
import com.sun.star.awt.XControlModel;
import com.sun.star.awt.XLayoutConstrains;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNameContainer;
import com.sun.star.container.XNamed;
import com.sun.star.wizards.common.*;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.AnyConverter;
import com.sun.star.drawing.XShape;
import com.sun.star.drawing.XShapes;
import com.sun.star.lang.IllegalArgumentException;

public class Control extends Shape
{

    XControlModel xControlModel;
    XControl xControl;
    public XPropertySet xPropertySet;
    XPropertySet xControlPropertySet;
    XWindowPeer xWindowPeer;
    Object oDefaultValue;
    GridControl oGridControl;
    String sServiceName;
    XNamed xNamed;
    final int SOMAXTEXTSIZE = 50;
    private int icontroltype;
    protected XNameContainer xFormName;
    protected final int IIMGFIELDWIDTH = 3000;

    public Control()
    {
    }

    public Control(FormHandler _oFormHandler, String _sServiceName, Point _aPoint)
    {
        super(_oFormHandler, _sServiceName, _aPoint, null);
    }

    public Control(FormHandler _oFormHandler, XNameContainer _xFormName, int _icontroltype, String _FieldName, Point _aPoint, Size _aSize)
    {
        super(_oFormHandler, _aPoint, _aSize);
        xFormName = _xFormName;
        createControl(_icontroltype, _aPoint, _aSize, null, _FieldName);
    }

    public Control(FormHandler _oFormHandler, XShapes _xGroupShapes, XNameContainer _xFormName, int _icontroltype, Point _aPoint, Size _aSize)
    {
        super(_oFormHandler, _aPoint, _aSize);
        xFormName = _xFormName;
        createControl(_icontroltype, _aPoint, _aSize, _xGroupShapes, null);
    }

    public Control(FormHandler _oFormHandler, int _icontroltype, Point _aPoint, Size _aSize)
    {
        super(_oFormHandler, _aPoint, _aSize);
        createControl(_icontroltype, _aPoint, _aSize, null, null);
    }

    public void createControl(int _icontroltype, Point _aPoint, Size _aSize, XShapes _xGroupShapes, String _FieldName)
    {
        try
        {
            icontroltype = _icontroltype;
            sServiceName = oFormHandler.sModelServices[getControlType()];
            Object oControlModel = oFormHandler.xMSFDoc.createInstance(sServiceName);
            xControlModel = UnoRuntime.queryInterface( XControlModel.class, oControlModel );
            xPropertySet = UnoRuntime.queryInterface( XPropertySet.class, oControlModel );

            XPropertySetInfo xPSI = xPropertySet.getPropertySetInfo();
            if ( xPSI.hasPropertyByName( "MouseWheelBehavior" ) )
                xPropertySet.setPropertyValue( "MouseWheelBehavior", new Short( com.sun.star.awt.MouseWheelBehavior.SCROLL_DISABLED ) );

            insertControlInContainer(_FieldName);
            xControlShape.setControl(xControlModel);
            if (_xGroupShapes == null)
            {
                oFormHandler.xDrawPage.add(xShape);
            }
            else
            {
                _xGroupShapes.add(xShape);
            }
            xControl = oFormHandler.xControlAccess.getControl(xControlModel);
            xControlPropertySet = UnoRuntime.queryInterface( XPropertySet.class, xControl );
            xWindowPeer = xControl.getPeer();
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
        }
    }

    public void insertControlInContainer(String _fieldname)
    {
        try
        {
            if (xFormName != null)
            {
                XNameAccess xNameAccess = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, xFormName);
                String sControlName = Desktop.getUniqueName(xNameAccess, getControlName(_fieldname));
                xPropertySet.setPropertyValue(PropertyNames.PROPERTY_NAME, sControlName);
                xFormName.insertByName(sControlName, xControlModel);
            }
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
        }
    }

    public String getControlName(String _fieldname)
    {
        String controlname = "";
        switch (getControlType())
        {
            case FormHandler.SOLABEL:
                controlname = "lbl" + _fieldname;
                break;
            case FormHandler.SOTEXTBOX:
                controlname = "txt" + _fieldname;
                break;
            case FormHandler.SOCHECKBOX:
                controlname = "chk" + _fieldname;
                break;
            case FormHandler.SODATECONTROL:
                controlname = "dat" + _fieldname;
                break;
            case FormHandler.SOTIMECONTROL:
                controlname = "tim" + _fieldname;
                break;
            case FormHandler.SONUMERICCONTROL:
                controlname = "fmt" + _fieldname;
                break;
            case FormHandler.SOGRIDCONTROL:
                controlname = "grd" + _fieldname;
                break;
            case FormHandler.SOIMAGECONTROL:
                controlname = "img" + _fieldname;
                break;
            default:
                controlname = "ctrl" + _fieldname;
        }
        return controlname;
    }

    private void setDefaultValue(Object DatabaseField)
    {
        oDefaultValue = Helper.getUnoPropertyValue(DatabaseField, "DefaultValue");
    }

    public int getPreferredWidth(String sText)
    {
        Size aPeerSize = getPreferredSize(sText);
        return ((aPeerSize.Width + 10) * oFormHandler.getXPixelFactor());
    }

    public int getPreferredHeight(String sText)
    {
        Size aPeerSize = getPreferredSize(sText);
        if (getControlType() == FormHandler.SOCHECKBOX)
        {
            return (aPeerSize.Height * oFormHandler.getXPixelFactor());
        }
        else
        {
            return ((aPeerSize.Height + 2) * oFormHandler.getXPixelFactor());
        }
    }

    public int getPreferredWidth()
    {
        if (getControlType() == FormHandler.SOIMAGECONTROL)
        {
            return IIMGFIELDWIDTH;
        }
        else
        {
            Size aPeerSize = getPeerSize();
            // We increase the preferred Width a bit so that the control does not become too small
            // when we change the border from "3D" to "Flat"
            if (getControlType() == FormHandler.SOCHECKBOX)
            {
                return ((aPeerSize.Width * oFormHandler.getXPixelFactor()));
            }
            else
            {
                return ((aPeerSize.Width * oFormHandler.getXPixelFactor()) + 200);
            }
        }
    }

    public int getPreferredHeight()
    {
        if (getControlType() == FormHandler.SOIMAGECONTROL)
        {
            return 2000;
        }
        else
        {
            Size aPeerSize = getPeerSize();
            int nHeight = aPeerSize.Height;
            // We increase the preferred Height a bit so that the control does not become too small
            // when we change the border from "3D" to "Flat"
            return ((nHeight + 1) * oFormHandler.getYPixelFactor());
        }
    }

    public Size getPreferredSize(String sText)
    {
        try
        {
            if (xPropertySet.getPropertySetInfo().hasPropertyByName("Text"))
            {
                xPropertySet.setPropertyValue("Text", sText);
            }
            else if (xPropertySet.getPropertySetInfo().hasPropertyByName(PropertyNames.PROPERTY_LABEL))
            {
                xPropertySet.setPropertyValue(PropertyNames.PROPERTY_LABEL, sText);
            }
            else
            {
                throw new IllegalArgumentException();
            }
            return getPeer().getPreferredSize();
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
            return null;
        }
    }

    public void setPropertyValue(String _sPropertyName, Object _aPropertyValue) throws Exception
    {
        if (xPropertySet.getPropertySetInfo().hasPropertyByName(_sPropertyName))
        {
            xPropertySet.setPropertyValue(_sPropertyName, _aPropertyValue);
        }
    }

    /** the peer should be retrieved every time before it is used because it
     * might be disposed otherwise
     *
     * @return
     */
    public XLayoutConstrains getPeer()
    {
        return (XLayoutConstrains) UnoRuntime.queryInterface(XLayoutConstrains.class, xControl.getPeer());
    }

    public Size getPeerSize()
    {
        try
        {
            Size aPreferredSize = null;
            double dblEffMax = 0;
            if (xPropertySet.getPropertySetInfo().hasPropertyByName("EffectiveMax"))
            {
                Object oValue = xPropertySet.getPropertyValue("EffectiveMax");
                if (xPropertySet.getPropertyValue("EffectiveMax") != com.sun.star.uno.Any.VOID)
                {
                    dblEffMax = AnyConverter.toDouble(xPropertySet.getPropertyValue("EffectiveMax"));
                }
                if (dblEffMax == 0)
                {
                    // This is relevant for decimal fields
                    xPropertySet.setPropertyValue("EffectiveValue", new Double(99999));
                }
                else
                {
                    xPropertySet.setPropertyValue("EffectiveValue", new Double(dblEffMax)); //new Double(100000.2)); //
                }
                aPreferredSize = getPeer().getPreferredSize();
                xPropertySet.setPropertyValue("EffectiveValue", com.sun.star.uno.Any.VOID);
            }
            else if (getControlType() == FormHandler.SOCHECKBOX)
            {
                aPreferredSize = getPeer().getPreferredSize();
            }
            else if (getControlType() == FormHandler.SODATECONTROL)
            {
                xPropertySet.setPropertyValue("Date", 4711);       //TODO find a better date
                aPreferredSize = getPeer().getPreferredSize();
                xPropertySet.setPropertyValue("Date", com.sun.star.uno.Any.VOID);
            }
            else if (getControlType() == FormHandler.SOTIMECONTROL)
            {
                xPropertySet.setPropertyValue("Time", 47114);      //TODO find a better time
                aPreferredSize = getPeer().getPreferredSize();
                xPropertySet.setPropertyValue("Time", com.sun.star.uno.Any.VOID);
            }
            else
            {
                String stext;
                short iTextLength = AnyConverter.toShort(xPropertySet.getPropertyValue("MaxTextLen"));
                if (iTextLength < this.SOMAXTEXTSIZE)
                {
                    stext = FormHandler.SOSIZETEXT.substring(0, this.SOMAXTEXTSIZE);
                }
                else
                {
                    stext = FormHandler.SOSIZETEXT.substring(0, iTextLength);
                }
                xPropertySet.setPropertyValue("Text", stext);
                aPreferredSize = getPeer().getPreferredSize();
                xPropertySet.setPropertyValue("Text", "");
            }
            return aPreferredSize;
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
    public int getControlType()
    {
        return icontroltype;
    }
}
