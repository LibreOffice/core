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
            e.printStackTrace(System.err);
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
            e.printStackTrace(System.err);
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
