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
import com.sun.star.beans.XPropertySet;
import com.sun.star.wizards.common.*;
import com.sun.star.wizards.db.FieldColumn;
import com.sun.star.wizards.ui.*;
import com.sun.star.sdbc.*;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.container.XNameContainer;
import com.sun.star.drawing.XControlShape;
import com.sun.star.drawing.XShapes;

public class TimeStampControl extends DatabaseControl
{

    private DatabaseControl oDateControl;
    private DatabaseControl oTimeControl;
    private Resource oResource;
    private int nDBWidth;

    public TimeStampControl(Resource _oResource, FormHandler _oFormHandler, XNameContainer _xFormName, String _curFieldName, Point _aPoint)
    {
        super(_oFormHandler, "com.sun.star.drawing.ShapeCollection", _aPoint);
        oResource = _oResource;
        oDateControl = new DatabaseControl(oFormHandler, _xFormName, _curFieldName, DataType.DATE, aPoint);
        int nDBHeight = oDateControl.getControlHeight();
        int nDateWidth = oDateControl.getPreferredWidth();
        oDateControl.setSize(new Size(nDateWidth, nDBHeight));
        Point aTimePoint = new Point(aPoint.X + 10 + nDateWidth, aPoint.Y);
        oTimeControl = new DatabaseControl(oFormHandler, _xFormName, _curFieldName, DataType.TIME, aTimePoint);
        int nTimeWidth = oTimeControl.getPreferredWidth();
        oTimeControl.setSize(new Size(nTimeWidth, nDBHeight));
        nDBWidth = nDateWidth + nTimeWidth + 10;
        xShapes.add(oDateControl.xShape);
        xShapes.add(oTimeControl.xShape);
        xShape = _oFormHandler.xShapeGrouper.group(xShapes);
        xShapes = UnoRuntime.queryInterface(XShapes.class, xShape);
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
        String sDateAppendix = oResource.getResText(UIConsts.RID_FORM + 88);
        String sTimeAppendix = oResource.getResText(UIConsts.RID_FORM + 89);
        oDateControl = new DatabaseControl(_oGridControl, _curfieldcolumn, DataType.DATE, _curfieldcolumn.getFieldTitle() + PropertyNames.SPACE + sDateAppendix);
        oTimeControl = new DatabaseControl(_oGridControl, _curfieldcolumn, DataType.TIME, _curfieldcolumn.getFieldTitle() + PropertyNames.SPACE + sTimeAppendix);
    }

    @Override
    public void setPropertyValue(String _sPropertyName, Object _aPropertyValue) throws Exception
    {
        oDateControl.setPropertyValue(_sPropertyName, _aPropertyValue);
        oTimeControl.setPropertyValue(_sPropertyName, _aPropertyValue);
    }

    @Override
    public int getPreferredWidth()
    {
        return nDBWidth;
    }

    @Override
    public int getControlType()
    {
        return FormHandler.SODATETIMECONTROL;
    }
}





