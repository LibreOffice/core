# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos
from libreoffice.uno.propertyvalue import mkPropertyValues
from tempfile import TemporaryDirectory
from org.libreoffice.unotest import systemPathToFileUrl
import os.path

class tdf144940(UITestCase):

    def test_tdf144940(self):

        with TemporaryDirectory() as tempdir:
            xFilePath = os.path.join(tempdir, "tdf144940-tmp.ods")

            calc_doc = self.ui_test.create_doc_in_start_center("calc")

            self.ui_test.execute_dialog_through_command(".uno:PageFormatDialog")
            xDialog = self.xUITest.getTopFocusWindow()
            xTabs = xDialog.getChild("tabcontrol")
            select_pos(xTabs, "4")

            xCheckHeaderOn = xDialog.getChild("checkHeaderOn")
            xCheckSameFP = xDialog.getChild("checkSameFP")
            xCheckSameLR = xDialog.getChild("checkSameLR")

            self.assertEqual("true", get_state_as_dict(xCheckHeaderOn)["Selected"])
            self.assertEqual("true", get_state_as_dict(xCheckSameLR)["Selected"])
            self.assertEqual("false", get_state_as_dict(xCheckSameFP)["Selected"])

            xCheckSameFP.executeAction("CLICK", tuple())

            self.assertEqual("true", get_state_as_dict(xCheckSameFP)["Selected"])

            xOkBtn = xDialog.getChild("ok")
            self.ui_test.close_dialog_through_button(xOkBtn)

            self.ui_test.execute_dialog_through_command(".uno:Save")
            xSaveDialog = self.xUITest.getTopFocusWindow()
            xFileName = xSaveDialog.getChild("file_name")
            xFileName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
            xFileName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
            xFileName.executeAction("TYPE", mkPropertyValues({"TEXT": xFilePath}))

            xOkBtn = xSaveDialog.getChild("open")
            self.ui_test.close_dialog_through_button(xOkBtn)

            self.ui_test.close_doc()

            self.ui_test.load_file(systemPathToFileUrl(xFilePath))
            document = self.ui_test.get_component()

            xPageStyles = document.StyleFamilies.getByIndex(1)
            xDefaultPageStyle = xPageStyles.getByIndex(0)

            # Without the fix in place, this test would have failed with
            # AssertionError: False is not true
            self.assertTrue(xDefaultPageStyle.HeaderOn)
            self.assertTrue(xDefaultPageStyle.FooterOn)
            self.assertTrue(xDefaultPageStyle.FirstPageHeaderIsShared)
            self.assertTrue(xDefaultPageStyle.FirstPageFooterIsShared)

            self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
