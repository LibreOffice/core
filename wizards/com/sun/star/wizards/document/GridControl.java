/*************************************************************************
 *
 *  $RCSfile: GridControl.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pjunck $  $Date: 2004-10-27 13:32:04 $
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
        // TODO Auto-generated catch block
        e.printStackTrace(System.out);
    }}
}
