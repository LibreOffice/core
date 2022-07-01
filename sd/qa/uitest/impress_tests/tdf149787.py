#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.framework import UITestCase
from uitest.uihelper.common import select_pos

class TestTdf149787(UITestCase):

    def testTdf149787(self):
        with self.ui_test.create_doc_in_start_center("impress") as document:

            xTemplateDlg = self.xUITest.getTopFocusWindow()
            xCancelBtn = xTemplateDlg.getChild("close")
            self.ui_test.close_dialog_through_button(xCancelBtn)

            xImpressDoc = self.xUITest.getTopFocusWindow()

            self.assertIsNone(document.CurrentSelection)

            xEditWin = xImpressDoc.getChild("impress_win")
            xEditWin.executeAction("SELECT", mkPropertyValues({"OBJECT":"Unnamed Drawinglayer object 1"}))
            self.assertEqual("com.sun.star.drawing.SvxShapeCollection", document.CurrentSelection.getImplementationName())

            with self.ui_test.execute_dialog_through_command(".uno:FormatArea", close_button="cancel") as xDialog:
                tabControl = xDialog.getChild("tabcontrol")
                select_pos(tabControl, "0")
                btnColor = xDialog.getChild("btncolor")
                btnColor.executeAction("CLICK", tuple())
                btnMoreColors = xDialog.getChild("btnMoreColors")

                with self.ui_test.execute_blocking_action(btnMoreColors.executeAction, args=('CLICK', ()), close_button="buttonClose") as dialog:
                    xCloseBtn = dialog.getChild("buttonClose")
                    self.ui_test.wait_until_property_is_updated(xCloseBtn, "Enabled", "true")


# vim: set shiftwidth=4 softtabstop=4 expandtab:
