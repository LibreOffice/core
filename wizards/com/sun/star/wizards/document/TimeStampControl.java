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
        xShapeGroup = UnoRuntime.queryInterface(XShape.class, xShapeGroup);
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
                XControlShape xControlShape = UnoRuntime.queryInterface(XControlShape.class, oControl);
                return UnoRuntime.queryInterface(XPropertySet.class, xControlShape.getControl());
            }
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
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
            int ndatewidth = (int) (nreldatewidth * _aSize.Width);
            int ntimewidth = (int) (nreltimewidth * _aSize.Width);
            oDateControl.xShape.setSize(new Size(ndatewidth, _aSize.Height));
            oTimeControl.xShape.setSize(new Size(ntimewidth, _aSize.Height));
        }
        catch (PropertyVetoException e)
        {
            e.printStackTrace(System.err);
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
        xShapeGroup.setPosition(_aPoint);
//      oDateControl.xShape.setPosition(_aPoint);
//      Point atimepoint = new Point(oDateControl.xShape.getPosition().X + oDateControl.xShape.getSize().Width, oDateControl.xShape.getPosition().Y );
//      oTimeControl.xShape.setPosition(atimepoint);
        // --> TESTING
        Point aAfterPt = xShapeGroup.getPosition();
    }

    public int getControlType()
    {
        return FormHandler.SODATETIMECONTROL;
    }
}





