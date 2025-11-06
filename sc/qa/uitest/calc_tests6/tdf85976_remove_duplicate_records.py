# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from libreoffice.calc.document import get_cell_by_position
from uitest.uihelper.common import get_url_for_data_file
from uitest.uihelper.common import get_state_as_dict

class tdf85976(UITestCase):
    def test_tdf85976(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf85976.ods")) as calc_doc:
            with self.ui_test.execute_dialog_through_command(".uno:HandleDuplicateRecords") as xDialog:
                xColumn = xDialog.getChild("column")
                xRow = xDialog.getChild("row")
                xIncludesHeaders = xDialog.getChild("includesheaders")
                xAllCheckBtn = xDialog.getChild("allcheckbtn")
                xCheckList = xDialog.getChild("checklist")
                xSelect = xDialog.getChild("select")
                xRemove = xDialog.getChild("remove")
                self.assertEqual("false", get_state_as_dict(xColumn)["Checked"])
                self.assertEqual("true", get_state_as_dict(xRow)["Checked"])
                self.assertEqual("false", get_state_as_dict(xIncludesHeaders)["Selected"])
                self.assertEqual("true", get_state_as_dict(xAllCheckBtn)["Selected"])
                self.assertEqual("6", get_state_as_dict(xCheckList)["Children"])
                self.assertEqual("false", get_state_as_dict(xSelect)["Checked"])
                self.assertEqual("true", get_state_as_dict(xRemove)["Checked"])

                expectedText = ["A", "B", "C", "D", "E", "F"]
                for i in range(6):
                    self.assertEqual(expectedText[i], get_state_as_dict(xCheckList.getChild(i))["Text"])
                    self.assertEqual("true", get_state_as_dict(xCheckList.getChild(i))["IsChecked"])

                xIncludesHeaders.executeAction("CLICK", tuple())

                expectedText = ["Id", "SepalLengthCm", "SepalWidthCm", "PetalLengthCm", "PetalWidthCm", "Species"]
                for i in range(6):
                    self.assertEqual(expectedText[i], get_state_as_dict(xCheckList.getChild(i))["Text"])
                    self.assertEqual("true", get_state_as_dict(xCheckList.getChild(i))["IsChecked"])

                xAllCheckBtn.executeAction("CLICK", tuple())
                for i in range(6):
                    self.assertEqual(expectedText[i], get_state_as_dict(xCheckList.getChild(i))["Text"])
                    self.assertEqual("false", get_state_as_dict(xCheckList.getChild(i))["IsChecked"])

                xCheckList.getChild(5).executeAction("CLICK", tuple())

                for i in range(5):
                    self.assertEqual(expectedText[i], get_state_as_dict(xCheckList.getChild(i))["Text"])
                    self.assertEqual("false", get_state_as_dict(xCheckList.getChild(i))["IsChecked"])

                self.assertEqual("Species", get_state_as_dict(xCheckList.getChild(5))["Text"])
                self.assertEqual("true", get_state_as_dict(xCheckList.getChild(5))["IsChecked"])

            self.assertEqual("Id", get_cell_by_position(calc_doc, 0, 0, 0).getString())
            self.assertEqual("1", get_cell_by_position(calc_doc, 0, 0, 1).getString())
            self.assertEqual("51", get_cell_by_position(calc_doc, 0, 0, 2).getString())
            self.assertEqual("101", get_cell_by_position(calc_doc, 0, 0, 3).getString())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
