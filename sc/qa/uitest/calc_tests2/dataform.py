# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues

class dataform(UITestCase):
    def test_dataform(self):
        with self.ui_test.create_doc_in_start_center("calc"):

            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A10"}))

            with self.ui_test.execute_dialog_through_command(".uno:DataForm", close_button="close"):
                pass


# vim: set shiftwidth=4 softtabstop=4 expandtab:
