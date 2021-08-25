# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

#Find similarities

class findSimilarities(UITestCase):
    def test_find_similarities(self):
        with self.ui_test.load_file(get_url_for_data_file("findReplace.odt")) as writer_doc:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")
            #verify: we are on page 1
            self.assertEqual(get_state_as_dict(xWriterEdit)["CurrentPage"], "1")
            # search and replace dialog
            #find = seco
            self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog")
            xDialog = self.xUITest.getTopFocusWindow()
            searchterm = xDialog.getChild("searchterm")
            searchterm.executeAction("TYPE", mkPropertyValues({"TEXT":"seco"}))  #seco
            # check similarities, button similarities, set values = 1; close dialog with OK
            similarity = xDialog.getChild("similarity")
            similaritybtn = xDialog.getChild("similaritybtn")
            similarity.executeAction("CLICK", tuple())
            with self.ui_test.execute_blocking_action(similaritybtn.executeAction, args=('CLICK', ())) as dialog:
                otherfld = dialog.getChild("otherfld")
                longerfld = dialog.getChild("longerfld")
                shorterfld = dialog.getChild("shorterfld")
                otherfld.executeAction("DOWN", tuple())
                longerfld.executeAction("DOWN", tuple())
                shorterfld.executeAction("DOWN", tuple())

            #open dialog again, verify values = 1; close dialog with cancel
            with self.ui_test.execute_blocking_action(similaritybtn.executeAction, args=('CLICK', ()), close_button="cancel") as dialog:
                otherfld = dialog.getChild("otherfld")
                longerfld = dialog.getChild("longerfld")
                shorterfld = dialog.getChild("shorterfld")
                self.assertEqual(get_state_as_dict(otherfld)["Text"], "1")
                self.assertEqual(get_state_as_dict(longerfld)["Text"], "1")
                self.assertEqual(get_state_as_dict(shorterfld)["Text"], "1")

            xsearch = xDialog.getChild("search")
            xsearch.executeAction("CLICK", tuple())
            # Find next - not found
            self.assertEqual(get_state_as_dict(xWriterEdit)["CurrentPage"], "1")
            #find = secon
            searchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
            searchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
            searchterm.executeAction("TYPE", mkPropertyValues({"TEXT":"secon"}))
            # Find next - verify: we are on page 2
            xsearch = xDialog.getChild("search")
            xsearch.executeAction("CLICK", tuple())
            self.assertEqual(get_state_as_dict(xWriterEdit)["CurrentPage"], "2")
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "LEFT"})) #word "second" is selected
            #find = irst
            searchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
            searchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
            searchterm.executeAction("TYPE", mkPropertyValues({"TEXT":"irst"}))
            # Find next - verify: we are on page 1
            xsearch.executeAction("CLICK", tuple())
            self.assertEqual(get_state_as_dict(xWriterEdit)["CurrentPage"], "1")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
