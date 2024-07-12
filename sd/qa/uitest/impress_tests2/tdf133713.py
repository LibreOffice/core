# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.uihelper.common import get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.framework import UITestCase

class Tdf133713(UITestCase):

    def test_Tdf133713(self):
        with self.ui_test.create_doc_in_start_center("impress") as document:

            xTemplateDlg = self.xUITest.getTopFocusWindow()
            xCancelBtn = xTemplateDlg.getChild("close")
            self.ui_test.close_dialog_through_button(xCancelBtn)

            xDoc = self.xUITest.getTopFocusWindow()
            xEditWin = xDoc.getChild("impress_win")

            xEditWin.executeAction("SELECT", mkPropertyValues({"OBJECT":"Unnamed Drawinglayer object 1"}))
            self.assertEqual("com.sun.star.drawing.SvxShapeCollection", document.CurrentSelection.getImplementationName())

            xEditWin.executeAction("TYPE", mkPropertyValues({"TEXT":"one"}))
            xEditWin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
            xEditWin.executeAction("TYPE", mkPropertyValues({"TEXT":"two"}))
            xEditWin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
            xEditWin.executeAction("TYPE", mkPropertyValues({"TEXT":"three"}))

            self.xUITest.executeCommand(".uno:SelectAll")

            with self.ui_test.execute_dialog_through_command(".uno:OutlineBullet") as xDialog:
                xNumFormat = xDialog.getChild("numfmtlb")
                xSelection = xDialog.getChild("selectionrb")
                xIndent = xDialog.getChild("indentmf")
                xRelSize = xDialog.getChild("relsize")

                # Check some default values
                self.assertEqual("Bullet", get_state_as_dict(xNumFormat)["DisplayText"])
                self.assertEqual("true", get_state_as_dict(xSelection)["Checked"])
                self.assertEqual("0", get_state_as_dict(xIndent)["Value"])
                self.assertEqual("45%", get_state_as_dict(xRelSize)["Text"])

            drawPage = document.getDrawPages()[0]
            shape = drawPage[1]
            xEnumeration = shape.Text.createEnumeration()

            # Without the fix in place, this test would have failed with
            # AssertionError: 0 is not None
            for i in range(3):
                self.assertEqual(0, xEnumeration.nextElement().NumberingLevel)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
