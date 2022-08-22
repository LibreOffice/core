# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf91762(UITestCase):

   def test_tdf91762(self):
        with self.ui_test.create_doc_in_start_center("impress") as document:
            xTemplateDlg = self.xUITest.getTopFocusWindow()
            xCancelBtn = xTemplateDlg.getChild("close")
            self.ui_test.close_dialog_through_button(xCancelBtn)

            self.xUITest.executeCommand(".uno:AssignLayout?WhatLayout:long=1")

            with self.ui_test.execute_dialog_through_command(".uno:InsertTable") as xDialog:
                self.assertEqual('5', get_state_as_dict(xDialog.getChild('columns'))['Text'])
                self.assertEqual('2', get_state_as_dict(xDialog.getChild('rows'))['Text'])

            self.assertEqual(1929, document.DrawPages[0].getByIndex(1).BoundRect.Height)
            self.assertEqual(25198, document.DrawPages[0].getByIndex(1).Size.Width)
            self.assertEqual(1923, document.DrawPages[0].getByIndex(1).Size.Height)

            self.assertEqual(1400, document.DrawPages[0].getByIndex(1).Position.X)
            self.assertEqual(3685, document.DrawPages[0].getByIndex(1).Position.Y)

            xDoc = self.xUITest.getTopFocusWindow()
            xEdit = xDoc.getChild("impress_win")
            for i in range(5):
                xEdit.executeAction("TYPE", mkPropertyValues({"TEXT":"test"}))
                xEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))

            # tdf#138011: Without the fix in place, this test would have failed with
            # AssertionError: 5504 != 3559
            self.assertEqual(5504, document.DrawPages[0].getByIndex(1).BoundRect.Height)


# vim: set shiftwidth=4 softtabstop=4 expandtab:
