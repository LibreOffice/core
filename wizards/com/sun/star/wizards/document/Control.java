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
import com.sun.star.awt.XControl;
import com.sun.star.awt.XControlModel;
import com.sun.star.awt.XLayoutConstrains;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNameContainer;
import com.sun.star.wizards.common.*;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.AnyConverter;
import com.sun.star.drawing.XShapes;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.util.Date;
import com.sun.star.util.Time;

public class Control extends Shape
{

    XControlModel xControlModel;
    private XControl xControl;
    public XPropertySet xPropertySet;
    XWindowPeer xWindowPeer;
    private static final int SOMAXTEXTSIZE = 50;
    private int icontroltype;
    private XNameContainer xFormName;
    private static final int IIMGFIELDWIDTH = 3000;

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
        createControl(_icontroltype, null, _FieldName);
    }

    public Control(FormHandler _oFormHandler, int _icontroltype, Point _aPoint, Size _aSize)
    {
        super(_oFormHandler, _aPoint, _aSize);
        createControl(_icontroltype, null, null);
    }

    private void createControl(int _icontroltype, XShapes _xGroupShapes, String _FieldName)
    {
        try
        {
            icontroltype = _icontroltype;
            String sServiceName = oFormHandler.sModelServices[icontroltype];
            Object oControlModel = oFormHandler.xMSFDoc.createInstance(sServiceName);
            xControlModel = UnoRuntime.queryInterface( XControlModel.class, oControlModel );
            xPropertySet = UnoRuntime.queryInterface( XPropertySet.class, oControlModel );

            XPropertySetInfo xPSI = xPropertySet.getPropertySetInfo();
            if ( xPSI.hasPropertyByName( "MouseWheelBehavior" ) )
                xPropertySet.setPropertyValue( "MouseWheelBehavior", Short.valueOf( com.sun.star.awt.MouseWheelBehavior.SCROLL_DISABLED ) );

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
            UnoRuntime.queryInterface( XPropertySet.class, xControl );
            xWindowPeer = xControl.getPeer();
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
        }
    }

    private void insertControlInContainer(String _fieldname)
    {
        try
        {
            if (xFormName != null)
            {
                XNameAccess xNameAccess = UnoRuntime.queryInterface(XNameAccess.class, xFormName);
                String sControlName = Desktop.getUniqueName(xNameAccess, getControlName(_fieldname));
                xPropertySet.setPropertyValue(PropertyNames.PROPERTY_NAME, sControlName);
                xFormName.insertByName(sControlName, xControlModel);
            }
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
        }
    }

    private String getControlName(String _fieldname)
    {
        String controlname = PropertyNames.EMPTY_STRING;
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
            int nWidth;
            if (aPeerSize == null)
                nWidth = 0;
            else
                nWidth = aPeerSize.Width;

            // We increase the preferred Width a bit so that the control does not become too small
            // when we change the border from "3D" to "Flat"
            if (getControlType() == FormHandler.SOCHECKBOX)
            {
                return nWidth * oFormHandler.getXPixelFactor();
            }
            else
            {
                return (nWidth * oFormHandler.getXPixelFactor()) + 200;
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
            int nHeight;
            if (aPeerSize == null)
                nHeight = 0;
            else
                nHeight = aPeerSize.Height;

            // We increase the preferred Height a bit so that the control does not become too small
            // when we change the border from "3D" to "Flat"
            return ((nHeight + 1) * oFormHandler.getYPixelFactor());
        }
    }

    private Size getPreferredSize(String sText)
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
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
        }
        return getPeer().getPreferredSize();
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
     */
    private XLayoutConstrains getPeer()
    {
        return UnoRuntime.queryInterface(XLayoutConstrains.class, xControl.getPeer());
    }

    private Size getPeerSize()
    {
        try
        {
            Size aPreferredSize = null;
            double dblEffMax = 0;
            if (xPropertySet.getPropertySetInfo().hasPropertyByName("EffectiveMax"))
            {
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
                    xPropertySet.setPropertyValue("EffectiveValue", new Double(dblEffMax)); //new Double(100000.2));
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
                Date d = new Date();
                d.Day = 30;
                d.Month = 12;
                d.Year = 9999;
                xPropertySet.setPropertyValue("Date", d);
                aPreferredSize = getPeer().getPreferredSize();
                xPropertySet.setPropertyValue("Date", com.sun.star.uno.Any.VOID);
            }
            else if (getControlType() == FormHandler.SOTIMECONTROL)
            {
                Time t = new Time();
                t.NanoSeconds = 999999999;
                t.Seconds = 59;
                t.Minutes = 59;
                t.Hours = 22;
                xPropertySet.setPropertyValue("Time", t);
                aPreferredSize = getPeer().getPreferredSize();
                xPropertySet.setPropertyValue("Time", com.sun.star.uno.Any.VOID);
            }
            else
            {
                String stext;
                short iTextLength = AnyConverter.toShort(xPropertySet.getPropertyValue("MaxTextLen"));
                if (iTextLength < SOMAXTEXTSIZE)
                {
                    stext = FormHandler.SOSIZETEXT.substring(0, SOMAXTEXTSIZE);
                }
                else
                {
                    stext = FormHandler.SOSIZETEXT.substring(0, iTextLength);
                }
                xPropertySet.setPropertyValue("Text", stext);
                aPreferredSize = getPeer().getPreferredSize();
                xPropertySet.setPropertyValue("Text", PropertyNames.EMPTY_STRING);
            }
            return aPreferredSize;
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
            return null;
        }
    }

    public int getControlType()
    {
        return icontroltype;
    }
}
