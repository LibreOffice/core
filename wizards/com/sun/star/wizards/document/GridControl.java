/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: GridControl.java,v $
 * $Revision: 1.7 $
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
import com.sun.star.awt.XControlModel;
import com.sun.star.beans.XPropertySet;
import com.sun.star.wizards.common.*;
import com.sun.star.wizards.db.FieldColumn;
import com.sun.star.sdbc.*;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNameContainer;
import com.sun.star.form.XGridColumnFactory;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;

public class GridControl extends Shape
{

    FieldColumn[] fieldcolumns;
    public XNameContainer xNameContainer;
    public XGridColumnFactory xGridColumnFactory;
    public XPropertySet xPropertySet;
    XNameAccess xNameAccess;
    final String SODEFAULTNAME = "Grid1";
    XControlModel xControlModel;
    public XComponent xComponent;

    public GridControl(XMultiServiceFactory _xMSF, String _sname, FormHandler _oFormHandler, XNameContainer _xFormName, FieldColumn[] _fieldcolumns, Point _aPoint, Size _aSize)
    {
        super(_oFormHandler, _aPoint, _aSize);
        try
        {
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
            xComponent = (XComponent) UnoRuntime.queryInterface(XComponent.class, oGridModel);

//      Helper.setUnoPropertyValue(oGridModel, "Name", _sname);
            for (int i = 0; i < fieldcolumns.length; i++)
            {
                FieldColumn curfieldcolumn = fieldcolumns[i];
                if (curfieldcolumn.FieldType == DataType.TIMESTAMP)
                {
                    TimeStampControl oControl = new TimeStampControl(new Resource(_xMSF, "", "dbw"), this, curfieldcolumn);
                }
                else
                {
                    Control oControl = new DatabaseControl(this, curfieldcolumn);
                }
            }

        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
        }
    }
}
