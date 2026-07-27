# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# Copyright the LibreOffice contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict

def select_chart_type(testCase, xDialog, rName):
    xTypeList = xDialog.getChild("charttype")
    for index in range(int(get_state_as_dict(xTypeList)["Children"])):
        xEntry = xTypeList.getChild(str(index))
        if get_state_as_dict(xEntry)["Text"] == rName:
            xEntry.executeAction("SELECT", mkPropertyValues({}))
            return
    testCase.fail("the chart type dialog offers no " + rName)
