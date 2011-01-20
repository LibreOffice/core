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
import com.sun.star.beans.PropertyVetoException;
import com.sun.star.beans.XPropertySet;
import com.sun.star.wizards.common.*;
import com.sun.star.wizards.db.FieldColumn;
import com.sun.star.wizards.ui.*;
import com.sun.star.sdbc.*;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.container.XNameContainer;
import com.sun.star.drawing.XControlShape;
import com.sun.star.drawing.XShape;
import com.sun.star.drawing.XShapes;

public class TimeStampControl extends DatabaseControl
{

    DatabaseControl oDateControl;
    DatabaseControl oTimeControl;
//  XShape xGroupShape;
    Resource oResource;
    private String sDateAppendix; // = GetResText(RID_FORM + 4)
    private String sTimeAppendix; // = GetResText(RID_FORM + 5)
    XShapes xGroupShapes = null;
    double nreldatewidth;
    double nreltimewidth;
    int nTimeWidth;
    int nDBWidth;
    int nDateWidth;
    XShape xShapeGroup;

    public TimeStampControl(Resource _oResource, FormHandler _oFormHandler, XNameContainer _xFormName, String _curFieldName, Point _aPoint)
    {
        super(_oFormHandler, "com.sun.star.drawing.ShapeCollection", _aPoint);
        oResource = _oResource;
//      xGroupShape = xShape;
        oDateControl = new DatabaseControl(oFormHandler, _xFormName, _curFieldName, DataType.DATE, aPoint);
        int nDBHeight = oDateControl.getControlHeight();
        nDateWidth = oDateControl.getPreferredWidth();
        oDateControl.setSize(new Size(nDateWidth, nDBHeight));
        Point aTimePoint = new Point(aPoint.X + 10 + nDateWidth, aPoint.Y);
        oTimeControl = new DatabaseControl(oFormHandler, _xFormName, _curFieldName, DataType.TIME, aTimePoint);
        nTimeWidth = oTimeControl.getPreferredWidth();
        oTimeControl.setSize(new Size(nTimeWidth, nDBHeight));
        nDBWidth = nDateWidth + nTimeWidth + 10;
        xShapes.add(oDateControl.xShape);
        xShapes.add(oTimeControl.xShape);
        xShapeGroup = _oFormHandler.xShapeGrouper.group(xShapes);
        xShapeGroup = (XShape) UnoRuntime.queryInterface(XShape.class, xShapeGroup);
        nreldatewidth = 1.0 / ((double) getSize().Width / (double) nDateWidth);
        nreltimewidth = 1.0 - nreldatewidth;
    }

    public XPropertySet getControlofGroupShapeByIndex(int _i)
    {
        try
        {
            if (_i < xShapes.getCount())
            {
                Object oControl = xShapes.getByIndex(_i);
                XControlShape xControlShape = (XControlShape) UnoRuntime.queryInterface(XControlShape.class, oControl);
                XPropertySet xPropertySet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xControlShape.getControl());
                return xPropertySet;
            }
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
        }
        return null;
    }

    public TimeStampControl(Resource _oResource, GridControl _oGridControl, FieldColumn _curfieldcolumn)
    {
        super(_oGridControl, _curfieldcolumn);
        oResource = _oResource;
        sDateAppendix = oResource.getResText(UIConsts.RID_FORM + 88);
        sTimeAppendix = oResource.getResText(UIConsts.RID_FORM + 89);
        oDateControl = new DatabaseControl(_oGridControl, _curfieldcolumn, DataType.DATE, _curfieldcolumn.getFieldTitle() + PropertyNames.SPACE + sDateAppendix);
        oTimeControl = new DatabaseControl(_oGridControl, _curfieldcolumn, DataType.TIME, _curfieldcolumn.getFieldTitle() + PropertyNames.SPACE + sTimeAppendix);
    }

    public void setPropertyValue(String _sPropertyName, Object _aPropertyValue) throws Exception
    {
        oDateControl.setPropertyValue(_sPropertyName, _aPropertyValue);
        oTimeControl.setPropertyValue(_sPropertyName, _aPropertyValue);
    }

    public int getPreferredWidth()
    {
        return nDBWidth;
    }

    public void setSize(Size _aSize)
    {
        try
        {
            int ndatewidth = (int) (nreldatewidth * (double) _aSize.Width);
            int ntimewidth = (int) (nreltimewidth * (double) _aSize.Width);
            oDateControl.xShape.setSize(new Size(ndatewidth, _aSize.Height));
            oTimeControl.xShape.setSize(new Size(ntimewidth, _aSize.Height));
        }
        catch (PropertyVetoException e)
        {
            e.printStackTrace(System.out);
        }
    }

    public Size getSize()
    {
        int ncontrolwidth = oDateControl.xShape.getSize().Width + oTimeControl.xShape.getSize().Width;
        return new Size(ncontrolwidth, oDateControl.xShape.getSize().Height);
    }

    public Point getPosition()
    {
        return xShapeGroup.getPosition();
    }

    public void setPosition(Point _aPoint)
    {
        // --> TESTING
        Point aBeforePt = xShapeGroup.getPosition();
        // <--
        xShapeGroup.setPosition(_aPoint);
//      oDateControl.xShape.setPosition(_aPoint);
//      Point atimepoint = new Point(oDateControl.xShape.getPosition().X + oDateControl.xShape.getSize().Width, oDateControl.xShape.getPosition().Y );
//      oTimeControl.xShape.setPosition(atimepoint);
        // --> TESTING
        Point aAfterPt = xShapeGroup.getPosition();
    // <--
    }

    public int getControlType()
    {
        return FormHandler.SODATETIMECONTROL;
    }
}





