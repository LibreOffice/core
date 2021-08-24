# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import select_pos, get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_url_for_data_file

class tdf127900(UITestCase):

    def test_tdf127900(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf127900.fodp")) as doc:

            xMasterLang = doc.MasterPages.getByIndex(0).getByIndex(1).CharLocale.Language
            xSlideLang = doc.DrawPages.getByIndex(1).getByIndex(1).CharLocale.Language

            self.assertEqual(xMasterLang, xSlideLang)

            self.xUITest.executeCommand(".uno:SlideMasterPage")

            xMaster = self.xUITest.getTopFocusWindow().getChild("impress_win")

            xMaster.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))
            xMaster.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))

            with self.ui_test.execute_dialog_through_command(".uno:FontDialog") as xDlg:

                select_pos(xDlg.getChild("westlanglb-cjk"), "10")

            xMasterLang = doc.MasterPages.getByIndex(0).getByIndex(1).CharLocale.Language
            xSlideLang = doc.DrawPages.getByIndex(1).getByIndex(1).CharLocale.Language

            self.assertEqual(xMasterLang, xSlideLang)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
