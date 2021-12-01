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
from uitest.uihelper.common import select_pos
from libreoffice.uno.propertyvalue import mkPropertyValues
from tempfile import TemporaryDirectory
from org.libreoffice.unotest import systemPathToFileUrl
import os.path

class tdf144940(UITestCase):

    def test_tdf144940(self):

        with TemporaryDirectory() as tempdir:
            xFilePath = os.path.join(tempdir, "tdf144940-tmp.ods")

            with self.ui_test.create_doc_in_start_center("calc"):

                with self.ui_test.execute_dialog_through_command(".uno:PageFormatDialog") as xDialog:
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

                with self.ui_test.execute_dialog_through_command(".uno:Save", close_button="open") as xSaveDialog:
                    xFileName = xSaveDialog.getChild("file_name")
                    xFileName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                    xFileName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                    xFileName.executeAction("TYPE", mkPropertyValues({"TEXT": xFilePath}))

            with self.ui_test.load_file(systemPathToFileUrl(xFilePath)) as document:

                xPageStyles = document.StyleFamilies.getByIndex(1)
                xDefaultPageStyle = xPageStyles.getByIndex(0)

                # Without the fix in place, this test would have failed with
                # AssertionError: False is not true
                self.assertTrue(xDefaultPageStyle.HeaderOn)
                self.assertTrue(xDefaultPageStyle.FooterOn)
                self.assertTrue(xDefaultPageStyle.FirstPageHeaderIsShared)
                self.assertTrue(xDefaultPageStyle.FirstPageFooterIsShared)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
