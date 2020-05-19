# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.path import get_srcdir_url
from uitest.uihelper.common import get_state_as_dict, type_text
from libreoffice.uno.propertyvalue import mkPropertyValues
import time

def get_url_for_data_file(file_name):
    return get_srcdir_url() + "/sw/qa/uitest/writer_tests/data/" + file_name

class tdf99711(UITestCase):
    def test_tdf99711(self):

        writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf99711.odt"))

        #set measurement to millimeters
        self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog")
        xDialogOpt = self.xUITest.getTopFocusWindow()
        xPages = xDialogOpt.getChild("pages")
        xWriterEntry = xPages.getChild('3')
        xWriterEntry.executeAction("EXPAND", tuple())
        xWriterGeneralEntry = xWriterEntry.getChild('0')
        xWriterGeneralEntry.executeAction("SELECT", tuple())
        xMetric = xDialogOpt.getChild("metric")
        props = {"TEXT": "Millimeter"}
        actionProps = mkPropertyValues(props)
        xMetric.executeAction("SELECT", actionProps)
        xOKBtn = xDialogOpt.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        self.xUITest.executeCommand(".uno:JumpToNextFrame")

        self.xUITest.executeCommand(".uno:Sidebar")
        xWriterEdit.executeAction("SIDEBAR", mkPropertyValues({"PANEL": "TextPropertyPanel"}))

        self.assertEqual(get_state_as_dict(xWriterEdit.getChild('selectwidth'))['Text'], '10.00 mm')
        self.assertEqual(get_state_as_dict(xWriterEdit.getChild('selectheight'))['Text'], '10.00 mm')

        self.xUITest.executeCommand(".uno:Sidebar")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:

