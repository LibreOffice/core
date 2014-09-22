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
            xNameContainer = UnoRuntime.queryInterface( XNameContainer.class, oGridModel );
            xNameAccess = UnoRuntime.queryInterface( XNameAccess.class, oGridModel );
            _xFormName.insertByName(_sname, oGridModel);
            xControlModel = UnoRuntime.queryInterface( XControlModel.class, oGridModel );
            xControlShape.setControl(xControlModel);
            xPropertySet = UnoRuntime.queryInterface( XPropertySet.class, oGridModel );
            oFormHandler.xDrawPage.add(xShape);
            xGridColumnFactory = UnoRuntime.queryInterface( XGridColumnFactory.class, oGridModel );
            xComponent = UnoRuntime.queryInterface( XComponent.class, oGridModel );

//      Helper.setUnoPropertyValue(oGridModel, PropertyNames.PROPERTY_NAME, _sname);
            for (int i = 0; i < fieldcolumns.length; i++)
            {
                FieldColumn curfieldcolumn = fieldcolumns[i];
                if (curfieldcolumn.getFieldType() == DataType.TIMESTAMP)
                {
                    TimeStampControl oControl = new TimeStampControl(new Resource(_xMSF, PropertyNames.EMPTY_STRING, "dbw"), this, curfieldcolumn);
                }
                else
                {
                    Control oControl = new DatabaseControl(this, curfieldcolumn);
                }
            }

        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
        }
    }
}
