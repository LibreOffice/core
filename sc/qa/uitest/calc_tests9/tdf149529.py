# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from uitest.uihelper.common import select_by_text
from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import get_cell_by_position

class Tdf149529(UITestCase):

   def test_tdf149529(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:

            with self.ui_test.execute_dialog_through_command(".uno:Insert") as xDialog:
                xBrowse = xDialog.getChild("browse")
                xFromFile = xDialog.getChild("fromfile")
                xTables = xDialog.getChild("tables")

                self.assertEqual("false", get_state_as_dict(xBrowse)['Enabled'])
                self.assertEqual("0", get_state_as_dict(xTables)['Children'])

                xFromFile.executeAction("CLICK", tuple())

                self.assertEqual("true", get_state_as_dict(xBrowse)['Enabled'])

                with self.ui_test.execute_blocking_action(
                        xBrowse.executeAction, args=('CLICK', ()), close_button="") as dialog:
                    xFileName = dialog.getChild("file_name")
                    xFileName.executeAction("TYPE", mkPropertyValues({"TEXT": get_url_for_data_file("tdf149529.ods")}))
                    xOpen = dialog.getChild("open")
                    xOpen.executeAction("CLICK", tuple())

                self.ui_test.wait_until_property_is_updated(xTables, "Children", "1")
                self.assertEqual("Codes", get_state_as_dict(xTables.getChild('0'))['Text'])

            self.assertEqual("GROUP", get_cell_by_position(document, 0, 0, 0).getString())

            with self.ui_test.execute_dialog_through_command(".uno:ConditionalFormatManagerDialog") as xCondFormatMgr:
                xList = xCondFormatMgr.getChild("CONTAINER")
                self.assertEqual("1", get_state_as_dict(xList)['Children'])
                self.assertEqual("P6:P10\tColorScale", get_state_as_dict(xList.getChild('0'))['Text'])

            # Without the fix in place, this test would have crashed here

# vim: set shiftwidth=4 softtabstop=4 expandtab:
