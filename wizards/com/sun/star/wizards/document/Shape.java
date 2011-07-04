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
import com.sun.star.drawing.XControlShape;
import com.sun.star.drawing.XShape;
import com.sun.star.drawing.XShapes;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.text.TextContentAnchorType;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.Helper;

/**
 * @author Administrator
 *
 * To change the template for this generated type comment go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
public class Shape
{

    public XShape xShape;
    protected FormHandler oFormHandler;
    public XServiceInfo xServiceInfo;
    protected Point aPoint;
    protected Size aSize;
    protected XControlShape xControlShape;
    public XMultiServiceFactory xMSF;
    public XShapes xShapes;

    public Shape(FormHandler _oFormHandler, Point _aPoint, Size _aSize)
    {
        this.aPoint = _aPoint;
        this.aSize = _aSize;
        this.oFormHandler = _oFormHandler;
        createShape("com.sun.star.drawing.ControlShape");
    }

    public Shape(FormHandler _oFormHandler, String _sServiceName, Point _aPoint, Size _aSize)
    {
        try
        {
            this.aPoint = _aPoint;
            this.aSize = _aSize;
            this.oFormHandler = _oFormHandler;
            Object oShape = oFormHandler.xMSF.createInstance(_sServiceName);
            xShapes = UnoRuntime.queryInterface(XShapes.class, oShape);
            xServiceInfo = UnoRuntime.queryInterface(XServiceInfo.class, oShape);
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    public Shape()
    {
    }

    private void createShape(String sServiceName)
    {
        try
        {
            xMSF = oFormHandler.xMSFDoc;
            Object oShape = xMSF.createInstance(sServiceName);
            xShape = UnoRuntime.queryInterface(XShape.class, oShape);
            xShape.setPosition(aPoint);
            if (aSize != null)
            {
                xShape.setSize(aSize);
            }
            else
            {
                xShape.setSize(new Size(1000, 100));
            }
            Helper.setUnoPropertyValue(xShape, "AnchorType", TextContentAnchorType.AT_PARAGRAPH);
            xServiceInfo = UnoRuntime.queryInterface(XServiceInfo.class, xShape);
            xControlShape = UnoRuntime.queryInterface(XControlShape.class, xShape);

        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
        }
    }

    public Size getSize()
    {
        return xShape.getSize();
    }

    public void setSize(Size _aSize)
    {
        try
        {
            xShape.setSize(_aSize);
        }
        catch (PropertyVetoException e)
        {
            e.printStackTrace(System.out);
        }
    }

    public Point getPosition()
    {
        return xShape.getPosition();
    }

    public void setPosition(Point _aPoint)
    {
        xShape.setPosition(_aPoint);
    }
}
