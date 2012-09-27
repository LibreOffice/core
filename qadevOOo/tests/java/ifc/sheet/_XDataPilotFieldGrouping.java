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

package ifc.sheet;

import com.sun.star.container.XNameAccess;
import com.sun.star.sheet.DataPilotFieldGroupBy;
import com.sun.star.sheet.DataPilotFieldGroupInfo;
import com.sun.star.sheet.XDataPilotField;
import com.sun.star.sheet.XDataPilotFieldGrouping;
import com.sun.star.uno.UnoRuntime;
import lib.MultiMethodTest;

public class _XDataPilotFieldGrouping extends MultiMethodTest
{
    public XDataPilotFieldGrouping oObj = null;

    public void _createNameGroup() {
        boolean result = true;
        try {
            XDataPilotField xDataPilotField = UnoRuntime.queryInterface(XDataPilotField.class, oObj);
            XNameAccess xNameAccess = UnoRuntime.queryInterface(XNameAccess.class, xDataPilotField.getItems ());
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
