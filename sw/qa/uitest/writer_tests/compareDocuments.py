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
from libreoffice.uno.propertyvalue import mkPropertyValues
import datetime

class compareDocuments(UITestCase):

    def test_tdf130960(self):

        writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf130960.odt"))

        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()

        self.ui_test.execute_dialog_through_command(".uno:CompareDocuments")

        xOpenDialog = self.xUITest.getTopFocusWindow()
        xFileName = xOpenDialog.getChild("file_name")
        xFileName.executeAction("TYPE", mkPropertyValues({"TEXT": get_url_for_data_file("tdf130960_2.odt")}))

        xOpenBtn = xOpenDialog.getChild("open")
        xOpenBtn.executeAction("CLICK", tuple())

        # Close the dialog and open it again so the list of changes is updated
        xTrackDlg = self.xUITest.getTopFocusWindow()
        xcloseBtn = xTrackDlg.getChild("close")
        xcloseBtn.executeAction("CLICK", tuple())

        self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptTrackedChanges")
        xTrackDlg = self.xUITest.getTopFocusWindow()
        changesList = xTrackDlg.getChild("writerchanges")

        text = "Unknown Author\t" + datetime.datetime.now().strftime("%m/%d/%Y")
        self.assertEqual(2, len(changesList.getChildren()))
        self.assertTrue(get_state_as_dict(changesList.getChild('0'))["Text"].startswith(text))
        self.assertTrue(get_state_as_dict(changesList.getChild('1'))["Text"].startswith(text))

        xcloseBtn = xTrackDlg.getChild("close")
        xcloseBtn.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

    def test_tdf137855(self):

        writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf137855.odt"))

        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()

        self.ui_test.execute_dialog_through_command(".uno:CompareDocuments")

        xOpenDialog = self.xUITest.getTopFocusWindow()
        xFileName = xOpenDialog.getChild("file_name")
        xFileName.executeAction("TYPE", mkPropertyValues({"TEXT": get_url_for_data_file("tdf137855_2.odt")}))

        xOpenBtn = xOpenDialog.getChild("open")
        xOpenBtn.executeAction("CLICK", tuple())

        # Close the dialog and open it again so the list of changes is updated
        xTrackDlg = self.xUITest.getTopFocusWindow()
        xcloseBtn = xTrackDlg.getChild("close")
        xcloseBtn.executeAction("CLICK", tuple())

        self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptTrackedChanges")
        xTrackDlg = self.xUITest.getTopFocusWindow()
        changesList = xTrackDlg.getChild("writerchanges")

        # Check the number of changes
        self.assertEqual(263, len(changesList.getChildren()))

        # Without the fix in place, this test would have crashed here
        xAccBtn = xTrackDlg.getChild("acceptall")
        xAccBtn.executeAction("CLICK", tuple())

        self.assertEqual(0, len(changesList.getChildren()))

        xcloseBtn = xTrackDlg.getChild("close")
        xcloseBtn.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
