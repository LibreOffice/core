/*************************************************************************
 *
 *  $RCSfile: Shape.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pjunck $  $Date: 2004-10-27 13:32:29 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): Berend Cornelius
 *
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
import com.sun.star.wizards.common.Desktop;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.JavaTools;

/**
 * @author Administrator
 *
 * To change the template for this generated type comment go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
public class Shape {
    public XShape xShape;
    protected FormHandler oFormHandler;
    public XServiceInfo xServiceInfo;
    protected Point aPoint;
    protected Size aSize;
    protected XControlShape xControlShape;
    public XMultiServiceFactory xMSF;
    public XShapes xShapes;



    public Shape(FormHandler _oFormHandler, Point _aPoint, Size _aSize) {
        this.aPoint = _aPoint;
        this.aSize = _aSize;
        this.oFormHandler = _oFormHandler;
        createShape("com.sun.star.drawing.ControlShape");
    }


    public Shape(FormHandler _oFormHandler, String _sServiceName, Point _aPoint, Size _aSize) {
    try {
        this.aPoint = _aPoint;
        this.aSize = _aSize;
        this.oFormHandler = _oFormHandler;
        Object oShape = oFormHandler.xMSF.createInstance(_sServiceName);
        xShapes = (XShapes) UnoRuntime.queryInterface(XShapes.class, oShape);
        xServiceInfo = (XServiceInfo) UnoRuntime.queryInterface(XServiceInfo.class, oShape);
    } catch (Exception e) {
        e.printStackTrace();
    }}


    public Shape() {
    }


    private void createShape(String sServiceName){
    try {
        String[] savailservicenames = oFormHandler.xMSFDoc.getAvailableServiceNames();
        xMSF = oFormHandler.xMSFDoc;
        Object oShape = xMSF.createInstance(sServiceName);
        xShape = (XShape) UnoRuntime.queryInterface(XShape.class, oShape);
        xShape.setPosition(aPoint);
        if (aSize != null)
            xShape.setSize(aSize);
        else
            xShape.setSize(new Size(1000, 100));
        Helper.setUnoPropertyValue(xShape, "AnchorType", TextContentAnchorType.AT_PARAGRAPH);
        xServiceInfo = (XServiceInfo) UnoRuntime.queryInterface(XServiceInfo.class, xShape);
        xControlShape = (XControlShape) UnoRuntime.queryInterface(XControlShape.class, xShape);

    } catch (Exception e) {
        e.printStackTrace(System.out);
    }}

    public Size getSize(){
        return xShape.getSize();
    }


    public void setSize(Size _aSize){
    try {
        xShape.setSize(_aSize);
    } catch (PropertyVetoException e) {
        e.printStackTrace(System.out);
    }}


    public Point getPosition(){
        return xShape.getPosition();
    }


    public void setPosition(Point _aPoint){
        xShape.setPosition(_aPoint);
    }
}
