/*************************************************************************
 *
 *  $RCSfile: TimeStampControl.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pjunck $  $Date: 2004-10-27 13:32:45 $
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
import com.sun.star.beans.XPropertySet;
import com.sun.star.wizards.common.*;
import com.sun.star.wizards.db.FieldColumn;
import com.sun.star.wizards.ui.*;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdbc.*;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.container.XNameContainer;
import com.sun.star.drawing.XControlShape;
import com.sun.star.drawing.XShape;
import com.sun.star.drawing.XShapeGroup;
import com.sun.star.drawing.XShapes;


public class TimeStampControl extends DatabaseControl {
    DatabaseControl oDateControl;
    DatabaseControl oTimeControl;
    XShape xGroupShape;
    Resource oResource;
    String sDateAppendix; // = GetResText(RID_FORM + 4)
    String sTimeAppendix; // = GetResText(RID_FORM + 5)
    XShapes xGroupShapes = null;
    double nreldatewidth;
    double nreltimewidth;
    int nTimeWidth;
    int nDBWidth;
    int nDateWidth;
    XShapeGroup xShapeGroup;



    public TimeStampControl(Resource _oResource, FormHandler _oFormHandler, XNameContainer _xFormName, String _curFieldName, Point _aPoint){
    super(_oFormHandler, "com.sun.star.drawing.ShapeCollection", _aPoint);
        oResource = _oResource;
        xGroupShape = xShape;
        oDateControl  = new DatabaseControl(oFormHandler, _xFormName, _curFieldName, DataType.DATE, aPoint);
        int nDBHeight = oDateControl.getDBHeight();
        nDateWidth = oDateControl.getPreferredWidth();
        oDateControl.setSize(new Size(nDateWidth , nDBHeight));
        Point aTimePoint = new Point(aPoint.X + 10 + nDateWidth, aPoint.Y);
        oTimeControl = new DatabaseControl(oFormHandler, _xFormName, _curFieldName, DataType.TIME, aTimePoint);
        nTimeWidth = oTimeControl.getPreferredWidth();
        oTimeControl.setSize(new Size(nTimeWidth,nDBHeight));
        nDBWidth = nDateWidth + nTimeWidth + 10;
        xShapes.add(oDateControl.xShape);
        xShapes.add(oTimeControl.xShape);
        xShapeGroup = _oFormHandler.xShapeGrouper.group(xShapes);
        xShape = (XShape) UnoRuntime.queryInterface(XShape.class, xShapeGroup);
    }


    public XPropertySet getControlofGroupShapeByIndex(int _i){
        try {
            if (_i < xShapes.getCount()){
                Object oControl = xShapes.getByIndex(_i);
                XControlShape xControlShape = (XControlShape) UnoRuntime.queryInterface(XControlShape.class, oControl);
                XPropertySet xPropertySet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xControlShape.getControl());
                return xPropertySet;
            }
        } catch (Exception e) {
            e.printStackTrace(System.out);
        }
        return null;
    }





    public TimeStampControl(Resource _oResource, GridControl _oGridControl, FieldColumn _curfieldcolumn){
        super(_oGridControl, _curfieldcolumn);
        oResource = _oResource;
        sDateAppendix = oResource.getResText(UIConsts.RID_FORM + 88);
        sTimeAppendix = oResource.getResText(UIConsts.RID_FORM + 89);
        oDateControl = new DatabaseControl(_oGridControl, _curfieldcolumn, DataType.DATE, _curfieldcolumn.FieldTitle + " " + sDateAppendix);
        oTimeControl = new DatabaseControl(_oGridControl, _curfieldcolumn, DataType.TIME, _curfieldcolumn.FieldTitle + " " + sTimeAppendix);
    }

    public void setPropertyValue(String _sPropertyName, Object _aPropertyValue) throws Exception{
        oDateControl.setPropertyValue(_sPropertyName, _aPropertyValue);
        oTimeControl.setPropertyValue(_sPropertyName, _aPropertyValue);
    }


    public int getPreferredWidth(){
        return nDBWidth;
    }


    public void setSize(Size _aSize){
    try {
        int ndatewidth = (int) (nreldatewidth * (double) _aSize.Width);
        int ntimewidth = (int) (nreltimewidth * (double) _aSize.Width);
        oDateControl.xShape.setSize(new Size(ndatewidth, _aSize.Height));
        oTimeControl.xShape.setSize( new Size(ntimewidth, _aSize.Height));
    } catch (PropertyVetoException e){
        e.printStackTrace(System.out);
    }}


    public Size getSize(){
        int ncontrolwidth = oDateControl.xShape.getSize().Width + oTimeControl.xShape.getSize().Width;
        return new Size(ncontrolwidth, oDateControl.xShape.getSize().Height);
    }


    public Point getPosition(){
        return oDateControl.xShape.getPosition();
    }


    public void setPosition(Point _aPoint){
        xShapeGroup.setPosition(_aPoint);
//      oDateControl.xShape.setPosition(_aPoint);
//      Point atimepoint = new Point(oDateControl.xShape.getPosition().X + oDateControl.xShape.getSize().Width, oDateControl.xShape.getPosition().Y );
//      oTimeControl.xShape.setPosition(atimepoint);
    }




}





