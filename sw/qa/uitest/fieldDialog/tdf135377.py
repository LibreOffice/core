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
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos

class Tdf135377(UITestCase):

    def test_tdf135377(self):

        with self.ui_test.create_doc_in_start_center("writer"):

            with self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField", close_button="cancel") as xDialog:
                xTab = xDialog.getChild("tabcontrol")
                select_pos(xTab, "4")

                xType = xDialog.getChild("type-var")
                xType.getChild('8').executeAction("SELECT", tuple())
                self.assertEqual("User Field", get_state_as_dict(xType)['SelectEntryText'])

                xName = xDialog.getChild("name-var")
                xName.executeAction("TYPE", mkPropertyValues({"TEXT": "AA"}))

                xValue = xDialog.getChild("value-var")
                xValue.executeAction("TYPE", mkPropertyValues({"TEXT": "BB"}))

                xNumFormat = xDialog.getChild("numformat-var")

                # Without the fix in place, this test would have failed with
                # AssertionError: 'General' != ''
                self.assertEqual("General", get_state_as_dict(xNumFormat)['SelectEntryText'])

                xApplyBtn = xDialog.getChild("apply")
                xApplyBtn.executeAction("CLICK", ())

                self.assertEqual("AA", get_state_as_dict(xDialog.getChild("select-var"))["SelectEntryText"])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
