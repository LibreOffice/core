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

class InsertField(UITestCase):

    def test_inputListField(self):

        with self.ui_test.create_doc_in_start_center("writer") as writer_doc:

            with self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField", close_button="cancel") as xDialog:
                xTab = xDialog.getChild("tabcontrol")
                select_pos(xTab, "2")

                xType = xDialog.getChild("type-func")
                xType.getChild('1').executeAction("SELECT", tuple())
                self.assertEqual("Input list", get_state_as_dict(xType)['SelectEntryText'])

                xItem = xDialog.getChild("item")
                xAdd = xDialog.getChild("add")

                xItem.executeAction("TYPE", mkPropertyValues({"TEXT":"Item1"}))
                xAdd.executeAction("CLICK", tuple())

                xItem.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xItem.executeAction("TYPE", mkPropertyValues({"TEXT":"Item2"}))
                xAdd.executeAction("CLICK", tuple())

                xItem.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xItem.executeAction("TYPE", mkPropertyValues({"TEXT":"Item3"}))
                xAdd.executeAction("CLICK", tuple())

                xListItem = xDialog.getChild("listitems")
                self.assertEqual('3', get_state_as_dict(xListItem)['Children'])

                xOkBtn = xDialog.getChild("ok")
                xOkBtn.executeAction("CLICK", tuple())

            # Select the field
            self.xUITest.executeCommand(".uno:SelectAll")

            with self.ui_test.execute_dialog_through_command(".uno:FieldDialog") as xDialog:
                xType = xDialog.getChild("type-func")
                self.assertEqual("Input list", get_state_as_dict(xType)['SelectEntryText'])

                xListItem = xDialog.getChild("listitems")
                self.assertEqual('3', get_state_as_dict(xListItem)['Children'])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
