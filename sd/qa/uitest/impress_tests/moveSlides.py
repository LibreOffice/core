# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file, type_text
from libreoffice.uno.propertyvalue import mkPropertyValues

class moveSlides(UITestCase):

    def renameSlide(self, editWin, currentPos):
        self.assertEqual(currentPos, get_state_as_dict(editWin)['CurrentSlide'])
        self.assertEqual("Slide " + currentPos, get_state_as_dict(editWin)['CurrentSlideName'])

        self.ui_test.execute_dialog_through_command(".uno:RenamePage")
        xDialog = self.xUITest.getTopFocusWindow()

        name_entry = xDialog.getChild("name_entry")
        name_entry.executeAction("TYPE", mkPropertyValues({"TEXT": "Test " + currentPos}))

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.assertEqual(currentPos, get_state_as_dict(editWin)['CurrentSlide'])
        self.assertEqual("Test " + currentPos, get_state_as_dict(editWin)['CurrentSlideName'])

    def test_moveSlides(self):

        self.ui_test.create_doc_in_start_center("impress")

        xTemplateDlg = self.xUITest.getTopFocusWindow()
        xCancelBtn = xTemplateDlg.getChild("close")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        xImpress = self.xUITest.getTopFocusWindow()
        xEditWin = xImpress.getChild("impress_win")

        self.renameSlide(xEditWin, '1')

        for i in range(2):
            self.xUITest.executeCommand(".uno:InsertPage")

            self.renameSlide(xEditWin, str(i + 2))

        # Move slide 'Test 3' up
        for i in range(2, 0, -1):
            self.xUITest.executeCommand(".uno:MovePageUp")

            self.assertEqual(str(i), get_state_as_dict(xEditWin)['CurrentSlide'])
            self.assertEqual("Test 3", get_state_as_dict(xEditWin)['CurrentSlideName'])

        # Move slide 'Test 3' down
        for i in range(2, 4, 1):
            self.xUITest.executeCommand(".uno:MovePageDown")

            self.assertEqual(str(i), get_state_as_dict(xEditWin)['CurrentSlide'])
            self.assertEqual("Test 3", get_state_as_dict(xEditWin)['CurrentSlideName'])

        # Move slide 'Test 3' to the top
        self.xUITest.executeCommand(".uno:MovePageFirst")

        self.assertEqual('1', get_state_as_dict(xEditWin)['CurrentSlide'])
        self.assertEqual("Test 3", get_state_as_dict(xEditWin)['CurrentSlideName'])

        # Move slide 'Test 3' to the bottom
        self.xUITest.executeCommand(".uno:MovePageLast")

        self.assertEqual('3', get_state_as_dict(xEditWin)['CurrentSlide'])
        self.assertEqual("Test 3", get_state_as_dict(xEditWin)['CurrentSlideName'])

        self.ui_test.close_doc()


# vim: set shiftwidth=4 softtabstop=4 expandtab:
