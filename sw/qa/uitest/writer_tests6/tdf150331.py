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
from uitest.uihelper.common import get_state_as_dict, select_pos, type_text

class tdf150331(UITestCase):

    def test_tdf150331(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:

            with self.ui_test.execute_dialog_through_command(".uno:AutoCorrectDlg") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "4")

                xEnableWordComplete = xDialog.getChild("enablewordcomplete")
                xAppendSpace = xDialog.getChild("appendspace")
                xShowAsTip = xDialog.getChild("showastip")
                xAcceptWith = xDialog.getChild("acceptwith")
                xMinWordLen = xDialog.getChild("minwordlen")
                xMaxEntries = xDialog.getChild("maxentries")

                xEnableWordComplete.executeAction("CLICK", tuple())

                self.assertEqual("true", get_state_as_dict(xEnableWordComplete)['Selected'])
                self.assertEqual("false", get_state_as_dict(xAppendSpace)['Selected'])
                self.assertEqual("true", get_state_as_dict(xShowAsTip)['Selected'])
                self.assertEqual("Return", get_state_as_dict(xAcceptWith)['SelectEntryText'])
                self.assertEqual("8", get_state_as_dict(xMinWordLen)['Value'])
                self.assertEqual("1000", get_state_as_dict(xMaxEntries)['Value'])

            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            type_text(xWriterEdit, "sun")
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))

            self.assertEqual("sunday", document.Text.String)

            with self.ui_test.execute_dialog_through_command(".uno:AutoCorrectDlg") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "4")

                xShowAsTip = xDialog.getChild("showastip")

                xShowAsTip.executeAction("CLICK", tuple())
                self.assertEqual("false", get_state_as_dict(xShowAsTip)['Selected'])

            type_text(xWriterEdit, " sunny")

            # Without the fix in place, this test would have failed with
            # AssertionError: 'sunday sunny' != 'Sunday sundayny'
            self.assertEqual("Sunday sunny", document.Text.String)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
