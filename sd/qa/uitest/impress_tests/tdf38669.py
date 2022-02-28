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

class tdf38669(UITestCase):

    def test_tdf38669_utf8_figure_text(self):

        with self.ui_test.create_doc_in_start_center("impress") as impress_document:

            # Close template dialog
            xTemplateDlg = self.xUITest.getTopFocusWindow()
            xCancelBtn = xTemplateDlg.getChild("close")
            self.ui_test.close_dialog_through_button(xCancelBtn)

            # Insert shape with Ctrl key
            xArgs = mkPropertyValues({"KeyModifier": 8192})
            self.xUITest.executeCommandWithParameters(".uno:BasicShapes.rectangle", xArgs)
            self.assertEqual("com.sun.star.drawing.SvxShapeCollection", impress_document.CurrentSelection.getImplementationName())

            # Insert an UTF-8 character
            xImpressDoc = self.xUITest.getTopFocusWindow()
            xEditWin = xImpressDoc.getChild("impress_win")
            xEditWin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "SHIFT+°"}))
            self.xUITest.executeCommand(".uno:SelectAll")

            # Without the fix in place, this test would have failed with
            # AssertionError: '°' != 'ㅀ'
            self.assertEqual("°", get_state_as_dict(xEditWin)["SelectedText"])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
