#! /usr/bin/env python
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

class versionDialog(UITestCase):

    def test_tdf131931(self):

        writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf131931.odt"))

        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()

        self.ui_test.execute_dialog_through_command(".uno:VersionDialog")

        xVersionDialog = self.xUITest.getTopFocusWindow()

        versiondList = xVersionDialog.getChild("versions")

        text = "04/06/2020 15:18\t\tHELLO"
        self.assertEqual(1, len(versiondList.getChildren()))
        self.assertEqual(get_state_as_dict(versiondList.getChild('0'))["Text"].strip(), text)

        xDeleteBtn = xVersionDialog.getChild("delete")
        xDeleteBtn.executeAction("CLICK", tuple())

        self.assertEqual(0, len(versiondList.getChildren()))

        xCloseBtn = xVersionDialog.getChild("close")
        xCloseBtn.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
