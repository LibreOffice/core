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
import time

class tdf144940(UITestCase):

    def test_tdf144940_hf(self):

        with TemporaryDirectory() as tempdir:
            xFilePath = os.path.join(tempdir, "hf.ods")

            with self.ui_test.create_doc_in_start_center("calc"):

                xCalcDoc = self.xUITest.getTopFocusWindow()
                gridwin = xCalcDoc.getChild("grid_window")
                gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))

                time.sleep(3)

                with self.ui_test.execute_dialog_through_command(".uno:PageFormatDialog", close_button="ok") as xDialog:
                    xTabs = xDialog.getChild("tabcontrol")
                    select_pos(xTabs, "4")
                    xFirstHeaderSameBtn = self.xUITest.getTopFocusWindow().getChild("checkSameFP")
                    xFirstHeaderSameBtn.executeAction("CLICK", tuple())
                    time.sleep(3)
                    select_pos(xTabs, "5")
                    xFirstFooterSameBtn = self.xUITest.getTopFocusWindow().getChild("checkSameFP")
                    xFirstFooterSameBtn.executeAction("CLICK", tuple())
                    time.sleep(3)




                with self.ui_test.execute_dialog_through_command(".uno:Save", close_button="open") as xSaveDialog:
                    xFileName = xSaveDialog.getChild("file_name")
                    xFileName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                    xFileName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                    xFileName.executeAction("TYPE", mkPropertyValues({"TEXT": xFilePath}))

                time.sleep(3)


            with self.ui_test.load_file(systemPathToFileUrl(xFilePath)) as document:

                time.sleep(3)
                with self.ui_test.execute_dialog_through_command(".uno:PageFormatDialog") as xDialog2:
                    xTabs = xDialog2.getChild("tabcontrol")
                    select_pos(xTabs, "4")
                    xHeaderOnBtn = xDialog2.getChild("checkHeaderOn")
                    self.assertEqual(get_state_as_dict(xHeaderOnBtn)["Selected"], "true")
                    xFirstHeaderSameBtn = xDialog2.getChild("checkSameFP")
                    self.assertEqual(get_state_as_dict(xFirstHeaderSameBtn)["Selected"], "true")

                    time.sleep(3)

                    select_pos(xTabs, "5")
                    xFooterOnBtn = xDialog2.getChild("checkFooterOn")
                    time.sleep(3)
                    #self.assertEqual(get_state_as_dict(xFooterOnBtn)["Selected"], "true")
                    xFirstFooterSameBtn = xDialog2.getChild("checkSameFP")
                    time.sleep(3)
                    self.assertEqual(get_state_as_dict(xFirstFooterSameBtn)["Selected"], "true")

                    time.sleep(3)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
