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
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

#Bug 128192 - Fix Tools/AutoCorrect/Apply to apply for all styles

class tdf128192(UITestCase):

    def test_tdf128192(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf128192.odt")):
            MainWindow = self.xUITest.getTopFocusWindow()
            writer_edit = MainWindow.getChild("writer_edit")

            # perform Tools/AutoCorrect/Apply on the whole document
            self.xUITest.executeCommand(".uno:SelectAll")
            self.xUITest.executeCommand(".uno:AutoFormatApply")

            # test for each style whether "Replace dashes" works in AutoCorrect
            writer_edit.executeAction("SELECT", mkPropertyValues({"END_POS": "9", "START_POS": "14"}))
            self.assertEqual(get_state_as_dict(writer_edit)["SelectedText"], "A – B")

            self.xUITest.executeCommand(".uno:GoUp")
            writer_edit.executeAction("SELECT", mkPropertyValues({"END_POS": "19", "START_POS": "24"}))
            self.assertEqual(get_state_as_dict(writer_edit)["SelectedText"], "A – B")

            self.xUITest.executeCommand(".uno:GoUp")
            writer_edit.executeAction("SELECT", mkPropertyValues({"END_POS": "12", "START_POS": "17"}))
            self.assertEqual(get_state_as_dict(writer_edit)["SelectedText"], "A – B")

            self.xUITest.executeCommand(".uno:GoUp")
            writer_edit.executeAction("SELECT", mkPropertyValues({"END_POS": "11", "START_POS": "16"}))
            self.assertEqual(get_state_as_dict(writer_edit)["SelectedText"], "A – B")

            self.xUITest.executeCommand(".uno:GoUp")
            writer_edit.executeAction("SELECT", mkPropertyValues({"END_POS": "11", "START_POS": "16"}))
            self.assertEqual(get_state_as_dict(writer_edit)["SelectedText"], "A – B")

            self.xUITest.executeCommand(".uno:GoUp")
            writer_edit.executeAction("SELECT", mkPropertyValues({"END_POS": "11", "START_POS": "16"}))
            self.assertEqual(get_state_as_dict(writer_edit)["SelectedText"], "A – B")

            self.xUITest.executeCommand(".uno:GoUp")
            writer_edit.executeAction("SELECT", mkPropertyValues({"END_POS": "11", "START_POS": "16"}))
            self.assertEqual(get_state_as_dict(writer_edit)["SelectedText"], "A – B")

            self.xUITest.executeCommand(".uno:GoUp")
            writer_edit.executeAction("SELECT", mkPropertyValues({"END_POS": "16", "START_POS": "11"}))
            self.assertEqual(get_state_as_dict(writer_edit)["SelectedText"], "A – B")

            self.xUITest.executeCommand(".uno:GoUp")
            writer_edit.executeAction("SELECT", mkPropertyValues({"END_POS": "25", "START_POS": "30"}))
            self.assertEqual(get_state_as_dict(writer_edit)["SelectedText"], "A – B")

            self.xUITest.executeCommand(".uno:GoUp")
            writer_edit.executeAction("SELECT", mkPropertyValues({"END_POS": "10", "START_POS": "15"}))
            self.assertEqual(get_state_as_dict(writer_edit)["SelectedText"], "A – B")

            self.xUITest.executeCommand(".uno:GoUp")
            writer_edit.executeAction("SELECT", mkPropertyValues({"END_POS": "7", "START_POS": "12"}))
            self.assertEqual(get_state_as_dict(writer_edit)["SelectedText"], "A – B")


# vim: set shiftwidth=4 softtabstop=4 expandtab:
