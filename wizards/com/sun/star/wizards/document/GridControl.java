/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: GridControl.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:27:15 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/package com.sun.star.wizards.document;

import com.sun.star.awt.Point;
import com.sun.star.awt.Size;
import com.sun.star.awt.XControlModel;
import com.sun.star.beans.XPropertySet;
import com.sun.star.wizards.common.*;
import com.sun.star.wizards.db.FieldColumn;
import com.sun.star.sdbc.*;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNameContainer;
import com.sun.star.container.XNamed;
import com.sun.star.form.XGridColumnFactory;
import com.sun.star.lang.XMultiServiceFactory;


public class GridControl extends Shape{
    FieldColumn[] fieldcolumns;
    public XNameContainer xNameContainer;
    public XGridColumnFactory xGridColumnFactory;
    public XPropertySet xPropertySet;
    XNameAccess xNameAccess;
    final String SODEFAULTNAME = "Grid1";
    XControlModel xControlModel;

    public GridControl(XMultiServiceFactory _xMSF, String _sname, FormHandler _oFormHandler, XNameContainer _xFormName, FieldColumn[] _fieldcolumns, Point _aPoint, Size _aSize) {
    super(_oFormHandler, _aPoint, _aSize);
    try {
        fieldcolumns = _fieldcolumns;
        Object oGridModel = oFormHandler.xMSFDoc.createInstance(oFormHandler.sModelServices[FormHandler.SOGRIDCONTROL]);
        xNameContainer = (XNameContainer) UnoRuntime.queryInterface(XNameContainer.class, oGridModel);
        xNameAccess = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, oGridModel);
        _xFormName.insertByName(_sname, oGridModel);
        xControlModel = (XControlModel) UnoRuntime.queryInterface(XControlModel.class, oGridModel);
        xControlShape.setControl(xControlModel);
        xPropertySet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, oGridModel);
        oFormHandler.xDrawPage.add(xShape);
        xGridColumnFactory = (XGridColumnFactory) UnoRuntime.queryInterface(XGridColumnFactory.class, oGridModel);
//      Helper.setUnoPropertyValue(oGridModel, "Name", _sname);
        for (int i = 0; i < fieldcolumns.length; i++){
            FieldColumn curfieldcolumn = fieldcolumns[i];
            if (curfieldcolumn.FieldType == DataType.TIMESTAMP){
                TimeStampControl oControl = new TimeStampControl(new Resource(_xMSF, "", "dbw"),this, curfieldcolumn);
            }
            else{
                Control oControl = new DatabaseControl(this, curfieldcolumn);
            }
        }

    } catch (Exception e) {
        e.printStackTrace(System.out);
    }}
}
