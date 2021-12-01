# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

class versionDialog(UITestCase):

    def test_tdf131931(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf131931.odt")) as writer_doc:

            xWriterDoc = self.xUITest.getTopFocusWindow()

            with self.ui_test.execute_dialog_through_command(".uno:VersionDialog", close_button="close") as xVersionDialog:


                versiondList = xVersionDialog.getChild("versions")

                text = "04/06/2020 15:18\t\tHELLO"
                self.assertEqual(1, len(versiondList.getChildren()))
                self.assertEqual(get_state_as_dict(versiondList.getChild('0'))["Text"].strip(), text)

                xDeleteBtn = xVersionDialog.getChild("delete")
                xDeleteBtn.executeAction("CLICK", tuple())

                self.assertEqual(0, len(versiondList.getChildren()))


# vim: set shiftwidth=4 softtabstop=4 expandtab:
