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

package ifc.sheet;

import com.sun.star.container.XNameAccess;
import com.sun.star.sheet.DataPilotFieldGroupBy;
import com.sun.star.sheet.DataPilotFieldGroupInfo;
import com.sun.star.sheet.XDataPilotField;
import com.sun.star.sheet.XDataPilotFieldGrouping;
import com.sun.star.uno.UnoRuntime;
import lib.MultiMethodTest;

/**
 *
 * @author sw93809
 */
public class _XDataPilotFieldGrouping extends MultiMethodTest
{
    public XDataPilotFieldGrouping oObj = null;

    public void _createNameGroup() {
        boolean result = true;
        try {
            XDataPilotField xDataPilotField = (XDataPilotField) UnoRuntime.queryInterface(XDataPilotField.class, oObj);
            XNameAccess xNameAccess = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, xDataPilotField.getItems ());
            String[] elements = xNameAccess.getElementNames ();
            oObj.createNameGroup(elements);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Exception while checking createNameGroup"+e);
            result = false;
        }
        tRes.tested ("createNameGroup()",result);
    }

    public void _createDateGroup() {
        boolean result = true;
        try {
            DataPilotFieldGroupInfo aInfo = new DataPilotFieldGroupInfo();
            aInfo.GroupBy = DataPilotFieldGroupBy.MONTHS;
            aInfo.HasDateValues = true;
            oObj.createDateGroup(aInfo);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Exception while checking createDateGroup"+e);
            result = false;
        }
        tRes.tested ("createDateGroup()",result);
    }
}
